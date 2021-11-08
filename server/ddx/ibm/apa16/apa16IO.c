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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16IO.c,v 9.1 88/10/17 14:45:03 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16IO.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16IO.c,v 9.1 88/10/17 14:45:03 erik Exp $";
static char sccsid[] = "@(#)apa16io.c	3.1 88/09/22 09:30:50";
#endif

#include "X.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "pixmapstr.h"
#include "miscstruct.h"
#include "input.h"
#include "colormapst.h"
#include "resource.h"

#include "mfb.h"

#include "OScompiler.h"

#include "ibmColor.h"
#include "ibmScreen.h"

#include "apa16Decls.h"
#include "apa16Hdwr.h"

#include "OSio.h"
#include "OScursor.h"

#include "ibmTrace.h"

extern	void	miRecolorCursor();
extern	void	ibmQueryBestSize() ;

/***==================================================================***/

Bool
apa16ScreenInit(index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;		/* these two may NOT be changed */
    char **argv;
{
    Bool retval;
    static int been_here;
    ColormapPtr cmap;

    extern mfbScreenClose() ;

    TRACE(("apa16ScreenInit( index= %d, pScreen= 0x%x, argc= %d, argv= 0x%x )\n",index,pScreen,argc,argv));

    if (!been_here) {
	if ( ibmScreenFD(index) < 0)
	{
            ErrorF(  "apa16 file descriptor is invalid\n");
            return FALSE; 
	} 
	been_here= TRUE;
    }

    retval = mfbScreenInit(index, pScreen, APA16_BASE, 
				APA16_WIDTH, APA16_HEIGHT, 80, 80);
    apa16CursorInit(index);
    pScreen->CloseScreen=	apa16ScreenClose ;
    pScreen->SaveScreen=	ibmSaveScreen;
    pScreen->RealizeCursor=	apa16RealizeCursor;
    pScreen->UnrealizeCursor=	apa16UnrealizeCursor;
    pScreen->DisplayCursor=	apa16DisplayCursor;
    pScreen->SetCursorPosition=	OS_SetCursorPosition;
    pScreen->CursorLimits=	OS_CursorLimits;
    pScreen->PointerNonInterestBox= OS_PointerNonInterestBox;
    pScreen->ConstrainCursor=	OS_ConstrainCursor;
    pScreen->RecolorCursor=	miRecolorCursor;
    pScreen->QueryBestSize=	ibmQueryBestSize;
    pScreen->ResolveColor=	ibmResolveColorMono;
    pScreen->CreateColormap=	ibmCreateColormapMono;
    pScreen->DestroyColormap=	ibmDestroyColormapMono;
    pScreen->RealizeFont=	afRealizeFont;
    pScreen->UnrealizeFont=	afUnrealizeFont;
    pScreen->BlockHandler=	OS_BlockHandler;
    pScreen->WakeupHandler=	OS_WakeupHandler;

    if (!ibmAllowBackingStore)
	pScreen->backingStoreSupport = NotUseful;

    if (ibmUseHardware) {
	pScreen->CreateGC=			apa16CreateGC;
    	pScreen->CreateWindow=			apa16CreateWindow;
    	pScreen->ChangeWindowAttributes=	apa16ChangeWindowAttributes;
    } 

    CreateColormap(pScreen->defColormap, pScreen,
		   LookupID(pScreen->rootVisual, RT_VISUALID, RC_CORE),
		   &cmap, AllocNone, 0);
    mfbInstallColormap(cmap);

    if (ibmScreenState(index)!=SCREEN_INACTIVE) {
	QUEUE_INIT();
	WHITE_ON_BLACK();
    }
    ibmScreen(index)= pScreen;
    return(retval);
}

apa16ScreenClose( index, pScreen )
int		index;
ScreenPtr	pScreen;
{
    TRACE(("apa16ScreenClose( index= 0x%x, pScreen= 0x%x )\n",index,pScreen));
    /* let mfb clean up anything it has to... */
    return mfbScreenClose(pScreen);
}
