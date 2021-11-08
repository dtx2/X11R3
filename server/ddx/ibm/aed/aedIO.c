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
		Copyright IBM Corporation 1987

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

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/aed/RCS/aedIO.c,v 9.1 88/10/17 14:31:25 erik Exp $";
#endif

#include "X.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "pixmapstr.h"
#include "miscstruct.h"
#include "colormapst.h"
#include "resource.h"

#include "mfb.h"

#include "OScursor.h"

#include "ibmColor.h"
#include "ibmScreen.h"

#include "aedHdwr.h"
#include "aedProcs.h"
#include "ibmTrace.h"

extern	void	miRecolorCursor();
void aedResolveColor();
extern ColormapPtr pColormap;
extern void ibmQueryBestSize() ;
/***==================================================================***/

Bool
aedScreenInit(index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;		/* these two may NOT be changed */
    char **argv;
{
    Bool retval;
    static int been_here;
    ColormapPtr cmap;

    TRACE(("aedScreenInit( index= %d, pScreen= 0x%x, argc= %d, argv= 0x%x )\n",
						index,pScreen,argc,argv));

    if (!been_here) {
	if ( !aedScreenInitHW( index ) )
	    return FALSE ;
	been_here = TRUE;
    }

    retval = aedInitScreen(index, pScreen, 0x80000000, 
				AED_SCREEN_WIDTH, AED_SCREEN_HEIGHT, 80, 80);
    aedCursorInit(index);
    pScreen->CloseScreen=	aedScreenClose;
    pScreen->SaveScreen=	ibmSaveScreen;
    pScreen->RealizeCursor=	aedRealizeCursor;
    pScreen->UnrealizeCursor=	aedUnrealizeCursor;
    pScreen->DisplayCursor=	aedDisplayCursor;

    pScreen->SetCursorPosition=	OS_SetCursorPosition;
    pScreen->CursorLimits=	OS_CursorLimits;
    pScreen->PointerNonInterestBox= OS_PointerNonInterestBox;
    pScreen->ConstrainCursor=	OS_ConstrainCursor;

    pScreen->RecolorCursor=	miRecolorCursor;
    pScreen->QueryBestSize=	ibmQueryBestSize;
    pScreen->CreateGC=		aedCreateGC;
    pScreen->CreateWindow=	aedCreateWindow;
    pScreen->ChangeWindowAttributes=	aedChangeWindowAttributes;
    pScreen->ResolveColor = 	ibmResolveColorMono;
    pScreen->CreateColormap = 	ibmCreateColormapMono;
    pScreen->DestroyColormap = 	ibmDestroyColormapMono;

    CreateColormap(pScreen->defColormap, pScreen,
		   LookupID(pScreen->rootVisual, RT_VISUALID, RC_CORE),
		   &cmap, AllocNone, 0);
    mfbInstallColormap(cmap);
    ibmScreen(index) = pScreen;

    return retval ;
}

aedScreenClose(index, pScreen)
register int		index;
register ScreenPtr	pScreen;
{
    TRACE(("aedScreenClose( index= %d, pScreen= 0x%x )\n",index,pScreen));

    if (pScreen->allowedDepths)
    {
	if (pScreen->allowedDepths->vids)
		Xfree(pScreen->allowedDepths->vids);
 
	Xfree(pScreen->allowedDepths);
    }

    if (pScreen->devPrivate)
	Xfree(pScreen->devPrivate);

    return TRUE ;
}
