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
#include "apc.h"
#include "Xmd.h"
#include "dixfontstr.h"
#include "fontstruct.h"
#include "pixmapstr.h"
#include "region.h"

#include "mistruct.h"
#include "apcmskbits.h"

#include "apctext.h"

Bool
apcCreateGC(pGC)
    register GCPtr pGC;
{
    GCInterestPtr pQ;

    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;

    /*
     * some of the output primitives aren't really necessary, since they
     * will be filled in ValidateGC because of dix/CreateGC() setting all
     * the change bits.  Others are necessary because although they depend
     * on being a color frame buffer, they don't change 
     */

    pGC->FillSpans = apcSolidFS;
    pGC->SetSpans = apcSetSpans;
    pGC->PutImage = miPutImage;
    pGC->CopyArea = apcCopyArea;
    pGC->CopyPlane = miCopyPlane;
    pGC->PolyPoint = miPolyPoint;
    pGC->Polylines = miZeroLine;
    pGC->PolySegment = miPolySegment;
    pGC->PolyRectangle = miPolyRectangle;
    pGC->PolyArc = miPolyArc;
    pGC->FillPolygon = miFillPolygon;
    pGC->PolyFillRect = miPolyFillRect;
    pGC->PolyFillArc = miPolyFillArc;
    pGC->PolyText8 = miPolyText8;
    pGC->ImageText8 = miImageText8;
    pGC->PolyText16 = miPolyText16;
    pGC->ImageText16 = miImageText16;
    pGC->ImageGlyphBlt = miImageGlyphBlt;
    pGC->PolyGlyphBlt = miPolyGlyphBlt;
    pGC->PushPixels = apcPushPixels;
    pGC->LineHelper = miMiter;
    pGC->ChangeClip = apcChangeClip;
    pGC->DestroyClip = apcDestroyClip;
    pGC->CopyClip = apcCopyClip;

    /* apc wants to translate before scan convesion */
    pGC->miTranslate = 1;

    {
	apcPrivGC  *pPriv;

	pPriv = (apcPrivGC *) Xalloc(sizeof(apcPrivGC));
	if (!pPriv)
	    return FALSE;
	else {
	    pPriv->fExpose = TRUE;
	    pGC->devPriv = (pointer) pPriv;
	    pPriv->pAbsClientRegion = (*pGC->pScreen->RegionCreate) (NULL, 1);
	    pPriv->pCompositeClip = (*pGC->pScreen->RegionCreate) (NULL, 1);
	    pPriv->freeCompClip = REPLACE_CC;
	}
    }
    pGC->devBackingStore = (pointer)NULL;

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
    pQ->ValidateGC = apcValidateGC;
    pQ->ChangeInterestMask = 0;	/* interested in nothing at change time */
    pQ->ChangeGC = (int (*) ()) NULL;
    pQ->CopyGCSource = (void (*) ()) NULL;
    pQ->CopyGCDest = apcCopyGCDest;
    pQ->DestroyGC = apcDestroyGC;
    return TRUE;
}

void
apcDestroyGC(pGC, pQ)
    GC 			*pGC;
    GCInterestPtr	pQ;

{
    apcPrivGC *pPriv;

    /* Most GCInterest pointers would free pQ->devPriv.  This one is privileged
     * and allowed to allocate its private data directly in the GC (this
     * saves an indirection).  We must also unlink and free the pQ.
     */
    pQ->pLastGCInterest->pNextGCInterest = pGC->pNextGCInterest;
    pQ->pNextGCInterest->pLastGCInterest = pGC->pLastGCInterest;

    pPriv = (apcPrivGC *)(pGC->devPriv);
    if (pPriv->freeCompClip == FREE_CC && pPriv->pCompositeClip)
	(*pGC->pScreen->RegionDestroy)(pPriv->pCompositeClip);
    if(pPriv->pAbsClientRegion)
	(*pGC->pScreen->RegionDestroy)(pPriv->pAbsClientRegion);
    Xfree(pGC->devPriv);
    Xfree(pQ);
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
apcValidateGC(pGC, pQ, changes, pDrawable)
    GC *pGC;
    GCInterestPtr	*pQ;
    Mask changes;
    DrawablePtr pDrawable;
{
    WindowPtr   pWin;
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int         xrot, yrot;	/* rotations for tile and stipple pattern */
    int         new_line, new_text, new_tile, new_fillspans;
    int         new_gprfill, new_gprplanemask, new_gprrop;
    apcPrivGCPtr devPriv;
    gpr_$bitmap_desc_t	bitmap_id;
    status_$t   status;      
    int         set_clip = FALSE;
    gpr_$raster_op_array_t ops;
    apDisplayDataPtr    pDisp = &apDisplayData[pGC->pScreen->myNum];
    Mask procChanges = 0;
    int         set_text_font = FALSE;
    gprFIDPtr gfp;

    if (pDrawable->type == DRAWABLE_WINDOW) {
    	pWin = (WindowPtr) pDrawable;
	    bitmap_id = pDisp->display_bitmap;
	}
    else {
	    pWin = (WindowPtr) NULL;
	    bitmap_id = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_desc;
    }

    devPriv = ((apcPrivGCPtr) (pGC->devPriv));
                 
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
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						pregWin,
					      devPriv->pAbsClientRegion);
		}
		else if ((freeTmpClip == FREE_CC) &&
			 (freeCompClip == REPLACE_CC)) {
		    (*pGC->pScreen->Intersect) (
						pregWin,
						pregWin,
					      devPriv->pAbsClientRegion);
		    devPriv->pCompositeClip = pregWin;
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
		devPriv->freeCompClip = FREE_CC;
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

    set_clip = TRUE;   /* also tell gpr about it */
    }

    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_tile = FALSE;
    new_gprfill = FALSE;
    new_gprplanemask = FALSE;
    new_gprrop = FALSE;

    mask = changes;
    while (mask) {
	index = ffs(mask) - 1;
	mask &= ~(index = (1 << index));

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */

        /* new_gprfill  - reset fill and draw value from gc foreground
         * new_gprplanemask - reset planemask from gc mask
         * new_gprrop - reset rops from gc alu
	 */           

	switch (index) {
	case GCFunction:
            new_gprrop = new_text = TRUE;
            break;
	case GCForeground:
	    new_text = TRUE;
	    new_gprfill = TRUE;
	    break;
	case GCPlaneMask:
	    new_gprplanemask = TRUE;
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
            new_tile = TRUE;
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    if (pGC->tile == (PixmapPtr) NULL)
		break;
            new_tile = TRUE;
	    new_fillspans = TRUE;
	    break;
	case GCStipple:
	    if (pGC->stipple == (PixmapPtr) NULL)
	    break;
            new_tile = TRUE;
	    new_fillspans = TRUE;
	    break;
	case GCTileStipXOrigin:
            new_tile = TRUE;
	    break;
	case GCTileStipYOrigin:
            new_tile = TRUE;
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
    case GC_CALL_VALIDATE_BIT:
        new_tile = TRUE;
        new_text = TRUE;
        new_gprfill = TRUE;
        new_gprrop = TRUE;
        new_gprplanemask = TRUE;
        set_clip = TRUE;
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
	pGC->SetSpans = apcSetSpans;
    }

    if (new_line) {
        pGC->PolySegment = miPolySegment; 
	if (pGC->lineStyle == LineSolid)
	{
	    if(pGC->lineWidth == 0)
	    {
	        if (pGC->fillStyle == FillSolid) {
                    pGC->PolySegment = apcPolySegment; 
		    pGC->Polylines = apcZeroLine;
		}
	        else
		    pGC->Polylines = miZeroLine;
	    }
	    else
	    {
		pGC->Polylines = miWideLine;
	    }
	}
	else
            pGC->Polylines = miWideDash;

	procChanges |= MIBS_POLYLINES;

	switch (pGC->joinStyle) {
	case JoinMiter:
	    pGC->LineHelper = miMiter;
	    break;
	case JoinRound:
	case JoinBevel:
	    pGC->LineHelper = miNotMiter;
        }
    }

    if (new_text && (pGC->font)) {

        gfp = (gprFIDPtr) pGC->font->devPriv[pGC->pScreen->myNum];

        pGC->PolyText8 = miPolyText8;

        /* first see if we can gpr it */
        if (gfp &&
            pGC->fillStyle == FillSolid) {

            if (gfp->nGprFonts == 1)
                set_text_font = TRUE;

            pGC->ImageText8 = gprImageText8;
            pGC->ImageText16 = gprImageText16;
            pDisp->noCrsrChk |= NO_CRSR_CHK_ITEXT | NO_CRSR_CHK_PTEXT;   /* assume both */

            devPriv->polyTextVal = pGC->fgPixel;

            switch( pGC->alu ) {
        	  case GXclear:
                devPriv->polyTextVal = 0;
                pGC->PolyText8 = gprPolyText8;
        	    break;
        	  case GXand:
                if (pGC->fgPixel & pGC->planemask == pGC->planemask)
                    pGC->PolyText8 = nopText;
                else if (pGC->fgPixel & pGC->planemask == 0) {
                    devPriv->polyTextVal = 0;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXandReverse:
                if (pGC->fgPixel & pGC->planemask == 0) {
                    devPriv->polyTextVal = 0;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXcopy:
                pGC->PolyText8 = gprPolyText8;
        	    break;
        	  case GXandInverted:
                if (pGC->fgPixel & pGC->planemask == 0)
                    pGC->PolyText8 = nopText;
                else if (pGC->fgPixel & pGC->planemask == pGC->planemask) {
                    devPriv->polyTextVal = 0;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXnoop:
                pGC->PolyText8 = nopText;
        	    break;
        	  case GXxor:
                if (pGC->fgPixel & pGC->planemask == 0)
                    pGC->PolyText8 = nopText;
                /* otherwise mi must do it */
        	    break;
        	  case GXor:
                if (pGC->fgPixel & pGC->planemask == 0)
                    pGC->PolyText8 = nopText;
                else if (pGC->fgPixel & pGC->planemask == pGC->planemask) {
                    devPriv->polyTextVal = pGC->planemask;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXnor:
                if (pGC->fgPixel & pGC->planemask == pGC->planemask) {
                    devPriv->polyTextVal = 0;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXequiv:
                if (pGC->fgPixel & pGC->planemask == pGC->planemask)
                    pGC->PolyText8 = nopText;
                /* otherwise mi must do it */
        	    break;
        	  case GXinvert:
                /* mi must do all */
        	    break;
        	  case GXorReverse:
                if (pGC->fgPixel & pGC->planemask == pGC->planemask) {
                    devPriv->polyTextVal = pGC->planemask;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXcopyInverted:
                devPriv->polyTextVal = ~devPriv->polyTextVal;
                pGC->PolyText8 = gprPolyText8;
        	    break;
        	  case GXorInverted:
                if (pGC->fgPixel & pGC->planemask == pGC->planemask)
                    pGC->PolyText8 = nopText;
                else if (pGC->fgPixel & pGC->planemask == 0) {
                    devPriv->polyTextVal = pGC->planemask;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXnand:
                if (pGC->fgPixel & pGC->planemask == 0) {
                    devPriv->polyTextVal = pGC->planemask;
                    pGC->PolyText8 = gprPolyText8;
                }
                /* otherwise mi must do it */
        	    break;
        	  case GXset:
                devPriv->polyTextVal = pGC->planemask;
                pGC->PolyText8 = gprPolyText8;
            }
        }
        else {  /* definitely using mi all the way */
            pGC->ImageText8 = miImageText8;
            pGC->ImageText16 = miImageText16;
            pDisp->noCrsrChk &= ~(NO_CRSR_CHK_ITEXT | NO_CRSR_CHK_PTEXT);
        }

        if (pGC->PolyText8 == gprPolyText8)
            pGC->PolyText16 = gprPolyText16;
        else if (pGC->PolyText8 == nopText)
            pGC->PolyText16 = nopText;
        else {  /* pGC->PolyText8 == miPolyText8 */
        /* we couldn't manage the polytext with gpr -- do everything the old way */
            pGC->PolyText16 = miPolyText16;
            pDisp->noCrsrChk &= ~NO_CRSR_CHK_PTEXT;
        }

	procChanges |= (MIBS_POLYTEXT8|MIBS_POLYTEXT16|
                    MIBS_IMAGETEXT8|MIBS_IMAGETEXT16);
    }

    if (new_fillspans) {
	switch (pGC->fillStyle) {
	case FillSolid:
	    pGC->FillSpans = apcSolidFS;
        procChanges |= MIBS_FILLSPANS;
	    break;
	case FillTiled:
	    pGC->FillSpans = apcUnnaturalTileFS;
        procChanges |= MIBS_FILLSPANS;
	    if (!pGC->tile)
		FatalError("apcValidateGC: tile mode & no tile\n");
	    if (((DrawablePtr)pGC->tile)->depth != pGC->depth)
		FatalError("apcValidateGC: tile wrong depth\n");
	    break;
	case FillStippled:
	    pGC->FillSpans = apcUnnaturalStippleFS;
        procChanges |= MIBS_FILLSPANS;
	    if (!pGC->stipple)
		FatalError("apcValidateGC: stipple mode & no stipple\n");
	    if (((DrawablePtr)pGC->stipple)->depth != 1)
		FatalError("apcValidateGC: stipple wrong depth\n");
	    break;
	case FillOpaqueStippled:
	    if (pGC->fgPixel == pGC->bgPixel) {
		pGC->FillSpans = apcSolidFS;
         procChanges |= MIBS_FILLSPANS;
	    }
         else
        {
		pGC->FillSpans = apcUnnaturalOpaqueStippleFS;
         procChanges |= MIBS_FILLSPANS;
		if (!pGC->stipple)
		    FatalError("apcValidateGC: stipple mode & no stipple\n");
		if (((DrawablePtr)pGC->stipple)->depth != 1)
		    FatalError("apcValidateGC: stipple wrong depth\n");
	    }
	    break;
	default:
	    FatalError("apcValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    /*
     * If this GC has ever been used with a window with backing-store enabled,
     * we must call miVaidateBackingStore to keep the backing-store module
     * up-to-date, should this GC be used with that drawable again. In addition,
     * if the current drawable is a window and has backing-store enabled, we
     * also call miValidateBackingStore to give it a chance to get its hooks in.
     */
    if (pGC->devBackingStore ||
	(pWin && (pWin->backingStore != NotUseful)))
    {
	miValidateBackingStore(pDrawable, pGC, procChanges);
    }

    /* to understand the following, read the story in apValidateGC */
    /* observe that if this *is* a shadowGC then the first if clause will always be false */
    if (pGC != pDisp->lastShadowGC) {
        if (pDisp->lastGC)
        {
            pDisp->lastGC->serialNumber |= GC_CHANGE_SERIAL_BIT;  /* set high bit to force revalidate */
            pDisp->lastShadowGC->serialNumber |= GC_CHANGE_SERIAL_BIT;  /* in both GCs */
        }
        pDisp->lastShadowGC = pGC;
    }

    /* if the serial number of this gc and lastgc validated are the
     * same, then we just have to validate changes.  Else we have to
     * completely validate all attributes of a NEW GC.  This means
     * set ALL gpr attributes being used.  Following is a list of
     * all gpr attributes being used.
     *
     *          gpr_$set_bitmap
     *		gpr_$set_fill_value
     *		gpr_$set_draw_value
     *          gpr_$set_raster_op_mask
     *          gpr_$set_plane_mask_32
     */

    apc_$set_bitmap( bitmap_id);

    /* deal with the changes we've collected */

    if (set_clip) {
        /* if there is only one clip window, give it to gpr now */
        if (devPriv->pCompositeClip->numRects == 1) {
            gpr_$window_t gwin;
            gwin.window_base.x_coord = devPriv->pCompositeClip->rects->x1;
            gwin.window_base.y_coord = devPriv->pCompositeClip->rects->y1;
            gwin.window_size.x_size = devPriv->pCompositeClip->rects->x2 - gwin.window_base.x_coord;
            gwin.window_size.y_size = devPriv->pCompositeClip->rects->y2 - gwin.window_base.y_coord;
            gpr_$set_clip_window( gwin, status );
        }
    }

    if (new_gprfill) {
        gpr_$set_fill_value(pGC->fgPixel, status);
        gpr_$set_draw_value(pGC->fgPixel, status);
        }

    if (new_gprrop) 
        gpr_$set_raster_op_mask( (gpr_$mask_32_t)0xFFFFFFFF, (gpr_$raster_op_t)pGC->alu, status);

    if (new_gprplanemask)
        gpr_$set_plane_mask_32( pGC->planemask, status);

    if ( set_text_font )
        gpr_$set_text_font( gfp->fontIds[0], status );

    if (new_tile) apcValidateTile( pDrawable, pGC );
}

void
apcDestroyClip(pGC)
    GCPtr	pGC;
{
    if(pGC->clientClipType == CT_NONE)
	return;
    (*pGC->pScreen->RegionDestroy)(pGC->clientClip);
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;
    pGC->stateChanges |= (GCClipXOrigin | GCClipYOrigin | GCClipMask);
}

void
apcChangeClip(pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    apcDestroyClip(pGC);
    if(type == CT_PIXMAP)
    {
	pGC->clientClip = (pointer) mfbPixmapToRegion(pvalue);
	(*pGC->pScreen->DestroyPixmap)(pvalue);
    }
    else if (type == CT_REGION) {
	pGC->clientClip = (pointer) (*pGC->pScreen->RegionCreate)( NULL, 0 );
	(*pGC->pScreen->RegionCopy)( pGC->clientClip, pvalue );
    }
    else if (type != CT_NONE)
    {
	pGC->clientClip = (pointer) miRectsToRegion(pGC, nrects, pvalue, type);
	Xfree(pvalue);
    }
    pGC->clientClipType = (pGC->clientClip) ? CT_REGION : CT_NONE;
    pGC->stateChanges |= (GCClipXOrigin | GCClipYOrigin | GCClipMask);
}

void
apcCopyClip (pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    RegionPtr prgnNew;

    switch(pgcSrc->clientClipType)
    {
      case CT_NONE:
      case CT_PIXMAP:
        apcChangeClip(pgcDst, pgcSrc->clientClipType, pgcSrc->clientClip, 0);
        break;
      case CT_REGION:
        prgnNew = (*pgcSrc->pScreen->RegionCreate)(NULL, 1);
        (*pgcSrc->pScreen->RegionCopy)(prgnNew,
                                       (RegionPtr)(pgcSrc->clientClip));
        apcChangeClip(pgcDst, CT_REGION, prgnNew, 0);
        break;
    }
}

void
apcCopyGCDest (pGC, pQ, changes, pGCSrc)
    GCPtr	pGC;
    GCInterestPtr	pQ;
    Mask 		changes;
    GCPtr		pGCSrc;
{
    RegionPtr		pClip;

    if(changes & GCClipMask)
    {
	if(pGC->clientClipType == CT_PIXMAP)
	{
	    ((PixmapPtr)pGC->clientClip)->refcnt++;
	}
	else if(pGC->clientClipType == CT_REGION)
	{
	    BoxRec pixbounds;

	    pixbounds.x1 = 0;
	    pixbounds.y1 = 0;
	    pixbounds.x2 = 0;
	    pixbounds.y2 = 0;

	    pClip = (RegionPtr) pGC->clientClip;
	    pGC->clientClip =
	        (pointer)(* pGC->pScreen->RegionCreate)(&pixbounds, 1);
	    (* pGC->pScreen->RegionCopy)(pGC->clientClip, pClip);
	}
    }
}
