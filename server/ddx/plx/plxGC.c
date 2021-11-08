/*
 *   Copyright (c) 1987, 88 by
 *   PARALLAX GRAPHICS, INCORPORATED, Santa Clara, California.
 *   All rights reserved
 *
 *   This software is furnished on an as-is basis, and may be used and copied
 *   only with the inclusion of the above copyright notice.
 *
 *   The information in this software is subject to change without notice.
 *   No committment is made as to the usability or reliability of this
 *   software.
 *
 *   Parallax Graphics, Inc.
 *   2500 Condensa Street
 *   Santa Clara, California  95051
 */

#ifndef lint
static char *sid_ = "@(#)plxGC.c	1.19 09/01/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

#include	"mistruct.h"
#ifndef X11R2
#include	"mibstore.h"
#endif /* not X11R2 */

Bool
plxCreateGC(pGC)
register GCPtr pGC;
{
	plxPrivGC  *pPriv;
	GCInterestPtr pQ;

	ifdebug(11) printf("plxCreateGC()\n");

	pGC->clientClip = NULL;
	pGC->clientClipType = CT_NONE;

#ifndef X11R2
	pGC->devBackingStore = NULL;
#endif /* not X11R2 */

	/* some of the output primitives aren't really necessary, since
	 * they will be filled in ValidateGC because of dix/CreateGC()
	 * setting all the change bits.
	 */
	pGC->FillSpans = plxSolidFS;
	pGC->SetSpans = plxSetSpans;
	pGC->PutImage = plxPutImage;
	pGC->PushPixels = plxPushPixels;
	pGC->CopyArea = plxCopyArea;
	pGC->CopyPlane = plxCopyPlane;
	pGC->PolyPoint = plxPolyPoint;

	pGC->Polylines = plxZeroPolylines;
	pGC->PolySegment = plxZeroPolySegment;
	pGC->PolyRectangle = plxZeroPolyRectangle;

	pGC->FillPolygon = plxFillPolygon;
	pGC->PolyFillRect = plxPolyFillRect;

	pGC->PolyArc = miPolyArc;
	pGC->PolyFillArc = miPolyFillArc;

	pGC->PolyText8 = miPolyText8;
	pGC->ImageText8 = miImageText8;
	pGC->PolyText16 = miPolyText16;
	pGC->ImageText16 = miImageText16;
	pGC->ImageGlyphBlt = plxImageGlyphBlt;
	pGC->PolyGlyphBlt = plxPolyGlyphBlt;

	pGC->LineHelper = miMiter;

	pGC->ChangeClip = plxChangeClip;
	pGC->CopyClip = plxCopyClip;
	pGC->DestroyClip = plxDestroyClip;

	pGC->miTranslate = 1;

	pPriv = (plxPrivGC *)Xalloc(sizeof(plxPrivGC));
	if (!pPriv) {
		return FALSE;
	}
	pGC->devPriv = (pointer)pPriv;
	pPriv->pAbsClientRegion = (* pGC->pScreen->RegionCreate)(NULL, 1);
	pPriv->pCompositeClip = (RegionPtr)NULL;
	pPriv->freeCompClip = REPLACE_CC;
	pPriv->plxStipple = NullPixmap;

	pQ = (GCInterestPtr) Xalloc(sizeof(GCInterestRec));
	if(!pQ) {
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
	pQ->ValInterestMask = ~0;	/* interested in everything at validate time */
	pQ->ValidateGC = plxValidateGC;
	pQ->ChangeInterestMask = 0; /* interested in nothing at change time */
	pQ->ChangeGC = (int (*) () ) NULL;
	pQ->CopyGCSource = (void (*) () ) NULL;

	pQ->CopyGCDest = plxCopyGCDest;
	pQ->DestroyGC = plxDestroyGC;

	return TRUE;
}

void
plxDestroyGC(pGC, pQ)
GC *pGC;
GCInterestPtr pQ;
{
	plxPrivGC *pPriv;

	ifdebug(11) printf("plxDestroyGC()\n");

	/* Most GCInterest pointers would free pQ->devPriv.  This one
	 * is privileged * and allowed to allocate its private data
	 * directly in the GC (this * saves an indirection).
	 * We must also unlink and free the pQ.
	 */
	pQ->pLastGCInterest->pNextGCInterest = pGC->pNextGCInterest;
	pQ->pNextGCInterest->pLastGCInterest = pGC->pLastGCInterest;

	pPriv = (plxPrivGC *)(pGC->devPriv);

	if (pPriv->freeCompClip == FREE_CC && pPriv->pCompositeClip)
		(*pGC->pScreen->RegionDestroy)(pPriv->pCompositeClip);
	if(pPriv->pAbsClientRegion)
		(*pGC->pScreen->RegionDestroy)(pPriv->pAbsClientRegion);
	if(pPriv->plxStipple)
		 (*pGC->pScreen->DestroyPixmap)(pPriv->plxStipple);
	Xfree(pGC->devPriv);
	Xfree(pQ);
}

#define WINMOVED(pWindow, pGC) \
((pWindow->absCorner.x != pGC->lastWinOrg.x) || \
 (pWindow->absCorner.y != pGC->lastWinOrg.y))

/* Clipping conventions
 *	if the drawable is a window
 *	    CT_REGION ==> pCompositeClip really is the composite
 *	    CT_other ==> pCompositeClip is the window clip region
 *	if the drawable is a pixmap
 *	    CT_REGION ==> pCompositeClip is the translated client region
 *		clipped to the pixmap boundary
 *	    CT_other ==> pCompositeClip is the pixmap bounding box
 */

/*
 * from plxClip.c
 */
extern RegionPtr plxclipcurrentregion;

void
plxValidateGC(pGC, pQ, changes, pDrawable)
register GC *pGC;
GCInterestPtr *pQ;
Mask changes;
DrawablePtr pDrawable;
{
	register WindowPtr pWindow;
	int state_mask;		/* stateChanges */
	int index;		/* used for stepping through bitfields */
	int xrot, yrot;		/* rotations for tile and stipple pattern */
	Bool new_rotation;	/* True if rotated pixmaps are needed */
	Bool new_stipple;	/* True if stipple colors chnage */
	int new_line, new_text;
	int new_fillspans;	/* flags for changing the proc vector */
	plxPrivGCPtr pPriv;
#ifndef X11R2
	int procChanges = 0;
#endif /* not X11R2 */

	ifdebug(11) printf("plxValidateGC()\n");

	if (pDrawable->type == DRAWABLE_WINDOW) {
		pWindow = (WindowPtr)pDrawable;
		/* Temporary - we should eventually offer 1-bit too. */
		if (pDrawable->depth != 8)
			FatalError("plx only supports 8-bit windows\n");
	} else {
		pWindow = (WindowPtr)NULL;
	}

	pPriv = ((plxPrivGCPtr)(pGC->devPriv));

	/*
	 * if the client clip is different or moved OR
	 * the subwindowMode has changed OR
	 * the window's clip has changed since the last validation
	 * we need to recompute the composite clip
	 */

	if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask)) ||
	    (changes & GCSubwindowMode) ||
	    (pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))) {

		/*
		 * if there is a client cliplist (always a region, for us) AND
		 * it has moved or is different OR
		 * the window has moved
		 * we need to (re)translate it.
		 */
		if ((pGC->clientClipType == CT_REGION) &&
		    ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask)) ||
		    (pWindow && WINMOVED(pWindow, pGC)))) {
			/* retranslate client clip */
			(* pGC->pScreen->RegionCopy)(pPriv->pAbsClientRegion, pGC->clientClip);

			if (pWindow) {
				pGC->lastWinOrg.x = pWindow->absCorner.x;
				pGC->lastWinOrg.y = pWindow->absCorner.y;
				(* pGC->pScreen->TranslateRegion)(pPriv->pAbsClientRegion, pGC->lastWinOrg.x + pGC->clipOrg.x, pGC->lastWinOrg.y + pGC->clipOrg.y);
			} else {
				pGC->lastWinOrg.x = 0;
				pGC->lastWinOrg.y = 0;
				(* pGC->pScreen->TranslateRegion)(pPriv->pAbsClientRegion, pGC->clipOrg.x, pGC->clipOrg.y);
			}
		}

		if (pWindow) {
			RegionPtr pregWin;
			int freeTmpClip, freeCompClip;

			if (pGC->subWindowMode == IncludeInferiors) {
				pregWin = NotClippedByChildren(pWindow);
				freeTmpClip = FREE_CC;
			}
			else {
				pregWin = pWindow->clipList;
				freeTmpClip = REPLACE_CC;
			}

			freeCompClip = pPriv->freeCompClip;

			/*
			 * if there is no client clip, we can get by with
			 * just keeping the pointer we got, and remembering
			 * whether or not should destroy (or maybe re-use)
			 * it later.  this way, we avoid unnecessary copying
			 * of regions.  (this wins especially if many
			 * clients clip by children and have no client
			 * clip.)
			 */
			if (pGC->clientClipType == CT_NONE) {
				if(freeCompClip == FREE_CC)  {
					(* pGC->pScreen->RegionDestroy) (pPriv->pCompositeClip);
				}
				pPriv->pCompositeClip = pregWin;
				pPriv->freeCompClip = freeTmpClip;
			} else {
				/* we need one 'real' region to put
				 * into the composite clip. if pregWin
				 * the current composite clip are real,
				 * we can get rid of one. if pregWin is
				 * real and the current composite clip isn't,
				 * use pregWin for the composite clip. if
				 * the current composite clip is real and
				 * pregWin isn't, use the current composite
				 * clip. if neither is real, create a
				 * new region.
				 */

				if ((freeTmpClip == FREE_CC) && (freeCompClip == FREE_CC)) {
					(* pGC->pScreen->Intersect)(pPriv->pCompositeClip, pregWin, pPriv->pAbsClientRegion);
					(* pGC->pScreen->RegionDestroy)(pregWin);
				} else if ((freeTmpClip == REPLACE_CC) &&  (freeCompClip == FREE_CC)) {
					pPriv->pCompositeClip = pregWin;
					(* pGC->pScreen->Intersect)(pPriv->pCompositeClip, pPriv->pCompositeClip, pPriv->pAbsClientRegion);
				} else if ((freeTmpClip == FREE_CC) && (freeCompClip == REPLACE_CC)) {
					(* pGC->pScreen->Intersect)(pPriv->pCompositeClip, pregWin, pPriv->pAbsClientRegion);
				} else if ((freeTmpClip == REPLACE_CC) && (freeCompClip == REPLACE_CC)) {
					pPriv->pCompositeClip = (* pGC->pScreen->RegionCreate)(NULL, 1);
					(* pGC->pScreen->Intersect)(pPriv->pCompositeClip, pregWin, pPriv->pAbsClientRegion);
				}
			}
			/* end of composite clip for a window */
		} else {
			BoxRec pixbounds;

			pixbounds.x1 = 0;
			pixbounds.y1 = 0;
			pixbounds.x2 = ((PixmapPtr)pDrawable)->width;
			pixbounds.y2 = ((PixmapPtr)pDrawable)->height;

			if (pPriv->freeCompClip == FREE_CC) {
				(* pGC->pScreen->RegionReset)( pPriv->pCompositeClip, &pixbounds);
			} else {
				pPriv->freeCompClip = FREE_CC;
				pPriv->pCompositeClip = (* pGC->pScreen->RegionCreate)(&pixbounds, 1);
			}

			if (pGC->clientClipType == CT_REGION) {
				(* pGC->pScreen->Intersect)(pPriv->pCompositeClip, pPriv->pCompositeClip, pPriv->pAbsClientRegion);
			}

			/* end of composite clip for pixmap */
		}
		if (pPriv->pCompositeClip == plxclipcurrentregion)
			plxclipinvalidate ();
	}

#ifdef DEBUG_CLIP
	printf("here is the composite clip ");
	miprintRects(pPriv->pCompositeClip);
#endif
	if (pWindow) {
		/* rotate tile patterns so that pattern can be
		 * combined in word by word, but the pattern seems
		 * to begin aligned with the window
		 */
		/* rotation code uses this */
		xrot = pWindow->absCorner.x;
		yrot = pWindow->absCorner.y;
	} else {
		yrot = 0;
		xrot = 0;
	}


	new_stipple = FALSE;
	new_line = FALSE;
	new_text = FALSE;
	new_fillspans = FALSE;
	new_rotation = FALSE;

	state_mask = changes;
	while (state_mask) {
		index = ffs(state_mask) - 1;
		state_mask &= ~(index = (1 << index));

		/* this switch acculmulates a list of which procedures
		 * might have to change due to changes in the GC.  in
		 * some cases (e.g. changing one 16 bit tile for another)
		 * we might not really need a change, but the code is
		 * being paranoid.
		 * this sort of batching wins if, for example, the alu
		 * and the font have been changed, or any other pair
		 * of items that both change the same thing.
		 */
		switch (index) {
		case GCFunction:
		case GCForeground:
			new_stipple = TRUE;
			new_text = TRUE;
			break;
		case GCPlaneMask:
			break;
		case GCBackground:
			new_stipple = TRUE;
			new_fillspans = TRUE;
			break;
		case GCLineStyle:
			break;
		case GCLineWidth:
		case GCCapStyle:
		case GCJoinStyle:
			new_line = TRUE;
			break;
		case GCFillStyle:
			new_text = TRUE;
			new_fillspans = TRUE;
			new_line = TRUE;
			break;
		case GCFillRule:
			break;
		case GCTile:
			if(pGC->tile == (PixmapPtr)NULL)
				break;
			new_rotation = TRUE;
			new_fillspans = TRUE;
			break;
		case GCStipple:
			if(pGC->stipple == (PixmapPtr)NULL) {
				break;
			}
			new_stipple = TRUE;
			new_rotation = TRUE;
			new_fillspans = TRUE;
			break;
		case GCTileStipXOrigin:
		case GCTileStipYOrigin:
			new_rotation = TRUE;
			new_stipple = TRUE;
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
	 * If the drawable has changed,  check its depth & ensure
	 * suitable entries are in the proc vector.
	 */
	if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
		new_fillspans = TRUE;		/* deal with FillSpans later */
	}

	/* deal with the changes we've collected */

	if (new_line) {
		if(pGC->lineWidth == 0) {
			pGC->Polylines = plxZeroPolylines;
			pGC->PolySegment = plxZeroPolySegment;
			pGC->PolyRectangle = plxZeroPolyRectangle;
		} else {
			pGC->Polylines = plxZeroPolylines;	/* XXXX wrong*/
			pGC->PolySegment = miPolySegment;
			pGC->PolyRectangle = miPolyRectangle;
			switch(pGC->joinStyle)
			{
			case JoinMiter:
				pGC->LineHelper = miMiter;
				break;
			case JoinRound:
			case JoinBevel:
				pGC->LineHelper = miNotMiter;
				break;
			}
		}
#ifndef X11R2
		procChanges |= MIBS_POLYLINES|MIBS_POLYSEGMENT|MIBS_POLYRECTANGLE;
#endif /* not X11R2 */
	}

	if (new_text && pGC->font) {
		pGC->PolyGlyphBlt = plxPolyGlyphBlt;
		pGC->ImageGlyphBlt = plxImageGlyphBlt;
#ifndef X11R2
		procChanges |= MIBS_POLYGLYPHBLT|MIBS_IMAGEGLYPHBLT;
#endif /* not X11R2 */
	}

	if (new_fillspans) {
		switch(pGC->fillStyle) {
		case FillSolid:
			pGC->FillSpans = plxSolidFS;
			break;
		case FillTiled:
			pGC->FillSpans = plxTileFS;
			break;
		case FillStippled:
			pGC->FillSpans = plxStippleFS;
			break;
		case FillOpaqueStippled:
			if (pGC->fgPixel == pGC->bgPixel)
				pGC->FillSpans = plxSolidFS;
			else
				pGC->FillSpans = plxStippleFS;
			break;
		default:
			FatalError("plxValidateGC: illegal fillStyle\n");
		}
#ifndef X11R2
		procChanges |= MIBS_FILLSPANS;
#endif /* not X11R2 */
	}

	if (pGC->fillStyle == FillTiled && !pGC->tile) {
		FatalError("plxValidateGC: no tile specified\n");
	}
	if (pGC->fillStyle == FillStippled && !pGC->stipple) {
		FatalError("plxValidateGC: no stipple specified\n");
	}

	if (new_rotation) {
		xrot += pGC->patOrg.x;
		yrot += pGC->patOrg.y;
	}

	/* 
	 * If there is a new tile/stipple pixmap, or the rotation has changed,
	 * Rotate the pixmap correctly.
	 */
	if (xrot || yrot || new_rotation) {
		if (IS_VALID_PIXMAP(pGC->stipple))
			plxrotatepixmap(pGC->stipple, xrot, yrot);
		if (IS_VALID_PIXMAP(pGC->tile))
			plxrotatepixmap(pGC->tile, xrot, yrot);
	}
	if (new_stipple) {
		if (IS_VALID_PIXMAP(pGC->stipple))
			plxchangestipplecolor(pGC, pDrawable);
	}
#ifndef X11R2
	/*
	 * If this GC has ever been used with a window with backing-store
	 * enabled, we must call miVaidateBackingStore to keep the
	 * backing-store module up-to-date, should this GC be used with that
	 * drawable again. In addition, if the current drawable is a window
	 * and has backing-store enabled, we also call miValidateBackingStore
	 * to give it a chance to get its hooks in.
	 */
	if (pGC->devBackingStore || (pWindow && (pWindow->backingStore != NotUseful))) {
		miValidateBackingStore(pDrawable, pGC, procChanges);
	}
#endif /* not X11R2 */
}

plxchangestipplecolor(pGC, pDrawable)
register GCPtr pGC;
DrawablePtr pDrawable;
{
	plxPrivGCPtr pPriv;
	ScreenPtr pScreen;
	PixmapPtr pstipple;
	short xorg, yorg, xt, yt;

	pPriv = ((plxPrivGCPtr)(pGC->devPriv));
	pScreen = pGC->pScreen;

	pstipple = pGC->stipple;

	pl_cache_lock(pstipple);

	/*
	 * first check the size of any allocated pixmap
	 */
	if (pPriv->plxStipple) {
		if ((pPriv->plxStipple->width != pstipple->width) || (pPriv->plxStipple->height != pstipple->height)) {
			 (*pScreen->DestroyPixmap)(pPriv->plxStipple);
			 pPriv->plxStipple = (PixmapPtr)0;
		}
	}

	/*
	 * second allocate pixmap for correctly colored stipple
	 */
	if (!pPriv->plxStipple) {
		pPriv->plxStipple = (*pScreen->CreatePixmap)(pScreen, pstipple->width, pstipple->height, pDrawable->depth);
		if (!pPriv->plxStipple) {
			ErrorF("Out of Cache for stipple");
			pl_cache_unlock(pstipple);
			return;
		}
	}
	pl_cache_lock(pPriv->plxStipple);

	/*
	 * now copy the stipple
	 */
	if (!plxpreparepattern(pstipple, &xt, &yt)) {
		ErrorF("plxchangestipplecolor: STIPPLE NOT IN CACHE\n");
		return;
	}
	if (!plxpixmapuse(PIXMAP_WRITE, pPriv->plxStipple, &xorg, &yorg)) {
		ErrorF("plxchangestipplecolor: PIXMAP NOT IN CACHE\n");
		return;
	}
	yorg = PTY(yorg);

	plxclipinvalidate();
	p_mask(0xffff);
	SetFontMaps(pGC->fgPixel, pGC->bgPixel, 0x01, 0);
	p_boxc(xt, yt+pstipple->height-1, xorg, PTY(yorg), xorg+pstipple->width-1, PTY(yorg+pstipple->height-1));
	p_rmap(0);
	pl_cache_unlock(pstipple);
	pl_cache_unlock(pPriv->plxStipple);
}

void
plxCopyGCDest(pGC, pQ, changes, pGCSrc)
GCPtr pGC;
GCInterestPtr pQ;
Mask  changes;
GCPtr pGCSrc;
{
	RegionPtr pClip;

	ifdebug(11) printf("plxCopyGCDest()\n");

	if (changes & GCClipMask) {
		if (pGC->clientClipType == CT_PIXMAP) {
			((PixmapPtr)pGC->clientClip)->refcnt++;
		} else if (pGC->clientClipType == CT_REGION) {
			BoxRec pixbounds;

			pixbounds.x1 = 0;
			pixbounds.y1 = 0;
			pixbounds.x2 = 0;
			pixbounds.y2 = 0;

			pClip = (RegionPtr) pGC->clientClip;
			pGC->clientClip = (pointer)(* pGC->pScreen->RegionCreate)(&pixbounds, 1);
			(* pGC->pScreen->RegionCopy)(pGC->clientClip, pClip);
		}
	}
}
