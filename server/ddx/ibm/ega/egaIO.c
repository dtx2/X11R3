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
		Copyright IBM Corporation 1988

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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaIO.c,v 9.0 88/10/18 12:52:05 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaIO.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaIO.c,v 9.0 88/10/18 12:52:05 erik Exp $";
#endif

#include "X.h"
#include "Xproto.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "screen.h"
#include "miscstruct.h"
#include "colormap.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "windowstr.h"
#include "window.h"
#include "font.h"
#include "resource.h"

#include "OScursor.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "egaVideo.h"
#include "egaProcs.h"

#include "ibmScreen.h"

#include "ibmTrace.h"

extern	void		AllocColor() ;
extern	void		ErrorF() ;
extern	pointer		LookupID() ;

extern	void		miRecolorCursor() ;

extern	int		egaScreenInitHW() ;
extern	Bool		egaInitScreen() ;
extern	void		egaCursorInit() ;
extern	void		egaCloseHW() ;

/* Global Variable */
int egaNumberOfPlanes  = 0 ;
int egaDisplayTubeType = 0 ;

PixmapFormatRec	egaFormats[] = { { 4, 8, 32 } } ;

static int HardwareReady = 0 ;

Bool
egaScreenClose( index, pScreen )
register const int index;
register ScreenPtr const pScreen ;
{
    TRACE( ( "egaScreenClose( index= %d, pScreen= 0x%x )\n", index, pScreen ));
    if ( HardwareReady ) {
	egaCloseHW( index ) ;
	HardwareReady = 0 ;
    }
    if ( pScreen->devPrivate )
	Xfree( pScreen->devPrivate ) ;

    return TRUE ;
}

Bool
egaScreenInit( index, pScreen, argc, argv )
register const int index ;
register ScreenPtr const pScreen ;
register int const argc ;		/* these two may NOT be changed */
register char * const * const argv ;
{
    static int been_here = 0 ;
    ColormapPtr pColormap ;
    register int retval ;

    TRACE(
	( "egaScreenInit( index= %d, pScreen= 0x%x, argc= %d, argv= 0x%x )\n",
		index, pScreen, argc, argv ) ) ;

    if ( !been_here ) {
/*	egaInitFontCache() ;	MAYBE SOMEDAY */
	been_here = TRUE ;
    }

    if ( !HardwareReady ) {
	egaNumberOfPlanes  = egaScreenInitHW( index ) ;
	HardwareReady      = 1 ;
    }

    retval = egaInitScreen( index, pScreen, MAX_COLUMN + 1, MAX_ROW + 1,
			    80, 60 ) ;

    pScreen->CloseScreen =		egaScreenClose ;
    pScreen->SaveScreen =		ibmSaveScreen ;
    pScreen->RealizeCursor =		ppcRealizeCursor ;
    pScreen->UnrealizeCursor =		ppcUnrealizeCursor ;
    pScreen->DisplayCursor =		egaDisplayCursor ;

    pScreen->SetCursorPosition =	OS_SetCursorPosition ;
    pScreen->CursorLimits =		OS_CursorLimits ;
    pScreen->PointerNonInterestBox =	OS_PointerNonInterestBox ;
    pScreen->ConstrainCursor =		OS_ConstrainCursor ;

    pScreen->RecolorCursor =		miRecolorCursor ;
    pScreen->QueryBestSize =		ppcQueryBestSize ;
    pScreen->CreateGC =			egaCreateGC ;
    pScreen->ChangeWindowAttributes =	ppcChangeWindowAttributes ;

    pScreen->CreateWindow =		ppcCreateWindowForXYhardware ;

    CreateColormap( pScreen->defColormap, pScreen,
		    LookupID( pScreen->rootVisual, RT_VISUALID, RC_CORE ),
		    &pColormap, AllocNone, 0 ) ;
    ppcDefineDefaultColormapColors( 
			(VisualPtr) LookupID( pScreen->rootVisual,
					      RT_VISUALID, RC_CORE ),
			pColormap ) ;
    pColormap->pScreen  = pScreen ;
    ppcInstallColormap( pColormap ) ;
    ppcAllocBlackAndWhitePixels( pColormap, ibmBlackPixelText,
			         ibmWhitePixelText ) ;

    egaCursorInit( index ) ;
    ibmScreen( index ) = pScreen ;

    return retval ;
}
