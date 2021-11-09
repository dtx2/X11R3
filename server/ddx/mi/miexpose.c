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

/* $XConsortium: miexpose.c,v 1.36 88/10/15 12:18:11 rws Exp $ */

#include "X.h"
#include "Xproto.h"
#include "Xprotostr.h"

#include "misc.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmap.h"
#include "input.h"

#include "dixstruct.h"
#include "mi.h"
#include "Xmd.h"

extern WindowRec WindowTable[];

/*
    machine-independent graphics exposure code.  any device that uses
the region package can call this.
*/


/* miHandleExposures 
    generate a region for exposures for areas that were copied from obscured or
non-existent areas to non-obscured areas of the destination.  Paint the
background for the region, if the destination is a window.

NOTE:
     this should generally be called, even if graphicsExposures is false,
because this is where bits get recovered from backing store.

NOTE:
     added argument 'plane' is used to indicate how exposures from backing
store should be accomplished. If plane is 0 (i.e. no bit plane), CopyArea
should be used, else a CopyPlane of the indicated plane will be used. The
exposing is done by the backing store's GraphicsExpose function, of course.

*/

RegionPtr
miHandleExposures(pSrcDrawable, pDstDrawable,
		  pGC, srcx, srcy, width, height, dstx, dsty, plane)
    register DrawablePtr	pSrcDrawable;
    register DrawablePtr	pDstDrawable;
    GCPtr 			pGC;
    int 			srcx, srcy;
    int 			width, height;
    int 			dstx, dsty;
    unsigned long		plane;
{
    register ScreenPtr pscr = pGC->pScreen;
    RegionPtr prgnSrcClip;	/* drawable-relative source clip */
    RegionPtr prgnDstClip;	/* drawable-relative dest clip */
    BoxRec srcBox;		/* unclipped source */
    RegionPtr prgnExposed;	/* exposed region, calculated source-
				   relative, made dst relative to
				   intersect with visible parts of
				   dest and send events to client, 
				   and then screen relative to paint 
				   the window background
				*/
    WindowPtr pSrcWin;

    /* avoid work if we can */
    if (!pGC->graphicsExposures &&
	(pDstDrawable->type == DRAWABLE_PIXMAP) &&
	((pSrcDrawable->type == DRAWABLE_PIXMAP) ||
	 (((WindowPtr)pSrcDrawable)->backingStore == NotUseful) ||
	 (((WindowPtr)pSrcDrawable)->backStorage == (BackingStorePtr)NULL)))
	return NULL;
	
    srcBox.x1 = srcx;
    srcBox.y1 = srcy;
    srcBox.x2 = srcx+width;
    srcBox.y2 = srcy+height;

    if (pSrcDrawable->type == DRAWABLE_WINDOW)
    {
	pSrcWin = (WindowPtr)pSrcDrawable;
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    prgnSrcClip = NotClippedByChildren(pSrcWin);
	}
	else
	{
	    BoxRec TsrcBox;

	    TsrcBox.x1 = srcx + pSrcWin->absCorner.x;
	    TsrcBox.y1 = srcy + pSrcWin->absCorner.x;
	    TsrcBox.x1 = TsrcBox.x1+width;
	    TsrcBox.x1 = TsrcBox.y1+height;
	    if (((*pscr->RectIn)(pSrcWin->clipList, &TsrcBox)) == rgnIN)
		return NULL;
	    prgnSrcClip = (*pscr->RegionCreate)(NullBox, 1);
	    (*pscr->RegionCopy)(prgnSrcClip, pSrcWin->clipList);
	}
	(*pscr->TranslateRegion)(prgnSrcClip,
				 -pSrcWin->absCorner.x,
				 -pSrcWin->absCorner.y);
    }
    else
    {
	BoxRec	box;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = ((PixmapPtr)pSrcDrawable)->width;
	box.y2 = ((PixmapPtr)pSrcDrawable)->height;
	if ((srcBox.x1 >= 0) && (srcBox.y1 >= 0) &&
	    (srcBox.x2 <= box.x2) && (srcBox.y2 <= box.y2))
	    return NULL;
	prgnSrcClip = (*pscr->RegionCreate)(&box, 1);
	pSrcWin = (WindowPtr)NULL;
    }

    if (pDstDrawable == pSrcDrawable)
    {
	prgnDstClip = prgnSrcClip;
    }
    else if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    prgnDstClip = NotClippedByChildren((WindowPtr)pDstDrawable);
	}
	else
	{
	    prgnDstClip = (*pscr->RegionCreate)(NullBox, 1);
	    (*pscr->RegionCopy)(prgnDstClip,
				((WindowPtr)pDstDrawable)->clipList);
	}
	(*pscr->TranslateRegion)(prgnDstClip,
				 -((WindowPtr)pDstDrawable)->absCorner.x,
				 -((WindowPtr)pDstDrawable)->absCorner.y);
    }
    else
    {
	BoxRec	box;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = ((PixmapPtr)pDstDrawable)->width;
	box.y2 = ((PixmapPtr)pDstDrawable)->height;
	prgnDstClip = (*pscr->RegionCreate)(&box, 1);
    }

    /* drawable-relative source region */
    prgnExposed = (*pscr->RegionCreate)(&srcBox, 1);

    /* now get the hidden parts of the source box*/
    (*pscr->Subtract)(prgnExposed, prgnExposed, prgnSrcClip);

    if (pSrcWin && (pSrcWin->backingStore != NotUseful) &&
	(pSrcWin->backStorage != (BackingStorePtr)NULL))
    {
	/*
	 * Copy any areas from the source backing store. Modifies
	 * prgnExposed.
	 */
	(* pSrcWin->backStorage->ExposeCopy) (pSrcDrawable,
					      pDstDrawable,
					      pGC,
					      prgnExposed,
					      srcx, srcy,
					      dstx, dsty,
					      plane);
    }
    
    /* move them over the destination */
    (*pscr->TranslateRegion)(prgnExposed, dstx-srcx, dsty-srcy);

    /* intersect with visible areas of dest */
    (*pscr->Intersect)(prgnExposed, prgnExposed, prgnDstClip);

    if ((pDstDrawable->type == DRAWABLE_WINDOW) &&
	(((WindowPtr)pDstDrawable)->backgroundTile != None))
    {
	WindowPtr pWin = (WindowPtr)pDstDrawable;

	/* make the exposed area screen-relative */
	(*pscr->TranslateRegion)(prgnExposed, 
				 pWin->absCorner.x, pWin->absCorner.y);

	(*pWin->PaintWindowBackground)(pDstDrawable, prgnExposed, 
				       PW_BACKGROUND);

	(*pscr->TranslateRegion)(prgnExposed,
 				 -pWin->absCorner.x, -pWin->absCorner.y);
    }
    if (prgnDstClip != prgnSrcClip)
	(*pscr->RegionDestroy)(prgnDstClip);
    (*pscr->RegionDestroy)(prgnSrcClip);
    if (pGC->graphicsExposures)
	return prgnExposed;
    else
    {
	(*pscr->RegionDestroy) (prgnExposed);
	return NULL;
    }
}

/* send GraphicsExpose events, or a NoExpose event, based on the region */

void
miSendGraphicsExpose (client, pRgn, drawable, major, minor)
    ClientPtr	client;
    RegionPtr	pRgn;
    XID		drawable;
    int	major;
    int	minor;
{
    if (pRgn && REGION_NOT_EMPTY(pRgn))
    {
        xEvent *pEvent;
	register xEvent *pe;
	register BoxPtr pBox = pRgn->rects;
	register int i;

	if(!(pEvent = (xEvent *)ALLOCATE_LOCAL(pRgn->numRects * 
					 sizeof(xEvent))))
		return;
	pe = pEvent;

	for (i=1; i<=pRgn->numRects; i++, pe++, pBox++)
	{
	    pe->u.u.type = GraphicsExpose;
	    pe->u.graphicsExposure.drawable = drawable;
	    pe->u.graphicsExposure.x = pBox->x1;
	    pe->u.graphicsExposure.y = pBox->y1;
	    pe->u.graphicsExposure.width = pBox->x2 - pBox->x1;
	    pe->u.graphicsExposure.height = pBox->y2 - pBox->y1;
	    pe->u.graphicsExposure.count = pRgn->numRects - i;
	    pe->u.graphicsExposure.majorEvent = major;
	    pe->u.graphicsExposure.minorEvent = minor;
	}
	TryClientEvents(client, pEvent, pRgn->numRects,
			    0, NoEventMask, NullGrab);
	DEALLOCATE_LOCAL(pEvent);
    }
    else
    {
        xEvent event;
	event.u.u.type = NoExpose;
	event.u.noExposure.drawable = drawable;
	event.u.noExposure.majorEvent = major;
	event.u.noExposure.minorEvent = minor;
	TryClientEvents(client, &event, 1,
	    0, NoEventMask, NullGrab);
    }
}

#ifdef notdef
void
miSendNoExpose(pGC)
    GCPtr pGC;
{
    if (pGC->graphicsExposures)
    {
        xEvent event;
	event.u.u.type = NoExpose;
	event.u.noExposure.drawable = 
		    requestingClient->lastDrawableID;
        TryClientEvents(requestingClient, &event, 1,
	        0, NoEventMask, NullGrab);
    }
}
#endif

void 
miWindowExposures(pWin)
    WindowPtr pWin;
{
    register RegionPtr prgn;

    prgn = pWin->exposed;
    if (prgn->numRects)
    {
	xEvent *pEvent;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	RegionPtr exposures = prgn;

	/*
	 * Restore from backing-store FIRST. Note that while the double
	 * translation is somewhat expensive, it's not as expensive as
	 * the double refresh. In addition, much of the time, pWin->exposed
	 * will come back empty from RestoreAreas, so the second
	 * translation won't happen.
	 */
 	if (pWin->backingStore != NotUseful && pWin->backStorage)
	{
	    /*
	     * RestoreAreas needs the region window-relative, but
	     * PaintWindowBackground needs it absolute, so translate down
	     * then back again. Note that RestoreAreas modifies prgn
	     * (pWin->exposed).
	     */
	    (* pWin->drawable.pScreen->TranslateRegion)(prgn,
							-pWin->absCorner.x,
							-pWin->absCorner.y);
	    exposures = (*pWin->backStorage->RestoreAreas)(pWin);
	    if (prgn->numRects == 0)
	    {
		return;
	    }
	    else
	    {
		(* pWin->drawable.pScreen->TranslateRegion)(prgn,
							    pWin->absCorner.x,
							    pWin->absCorner.y);
	    }
	}
        (*pWin->PaintWindowBackground)(pWin, prgn, PW_BACKGROUND);
	if (exposures)
	{
	    (* pWin->drawable.pScreen->TranslateRegion)(exposures,
			-pWin->absCorner.x, -pWin->absCorner.y);
	    pBox = exposures->rects;

	    if(!(pEvent = (xEvent *)
		ALLOCATE_LOCAL(exposures->numRects * sizeof(xEvent))))
		return;
	    pe = pEvent;

	    for (i=1; i<=exposures->numRects; i++, pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pWin->wid;
		pe->u.expose.x = pBox->x1;
		pe->u.expose.y = pBox->y1;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = (exposures->numRects - i);
	    }
	    DeliverEvents(pWin, pEvent, exposures->numRects, NullWindow);
	    DEALLOCATE_LOCAL(pEvent);
	    if (exposures != prgn)
	        (* pWin->drawable.pScreen->RegionDestroy) (exposures);
	}
	prgn->numRects = 0;
    }
}


/*
    this code is highly unlikely.  it is not haile selassie.

    there is some hair here.  we can't just use the window's
clip region as it is, because if we are painting the border,
the border is not in the client area and so we will be excluded
when we validate the GC, and if we are painting a parent-relative
background, the area we want to paint is in some other window.
since we trust the code calling us to tell us to paint only areas
that are really ours, we will temporarily give the window a
clipList the size of the whole screen and an origin at (0,0).
this more or less assumes that ddX code will do translation
based on the window's absCorner, and that ValidateGC will
look at clipList, and that no other fields from the
window will be used.  it's not possible to just draw
in the root because it may be a different depth.

to get the tile to align correctly we set the GC's tile origin to
be the (x,y) of the window's upper left corner, after which we
get the right bits when drawing into the root.

because the clip_mask is being set to None, we may call DoChangeGC with
fPointer set true, thus we no longer need to install the background or
border tile in the resource table.
*/

static GCPtr	screenContext[MAXSCREENS];

static
tossGC (pGC)
GCPtr pGC;
{
    int i;

    if (screenContext[i = pGC->pScreen->myNum] == pGC)
	screenContext[i] = (GCPtr)NULL;
    FreeGC (pGC);
}


void
miPaintWindow(pWin, prgn, what)
WindowPtr pWin;
RegionPtr prgn;
int what;
{
    int	status;

    Bool usingScratchGC = FALSE;
    WindowPtr pRoot;
	
#define FUNCTION	0
#define FOREGROUND	1
#define TILE		2
#define FILLSTYLE	3
#define ABSX		4
#define ABSY		5
#define CLIPMASK	6
#define SUBWINDOW	7
#define COUNT_BITS	8

    XID gcval[7];
    XID newValues [COUNT_BITS];

    BITS32 gcmask, index, mask;
    RegionPtr prgnWin;
    DDXPointRec oldCorner;
    BoxRec box;
    GCPtr pGC;
    register int i;
    register BoxPtr pbox;
    register ScreenPtr pScreen = pWin->drawable.pScreen;
    register xRectangle *prect;

    gcmask = 0;

    if (what == PW_BACKGROUND)
    {
	if (pWin->backgroundTile == (PixmapPtr)None)
	    return;
	else if (pWin->backgroundTile == (PixmapPtr)ParentRelative)
	{
	    (*pWin->parent->PaintWindowBackground)(pWin->parent, prgn, what);
	    return;
	}
	else if (pWin->backgroundTile == (PixmapPtr)USE_BACKGROUND_PIXEL)
	{
	    newValues[FOREGROUND] = pWin->backgroundPixel;
	    newValues[FILLSTYLE] = FillSolid;
	    gcmask |= GCForeground | GCFillStyle;
	}
	else
	{
	    newValues[TILE] = (XID)pWin->backgroundTile;
	    newValues[FILLSTYLE] = FillTiled;
	    gcmask |= GCTile | GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	}
    }
    else
    {
	if (pWin->borderTile == (PixmapPtr)USE_BORDER_PIXEL)
	{
	    newValues[FOREGROUND] = pWin->borderPixel;
	    newValues[FILLSTYLE] = FillSolid;
	    gcmask |= GCForeground | GCFillStyle;
	}
	else
	{
	    newValues[TILE] = (XID)pWin->borderTile;
	    newValues[FILLSTYLE] = FillTiled;
	    gcmask |= GCTile | GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	}
    }

    newValues[FUNCTION] = GXcopy;
    gcmask |= GCFunction | GCClipMask;

    i = pScreen->myNum;
    pRoot = &WindowTable[i];

    if (pWin->visual != pRoot->visual)
    {
	usingScratchGC = TRUE;
	/*
	 * mash the clip list so we can paint the border by
	 * mangling the window in place, pretending it
	 * spans the entire screen
	 */
	if (what == PW_BORDER)
	{
	    prgnWin = pWin->clipList;
	    oldCorner = pWin->absCorner;
	    pWin->absCorner.x = pWin->absCorner.y = 0;
	    box.x1 = 0;
	    box.y1 = 0;
	    box.x2 = pScreen->width;
	    box.y2 = pScreen->height;
	    pWin->clipList = (*pScreen->RegionCreate)(&box, 1);
	    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    newValues[ABSX] = pWin->absCorner.x;
	    newValues[ABSY] = pWin->absCorner.y;
	}
	else
	{
	    newValues[ABSX] = 0;
	    newValues[ABSY] = 0;
	}
	pGC = GetScratchGC(pWin->drawable.depth, pWin->drawable.pScreen);
    } else {
	/*
	 * draw the background to the root window
	 */
	if (screenContext[i] == (GCPtr)NULL)
	{
	    screenContext[i] = CreateGC(pWin, (BITS32) 0, (XID *) 0, &status);
	    AddResource (FakeClientID (0), RT_GC, (pointer) screenContext[i],
	    		 tossGC, RC_CORE);
	}
	pGC = screenContext[i];
	newValues[SUBWINDOW] = IncludeInferiors;
	newValues[ABSX] = pWin->absCorner.x;
	newValues[ABSY] = pWin->absCorner.y;
	gcmask |= GCSubwindowMode;
	pWin = pRoot;
    }
    
    if (pWin->backingStore != NotUseful && pWin->backStorage)
	(*pWin->backStorage->DrawGuarantee) (pWin, pGC, GuaranteeVisBack);

    mask = gcmask;
    gcmask = 0;
    i = 0;
    while (mask) {
    	index = lowbit (mask);
	mask &= ~index;
	switch (index) {
	case GCFunction:
	    if ((XID) pGC->alu != newValues[FUNCTION]) {
		gcmask |= index;
		gcval[i++] = newValues[FUNCTION];
	    }
	    break;
	case GCTileStipXOrigin:
	    if ((XID) pGC->patOrg.x != newValues[ABSX]) {
		gcmask |= index;
		gcval[i++] = newValues[ABSX];
	    }
	    break;
	case GCTileStipYOrigin:
	    if ((XID) pGC->patOrg.y != newValues[ABSY]) {
		gcmask |= index;
		gcval[i++] = newValues[ABSY];
	    }
	    break;
	case GCClipMask:
	    if ((XID) pGC->clientClipType != CT_NONE) {
		gcmask |= index;
		gcval[i++] = CT_NONE;
	    }
	    break;
	case GCSubwindowMode:
	    if ((XID) pGC->subWindowMode != newValues[SUBWINDOW]) {
		gcmask |= index;
		gcval[i++] = newValues[SUBWINDOW];
	    }
	    break;
	case GCTile:
	    if ((XID) pGC->tile != newValues[TILE]) {
		gcmask |= index;
		gcval[i++] = newValues[TILE];
	    }
	    break;
	case GCFillStyle:
	    if ((XID) pGC->fillStyle != newValues[FILLSTYLE]) {
		gcmask |= index;
		gcval[i++] = newValues[FILLSTYLE];
	    }
	    break;
	case GCForeground:
	    if ((XID) pGC->fgPixel != newValues[FOREGROUND]) {
		gcmask |= index;
		gcval[i++] = newValues[FOREGROUND];
	    }
	    break;
	}
    }

    if (gcmask)
        DoChangeGC(pGC, gcmask, gcval, 1);

    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC(pWin, pGC);

    prect = (xRectangle *)ALLOCATE_LOCAL(prgn->numRects * sizeof(xRectangle));
    pbox = prgn->rects;
    for (i= 0; i < prgn->numRects; i++, pbox++, prect++)
    {
	prect->x = pbox->x1;
	prect->y = pbox->y1;
	prect->width = pbox->x2 - pbox->x1;
	prect->height = pbox->y2 - pbox->y1;
    }
    prect -= prgn->numRects;
    (*pGC->PolyFillRect)(pWin, pGC, prgn->numRects, prect);
    DEALLOCATE_LOCAL(prect);

    if (pWin->backingStore != NotUseful && pWin->backStorage)
	(*pWin->backStorage->DrawGuarantee) (pWin, pGC, GuaranteeNothing);

    if (usingScratchGC)
    {
	if (what == PW_BORDER)
	{
	    (*pScreen->RegionDestroy)(pWin->clipList);
	    pWin->clipList = prgnWin;
	    pWin->absCorner = oldCorner;
	    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	}
	FreeScratchGC(pGC);
    }
}
