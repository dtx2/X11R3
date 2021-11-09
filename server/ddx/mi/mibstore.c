/* $XConsortium: mibstore.c,v 1.20 88/10/20 19:59:28 keith Exp $ */
/***********************************************************
Copyright 1987 by the Regents of the University of California
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written prior
permission.  

The University of California makes no representations about the suitability
of this software for any purpose.  It is provided "as is" without express or
implied warranty.

 *
 * XXX: Necessitates a change to the GC structure to implement. If one
 * could find an efficient way to find changed vectors and copy and replace
 * them, it could all be taken care of with the GCInterest structure, which
 * is what said structure was designed for, though this would make the actual
 * drawing functions a bit trickier, having to find the GCInterest structure...
 *

******************************************************************/

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "misc.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "extnsionst.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "dixstruct.h"		/* For requestingClient */
#include "mi.h"
#include "mibstore.h"

/*-
 * NOTES ON USAGE:
 *
 * The functions in this file implement a machine-independent backing-store
 * scheme. To use it, the output library must do the following:
 *	- Provide a SaveAreas function that takes a destination pixmap, a
 *	    region of the areas to save (in the pixmap's coordinate system)
 *	    and the screen origin of the region. It should copy the areas from
 *	    the screen into the pixmap.
 *	- Provide a RestoreAreas function that takes a source pixmap, a region
 *	    of the areas to restore (in the screen's coordinate system) and the
 *	    origin of the pixmap on the screen. It should copy the areas from
 *	    the pixmap into the screen.
 *	- Provide a SetClipmaskRgn function that takes a gc and a region
 *	    and merges the region into any CT_PIXMAP client clip that
 *	    is specified in the GC.  This routine is only needed if
 *	    miValidateBackingStore will see CT_PIXMAP clip lists; not
 *	    true for any of the sample servers (which convert the PIXMAP
 *	    clip lists into CT_REGION clip lists; an expensive but simple
 *	    to code option).
 *	- ValidateGC must call miValidateBackingStore for any GC that is now
 *	    being, or has been, used with a window with backing-store enabled.
 *	    How the library keeps track of this is its own business, but these
 *	    functions guarantee that the devBackingStore field in the GC will
 *	    be non-NULL for every GC used with a window with backing-store on.
 *	- ValidateGC must also maintain a bitmask made from the MIBS constants
 *	    (found in mibstore.h) indicating what vectors it altered. This mask
 *	    is passed as the third argument to miValidateBackingStore.
 *	- The CreateGC function should initialize devBackingStore to be NULL.
 *	- The ChangeWindowAttributes functions must call miInitBackingStore
 *	    when backing-store is enabled (backingStore != NotUseful) for the
 *	    window and miFreeBackingStore when backing-store is disabled for
 *	    it.  ChangeWindowAttributes must update the window's serialNumber
 *	    when backing-store changes.
 *	- The DestroyWindow function should call miFreeBackingStore if
 *	    backingStore != NotUseful.
 *	- The GetSpans function must call miBSGetSpans at the end of its
 *	    operation, passing in the source drawable, a pixmap via which
 *	    spans from the backing store may be drawn into those fetched from
 *	    the screen, followed by the other four arguments GetSpans received
 *	    and the array of widths with each width padded to the actual width
 *	    of the span in the pixmap (see the declaration for miBSGetSpans,
 *	    below). This should only be done if the source drawable is a
 *	    window and backingStore is not NotUseful.
 *	- The GetImage function should call miBSGetImage at its end, passing
 *	    the source drawable, a pixmap through which the extracted image
 *	    may be modified, followed by all the other arguments, in their
 *	    original order, except for the image memory's address. Once again
 *	    this should only be done if the source drawable is a window with
 *	    backingStore enabled.
 *	- The function placed in a window's ClearToBackground vector must call
 *	    pWin->backStorage->ClearToBackground with the window, followed by
 *	    the window-relative x and y coordinates, followed by the width and
 *	    height of the area to be cleared, followed by the generateExposures
 *	    flag. This has been taken care of in miClearToBackground.
 *	- Whatever determines GraphicsExpose events for the CopyArea and
 *	    CopyPlane requests should call pWin->backStorage->ExposeCopy
 *	    with the source and destination drawables, the GC used, a source-
 *	    window-relative region of exposed areas, the source and destination
 *	    coordinates and the bitplane copied, if CopyPlane, or 0, if
 *	    CopyArea.
 *
 * MODIFICATIONS MADE TO THE SERVER, TO MAKE THIS WORK, THAT SHOULD BE NOTED
 * WHEN ALTERING AN EXISTING IMPLEMENTATION:
 *	- miHandleExposures now takes an extra argument, plane, which is 0
 *	    if called from CopyArea and the plane-to-copy if called from
 *	    CopyPlane.
 *	- An extra field, devBackingStore, was added to the GC structure.
 *	- The code in sunCursor.c was modified to also intercept calls through
 *	    a window's SaveDoomedAreas and RestoreAreas procedure vectors in
 *	    the backStorage structure and remove the cursor if it interfered
 *	    with the operation.
 *	- miValidateTree was changed to modify backStorage->oldAbsCorner when
 *	    a window moves so the proper region of the screen can be saved
 *	    in miSaveAreas.
 *
 * JUSTIFICATION
 *    This is a cross between saving everything and just saving the
 * obscued areas (as in Pike's layers.)  This method has the advantage
 * of only doing each output operation once per pixel, visible or
 * invisible, and avoids having to do all the crufty storage
 * management of keeping several separate rectangles.  Since the
 * ddx layer ouput primitives are required to draw through clipping
 * rectangles anyway, sending multiple drawing requests for each of
 * several rectangles isn't necessary.  (Of course, it could be argued
 * that the ddx routines should just take one rectangle each and
 * get called multiple times, but that would make taking advantage of
 * smart hardware harder, and probably be slower as well.)
 */

#define PROLOGUE(pWin) \
    MIBackingStorePtr pBackingStore = \
    	(MIBackingStorePtr)((WindowPtr)pWin)->devBackingStore; \
    MIBSGCPrivPtr pPriv = (MIBSGCPrivPtr)pGC->devBackingStore; \
    GCPtr pBackingGC = pPriv->pBackingGC
   
/*
 * One of these structures is allocated per GC used with a backing-store
 * drawable. Once this is allocated, miValidateBackingStore must always
 * be called for the GC. While this could be done using the GCInterest
 * structure we place on the GC for when the GC is destroyed (so we can
 * free this data), we would have to go through a not-inexpensive process
 * to figure out what procedures had changed.
 */
typedef struct {
    GCPtr   	  pBackingGC;	    /* Copy of the GC but with graphicsExposures
				     * set FALSE and the clientClip set to
				     * clip output to the valid regions of the
				     * backing pixmap. */
    Bool    	  inUse;    	    /* Set TRUE if performing an output
				     * operation using this structure. Used
				     * to avoid "double-buffering", so to
				     * speak */
    Bool    	  gcHooked; 	    /* Set TRUE if our functions are in the
				     * GC's procedure vectors */
    unsigned long changes;  	    /* Accumulated changes since GC was last
				     * fully-validated (i.e. used with a window
				     * with backing-store on). */
    int 	  guarantee;        /* GuaranteeNothing, etc. */
    unsigned long serialNumber;	    /* clientClip computed time */
    /* The real procedures to call */
    void    	  (* FillSpans)();
    void    	  (* SetSpans)();
    void    	  (* PutImage)();
    RegionPtr  	  (* CopyArea)();
    RegionPtr     (* CopyPlane)();
    void    	  (* PolyPoint)();
    void    	  (* Polylines)();
    void    	  (* PolySegment)();
    void    	  (* PolyRectangle)();
    void    	  (* PolyArc)();
    void    	  (* FillPolygon)();
    void    	  (* PolyFillRect)();
    void    	  (* PolyFillArc)();
    int     	  (* PolyText8)();
    int     	  (* PolyText16)();
    void    	  (* ImageText8)();
    void    	  (* ImageText16)();
    void    	  (* ImageGlyphBlt)();
    void    	  (* PolyGlyphBlt)();
    void    	  (* PushPixels)();
} MIBSGCPrivRec, *MIBSGCPrivPtr;

static RegionPtr miRestoreAreas();
static void miSaveAreas(), miBSDrawGuarantee(),
	    miTranslateBackingStore(), miExposeCopy(),
	    miCreateBSPixmap(), miDestroyBSPixmap(), miTileVirtualBS();

/*-
 *-----------------------------------------------------------------------
 * miBSFillSpans --
 *	Perform a FillSpans, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    PROLOGUE(pDrawable);

    if (!pPriv->inUse)
    {
	DDXPointPtr	pptCopy;
	int 	  	*pwidthCopy;

	pPriv->inUse = TRUE;

	pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(nInit*sizeof(DDXPointRec));
	pwidthCopy=(int *)ALLOCATE_LOCAL(nInit*sizeof(int));
	bcopy((char *)pptInit,(char *)pptCopy,nInit*sizeof(DDXPointRec));
	bcopy((char *)pwidthInit,(char *)pwidthCopy,nInit*sizeof(int));

	(* pPriv->FillSpans)(pDrawable, pGC, nInit, pptInit,
			     pwidthInit, fSorted);
	(* pBackingGC->FillSpans)(pBackingStore->pBackingPixmap,
				  pBackingGC, nInit, pptCopy, pwidthCopy,
				  fSorted);

	DEALLOCATE_LOCAL(pptCopy);
	DEALLOCATE_LOCAL(pwidthCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
	(* pPriv->FillSpans)(pDrawable, pGC, nInit, pptInit,
			     pwidthInit, fSorted);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSSetSpans --
 *	Perform a SetSpans, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc;
    register DDXPointPtr ppt;
    int			*pwidth;
    int			nspans;
    int			fSorted;
{
    PROLOGUE(pDrawable);

    if (!pPriv->inUse)
    {
	DDXPointPtr	pptCopy;
	int 	*pwidthCopy;

	pPriv->inUse = TRUE;

	pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(nspans*sizeof(DDXPointRec));
	pwidthCopy=(int *)ALLOCATE_LOCAL(nspans*sizeof(int));
	bcopy((char *)ppt,(char *)pptCopy,nspans*sizeof(DDXPointRec));
	bcopy((char *)pwidth,(char *)pwidthCopy,nspans*sizeof(int));

	(* pPriv->SetSpans)(pDrawable, pGC, psrc, ppt, pwidth,
			    nspans, fSorted);
	(* pBackingGC->SetSpans)(pBackingStore->pBackingPixmap, pBackingGC,
				 psrc, pptCopy, pwidthCopy, nspans,
				 fSorted);

	DEALLOCATE_LOCAL(pptCopy);
	DEALLOCATE_LOCAL(pwidthCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
	(* pPriv->SetSpans)(pDrawable, pGC, psrc, ppt, pwidth,
			    nspans, fSorted);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSGetSpans --
 *	Get obscured spans from a window's backing-store and place them
 *	in the correct place in those spans already fetched by the screen's
 *	regular GetSpans routine. The passed pixmap is expected to contain
 *	the spans already fetched with each span being on a separate
 *	scanline in the pixmap.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Invalid bits in the spans are overwritten with valid ones from
 *	the backing pixmap.
 *
 * Notes:
 *	Worry about spans being wMax from GetSpans and w to SetSpans...
 *
 *-----------------------------------------------------------------------
 */
void
miBSGetSpans(pDrawable, pPixmap, wMax, ppt, pwidth, pwidthPadded, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    PixmapPtr	  	pPixmap;    	/* pixmap of already-gotten spans */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;	    	/* points to start copying from */
    int			*pwidth;	/* list of number of pixels to copy */
    int	    	  	*pwidthPadded;	/* Actual width of span */
    int			nspans;		/* number of scanlines to copy */
{
    int	    	  	n;  	    	/* Max number of clipped spans */
    DDXPointPtr	  	pptClipped; 	/* Points for the clipped spans */
    DDXPointPtr	  	pptDest;    	/* Points for setting clipped spans */
    int	    	  	*pwidthClipped;	/* Widths for the clipped spans */
    register int  	i, j;	    	/* General indices */
    DDXPointRec	  	pt; 	    	/* Translated span point */
    register int  	k;  	    	/* Index into clipped spans */
    register WindowPtr 	pWin;	    	/* Source window */
    MIBackingStorePtr	pBackingStore;	/* Pointer to our data */
    GCPtr   	  	pGC;	    	/* GC for SetSpans */
    unsigned int  	*pBits;	    	/* Data fetched from backing pixmap */
    int	    	  	xorg;	    	/* Origin of cur span in the pixmap */
    
    if (pDrawable->type != DRAWABLE_WINDOW)
	FatalError("miBSGetSpans called with a pixmap");
    pWin = (WindowPtr)pDrawable;
    pBackingStore = (MIBackingStorePtr)pWin->devBackingStore;
    
    /*
     * First allocate enough room for points and widths to clip a single
     * span (since a span may only be divided into the same number of spans
     * as there are boxes in the largest band of the region, we allocate
     * that much room).
     */
    n = miFindMaxBand(pBackingStore->pSavedRegion) * nspans;
    pptClipped = (DDXPointPtr) ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    pptDest = (DDXPointPtr)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    pwidthClipped = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    
    if(!pptClipped || !pwidthClipped || !pptDest)
    {
	DEALLOCATE_LOCAL(pptClipped);
	DEALLOCATE_LOCAL(pwidthClipped);
	DEALLOCATE_LOCAL(pptDest);
	return;
    }

    pGC = GetScratchGC(pDrawable->depth, pDrawable->pScreen);
    ValidateGC((DrawablePtr)pPixmap, pGC);
    
    /*
     * For each span, translate it into the backing pixmap's coordinates and
     * clip it to the valid area of the pixmap. If any spans result, update
     * wMax and translate the clipped spans into pPixmap's coordinates (x
     * origin is start of span, y coordinate is span number) and store the
     * resulting points in pptDest.
     * XXX: This is probably slow, but I don't relish the thought of figuring
     * out which clipped spans go with which original span in order to do the
     * translation to pPixmap's mucked-up coordinate system.
     */
    xorg = 0;
    for (k = wMax = i = 0; i < nspans; i++)
    {
	pt.x = ppt[i].x - pWin->absCorner.x;
	pt.y = ppt[i].y - pWin->absCorner.y;
	n = miClipSpans(pBackingStore->pSavedRegion, &pt, &pwidth[i], 1,
			&pptClipped[k], &pwidthClipped[k], TRUE);
	if (n != 0) {
	    /*
	     * Figure wMax for GetSpans
	     */
	    for (j = n-1 ; j >= 0; j--) {
		if (pwidthClipped[k+j] > wMax) {
		    wMax = pwidthClipped[k+j];
		}
	    }

	    /*
	     * Retranslate clipped spans to modify destination pixmap.
	     */
	    for (j = n-1; j >= 0; j--) {
		pptDest[k+j].x = pptClipped[k+j].x - pt.x + xorg;
		pptDest[k+j].y = 0;
	    }
	}
	xorg += pwidthPadded[i];
	k += n;
    }

	    
    if (k != 0)
    {
	/*
	 * Get all the bits at once, then set them in.
	 */
	if (pBackingStore->pBackingPixmap == NullPixmap)
	    miCreateBSPixmap (pWin);

	if (pBackingStore->pBackingPixmap != NullPixmap)
	{
	    pBits = (*pDrawable->pScreen->GetSpans) (
 	    		pBackingStore->pBackingPixmap,
			wMax, pptClipped,
			pwidthClipped,
			k);
	    (* pGC->SetSpans) (pPixmap, pGC, pBits, pptDest, pwidthClipped,
			   k, FALSE);
	
	    Xfree((pointer)pBits);
	}
    }
    FreeScratchGC(pGC);
    DEALLOCATE_LOCAL(pptClipped);
    DEALLOCATE_LOCAL(pwidthClipped);
    DEALLOCATE_LOCAL(pptDest);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPutImage --
 *	Perform a PutImage, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int		  depth;
    int	    	  x;
    int	    	  y;
    int	    	  w;
    int	    	  h;
    int	    	  format;
    char    	  *pBits;
{
    PROLOGUE(pDst);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
	(*pPriv->PutImage)(pDst, pGC, depth, x, y, w, h, leftPad,
			   format, pBits);
	(*pBackingGC->PutImage)(pBackingStore->pBackingPixmap, pBackingGC,
				depth, x, y, w, h, leftPad, format, pBits);
	pPriv->inUse = FALSE;
    }
    else
    {
	(*pPriv->PutImage)(pDst, pGC, depth, x, y, w, h, leftPad,
			   format, pBits);
    }
}

/*-
 * miGetImageWithBS
 *
 * use miGetImage and miBSGetImage to form a composite image for a
 * backing stored window
 */

void
miGetImageWithBS ( pDraw, x, y, w, h, format, planemask, pImage)
    DrawablePtr		pDraw;
    int			x, y, w, h;
    unsigned int	format;
    unsigned long	planemask;
    unsigned char	*pImage;
{
    int	oldBackingStore = NotUseful;

    /*
     * miGetImage uses the pScreen->GetSpans but GetSpans has been mangled to
     * get bits from backing store for, which we desparately want to avoid at
     * this point as GetImage is expected to return screen contents.
     */
    if (pDraw->type == DRAWABLE_WINDOW)
    {
	oldBackingStore = ((WindowPtr)pDraw)->backingStore;
	((WindowPtr)pDraw)->backingStore = NotUseful;
    }
    miGetImage( pDraw, x, y, w, h, format, planemask, pImage);
    if (oldBackingStore != NotUseful)
    {
	((WindowPtr)pDraw)->backingStore = oldBackingStore;
	miBSGetImage((WindowPtr) pDraw, NullPixmap, x, y, w, h,
		     format, planemask, pImage);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSGetImage --
 *	Retrieve the missing pieces of the given image from the window's
 *	backing-store and place them into the given pixmap at the proper
 *	location.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Parts of pImage are modified.
 *
 *-----------------------------------------------------------------------
 */

void
miBSGetImage (pWin, pOldPixmapPtr, x, y, w, h, format, planeMask, pImage)
    WindowPtr	  pWin;	    	    /* Source window with bstore */
    PixmapPtr	  pOldPixmapPtr;    /* pixmap containing screen contents */
    int	    	  x;	    	    /* Window-relative x of source box */
    int	    	  y;	    	    /* Window-relative y of source box */
    int	    	  w;	    	    /* Width of source box */
    int	    	  h;	    	    /* Height of source box */
    unsigned int  format;   	    /* Format for request */
    unsigned long  planeMask;	    /* Mask of planes to fetch */
    pointer	  pImage;	    /* pointer to space allocated by caller */
{
    ScreenPtr	  	pScreen;    /* Screen for proc vectors */
    PixmapPtr		pNewPixmapPtr; /* local pixmap hack */
    PixmapPtr		pPixmapPtr;
    RegionPtr		pRgn;
    BoxRec		box;
    GCPtr		pGC;
    static void		miBSDoGetImage ();
    int			depth;

    pScreen = pWin->drawable.pScreen;

    /*
     * First figure out what part of the source box is actually not visible
     * by taking the inverse of the not-clipped-by-children region.
     */

    pRgn = (* pScreen->RegionCreate)(NULL, 1);
    box.x1 = x + pWin->absCorner.x;
    box.y1 = y + pWin->absCorner.y;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;
    
    (* pScreen->Inverse)(pRgn, pWin->borderClip, &box);

    /*
     * Nothing that wasn't visible -- return immediately.
     */
    if (!(* pScreen->RegionNotEmpty) (pRgn))
    {
	(* pScreen->RegionDestroy) (pRgn);
	return;
    }

    /*
     * if no pixmap was given to us, create one now
     */

    depth = format == ZPixmap ? pWin->drawable.depth : 1;
    
    pGC = GetScratchGC (depth, pScreen);

    /*
     * make sure the CopyArea operations below never
     * end up sending NoExpose or GraphicsExpose events.
     */
    pGC->graphicsExposures = FALSE;
    if (!pOldPixmapPtr && pImage) {
	pNewPixmapPtr = (*pScreen->CreatePixmap) (pScreen, w, h, depth);
	if (pNewPixmapPtr) {
	    ValidateGC ((DrawablePtr)pNewPixmapPtr, pGC);
	    (*pGC->PutImage) (pNewPixmapPtr, pGC,
 			      depth, 0, 0, w, h, 0,
			      ZPixmap, pImage);
	}
	pPixmapPtr = pNewPixmapPtr;
    }
    else
    {
	pNewPixmapPtr = NullPixmap;
	pPixmapPtr = pOldPixmapPtr;
	if (pPixmapPtr)
	    ValidateGC ((DrawablePtr)pPixmapPtr, pGC);
    }

    /*
     * translate to window-relative coordinates
     */

    if (pPixmapPtr)
    {
	(* pScreen->TranslateRegion) (pRgn, -pWin->absCorner.x, -pWin->absCorner.y);

	miBSDoGetImage (pWin, pPixmapPtr, pRgn, x, y, pGC, planeMask);
    }

    /*
     * now we have a composite view in pNewPixmapPtr; create the resultant
     * image
     */

    if (pNewPixmapPtr)
    {
	(*pScreen->GetImage) (pNewPixmapPtr, 0, 0, w, h,
 			      format, (format == ZPixmap) ? planeMask : 1, pImage);
	(*pScreen->DestroyPixmap) (pNewPixmapPtr);
    }
    FreeScratchGC (pGC);
    (*pScreen->RegionDestroy) (pRgn);
}

/*
 * fill a region of the destination with virtual bits
 *
 * pRgn is offset by (x, y) into the drawable
 */

static void
miBSFillVirtualBits (pDrawable, pGC, pRgn, x, y, pixel, tile, use_pixel, planeMask)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    RegionPtr		pRgn;
    unsigned long	pixel;
    int			x, y;
    PixmapPtr		tile;
    PixmapPtr		use_pixel;
    unsigned long	planeMask;
{
    int		i;
    BITS32	gcmask;
    XID		gcval[5];
    xRectangle	*pRect;
    BoxPtr	pBox;
    WindowPtr	pWin;

    if (tile == (PixmapPtr) None)
	return;
    pWin = 0;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDrawable;
	if (pWin->backingStore == NotUseful || !pWin->backStorage)
	    pWin = 0;
    }
    i = 0;
    gcmask = 0;
    gcval[i++] = planeMask;
    gcmask |= GCPlaneMask;
    if (tile == use_pixel)
    {
	if (pGC->fgPixel != pixel)
	{
	    gcval[i++] = (XID) pixel;
	    gcmask |= GCForeground;
	}
	if (pGC->fillStyle != FillSolid)
	{
	    gcval[i++] = (XID) FillSolid;
	    gcmask |= GCFillStyle;
	}
    }
    else
    {
	if (pGC->fillStyle != FillTiled)
	{
	    gcval[i++] = (XID) FillTiled;
	    gcmask |= GCFillStyle;
	}
	if (pGC->tile != tile)
	{
	    gcval[i++] = (XID) tile;
	    gcmask |= GCTile;
	}
	if (pGC->patOrg.x != -x)
	{
	    gcval[i++] = (XID) -x;
	    gcmask |= GCTileStipXOrigin;
	}
	if (pGC->patOrg.y != -y)
	{
	    gcval[i++] = (XID) -y;
	    gcmask |= GCTileStipYOrigin;
	}
    }
    if (gcmask)
	DoChangeGC (pGC, gcmask, gcval, 1);

    if (pWin)
	(*pWin->backStorage->DrawGuarantee) (pWin, pGC, GuaranteeVisBack);

    if (pDrawable->serialNumber != pGC->serialNumber)
	ValidateGC (pDrawable, pGC);

    pRect = (xRectangle *)ALLOCATE_LOCAL(pRgn->numRects * sizeof(xRectangle));
    pBox = pRgn->rects;
    for (i = 0; i < pRgn->numRects; i++, pBox++, pRect++)
    {
    	pRect->x = pBox->x1 - x;
	pRect->y = pBox->y1 - y;
	pRect->width = pBox->x2 - pBox->x1;
	pRect->height = pBox->y2 - pBox->y1;
    }
    pRect -= pRgn->numRects;
    (*pGC->PolyFillRect) (pDrawable, pGC, pRgn->numRects, pRect);
    if (pWin)
	(*pWin->backStorage->DrawGuarantee) (pWin, pGC, GuaranteeNothing);
    DEALLOCATE_LOCAL (pRect);
}

/*
 * copy this window's backing store and all of it's childrens
 * backing store into pPixmap
 *
 * (x, y) is the offset of the pixmap into the window
 */

static void
miBSDoGetImage (pWin, pPixmap, pRgn, x, y, pGC, planeMask)
    WindowPtr	pWin;
    PixmapPtr	pPixmap;
    RegionPtr	pRgn;
    int		x, y;
    GCPtr	pGC;
    unsigned long	planeMask;
{
    MIBackingStorePtr	pBackingStore;
    BoxPtr	pBox;
    WindowPtr	pChild;
    RegionPtr	pBackRgn;
    ScreenPtr	pScreen;
    int		dx, dy;
    int		n;
    
    pBox = pRgn->rects;
    pScreen = pWin->drawable.pScreen;
    pBackingStore = (MIBackingStorePtr)pWin->devBackingStore;
    pBackRgn = (*pScreen->RegionCreate) (NULL, 1);
    if (pWin->backingStore != NotUseful && pWin->backStorage &&
        pBackingStore->status != StatusNoPixmap)
    {
	(*pScreen->Intersect) (pBackRgn, pRgn, pBackingStore->pSavedRegion);

	if ((*pScreen->RegionNotEmpty) (pBackRgn))
	{
	    if (!pBackingStore->pBackingPixmap &&
 		pBackingStore->backgroundTile != (PixmapPtr) ParentRelative &&
		pWin->drawable.depth == pPixmap->drawable.depth)
	    {
		miBSFillVirtualBits ((DrawablePtr)pPixmap, pGC, pBackRgn, x, y,
	    			     pBackingStore->backgroundPixel,
				     pBackingStore->backgroundTile,
				     (PixmapPtr) USE_BACKGROUND_PIXEL,
				     planeMask);
	    }
	    else
	    {
		if (pBackingStore->pBackingPixmap == NullPixmap)
		    miCreateBSPixmap (pWin);

		if (pBackingStore->pBackingPixmap != NullPixmap)
		{
		    if (pWin->drawable.depth != pPixmap->drawable.depth && pBackRgn->numRects)
		    {
			XID	gcval[3];

			gcval[0] = 1;	/* plane mask */
			gcval[1] = 1;	/* foreground */
			gcval[2] = 0;	/* background */
			DoChangeGC (pGC, GCPlaneMask|GCForeground|GCBackground, gcval, 1);
			ValidateGC ((DrawablePtr) pPixmap, pGC);
		    }
		    pBox = pBackRgn->rects;
		    for (n = 0; n < pBackRgn->numRects; n++)
		    {
			if (pWin->drawable.depth == pPixmap->drawable.depth)
			    (*pGC->CopyArea) (pBackingStore->pBackingPixmap,
 					  pPixmap, pGC,
					  pBox->x1, pBox->y1,
 					  pBox->x2 - pBox->x1,
 					    pBox->y2 - pBox->y1,
					  pBox->x1 - x, pBox->y1 - y);
			else
			    (*pGC->CopyPlane) (pBackingStore->pBackingPixmap,
			    		  pPixmap, pGC,
					  pBox->x1, pBox->y1,
 					  pBox->x2 - pBox->x1,
 					    pBox->y2 - pBox->y1,
					  pBox->x1 - x, pBox->y1 - y, planeMask);
			pBox++;
		    }
		}
	    }
	}
    }

    /*
     * draw the border of this window into the pixmap
     */

    /* XXX can't tile the border into the wrong depth pixmap */

    if (pWin->borderWidth > 0 &&
	(pWin->drawable.depth == pPixmap->drawable.depth ||
	 pWin->borderTile == (PixmapPtr) USE_BORDER_PIXEL))
    {
	unsigned long	pixel;

	pBox = (*pScreen->RegionExtents) (pRgn);

	if (pBox->x1 < 0 || pBox->y1 < 0 ||
	    pWin->borderWidth + pWin->clientWinSize.width < pBox->x2 ||
	    pWin->borderWidth + pWin->clientWinSize.height < pBox->y2)
	{
	    /*
	     * compute areas of border to display
	     */

	    (*pScreen->Subtract) (pBackRgn, pWin->borderSize, pWin->winSize);

	    /*
	     * translate relative to pWin
	     */
	    (*pScreen->TranslateRegion) (pBackRgn,
 					 -pWin->absCorner.x,
 					 -pWin->absCorner.y);

	    /*
	     * extract regions to fill
	     */

	    (*pScreen->Intersect) (pBackRgn, pBackRgn, pRgn);

	    if ((*pScreen->RegionNotEmpty) (pBackRgn))
	    {
	        pixel = pWin->borderPixel;
	        if (pWin->drawable.depth != pPixmap->drawable.depth)
		    pixel = (pWin->borderPixel & planeMask) != 0;
	        miBSFillVirtualBits ((DrawablePtr)pPixmap, pGC, pBackRgn,
				     x, y,
				     pixel,
				     pWin->borderTile,
				     (PixmapPtr) USE_BORDER_PIXEL, planeMask);
	    }
	}
    }

    (*pScreen->RegionDestroy) (pBackRgn);

    /*
     * now fetch any bits from children's backing store
     */
    if (pWin->lastChild)
    {
	for (pChild = pWin->lastChild; pChild; pChild = pChild->prevSib) {
	    /*
	     * create a region which covers the non-visibile
	     * portions of the child which need to
	     * be restored
	     */

	    dx = pChild->absCorner.x - pWin->absCorner.x;
	    dy = pChild->absCorner.y - pWin->absCorner.y;

	    (*pScreen->TranslateRegion) (pRgn, -dx, -dy);

	    miBSDoGetImage (pChild, pPixmap, pRgn, x - dx, y - dy,
 	    		    pGC, planeMask);

	    (*pScreen->TranslateRegion) (pRgn, dx, dy);
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSDoCopy --
 *	Perform a CopyArea or CopyPlane within a window that has backing
 *	store enabled.
 *
 * Results:
 *	TRUE if the copy was performed or FALSE if a regular one should
 *	be done.
 *
 * Side Effects:
 *	Things are copied (no s***!)
 *
 * Notes:
 *	The idea here is to form two regions that cover the source box.
 *	One contains the exposed rectangles while the other contains
 *	the obscured ones. An array of <box, drawable> pairs is then
 *	formed where the <box> indicates the area to be copied and the
 *	<drawable> indicates from where it is to be copied (exposed regions
 *	come from the screen while obscured ones come from the backing
 *	pixmap). The array 'sequence' is then filled with the indices of
 *	the pairs in the order in which they should be copied to prevent
 *	things from getting screwed up. A call is also made through the
 *	backingGC to take care of any copying into the backing pixmap.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miBSDoCopy(pWin, pGC, srcx, srcy, w, h, dstx, dsty, plane, copyProc, ppRgn)
    WindowPtr	  pWin;	    	    /* Window being scrolled */
    GCPtr   	  pGC;	    	    /* GC we're called through */
    int	    	  srcx;	    	    /* X of source rectangle */
    int	    	  srcy;	    	    /* Y of source rectangle */
    int	    	  w;	    	    /* Width of source rectangle */
    int	    	  h;	    	    /* Height of source rectangle */
    int	    	  dstx;	    	    /* X of destination rectangle */
    int	    	  dsty;	    	    /* Y of destination rectangle */
    unsigned long plane;    	    /* Plane to copy (0 for CopyArea) */
    void    	  (*copyProc)();    /* Procedure to call to perform the copy */
    RegionPtr	  *ppRgn;	    /* resultant Graphics Expose region */
{
    RegionPtr 	    	pRgnExp;    /* Exposed region */
    RegionPtr	  	pRgnObs;    /* Obscured region */
    BoxRec	  	box;	    /* Source box (screen coord) */
    PROLOGUE(pWin);
    struct BoxDraw {
	BoxPtr	  	pBox;	    	/* Source box */
	enum {
	    win, pix
	}   	  	source;	    	/* Place from which to copy */
    }	    	  	*boxes;	    /* Array of box/drawable pairs covering
				     * source box. */
    int  	  	*sequence;  /* Sequence of boxes to move */
    register int  	i, j, k, l, y;
    register BoxPtr	pBox;
    PixmapPtr	  	pBackingPixmap;
    int	    	  	dx, dy, nrects;
    Bool    	  	graphicsExposures;
    RegionPtr	  	(*pixCopyProc)();
    
    /*
     * Create a region of exposed boxes in pRgnExp.
     */
    box.x1 = srcx + pWin->absCorner.x;
    box.x2 = box.x1 + w;
    box.y1 = srcy + pWin->absCorner.y;
    box.y2 = box.y1 + h;
    
    pRgnExp = (*pGC->pScreen->RegionCreate) (&box, 1);
    (*pGC->pScreen->Intersect) (pRgnExp, pRgnExp, pWin->clipList);
    pRgnObs = (*pGC->pScreen->RegionCreate) (NULL, 1);
    (* pGC->pScreen->Inverse) (pRgnObs, pRgnExp, &box);

    /*
     * Translate regions into window coordinates for proper calls
     * to the copyProc, then make sure none of the obscured region sticks
     * into invalid areas of the backing pixmap.
     */
    (*pGC->pScreen->TranslateRegion) (pRgnExp,
				      -pWin->absCorner.x,
				      -pWin->absCorner.y);
    (*pGC->pScreen->TranslateRegion) (pRgnObs,
				      -pWin->absCorner.x,
				      -pWin->absCorner.y);
    (*pGC->pScreen->Intersect) (pRgnObs, pRgnObs, pBackingStore->pSavedRegion);

    /*
     * If the obscured region is empty, there's no point being fancy.
     */
    if (!(*pGC->pScreen->RegionNotEmpty) (pRgnObs))
    {
	(*pGC->pScreen->RegionDestroy) (pRgnExp);
	(*pGC->pScreen->RegionDestroy) (pRgnObs);
	return (FALSE);
    }

    pBackingPixmap = pBackingStore->pBackingPixmap;

    nrects = pRgnExp->numRects + pRgnObs->numRects;
    
    boxes = (struct BoxDraw *)ALLOCATE_LOCAL(nrects * sizeof(struct BoxDraw));
    sequence = (int *) ALLOCATE_LOCAL(nrects * sizeof(int));
    *ppRgn = NULL;

    if ((boxes == (struct BoxDraw *)NULL) || (sequence == (int *)NULL))
    {
	(*pGC->pScreen->RegionDestroy) (pRgnExp);
	(*pGC->pScreen->RegionDestroy) (pRgnObs);
	return(TRUE);
    }

    /*
     * Order the boxes in the two regions so we know from which drawable
     * to copy which box, storing the result in the boxes array
     */
    for (i = 0, j = 0, k = 0;
	 (i < pRgnExp->numRects) && (j < pRgnObs->numRects);
	 k++)
    {
	if (pRgnExp->rects[i].y1 < pRgnObs->rects[j].y1)
	{
	    boxes[k].pBox = &pRgnExp->rects[i];
	    boxes[k].source = win;
	    i++;
	}
	else if ((pRgnObs->rects[j].y1 < pRgnExp->rects[i].y1) ||
		 (pRgnObs->rects[j].x1 < pRgnExp->rects[i].x1))
	{
	    boxes[k].pBox = &pRgnObs->rects[j];
	    boxes[k].source = pix;
	    j++;
	}
	else
	{
	    boxes[k].pBox = &pRgnExp->rects[i];
	    boxes[k].source = win;
	    i++;
	}
    }

    /*
     * Catch any leftover boxes from either region (note that only
     * one can have leftover boxes...)
     */
    if (i != pRgnExp->numRects)
    {
	do
	{
	    boxes[k].pBox = &pRgnExp->rects[i];
	    boxes[k].source = win;
	    i++;
	    k++;
	} while (i < pRgnExp->numRects);

    }
    else
    {
	do
	{
	    boxes[k].pBox = &pRgnObs->rects[j];
	    boxes[k].source = pix;
	    j++;
	    k++;
	} while (j < pRgnObs->numRects);
    }
    
    if (dsty <= srcy)
    {
	/*
	 * Scroll up or vertically stationary, so vertical order is ok.
	 */
	if (dstx <= srcx)
	{
	    /*
	     * Scroll left or horizontally stationary, so horizontal order
	     * is ok as well.
	     */
	    for (i = 0; i < nrects; i++)
	    {
		sequence[i] = i;
	    }
	}
	else
	{
	    /*
	     * Scroll right. Need to reverse the rectangles within each
	     * band.
	     */
	    for (i = 0, j = 1, k = 0;
		 i < nrects;
		 j = i + 1, k = i)
	    {
		y = boxes[i].pBox->y1;
		while ((j < nrects) && (boxes[j].pBox->y1 == y))
		{
		    j++;
		}
		for (j--; j >= k; j--, i++)
		{
		    sequence[i] = j;
		}
	    }
	}
    }
    else
    {
	/*
	 * Scroll down. Must reverse vertical banding, at least.
	 */
	if (dstx < srcx)
	{
	    /*
	     * Scroll left. Horizontal order is ok.
	     */
	    for (i = nrects - 1, j = i - 1, k = i, l = 0;
		 i >= 0;
		 j = i - 1, k = i)
	    {
		/*
		 * Find extent of current horizontal band, then reverse
		 * the order of the whole band.
		 */
		y = boxes[i].pBox->y1;
		while ((j >= 0) && (boxes[j].pBox->y1 == y))
		{
		    j--;
		}
		for (j++; j <= k; j++, i--, l++)
		{
		    sequence[l] = j;
		}
	    }
	}
	else
	{
	    /*
	     * Scroll right or horizontal stationary.
	     * Reverse horizontal order as well (if stationary, horizontal
	     * order can be swapped without penalty and this is faster
             * to compute).
	     */
	    for (i = 0, j = nrects - 1; i < nrects; i++, j--)
	    {
		sequence[i] = j;
	    }
	}
    }
	    
    /*
     * XXX: To avoid getting multiple NoExpose events from this operation,
     * we turn OFF graphicsExposures in the gc and deal with any uncopied
     * areas later, if there's something not in backing-store.
     */
    graphicsExposures = pGC->graphicsExposures;
    pGC->graphicsExposures = FALSE;
    
    dx = dstx - srcx;
    dy = dsty - srcy;

    /*
     * Figure out which copy procedure to use from the backing GC. Note we
     * must do this because some implementations (sun's, e.g.) have
     * pBackingGC a fake GC with the real one below it, thus the devPriv for
     * pBackingGC won't be what the output library expects.
     */
    if (plane != 0)
    {
	pixCopyProc = pBackingGC->CopyPlane;
    }
    else
    {
	pixCopyProc = pBackingGC->CopyArea;
    }
    
    for (i = 0; i < nrects; i++)
    {
	pBox = boxes[sequence[i]].pBox;
	
	/*
	 * If we're copying from the pixmap, we need to place its contents
	 * onto the screen before scrolling the pixmap itself. If we're copying
	 * from the window, we need to copy its contents into the pixmap before
	 * we scroll the window itself.
	 */
	if (boxes[sequence[i]].source == pix)
	{
	    (void) (* copyProc) (pBackingPixmap, pWin, pGC,
			  pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	    (void) (* pixCopyProc) (pBackingPixmap, pBackingPixmap, pBackingGC,
			     pBox->x1, pBox->y1,
			     pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			     pBox->x1 + dx, pBox->y1 + dy, plane);
	}
	else
	{
	    (void) (* pixCopyProc) (pWin, pBackingPixmap, pBackingGC,
			     pBox->x1, pBox->y1,
			     pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			     pBox->x1 + dx, pBox->y1 + dy, plane);
	    (void) (* copyProc) (pWin, pWin, pGC,
			  pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	}
    }
    DEALLOCATE_LOCAL(boxes);
    DEALLOCATE_LOCAL(sequence);

    pGC->graphicsExposures = graphicsExposures;
    if (graphicsExposures)
    {
	/*
	 * Form union of rgnExp and rgnObs and see if covers entire area
	 * to be copied.  Store the resultant region for miBSCopyArea
	 * to return to dispatch which will send the appropriate expose
	 * events.
	 */
	(* pGC->pScreen->Union) (pRgnExp, pRgnExp, pRgnObs);
	box.x1 = srcx;
	box.x2 = srcx + w;
	box.y1 = srcy;
	box.y2 = srcy + h;
	if ((* pGC->pScreen->RectIn) (pRgnExp, &box) == rgnIN)
	    (*pGC->pScreen->RegionEmpty) (pRgnExp);
	else
	    (* pGC->pScreen->Inverse) (pRgnExp, pRgnExp, &box);
	*ppRgn = pRgnExp;
    }
    else
    {
	(*pGC->pScreen->RegionDestroy) (pRgnExp);
    }
    (*pGC->pScreen->RegionDestroy) (pRgnObs);
    return (TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * miBSCopyArea --
 *	Perform a CopyArea from the source to the destination, extracting
 *	from the source's backing-store and storing into the destination's
 *	backing-store without messing anything up. If the source and
 *	destination are different, there's not too much to worry about:
 *	we can just issue several calls to the regular CopyArea function.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
RegionPtr
miBSCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    BoxPtr	pExtents;
    long	dx, dy;
    int		bsrcx, bsrcy, bw, bh, bdstx, bdsty;
    RegionPtr	pixExposed = 0, winExposed = 0;

    PROLOGUE(pDst);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
	if ((pSrc != pDst) ||
	    (!miBSDoCopy((WindowPtr)pSrc, pGC, srcx, srcy, w, h, dstx, dsty,
			 (unsigned long) 0, pPriv->CopyArea, &winExposed)))
	{
	    /*
	     * always copy to the backing store first, miBSDoCopy
	     * returns FALSE if the *source* region is disjoint
	     * from the backing store saved region.  So, copying
	     * *to* the backing store is always safe
	     */
	    if (pGC->clientClipType != CT_PIXMAP)
	    {
		/*
		 * adjust srcx, srcy, w, h, dstx, dsty to be clipped to
		 * the backing store.  An unnecessary optimisation,
		 * but a useful one when GetSpans is slow.
		 */
		pExtents = (*pDst->pScreen->RegionExtents)
			(pBackingGC->clientClip);
		bsrcx = srcx;
		bsrcy = srcy;
		bw = w;
		bh = h;
		bdstx = dstx;
		bdsty = dsty;
		dx = pExtents->x1 - bdstx;
		if (dx > 0)
		{
		    bsrcx += dx;
		    bdstx += dx;
		    bw -= dx;
		}
		dy = pExtents->y1 - bdsty;
		if (dy > 0)
		{
		    bsrcy += dy;
		    bdsty += dy;
		    bh -= dy;
		}
		dx = (bdstx + bw) - pExtents->x2;
		if (dx > 0)
		    bw -= dx;
		dy = (bdsty + bh) - pExtents->y2;
		if (dy > 0)
		    bh -= dy;
		if (bw > 0 && bh > 0)
		    pixExposed = (* pBackingGC->CopyArea) (pSrc, 
				pBackingStore->pBackingPixmap, pBackingGC, 
				bsrcx, bsrcy, bw, bh, bdstx, bdsty);
	    }
	    else
		pixExposed = (* pBackingGC->CopyArea) (pSrc, 
				pBackingStore->pBackingPixmap, pBackingGC,
				srcx, srcy, w, h, dstx, dsty);

	    winExposed = (* pPriv->CopyArea) (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
	}
	pPriv->inUse = FALSE;
    }
    else
    {
	winExposed = (* pPriv->CopyArea) (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
    }
    /*
     * compute the composite graphics exposure region
     */
    if (winExposed)
    {
	if (pixExposed){
	    (*pDst->pScreen->Union) (winExposed, winExposed, pixExposed);
	    (*pDst->pScreen->RegionDestroy) (pixExposed);
	}
    } else
	winExposed = pixExposed;
    return winExposed;
}

/*-
 *-----------------------------------------------------------------------
 * miBSCopyPlane --
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
RegionPtr
miBSCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    register GC   *pGC;
    int     	  srcx,
		  srcy;
    int     	  w,
		  h;
    int     	  dstx,
		  dsty;
    unsigned long  plane;
{
    BoxPtr	pExtents;
    long	dx, dy;
    int		bsrcx, bsrcy, bw, bh, bdstx, bdsty;
    RegionPtr	winExposed = 0, pixExposed = 0;

    PROLOGUE(pDst);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
	if ((pSrc != pDst) ||
	    (!miBSDoCopy((WindowPtr)pSrc, pGC, srcx, srcy, w, h, dstx, dsty,
			 plane,  pPriv->CopyPlane, &winExposed)))
	{
	    /*
	     * always copy to the backing store first, miBSDoCopy
	     * returns FALSE if the *source* region is disjoint
	     * from the backing store saved region.  So, copying
	     * *to* the backing store is always safe
	     */
	    if (pGC->clientClipType != CT_PIXMAP)
	    {
		/*
		 * adjust srcx, srcy, w, h, dstx, dsty to be clipped to
		 * the backing store.  An unnecessary optimisation,
		 * but a useful one when GetSpans is slow.
		 */
		pExtents = (*pDst->pScreen->RegionExtents) (pBackingGC->clientClip);
		bsrcx = srcx;
		bsrcy = srcy;
		bw = w;
		bh = h;
		bdstx = dstx;
		bdsty = dsty;
		dx = pExtents->x1 - bdstx;
		if (dx > 0)
		{
		    bsrcx += dx;
		    bdstx += dx;
		    bw -= dx;
		}
		dy = pExtents->y1 - bdsty;
		if (dy > 0)
		{
		    bsrcy += dy;
		    bdsty += dy;
		    bh -= dy;
		}
		dx = (bdstx + bw) - pExtents->x2;
		if (dx > 0)
		    bw -= dx;
		dy = (bdsty + bh) - pExtents->y2;
		if (dy > 0)
		    bh -= dy;
		if (bw > 0 && bh > 0)
		    pixExposed = (* pBackingGC->CopyPlane) (pSrc, 
					pBackingStore->pBackingPixmap,
					pBackingGC, bsrcx, bsrcy, bw, bh,
					bdstx, bdsty, plane);
	    }
	    else
		pixExposed = (* pBackingGC->CopyPlane) (pSrc, 
					pBackingStore->pBackingPixmap,
					pBackingGC, srcx, srcy, w, h,
					dstx, dsty, plane);

	    winExposed = (* pPriv->CopyPlane) (pSrc, pDst, pGC, srcx, srcy, w, h,
				  dstx, dsty, plane);
	    
	}
	pPriv->inUse = FALSE;
    }
    else
    {
	winExposed = (* pPriv->CopyPlane) (pSrc, pDst, pGC, srcx, srcy, w, h,
			      dstx, dsty, plane);
    }
    /*
     * compute the composite graphics exposure region
     */
    if (winExposed)
    {
	if (pixExposed)
	{
	    (*pDst->pScreen->Union) (winExposed, winExposed, pixExposed);
	    (*pDst->pScreen->RegionDestroy) (pixExposed);
	}
    } else
	winExposed = pixExposed;
    return winExposed;
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyPoint --
 *	Perform a PolyPoint, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolyPoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
    PROLOGUE(pDrawable);

    if (!pPriv->inUse)
    {
	xPoint	  *pptCopy;

	pPriv->inUse = TRUE;

	pptCopy = (xPoint *)ALLOCATE_LOCAL(npt*sizeof(xPoint));
	bcopy((char *)pptInit,(char *)pptCopy,npt*sizeof(xPoint));

	(* pPriv->PolyPoint) (pDrawable, pGC, mode, npt, pptInit);

	(* pBackingGC->PolyPoint) (pBackingStore->pBackingPixmap,
				   pBackingGC, mode, npt, pptCopy);

	DEALLOCATE_LOCAL(pptCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
	(* pPriv->PolyPoint) (pDrawable, pGC, mode, npt, pptInit);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolylines --
 *	Perform a Polylines, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolylines (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode;
    int	    	  npt;
    DDXPointPtr	  pptInit;
{
    PROLOGUE(pDrawable);

    if (!pPriv->inUse)
    {
	DDXPointPtr	pptCopy;

	pPriv->inUse = TRUE;

	pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(npt*sizeof(DDXPointRec));
	bcopy((char *)pptInit,(char *)pptCopy,npt*sizeof(DDXPointRec));

	(* pPriv->Polylines)(pDrawable, pGC, mode, npt, pptInit);
	(* pBackingGC->Polylines)(pBackingStore->pBackingPixmap,
				  pBackingGC, mode, npt, pptCopy);
	DEALLOCATE_LOCAL(pptCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->Polylines)(pDrawable, pGC, mode, npt, pptInit);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolySegment --
 *	Perform a PolySegment, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolySegment(pDraw, pGC, nseg, pSegs)
    DrawablePtr pDraw;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	xSegment	*pSegsCopy;

	pPriv->inUse = TRUE;

	pSegsCopy = (xSegment *)ALLOCATE_LOCAL(nseg*sizeof(xSegment));
	bcopy((char *)pSegs,(char *)pSegsCopy,nseg*sizeof(xSegment));

	(* pPriv->PolySegment)(pDraw, pGC, nseg, pSegs);
	(* pBackingGC->PolySegment)(pBackingStore->pBackingPixmap,
				    pBackingGC, nseg, pSegsCopy);

	DEALLOCATE_LOCAL(pSegsCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolySegment)(pDraw, pGC, nseg, pSegs);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyRectangle --
 *	Perform a PolyRectangle, routing output to backing-store as needed.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolyRectangle(pDraw, pGC, nrects, pRects)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	xRectangle	*pRectsCopy;

	pPriv->inUse = TRUE;

	pRectsCopy =(xRectangle *)ALLOCATE_LOCAL(nrects*sizeof(xRectangle));
	bcopy((char *)pRects,(char *)pRectsCopy,nrects*sizeof(xRectangle));

	(* pPriv->PolyRectangle)(pDraw, pGC, nrects, pRects);
	(* pBackingGC->PolyRectangle)(pBackingStore->pBackingPixmap,
				      pBackingGC, nrects, pRectsCopy);

	DEALLOCATE_LOCAL(pRectsCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolyRectangle)(pDraw, pGC, nrects, pRects);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyArc --
 *	Perform a PolyArc, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	xArc  *pArcsCopy;

	pPriv->inUse = TRUE;

	pArcsCopy = (xArc *)ALLOCATE_LOCAL(narcs*sizeof(xArc));
	bcopy((char *)parcs,(char *)pArcsCopy,narcs*sizeof(xArc));

	(* pPriv->PolyArc)(pDraw, pGC, narcs, parcs);
	(* pBackingGC->PolyArc)(pBackingStore->pBackingPixmap, pBackingGC,
				narcs, pArcsCopy);

	DEALLOCATE_LOCAL(pArcsCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolyArc)(pDraw, pGC, narcs, parcs);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSFillPolygon --
 *	Perform a FillPolygon, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSFillPolygon(pDraw, pGC, shape, mode, count, pPts)
    DrawablePtr		pDraw;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	DDXPointPtr	pPtsCopy;

	pPriv->inUse = TRUE;

	pPtsCopy = (DDXPointPtr)ALLOCATE_LOCAL(count*sizeof(DDXPointRec));
	bcopy((char *)pPts,(char *)pPtsCopy,count*sizeof(DDXPointRec));

	(* pPriv->FillPolygon)(pDraw, pGC, shape, mode, count, pPts);
	(* pBackingGC->FillPolygon)(pBackingStore->pBackingPixmap,
				    pBackingGC, shape, mode,
				    count, pPtsCopy);

	DEALLOCATE_LOCAL(pPtsCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->FillPolygon)(pDraw, pGC, shape, mode, count, pPts);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyFillRect --
 *	Perform a PolyFillRect, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    PROLOGUE(pDrawable);

    if (!pPriv->inUse)
    {
	xRectangle	*pRectCopy;

	pPriv->inUse = TRUE;

	pRectCopy =
	    (xRectangle *)ALLOCATE_LOCAL(nrectFill*sizeof(xRectangle));
	bcopy((char *)prectInit,(char *)pRectCopy,
	      nrectFill*sizeof(xRectangle));

	(* pPriv->PolyFillRect)(pDrawable, pGC, nrectFill, prectInit);
	(* pBackingGC->PolyFillRect)(pBackingStore->pBackingPixmap,
				     pBackingGC, nrectFill, pRectCopy);

	DEALLOCATE_LOCAL(pRectCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolyFillRect)(pDrawable, pGC, nrectFill, prectInit);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyFillArc --
 *	Perform a PolyFillArc, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	xArc  *pArcsCopy;

	pPriv->inUse = TRUE;

	pArcsCopy = (xArc *)ALLOCATE_LOCAL(narcs*sizeof(xArc));
	bcopy((char *)parcs,(char *)pArcsCopy,narcs*sizeof(xArc));

	(* pPriv->PolyFillArc)(pDraw, pGC, narcs, parcs);
	(* pBackingGC->PolyFillArc)(pBackingStore->pBackingPixmap,
				    pBackingGC, narcs, pArcsCopy);

	DEALLOCATE_LOCAL(pArcsCopy);

	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolyFillArc)(pDraw, pGC, narcs, parcs);
    }
}


/*-
 *-----------------------------------------------------------------------
 * miBSPolyText8 --
 *	Perform a PolyText8, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
 int
miBSPolyText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
        (* pPriv->PolyText8)(pDraw, pGC, x, y, count, chars);
	(* pBackingGC->PolyText8)(pBackingStore->pBackingPixmap,
				  pBackingGC, x, y, count, chars);
	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolyText8)(pDraw, pGC, x, y, count, chars);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyText16 --
 *	Perform a PolyText16, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
 int
miBSPolyText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
        (* pPriv->PolyText16)(pDraw, pGC, x, y, count, chars);
	(* pBackingGC->PolyText16)(pBackingStore->pBackingPixmap,
				   pBackingGC, x, y, count, chars);
	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolyText16)(pDraw, pGC, x, y, count, chars);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageText8 --
 *	Perform a ImageText8, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSImageText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
        (* pPriv->ImageText8)(pDraw, pGC, x, y, count, chars);
	(* pBackingGC->ImageText8)(pBackingStore->pBackingPixmap,
				   pBackingGC, x, y, count, chars);
	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->ImageText8)(pDraw, pGC, x, y, count, chars);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageText16 --
 *	Perform a ImageText16, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSImageText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    PROLOGUE(pDraw);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
        (* pPriv->ImageText16)(pDraw, pGC, x, y, count, chars);
	(* pBackingGC->ImageText16)(pBackingStore->pBackingPixmap,
				    pBackingGC, x, y, count, chars);
	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->ImageText16)(pDraw, pGC, x, y, count, chars);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageGlyphBlt --
 *	Perform a ImageGlyphBlt, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
    PROLOGUE(pDrawable);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
        (* pPriv->ImageGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci,
				 pglyphBase);
	(* pBackingGC->ImageGlyphBlt)(pBackingStore->pBackingPixmap,
				      pBackingGC, x, y, nglyph, ppci,
				      pglyphBase);
	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->ImageGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci,
				 pglyphBase);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyGlyphBlt --
 *	Perform a PolyGlyphBlt, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
    PROLOGUE(pDrawable);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
        (* pPriv->PolyGlyphBlt)(pDrawable, pGC, x, y, nglyph,
				ppci, pglyphBase);
	(* pBackingGC->PolyGlyphBlt)(pBackingStore->pBackingPixmap,
				     pBackingGC, x, y, nglyph, ppci,
				     pglyphBase);
	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PolyGlyphBlt)(pDrawable, pGC, x, y, nglyph,
				ppci, pglyphBase);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSPushPixels --
 *	Perform a PushPixels, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
miBSPushPixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
    PROLOGUE(pDst);

    if (!pPriv->inUse)
    {
	pPriv->inUse = TRUE;
        (* pPriv->PushPixels)(pGC, pBitMap, pDst, w, h, x, y);
	(* pBackingGC->PushPixels)(pBackingGC, pBitMap,
				   pBackingStore->pBackingPixmap, w, h,
				   x, y);
	pPriv->inUse = FALSE;
    }
    else
    {
        (* pPriv->PushPixels)(pGC, pBitMap, pDst, w, h, x, y);
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSClearToBackground --
 *	Clear the given area of the backing pixmap with the background of
 *	the window, whatever it is. If generateExposures is TRUE, generate
 *	exposure events for the area. Note that if the area has any
 *	part outside the saved portions of the window, we do not allow the
 *	count in the expose events to be 0, since there will be more
 *	expose events to come.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Areas of pixmap are cleared and Expose events are generated.
 *
 *-----------------------------------------------------------------------
 */
void
miBSClearToBackground(pWin, x, y, w, h, generateExposures)
    WindowPtr	  	pWin;
    int	    	  	x;
    int	    	  	y;
    int	    	  	w;
    int	    	  	h;
    Bool    	  	generateExposures;
{
    RegionPtr	  	pRgn;
    int	    	  	i;
    MIBackingStorePtr	pBackingStore;
    ScreenPtr	  	pScreen;
    GCPtr   	  	pGC;
    int	    	  	ts_x_origin,
			ts_y_origin;
    XID	    	  	gcvalues[4];
    unsigned long 	gcmask;
    xRectangle	  	*rects;
    BoxPtr  	  	pBox;
    BoxRec  	  	box;
    unsigned long 	backgroundPixel;
    PixmapPtr		backgroundTile;

    pBackingStore = (MIBackingStorePtr)pWin->devBackingStore;
    pScreen = pWin->drawable.pScreen;

    if (pBackingStore->status == StatusNoPixmap)
	return;
    
    if (w == 0)
	w = pWin->clientWinSize.width - x;
    if (h == 0)
	h = pWin->clientWinSize.height - y;

    box.x1 = x;
    box.y1 = y;
    box.x2 = x + w;
    box.y2 = y + h;
    pRgn = (*pWin->drawable.pScreen->RegionCreate)(&box, 1);

    (* pScreen->Intersect) (pRgn, pRgn, pBackingStore->pSavedRegion);

    if ((* pScreen->RegionNotEmpty) (pRgn))
    {
	/*
	 * if clearing entire window, simply make new virtual
	 * tile.  For the root window, we also destroy the pixmap
	 * to save a pile of memory
	 */
	if (x == 0 && y == 0 &&
 	    w == pWin->clientWinSize.width &&
 	    h == pWin->clientWinSize.height)
	{
	    if (!pWin->parent)
		miDestroyBSPixmap (pWin);
	    if (pBackingStore->status != StatusContents)
		 miTileVirtualBS (pWin);
	}

	backgroundPixel = pWin->backgroundPixel;
	backgroundTile = pWin->backgroundTile;
	ts_x_origin = ts_y_origin = 0;

	if (backgroundTile == (PixmapPtr) ParentRelative) {
	    WindowPtr	pParent;

	    pParent = pWin;
	    while (pParent->backgroundTile == (PixmapPtr) ParentRelative) {
		ts_x_origin -= pParent->clientWinSize.x;
		ts_y_origin -= pParent->clientWinSize.y;
		pParent = pParent->parent;
	    }
	    backgroundTile = pParent->backgroundTile;
	    backgroundPixel = pParent->backgroundPixel;
	}

	if ((backgroundTile != (PixmapPtr)None) &&
	    ((pBackingStore->status == StatusContents) ||
	     (pBackingStore->backgroundTile != backgroundTile) ||
	     (pBackingStore->backgroundPixel != backgroundPixel)))
	{
	    if (pBackingStore->pBackingPixmap == NullPixmap)
		miCreateBSPixmap(pWin);

	    if (pBackingStore->pBackingPixmap != NullPixmap)
	    {
		/*
		 * First take care of any ParentRelative stuff by altering the
		 * tile/stipple origin to match the coordinates of the upper-left
		 * corner of the first ancestor without a ParentRelative background.
		 * This coordinate is, of course, negative.
		 */
		pGC = GetScratchGC(pWin->drawable.depth, pScreen);
	    
		if (backgroundTile == (PixmapPtr) USE_BACKGROUND_PIXEL)
		{
		    gcvalues[0] = (XID) backgroundPixel;
		    gcvalues[1] = FillSolid;
		    gcmask = GCForeground|GCFillStyle;
		}
		else
		{
		    gcvalues[0] = FillTiled;
		    gcvalues[1] = (XID) backgroundTile;
		    gcmask = GCFillStyle|GCTile;
		}
		gcvalues[2] = ts_x_origin;
		gcvalues[3] = ts_y_origin;
		gcmask |= GCTileStipXOrigin|GCTileStipYOrigin;
		DoChangeGC(pGC, gcmask, gcvalues, TRUE);
		ValidateGC((DrawablePtr)pBackingStore->pBackingPixmap, pGC);
    
		/*
		 * Figure out the array of rectangles to fill and fill them with
		 * PolyFillRect in the proper mode, as set in the GC above.
		 */
		rects = (xRectangle *)ALLOCATE_LOCAL(pRgn->numRects*sizeof(xRectangle));
	    
		for (i = 0, pBox = pRgn->rects; i < pRgn->numRects; i++, pBox++)
 		{
		    rects[i].x = pBox->x1;
		    rects[i].y = pBox->y1;
		    rects[i].width = pBox->x2 - pBox->x1;
		    rects[i].height = pBox->y2 - pBox->y1;
		}
		(* pGC->PolyFillRect) (pBackingStore->pBackingPixmap,
				   pGC, pRgn->numRects, rects);
	
		FreeScratchGC(pGC);
		DEALLOCATE_LOCAL(rects);
	    }
	}	

	if (generateExposures) {
	    int	  offset;
	    xEvent *events, *ev;

	    /*
	     * If there are exposed areas in the box the client wanted cleared,
	     * make sure u.expose.count doesn't go to 0. Note that the count
	     * is only a hint, the only guarantee being the last Expose event
	     * from a single operation has a count of 0, so this numbering
	     * scheme is ok.
	     */
	    offset = (((* pScreen->RectIn) (pRgn, &box) != rgnIN)?1:0);

	    events = (xEvent *)ALLOCATE_LOCAL(pRgn->numRects*sizeof(xEvent));
	    for (i = pRgn->numRects-1, pBox = pRgn->rects, ev = events;
		 i >= 0;
		 i--, pBox++, ev++)
	    {
		ev->u.u.type = Expose;
		ev->u.expose.window = pWin->wid;
		ev->u.expose.x = pBox->x1;
		ev->u.expose.y = pBox->y1;
		ev->u.expose.width = pBox->x2 - pBox->x1;
		ev->u.expose.height = pBox->y2 - pBox->y1;
		ev->u.expose.count = i + offset;
	    }
	    DeliverEvents(pWin, events, pRgn->numRects, NullWindow);
	    DEALLOCATE_LOCAL(events);
	}
    }

    (* pScreen->RegionDestroy) (pRgn);
}

/*-
 *-----------------------------------------------------------------------
 * miInitBackingStore --
 *	Creates the backing-store information for a window but doesn't
 *	install it.
 *
 * Results:
 *	A pointer to an MIBackingStore structure that may be installed
 *	in the window.
 *
 * Side Effects:
 *	Pixmaps and GC's are allocated.
 *
 *-----------------------------------------------------------------------
 */
void
miInitBackingStore(pWin, SaveAreas, RestoreAreas, SetClipmaskRgn)
    WindowPtr 	  pWin;
    void    	  (*SaveAreas)();
    void    	  (*RestoreAreas)();
    void    	  (*SetClipmaskRgn)();
{
    register MIBackingStorePtr pBackingStore;
    register ScreenPtr 	    pScreen;
    int     	  	    status;
    BackingStorePtr	    pBS;
	

    if ((pWin->devBackingStore == (pointer)NULL) &&
	(pWin->drawable.pScreen->backingStoreSupport != NotUseful))
    {
	XID false = xFalse;

	pScreen = pWin->drawable.pScreen;
	pBackingStore = (MIBackingStorePtr)Xalloc(sizeof(MIBackingStoreRec));
	pBackingStore->pBackingPixmap = NullPixmap;
	pBackingStore->pSavedRegion = (* pScreen->RegionCreate)(NullBox, 1);
	pBackingStore->pgcBlt = CreateGC((DrawablePtr)pWin,
					 (BITS32)GCGraphicsExposures, &false,
					 &status);
	pBackingStore->SaveAreas = SaveAreas;
	pBackingStore->RestoreAreas = RestoreAreas;
	pBackingStore->SetClipmaskRgn = SetClipmaskRgn;
	pBackingStore->viewable = (Bool)pWin->viewable;
	pBackingStore->status = StatusNoPixmap;
	pBackingStore->backgroundTile = NullPixmap;
	
	pWin->devBackingStore = (pointer)pBackingStore;

	pBS = (BackingStorePtr) Xalloc(sizeof(BackingStoreRec));
	pBS->obscured = (* pScreen->RegionCreate) (NULL, 1);
	pBS->SaveDoomedAreas = 	    	miSaveAreas;
	pBS->RestoreAreas = 	    	miRestoreAreas;
	pBS->ExposeCopy =	    	miExposeCopy;
	pBS->TranslateBackingStore = 	miTranslateBackingStore;
	pBS->ClearToBackground =    	miBSClearToBackground;
	pBS->DrawGuarantee =		miBSDrawGuarantee;

	pWin->backStorage = pBS;

	/*
	 * Now want to initialize the backing pixmap and pSavedRegion if
	 * necessary. The initialization consists of finding all the
	 * currently-obscured regions, by taking the inverse of the window's
	 * clip list, storing the result in pSavedRegion, and exposing those
	 * areas of the window.
	 */

	if (((pWin->backingStore & 3) == WhenMapped && pWin->viewable) ||
	    ((pWin->backingStore & 3) == Always))
 	{
	    BoxPtr	pBox;
	    BoxRec  	box;
	    RegionPtr	pSavedRegion;
	    xEvent	*pEvent, *pe;
	    int		i;

	    pSavedRegion = pBackingStore->pSavedRegion;

	    box.x1 = pWin->absCorner.x;
	    box.x2 = box.x1 + pWin->clientWinSize.width;
	    box.y1 = pWin->absCorner.y;
	    box.y2 = pWin->absCorner.y + pWin->clientWinSize.height;

	    (* pScreen->Inverse)(pSavedRegion, pWin->clipList,  &box);
	    (* pScreen->TranslateRegion) (pSavedRegion,
					  -pWin->absCorner.x,
					  -pWin->absCorner.y);
	    miTileVirtualBS (pWin);
	    
	    /*
	     * deliver all the newly availible regions
	     * as exposure events to the window
	     */

	    pBox = pSavedRegion->rects;
        
	    if(!(pEvent = (xEvent *) ALLOCATE_LOCAL(pSavedRegion->numRects *
						    sizeof(xEvent))))
		return;

	    for (i=1, pe = pEvent;
		 i<=pSavedRegion->numRects;
		 i++, pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pWin->wid;
		pe->u.expose.x = pBox->x1;
		pe->u.expose.y = pBox->y1;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = (pSavedRegion->numRects - i);
	    }
	    DeliverEvents(pWin, pEvent, pSavedRegion->numRects, NullWindow);
	    DEALLOCATE_LOCAL(pEvent);
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * miFreeBackingStore --
 *	Destroy and free all the stuff associated with the backing-store
 *	for the given window.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The backing pixmap and all the regions and GC's are destroyed.
 *
 *-----------------------------------------------------------------------
 */
void
miFreeBackingStore(pWin)
    WindowPtr pWin;
{
    MIBackingStorePtr 	pBackingStore;
    register ScreenPtr	pScreen = pWin->drawable.pScreen;

    pBackingStore = (MIBackingStorePtr)pWin->devBackingStore;

    if (pBackingStore != (MIBackingStorePtr)NULL)
    {
	FreeGC(pBackingStore->pgcBlt);
	if (pBackingStore->pBackingPixmap)
	    miDestroyBSPixmap (pWin);

	(* pScreen->RegionDestroy)(pBackingStore->pSavedRegion);
	Xfree((pointer)pBackingStore);
	
	pWin->devBackingStore = (pointer)NULL;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miResizeBackingStore --
 *	Alter the size of the backing pixmap when the window changes
 *	size. The contents of the old pixmap are copied into the new
 *	one displaced by the given amounts. When copying, copies the
 *	bounding box of the saved regions, on the assumption that that
 *	is faster than copying the component boxes...?
 *
 * Results:
 *	The new Pixmap.
 *
 * Side Effects:
 *	The old pixmap is destroyed.
 *
 *-----------------------------------------------------------------------
 */
static void
miResizeBackingStore(pWin, dx, dy)
    WindowPtr 	  pWin;
    int	    	  dx,
		  dy;
{
    MIBackingStorePtr pBackingStore;
    PixmapPtr pBackingPixmap;
    ScreenPtr pScreen;
    GC	   *pGC;
    BoxPtr  extents;
    BoxRec pixbounds;
    RegionPtr prgnTmp;


    pBackingStore = (MIBackingStorePtr)(pWin->devBackingStore);
    pGC = pBackingStore->pgcBlt;
    pScreen = pWin->drawable.pScreen;
    pBackingPixmap = pBackingStore->pBackingPixmap;

    if (pBackingPixmap)
    {
	PixmapPtr pNewPixmap;

	pNewPixmap = (PixmapPtr)(*pScreen->CreatePixmap)
					(pScreen, 
					 pWin->clientWinSize.width, 
					 pWin->clientWinSize.height, 
					 pWin->drawable.depth);

	if (pNewPixmap)
	{
		ValidateGC((DrawablePtr)pNewPixmap, pGC);

		if ((* pScreen->RegionNotEmpty) (pBackingStore->pSavedRegion))
		{
		    extents = (*pScreen->RegionExtents)(pBackingStore->pSavedRegion);
		    (*pGC->CopyArea)(pBackingPixmap, pNewPixmap, pGC,
				     extents->x1, extents->y1,
				     extents->x2 - extents->x1,
				     extents->y2 - extents->y1,
				     extents->x1 + dx, extents->y1 + dy);
		}
	}
	else
	{
		pBackingStore->status = StatusNoPixmap;
	}

	(* pScreen->DestroyPixmap)(pBackingPixmap);
	pBackingStore->pBackingPixmap = pNewPixmap;
    }

    /*
     * Now we need to translate pSavedRegion, as appropriate, and clip it
     * to be within the window's new bounds.
     */
    if (dx || dy)
    {
	(* pWin->drawable.pScreen->TranslateRegion)
				(pBackingStore->pSavedRegion, dx, dy);
    }
    pixbounds.x1 = 0;
    pixbounds.x2 = pWin->clientWinSize.width;
    pixbounds.y1 = 0;
    pixbounds.y2 = pWin->clientWinSize.height;
    prgnTmp = (* pScreen->RegionCreate)(&pixbounds, 1);
    (* pScreen->Intersect)(pBackingStore->pSavedRegion,
			   pBackingStore->pSavedRegion,
			   prgnTmp);
    (* pScreen->RegionDestroy)(prgnTmp);
}

/*-
 *-----------------------------------------------------------------------
 * miSaveAreas --
 *	Saved the areas of the given window that are about to be
 *	obscured.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The region is copied from the screen into pBackingPixmap and
 *	pSavedRegion is updated.
 *
 *-----------------------------------------------------------------------
 */
static void
miSaveAreas(pWin)
    register WindowPtr pWin;
{
    MIBackingStorePtr 	pBackingStore;
    RegionPtr 		prgnDoomed;
    BoxRec		winBox;
    RegionPtr		winSize;
    ScreenPtr	  	pScreen;
    

    pBackingStore = (MIBackingStorePtr)pWin->devBackingStore;
    pScreen = pWin->drawable.pScreen;

    /*
     * If the window isn't realized, it's being unmapped, thus we don't
     * want to save anything if backingStore isn't Always. Note we only examine
     * the bottom 2 bits to account for the possibility of the save-under
     * mechanism in dix/window.c being in use.
     */
    if (!pWin->realized && ((pWin->backingStore & 3) != Always))
    {
	pBackingStore->viewable = pWin->viewable;
	(* pScreen->RegionEmpty) (pBackingStore->pSavedRegion);
	if (pBackingStore->pBackingPixmap)
	    miDestroyBSPixmap (pWin);
	return;
    }

    /*
     * Translate the region to be window-relative, then copy the region from
     * the screen at the window's (old) position. Note that if the window
     * has moved, backStorage->obscured is expected to be at the new
     * location and backStorage->oldAbsCorner is expected to contain the
     * window's previous location so the correct areas of the screen may
     * be copied...
     */
    prgnDoomed = pWin->backStorage->obscured;
    (* pScreen->TranslateRegion)(prgnDoomed, 
				 -pWin->absCorner.x,
				 -pWin->absCorner.y);
    /*
     * When a window shrinks, the obscured region will be larger than the
     * window actually is. To avoid wasted effort, therefore, we trim the
     * region to save to be within the boundaries of the window. We don't
     * bother with resizing the pixmap now, because....we don't feel like it.
     */
    winBox.x1 = 0;
    winBox.y1 = 0;
    winBox.x2 = pWin->clientWinSize.width;
    winBox.y2 = pWin->clientWinSize.height;
    winSize = (* pScreen->RegionCreate) (&winBox, 1);
    (* pScreen->Intersect) (prgnDoomed, prgnDoomed, winSize);
    (* pScreen->RegionDestroy) (winSize);

    /*
     * only save the bits if we've actually
     * started using backing store
     */
    if (pBackingStore->status != StatusVirtual)
    {

	if (pBackingStore->pBackingPixmap == NullPixmap)
	    miCreateBSPixmap (pWin);

	if (pBackingStore->pBackingPixmap != NullPixmap)
	    (* pBackingStore->SaveAreas) (pBackingStore->pBackingPixmap,
 	    				  prgnDoomed,
					  pWin->backStorage->oldAbsCorner.x,
					  pWin->backStorage->oldAbsCorner.y);
    }

    (* pScreen->Union)(pBackingStore->pSavedRegion,
		       pBackingStore->pSavedRegion,
		       prgnDoomed);

    pBackingStore->viewable = (int)pWin->viewable;

    /*
     * If you think of it the right way, we've just altered the clip-list for
     * the pixmap, so we need to update its serial number (plus the sun and
     * apollo ValidateGC functions clear out the GCClipMask from the
     * stateChanges for the GC, so unless we do this, the pBackingGC won't be
     * validated properly...
     */

    if (pBackingStore->pBackingPixmap != NullPixmap)
	pBackingStore->pBackingPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
}

/*-
 *-----------------------------------------------------------------------
 * miRestoreAreas --
 *	Restore areas from backing-store that are no longer obscured.
 *
 * Results:
 *	The region to generate exposure events on (which may be
 *	different from the region to paint).
 *
 * Side Effects:
 *	Areas are copied from pBackingPixmap to the screen. pWin->exposed
 *	is replaced with the region that could not be restored from
 *	backing-store.
 *
 * Notes:
 *	This is called before sending any exposure events to the client,
 *	and so might be called if the window has grown.  Changing the backing
 *	pixmap doesn't require revalidating the backingGC because the
 *	client's next output request will result in a call to ValidateGC,
 *	since the window clip region has changed, which will in turn call
 *	miValidateBackingStore.
 *	   it replaces pWin->exposed with the region that could not be
 *	restored from backing-store.
 *	   NOTE: this must be called with pWin->exposed window-relative.
 *-----------------------------------------------------------------------
 */
static RegionPtr
miRestoreAreas(pWin)
    register WindowPtr pWin;
{
    PixmapPtr pBackingPixmap;
    MIBackingStorePtr pBackingStore;
    RegionPtr prgnSaved;
    RegionPtr prgnExposed;
    RegionPtr prgnRestored;
    register ScreenPtr pScreen;
    RegionPtr exposures = pWin->exposed;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (MIBackingStorePtr)pWin->devBackingStore;
    pBackingPixmap = pBackingStore->pBackingPixmap;

    if (pBackingStore->status == StatusContents)
    {
	/*
	 * We can only restore things if we have a pixmap from which to do
	 * so...
	 */
	prgnSaved = pBackingStore->pSavedRegion;
	prgnExposed = pWin->exposed;
	prgnRestored = (* pScreen->RegionCreate)((BoxPtr)NULL, 1);
	
	(* pScreen->Intersect)(prgnRestored, prgnSaved, prgnExposed);
	
	/*
	 * Since pWin->exposed is no longer obscured, we no longer
	 * will have a valid copy of it in backing-store, but there is a valid
	 * copy of it on screen, so subtract the area we just restored from
	 * from the area to be exposed.
	 */

	(* pScreen->Subtract)(prgnSaved, prgnSaved, prgnExposed);
	(* pScreen->Subtract)(prgnExposed, prgnExposed, prgnRestored);
	
	/*
	 * Again, we've changed the clip-list for the backing pixmap, so update
	 * its serial number
	 */

	pBackingPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	
	/*
	 * Do the actual restoration after translating the region-to-be-
	 * restored into screen coordinates.
	 */

	(* pScreen->TranslateRegion) (prgnRestored,
				      pWin->absCorner.x,
				      pWin->absCorner.y);
	(* pBackingStore->RestoreAreas) (pBackingPixmap,
					 prgnRestored,
					 pWin->absCorner.x,
					 pWin->absCorner.y);
	
	(* pScreen->RegionDestroy)(prgnRestored);

	/*
	 * if the saved region is completely empty, dispose of the
	 * backing pixmap
	 */

	if (!(*pScreen->RegionNotEmpty) (prgnSaved))
	{
	    if (pBackingStore->pBackingPixmap)
		miDestroyBSPixmap (pWin);
	}
    }
    else if ((pBackingStore->status == StatusVirtual) ||
	     (pBackingStore->status == StatusVDirty))
    {
	exposures = (* pScreen->RegionCreate)(NullBox, 1);
	if (pBackingStore->backgroundTile == pWin->backgroundTile &&
	    pBackingStore->backgroundPixel == pWin->backgroundPixel)
	{
	    (* pScreen->Subtract)(exposures,
				  pWin->exposed,
				  pBackingStore->pSavedRegion);
	}
	else
	{
	    /* background has changed, virtually retile and expose */
	    if (IS_VALID_PIXMAP (pBackingStore->backgroundTile))
		(* pScreen->DestroyPixmap) (pBackingStore->backgroundTile);
	    miTileVirtualBS(pWin);

	    /* we need to expose all we have (virtually) retiled */
	    (* pScreen->Union) (exposures,
				pWin->exposed,
				pBackingStore->pSavedRegion);
	}
	(* pScreen->TranslateRegion) (exposures,
				      pWin->absCorner.x,
				      pWin->absCorner.y);
	(* pScreen->Subtract)(pBackingStore->pSavedRegion,
			      pBackingStore->pSavedRegion,
			      pWin->exposed);
    }
    else if (pWin->viewable && !pBackingStore->viewable &&
	     ((pWin->backingStore & 3) == WhenMapped))
    {
	/*
	 * The window was just mapped and nothing has been saved in
	 * backing-store from the last time it was mapped. We want to capture
	 * any output to regions that are already obscured but there are no
	 * bits to snag off the screen, so we initialize things just as we did
	 * in miInitBackingStore, above.
	 */
	BoxRec  box;
	
	prgnSaved = pBackingStore->pSavedRegion;

	box.x1 = pWin->absCorner.x;
	box.x2 = box.x1 + pWin->clientWinSize.width;
	box.y1 = pWin->absCorner.y;
	box.y2 = pWin->absCorner.y + pWin->clientWinSize.height;
	
	(* pScreen->Inverse)(prgnSaved, pWin->clipList,  &box);
	(* pScreen->TranslateRegion) (prgnSaved,
				      -pWin->absCorner.x,
				      -pWin->absCorner.y);
	miTileVirtualBS(pWin);

	exposures = (* pScreen->RegionCreate)(&box, 1);
    }
    pBackingStore->viewable = (int)pWin->viewable;
    return exposures;
}


/*-
 *-----------------------------------------------------------------------
 * miTranslateBackingStore --
 *	Shift the backing-store in the given direction. Called when bit
 *	gravity is shifting things around. 
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the window changed size as well as position, the backing pixmap
 *	is resized. The contents of the backing pixmap are shifted
 *
 *-----------------------------------------------------------------------
 */
static void
miTranslateBackingStore(pWin, dx, dy, oldClip)
    WindowPtr 	  pWin;
    int     	  dx;		/* translation distance */
    int     	  dy;
    RegionPtr	  oldClip;  	/* Region being copied */
{
    register MIBackingStorePtr 	pBackingStore;
    register RegionPtr 	    	pSavedRegion;
    register RegionPtr 	    	newSaved, obscured;
    register ScreenPtr		pScreen;
    BoxRec			extents;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (MIBackingStorePtr)(pWin->devBackingStore);
    if (pBackingStore->status == StatusNoPixmap)
	return;
    /* bit gravity makes things virtually too hard, punt */
    if (((dx != 0) || (dy != 0)) && (pBackingStore->status != StatusContents))
	miCreateBSPixmap(pWin);
    pSavedRegion = pBackingStore->pSavedRegion;
    if (!oldClip)
	(* pScreen->RegionEmpty) (pSavedRegion);
    newSaved = (* pScreen->RegionCreate) (NullBox, 1);
    obscured = pWin->backStorage->obscured;
    /* resize and translate backing pixmap and pSavedRegion */
    miResizeBackingStore(pWin, dx, dy);
    /* now find any already saved areas we should retain */
    if (pWin->viewable)
    {
	(* pScreen->RegionCopy) (newSaved, pWin->clipList);
	(* pScreen->TranslateRegion)(newSaved, -dx, -dy);
	(* pScreen->Intersect) (pSavedRegion, pSavedRegion, newSaved);
    }
    /* compute what the new pSavedRegion will be */
    extents.x1 = pWin->absCorner.x;
    extents.x2 = pWin->absCorner.x + pWin->clientWinSize.width;
    extents.y1 = pWin->absCorner.y;
    extents.y2 = pWin->absCorner.y + pWin->clientWinSize.height;
    (* pScreen->Inverse)(newSaved, pWin->clipList, &extents);
    /* now find any visible areas we can save from the screen */
    (* pScreen->TranslateRegion)(newSaved, -dx, -dy);
    if (oldClip)
    {
	(* pScreen->Intersect) (obscured, oldClip, newSaved);
	if ((* pScreen->RegionNotEmpty) (obscured))
	{
	    /* save those visible areas */
	    (* pScreen->TranslateRegion) (obscured, dx, dy);
	    pWin->backStorage->oldAbsCorner.x = pWin->absCorner.x - dx;
	    pWin->backStorage->oldAbsCorner.y = pWin->absCorner.y - dy;
	    miSaveAreas(pWin);
	}
    }
    /* translate newSaved to local coordinates */
    (* pScreen->TranslateRegion) (newSaved,
				  dx-pWin->absCorner.x,
				  dy-pWin->absCorner.y);
    /* subtract out what we already have saved */
    (* pScreen->Subtract) (obscured, newSaved, pSavedRegion);
    /* and expose whatever there is */
    if ((* pScreen->RegionNotEmpty) (obscured))
    {
	extents = *((* pScreen->RegionExtents) (obscured));
	pBackingStore->pSavedRegion = obscured;
	/* XXX there is a problem here.  The protocol requires that
	 * only the last exposure event in a series can have a zero
	 * count, but we might generate one here, and then another
	 * one will be generated if there are unobscured areas that
	 * are newly exposed.  Is it worth computing, or cheating?
	 */
	miBSClearToBackground(pWin, extents.x1, extents.y1,
			      extents.x2 - extents.x1,
			      extents.y2 - extents.y1,
			      TRUE);
	pBackingStore->pSavedRegion = pSavedRegion;
	(* pScreen->RegionEmpty) (obscured);
    }
    /* finally install new pSavedRegion */
    (* pScreen->Union) (pSavedRegion, pSavedRegion, newSaved);
    (* pScreen->RegionDestroy) (newSaved);
}

/*-
 *-----------------------------------------------------------------------
 * miBSDestroyGC --
 *	Destroy the private info associated with a graphics context that
 *	has been used with backing-store.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	All data for it are freed up.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSDestroyGC (pGC, pGCI)
    GCPtr   	  pGC;
    GCInterestPtr pGCI;
{
    MIBSGCPrivPtr pPriv = (MIBSGCPrivPtr) pGC->devBackingStore;
    
    FreeGC(pPriv->pBackingGC);
    Xfree(pGC->devBackingStore);
    Xfree((pointer)pGCI);
}

static
miBSInstallPriv (pGC)
    GCPtr	pGC;
{
    MIBSGCPrivPtr 	pPriv;

    pPriv = (MIBSGCPrivPtr)Xalloc(sizeof(MIBSGCPrivRec));
    pGC->devBackingStore = (pointer)pPriv;
    pPriv->inUse = FALSE;
    pPriv->gcHooked = FALSE;
    pPriv->changes = 0;
    pPriv->guarantee = GuaranteeNothing;
    pPriv->serialNumber = NEXT_SERIAL_NUMBER;
}

/*
 * Inform the backing store layer that you are about to validate
 * a gc with a window, and that subsequent output to the window
 * is (or is not) guaranteed to be already clipped to the visible
 * regions of the window.
 */

static void
miBSDrawGuarantee (pWin, pGC, guarantee)
    WindowPtr	pWin;
    GCPtr	pGC;
    int		guarantee;
{
    MIBSGCPrivPtr 	pPriv;

    if (pWin->backingStore != NotUseful && pWin->backStorage)
    {
	if (!pGC->devBackingStore)
	    miBSInstallPriv (pGC);
	pPriv = (MIBSGCPrivPtr)pGC->devBackingStore;
	pPriv->guarantee = guarantee;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miValidateBackingStore --
 *	Called from output-library's ValidateGC routine once it has done
 *	what it needs to do. Takes note of changes in the GC and updates
 *	pGC in the GC's devBackingStore accordingly.
 *	Once this function has been called when validating a GC, it must 
 *	be called every time the GC is validated. It's addictive...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Any changed procedure vectors are copied into our private data and
 *	replaced by our own procedures. The clip list in the GC used for
 *	drawing into the backing pixmap is altered to match the window's
 *	clip list and the clientClip specified in the GC. A GCInterest
 *	structure will be hung from the GC if this is the first time this
 *	function was called for it....etc. etc. etc.
 *
 * Notes:
 *	The idea here is to perform several functions:
 *	    - All the output calls must be intercepted and routed to
 *	      backing-store as necessary.
 *	    - pGC in the window's devBackingStore must be set up with the
 *	      clip list appropriate for writing to pBackingPixmap (i.e.
 *	      the inverse of the window's clipList intersected with the
 *	      clientClip of the GC). Since the destination for this GC is
 *	      a pixmap, it is sufficient to set the clip list as its
 *	      clientClip.
 *-----------------------------------------------------------------------
 */

void
miValidateBackingStore(pDrawable, pGC, procChanges)
    DrawablePtr   pDrawable;
    GCPtr   	  pGC;
    int	    	  procChanges;	/* Bits indicating changed output functions.
				 * Bitwise OR of MIBS_ constants */
{
    GCPtr   	  	pBackingGC;
    MIBackingStorePtr	pBackingStore;
    MIBSGCPrivPtr 	pPriv;
    WindowPtr		pWin;
    int	    	  	stateChanges;
    register int  	index, mask;
    int			lift_functions;
    RegionPtr		backingCompositeClip = NULL;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
        pWin = (WindowPtr) pDrawable;
	pBackingStore = (MIBackingStorePtr)pWin->devBackingStore;
	lift_functions = (pBackingStore == (MIBackingStorePtr)NULL);
    }
    else
    {
        pWin = (WindowPtr) NULL;
	lift_functions = TRUE;
    }

    pPriv = (MIBSGCPrivPtr)pGC->devBackingStore;

    /*
     * Figure out what part of the GC to copy: if it's the same GC as
     * used last time, copy only those things listed as changed in the
     * GC, else copy the whole thing. Note that setting stateChanges to all
     * ones forces reseting of pBackingGC's clientClip.
     */

    if (pPriv != (MIBSGCPrivPtr)NULL && pPriv->pBackingGC != (GCPtr) NULL)
    {
	stateChanges = pGC->stateChanges;
	pBackingGC = pPriv->pBackingGC;
    }
    else
    {
	int	  status;
	GCInterestPtr	pGCI;

	/*
	 * First create our private data for this GC, initializing everything
	 * but the procedure vectors.
	 */
	if (pPriv == (MIBSGCPrivPtr)NULL) {
	    miBSInstallPriv (pGC);
	    pPriv = (MIBSGCPrivPtr)pGC->devBackingStore;
	}
	pBackingGC = pPriv->pBackingGC = CreateGC(pDrawable, (BITS32)0,
						  (XID *)NULL, &status);

	/*
	 * Now set up an interest in the GC on destruction to allow us to
	 * free this data.
	 */
	pGCI = (GCInterestPtr)Xalloc(sizeof(GCInterestRec));
	pGCI->length = sizeof(GCInterestRec);
	pGCI->owner = 0;    /* Server owns this */
	pGCI->ValInterestMask = 0;
	pGCI->CopyGCSource =
	    pGCI->CopyGCDest =
		pGCI->ValidateGC = (void (*)())NULL;
	pGCI->ChangeGC = (int (*)())NULL;
	pGCI->ChangeInterestMask = 0;
	pGCI->DestroyGC = miBSDestroyGC;
	pGCI->extPriv = (pointer)NULL;
	InsertGCI(pGC, pGCI, GCI_FIRST, NULL);
	/*
	 * Mark everything as changed so initialization happens
	 */
	stateChanges = (1 << (GCLastBit+1)) - 1;
	procChanges = MIBS_ALLPROCS;
    }

    if (pPriv->guarantee == GuaranteeVisBack)
        lift_functions = TRUE;

    /*
     * check to see if a new backingCompositeClip region must
     * be generated
     */

    if (!lift_functions && 
        ((pDrawable->serialNumber != pPriv->serialNumber) ||
	 (stateChanges&(GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode))))
    {
	if ((*pGC->pScreen->RegionNotEmpty) (pBackingStore->pSavedRegion))
 	{
	
	    backingCompositeClip = (*pGC->pScreen->RegionCreate) (NULL, 1);
	    if ((pGC->clientClipType == CT_NONE) || 
		(pGC->clientClipType == CT_PIXMAP))
	    {
		(*pGC->pScreen->RegionCopy) (backingCompositeClip, pBackingStore->pSavedRegion); 
	    }
	    else
	    {
		/*
		 * Make a new copy of the client clip, translated to
		 * its proper origin.
		 */

		(*pGC->pScreen->RegionCopy) (backingCompositeClip, pGC->clientClip);
		(*pGC->pScreen->TranslateRegion) (backingCompositeClip,
						  pGC->clipOrg.x,
						  pGC->clipOrg.y);
		(*pGC->pScreen->Intersect) (backingCompositeClip, backingCompositeClip,
					    pBackingStore->pSavedRegion);
	    }
	    if (pGC->subWindowMode == IncludeInferiors)
 	    {
		RegionPtr translatedClip;

		/* XXX
		 * any output in IncludeInferiors mode will not
		 * be redirected to Inferiors backing store.  This
		 * can be fixed only at great cost to the shadow routines.
		 */
		translatedClip = NotClippedByChildren (pWin);
		(*pGC->pScreen->TranslateRegion) (translatedClip,
						  pGC->clipOrg.x,
						  pGC->clipOrg.y);
		(*pGC->pScreen->Subtract) (backingCompositeClip, backingCompositeClip, translatedClip);
		(*pGC->pScreen->RegionDestroy) (translatedClip);
	    }

	    if (!(*pGC->pScreen->RegionNotEmpty) (backingCompositeClip)) {
		lift_functions = TRUE;
	    }

	    if (pGC->clientClipType != CT_PIXMAP)
	    {
		/*
		 * Finally, install the new clip list as the clientClip for the
		 * backing GC. The output library is responsible for destroying
		 * backingCompositeClip when the time comes.
		 */

		(*pBackingGC->ChangeClip) (pBackingGC, CT_REGION, backingCompositeClip, 0);
		backingCompositeClip = NULL;
	    }

	    pPriv->serialNumber = pDrawable->serialNumber;
	}
 	else
 	{
	    lift_functions = TRUE;
	}

	/* Reset the status when drawing to an unoccluded window so that
	 * future SaveAreas will actually copy bits from the screen.  Note that
	 * output to root window in IncludeInferiors mode will not cause this
	 * to change.  This causes all transient graphics by the window
	 * manager to the root window to not enable backing store.
	 */
	if (lift_functions && (pBackingStore->status == StatusVirtual) &&
	    (pWin->parent || pGC->subWindowMode != IncludeInferiors))
	    pBackingStore->status = StatusVDirty;
    }


    if (pGC->clientClipType != CT_PIXMAP)
    {
	/*
	 * all of these bits have already been dealt with for
	 * pBackingGC above and needn't be looked at again
	 */
	pPriv->changes &= 
		~(GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode);
	stateChanges &= 
		~(GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode);
    }

    if (!lift_functions && (pWin->backingStore == NotUseful ||
        ((pWin->backingStore & 3) == WhenMapped && !pWin->realized)))
    {
	lift_functions = TRUE;
    }

    /*
     * if no backing store has been allocated, and it's needed,
     * create it now
     */

    if (!lift_functions && pBackingStore->pBackingPixmap == NullPixmap)
	miCreateBSPixmap (pWin);

    /*
     * if this window still doesn't have a backing pixmap,
     * lift functions!
     */

    if (!lift_functions && pBackingStore->pBackingPixmap == NullPixmap)
        lift_functions = TRUE;
    
    if (lift_functions)
    {
	/*
	 * If the thing isn't a window or it shouldn't be having backing-store
	 * things done to it, simply save the changes made. When backing-store
	 * comes back on again, all the changes will be copied at once. If the
	 * GC is hooked, replace all the functions but those that were changed
	 * during this validation.
	 */
	pPriv->changes |= stateChanges;
	if (pPriv->gcHooked)
	{
	    mask = (~procChanges) & MIBS_ALLPROCS;
	    while (mask)
	    {
		index = lowbit (mask);
		mask &= ~index;
		switch(index)
		{
		    case MIBS_FILLSPANS:
			pGC->FillSpans = pPriv->FillSpans;
			break;
		    case MIBS_SETSPANS:
			pGC->SetSpans = pPriv->SetSpans;
			break;
		    case MIBS_PUTIMAGE:
			pGC->PutImage = pPriv->PutImage;
			break;
		    case MIBS_COPYAREA:
			pGC->CopyArea = pPriv->CopyArea;
			break;
		    case MIBS_COPYPLANE:
			pGC->CopyPlane = pPriv->CopyPlane;
			break;
		    case MIBS_POLYPOINT:
			pGC->PolyPoint = pPriv->PolyPoint;
			break;
		    case MIBS_POLYLINES:
			pGC->Polylines = pPriv->Polylines;
			break;
		    case MIBS_POLYSEGMENT:
			pGC->PolySegment = pPriv->PolySegment;
			break;
		    case MIBS_POLYRECTANGLE:
			pGC->PolyRectangle = pPriv->PolyRectangle;
			break;
		    case MIBS_POLYARC:
			pGC->PolyArc = pPriv->PolyArc;
			break;
		    case MIBS_FILLPOLYGON:
			pGC->FillPolygon = pPriv->FillPolygon;
			break;
		    case MIBS_POLYFILLRECT:
			pGC->PolyFillRect = pPriv->PolyFillRect;
			break;
		    case MIBS_POLYFILLARC:
			pGC->PolyFillArc = pPriv->PolyFillArc;
			break;
		    case MIBS_POLYTEXT8:
			pGC->PolyText8 = pPriv->PolyText8;
			break;
		    case MIBS_POLYTEXT16:
			pGC->PolyText16 = pPriv->PolyText16;
			break;
		    case MIBS_IMAGETEXT8:
			pGC->ImageText8 = pPriv->ImageText8;
			break;
		    case MIBS_IMAGETEXT16:
			pGC->ImageText16 = pPriv->ImageText16;
			break;
		    case MIBS_IMAGEGLYPHBLT:
			pGC->ImageGlyphBlt = pPriv->ImageGlyphBlt;
			break;
		    case MIBS_POLYGLYPHBLT:
			pGC->PolyGlyphBlt = pPriv->PolyGlyphBlt;
			break;
		    case MIBS_PUSHPIXELS:
			pGC->PushPixels = pPriv->PushPixels;
			break;
		}
	    }
	    pPriv->gcHooked = FALSE;
	}
	if (backingCompositeClip)
	    (* pGC->pScreen->RegionDestroy) (backingCompositeClip);

	return;
    }

    /*
     * the rest of this function gets the pBackingGC
     * into shape for possible draws
     */

    stateChanges |= pPriv->changes;
    pPriv->changes = 0;
    CopyGC(pGC, pBackingGC, stateChanges);

    if (!pPriv->gcHooked)
    {
	procChanges = MIBS_ALLPROCS;
    }
    pPriv->gcHooked = TRUE;

    mask = procChanges;
    while (mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
	    case MIBS_FILLSPANS:
		pPriv->FillSpans = pGC->FillSpans;
		pGC->FillSpans = miBSFillSpans;
		break;
	    case MIBS_SETSPANS:
		pPriv->SetSpans = pGC->SetSpans;
		pGC->SetSpans = miBSSetSpans;
		break;
	    case MIBS_PUTIMAGE:
		pPriv->PutImage = pGC->PutImage;
		pGC->PutImage = miBSPutImage;
		break;
	    case MIBS_COPYAREA:
		pPriv->CopyArea = pGC->CopyArea;
		pGC->CopyArea = miBSCopyArea;
		break;
	    case MIBS_COPYPLANE:
		pPriv->CopyPlane = pGC->CopyPlane;
		pGC->CopyPlane = miBSCopyPlane;
		break;
	    case MIBS_POLYPOINT:
		pPriv->PolyPoint = pGC->PolyPoint;
		pGC->PolyPoint = miBSPolyPoint;
		break;
	    case MIBS_POLYLINES:
		pPriv->Polylines = pGC->Polylines;
		pGC->Polylines = miBSPolylines;
		break;
	    case MIBS_POLYSEGMENT:
		pPriv->PolySegment = pGC->PolySegment;
		pGC->PolySegment = miBSPolySegment;
		break;
	    case MIBS_POLYRECTANGLE:
		pPriv->PolyRectangle = pGC->PolyRectangle;
		pGC->PolyRectangle = miBSPolyRectangle;
		break;
	    case MIBS_POLYARC:
		pPriv->PolyArc = pGC->PolyArc;
		pGC->PolyArc = miBSPolyArc;
		break;
	    case MIBS_FILLPOLYGON:
		pPriv->FillPolygon = pGC->FillPolygon;
		pGC->FillPolygon = miBSFillPolygon;
		break;
	    case MIBS_POLYFILLRECT:
		pPriv->PolyFillRect = pGC->PolyFillRect;
		pGC->PolyFillRect = miBSPolyFillRect;
		break;
	    case MIBS_POLYFILLARC:
		pPriv->PolyFillArc = pGC->PolyFillArc;
		pGC->PolyFillArc = miBSPolyFillArc;
		break;
	    case MIBS_POLYTEXT8:
		pPriv->PolyText8 = pGC->PolyText8;
		pGC->PolyText8 = miBSPolyText8;
		break;
	    case MIBS_POLYTEXT16:
		pPriv->PolyText16 = pGC->PolyText16;
		pGC->PolyText16 = miBSPolyText16;
		break;
	    case MIBS_IMAGETEXT8:
		pPriv->ImageText8 = pGC->ImageText8;
		pGC->ImageText8 = miBSImageText8;
		break;
	    case MIBS_IMAGETEXT16:
		pPriv->ImageText16 = pGC->ImageText16;
		pGC->ImageText16 = miBSImageText16;
		break;
	    case MIBS_IMAGEGLYPHBLT:
		pPriv->ImageGlyphBlt = pGC->ImageGlyphBlt;
		pGC->ImageGlyphBlt = miBSImageGlyphBlt;
		break;
	    case MIBS_POLYGLYPHBLT:
		pPriv->PolyGlyphBlt = pGC->PolyGlyphBlt;
		pGC->PolyGlyphBlt = miBSPolyGlyphBlt;
		break;
	    case MIBS_PUSHPIXELS:
		pPriv->PushPixels = pGC->PushPixels;
		pGC->PushPixels = miBSPushPixels;
		break;
	}
    }

    /*
     * We never want operations with the backingGC to generate GraphicsExpose
     * events...
     */
    if (stateChanges & GCGraphicsExposures) {
	XID false = xFalse;

	DoChangeGC(pBackingGC, GCGraphicsExposures, &false, FALSE);
    }

    if (pBackingStore->pBackingPixmap->drawable.serialNumber
    	!= pBackingGC->serialNumber)
    {
	ValidateGC((DrawablePtr)pBackingStore->pBackingPixmap, pBackingGC);

	/*
	 * This function has the dubious duty of merging this
	 * region into the PIXMAP clip list for the GC.
	 */

	if ((pGC->clientClipType == CT_PIXMAP) && backingCompositeClip)
	    (* pBackingStore->SetClipmaskRgn) (pBackingGC, backingCompositeClip);

    }

    if (backingCompositeClip)
	(* pGC->pScreen->RegionDestroy) (backingCompositeClip);

    if (pBackingGC->clientClip == 0)
    	ErrorF ("backing store clip list nil");
}


static void
miDestroyBSPixmap (pWin)
    WindowPtr	pWin;
{
    MIBackingStorePtr	pBackingStore;
    ScreenPtr		pScreen;
    
    pScreen = pWin->drawable.pScreen;
    pBackingStore = (MIBackingStorePtr) pWin->devBackingStore;
    if (pBackingStore->pBackingPixmap)
	(* pScreen->DestroyPixmap)(pBackingStore->pBackingPixmap);
    pBackingStore->pBackingPixmap = NullPixmap;
    if (IS_VALID_PIXMAP(pBackingStore->backgroundTile))
	(* pScreen->DestroyPixmap)(pBackingStore->backgroundTile);
    pBackingStore->backgroundTile = NullPixmap;
    pBackingStore->status = StatusNoPixmap;
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
}

static void
miTileVirtualBS (pWin)
    WindowPtr	pWin;
{
    MIBackingStorePtr	pBackingStore;

    pBackingStore = (MIBackingStorePtr) pWin->devBackingStore;
    pBackingStore->backgroundPixel = pWin->backgroundPixel;
    pBackingStore->backgroundTile = pWin->backgroundTile;
    if (IS_VALID_PIXMAP (pBackingStore->backgroundTile))
	pBackingStore->backgroundTile->refcnt++;

    if (pBackingStore->status != StatusVDirty)
	pBackingStore->status = StatusVirtual;

    /*
     * punt parent relative tiles and do it now
     */
    if (pBackingStore->backgroundTile == (PixmapPtr) ParentRelative)
	miCreateBSPixmap (pWin);
}

static int BSAllocationsFailed = 0;

# define FAILEDSIZE	32

static struct { int w, h; } failedRecord[FAILEDSIZE];
static int failedIndex;

static void
miCreateBSPixmap (pWin)
    WindowPtr	pWin;
{
    MIBackingStorePtr	pBackingStore;
    ScreenPtr		pScreen;
    PixmapPtr		backgroundTile;
    unsigned long	backgroundPixel;
    BoxPtr		extents;
    Bool		backSet;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (MIBackingStorePtr) pWin->devBackingStore;
    backSet = ((pBackingStore->status == StatusVirtual) ||
	       (pBackingStore->status == StatusVDirty));

    if (pBackingStore->pBackingPixmap == NullPixmap)
	pBackingStore->pBackingPixmap =
    	    (PixmapPtr)(* pScreen->CreatePixmap)
			   (pScreen,
			    pWin->clientWinSize.width,
			    pWin->clientWinSize.height,
			    pWin->drawable.depth);

    if (pBackingStore->pBackingPixmap == NullPixmap)
    {
	BSAllocationsFailed++;
	/*
	 * record failed allocations
	 */
	failedRecord[failedIndex].w = pWin->clientWinSize.width;
	failedRecord[failedIndex].h = pWin->clientWinSize.height;
	failedIndex++;
	if (failedIndex == FAILEDSIZE)
		failedIndex = 0;

	pBackingStore->status = StatusNoPixmap;
	return; /* XXX */
    }

    pBackingStore->status = StatusContents;

    if (backSet)
    {
	backgroundTile = pWin->backgroundTile;
	backgroundPixel = pWin->backgroundPixel;
    
	pWin->backgroundTile = pBackingStore->backgroundTile;
	pWin->backgroundPixel = pBackingStore->backgroundPixel;
	if (IS_VALID_PIXMAP (pWin->backgroundTile))
	    pWin->backgroundTile->refcnt++;
    }

    extents = (* pScreen->RegionExtents) (pBackingStore->pSavedRegion);
    miBSClearToBackground(pWin,
			  extents->x1, extents->y1,
			  extents->x2 - extents->x1,
			  extents->y2 - extents->y1,
			  FALSE);

    if (backSet)
    {
	if (IS_VALID_PIXMAP (pWin->backgroundTile))
	    (* pScreen->DestroyPixmap) (pWin->backgroundTile);
	pWin->backgroundTile = backgroundTile;
	pWin->backgroundPixel = backgroundPixel;
	if (IS_VALID_PIXMAP (pBackingStore->backgroundTile))
	    (* pScreen->DestroyPixmap) (pBackingStore->backgroundTile);
	pBackingStore->backgroundTile = NullPixmap;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miExposeCopy --
 *	Handle the restoration of areas exposed by graphics operations.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	prgnExposed has the areas exposed from backing-store removed
 *	from it.
 *
 *-----------------------------------------------------------------------
 */
static void
miExposeCopy (pSrc, pDst, pGC, prgnExposed, srcx, srcy, dstx, dsty, plane)
    WindowPtr	  	pSrc;
    DrawablePtr	  	pDst;
    GCPtr   	  	pGC;
    RegionPtr	  	prgnExposed;
    int	    	  	srcx, srcy;
    int	    	  	dstx, dsty;
    unsigned long 	plane;
{
    RegionPtr	  	tempRgn;
    MIBackingStorePtr	pBackingStore;
    RegionPtr	  	(*copyProc)();
    register BoxPtr	pBox;
    register int  	i;
    register int  	dx, dy;

    if (!(*pGC->pScreen->RegionNotEmpty) (prgnExposed))
	return;
    pBackingStore = (MIBackingStorePtr)pSrc->devBackingStore;
    
    if (pBackingStore->status == StatusNoPixmap)
    	return;

    tempRgn = (* pGC->pScreen->RegionCreate) (NULL, 1);
    (* pGC->pScreen->Intersect) (tempRgn, prgnExposed,
				 pBackingStore->pSavedRegion);
    (* pGC->pScreen->Subtract) (prgnExposed, prgnExposed, tempRgn);

    if (plane != 0) {
	copyProc = pGC->CopyPlane;
    } else {
	copyProc = pGC->CopyArea;
    }
    
    dx = dstx - srcx;
    dy = dsty - srcy;
    
    switch (pBackingStore->status) {
    case StatusVirtual:
    case StatusVDirty:
	pGC = GetScratchGC (pDst->depth, pDst->pScreen);
	miBSFillVirtualBits (pDst, pGC, tempRgn, dx, dy,
		pBackingStore->backgroundPixel, pBackingStore->backgroundTile,
		(PixmapPtr) USE_BACKGROUND_PIXEL, ~0L);
	FreeScratchGC (pGC);
	break;
    case StatusContents:
	for (i = tempRgn->numRects, pBox = tempRgn->rects; i > 0; i--, pBox++) {
	    (* copyProc) (pBackingStore->pBackingPixmap,
			  pDst, pGC, pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	}
	break;
    }
}
    
