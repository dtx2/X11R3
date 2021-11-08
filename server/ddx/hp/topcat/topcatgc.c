/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
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
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "dixfontstr.h"
#include "fontstruct.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "../cfb/cfb.h"
#include "mistruct.h"

#include "topcat.h"

#include "shadow.h"

#include "../cfb/cfbmskbits.h"

#include "mibstore.h"	/* for the MIBS masks */

extern cfbXRotatePixmap();
extern cfbYRotatePixmap();
extern void mfbPushPixels();
extern void hpfbPolyFillRect();
extern void hpZeroDash();
extern void topcatZeroDash();
extern void tcImageGlyphBlt();
extern void tcImageOptText();
extern void tcImageText8();
extern void tcsolidfs();
extern void tcpolyfillstippledrect();
void topcatValidateGC();

Bool
topcatCreateGC(pGC)
    register GCPtr pGC;
{
    GCInterestPtr pQ;

    switch (pGC->depth) {
    case 1:
	mfbCreateGC(pGC);
 	pGC->CopyArea = hpccopyarea;
	return;
    case PSZ:
	break;
    default:
	ErrorF("topcatCreateGC: unsupported depth: %d\n", pGC->depth);
	return FALSE;
    }
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;

    /*
     * some of the output primitives aren't really necessary, since they
     * will be filled in ValidateGC because of dix/CreateGC() setting all
     * the change bits.  Others are necessary because although they depend
     * on being a color frame buffer, they don't change 
     */

    pGC->FillSpans = cfbsolidfs;	/* cfbSolidFS; */
    pGC->SetSpans = cfbsetspans;	/* cfbSetSpans; */
    pGC->PutImage = tcputimage;		/* tcPutImage; */
    pGC->CopyArea = hpccopyarea;
    pGC->CopyPlane = tccopyplane;	/* miCopyPlane; */
    pGC->PolyPoint = mipolypoint;	/* miPolyPoint; */
    pGC->Polylines = mizeroline;    	/* mizeroline */
    pGC->PolySegment = mipolysegment;	/* miPolySegment; */
    pGC->PolyRectangle = mipolyrectangle;	/* miPolyRectangle; */
    pGC->PolyArc = mipolyarc;		/* miPolyArc; */
    pGC->FillPolygon = mifillpolygon;	/* miFillPolygon; */
    pGC->PolyFillRect = hpfbpolyfillrect;	/* hpfbPolyFillRect; */
    pGC->PolyFillArc = mipolyfillarc;	/* miPolyFillArc; */
    pGC->PolyText8 = mipolytext8;	/* miPolyText8; */
    pGC->ImageText8 = miimagetext8;	/* miImageText8; */
    pGC->PolyText16 = mipolytext16;	/* miPolyText16; */
    pGC->ImageText16 = miimagetext16;	/* miImageText16; */
    pGC->ImageGlyphBlt = miimageglyphblt;	/* miImageGlyphBlt; */
    pGC->PolyGlyphBlt = mipolyglyphblt;	/* miPolyGlyphBlt; */
    pGC->PushPixels = mfbpushpixels;	/* mfbPushPixels /* but mfbPushPixels isn't depth
					 * dependent */
    pGC->LineHelper = miMiter;
    pGC->ChangeClip = cfbChangeClip;
    pGC->DestroyClip = cfbDestroyClip;
    pGC->CopyClip = cfbCopyClip;

    pGC->devBackingStore = (pointer)NULL;

    /* cfb wants to translate before scan convesion */
    pGC->miTranslate = 1;

    {
	cfbPrivGC  *pPriv;

	pPriv = (cfbPrivGC *) Xalloc(sizeof(cfbPrivGC));
	if (!pPriv)
	    return FALSE;
	else {
	    pPriv->rop = pGC->alu;
	    pPriv->fExpose = TRUE;
	    pGC->devPriv = (pointer) pPriv;
	    pPriv->pRotatedTile = NullPixmap;
	    pPriv->pRotatedStipple = NullPixmap;
	    pPriv->pAbsClientRegion = (*pGC->pScreen->RegionCreate) (NULL, 1);
	    pPriv->pCompositeClip = (*pGC->pScreen->RegionCreate) (NULL, 1);
	    pPriv->freeCompClip = REPLACE_CC;
	}
    }
    pQ = (GCInterestPtr) Xalloc(sizeof(GCInterestRec));
    if (!pQ) {
	Xfree(pGC->devPriv);
	return FALSE;
    }

    /* Now link this device into the GCque */
    pGC->pNextGCInterest = pQ;
    pGC->pLastGCInterest = pQ;
    pQ->pNextGCInterest = (GCInterestPtr) & pGC->pNextGCInterest;
    pQ->pLastGCInterest = (GCInterestPtr) & pGC->pNextGCInterest;
    pQ->length = sizeof(GCInterestRec);
    pQ->owner = 0;		/* server owns this */
    pQ->ValInterestMask = ~0;	/* interested in everything at validate
				 * time */
    pQ->ValidateGC = topcatValidateGC;
    pQ->ChangeInterestMask = 0;	/* interested in nothing at change time */
    pQ->ChangeGC = (int (*) ()) NULL;
    pQ->CopyGCSource = (void (*) ()) NULL;
    pQ->CopyGCDest = cfbCopyGCDest;
    pQ->DestroyGC = cfbDestroyGC;
    return TRUE;
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
topcatValidateGC(pGC, pQ, changes, pDrawable)
    register GC *pGC;
    GCInterestPtr	*pQ;
    Mask changes;
    DrawablePtr pDrawable;
{
    WindowPtr   pWin;
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int         xrot, yrot;	/* rotations for tile and stipple pattern */
    Bool        fRotate = FALSE;/* True if rotated pixmaps are needed */
    int         new_line, new_text, new_fillspans, new_fillrects;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    unsigned long int BS_mask = 0;

    switch (pGC->depth) {
    case PSZ:
	break;
    case 1:
	if (pDrawable->type == DRAWABLE_PIXMAP) {
	    mfbValidateGC(pGC, pQ, changes, pDrawable);
 	    pGC->CopyArea = hpccopyarea;
	    return;
	}
	/* WARNING - FALL THROUGH */
    default:
	ErrorF("topcatCreateGC: unsupported depth: %d\n", pGC->depth);
	return;
    }

    if (pDrawable->type == DRAWABLE_WINDOW)
	pWin = (WindowPtr) pDrawable;
    else
	pWin = (WindowPtr) NULL;

    devPriv = ((cfbPrivGCPtr) (pGC->devPriv));

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin | GCClipYOrigin | GCClipMask)) ||
	(changes & GCSubwindowMode) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
	) {

	/*
	 * if there is a client clip (always a region, for us) AND it has
	 * moved or is different OR the window has moved we need to
	 * (re)translate it. 
	 */
	if ((pGC->clientClipType == CT_REGION) &&
	    ((changes & (GCClipXOrigin | GCClipYOrigin | GCClipMask)) ||
	     (pWin && WINMOVED(pWin, pGC))
	     )
	    ) {
	    /* retranslate client clip */
	    (*pGC->pScreen->RegionCopy) (devPriv->pAbsClientRegion,
					 pGC->clientClip);

	    if (pWin) {
		pGC->lastWinOrg.x = pWin->absCorner.x;
		pGC->lastWinOrg.y = pWin->absCorner.y;
		(*pGC->pScreen->TranslateRegion) (
					       devPriv->pAbsClientRegion,
				      pGC->lastWinOrg.x + pGC->clipOrg.x,
				     pGC->lastWinOrg.y + pGC->clipOrg.y);
	    }
	    else {
		pGC->lastWinOrg.x = 0;
		pGC->lastWinOrg.y = 0;
		(*pGC->pScreen->TranslateRegion) (
						  devPriv->pAbsClientRegion, pGC->clipOrg.x, pGC->clipOrg.y);
	    }
	}

	if (pWin) {
	    RegionPtr   pregWin;
	    int         freeTmpClip, freeCompClip;

	    if (pGC->subWindowMode == IncludeInferiors) {
		pregWin = NotClippedByChildren(pWin);
		freeTmpClip = FREE_CC;
	    }
	    else {
		pregWin = pWin->clipList;
		freeTmpClip = REPLACE_CC;
	    }
	    freeCompClip = devPriv->freeCompClip;

	    /*
	     * if there is no client clip, we can get by with just keeping
	     * the pointer we got, and remembering whether or not should
	     * destroy (or maybe re-use) it later.  this way, we avoid
	     * unnecessary copying of regions.  (this wins especially if
	     * many clients clip by children and have no client clip.) 
	     */
	    if (pGC->clientClipType == CT_NONE) {
		if (freeCompClip == FREE_CC) {
		    (*pGC->pScreen->RegionDestroy) (devPriv->pCompositeClip);
		}
		devPriv->pCompositeClip = pregWin;
		devPriv->freeCompClip = freeTmpClip;
	    }
	    else {
		/*
		 * we need one 'real' region to put into the composite
		 * clip. if pregWin the current composite clip are real,
		 * we can get rid of one. if pregWin is real and the
		 * current composite clip isn't, use pregWin for the
		 * composite clip. if the current composite clip is real
		 * and pregWin isn't, use the current composite clip. if
		 * neither is real, create a new region. 
		 */

		if ((freeTmpClip == FREE_CC) && (freeCompClip == FREE_CC)) {
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						pregWin,
					      devPriv->pAbsClientRegion);
		    (*pGC->pScreen->RegionDestroy) (pregWin);
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			 (freeCompClip == FREE_CC)) {
		    devPriv->pCompositeClip = pregWin;
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						devPriv->pCompositeClip,
					      devPriv->pAbsClientRegion);
		}
		else if ((freeTmpClip == FREE_CC) &&
			 (freeCompClip == REPLACE_CC)) {
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						pregWin,
					      devPriv->pAbsClientRegion);
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			 (freeCompClip == REPLACE_CC)) {
		    devPriv->pCompositeClip =
			(*pGC->pScreen->RegionCreate) (NULL, 1);
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						pregWin,
					      devPriv->pAbsClientRegion);
		}
	    }
	}			/* end of composite clip for a window */
	else {
	    BoxRec      pixbounds;

	    pixbounds.x1 = 0;
	    pixbounds.y1 = 0;
	    pixbounds.x2 = ((PixmapPtr) pDrawable)->width;
	    pixbounds.y2 = ((PixmapPtr) pDrawable)->height;

	    if (devPriv->freeCompClip == FREE_CC)
		(*pGC->pScreen->RegionReset) (
				    devPriv->pCompositeClip, &pixbounds);
	    else {
		devPriv->freeCompClip = FREE_CC;
		devPriv->pCompositeClip =
		    (*pGC->pScreen->RegionCreate) (&pixbounds, 1);
	    }

	    if (pGC->clientClipType == CT_REGION)
		(*pGC->pScreen->Intersect) (
					    devPriv->pCompositeClip,
					    devPriv->pCompositeClip,
					    devPriv->pAbsClientRegion);
	}			/* end of composute clip for pixmap */
    }

/*
    if (pWin) {

	*
	 * rotate tile patterns so that pattern can be combined in word by
	 * word, but the pattern seems to begin aligned with the window 
	 *
	xrot = pWin->absCorner.x;
	yrot = pWin->absCorner.y;
    }
    else {
*/
	yrot = 0;
	xrot = 0;
/*
    }
*/


    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fillrects = FALSE;

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */
	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_text = TRUE;
	    break;
	case GCPlaneMask:
	    break;
	case GCBackground:
	    new_fillspans = TRUE;
	    break;
	case GCLineStyle:
	case GCLineWidth:
	case GCCapStyle:
	case GCJoinStyle:
	    new_line = TRUE;
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    new_fillrects = TRUE;
	    break;
	case GCFillRule:
	    new_fillrects = TRUE;
	    break;
	case GCTile:
	    if (pGC->tile == (PixmapPtr) NULL)
		break;
	    cfbPadPixmap(pGC->tile);
	    fRotate = TRUE;
	    new_fillspans = TRUE;
	    new_fillrects = TRUE;
	    break;

	case GCStipple:
	    if (pGC->stipple == (PixmapPtr) NULL)
		break;
	    cfbPadPixmap(pGC->stipple);
	    fRotate = TRUE;
	    new_fillspans = TRUE;
	    new_fillrects = TRUE;
	    break;

	case GCTileStipXOrigin:
	    fRotate = TRUE;
	    break;

	case GCTileStipYOrigin:
	    fRotate = TRUE;
	    break;

	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	    break;
	case GCGraphicsExposures:
	    break;
	case GCClipXOrigin:
	    break;
	case GCClipYOrigin:
	    break;
	case GCClipMask:
	    break;
	case GCDashOffset:
	    break;
	case GCDashList:
	    break;
	case GCArcMode:
	    break;
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  check its depth & ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
	new_fillspans = TRUE;	/* deal with FillSpans later */
	new_fillrects = TRUE;
	new_text = TRUE;
	new_line = TRUE;
	BS_mask |= MIBS_SETSPANS;
	pGC->SetSpans = cfbsetspans;	/* cfbSetSpans; */
    }

    /* deal with the changes we've collected */

    if (new_line) {
	if (pGC->lineWidth == 0) {
	    BS_mask |= MIBS_POLYLINES;
	    if(pWin) {
	      if(pGC->fillStyle == FillSolid) {
	        if (pGC->lineStyle == LineSolid) 
	          pGC->Polylines = topcatzeroline;	/* topcatZeroLine; */
	        else
		    pGC->Polylines = topcatzerodash;	/* topcatZeroDash; */
	      }
	      else {
		if(pGC->lineStyle == LineSolid)
		  pGC->Polylines = mizeroline;
		else
		  pGC->Polylines = hpzerodash;
	      }
	    }
	    else {
	      /*
	       * we're drawing to a pixmap, not a window
	       */
	      if(pGC->lineStyle == LineSolid) {
	        pGC->Polylines = miZeroLine;
	      }
	      else {
		pGC->Polylines = hpZeroDash;
	      }
	    }
	}
	else {
	    switch (pGC->lineStyle) {
	      case LineSolid :
		BS_mask |= MIBS_POLYLINES;
		pGC->Polylines = miwideline;	/* miWideLine; */
		break;
	      case LineOnOffDash:
	      case LineDoubleDash:
		BS_mask |= MIBS_POLYLINES;
		pGC->Polylines = miwidedash;	/* miWideDash; */
		break;
	    }
	    switch (pGC->joinStyle) {
	      case JoinMiter:
		pGC->LineHelper = miMiter;
		break;
	      case JoinRound:
	      case JoinBevel:
		pGC->LineHelper = miNotMiter;
		break;
	    }
	}
    }

    if (new_text && pGC->font) {
      BS_mask |= MIBS_IMAGEGLYPHBLT | MIBS_IMAGETEXT8 | MIBS_POLYGLYPHBLT;
      if(pWin) {
        pGC->ImageGlyphBlt = tcimageglyphblt;	/* tcImageGlyphBlt; */
	/*
	 * if the font has been optimized, then select the appropriate
	 * ImageText routine based on whether or not the font is fixed-width
	 */
        if(pGC->font->devPriv[pGC->pScreen->myNum]) {
	  if(pGC->font->pFI->terminalFont) {
            pGC->ImageText8 = tcImageOptText;  /*XXX should have an 8 and 16 */
	  }
	  else { pGC->ImageText8 = tcImageVarText; }
          pGC->PolyGlyphBlt = mipolyglyphblt;  /* miPolyGlyphBlt */
	  pGC->PolyText8 = tcPolyOptText; BS_mask |= MIBS_POLYTEXT8;
        }
	else {
	  /*
	   * un-optimized font, blit from main memory
	   */
	  pGC->ImageText8 = miimagetext8;
	  pGC->PolyGlyphBlt = miPolyGlyphBlt;
	}
      }
      else {
        pGC->ImageText8 = miImageText8;
        pGC->PolyGlyphBlt = miPolyGlyphBlt;
        pGC->ImageGlyphBlt = miImageGlyphBlt;
      }
    }

    if (new_fillspans) {
	BS_mask |= MIBS_FILLSPANS;
	switch (pGC->fillStyle) {
	case FillSolid:
	    pGC->FillSpans = cfbsolidfs;	/* cfbSolidFS; */
	    if (pDrawable->type == DRAWABLE_WINDOW)
	      pGC->FillSpans = tcsolidfs;	/* tcSolidFS; */
	    break;
	case FillTiled:
	    pGC->FillSpans = cfbunnaturaltilefs;     /* cfbUnnaturalTileFS; */
	    if (!pGC->tile)
		FatalError("topcatValidateGC: tile mode & no tile\n");
	    if (((DrawablePtr)pGC->tile)->depth != pGC->depth)
		FatalError("topcatValidateGC: tile wrong depth\n");
	    break;
	case FillStippled:
	    pGC->FillSpans = cfbunnaturalstipplefs; /* cfbUnnaturalStippleFS; */
	    if (!pGC->stipple)
		FatalError("topcatValidateGC: stipple mode & no stipple\n");
	    if (((DrawablePtr)pGC->stipple)->depth != 1)
		FatalError("topcatValidateGC: stipple wrong depth\n");
	    break;
	case FillOpaqueStippled:
	    if (pGC->fgPixel == pGC->bgPixel)
		pGC->FillSpans = cfbsolidfs;	/* cfbSolidFS; */
	    else {
		pGC->FillSpans = cfbunnaturalstipplefs;	/* cfbUnnaturalStippleFS; */
		if (!pGC->stipple)
		    FatalError("topcatValidateGC: stipple mode & no stipple\n");
		if (((DrawablePtr)pGC->stipple)->depth != 1)
		    FatalError("topcatValidateGC: stipple wrong depth\n");
	    }
	    break;
	default:
	    FatalError("topcatValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (xrot || yrot || fRotate) {
	/*
	 * First destroy any previously-rotated tile/stipple
	 */
	if (devPriv->pRotatedTile) {
	    cfbDestroyPixmap(devPriv->pRotatedTile);
	    devPriv->pRotatedTile = (PixmapPtr)NULL;
	}
	if (devPriv->pRotatedStipple) {
	    cfbDestroyPixmap(devPriv->pRotatedStipple);
	    devPriv->pRotatedStipple = (PixmapPtr)NULL;
	}
	if (pGC->tile &&
	    (devPriv->pRotatedTile = cfbCopyPixmap(pGC->tile))
		== (PixmapPtr) NULL)
	    FatalError("topcatValidateGC: cannot rotate tile\n");
	if (pGC->stipple && 
	    (devPriv->pRotatedStipple = cfbCopyPixmap(pGC->stipple))
		== (PixmapPtr) NULL)
	    FatalError("topcatValidateGC: cannot rotate stipple\n");
	/*
	 * If we've gotten here, we're probably going to rotate the tile
	 * and/or stipple, so we have to add the pattern origin into
	 * the rotation factor, even if it hasn't changed.
	 */
	xrot += pGC->patOrg.x;
	yrot += pGC->patOrg.y;
	if (xrot) {
	    if (pGC->tile && devPriv->pRotatedTile)
		cfbXRotatePixmap(devPriv->pRotatedTile, xrot);
	    if (pGC->stipple && devPriv->pRotatedStipple)
		cfbXRotatePixmap(devPriv->pRotatedStipple, xrot);
	}
	if (yrot) {
	    if (pGC->tile && devPriv->pRotatedTile)
		cfbYRotatePixmap(devPriv->pRotatedTile, yrot);
	    if (pGC->stipple && devPriv->pRotatedStipple)
		cfbYRotatePixmap(devPriv->pRotatedStipple, yrot);
	}
    }

    if (new_fillrects) {

	BS_mask |= MIBS_POLYFILLRECT;
	pGC->PolyFillRect = hpfbpolyfillrect; /* hpfbPolyFillRect */

	if ((pDrawable->type == DRAWABLE_PIXMAP) &&
	     (((PixmapPtr)(pDrawable))->devKind != PIXMAP_FRAME_BUFFER)) {
	  pGC->PolyFillRect = mipolyfillrect;	/* miPolyFillRect; */
	  hpValidateBS(pGC, pQ, changes, pDrawable,BS_mask);
	  return;
	}
		/* if got here its a window or frame buffer pixmap */
	if(pGC->fillStyle == FillSolid)
	{
	  pGC->PolyFillRect = tcpolyfillsolidrect;
	  hpValidateBS(pGC, pQ, changes, pDrawable,BS_mask);
	  return;
	}

	if(pGC->fillStyle != FillTiled)
	{
	  pGC->PolyFillRect = tcpolyfillstippledrect;
	  hpValidateBS(pGC, pQ, changes, pDrawable,BS_mask);
	  return;
	}

	/* If we're here, fillStyle is tiled */
	if (pGC->tile->devKind != PIXMAP_FRAME_BUFFER ||
	    pDrawable->pScreen != pGC->tile->drawable.pScreen)
	  pGC->PolyFillRect = mipolyfillrect;	/* miPolyFillRect; */
    } /* end of new_fillrects */

  hpValidateBS(pGC, pQ, changes, pDrawable,BS_mask);
}
