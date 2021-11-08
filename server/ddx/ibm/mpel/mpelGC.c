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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelGC.c,v 6.3 88/10/25 01:48:41 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelGC.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelGC.c,v 6.3 88/10/25 01:48:41 kbg Exp $";
#endif

#include "X.h"
#include "Xproto.h"
#include "font.h"
#include "misc.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mfb.h"

#include "mistruct.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "ibmTrace.h"

#include "mpelProcs.h"
#include "mpelFifo.h"
#include "mpelHdwr.h"

extern	int		ibmAllowBackingStore;
static	PixmapPtr 	BogusPixmap = (PixmapPtr)1;

extern void ppcWideLine();	/* should be in ppcProcs.h */

#define mpelGCInterestValidateMask \
( GCLineStyle | GCLineWidth | GCJoinStyle | GCBackground | GCForeground	\
| GCFunction | GCPlaneMask | GCFillStyle | GC_CALL_VALIDATE_BIT		\
| GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode )

Bool
mpelCreateGC(pGC)
    register GCPtr pGC;
{
    ppcPrivGC 	*pPriv;
    mpelPrivGCPtr	mpelPriv;
    GCInterestPtr	pQ;

    TRACE(("mpelCreateGC(pGC= 0x%x)\n", pGC));

    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;

    /* some of the output primitives aren't really necessary, since
       they will be filled in ValidateGC because of dix/CreateGC()
       setting all the change bits.  Others are necessary because although
       they depend on being a monochrome frame buffer, they don't change
    */

    pGC->SetSpans = ppcSetSpans;
    pGC->PutImage = miPutImage;
    pGC->CopyPlane = miCopyPlane;
    pGC->PolyPoint = mpelPolyPoint;

/* below shouldn't be needed */
    pGC->PolySegment = miPolySegment;
    pGC->PolyFillRect = miPolyFillRect;
    pGC->PolyText8 = mpelPolyText8;
    pGC->PolyText16 = mpelPolyText16;
    pGC->ImageText8 = mpelImageText8;
    pGC->ImageText16 = mpelImageText16;
    pGC->LineHelper = miMiter;
    pGC->PushPixels = miPushPixels;
    pGC->FillSpans = ppcTileFS;
    pGC->CopyArea = ppcCopyArea;
    pGC->Polylines = miZeroLine;
    pGC->PolyArc = mpelPolyArc;
    pGC->FillPolygon = mpelFillPolygon;
/* above shouldn't be needed */

    pGC->PolyRectangle = miPolyRectangle;
    pGC->PolyFillArc = miPolyFillArc;
    pGC->ImageGlyphBlt = ppcImageGlyphBlt;
    pGC->PolyGlyphBlt = ppcPolyGlyphBlt;
    pGC->ChangeClip = mfbChangeClip;
    pGC->DestroyClip = mfbDestroyClip;
    pGC->CopyClip = mfbCopyClip;
    pGC->devBackingStore= (pointer)NULL;

    /* mfb wants to translate before scan convesion */
    pGC->miTranslate = 1;

    if ( !( pPriv = (ppcPrivGC *)Xalloc(sizeof(ppcPrivGC)) ) )
	return FALSE;

    pGC->devPriv = (pointer)pPriv;
    pPriv->rop = ReduceRop(pGC->alu, pGC->fgPixel);
    pPriv->ropFillArea = pPriv->rop;
    pPriv->fExpose = TRUE;
    pPriv->pAbsClientRegion =(* pGC->pScreen->RegionCreate)(NULL, 1);

    pPriv->freeCompClip = REPLACE_CC;
    pPriv->lastDrawableType = -1;
    pPriv->lastDrawableDepth = -1;
    pPriv->pRotatedTile = NullPixmap;
    pPriv->pRotatedStipple = NullPixmap;
    pPriv->ppPixmap = &BogusPixmap;
    pPriv->FillArea = mfbSolidInvertArea;

    if ( !( mpelPriv = (mpelPrivGC *)Xalloc(sizeof(mpelPrivGC)) ) ){
	Xfree(pPriv);
	return FALSE;
    }

    pPriv->devPriv = (pointer)mpelPriv;
    mpelPriv->LineType = 1;

    if ( !( pQ = (GCInterestPtr) Xalloc(sizeof(GCInterestRec)) ) ) {
	Xfree(mpelPriv);
	Xfree(pPriv);
	return FALSE;
    }

    /* Now link this device into the GCque */
    pGC->pNextGCInterest = pQ;
    pGC->pLastGCInterest = pQ;
    pQ->pNextGCInterest = (GCInterestPtr) &pGC->pNextGCInterest;
    pQ->pLastGCInterest = (GCInterestPtr) &pGC->pNextGCInterest;
    pQ->length = sizeof(GCInterestRec);
    pQ->owner = 0;		/* server owns this */
    pQ->ValInterestMask = mpelGCInterestValidateMask ;
    pQ->ValidateGC = mpelValidateGC;
    pQ->ChangeInterestMask = 0; /* interested in nothing at change time */
    pQ->ChangeGC = (int (*) () ) NULL;
    pQ->CopyGCSource = (void (*) () ) NULL;
    pQ->CopyGCDest = mfbCopyGCDest;
    pQ->DestroyGC = ppcDestroyGC;
    return TRUE;
}

/*
 * mpelDestroyGCPriv - do whatever is necessary to clean up
 *	the devPriv field in the ppcGC structure.
 *	In this case, I could just not have mpelDestroyGCPtr
 *	and have it call Xfree directly.  But I am using this as
 *	an example for now.
 */

void
mpelDestroyGCPriv(pPriv)
pointer pPriv;
{
	Xfree(pPriv);
}

#define WINMOVED(pWin, pGC) \
((pWin->absCorner.x != pGC->lastWinOrg.x) || \
 (pWin->absCorner.y != pGC->lastWinOrg.y))

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
mpelValidateGC(pGC, pQ, changes, pDrawable)
    register GCPtr	pGC;
    GCInterestPtr	pQ;
    register Mask	changes;
    DrawablePtr		pDrawable;
{
    register ppcPrivGCPtr devPriv;
    register int index;	/* used for stepping through bitfields */
    WindowPtr	pWin;
    Mask	bsChanges= 0;
    int		winMoved;
    DDXPointRec	oldOrg;

    devPriv = ((ppcPrivGCPtr) (pGC->devPriv));

    if ( pDrawable->type != devPriv->lastDrawableType ) {
	devPriv->lastDrawableType = pDrawable->type ;
	if (pDrawable->type == DRAWABLE_PIXMAP) {
	    devPriv->FillArea = mfbSolidInvertArea; /* XX (ef) -- huh? mfb?! */
	    pGC->CopyArea = miCopyArea;
	    pGC->PolyFillRect = miPolyFillRect;
	    pGC->PushPixels = miPushPixels;
	    pGC->ImageText8 = miImageText8;
	    pGC->ImageText16 = miImageText16;
	    pGC->PolyText8 = miPolyText8;
	    pGC->PolyText16 = miPolyText16;
	}
	else {
	    devPriv->FillArea = ppcAreaFill;
	    pGC->CopyArea = ppcCopyArea;
	    pGC->PolyFillRect = ppcPolyFillRect;
	    pGC->PushPixels = ppcPushPixels;
	    pGC->ImageText8 = mpelImageText8;
	    pGC->ImageText16 = mpelImageText16;
	    pGC->PolyText8 = mpelPolyText8;
	    pGC->PolyText16 = mpelPolyText16;
	}
	bsChanges |= MIBS_COPYAREA|MIBS_POLYRECTANGLE|MIBS_PUSHPIXELS|
			MIBS_IMAGETEXT8|MIBS_IMAGETEXT16|
			MIBS_POLYTEXT8|MIBS_POLYTEXT16;
	changes = ~0 ;
    }

    if (pDrawable->type == DRAWABLE_PIXMAP) {
	ppcValidateGC( pGC, pQ, changes, pDrawable ) ;
	return;
    }

    changes &= mpelGCInterestValidateMask ;
    /* If Nothing REALLY Changed, Just Return */
    if ( pDrawable->serialNumber == (pGC->serialNumber & DRAWABLE_SERIAL_BITS) )
	if ( !( changes &= ~ GC_CALL_VALIDATE_BIT ) )
	    return ;

    if (pDrawable->type == DRAWABLE_WINDOW)
	pWin = (WindowPtr)pDrawable;
    else /* i.e. pDrawable->type == UNDRAWABLE_WINDOW */
	pWin = (WindowPtr)NULL;

    if (( changes & ( GCClipXOrigin | GCClipYOrigin | GCClipMask|
		      GCSubwindowMode | GC_CALL_VALIDATE_BIT )) ||
	( pDrawable->serialNumber!=(pGC->serialNumber&DRAWABLE_SERIAL_BITS)))
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
	    if (pWin) {
		pGC->lastWinOrg.x = pWin->absCorner.x;
		pGC->lastWinOrg.y = pWin->absCorner.y;
	    }
	    else {
		pGC->lastWinOrg.x = 0;
		pGC->lastWinOrg.y = 0;
	    }
	    (* pGC->pScreen->TranslateRegion)(
		       devPriv->pAbsClientRegion,
		       pGC->lastWinOrg.x + pGC->clipOrg.x,
		       pGC->lastWinOrg.y + pGC->clipOrg.y);
	}

	if (pGC->subWindowMode == IncludeInferiors) {
	    pregWin = NotClippedByChildren(pWin);
	    freeTmpClip = FREE_CC;
	}
	else {
	    pregWin = pWin->clipList;
	    freeTmpClip = REPLACE_CC;
	}
	freeCompClip = devPriv->freeCompClip;

	if (pGC->clientClipType == CT_NONE) {
	    if (freeCompClip == FREE_CC)
		(* pGC->pScreen->RegionDestroy) (devPriv->pCompositeClip);
	    devPriv->pCompositeClip = pregWin;
	    devPriv->freeCompClip = freeTmpClip;
	}
	else {
	     if ((freeTmpClip == FREE_CC) && (freeCompClip == FREE_CC)) {
		    (* pGC->pScreen->Intersect)(
			devPriv->pCompositeClip,
			pregWin,
			devPriv->pAbsClientRegion);
		    (* pGC->pScreen->RegionDestroy)(pregWin);
	    }
	    else if ((freeTmpClip == REPLACE_CC) &&
			(freeCompClip == FREE_CC)) {
		(* pGC->pScreen->Intersect)(
			devPriv->pCompositeClip,
			pregWin,
			devPriv->pAbsClientRegion);
	    }
	    else if ((freeTmpClip == FREE_CC) &&
			 (freeCompClip == REPLACE_CC)) {
		    (* pGC->pScreen->Intersect)(
		       pregWin,
		       pregWin,
		       devPriv->pAbsClientRegion);
		    devPriv->pCompositeClip = pregWin;
	    }
	    else if ((freeTmpClip == REPLACE_CC) &&
			 (freeCompClip == REPLACE_CC)) {
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

    while (changes) {
	index = lowbit(changes);
	changes &= (~index);
	switch ( index ) {

	  case GCLineStyle:
	  case GCLineWidth:
	    if ( pGC->lineWidth ) {
	    	pGC->PolySegment = miPolySegment;
#if !defined(AIXrt)
		pGC->Polylines =
			( pGC->lineStyle == LineSolid )
			? ppcWideLine : miWideDash ;
#else
		if ( pGC->lineStyle == LineSolid)
			pGC->Polylines = ppcWideLine;
		else
			pGC->Polylines = miWideDash;
#endif
		pGC->PolyArc = miPolyArc;
	    }
	    else {
		if ( pGC->lineStyle != LineSolid ) {
			void mpel_do_dashline_gc();

			mpel_do_dashline_gc(pGC);
		}
		else {
	    		pGC->PolySegment = mpelPolySegment;
			pGC->Polylines =  mpelZeroLine;
			pGC->PolyArc = mpelPolyArc;
			((mpelPrivGCPtr) (devPriv->devPriv))->LineType
				= MPEL_SOLIDLINE;
		}
		bsChanges|= MIBS_POLYSEGMENT|MIBS_POLYLINES|MIBS_POLYARC;
	    }
	    changes &= ~( GCLineStyle | GCLineWidth ) ;
	    break;
	  case GCJoinStyle:
#if !defined(AIXrt)
	    pGC->LineHelper =
		( pGC->joinStyle == JoinMiter ) ? miMiter : miNotMiter ;
#else
	if ( pGC->joinStyle == JoinMiter )
	    pGC->LineHelper = miMiter;
	else
	    pGC->LineHelper = miNotMiter;
#endif
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
		if ( fillStyle == FillSolid ) {
		    pGC->FillSpans = ppcSolidWindowFS;
		    pGC->FillPolygon = mpelFillPolygon;
		}
		else {
		    if ( fillStyle == FillStippled ) {
			pGC->FillSpans = ppcStippleWindowFS ;
		    	pGC->FillPolygon = miFillPolygon;
		    }
		    else if ( fillStyle == FillOpaqueStippled ) {
			pGC->FillSpans = ppcOpStippleWindowFS ;
		    	pGC->FillPolygon = miFillPolygon;
		    }
		    else { /*  fillStyle == FillTiled */
			switch (pGC->tile->width) {
				case 1: case 2: case 4: case 8: case 16:
					switch (pGC->tile->height) {
						case 1: case 2: case 4:
						case 8: case 16:
		    					pGC->FillPolygon = 
								mpelTilePolygon;
							break;
						default:
		    					pGC->FillPolygon = 
								miFillPolygon;
							break;
					}
					break;
				default:
		    			pGC->FillPolygon = miFillPolygon;
					break;
				}
			pGC->FillSpans = ppcTileFS;
			}
		}
	    } /* end of new_fill */
	    bsChanges |= MIBS_FILLSPANS|MIBS_FILLPOLYGON;
	    changes &= ~( GCBackground | GCForeground | GCFunction
		     | GCPlaneMask | GCFillStyle ) ;
	    break;

	default:
	    ErrorF("mpelValidateGC: Unexpected GC Change\n") ;
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
