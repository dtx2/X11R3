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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaIO.c,v 6.2 88/10/24 22:21:52 paul Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaIO.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaIO.c,v 6.2 88/10/24 22:21:52 paul Exp $" ;
#endif

#include "X.h"
#include "Xproto.h"
#include "resource.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "screen.h"
#include "window.h"
#include "font.h"
#include "miscstruct.h"
#include "colormap.h"
#include "colormapst.h"
#include "cursorstr.h"

#include "pixmapstr.h"
#include "mistruct.h"

#include "OScompiler.h"
#include "ibmScreen.h"
#include "OScursor.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "vgaVideo.h"
#include "vgaProcs.h"

#include "ibmTrace.h"

extern	void		AllocColor() ;
extern	void		ErrorF() ;
extern	pointer		LookupID() ;

/* Global Variable */
extern int vgaNumberOfPlanes ;
extern int vgaDisplayTubeType ;

PixmapFormatRec	vgaFormats[] = { { 4, 8, 32 } } ;

static int HardwareReady = 0 ;
extern void NoopDDA() ;

/*ARGSUSED*/
Bool
vgaScreenClose( index, pScreen )
register const int index ;
register ScreenPtr const pScreen ;
{
TRACE( ( "vgaScreenClose( index= %d, pScreen= 0x%x )\n", index, pScreen )) ;
if ( HardwareReady ) {
	vgaCloseHW( index ) ;
	HardwareReady = 0 ;
}

return TRUE ;
}

/* Declared in "vgaData.c" */
extern GC vgaPrototypeGC ;
extern ScreenRec vgaScreenRec ;
extern ppcScrnPriv vgaScrnPriv ;
extern VisualRec vgaVisuals[] ;
extern DepthRec vgaDepths[] ;

Bool
vgaScreenInit( index, pScreen, argc, argv )
register const int index ;
register ScreenPtr const pScreen ;
register int const argc ;		/* these two may NOT be changed */
register char * const * const argv ;
{
	static int been_here = 0 ;
	ColormapPtr pColormap ;

	TRACE(
	( "vgaScreenInit( index= %d, pScreen= 0x%x, argc= %d, argv= 0x%x )\n",
		index, pScreen, argc, argv ) ) ;

	if ( !been_here ) {
/*		vgaInitFontCache() ;	MAYBE SOMEDAY */
		been_here = TRUE ;
	}

	if ( !HardwareReady ) {
		vgaNumberOfPlanes  = vgaScreenInitHW( index ) ;
		HardwareReady      = 1 ;
	}
	*pScreen = vgaScreenRec ; /* Copy The Prototype Structure */

	vgaPrototypeGC.pScreen = vgaScrnPriv.pixmap.drawable.pScreen = pScreen ;

	/* Set up the visuals */
	vgaVisuals[0].class = ( vgaDisplayTubeType == COLOR_TUBE )
			 ? PseudoColor : GrayScale ;
	vgaVisuals[0].screen = pScreen->myNum = index ;

	AddResource( ( vgaDepths[1].vids[0] = vgaVisuals[0].vid =
		       pScreen->rootVisual = FakeClientID( 0 ) ),
		     RT_VISUALID, vgaVisuals, NoopDDA, RC_CORE ) ;

	/* "dix" Colormap Stuff */
	CreateColormap( pScreen->defColormap = (Colormap) FakeClientID( 0 ),
			pScreen, vgaVisuals, &pColormap, AllocNone, 0 ) ;
	pColormap->pScreen  = pScreen ;

	/* "ppc" Colormap Stuff */
	ppcDefineDefaultColormapColors( vgaVisuals, pColormap ) ;
	ppcInstallColormap( pColormap ) ;
	ppcAllocBlackAndWhitePixels( pColormap,
				     ibmBlackPixelText,
				     ibmWhitePixelText ) ;

	vgaCursorInit( index ) ;
	ibmScreen( index ) = pScreen ;

	/* we MIGHT return 0 if we had been keeping track of potential
	   allocation failures.  one day someone will clean this up.
	*/
	return 1 ;
}
