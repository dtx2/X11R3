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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelIO.c,v 6.1 88/10/25 01:49:27 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelIO.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelIO.c,v 6.1 88/10/25 01:49:27 kbg Exp $";
#endif

#include "X.h"
#include "Xproto.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "pixmapstr.h"
#include "miscstruct.h"
#include "colormapst.h"
#include "windowstr.h"
#include "resource.h"
#include "font.h"

#include "OScursor.h"

#include "ibmScreen.h"
#include "ibmTrace.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "mpelFifo.h"
#include "mpelProcs.h"
#ifdef AIXEXTENSIONS
Bool mpelCreateWindow();
#endif

extern	void	miRecolorCursor();

PixmapFormatRec	mpelFormats[]= { { 8, 8, 32 } };

/***==================================================================***/

Bool
mpelScreenInit(index, pScreen, argc, argv)
    register int	index;
    register ScreenPtr	pScreen;
    int		argc;		/* these two may NOT be changed */
    char	**argv;
{
    static int	been_here;
    ColormapPtr pColormap;
    Bool	retval;

    TRACE(("mpelScreenInit(%d,0x%x,%d,0x%x)\n",index,pScreen,argc,argv));

    if (!been_here) {
	if (ibmScreenFD(index)==-1) {
	    ErrorF("mpel file descriptor is invalid\nExiting.\n");
	    exit(1);
	    /*NOTREACHED*/
	}
	mpelLoaduCode();
	been_here= TRUE;
    }

    retval = mpelInitScreen( index, pScreen, MPEL_WIDTH, MPEL_HEIGHT, 80, 80 ) ;
    pScreen->CloseScreen=	mpelScreenClose;
    pScreen->SaveScreen=	ibmSaveScreen;
    pScreen->RealizeCursor=	ppcRealizeCursor;
    pScreen->UnrealizeCursor=	ppcUnrealizeCursor;
    pScreen->DisplayCursor=	mpelDisplayCursor;

    pScreen->SetCursorPosition=	OS_SetCursorPosition;
    pScreen->CursorLimits=	OS_CursorLimits;
    pScreen->PointerNonInterestBox= OS_PointerNonInterestBox;
    pScreen->ConstrainCursor=	OS_ConstrainCursor;

    pScreen->RecolorCursor=	miRecolorCursor;
    pScreen->QueryBestSize=	ppcQueryBestSize;
    pScreen->CreateGC=		mpelCreateGC;
#ifdef AIXEXTENSIONS
    pScreen->CreateWindow=	mpelCreateWindow;
#else
    pScreen->CreateWindow=	ppcCreateWindow;   /* why here */
#endif
    pScreen->ChangeWindowAttributes=	ppcChangeWindowAttributes;

    CreateColormap(pScreen->defColormap, pScreen,
		   LookupID(pScreen->rootVisual, RT_VISUALID, RC_CORE),
		   &pColormap, AllocNone, 0);
    pColormap->pScreen = pScreen;

    /* Initializeation Hack -- Pre-define "reasonable" colors */
    ppcDefineDefaultColormapColors( 
			LookupID(pScreen->rootVisual, RT_VISUALID, RC_CORE),
			pColormap);

    ppcAllocBlackAndWhitePixels( pColormap, ibmBlackPixelText, 
					    ibmWhitePixelText ) ;
    mpelInstallColormap(pColormap);

    mpelInitPlaneMask();

    mpelCursorInit(index);
    ibmScreen(index) = pScreen;

    return retval ;
}

/***==================================================================***/

/*ARGSUSED*/
Bool
mpelScreenClose(index,pScreen)
    register int	index;
    register ScreenPtr	pScreen;
{
    TRACE(("mpelScreenClose( index= %d, pScreen= 0x%x )\n",index,pScreen));

    if ( pScreen->devPrivate )
	Xfree( pScreen->devPrivate ) ;

    return TRUE ;
}
