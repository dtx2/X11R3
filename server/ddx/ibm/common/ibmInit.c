/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/***********************************************************
		Copyright IBM Corporation 1987,1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmInit.c,v 9.0 88/10/17 14:55:16 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmInit.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmInit.c,v 9.0 88/10/17 14:55:16 erik Exp $";
static char sccsid[] = "@(#)ibminit.c	1.1 88/09/13 22:20:58";
#endif


#include <sys/types.h>
#include <sys/file.h>

#include "X.h"
#include "servermd.h"
#include "misc.h"
#include "miscstruct.h"
#include "input.h"
#include "opaque.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "ibmScreen.h"

#include "ibmKeybd.h"
#include "ibmMouse.h"

#include "OSio.h"

#include "ibmTrace.h"

#define MOTION_BUFFER_SIZE 0
#define NUMDEVICES 2

static	int	ibmOpenAllScreens=	FALSE;
static	int	ibmScreensWanted=	0;
char 	*ibmBlackPixelText = "black";
char 	*ibmWhitePixelText = "white";
Bool	ibmDontZap = FALSE;

static	Bool
ibmFormatExists(screenInfo,newFmt)
    ScreenInfo		*screenInfo;
    PixmapFormatPtr	newFmt;
{
    PixmapFormatPtr	oldFmt;
    int			ndx;

    TRACE(("ibmFormatExisits(screenInfo= 0x%x,newFmt=0x%x(%d,%d,%d))\n",
				     screenInfo,newFmt,newFmt->depth,
				     newFmt->bitsPerPixel,newFmt->scanlinePad));
    for (ndx=0;ndx<screenInfo->numPixmapFormats;ndx++) {
	oldFmt= &screenInfo->formats[ndx];
	if ((newFmt->depth==oldFmt->depth)&&
	    (newFmt->bitsPerPixel==oldFmt->bitsPerPixel)&&
	    (newFmt->scanlinePad==oldFmt->scanlinePad)) {
		return TRUE;
	}
    }
    return FALSE;
}

/***==================================================================***/

static	int
ibmFindSomeScreens()
{
register ibmPerScreenInfo **scrPtr = ibmPossibleScreens ;
register ibmPerScreenInfo *scr ;

    TRACE(("ibmFindSomeScreens()\n"));
    OS_GetDefaultScreens();
    if ( !ibmNumScreens ) {
	while ( ( scr = *scrPtr++ ) && scr->ibm_ScreenFlag ) {
	    if ( ( scr->ibm_ScreenFD = (* scr->ibm_ProbeFunc)() ) >= 0 ) {
		ibmScreens[ibmNumScreens++]= scr ;
		if (!ibmOpenAllScreens)
		    return TRUE ;
	    }
	    else
		scr->ibm_ScreenFD = -1 ;
	}
    }
    else
	return TRUE ;

    if ((ibmOpenAllScreens)&&(ibmNumScreens>0))
	return TRUE ;
    return FALSE ;
}

/***==================================================================***/

static	void
ibmAddScreens(screenInfo, argc, argv)
    ScreenInfo		*screenInfo;
    int			 argc;
    char		*argv[];
{
    PixmapFormatPtr	newFmt,oldFmt;
    int			 ndx,fmtNdx;
    static int		 been_here;

    TRACE(("ibmAddScreens(screenInfo= 0x%x, argc= %d, argv]=x%x)\n", 
							screenInfo,argc,argv));

    for (ndx=0;ndx<ibmNumScreens;ndx++) {
	if (!been_here) {
	    if (ibmScreens[ndx]->ibm_Wanted) {
		ErrorF("Multiple requests for screen '%s'  -- ignored\n",
							ibmScreenFlag(ndx));
		continue;
	    }
	    ibmScreens[ndx]->ibm_Wanted= TRUE;
#if !defined(AIXrt)
	    if (ndx>0) {
		ibmScreenMinX(ndx)+= ibmScreenMaxX(ndx-1);
		ibmScreenMaxX(ndx)+= ibmScreenMaxX(ndx-1);
	    }
#endif /* RtAIX */
	}

	for (fmtNdx=0;fmtNdx<ibmNumFormats(ndx);fmtNdx++) {
	    if (!ibmFormatExists(screenInfo,&ibmScreenFormats(ndx)[fmtNdx])) {
		newFmt= &ibmScreenFormats(ndx)[fmtNdx];
		oldFmt= &screenInfo->formats[screenInfo->numPixmapFormats++];
		oldFmt->depth= 		newFmt->depth;
		oldFmt->bitsPerPixel=	newFmt->bitsPerPixel;
		oldFmt->scanlinePad=	newFmt->scanlinePad;
		if (screenInfo->numPixmapFormats>MAXFORMATS) {
		    ErrorF("WSGO!! Too many formats! Exiting\n");
		    exit(1);
		}
	    }
	}
	ibmSetScreenState(ndx,SCREEN_ACTIVE);
	AddScreen(ibmScreenInit(ndx),argc,argv);
    }
    been_here= TRUE;
}

/***==================================================================***/

static DevicePtr keyboard;
static DevicePtr mouse;

void
InitInput()
{
extern	DevicePtr	OS_MouseProc(),OS_KeybdProc();

    TRACE(("InitInput()\n"));

    OS_InitInput();
    mouse=	AddInputDevice(OS_MouseProc,	TRUE);
    keyboard=	AddInputDevice(OS_KeybdProc,	TRUE);

    RegisterPointerDevice( mouse, MOTION_BUFFER_SIZE );
    RegisterKeyboardDevice( keyboard );

    OS_AddAndRegisterOtherDevices();
    return ;
}

/***==================================================================***/

int
ddxProcessArgument(argc,argv,i)
int	argc;
char	*argv[];
int	i;
{
int			skip= 1;

extern	char *getenv();
extern	char *ibmArenaFile;
extern	int ibmQuietFlag ;

    TRACE(("ddxProcessArgument( argc= %d, argv= 0x%x, i=%d )\n",argc,argv,i));

#ifdef OS_ProcessArgument
    if (skip=OS_ProcessArgument(argc,argv,i))		return(skip);
    else						skip= 1;
#endif
    if 	    ( strcmp( argv[i], "-bs" ) == 0 )		ibmAllowBackingStore= 1;
    else if ( strcmp( argv[i], "-nobs" ) == 0 )		ibmAllowBackingStore= 0;
    else if ( strcmp( argv[i], "-lock" ) == 0 )		ibmLockEnabled= 1;
    else if ( strcmp( argv[i], "-nolock" ) == 0 )	ibmLockEnabled= 0;
    else if ( strcmp( argv[i], "-pckeys" ) == 0 )	ibmUsePCKeys= 1;
    else if ( strcmp( argv[i], "-rtkeys" ) == 0 )	ibmUsePCKeys= 0;
    else if ( strcmp( argv[i], "-quiet" ) == 0 )	ibmQuietFlag = 1 ;
    else if ( strcmp( argv[i], "-verbose" ) == 0 )	ibmQuietFlag = 0;
    else if ( strcmp( argv[i], "-wrapx"  ) == 0 )	ibmXWrapScreen= TRUE;
    else if ( strcmp( argv[i], "-wrapy"  ) == 0 )	ibmYWrapScreen= TRUE;
    else if ( strcmp( argv[i], "-wrap"  ) == 0 )	
					ibmXWrapScreen= ibmYWrapScreen= TRUE;
#if     TRACE_X
    else if ( strcmp( argv[i], "-trace"  ) == 0 )	ibmTrace= TRUE;
#endif
#ifndef IBM_MUST_USE_HDWR
    else if ( strcmp( argv[i], "-nohdwr" ) == 0 )	ibmUseHardware= FALSE;
#endif IBM_MUST_USE_HDWR
#ifdef IBM_SPECIAL_MALLOC
    else if ( strcmp( argv[i], "-malloc" ) == 0 )	{
	int lvl= atoi(argv[++i]);
	SetMallocCheckLevel(lvl);
	ErrorF("allocator check level set to %d...\n",lvl);
	skip= 2;
    }
    else if ( strcmp( argv[i], "-plumber" ) == 0 ) {
	ibmSetupPlumber(argv[++i]);
	skip= 2;
    }
#endif IBM_SPECIAL_MALLOC
    else if ( strcmp( argv[i], "-T") == 0)
	ibmDontZap = TRUE;
    else if ( strcmp( argv[i], "-wp") == 0)
	{
	ibmWhitePixelText = argv[++i];
	skip= 2;
    	}
    else if ( strcmp( argv[i], "-bp") == 0)
	{
	ibmBlackPixelText = argv[++i];
	skip= 2;
    	}
    else if ( strcmp( argv[i], "-all" ) == 0 )	ibmOpenAllScreens= TRUE;
    else {
	register ibmPerScreenInfo **ppScr = ibmPossibleScreens;
	register ibmPerScreenInfo  *pScr;
	int found= FALSE;

	skip= 0;
	while ( ( pScr = *ppScr++ ) && pScr->ibm_ScreenFlag && !found ) {
    	    if (!strcmp(argv[i],pScr->ibm_ScreenFlag)) {
		skip= 1;
		ibmScreensWanted++;
		if ( ( pScr->ibm_ScreenFD = (*(pScr->ibm_ProbeFunc))() ) >= 0 )
		    ibmScreens[ibmNumScreens++]= pScr;
		else  {
		    ErrorF("%s not available\n",&argv[i][1]);
		}
		found= TRUE;
	    }
	}
	/* No More pre-linked screens! Try dynamic linking. */
#if defined(DYNAMIC_LINK_SCREENS)
	if (!found) {
	    if ( pScr = ibmDynamicScreenAttach( argv[i] ) ) {
		ibmScreens[ ibmNumScreens++ ] = pScr;
		skip= 1;
	    }
	}
#endif
    }
    return(skip);
}

/***==================================================================***/

extern void ibmPrintBuildDate() ;
extern void ibmInfoMsg() ;

void
InitOutput(screenInfo, argc, argv)
    ScreenInfo	*screenInfo;
    int		 argc;
    char	*argv[];
{
    static	int	been_here= 0;

    TRACE(("InitOutput( screenInfo= 0x%x)\n",screenInfo));

    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    screenInfo->numPixmapFormats = 1;
    screenInfo->formats[0].depth= 1;
    screenInfo->formats[0].bitsPerPixel= 1;
    screenInfo->formats[0].scanlinePad= BITMAP_SCANLINE_PAD;

    if (!been_here) {
	been_here= TRUE;

	if (ibmNumScreens!=ibmScreensWanted) {
	    ErrorF("Couldn't open all requested screens.");
	    exit(1);
	}
	else if ((ibmNumScreens==0)&&(!ibmFindSomeScreens())) {
	    ErrorF("Couldn't open any screens.");
	    exit(1);
	}

	/* Informational Messages */
	ibmPrintBuildDate();
	ibmInfoMsg(
  "X Window System protocol version %d, revision %d (vendor release %d)\n",
		X_PROTOCOL, X_PROTOCOL_REVISION, VENDOR_RELEASE ) ;

	OS_PreScreenInit();	/* usually opens /dev/bus */
	if (ibmUsePCKeys)	ibmInfoMsg( "Using PC keyboard layout...\n" );
	else			ibmInfoMsg( "Using RT keyboard layout...\n" );
    }
    ibmAddScreens(screenInfo,argc,argv);
    OS_PostScreenInit();
    return;
}
