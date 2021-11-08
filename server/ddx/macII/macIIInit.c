/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
/*-
 * macIIInit.c --
 *	Initialization functions for screen/keyboard/mouse, etc.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDyNG ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include    "macII.h"
#include    <servermd.h>
#include    "dixstruct.h"
#include    "dix.h"
#include    "opaque.h"

extern int macIIMouseProc();
extern void macIIKbdProc();
extern int macIIKbdSetUp();
extern Bool macIIMonoProbe();
extern void ProcessInputEvents();

extern void SetInputCheck();
extern char *strncpy();
extern GCPtr CreateScratchGC();

int macIISigIO = 0;	 /* For use with SetInputCheck */
static int autoRepeatHandlersInstalled; /* FALSE each time InitOutput called */

	/* What should this *really* be? */
#define MOTION_BUFFER_SIZE 0

/*-
 *-----------------------------------------------------------------------
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	isItTimeToYield is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
SigIOHandler(sig, code, scp)
    int		code;
    int		sig;
    struct sigcontext *scp;
{
    macIISigIO++;
    isItTimeToYield++;
}

macIIFbDataRec macIIFbData[] = {
    macIIMonoProbe,  	"/dev/console",	    neverProbed,
};

/*
 * NUMSCREENS is the number of supported frame buffers (i.e. the number of
 * structures in macIIFbData which have an actual probeProc).
 */
#define NUMSCREENS (sizeof(macIIFbData)/sizeof(macIIFbData[0]))
#define NUMDEVICES 2

fbFd	macIIFbs[NUMSCREENS];  /* Space for descriptors of open frame buffers */

static PixmapFormatRec	formats[] = {
    1, 1, BITMAP_SCANLINE_PAD,	/* 1-bit deep */
    8, 8, BITMAP_SCANLINE_PAD,	/* 8-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

/*-
 *-----------------------------------------------------------------------
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *	The
 *
 * Results:
 *	screenInfo init proc field set
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */

InitOutput(pScreenInfo, argc, argv)
    ScreenInfo 	  *pScreenInfo;
    int     	  argc;
    char    	  **argv;
{
    int     	  i, index, ac = argc;
    char	  **av = argv;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
    {
        pScreenInfo->formats[i] = formats[i];
    }

    autoRepeatHandlersInstalled = FALSE;

    for (i = 0, index = 0; i < NUMSCREENS; i++) {
	if ((* macIIFbData[i].probeProc) (pScreenInfo, index, i, argc, argv)) {
	    /* This display exists OK */
	    index++;
	} else {
	    /* This display can't be opened */
	    ;
	}
    }
    if (index == 0)
	FatalError("Can't find any displays\n");

    pScreenInfo->numScreens = index;

    macIIInitCursor();
}

/*-
 *-----------------------------------------------------------------------
 * InitInput --
 *	Initialize all supported input devices...what else is there
 *	besides pointer and keyboard?
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Two DeviceRec's are allocated and registered as the system pointer
 *	and keyboard devices.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
{
    DevicePtr p, k;
    static int  zero = 0;
    
    p = AddInputDevice(macIIMouseProc, TRUE);
    k = AddInputDevice(macIIKbdProc, TRUE);

    RegisterPointerDevice(p, MOTION_BUFFER_SIZE);
    RegisterKeyboardDevice(k);

#ifdef notdef
    signal(SIGIO, SigIOHandler);

    SetInputCheck (&zero, &isItTimeToYield);
#endif
}

/*-
 *-----------------------------------------------------------------------
 * macIIQueryBestSize --
 *	Supposed to hint about good sizes for things.
 *
 * Results:
 *	Perhaps change *pwidth (Height irrelevant)
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
void
macIIQueryBestSize(class, pwidth, pheight)
int class;
short *pwidth;
short *pheight;
{
    unsigned width, test;

    switch(class)
    {
      case CursorShape:
      case TileShape:
      case StippleShape:
	  width = *pwidth;
	  if ((int)width > 0) {
	      /* Return the closest power of two not less than what they gave me */
	      test = 0x80000000;
	      /* Find the highest 1 bit in the width given */
	      while(!(test & width))
		 test >>= 1;
	      /* If their number is greater than that, bump up to the next
	       *  power of two */
	      if((test - 1) & width)
		 test <<= 1;
	      *pwidth = test;
	  }
	  /* We don't care what height they use */
	  break;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIScreenInit --
 *	Things which must be done for all types of frame buffers...
 *	Should be called last of all.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The graphics context for the screen is created. The CreateGC,
 *	CreateWindow and ChangeWindowAttributes vectors are changed in
 *	the screen structure.
 *
 *	Both a BlockHandler and a WakeupHandler are installed for the
 *	first screen.  Together, these handlers implement autorepeat
 *	keystrokes.
 *
 *-----------------------------------------------------------------------
 */
void
macIIScreenInit (pScreen)
    ScreenPtr	  pScreen;
{
    fbFd    	  *fb;
    DrawablePtr	  pDrawable;
    extern void   macIIBlockHandler();
    extern void   macIIWakeupHandler();
    static ScreenPtr autoRepeatScreen;

    fb = &macIIFbs[pScreen->myNum];

    /*
     * Prepare the GC for cursor functions on this screen.
     * Do this before setting interceptions to avoid looping when
     * putting down the cursor...
     */
    pDrawable = (DrawablePtr)(pScreen->devPrivate);

    fb->pGC = CreateScratchGC (pDrawable->pScreen, pDrawable->depth);

    /*
     * By setting graphicsExposures false, we prevent any expose events
     * from being generated in the CopyArea requests used by the cursor
     * routines.
     */
    fb->pGC->graphicsExposures = FALSE;

    /*
     * Preserve the "regular" functions
     */
    fb->CreateGC =	    	    	pScreen->CreateGC;
    fb->CreateWindow = 	    	    	pScreen->CreateWindow;
    fb->ChangeWindowAttributes =    	pScreen->ChangeWindowAttributes;
    fb->GetImage =	    	    	pScreen->GetImage;
    fb->GetSpans =			pScreen->GetSpans;

    /*
     * Interceptions
     */
    pScreen->CreateGC =	    	    	macIICreateGC;
    pScreen->CreateWindow = 	    	macIICreateWindow;
    pScreen->ChangeWindowAttributes = 	macIIChangeWindowAttributes;
    pScreen->QueryBestSize =		macIIQueryBestSize;
    pScreen->GetImage =	    	    	macIIGetImage;
    pScreen->GetSpans =			macIIGetSpans;

    /*
     * Cursor functions
     */
    pScreen->RealizeCursor = 	    	macIIRealizeCursor;
    pScreen->UnrealizeCursor =	    	macIIUnrealizeCursor;
    pScreen->DisplayCursor = 	    	macIIDisplayCursor;
    pScreen->SetCursorPosition =    	macIISetCursorPosition;
    pScreen->CursorLimits = 	    	macIICursorLimits;
    pScreen->PointerNonInterestBox = 	macIIPointerNonInterestBox;
    pScreen->ConstrainCursor = 	    	macIIConstrainCursor;
    pScreen->RecolorCursor = 	    	macIIRecolorCursor;

    /*
     *	Block/Unblock handlers
     */
    if (autoRepeatHandlersInstalled == FALSE) {
	autoRepeatScreen = pScreen;
	autoRepeatHandlersInstalled = TRUE;
    }

    if (pScreen == autoRepeatScreen) {
        pScreen->BlockHandler = macIIBlockHandler;
        pScreen->WakeupHandler = macIIWakeupHandler;
    }

}

/*-
 *-----------------------------------------------------------------------
 * macIIOpenFrameBuffer --
 *	Open a frame buffer through the /dev/console interface.
 *
 * Results:
 *	The fd of the framebuffer.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
int
macIIOpenFrameBuffer(expect, pfbType, index, fbNum, argc, argv)
    int	    	  expect;   	/* The expected type of framebuffer */
    fbtype 	  *pfbType; 	/* Place to store the fb info */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  index;    	/* Screen index */
    int	    	  argc;	    	/* Command-line arguments... */
    char	  **argv;   	/* ... */
{
    int           fd = -1;	    	/* Descriptor to device */
    struct strioctl ctl;

    fd = open("/dev/console", O_RDWR, 0);
    if (fd < 0) {
	return (-1);
    } 

    ctl.ic_cmd = VIDEO_DATA;
    ctl.ic_timout = -1;
    ctl.ic_len = sizeof(fbtype);
    ctl.ic_dp = (char *)pfbType;
    if (ioctl(fd, I_STR, &ctl) < 0) {
        FatalError("Failed to ioctl I_STR VIDEO_DATA.\n");
	(void) close(fd);
        return(!Success);
    }

    return (fd);
}

/*-
 *-----------------------------------------------------------------------
 * macIIBlackScreen --
 *	Fill a frame buffer with pixel 1.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
int
macIIBlackScreen(index)
	int index;
{
    fbFd *pf;
    register unsigned char* fb;
    register int fbinc, line, lw;
    register unsigned int *fbt;

    pf = &macIIFbs[index];
    fb = pf->fb; /* Assumed longword aligned! */

    switch (pf->info.v_pixelsize) {
    case 1:
    {
	fbinc = pf->info.v_rowbytes;
        for (line = pf->info.v_top; line < pf->info.v_bottom; line++) {
	    lw = ((pf->info.v_right - pf->info.v_left) + 31) >> 5;
	    fbt = (unsigned int *)fb;
	    do {
		*fbt++ = 0xffffffff;
	    } while (--lw);
	    fb += fbinc;
	}
	break;
    }
    default:
	ErrorF("Bad depth in macIIBlackScreen.");
	break;
    }
}

void
AbortDDX()
{
    extern int consoleFd, devosmFd;

    if (devosmFd > 0) close(devosmFd);
    if (consoleFd > 0) {
	macIIKbdSetUp(consoleFd, FALSE); /* Must NOT FatalError() anywhere! */
        close(consoleFd);
	consoleFd = 0;
    }
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
}
