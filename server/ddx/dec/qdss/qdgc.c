/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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
#include "gcstruct.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mibstore.h"

#include "qd.h"
#include "qdprocs.h"

#include "mi.h"

/*
 * The following procedure declarations probably should be in mi.h
 */
extern void miGetImage();
extern PixmapPtr mfbCreatePixmap();
extern void miRecolorCursor();
extern void miImageGlyphBlt();
extern void miPolyGlyphBlt();

extern void miClearToBackground();
extern void miSaveAreas();
extern Bool miRestoreAreas();
extern void miTranslateBackingStore();

extern void miPolyFillRect();
extern void miPolyFillArc();

extern RegionPtr miCopyArea();
extern RegionPtr miCopyPlane();
extern void miPolyPoint();
extern void miPutImage();

extern void miMiter();
extern void miNotMiter();
extern void miZeroLine();
extern void miPolySegment();
extern void miPolyRectangle();
extern void miPolyArc();
extern void miFillPolygon();

extern void miImageText8(), miImageText16();
extern int miPolyText8(), miPolyText16();
extern void  miPolyArc();
extern void  miFillPolyArc();
extern void  miPushPixels();

#include "qdvalidate.h"

#define POW2( x)	((x) == power2ceiling(x))

Bool
qdCreateGC(pGC)
    GCPtr pGC;
{
    QDPrivGCPtr		pPriv;
    GCInterestPtr	pQ;

    if ( pGC->depth == 1)
	return mfbCreateGC(pGC);

    pGC->miTranslate = 0;    /* all qd output routines do window translation */
    pGC->clipOrg.x = pGC->clipOrg.y = 0;
    pGC->clientClip = (pointer)NULL;
    pGC->clientClipType = CT_NONE;

    pGC->ChangeClip = qdChangeClip;
    pGC->DestroyClip = qdDestroyClip;
    pGC->CopyClip = qdCopyClip;

    pPriv = (QDPrivGCPtr) Xalloc( sizeof(QDPrivGCRec));
    pPriv->mask = QD_NEWLOGIC;
    pPriv->ptresult = (unsigned char *) NULL;
    pPriv->pAbsClientRegion = (RegionPtr) NULL;
    pPriv->pCompositeClip = (RegionPtr) NULL;
    pPriv->freeCompClip = REPLACE_CC;
    pPriv->lastDest = 2;	/* impossible value */
    pPriv->GCstate = VSFullReset;

    pGC->devPriv = (pointer)pPriv;
    pGC->devBackingStore = (pointer)NULL;

    pQ = (GCInterestPtr) Xalloc(sizeof(GCInterestRec));
    if (!pQ)
    {
	Xfree(pPriv);
	return FALSE;
    }
     
    /*
     * Now link in this first GCInterest structure
     */
    pGC->pNextGCInterest = pQ;
    pGC->pLastGCInterest = pQ;
    pQ->pNextGCInterest = (GCInterestPtr) &pGC->pNextGCInterest;
    pQ->pLastGCInterest = (GCInterestPtr) &pGC->pNextGCInterest;
    pQ->length = sizeof(GCInterestRec);
    pQ->owner = 0;		/* server owns this */
    pQ->ValInterestMask = ~0;	/* interested in everything at validate time */
    pQ->ValidateGC = qdValidateGC;
    pQ->ChangeInterestMask = 0; /* interested in nothing at change time */
    pQ->ChangeGC = (int (*) () ) NULL;
    pQ->CopyGCSource = (void (*) () ) NULL;
    pQ->CopyGCDest = qdCopyGCDest;
    pQ->DestroyGC = qdDestroyGC;

    return TRUE;
}

void
qdDestroyGC( pGC, pQ)
    GCPtr		pGC;
    GCInterestPtr       pQ;
{
    QDPrivGCPtr devPriv = (QDPrivGCPtr)pGC->devPriv;

    if ( pGC->depth == 1)
    {
	mfbDestroyGC(pGC);
	return;
    }

    if (devPriv->ptresult)
	Xfree(devPriv->ptresult);

    if ( devPriv->pAbsClientRegion)
	(*pGC->pScreen->RegionDestroy)(
				devPriv->pAbsClientRegion);
    if (devPriv->freeCompClip == FREE_CC
     && devPriv->pCompositeClip)
	(*pGC->pScreen->RegionDestroy)(
				devPriv->pCompositeClip);
    Xfree( devPriv);
    Xfree( pQ);
}


void
qdDestroyClip( pGC)
    GCPtr	pGC;
{
    if ( pGC->clientClipType == CT_NONE)
	return;
    (*pGC->pScreen->RegionDestroy)( (RegionPtr)pGC->clientClip);
    pGC->clientClip = (pointer)NULL;
    pGC->clientClipType = CT_NONE;
}

void
qdChangeClip( pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    qdDestroyClip(pGC);
    if(type == CT_PIXMAP)
    {
	pGC->clientClip = (pointer) mfbPixmapToRegion(pvalue);
	(*pGC->pScreen->DestroyPixmap)(pvalue);
    }
    else if (type == CT_REGION)
    {
	pGC->clientClip = pvalue;
    }
    else if (type != CT_NONE)
    {
	pGC->clientClip = (pointer) miRectsToRegion(pGC, nrects, pvalue, type);
	Xfree(pvalue);
    }
    pGC->clientClipType = (pGC->clientClip) ? CT_REGION : CT_NONE;
    pGC->stateChanges |= GCClipMask;
}

void
qdCopyGCDest( pgcDst, pQ, maskQ, pgcSrc)
    GCPtr		pgcDst;
    GCInterestPtr	pQ;
    int                 maskQ;
    GCPtr		pgcSrc;
{
    RegionPtr		pregionsrc = (RegionPtr) pgcSrc->clientClip;

    if ( ! (maskQ & GCClipMask)
	|| pgcDst->clientClipType != CT_PIXMAP)
	return;

    pgcDst->clientClip =
	    (pointer) miRegionCreate( pregionsrc->rects, pregionsrc->numRects);
    miRegionCopy( (RegionPtr) pgcDst->clientClip, pregionsrc);
}

#define WINMOVED(pWin, pGC) \
((pWin->winSize->extents.x1 != pGC->lastWinOrg.x) || \
 (pWin->winSize->extents.y1 != pGC->lastWinOrg.y))

/*
 * Clipping conventions
 *	if the drawable is a window
 *	    CT_REGION ==> pCompositeClip really is the composite
 *	    CT_other ==> pCompositeClip is the window clip region
 *	if the drawable is a pixmap
 *	    CT_REGION ==> pCompositeClip is the translated client region
 *		clipped to the pixmap boundary
 *	    CT_other ==> pCompositeClip is the pixmap bounding box
 */
void
qdValidateGC( pGC, pQ, changes, pDrawable)
    register GCPtr	pGC;
    GCInterestPtr	pQ;
    Mask		changes; /* this arg should equal pGC->stateChanges */
    DrawablePtr		pDrawable;
{
    QDPrivGCPtr	devPriv = (QDPrivGCPtr)pGC->devPriv;
    WindowPtr	pWin;
    RegionPtr	pReg;
    Bool	newClientClip = FALSE;
    unsigned long	procChanges = 0;	/* for mibstore */
    unsigned long	vdone = 0;	/* vector already validated */
	/* vdone is a field of VECMAX bits indicating validation complete */

    /* throw away offscreen pixmap if it about to be changed. */
    if (pDrawable->type == DRAWABLE_PIXMAP && ((QDPixPtr)
	((PixmapPtr) pDrawable)->devPrivate)->offscreen != NOTOFFSCREEN)
    {
	tlCancelPixmap( ((PixmapPtr) pDrawable)->devPrivate );
	((QDPixPtr) ((PixmapPtr) pDrawable)->devPrivate)->offscreen
		= NOTOFFSCREEN;
    }

    if (pDrawable->type == DRAWABLE_WINDOW)
	pWin = (WindowPtr)pDrawable;
    else
	pWin = (WindowPtr)NULL;
    
    if ( pGC->clientClipType == CT_PIXMAP)
    {
	/*
	 * Always convert clipmasks to regions and then process as a
	 * changed region
	 */
	pReg = mfbPixmapToRegion( (PixmapPtr)pGC->clientClip, pGC->clipOrg.x,
	                   pGC->clipOrg.y);
	qdDestroyPixmap( (PixmapPtr)pGC->clientClip);
	pGC->clientClip = (pointer)pReg;
	if ( pReg == NullRegion)
	    pGC->clientClipType = CT_NONE;
	else
	    pGC->clientClipType = CT_REGION;
	pGC->stateChanges |= GCClipMask;
    }

    /*
     * if necessary, retranslate the client clip.
     * even if the window is different, if the origin
     * is the same there's no need to move the client
     * clip region around
     */
    if ( pGC->clientClipType == CT_REGION
     && ((pGC->stateChanges & (GCClipXOrigin|GCClipYOrigin|GCClipMask))
					 || (pWin && WINMOVED(pWin, pGC)) ))
    {
	/* retranslate client clip */
	if ( ! devPriv->pAbsClientRegion)
	    devPriv->pAbsClientRegion = miRegionCreate(NULL, 1);
	miRegionCopy( devPriv->pAbsClientRegion,
			(RegionPtr)pGC->clientClip);

	if (pWin)
	{
	    pGC->lastWinOrg.x = pWin->winSize->extents.x1;
	    pGC->lastWinOrg.y = pWin->winSize->extents.y1;
	    miTranslateRegion(
			    devPriv->pAbsClientRegion,
			    pGC->lastWinOrg.x + pGC->clipOrg.x,
			    pGC->lastWinOrg.y + pGC->clipOrg.y);
	}
	else
	{
	    pGC->lastWinOrg.x = 0;
	    pGC->lastWinOrg.y = 0;
	    miTranslateRegion(
			    devPriv->pAbsClientRegion,
			    pGC->clipOrg.x,
			    pGC->clipOrg.y);
	}
	newClientClip = TRUE;
    }

    /* 
     * recompute the composite clip if
     *	the translated client region has changed,
     *  clip-to-children enable has changed,
     *	the window's clip list has changed (serialNumber was bumped),
     *	the GC was used with some other window, or
     *					the window was used with some other GC
     */
    if ( newClientClip || 
	pGC->stateChanges & GCSubwindowMode ||
	pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
    {
	if (pWin)
	{
	    RegionPtr pregWin;	/* clip for this window, without client clip */
	    int freeTmpClip;

	    if (pGC->subWindowMode == IncludeInferiors)
	    {
	        pregWin = NotClippedByChildren( pWin);
		freeTmpClip = FREE_CC;		/* 1 */
	    }
	    else   /* ClipByChildren */
	    {
	        pregWin = pWin->clipList;
		freeTmpClip = REPLACE_CC;	/* 0 */
	    }

	    /* 
	     * if there is no client clip, we can get by with
	     * just keeping the pointer we got, and remembering
	     * whether or not should destroy (or maybe re-use)
	     * it later.  this way, we avoid unnecessary copying
	     * of regions.  (this wins especially if many clients clip
	     * by children and have no client clip.)
	     */
	    if ( pGC->clientClipType == CT_NONE)
	    {
		if ( devPriv->freeCompClip == FREE_CC)
		    (*pGC->pScreen->RegionDestroy)( devPriv->pCompositeClip);
		devPriv->pCompositeClip = pregWin;
		devPriv->freeCompClip = freeTmpClip;
	    }
	    else	/* client clipping enabled */
	    {
		/*
		 * We need one 'real' region to put into the composite clip.
		 * If pregWin and the current composite clip 
		 *  are real, we can get rid of one.
		 * If pregWin is real and the current composite
		 *  clip isn't, use pregWin for the composite clip.
		 * If the current composite clip is real and
		 *  pregWin isn't, use the current composite clip.
		 * If neither is real, create a new region.
		 */
		if ((freeTmpClip == FREE_CC) && 
		    (devPriv->freeCompClip == FREE_CC))
		{
		    miIntersect(
			    devPriv->pCompositeClip,
			    pregWin,
			    devPriv->pAbsClientRegion);
		    (*pGC->pScreen->RegionDestroy)(pregWin);
		}
		else if ((freeTmpClip == REPLACE_CC) && 
		    (devPriv->freeCompClip == FREE_CC))
		{
		    miIntersect(
			    devPriv->pCompositeClip,
			    pregWin,
			    devPriv->pAbsClientRegion);
		}
		else if ((freeTmpClip == FREE_CC) &&
		     (devPriv->freeCompClip == REPLACE_CC))
		{
		    miIntersect(
			    pregWin,
			    pregWin,
			    devPriv->pAbsClientRegion);
                    devPriv->pCompositeClip = pregWin;
		}
		else if ((freeTmpClip == REPLACE_CC) &&
		     (devPriv->freeCompClip == REPLACE_CC))
		{
		    devPriv->pCompositeClip = miRegionCreate(NULL, 1);
		    miIntersect(
			    devPriv->pCompositeClip,
			    pregWin,
			    devPriv->pAbsClientRegion);
		}
                devPriv->freeCompClip = FREE_CC;
	    }

	}
	else	/* output to a pixmap */
	{
	    BoxRec pixbounds;

	    pixbounds.x1 = 0;
	    pixbounds.y1 = 0;
	    pixbounds.x2 = ((PixmapPtr)pDrawable)->width;
	    pixbounds.y2 = ((PixmapPtr)pDrawable)->height;

	    if (devPriv->freeCompClip == FREE_CC)
	        miRegionReset( devPriv->pCompositeClip, &pixbounds);
	    else
	    {
		devPriv->freeCompClip = FREE_CC;
		devPriv->pCompositeClip = miRegionCreate(&pixbounds, 1);
	    }

	    if (pGC->clientClipType == CT_REGION)
		miIntersect(
			devPriv->pCompositeClip,
			devPriv->pCompositeClip,
			devPriv->pAbsClientRegion);
	}

    }

    /*
     * invalidate the version of this stipple in the off screen cache
     */


    if (pGC->stateChanges & GCStipple && pGC->stipple)
	tlCancelPixmap (pGC->stipple->devPrivate);

    if ( pGC->stateChanges & GCTile && pGC->tile)
    	tlCancelPixmap (pGC->tile->devPrivate);
	
    if ( pGC->stateChanges &
	    (GCFunction|GCPlaneMask|GCForeground|GCBackground|GCFillStyle))
	devPriv->mask |= QD_NEWLOGIC;

    /* VALIDATION */
    {
	register /* char */	ibit;	/* index of changed bit */
	register char	*plvec;	/* pointer to list of vector indexes */
	register char	*plvfac;	/* pointer to list of vector factors */
	tlvecchoice	*pvec;	/* validation choice entry */
	register int	ifunc;
	register int	row;
	unsigned long	privState;
	PFN	*gcVecs;
	/*
	 * this array maps between position in the GC and
	 * interest bits for miValidateBackingStore.  It
	 * is dependent on the ordering of functions in the GC
	 */
	static unsigned long miMap[32] = {
		MIBS_FILLSPANS, MIBS_SETSPANS, MIBS_PUTIMAGE,
		MIBS_COPYAREA, MIBS_COPYPLANE, MIBS_POLYPOINT,
		MIBS_POLYLINES, MIBS_POLYSEGMENT, MIBS_POLYRECTANGLE,
		MIBS_POLYARC, MIBS_FILLPOLYGON, MIBS_POLYFILLRECT,
		MIBS_POLYFILLARC, MIBS_POLYTEXT8, MIBS_POLYTEXT16,
		MIBS_IMAGETEXT8, MIBS_IMAGETEXT16,
		MIBS_IMAGEGLYPHBLT, MIBS_POLYGLYPHBLT,
		MIBS_PUSHPIXELS
	};

		/* cast function pointers to array entries of func list */
	gcVecs = (PFN *) (&pGC->FillSpans);
		/* tag private change bits on msb end of changes */
	privState = (devPriv->GCstate
		    | ((devPriv->lastDest == pDrawable->type ? 0 :
			VSDest)));
	switch (privState)
	{
	  case 0:
	  case VSFullReset:
	    break;
	  case VSDest:
	    devPriv->lastDest = pDrawable->type;
	    break;
	  case VSFullReset|VSDest:
	    devPriv->lastDest = pDrawable->type;
	    break;
	  default:
	    FatalError("weird qdss private validation factor, %ld\n",
		devPriv->GCstate);
	    break;
	}
	pGC->stateChanges |= privState << (GCLastBit+1);
	devPriv->GCstate = 0;
	for (ibit = 0; ibit < GCLastBit + VSNewBits + 1; ibit++)
	{
		/* step through change bits */
	  if (!(pGC->stateChanges & (1L<<ibit)))
	      continue;	/* change bit not set */
	  for (plvec = tlChangeVecs[ibit].pivec; *plvec != VECNULL; plvec++)
	  {
	    if (vdone & (1L<<(*plvec)))	/* if already validated */
		continue;
	    pvec = &tlVecChoice[*plvec];
	    ifunc = 0; row = 1;
	    for (plvfac = pvec->vfacs; *plvfac != VFACNULL; plvfac++)
	    {
		switch(*plvfac)	/* validation factors */
		{
		  case VFAClineWidth:
		    ifunc += (pGC->lineWidth == 0 ? 0 : 1) * row;
		    row *= 2;
		    break;
		  case VFAClineStyle:
		    ifunc += pGC->lineStyle * row;
		    row *= 3;
		    break;
		  case VFACcapStyle:
		    ifunc += pGC->capStyle * row;
		    row *= 4;
		    break;
		  case VFACjoinStyle:
		    ifunc += pGC->joinStyle * row;
		    row *= 3;
		    break;
		  case VFACfillStyle:
		    ifunc += pGC->fillStyle * row;
		    row *= 4;
		    break;
		  case VFACfillRule:
		    ifunc += pGC->fillRule * row;
		    row *= 2;
		    break;
		  case VFACarcMode:
		    ifunc += pGC->arcMode * row;
		    row *= 2;
		    break;
		  case VFACdest:
		    ifunc += (pDrawable->type < 0 ? 2:pDrawable->type) * row;
		    row *= 3;
		    break;
		  case VFACfont:
		    ifunc += (QDCreateFont(pGC->pScreen,pGC->font) ? 1:0)
			* row;
		    row *= 2;
		    break;
		  case VFACpat:
		    switch ( pGC->fillStyle)
		    {
		      case FillSolid:
			row *= 2;
			break;
		      case FillStippled:
		      case FillOpaqueStippled:
			if ( ! POW2( pGC->stipple->width)
			  || ! POW2( pGC->stipple->height))
			    ifunc += row;
			row *= 2;
			break;
		      case FillTiled:
			if ( ! POW2( pGC->tile->width)
			  || ! POW2( pGC->tile->height))
			    ifunc += row;
			row *= 2;
			break;
		    }
		    break;
		  default:
		    FatalError("weird qdss validation factor, %ld\n",
			(int) *plvfac);
		    break;
		}
	    }	/* for vector factors */
	    gcVecs[*plvec] = pvec->function[ifunc];
	    procChanges |= miMap[*plvec];
	    vdone |= 1L<<(*plvec);	/* indicate valid done */
	  }	/* for changed vectors */
	}	/* for GC + devPriv changed fields */
    }

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
}

/*
 * Install a vector to mi code for polygon filling, so that the set of pixels
 * touched conforms to the protocol specification.
 * The default is to use the fast dragon polygons, violating the protocol
 * specification.
 */
slowPolygons()	/* called from qdScreenInit */
{
    tlVecChoice[VECFillPolygon].function[0] = miFillPolygon;
}

void
qdCopyClip( pgcDst, pgcSrc)
    GCPtr	pgcDst, pgcSrc;
{
    RegionPtr	prgnNew;

    switch( pgcSrc->clientClipType)
    {
      case CT_NONE:
      case CT_PIXMAP:
        qdChangeClip( pgcDst, pgcSrc->clientClipType, pgcSrc->clientClip, 0);
        break;
      case CT_REGION:
        prgnNew = (* pgcSrc->pScreen->RegionCreate)(NULL, 1);
        (* pgcSrc->pScreen->RegionCopy)(prgnNew,
                                       (RegionPtr)(pgcSrc->clientClip));
        qdChangeClip( pgcDst, CT_REGION, prgnNew, 0);
        break;
    }
}
