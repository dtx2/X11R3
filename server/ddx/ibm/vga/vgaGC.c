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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaGC.c,v 6.3 88/10/24 22:21:44 paul Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaGC.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaGC.c,v 6.3 88/10/24 22:21:44 paul Exp $" ;
#endif

#include "X.h"
#include "Xproto.h"
#include "windowstr.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "screen.h"
#include "misc.h"
#include "font.h"
#include "gcstruct.h"
#include "cursorstr.h"
#include "region.h"

#include "mistruct.h"

#include "../../mfb/mfb.h"

#include "OScompiler.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "vgaProcs.h"

extern	void	miPolyPoint() ;
extern	int	ReduceRop() ;
extern	int	ibmAllowBackingStore ;

extern GCInterestRec vgaPrototypeGCInterest ;
extern ppcPrivGC vgaPrototypeGCPriv ;
extern GC vgaPrototypeGC ;

#define vgaGCInterestValidateMask \
( GCLineStyle | GCLineWidth | GCJoinStyle | GCBackground | GCForeground	\
| GCFunction | GCPlaneMask | GCFillStyle | GC_CALL_VALIDATE_BIT		\
| GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode )

Bool
vgaCreateGC( pGC )
register GCPtr const pGC ;
{
	register ppcPrivGC *pPriv ;
	register GCInterestPtr pQ ;

	if ( !( pPriv = (ppcPrivGC *) Xalloc( sizeof( ppcPrivGC ) ) ) )
		return FALSE ;

	if ( !( pQ = (GCInterestPtr) Xalloc( sizeof( GCInterestRec ) ) ) ) {
		Xfree( pPriv ) ;
		return FALSE ;
	}

	{ /* Save & Restore any passed-in variables !! */
		int         	orig_depth		= pGC->depth ;
		PixmapPtr	orig_tile		= pGC->tile ;
		PixmapPtr	orig_stipple		= pGC->stipple ;
		FontPtr		orig_font		= pGC->font ;
		int		orig_numInDashList	= pGC->numInDashList ;
		unsigned char	*orig_dash		= pGC->dash ;
		unsigned long	orig_serialNumber	= pGC->serialNumber ;

		/* Copy The Prototype GC */
		*pGC = vgaPrototypeGC ;

		/* Now restore the pointers ! */
		pGC->depth = orig_depth ;
		pGC->tile = orig_tile ;
		pGC->stipple = orig_stipple ;
		pGC->font = orig_font ;
		pGC->numInDashList = orig_numInDashList ;
		pGC->dash = orig_dash ;
		pGC->serialNumber  = orig_serialNumber ;
	}

	/* Copy The Prototype devPriv */
	pGC->devPriv = (pointer) pPriv ;
	*pPriv = vgaPrototypeGCPriv ;

	pPriv->pAbsClientRegion = (* pGC->pScreen->RegionCreate)( NULL, 1 ) ;
	pPriv->pCompositeClip = (* pGC->pScreen->RegionCreate)( NULL, 1 ) ;

	*pQ = vgaPrototypeGCInterest ;
	/* Now link this device into the GCque */
	pGC->pNextGCInterest = pGC->pLastGCInterest = pQ ;
	pQ->pNextGCInterest = (GCInterestPtr) &pGC->pNextGCInterest ;
	pQ->pLastGCInterest = (GCInterestPtr) &pGC->pNextGCInterest ;

	return TRUE ;
}

#define WINMOVED( pWin, pGC ) \
 ( ( pWin->absCorner.x != pGC->lastWinOrg.x ) \
|| ( pWin->absCorner.y != pGC->lastWinOrg.y ) )

/* Clipping conventions
	if the drawable is a window
	    CT_REGION ==> pCompositeClip really is the composite
	    CT_other ==> pCompositeClip is the window clip region
	if the drawable is a pixmap
	    CT_REGION ==> pCompositeClip is the translated client region
		clipped to the pixmap boundary
	    CT_other ==> pCompositeClip is the pixmap bounding box
*/

void
vgaValidateGC( pGC, pQ, changes, pDrawable )
    register GCPtr	pGC ;
    GCInterestPtr	pQ ;
    register Mask	changes ;
    DrawablePtr		pDrawable ;
{
    register ppcPrivGCPtr devPriv ;
    register int index ;	/* used for stepping through bitfields */
    WindowPtr 	pWin ;
    Mask	bsChanges = 0 ;

    devPriv = (ppcPrivGCPtr) (pGC->devPriv) ;

    if ( pDrawable->type != devPriv->lastDrawableType ) {
        devPriv->lastDrawableType = pDrawable->type ;
	if ( pDrawable->type == DRAWABLE_PIXMAP ) {
	    devPriv->FillArea	= mfbSolidInvertArea ;
	    pGC->CopyArea	= miCopyArea ;
    	    pGC->PolyFillRect	= miPolyFillRect ;
	    pGC->PushPixels	= miPushPixels ;
	}
	else {
	    devPriv->FillArea	= ppcAreaFill ;
	    pGC->CopyArea	= ppcCopyArea ;
    	    pGC->PolyFillRect	= ppcPolyFillRect ;
	    pGC->PushPixels	= ppcPushPixels ;
	}
	bsChanges |= MIBS_COPYAREA | MIBS_POLYRECTANGLE | MIBS_PUSHPIXELS ;
	changes = ~0 ;
    }

    if ( pDrawable->type == DRAWABLE_PIXMAP ) {
	ppcValidateGC( pGC, pQ, changes, pDrawable ) ;
	return ;
    }

    changes &= vgaGCInterestValidateMask ;
    /* If Nothing REALLY Changed, Just Return */
    if ( pDrawable->serialNumber == (pGC->serialNumber & DRAWABLE_SERIAL_BITS) )
        if ( !( changes &= ~ GC_CALL_VALIDATE_BIT ) )
	    return ;

    pWin = ( pDrawable->type == DRAWABLE_WINDOW )
	 ? (WindowPtr) pDrawable
    /* else i.e. pDrawable->type == UNDRAWABLE_WINDOW */
	 : (WindowPtr) NULL ;

    /*
	if the client clip is different or moved OR
	the subwindowMode has changed OR
	the window's clip has changed since the last validation
	we need to recompute the composite clip
    */
    if ( changes & ( GCClipXOrigin | GCClipYOrigin | GCClipMask
		   | GCSubwindowMode | GC_CALL_VALIDATE_BIT ) )
    {
	    int freeTmpClip, freeCompClip ;
	    RegionPtr pregWin ;	/* clip for this window, without client clip */

	/* if there is a client clip (always a region, for us) AND
		it has moved or is different OR
		the window has moved
	   we need to (re)translate it.
	*/
	if ( ( pGC->clientClipType == CT_REGION )
	  && ( ( changes & ( GCClipXOrigin|GCClipYOrigin|GCClipMask ) )
	    || ( pWin && WINMOVED( pWin, pGC ) ) ) ) {
	    /* retranslate client clip */
	    (* pGC->pScreen->RegionCopy)( devPriv->pAbsClientRegion,
					  pGC->clientClip) ;
	    if (pWin)
	    {
		pGC->lastWinOrg.x = pWin->absCorner.x ;
		pGC->lastWinOrg.y = pWin->absCorner.y ;
	    }
	    else
	    {
		pGC->lastWinOrg.x = 0 ;
		pGC->lastWinOrg.y = 0 ;
	    }
	    (* pGC->pScreen->TranslateRegion)(
		       devPriv->pAbsClientRegion,
		       pGC->lastWinOrg.x + pGC->clipOrg.x,
		       pGC->lastWinOrg.y + pGC->clipOrg.y) ;
	}

	    if (pGC->subWindowMode == IncludeInferiors)
	    {
		pregWin = NotClippedByChildren(pWin) ;
		freeTmpClip = FREE_CC ;
	    }
	    else
	    {
		pregWin = pWin->clipList ;
		freeTmpClip = REPLACE_CC ;
	    }
	    freeCompClip = devPriv->freeCompClip ;

	    /* if there is no client clip, we can get by with
	       just keeping the pointer we got, and remembering
	       whether or not should destroy (or maybe re-use)
	       it later.  this way, we avoid unnecessary copying
	       of regions.  (this wins especially if many clients clip
	       by children and have no client clip.)
	    */
	    if (pGC->clientClipType == CT_NONE)
	    {
		if(freeCompClip == FREE_CC)
		{
		    (* pGC->pScreen->RegionDestroy) (devPriv->pCompositeClip) ;
		}
		devPriv->pCompositeClip = pregWin ;
		devPriv->freeCompClip = freeTmpClip ;
	    }
	    else
	    {
		/* we need one 'real' region to put into the composite
		   clip.
			if pregWin and the current composite clip
		   are real, we can get rid of one.
			if the current composite clip is real and
		   pregWin isn't, intersect the client clip and
		   pregWin into the existing composite clip.
			if pregWin is real and the current composite
		   clip isn't, intersect pregWin with the client clip
		   and replace the composite clip with it.
			if neither is real, create a new region and
		   do the intersection into it.
		*/

		if ((freeTmpClip == FREE_CC) && (freeCompClip == FREE_CC))
		{
		    (* pGC->pScreen->Intersect)(
			devPriv->pCompositeClip,
			pregWin,
			devPriv->pAbsClientRegion) ;
		    (* pGC->pScreen->RegionDestroy)(pregWin) ;
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			(freeCompClip == FREE_CC))
		{
		    (* pGC->pScreen->Intersect)(
			devPriv->pCompositeClip,
			pregWin,
			devPriv->pAbsClientRegion) ;
		}
		else if ((freeTmpClip == FREE_CC) &&
			 (freeCompClip == REPLACE_CC))
		{
		    (* pGC->pScreen->Intersect)(
		       pregWin,
		       pregWin,
		       devPriv->pAbsClientRegion) ;
		    devPriv->pCompositeClip = pregWin ;
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			 (freeCompClip == REPLACE_CC))
		{
		    devPriv->pCompositeClip =
			(* pGC->pScreen->RegionCreate)(NULL, 1) ;
		    (* pGC->pScreen->Intersect)(
			devPriv->pCompositeClip,
			pregWin,
			devPriv->pAbsClientRegion) ;
		}
		devPriv->freeCompClip = FREE_CC ;
	    }
	 /* end of composite clip for a window */
    }

    changes &= ~ ( GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode
		| GC_CALL_VALIDATE_BIT ) ;

#define LOWBIT( x ) ( x & - x ) /* Two's complement */
    while ( index = LOWBIT( changes ) ) {
	switch ( index ) {

	  case GCLineStyle:
	  case GCLineWidth:
	    pGC->Polylines = ( ( pGC->lineStyle == LineSolid )
		? ( ( pGC->lineWidth == 0 ) ? ppcScrnZeroLine
					    : miWideLine )
		: ( ( pGC->lineWidth == 0 ) ? ppcScrnZeroDash
					    : miWideDash ) ) ;
	    changes &= ~( GCLineStyle | GCLineWidth ) ;
	    bsChanges |= MIBS_POLYLINES ;
	    break ;
	  case GCJoinStyle:
	    pGC->LineHelper =
		( pGC->joinStyle == JoinMiter ) ? miMiter : miNotMiter ;
	    changes &= ~ index ; /* i.e. changes &= ~ GCJoinStyle */
	    break ;

	  case GCBackground:
	    if ( pGC->fillStyle != FillOpaqueStippled ) {
		changes &= ~ index ; /* i.e. changes &= ~GCBackground */
		break ;
	    } /* else Fall Through */
	  case GCForeground:
	    if ( pGC->fillStyle == FillTiled ) {
		changes &= ~ index ; /* i.e. changes &= ~GCForeground */
		break ;
	    } /* else Fall Through */
	  case GCFunction:
	  case GCPlaneMask:
	  case GCFillStyle:
	    { /* new_fill */
		int fillStyle ;
		ppcGetReducedColorRrop( pGC, pDrawable->depth,
					&devPriv->colorRrop ) ;
		fillStyle = devPriv->colorRrop.fillStyle ;
		/* install a suitable fillspans */
		if ( fillStyle == FillSolid )
		    pGC->FillSpans = ppcSolidWindowFS ;
		else if ( fillStyle == FillStippled )
		    pGC->FillSpans = ppcStippleWindowFS ;
		else if ( fillStyle == FillOpaqueStippled )
		    pGC->FillSpans = ppcOpStippleWindowFS ;
		else /*  fillStyle == FillTiled */
		    pGC->FillSpans = ppcTileFS ;
	    } /* end of new_fill */
	    changes &= ~( GCBackground | GCForeground
		     | GCFunction
		     | GCPlaneMask | GCFillStyle ) ;
	    bsChanges |= MIBS_FILLSPANS ;
	    break ;

	default:
	    ErrorF("vgaValidateGC: Unexpected GC Change\n") ;
	    changes &= ~ index ; /* Remove it anyway */
	    break ;
	}
    }

    /*
     * If this GC has ever been used with a window with backing-store enabled,
     * we must call miVaidateBackingStore to keep the backing-store module
     * up-to-date, should this GC be used with that drawable again. In addition,
     * if the current drawable is a window and has backing-store enabled, we
     * also call miValidateBackingStore to give it a chance to get its hooks in.
     */
    if ( ibmAllowBackingStore
      && ( pGC->devBackingStore
	|| ( pWin && ( pWin->backingStore != NotUseful ) ) ) )
	miValidateBackingStore( pDrawable, pGC, bsChanges ) ;

    return ;
}
