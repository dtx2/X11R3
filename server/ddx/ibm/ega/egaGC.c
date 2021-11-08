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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/ega/RCS/egaGC.c,v 9.0 88/10/18 12:51:56 erik Exp Locker: paul $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/ega/RCS/egaGC.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/ega/RCS/egaGC.c,v 9.0 88/10/18 12:51:56 erik Exp Locker: paul $";
static char sccsid[] = "@(#)egagc.c	1.1 88/09/13 22:15:40";
#endif

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "screen.h"
#include "font.h"
#include "region.h"

#include "mistruct.h"
#include "mfb.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "egaProcs.h"

extern	int	ReduceRop() ;
extern	int	ibmAllowBackingStore;

static PixmapPtr BogusPixmap = (PixmapPtr) 1 ;

#define egaGCInterestValidateMask \
( GCLineStyle | GCLineWidth | GCJoinStyle | GCBackground | GCForeground	\
| GCFunction | GCPlaneMask | GCFillStyle | GC_CALL_VALIDATE_BIT		\
| GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode )

Bool
egaCreateGC( pGC )
    register GCPtr const pGC ;
{
    register ppcPrivGC *pPriv ;
    register GCInterestPtr pQ ;

    pGC->clientClip = NULL ;
    pGC->clientClipType = CT_NONE ;

    /* some of the output primitives aren't really necessary, since
       they will be filled in ValidateGC because of dix/CreateGC()
       setting all the change bits.  Others are necessary because although
       they depend on being a monochrome frame buffer, they don't change
    */

    pGC->FillSpans = ppcSolidFS ;
    pGC->SetSpans = ppcSetSpans ;
    pGC->PutImage = miPutImage ;
    pGC->CopyPlane = miCopyPlane ;
    pGC->PolyPoint = ppcPolyPoint ;

    pGC->Polylines = ppcScrnZeroLine ;
    pGC->PolySegment = miPolySegment ;
    pGC->PolyRectangle = miPolyRectangle ;
    pGC->PolyArc = miPolyArc ;
    pGC->FillPolygon = miFillPolygon ;
    pGC->PolyFillArc = miPolyFillArc ;
    pGC->PolyText8 = miPolyText8 ;
    pGC->ImageText8 = miImageText8 ;
    pGC->PolyText16 = miPolyText16 ;
    pGC->ImageText16 = miImageText16 ;
    pGC->ImageGlyphBlt = ppcImageGlyphBlt ;
    pGC->PolyGlyphBlt = ppcPolyGlyphBlt ;
    pGC->LineHelper = miMiter ;
    pGC->ChangeClip = mfbChangeClip ;
    pGC->DestroyClip = mfbDestroyClip ;


    pGC->PushPixels = miPushPixels ;
    pGC->PolyFillRect = miPolyFillRect ;
    pGC->CopyArea = ppcCopyArea ;
    pGC->devBackingStore = (pointer)NULL;

    /* mfb wants to translate before scan convesion */
    pGC->miTranslate = 1 ;

    if ( !( pPriv = (ppcPrivGC *) Xalloc( sizeof( ppcPrivGC ) ) ) )
	return FALSE ;

    pGC->devPriv = (pointer) pPriv ;
    pPriv->rop = ReduceRop( pGC->alu, pGC->fgPixel ) ;
    pPriv->fExpose = TRUE ;
    pPriv->pAbsClientRegion =(* pGC->pScreen->RegionCreate)(NULL, 1) ;
    pPriv->pCompositeClip = (*pGC->pScreen->RegionCreate)( NULL, 1 ) ;
    pPriv->freeCompClip = FREE_CC ;
    pPriv->lastDrawableType = -1 ;
    pPriv->lastDrawableDepth = -1 ;
    pPriv->pRotatedTile = NullPixmap ;
    pPriv->pRotatedStipple = NullPixmap ;
    pPriv->ppPixmap = &BogusPixmap ;
    pPriv->FillArea = mfbSolidInvertArea ;
    pPriv->devPriv = (pointer)0;

    if ( !( pQ = (GCInterestPtr) Xalloc( sizeof( GCInterestRec ) ) ) ) {
	Xfree( pPriv ) ;
	return FALSE ;
    }

    /* Now link this device into the GCque */
    pGC->pNextGCInterest = pQ ;
    pGC->pLastGCInterest = pQ ;
    pQ->pNextGCInterest = (GCInterestPtr) &pGC->pNextGCInterest ;
    pQ->pLastGCInterest = (GCInterestPtr) &pGC->pNextGCInterest ;
    pQ->length = sizeof( GCInterestRec ) ;
    pQ->owner = 0 ;		/* server owns this */
    pQ->ValInterestMask = egaGCInterestValidateMask ;
    pQ->ValidateGC = egaValidateGC ;
    pQ->ChangeInterestMask = 0 ; /* interested in nothing at change time */
    pQ->ChangeGC = (int (*) () ) NULL ;
    pQ->CopyGCSource = (void (*) () ) NULL ;
    pQ->CopyGCDest = mfbCopyGCDest ;
    pQ->DestroyGC = ppcDestroyGC ;
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
egaValidateGC(pGC, pQ, changes, pDrawable)
    register GCPtr	pGC;
    GCInterestPtr	pQ;
    register Mask	changes;
    DrawablePtr		pDrawable;
{
    register ppcPrivGCPtr devPriv;
    register int index;	/* used for stepping through bitfields */
    WindowPtr 	pWin;
    Mask	bsChanges= 0;

    devPriv = ((ppcPrivGCPtr) (pGC->devPriv));

    if ( pDrawable->type != devPriv->lastDrawableType ) {
        devPriv->lastDrawableType = pDrawable->type ;
	if ( pDrawable->type == DRAWABLE_PIXMAP ) {
	    pGC->CopyArea	= miCopyArea ;
    	    pGC->PolyFillRect	= miPolyFillRect ;
	    pGC->PushPixels	= miPushPixels ;
	}
	else {
	    pGC->CopyArea	= ppcCopyArea ;
    	    pGC->PolyFillRect	= ppcPolyFillRect ;
	    pGC->PushPixels	= ppcPushPixels ;
	}
	changes = ~0 ;
	bsChanges |= MIBS_COPYAREA|MIBS_POLYRECTANGLE|MIBS_PUSHPIXELS;
    }

    if (pDrawable->type == DRAWABLE_PIXMAP)
    {
	ppcValidateGC( pGC, pQ, changes, pDrawable ) ;
	return;
    }

    changes &= egaGCInterestValidateMask ;
    /* If Nothing REALLY Changed, Just Return */
    if ( pDrawable->serialNumber == (pGC->serialNumber & DRAWABLE_SERIAL_BITS) )
        if ( !( changes &= ~ GC_CALL_VALIDATE_BIT ) )
	    return ;

    if (pDrawable->type == DRAWABLE_WINDOW)
	pWin = (WindowPtr)pDrawable;
    else /* i.e. pDrawable->type == UNDRAWABLE_WINDOW */
	pWin = (WindowPtr)NULL;

    /*
	if the client clip is different or moved OR
	the subwindowMode has changed OR
	the window's clip has changed since the last validation
	we need to recompute the composite clip
    */
    if ( changes & ( GCClipXOrigin | GCClipYOrigin | GCClipMask
		   | GCSubwindowMode | GC_CALL_VALIDATE_BIT ) )
    {
	    int freeTmpClip, freeCompClip;
	    RegionPtr pregWin;	/* clip for this window, without client clip */

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
					  pGC->clientClip);
	    if (pWin)
	    {
		pGC->lastWinOrg.x = pWin->absCorner.x;
		pGC->lastWinOrg.y = pWin->absCorner.y;
	    }
	    else
	    {
		pGC->lastWinOrg.x = 0;
		pGC->lastWinOrg.y = 0;
	    }
	    (* pGC->pScreen->TranslateRegion)(
		       devPriv->pAbsClientRegion,
		       pGC->lastWinOrg.x + pGC->clipOrg.x,
		       pGC->lastWinOrg.y + pGC->clipOrg.y);
	}

	    if (pGC->subWindowMode == IncludeInferiors)
	    {
		pregWin = NotClippedByChildren(pWin);
		freeTmpClip = FREE_CC;
	    }
	    else
	    {
		pregWin = pWin->clipList;
		freeTmpClip = REPLACE_CC;
	    }
	    freeCompClip = devPriv->freeCompClip;

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
		    (* pGC->pScreen->RegionDestroy) (devPriv->pCompositeClip);
		}
		devPriv->pCompositeClip = pregWin;
		devPriv->freeCompClip = freeTmpClip;
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
			devPriv->pAbsClientRegion);
		    (* pGC->pScreen->RegionDestroy)(pregWin);
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			(freeCompClip == FREE_CC))
		{
		    (* pGC->pScreen->Intersect)(
			devPriv->pCompositeClip,
			pregWin,
			devPriv->pAbsClientRegion);
		}
		else if ((freeTmpClip == FREE_CC) &&
			 (freeCompClip == REPLACE_CC))
		{
		    (* pGC->pScreen->Intersect)(
		       pregWin,
		       pregWin,
		       devPriv->pAbsClientRegion);
		    devPriv->pCompositeClip = pregWin;
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			 (freeCompClip == REPLACE_CC))
		{
		    devPriv->pCompositeClip =
			(* pGC->pScreen->RegionCreate)(NULL, 1);
		    (* pGC->pScreen->Intersect)(
			devPriv->pCompositeClip,
			pregWin,
			devPriv->pAbsClientRegion);
		}
		devPriv->freeCompClip = FREE_CC;
	    }
	 /* end of composite clip for a window */
    }

    changes &= ~ ( GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode
		| GC_CALL_VALIDATE_BIT ) ;

    for ( index = 1 ; changes ; index <<= 1 ) {
	while ( !( changes & index ) )
		index <<= 1 ; /* Search For first set bit */
	switch ( index ) {

	  case GCLineStyle:
	  case GCLineWidth:
	    pGC->Polylines = ( ( pGC->lineStyle == LineSolid ) 
		? ( ( pGC->lineWidth == 0 ) ? ppcScrnZeroLine
					    : miWideLine )
		: ( ( pGC->lineWidth == 0 ) ? ppcScrnZeroDash
					    : miWideDash ) ) ;
	    changes &= ~( GCLineStyle | GCLineWidth ) ;
	    bsChanges |= MIBS_POLYLINES;
	    break;
	  case GCJoinStyle:
	    pGC->LineHelper =
		( pGC->joinStyle == JoinMiter ) ? miMiter : miNotMiter ;
	    changes &= ~ index ; /* i.e. changes &= ~ GCJoinStyle */
	    break;

	  case GCBackground:
	    if ( pGC->fillStyle != FillOpaqueStippled ) {
		changes &= ~ index ; /* i.e. changes &= ~GCBackground */
		break;
	    } /* else Fall Through */
	  case GCForeground:
	    if ( pGC->fillStyle == FillTiled ) {
		changes &= ~ index ; /* i.e. changes &= ~GCForeground */
		break;
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
		    pGC->FillSpans = ppcSolidWindowFS;
		else if ( fillStyle == FillStippled )
		    pGC->FillSpans = ppcStippleWindowFS ;
		else if ( fillStyle == FillOpaqueStippled )
		    pGC->FillSpans = ppcOpStippleWindowFS ;
		else /*  fillStyle == FillTiled */
		    pGC->FillSpans = ppcTileFS;
	    } /* end of new_fill */
	    changes &= ~( GCBackground | GCForeground
		     | GCFunction
		     | GCPlaneMask | GCFillStyle ) ;
	    bsChanges |= MIBS_FILLSPANS;
	    break;

	default:
	    ErrorF("egaValidateGC: Unexpected GC Change\n") ;
	    changes &= ~ index ; /* Remove it anyway */
	    break;
	}
    }

    /*
     * If this GC has ever been used with a window with backing-store enabled,
     * we must call miVaidateBackingStore to keep the backing-store module
     * up-to-date, should this GC be used with that drawable again. In addition,
     * if the current drawable is a window and has backing-store enabled, we
     * also call miValidateBackingStore to give it a chance to get its hooks in.
     */
    if (ibmAllowBackingStore && (pGC->devBackingStore ||
	(pWin && (pWin->backingStore != NotUseful))))
    {
	miValidateBackingStore(pDrawable, pGC, bsChanges);
    }
    return ;
}
