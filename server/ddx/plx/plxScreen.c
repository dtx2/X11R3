/*
 *   Copyright (c) 1987, 88 by
 *   PARALLAX GRAPHICS, INCORPORATED, Santa Clara, California.
 *   All rights reserved
 *
 *   This software is furnished on an as-is basis, and may be used and copied
 *   only with the inclusion of the above copyright notice.
 *
 *   The information in this software is subject to change without notice.
 *   No committment is made as to the usability or reliability of this
 *   software.
 *
 *   Parallax Graphics, Inc.
 *   2500 Condensa Street
 *   Santa Clara, California  95051
 */

#ifndef lint
static char *sid_ = "@(#)plxScreen.c	1.32 08/31/88 Parallax Graphics Inc";
#endif

#define	PARALLAX_QEVENT
#include	"Xplx.h"
extern int plx_wfd;

#define  NEED_EVENTS
#include "Xproto.h"
#include "cursorstr.h"
#include "input.h"
#include "colormapst.h"

#include "mi.h"

extern void miRecolorCursor();
extern RegionPtr miRegionCreate();

extern void px_flush();

vsEventQueue *queue;
vsBox *mbox;
vsCursor *mouse;

int lastEventTime;
DevicePtr plxKeyboard;
DevicePtr plxPointer;

/*
 * vid screen class     rMask gMask bMask oRed oGreen oBlue bpRGB cmpE nplane
 */
VisualRec plxvisuals[] = {
 0,  0,     PseudoColor,0,    0,    0,    0,   0,     0,    8,    192, 8,
#ifdef notdef
 0,  0,     StaticColor,0,    0,    0,    0,   0,     0,    8,    192, 8,
#endif
};

/*
	depth,	numvis,	vids
*/
DepthRec plxdepths[] = {
	1,	0, 	NULL,
	8,	1,	NULL,			/* last one is ROOTVISUAL */
};

#define	Nplxvisuals	(sizeof(plxvisuals)/sizeof(plxvisuals[0]))
#define	Nplxdepths	(sizeof(plxdepths)/sizeof(plxdepths[0]))

extern ColormapPtr plxcurrentmap[];

#define	ROOTVISUAL	(Nplxvisuals - 1)

Bool
plxScreenInit(idx, pScreen, argc, argv)
ScreenPtr pScreen;
char *argv[];
{
	register int i;
	char *display = "", *microcodename = "";
	char *displayconfig = "";
	int bswapflag = -1, sbit15flag = -1;

	ifdebug(5) printf("plxScreenInit(), index=%d\n", idx);

	for (i=0;i<argc;i++) {
		if (strcmp(argv[i], "-display") == 0)
			display = argv[++i];
		else
		if (strcmp(argv[i], "-displayconfig") == 0)
			displayconfig = argv[++i];
		else
		if (strcmp(argv[i], "-microcode") == 0)
			microcodename = argv[++i];
		else
		if (strcmp(argv[i], "-bswap") == 0)
			bswapflag = atoi(argv[++i]);
		else
		if (strcmp(argv[i], "-sbit0") == 0)
			sbit15flag = 0;
		else
		if (strcmp(argv[i], "-sbit15") == 0)
			sbit15flag = 1;
	}

	plxboardinit(display, displayconfig, microcodename, bswapflag, sbit15flag);

	pl_cache_init();
	pl_cache_add(CACHE_1_X, CACHE_1_Y, CACHE_1_X + CACHE_1_WIDTH - 1, CACHE_1_Y + CACHE_1_HEIGHT - 1);
	pl_cache_add(CACHE_2_X, CACHE_2_Y, CACHE_2_X + CACHE_2_WIDTH - 1, CACHE_2_Y + CACHE_2_HEIGHT - 1);

	plxfontmeminit();

	plxclipinit(pScreen);

	px_get_mmq(&mouse, &mbox, &queue);

	pScreen->myNum = idx;
	pScreen->width = 1280;
	pScreen->height = 1024;
	pScreen->mmWidth =  350;
	pScreen->mmHeight = 260;
	pScreen->numDepths = Nplxdepths;
	pScreen->allowedDepths = plxdepths;

	pScreen->rootDepth = 8;
	pScreen->minInstalledCmaps = 1;
	pScreen->maxInstalledCmaps = 1;
#ifdef X11R2
	pScreen->backingStoreSupport = NotUseful;
#else
	pScreen->backingStoreSupport = Always;
#endif /* X11R2 */
	pScreen->saveUnderSupport = NotUseful;

	pScreen->numVisuals = Nplxvisuals;
	pScreen->visuals = plxvisuals;

	pScreen->QueryBestSize = plxQueryBestSize;

	pScreen->CreateWindow = plxCreateWindow;
	pScreen->ChangeWindowAttributes = plxChangeWindowAttributes;
	pScreen->DestroyWindow = plxDestroyWindow;
	pScreen->PositionWindow = plxPositionWindow;
	pScreen->RealizeWindow = plxRealizeWindow;
	pScreen->UnrealizeWindow = plxUnrealizeWindow;

	pScreen->RealizeFont = plxRealizeFont;
	pScreen->UnrealizeFont = plxUnrealizeFont;

	pScreen->GetImage = plxGetImage;
	pScreen->GetSpans = plxGetSpans;

	pScreen->CreateGC = plxCreateGC;
	pScreen->CreatePixmap = plxCreatePixmap;
	pScreen->DestroyPixmap = plxDestroyPixmap;

	pScreen->ValidateTree = miValidateTree;

	pScreen->RegionCreate = miRegionCreate;		/* XXX */
	pScreen->RegionDestroy = miRegionDestroy;
	pScreen->RegionCopy = miRegionCopy;
	pScreen->RegionReset = miRegionReset;
	pScreen->TranslateRegion = miTranslateRegion;

	pScreen->RegionEmpty = miRegionEmpty;
	pScreen->RegionNotEmpty = miRegionNotEmpty;
	pScreen->RegionExtents = miRegionExtents;
	pScreen->PointInRegion = miPointInRegion;
	pScreen->RectIn = miRectIn;

	pScreen->Intersect = miIntersect;
	pScreen->Inverse = miInverse;
	pScreen->Subtract = miSubtract;
	pScreen->Union = miUnion;
#ifndef X11R2
	pScreen->SendGraphicsExpose = miSendGraphicsExpose;
#endif

	pScreen->WindowExposures = miWindowExposures;

	pScreen->BlockHandler = px_flush;
	pScreen->WakeupHandler = NoopDDA;
	pScreen->blockData = (pointer)0;
	pScreen->wakeupData = (pointer)0;

	pScreen->CloseScreen = plxCloseScreen;
	pScreen->SaveScreen = plxSaveScreen;

	pScreen->CreateColormap = plxCreateColormap;
	pScreen->DestroyColormap = plxDestroyColormap;
	pScreen->ListInstalledColormaps = plxListInstalledColormaps;
	pScreen->InstallColormap = plxInstallColormap;
	pScreen->UninstallColormap = plxUninstallColormap;
	pScreen->StoreColors = plxStoreColors;
	pScreen->ResolveColor = plxResolveColor;

	pScreen->DisplayCursor = plxDisplayCursor;
	pScreen->RealizeCursor = plxRealizeCursor;
	pScreen->UnrealizeCursor = plxUnrealizeCursor;
	pScreen->RecolorCursor = miRecolorCursor;
	pScreen->ConstrainCursor = plxConstrainCursor;
	pScreen->CursorLimits = plxCursorLimits;
	pScreen->SetCursorPosition = plxSetCursorPosition;
	pScreen->PointerNonInterestBox = plxPointerNonInterestBox;

	for (i=0;i<Nplxvisuals;i++) {
		ColormapPtr plxcolormap;

		plxvisuals[i].vid = FakeClientID(0);
		plxvisuals[i].screen = idx;
		AddResource(plxvisuals[i].vid,
			RT_VISUALID, &plxvisuals[i], NoopDDA, RC_CORE);

		switch (plxvisuals[i].class) {
		case StaticColor:
		case StaticGray:
			CreateColormap(FakeClientID(0), pScreen, &plxvisuals[i], &plxcolormap, AllocAll, 0);
			break;
		case PseudoColor:
		case GrayScale:
			CreateColormap(FakeClientID(0), pScreen, &plxvisuals[i], &plxcolormap, AllocNone, 0);
			break;
		case TrueColor:
		case DirectColor:
			FatalError("Bad visual in plxScreenInit\n");
		}
		if (!plxcolormap) {
			FatalError("plxScreenInit: Can't create colormap\n");
		}
		if (i == ROOTVISUAL) {
			pScreen->defColormap = plxcolormap->mid;
			pScreen->rootVisual = plxvisuals[i].vid;
		}
	}

	for (i=0;i<Nplxdepths;i++) {
		if (plxdepths[i].numVids > 0) {
			register unsigned long *pVids;

			plxdepths[i].vids = pVids = (unsigned long *)Xalloc(sizeof(unsigned long) * plxdepths[i].numVids);
			/* XXX - here we offer only the 8-bit visual */
			pVids[0] = plxvisuals[ROOTVISUAL].vid;
		}
	}

	{
    		CARD16 zero = 0, ones = ~0;
		ColormapPtr pColormap = (ColormapPtr)LookupID(pScreen->defColormap, RT_COLORMAP, RC_CORE);

		if (!pColormap)
			FatalError("plxScreenInit: Can't find default colormap\n");

		/*
		 * prevent reinitialization errors
		 */
		plxcurrentmap[pScreen->myNum] = (ColormapPtr)NULL;
		plxInstallColormap(pColormap);
		if (AllocColor(pColormap, &zero, &zero, &zero, &(pScreen->blackPixel), 0))
			FatalError("plxScreenInit: Can't allocate black pixel\n");
		if (AllocColor(pColormap, &ones, &ones, &ones, &(pScreen->whitePixel), 0))
			FatalError("plxScreenInit: Can't allocate white pixel\n");
	}

	/*
	 * any better place to do this?
	 */
	plxVideoInit(pScreen);

	return TRUE;
}

static Bool
plxSaveScreen(pScreen, on)
ScreenPtr pScreen;
{
	if (on == SCREEN_SAVER_FORCER) {
		lastEventTime = GetTimeInMillis();
		return TRUE;
	} else {
		return FALSE;
	}
}

static Bool
plxCloseScreen(pScreen)
ScreenPtr pScreen;
{
	ifdebug(5) printf("plxCloseScreen()\n");

	pl_cursor_active(0);
	p_floff();
	px_flush();
	px_close();
	return (TRUE);
}
