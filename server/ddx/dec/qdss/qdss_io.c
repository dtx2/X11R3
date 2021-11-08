/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <errno.h>

#define  NEED_EVENTS

#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdioctl.h"
#include "qdreg.h"

#include "X.h"
#include "Xproto.h"	/* needed for xEvent */

#include "scrnintstr.h"
#include "pixmapstr.h"
#include "inputstr.h"
#include "cursorstr.h"		/* needed by ProcessInputEvents */

#include "mi.h"
#include "qd.h"
#include "qdprocs.h"
#include "libtl/tl.h"

static Bool qdSaveScreen();
void	qdFreeResource();
Bool	qdScreenClose();

extern void FatalError();
extern int  qdGetMotionEvents();
extern void qdChangePointerControl(), qdChangeKeyboardControl(), qdBell();
extern void qdBlockHandler();
extern void NoopDDA();

extern int errno;

int	fd_qdss;
int     Nplanes;
int     Nentries;
                      /* This variable will be set when tlinit is     */
                      /* called and we determine the number of planes */
int     Nchannels;
unsigned int   Allplanes;


static struct qdmap *	qdmap;
vsEventQueue *		queue;		/* shorthand to a struct in qdmap */
vsBox *			mbox;		/* shorthand to a struct in qdmap */
vsCursor *		mouse;		/* shorthand to a struct in qdmap */

DevicePtr		qdKeyboard;
DevicePtr		qdPointer;
int			lastEventTime;

static int		qLimit;		/* last slot in the event queue */

static int		InitialClickVolume = 20;

extern int		screenIsSaved;	/* written to by DIX! */
extern int		Vaxstar; /* set in libtl/tlinit.c */

#define	NDEPTHS	1	/* one-plane required, and Nplanes natural */

static Bool
commandLineMatch( argc, argv, pat, pmatch)
    int         argc;		/* may NOT be changed */
    char *      argv[];		/* may NOT be changed */
    char *	pat;
{
    int		ic;

    for ( ic=0; ic<argc; ic++)
	if ( strcmp( argv[ic], pat) == 0)
	    return TRUE;
    return FALSE;
}

static Bool
commandLinePairMatch( argc, argv, pat, pmatch)
    int         argc;		/* may NOT be changed */
    char *      argv[];		/* may NOT be changed */
    char *	pat;
    char **	pmatch;		/* RETURN */
{
    int		ic;

    for ( ic=0; ic<argc; ic++)
	if ( strcmp( argv[ic], pat) == 0)
	{
	    *pmatch = argv[ ic+1];
	    return TRUE;
	}
    return FALSE;
}

static
colorNameToColor( pname, pred, pgreen, pblue)
    char *	pname;
    u_int *	pred;
    u_int *	pgreen;
    u_int *	pblue;
{
    if ( *pname == '#')
    {
	pname++;		/* skip over # */
	sscanf( pname, "%2x", pred);
	*pred <<= 8;

	pname += 2;
	sscanf( pname, "%2x", pgreen);
	*pgreen <<= 8;

	pname += 2;
	sscanf( pname, "%2x", pblue);
	*pblue <<= 8;
    }
    else /* named color */
    {
	*pred = *pgreen = *pblue = 0; /*OsLookupColor thinks these are shorts*/
	OsLookupColor( 0 /*"screen", not used*/, pname, strlen( pname),
		pred, pgreen, pblue);
    }
}

Bool
qdScreenInit( index, pScr, argc, argv)
    int		index;
    register ScreenPtr pScr;
    int		argc;		/* these two may NOT be changed */
    char *	argv[];
{
    VisualPtr   	pVisWritable;
    VisualPtr   	pVisReadonly;
    ColormapPtr		pdefcmap;	/* returned by CreateColormap */
    DepthPtr    	pDepth;
    struct qdinput *	pQdinput;
    char *		pblackname;
    char *		pwhitename;
    char *		pdragonpix;	/* max size offscreen pixmap */
    int			i;

    extern ColormapPtr	pInstalledMap;	/* lives in qdcolor.c */
    extern int		defaultBackingStore;

    /*
     * defaults: may be overridden by command line
     * should allow named colors		XXX
     * should also look at Xdefaults?		XX
     */
    u_int blackred	= 0x0000;
    u_int blackgreen	= 0x0000;
    u_int blackblue	= 0x0000;

    u_int whitered	= 0xffff;
    u_int whitegreen	= 0xffff;
    u_int whiteblue	= 0xffff;

    if ( commandLinePairMatch( argc, argv, "-bp", &pblackname))
	colorNameToColor( pblackname, &blackred, &blackgreen, &blackblue);
    if ( commandLinePairMatch( argc, argv, "-wp", &pwhitename))
	colorNameToColor( pwhitename, &whitered, &whitegreen, &whiteblue);

    if ( commandLineMatch( argc, argv, "-C")) /* conform to protocol */
    {
	ErrorF( "Xqdss: using slow, protocol-conforming polygon code");
	slowPolygons();
    }

    /* setup max y size of offscreen pixmaps */
    if ( commandLinePairMatch( argc, argv, "-dp", &pdragonpix))
    {
	sscanf( pdragonpix, "%d", &DragonPix);
	if (DragonPix < 0)
	    DragonPix = 0;
	if (DragonPix > 2048-864)
	    DragonPix = 2048-864;	/* full screen - visible screen */
    }
    /*
     * turn off console output, which would hose DMA work in progress.
     * is this sufficient?	XX
     */
    ioctl( fd_qdss, QD_KERN_LOOP);

    /*
     * set keyclick, mouse acceleration and threshold
     */
    {
    char *	clickvolume;
    char *	mouseAcceleration;
    int		ma = 4;
    char *	mouseThreshold;
    int		mt = 4;
    PtrCtrl	ctrl;

    if ( commandLinePairMatch( argc, argv, "c", &clickvolume))
	sscanf( clickvolume, "%d", &InitialClickVolume);
    if ( commandLineMatch( argc, argv, "-c"))
	InitialClickVolume = 0;

    /*
     * calling qdChangePointerControl here may be unclean	XX
     */
    if ( commandLinePairMatch( argc, argv, "-a", &mouseAcceleration))
	sscanf( mouseAcceleration, "%d", &ma);
    if ( commandLinePairMatch( argc, argv, "-t", &mouseThreshold))
	sscanf( mouseThreshold, "%d", &mt);
    ctrl.num = ma;
    ctrl.den = 1;
    ctrl.threshold = mt;
    qdChangePointerControl( (DevicePtr) NULL, &ctrl);
    }

    /*
     * now get the address (in system space) of the event queue and
     * associated headers.
     */
    if ( ioctl( fd_qdss, QD_MAPEVENT, &pQdinput) == -1)
    {
	ErrorF( "qdScreenInit:  QD_MAPEVENT ioctl failed\n");
	return FALSE;
    }

    mouse = (vsCursor *) &pQdinput->curs_pos;
    mbox = (vsBox *) &pQdinput->curs_box;
    queue = (vsEventQueue *) &pQdinput->header;
    qLimit = queue->size - 1;


    pScr->myNum = index;
    pScr->width = 1024;
    pScr->height = 864;
    pScr->mmWidth = (pScr->width * 254) / (75 * 10);	/* ~75 dpi */
    pScr->mmHeight = (pScr->height * 254) / (75 * 10);	/* ~75 dpi */
    pScr->numDepths = NDEPTHS;
    pScr->allowedDepths = pDepth =
				(DepthPtr) Xalloc( NDEPTHS * sizeof(DepthRec));
    pScr->rootDepth = Nplanes;
    pScr->defColormap = FakeClientID(0);	/* resource ID */
    pScr->minInstalledCmaps = pScr->maxInstalledCmaps = 1; /* see protocol
				document under InstallColormap request */
    pScr->backingStoreSupport = Always;		/* is this lying? */
    pScr->saveUnderSupport = NotUseful;
    /*
     * GCperDepth and devPrivate initialized below
     */
    pScr->numVisuals = 2;
    pScr->visuals = (VisualPtr) Xalloc( pScr->numVisuals * sizeof(VisualRec));
    pVisWritable = &pScr->visuals[0];
    pVisReadonly = &pScr->visuals[1];

    /*
     * there is no pixmap corresponding to the root window
     */
    pScr->devPrivate = (pointer) NULL;

    pScr->CreateGC = qdCreateGC;

    pScr->CloseScreen= qdScreenClose;
    pScr->QueryBestSize = qdQueryBestSize;
    pScr->SaveScreen = qdSaveScreen;
    pScr->GetImage = qdGetImage;
    pScr->PointerNonInterestBox = qdPointerNonInterestBox;

    pScr->GetSpans = qdGetSpans;

    pScr->CreateWindow = qdCreateWindow;
    pScr->DestroyWindow = qdDestroyWindow;
    pScr->PositionWindow = qdPositionWindow;
    pScr->ChangeWindowAttributes = qdChangeWindowAttributes;
    pScr->RealizeWindow = qdMapWindow;
    pScr->UnrealizeWindow = qdUnmapWindow;
    pScr->ValidateTree = miValidateTree;
    pScr->WindowExposures = miWindowExposures;

    pScr->CreatePixmap = qdCreatePixmap;
    pScr->DestroyPixmap = qdDestroyPixmap;

    pScr->RealizeFont = qdRealizeFont;
    pScr->UnrealizeFont = qdUnrealizeFont;

    pScr->ConstrainCursor = qdConstrainCursor;
    pScr->CursorLimits = qdCursorLimits;
    pScr->DisplayCursor = qdDisplayCursor;
    pScr->RealizeCursor = qdRealizeCursor;
    pScr->UnrealizeCursor = qdUnrealizeCursor;
    pScr->RecolorCursor = miRecolorCursor;
    pScr->SetCursorPosition = qdSetCursorPosition;

    pScr->CreateColormap = qdCreateColormap;
    pScr->DestroyColormap = qdDestroyColormap;
    pScr->InstallColormap = qdInstallColormap;
    pScr->UninstallColormap = qdUninstallColormap;
    pScr->ListInstalledColormaps = qdListInstalledColormaps;
    pScr->StoreColors = qdStoreColors;
    pScr->ResolveColor = qdResolveColor;

    pScr->RegionCreate = miRegionCreate;	/* only mi from here down */
    pScr->RegionCopy = miRegionCopy;
    pScr->RegionDestroy = miRegionDestroy;
    pScr->Intersect = miIntersect;
    pScr->Union = miUnion;
    pScr->Subtract = miSubtract;
    pScr->Inverse = miInverse;
    pScr->RegionReset = miRegionReset;
    pScr->TranslateRegion = miTranslateRegion;
    pScr->RectIn = miRectIn;
    pScr->PointInRegion = miPointInRegion;
    pScr->RegionNotEmpty = miRegionNotEmpty;
    pScr->RegionEmpty = miRegionEmpty;
    pScr->RegionExtents = miRegionExtents;
    pScr->SendGraphicsExpose = miSendGraphicsExpose;
    pScr->BlockHandler = qdBlockHandler;
    pScr->WakeupHandler = NoopDDA;

    pVisWritable->vid = FakeClientID(0);  /* Visual associated with
					    root window */
    pVisWritable->screen = index;
#if NPLANES==24
    pVisWritable->class = DirectColor;	/* from X.h */
    pVisWritable->redMask = 0x0000ff;
    pVisWritable->greenMask = 0x00ff00;
    pVisWritable->blueMask = 0xff0000;
    pVisWritable->offsetRed = 0;	/* offset within pixel value */
    pVisWritable->offsetGreen = 8;
    pVisWritable->offsetBlue = 16;
#else
    pVisWritable->class = PseudoColor;	/* from X.h */
    pVisWritable->redMask = 0x0;	/* these should never be looked at */
    pVisWritable->greenMask = 0x0;
    pVisWritable->blueMask = 0x0;
    pVisWritable->offsetRed = 0;	/* offset within pixel value */
    pVisWritable->offsetGreen = 0;
    pVisWritable->offsetBlue = 0;
#endif
    if (Nplanes == 4)
	pVisWritable->bitsPerRGBValue = 4;	/* for putimage to work */
    else
	pVisWritable->bitsPerRGBValue = 8;
    if (Vaxstar)
    	pVisWritable->ColormapEntries = ((Nplanes == 4) ? 16 : 256 );
    else
    	pVisWritable->ColormapEntries = ((Nplanes == 4) ? 14 : 254 );
	/* avoid 254 and 255 (cursor), but include 0 and 1 */
    pVisWritable->nplanes = (Nentries == 4 ? 4 : 8);  /* APPROXIMATELY 
						    log2( ColormapEntries ) */
    /*
     * also create a read-only Visual
     */
    *pVisReadonly = *pVisWritable;
    pVisReadonly->vid = FakeClientID(0);
#if NPLANES==24
    pVisReadonly->class = TrueColor;	/* from X.h */
#else	/* NPLANES==8 or 4 */
    pVisReadonly->class = StaticColor;	/* from X.h */
#endif

#define NOMAPYET      (ColormapPtr) 1
    pInstalledMap = NOMAPYET;

    if ( CreateColormap( pScr->defColormap, pScr,
#if NPLANES==24
	pVisReadonly,
#else   /* NPLANES==8 or 4 */
	pVisWritable,
#endif
	&pdefcmap, AllocNone, 0 /*client*/) != Success)
    {
        ErrorF( "qdScreenInit: CreateColormap of %s failed\n",
			"pVisWritable");
	return FALSE;
    }

    /*
     * If requested pixel values are honored, whitePixel and blackPixel
     * will be inverses of each other.  More importantly, they will live
     * at nearly opposite ends of the color map.  This means that if a
     * read-only colormap is installed, e.g. StaticColor, the root window
     * will still look right.
     */
#if NPLANES==24
    pScr->blackPixel = 0xfdfdfd;
    pScr->whitePixel = 0xfcfcfc;
#else /* NPLANES==8 or 4 */
    pScr->blackPixel = (Nplanes == 4 ? 0x0f : 0xfd);
    pScr->whitePixel = (Nplanes == 4 ? 0x0e : 0xfc);
#endif
    if ( AllocColor( pdefcmap,
	(u_short *)&blackred, (u_short *)&blackgreen, (u_short *)&blackblue,
					    &pScr->blackPixel, 0) != Success
      || AllocColor( pdefcmap,
	(u_short *)&whitered, (u_short *)&whitegreen, (u_short *)&whiteblue,
					    &pScr->whitePixel, 0) != Success)
    {
	ErrorF( "qdScreenInit: AllocColor failed\n");
	return FALSE;
    }

    qdInstallColormap( pdefcmap);

    /*
     * support for depth NPLANES pixmaps and windows
     *
     * There is a potential problem with the base server data structures here;
     * the second visual lives in the same array as the first but is a
     * separate resource.  If the first resource is freed, the second visual
     * points to freed storage.  Actually, this probably never happens.   XX
     */
    pDepth[0].depth = Nplanes;
    pDepth[0].numVids = 2;
    pDepth[0].vids = (VisualID *) Xalloc( pDepth[0].numVids * sizeof(long));
 
    pDepth[0].vids[0] = pScr->visuals[0].vid;
    AddResource( pScr->visuals[0].vid, RT_VISUALID,
	    &pScr->visuals[0],
	    qdFreeResource,
	    RC_CORE);
    pDepth[0].vids[1] = pScr->visuals[1].vid;
    AddResource( pScr->visuals[1].vid, RT_VISUALID,
	    &pScr->visuals[1],
	    NoopDDA,  /* visuals are stored in a single Xalloc'ed array */
	    RC_CORE);
#if NPLANES==24
    pScr->rootVisual = pVisReadonly->vid;
#else /* NPLANES==8 or 4 */
    pScr->rootVisual = pVisWritable->vid;
#endif
    return TRUE;
}

Bool
qdScreenClose( index, pScr)
    int index;		/* NOT USED */
    ScreenPtr pScr;	/* NOT USED */
{
    int	errno;

    if ( (errno=tlCleanup()) != 0)
    {
	ErrorF("Closing QDSS yielded %d\n", errno);
	return (FALSE);
    }
    return (TRUE);
}

extern struct qdmap	Qdss;

static Bool
qdSaveScreen(pScr, on)
    ScreenPtr pScr;   /* NOT USED */
    Bool on;
{
    if (on != SCREEN_SAVER_ON)
    {
        lastEventTime = GetTimeInMillis();
	if (!Vaxstar)
	    *(short *) Qdss.memcsr = UNBLANK | SYNC_ON; 
	else
	    ioctl(fd_qdss, SG_VIDEOON, &Sg);
    }
    else
    {
	if (!Vaxstar)
	    *(short *) Qdss.memcsr = SYNC_ON; 
	else
	    ioctl(fd_qdss, SG_VIDEOOFF, &Sg);
    }
    return TRUE;
}


void
qdFreeResource( p, id)
    pointer p;
    int id;
{
    Xfree(p);
}

int
qdMouseProc( pDev, onoff, argc, argv)
    DevicePtr pDev;
    int onoff;
    int argc;
    char *argv[];
{
    BYTE map[4];

    switch (onoff)
    {
	case DEVICE_INIT: 
	    qdPointer = pDev;
	    pDev->devicePrivate = (pointer) &queue;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(
		qdPointer, map, 3, qdGetMotionEvents, qdChangePointerControl);
	    SetInputCheck(&queue->head, &queue->tail);
	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    AddEnabledDevice( fd_qdss);
	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
/*	RemoveEnabledDevice( fd_qdss);   */
	    break;
	case DEVICE_CLOSE: 
	    break;
    }
    return Success;

}

int
qdKeybdProc( pDev, onoff, argc, argv)
    DevicePtr pDev;
    int onoff;
    int argc;
    char *argv[];
{
    KeySymsRec keySyms;
    CARD8 modMap[MAP_LENGTH];

    switch (onoff)
    {
	case DEVICE_INIT: 
	    qdKeyboard = pDev;
	    pDev->devicePrivate = (pointer) & queue;
            GetLK201Mappings( &keySyms, modMap);
	    InitKeyboardDeviceStruct(
		    qdKeyboard, &keySyms, modMap, qdBell,
		    qdChangeKeyboardControl);
            QDClick( pDev, InitialClickVolume);

	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    AddEnabledDevice( fd_qdss);
	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
/*	    RemoveEnabledDevice( fd_qdss);  */
	    break;
	case DEVICE_CLOSE: 
	    break;
    }
    return Success;
}


/*
 * processes all the pending input events
 */
void
ProcessInputEvents()
{
#define DEVICE_KEYBOARD 2
    register int    i;
    register    vsEvent * pE;
    xEvent	x;
    int     nowInCentiSecs, nowInMilliSecs, adjustCentiSecs;
    struct timeval  tp;
    int     needTime = 1;
    extern CursorRec CurrentCurs;	/* defined in qdcursor.c */
    extern CursorRec LastAlignCurs;	/* defined in qdcursor.c */

    i = queue->head;
    while (i != queue->tail)
    {
	if (screenIsSaved == SCREEN_SAVER_ON)
	    SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
	pE = &queue->events[i];
	x.u.keyButtonPointer.rootX = pE->vse_x + LastAlignCurs.xhot;
	x.u.keyButtonPointer.rootY = pE->vse_y + LastAlignCurs.yhot;
	if (sizeof(pE->vse_time) == 4)
	    x.u.keyButtonPointer.time = lastEventTime = pE->vse_time;
	else {
	    /* 
	     * The following silly looking code is because the old version of the
	     * driver only delivers 16 bits worth of centiseconds. We are supposed
	     * to be keeping time in terms of 32 bits of milliseconds.
	     */
	    if (needTime)
	    {
		needTime = 0;
		gettimeofday(&tp, 0);
		nowInCentiSecs = ((tp.tv_sec * 100) + (tp.tv_usec / 10000)) & 0xFFFF;
	    /* same as driver */
		nowInMilliSecs = (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
	    /* beware overflow */
	    }
	    if ((adjustCentiSecs = nowInCentiSecs - pE->vse_time) < -20000)
		adjustCentiSecs += 0x10000;
	    else
		if (adjustCentiSecs > 20000)
		    adjustCentiSecs -= 0x10000;
	    x.u.keyButtonPointer.time = lastEventTime =
		nowInMilliSecs - adjustCentiSecs * 10;
	}

	if ((pE->vse_type == VSE_BUTTON) &&
	    (pE->vse_device == DEVICE_KEYBOARD))
	{					/* better be a button */
	    x.u.u.detail = pE->vse_key;
	    switch (pE->vse_direction)
	    {
		case VSE_KBTDOWN: 
		    x.u.u.type = KeyPress;
		    (qdKeyboard->processInputProc)(&x, qdKeyboard);
		    break;
		case VSE_KBTUP: 
		    x.u.u.type = KeyRelease;
		    (qdKeyboard->processInputProc)(&x, qdKeyboard);
		    break;
		default: 	       /* hopefully BUTTON_RAW_TYPE */
		    ProcessLK201Input(&x, qdKeyboard);
	    }
	}
	else
	{
	    if (pE->vse_type == VSE_BUTTON)
	    {
		if (pE->vse_direction == VSE_KBTDOWN)
		    x.u.u.type = ButtonPress;
		else
		    x.u.u.type = ButtonRelease;
		/* mouse buttons numbered from one */
		x.u.u.detail = pE->vse_key + 1;
	    }
	    else {
#ifndef NO_EVENT_COMPRESSION
		int j = (i == qLimit) ? 0 : i + 1;
		/*
		 * to get here we knew that 
		 *
		 *     (vse_type != VSE_BUTTON || 
		 *      vse_device != DEVICE_KEYBOARD) && 
		 *     (vse_type != VSE_BUTTON)
		 *
		 * which means that for the next event to be a mouse
		 * motion, it must satisfy vse_type != VSE_BUTTON
		 *
		 * XXX -- We should implement motion history since we are 
		 * throwing device events away....
		 */
		if (j != queue->tail &&
		    queue->events[j].vse_type != VSE_BUTTON)
		  goto next;		/* sometimes the dragon wins */

#endif
		/* tell the server that the mouse moved */
		x.u.u.type = MotionNotify;
	    }
	    (* qdPointer->processInputProc)(&x, qdPointer);
	}

      next:
	if (i == qLimit)
	    i = queue->head = 0;
	else
	    i = ++queue->head;
    }
    dmafxns.flush ( FALSE);	/* necessary for interaction. due to grabs? */
    /*
     * The intent here is to align the cursor at the last possible moment.
     * It cannot be aligned earlier because the warps cannot keep up
     * with the cursor motion hardware.
     */
    QDAlignCursor( pE->vse_x, pE->vse_y);
#undef DEVICE_KEYBOARD
}

TimeSinceLastInputEvent()
{
    if (lastEventTime == 0)
	lastEventTime = GetTimeInMillis();
    return GetTimeInMillis() - lastEventTime;
}

static void
qdChangePointerControl(pDevice, ctrl)
    DevicePtr pDevice;
    PtrCtrl   *ctrl;
{
    struct prg_cursor	curs;

    curs.threshold = ctrl->threshold;
    if ( (curs.acc_factor = ctrl->num / ctrl->den) == 0)
	curs.acc_factor = 1;	/* watch for den > num */
    ioctl( fd_qdss, QD_PRGCURSOR, &curs);
}

int
qdGetMotionEvents( pDevice, buff, start, stop)
    DevicePtr	pDevice;
    CARD32	start, stop;
    xTimecoord *buff;
{
    return 0;
}

#define MAX_LED 3  /* only 3 LED's can be set by user; Lock LED is controlled by server */
static void
ChangeLED( led, on)
    int		led;
    Bool	on;
{
    struct prgkbd ioc;

    switch (led) {
       case 1:
	  ioc.param1 = LED_1;
	  break;
       case 2:
          ioc.param1 = LED_2;
	  break;
       case 3:
          /* the keyboard's LED_3 is the Lock LED, which the server owns.
             So the user's LED #3 maps to the keyboard's LED_4. */
          ioc.param1 = LED_4;
	  break;
       default:
	  return;   /* out-of-range LED value */
	  }
    ioc.cmd = on ? LK_LED_ENABLE : LK_LED_DISABLE;
    ioc.param2  = 0;
    ioctl(fd_qdss, QD_PRGKBD, &ioc);
}

SetLockLED (on)
    Bool on;
    {
    struct prgkbd ioc;
    ioc.cmd = on ? LK_LED_ENABLE : LK_LED_DISABLE;
    ioc.param1 = LED_3;
    ioc.param2 = 0;
    ioctl(fd_qdss, QD_PRGKBD, &ioc);
    }

static void
qdChangeKeyboardControl(device, ctrl)
    DevicePtr	device;
    KeybdCtrl *	ctrl;
{
    int i;

    QDClick( device, ctrl->click);

    /* ctrl->bell: the DIX layer handles the base volume for the bell */
    
    /* ctrl->bell_pitch: as far as I can tell, you can't set this on lk201 */

    /* ctrl->bell_duration: as far as I can tell, you can't set this  */

    /* LEDs */
    for (i=1; i<=MAX_LED; i++)
        ChangeLED(i, (ctrl->leds & (1 << (i-1))));

    /* ctrl->autoRepeat: I'm turning it all on or all off.  */

    SetLKAutoRepeat(ctrl->autoRepeat);
}

static
QDClick( device, click)
    DevicePtr   device;
    int		click;
{
#define LK_ENABLE_CLICK 0x1b	/* enable keyclick / set volume	*/
#define LK_DISABLE_CLICK 0x99	/* disable keyclick entirely	*/
#define LK_ENABLE_BELL 0x23	/* enable bell / set volume 	*/

    struct prgkbd ioc;

    if (click == 0)    /* turn click off */
    {
	ioc.cmd = LK_DISABLE_CLICK;
	ioc.param1 = 0;
    }
    else 
    {
        int volume;

	volume = 7 - ((click / 14) & 0x7);
	ioc.cmd = LK_ENABLE_CLICK;
	ioc.param1 = volume | LAST_PARAM;
    }
    ioc.param2 = 0;
    ioctl(fd_qdss, QD_PRGKBD, &ioc);

}

static void
qdBell( loud, pDevice)
    int		loud;
    DevicePtr	pDevice;
{
    struct prgkbd ioc;

    /*
     * the lk201 volume is between 7 (quiet but audible) and 0 (loud)
     */
    loud = 7 - ((loud / 14) & 7);

    ioc.cmd = LK_ENABLE_BELL;
    ioc.param1 = loud | LAST_PARAM;
    ioc.param2 = 0;

    ioctl( fd_qdss, QD_PRGKBD, &ioc);
    ioc.cmd = LK_RING_BELL | LAST_PARAM;
    ioc.param1 = 0;
    ioctl( fd_qdss, QD_PRGKBD, &ioc);
}

#define LK_REPEAT_ON  0xe3
#define LK_REPEAT_OFF 0xe1

int
SetLKAutoRepeat (onoff)
    Bool onoff;
{
    extern char *AutoRepeatLKMode();
    extern char *UpDownLKMode();
    
    struct prgkbd	ioc;
    register char *	divsets;

    ioc.param1 = 0;
    divsets = onoff ? (char *) AutoRepeatLKMode() : (char *) UpDownLKMode();
    while (ioc.cmd = *divsets++)
	ioctl(fd_qdss, QD_PRGKBD, &ioc);
    ioc.cmd = ((onoff > 0) ? LK_REPEAT_ON : LK_REPEAT_OFF);
    return( ioctl( fd_qdss, QD_PRGKBD, &ioc));
}

static void
qdBlockHandler( iscr, data)
    int		iscr;
    pointer	data;
{
    dmafxns.flush ( FALSE);
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX()
{
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
}

int
ddxProcessArgument (argc, argv, i)
    int	argc;
    char *argv[];
    int	i;
{
    return 0;
}

void
ddxUseMsg()
{
    ErrorF("-bp<:screen> color     BlackPixel for screen\n");
    ErrorF("-wp<:screen> color     WhitePixel for screen\n");
}
