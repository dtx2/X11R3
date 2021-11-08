/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.

                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/*
 * Functions implementing Apollo-display-independent parts of the driver
 * having to do with replacing the GC entries for the benefit of a correct
 * software cursor (transparently to the rest of the driver).
 */

#include "apollo.h"
#include "dixfontstr.h"
#include "fontstruct.h"
#include "mifpoly.h"    /* for SppPointPtr */

/*
 * Call this to get the cursor out of the way.  Someone else will put it back.
 */
extern void     apRemoveCursor();

/*
 * Overlap BoxPtr and Box elements
 */
#define BOX_OVERLAP(pCbox,X1,Y1,X2,Y2) \
        (((pCbox)->x1 <= (X2)) && ((X1) <= (pCbox)->x2) && \
         ((pCbox)->y1 <= (Y2)) && ((Y1) <= (pCbox)->y2))

/*
 * Overlap BoxPtr, origins, and rectangle
 */
#define ORG_OVERLAP(pCbox,xorg,yorg,x,y,w,h) \
    BOX_OVERLAP((pCbox),(x)+(xorg),(y)+(yorg),(x)+(xorg)+(w),(y)+(yorg)+(h))

/*
 * Overlap BoxPtr, origins and RectPtr
 */
#define ORGRECT_OVERLAP(pCbox,xorg,yorg,pRect) \
    ORG_OVERLAP((pCbox),(xorg),(yorg),(pRect)->x,(pRect)->y, \
		((int)(pRect)->width), ((int)(pRect)->height))
/*
 * Overlap BoxPtr and horizontal span
 */
#define SPN_OVERLAP(pCbox,y,x,w) BOX_OVERLAP((pCbox),(x),(y),(x)+(w),(y))

/*
  * See if this drawable is worth doing a cursor check on.
  */
#define       IsWorthChecking(d) \
      ((d->type == DRAWABLE_WINDOW) && (((WindowPtr)d)->viewable))

#ifdef  notdef
#define COMPARE_GCS(g1, g2)     compare_gcs(g1, g2)

static
compare_gcs(g1, g2)
GCPtr g1, g2;
{
    if (g1->pScreen != g2->pScreen) FatalError("GC and Shadow mis-match - pScreen\n");
    if (g1->depth != g2->depth) FatalError("GC and Shadow mis-match - depth\n");
#ifdef  notdef
    if (g1->serialNumber != g2->serialNumber) FatalError("GC and Shadow mis-match - serialNumber\n");
#endif
    if (g1->alu != g2->alu) FatalError("GC and Shadow mis-match - alu\n");
    if (g1->planemask != g2->planemask) FatalError("GC and Shadow mis-match - planemask\n");
    if (g1->fgPixel != g2->fgPixel) FatalError("GC and Shadow mis-match - fgPixel\n");
    if (g1->bgPixel != g2->bgPixel) FatalError("GC and Shadow mis-match - bgPixel\n");
    if (g1->lineWidth != g2->lineWidth) FatalError("GC and Shadow mis-match - lineWidth\n");
    if (g1->lineStyle != g2->lineStyle) FatalError("GC and Shadow mis-match - lineStyle\n");
    if (g1->capStyle != g2->capStyle) FatalError("GC and Shadow mis-match - capStyle\n");
    if (g1->joinStyle != g2->joinStyle) FatalError("GC and Shadow mis-match - joinStyle\n");
    if (g1->fillStyle != g2->fillStyle) FatalError("GC and Shadow mis-match - fillStyle\n");
    if (g1->fillRule != g2->fillRule) FatalError("GC and Shadow mis-match - fillRule\n");
    if (g1->arcMode != g2->arcMode) FatalError("GC and Shadow mis-match - arcMode\n");
    if (g1->tile != g2->tile) FatalError("GC and Shadow mis-match - tile\n");
    if (g1->stipple != g2->stipple) FatalError("GC and Shadow mis-match - stipple\n");
    if (g1->patOrg.x != g2->patOrg.x) FatalError("GC and Shadow mis-match - patOrg.x\n");
    if (g1->patOrg.y != g2->patOrg.y) FatalError("GC and Shadow mis-match - patOrg.y\n");
    if (g1->font != g2->font) FatalError("GC and Shadow mis-match - font\n");
    if (g1->subWindowMode != g2->subWindowMode) FatalError("GC and Shadow mis-match - subWindowMode\n");
    if (g1->graphicsExposures != g2->graphicsExposures) FatalError("GC and Shadow mis-match - graphicsExposures\n");
    if (g1->clipOrg.x != g2->clipOrg.x) FatalError("GC and Shadow mis-match - clipOrg.x\n");
    if (g1->clipOrg.y != g2->clipOrg.y) FatalError("GC and Shadow mis-match - clipOrg.y\n");
    if (g1->clientClip != g2->clientClip) FatalError("GC and Shadow mis-match - clientClip\n");
    if (g1->clientClipType != g2->clientClipType) FatalError("GC and Shadow mis-match - clientClipType\n");
    if (g1->dashOffset != g2->dashOffset) FatalError("GC and Shadow mis-match - dashOffset\n");
    if (g1->numInDashList != g2->numInDashList) FatalError("GC and Shadow mis-match - numInDashList\n");
#ifdef  notdef
    if (g1->dash != g2->dash) FatalError("GC and Shadow mis-match - dash\n");
    if (g1->stateChanges != g2->stateChanges) FatalError("GC and Shadow mis-match - stateChanges\n");
    if (g1->lastWinOrg.x != g2->lastWinOrg.x) FatalError("GC and Shadow mis-match - lastWinOrg.x\n");
    if (g1->lastWinOrg.y != g2->lastWinOrg.y) FatalError("GC and Shadow mis-match - lastWinOrg.y\n");
#endif
    if (g1->miTranslate != g2->miTranslate) FatalError("GC and Shadow mis-match - miTranslate\n");

}
#else
#define COMPARE_GCS(g1, g2)
#endif


/*
 * apSaveCursorBox -- Driver internal code
 *      Given an array of points, figure out the bounding box for the
 *      series and remove the cursor if it overlaps that box.
 */
static void
apSaveCursorBox (xorg, yorg, mode, lw, pPts, nPts, pCursorBox)
    register int            xorg;           /* X-Origin for points */
    register int            yorg;           /* Y-Origin for points */
    int                     mode;           /* CoordModeOrigin or
                                             * CoordModePrevious */
    int			    lw;		    /* line width */
    register DDXPointPtr    pPts;           /* Array of points */
    int                     nPts;           /* Number of points */
    register BoxPtr         pCursorBox;     /* Bounding box for cursor */
{
    register int            minx,
                            miny,
                            maxx,
                            maxy;

    minx = maxx = pPts->x + xorg;
    miny = maxy = pPts->y + yorg;

    pPts++;
    nPts--;

    if (mode == CoordModeOrigin) {
        while (nPts--) {
            minx = min(minx, pPts->x + xorg);
            maxx = max(maxx, pPts->x + xorg);
            miny = min(miny, pPts->y + yorg);
            maxy = max(maxy, pPts->y + yorg);
            pPts++;
        }
    } else {
        xorg = minx;
        yorg = miny;
        while (nPts--) {
            minx = min(minx, pPts->x + xorg);
            maxx = max(maxx, pPts->x + xorg);
            miny = min(miny, pPts->y + yorg);
            maxy = max(maxy, pPts->y + yorg);
            xorg += pPts->x;
            yorg += pPts->y;
            pPts++;
        }
    }
    lw = lw >> 1;
    if (BOX_OVERLAP(pCursorBox,minx-lw,miny-lw,maxx+lw,maxy+lw)) {
        apRemoveCursor();
    }
}
                       
/*
 * apFillSpans -- DDX interface (GC)
 *      Remove the cursor if any of the spans overlaps the area covered
 *      by the cursor, then call the "actual" FillSpans.
 *      This assumes the points have been translated already.
 */
static void
apFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr       pGC;
    int         nInit;                  /* number of spans to fill */
    DDXPointPtr pptInit;                /* pointer to list of start points */
    int         *pwidthInit;            /* pointer to list of n widths */
    int         fSorted;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;

    if (IsWorthChecking(pDrawable)) {
        BoxRec    cursorBox;

        if (apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            register DDXPointPtr    pts;
            register int            *widths;
            register int            nPts;

            for (pts = pptInit, widths = pwidthInit, nPts = nInit;
                 nPts--;
                 pts++, widths++) {
                     if (SPN_OVERLAP(&cursorBox,pts->y,pts->x,*widths)) {
                         apRemoveCursor();
                         break;
                     }
            }
        }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->FillSpans)(pDrawable, pShadowGC, nInit, pptInit,
                             pwidthInit, fSorted);
}

/*
 * apSetSpans -- DDX interface (GC)
 *      Remove the cursor if any of the spans overlaps the area covered
 *      by the cursor, then call the "actual" SetSpans.
 *      This assumes the points have been translated already.
 */
static void
apSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr         pDrawable;
    GCPtr               pGC;
    int                 *psrc;
    register DDXPointPtr ppt;
    int                 *pwidth;
    int                 nspans;
    int                 fSorted;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;

    if (IsWorthChecking(pDrawable)) {
        BoxRec    cursorBox;

        if (apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            register DDXPointPtr    pts;
            register int            *widths;
            register int            nPts;

            for (pts = ppt, widths = pwidth, nPts = nspans;
                 nPts--;
                 pts++, widths++) {
                     if (SPN_OVERLAP(&cursorBox,pts->y,pts->x,*widths)) {
                         apRemoveCursor();
                         break;
                     }
            }
        }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->SetSpans) (pDrawable, pShadowGC, psrc, ppt, pwidth,
                             nspans, fSorted);
}

/*
 * apGetSpans -- DDX interface (GC)
 *      Remove the cursor if any of the spans overlaps the area covered
 *      by the cursor, then call the "actual" GetSpans.
 *      This assumes the points have been translated already.
 */
unsigned int *
apGetSpans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr         pDrawable;      /* drawable from which to get bits */
    int                 wMax;           /* largest value of all *pwidths */
    register DDXPointPtr ppt;           /* points to start copying from */
    int                 *pwidth;        /* list of number of bits to copy */
    int                 nspans;         /* number of scanlines to copy */
{
    if (IsWorthChecking(pDrawable)) {
        BoxRec    cursorBox;

        if (apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            register DDXPointPtr    pts;
            register int            *widths;
            register int            nPts;
            register int            xorg,
                                    yorg;

            xorg = ((WindowPtr)pDrawable)->absCorner.x;
            yorg = ((WindowPtr)pDrawable)->absCorner.y;

            for (pts = ppt, widths = pwidth, nPts = nspans;
                 nPts--;
                 pts++, widths++) {
                     if (SPN_OVERLAP(&cursorBox,pts->y+yorg,
                                     pts->x+xorg,*widths)) {
                                         apRemoveCursor();
                                         break;
                     }
            }
        }
    }

    /*
     * Because we have no way to get at the GC used to call us,
     * we must rely on the GetSpans vector never changing and stick it
     * in the apDisplayData array.
     */
    return (* apDisplayData[pDrawable->pScreen->myNum].GetSpans) (pDrawable, wMax, ppt, pwidth, nspans);
}

/*
 * apPutImage -- DDX interface (GC)
 *      Remove the cursor if it is in the way of the image to be
 *      put down, then call the "actual" PutImage.
 */
static void
apPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr   pDst;
    GCPtr         pGC;
    int           depth;
    int           x;
    int           y;
    int           w;
    int           h;
    int           format;
    char          *pBits;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;

    if (IsWorthChecking(pDst)) {
        BoxRec    cursorBox;
        if (apCursorLoc (pDst->pScreen, &cursorBox)) {
            register WindowPtr pWin = (WindowPtr)pDst;

            if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
                            x,y,w,h)) {
                                apRemoveCursor();
            }
        }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PutImage) (pDst, pShadowGC, depth, x, y, w, h,
                             leftPad, format, pBits);
}

/*
 * apGetImage -- DDX interface (GC)
 *      Remove the cursor if it overlaps the image to be gotten;
 *      then call the "actual" GetImage.
 */
void
apGetImage (pSrc, x, y, w, h, format, planeMask, pBits)
    DrawablePtr   pSrc;
    int           x;
    int           y;
    int           w;
    int           h;
    unsigned int  format;
    unsigned int  planeMask;
    int           *pBits;
{
    if (pSrc->type == DRAWABLE_WINDOW) {
        BoxRec    cursorBox;

        if (apCursorLoc(pSrc->pScreen, &cursorBox)) {
            register WindowPtr  pWin = (WindowPtr)pSrc;

            if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
                            x,y,w,h)) {
                                apRemoveCursor();
            }
        }
    }

        (* apDisplayData[pSrc->pScreen->myNum].GetImage) (pSrc, x, y, w, h, format, planeMask, pBits);
}

/*
 * apCopyArea -- DDX interface (GC)
 *      Remove the cursor if it overlaps either the source or destination
 *      drawables, then call the "actual" CopyArea routine.
 */
static RegionPtr
apCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr   pSrc;
    DrawablePtr   pDst;
    GCPtr         pGC;
    int           srcx;
    int           srcy;
    int           w;
    int           h;
    int           dstx;
    int           dsty;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;
    BoxRec        cursorBox;
    register WindowPtr  pWin;
    int           out = FALSE;

    if (IsWorthChecking(pSrc) &&
        apCursorLoc(pSrc->pScreen, &cursorBox)) {
            pWin = (WindowPtr)pSrc;

            if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
                            srcx, srcy, w, h)) {
                                apRemoveCursor();
                                out = TRUE;
            }
    }

    if (!out && pDst->type == DRAWABLE_WINDOW &&
        apCursorLoc(pDst->pScreen, &cursorBox)) {
            pWin = (WindowPtr)pDst;
            
            if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
                            dstx, dsty, w, h)) {
                                apRemoveCursor();
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    return (* pShadowGC->CopyArea) (pSrc, pDst, pShadowGC, srcx, srcy,
                             w, h, dstx, dsty);
}

/*
 * apCopyPlane -- DDX interface (GC)
 *      Remove the cursor as necessary and call the "actual"
 *      CopyPlane function.
 */
static RegionPtr
apCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr   pSrc;
    DrawablePtr   pDst;
    register GC   *pGC;
    int           srcx,
                  srcy;
    int           w,
                  h;
    int           dstx,
                  dsty;
    unsigned int  plane;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;
    BoxRec        cursorBox;
    register WindowPtr  pWin;
    int           out = FALSE;

    if (IsWorthChecking(pSrc) &&
        apCursorLoc(pSrc->pScreen, &cursorBox)) {
            pWin = (WindowPtr)pSrc;

            if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
                            srcx, srcy, w, h)) {
                                apRemoveCursor();
                                out = TRUE;
            }
    }

    if (!out && IsWorthChecking(pDst) &&
        apCursorLoc(pDst->pScreen, &cursorBox)) {
            pWin = (WindowPtr)pDst;
            
            if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
                            dstx, dsty, w, h)) {
                                apRemoveCursor();
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    return (* pShadowGC->CopyPlane) (pSrc, pDst, pShadowGC, srcx, srcy, w, h,
                              dstx, dsty, plane);
}

/*
 * apPolyPoint -- DDX interface (GC)
 *      See if any of the points lies within the area covered by the
 *      cursor and remove the cursor if one does.  Then put the points
 *      down via the "actual" PolyPoint routine.
 */
static void
apPolyPoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr       pGC;
    int         mode;           /* Origin or Previous */
    int         npt;
    xPoint      *pptInit;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    register xPoint     *pts;
    register int        nPts;
    register int        xorg;
    register int        yorg;
    BoxRec              cursorBox;

    if (IsWorthChecking(pDrawable) &&
        apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            xorg = ((WindowPtr)pDrawable)->absCorner.x;
            yorg = ((WindowPtr)pDrawable)->absCorner.y;

            if (mode == CoordModeOrigin) {
                for (pts = pptInit, nPts = npt; nPts--; pts++) {
                    if (ORG_OVERLAP(&cursorBox,xorg,yorg,pts->x,pts->y,0,0)){
                        apRemoveCursor();
                        break;
                    }
                }
            } else {
                for (pts = pptInit, nPts = npt; nPts--; pts++) {
                    if (ORG_OVERLAP(&cursorBox,xorg,yorg,pts->x,pts->y,0,0)){
                        apRemoveCursor();
                        break;
                    } else {
                        xorg += pts->x;
                        yorg += pts->y;
                    }
                }
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PolyPoint) (pDrawable, pShadowGC, mode, npt, pptInit);
}

/*
 * apPolylines -- DDX interface (GC)
 *      Find the bounding box of the lines and remove the cursor if
 *      the box overlaps the area covered by the cursor.  Then call
 *      the "actual" Polylines function to draw the lines themselves.
 */
static void
apPolylines (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr   pDrawable;
    GCPtr         pGC;
    int           mode;
    int           npt;
    DDXPointPtr   pptInit;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;
    BoxRec        cursorBox;

    if (IsWorthChecking(pDrawable) &&
        apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            apSaveCursorBox(((WindowPtr)pDrawable)->absCorner.x,
                             ((WindowPtr)pDrawable)->absCorner.y,
                             mode,
			     pShadowGC->lineWidth,
                             pptInit,
                             npt,
                             &cursorBox);
    }
    COMPARE_GCS(pGC,pShadowGC);
    (*pShadowGC->Polylines) (pDrawable, pShadowGC, mode, npt, pptInit);
}

/*
 * apPolySegment -- DDX interface (GC)
 *      Treat each segment as a box and remove the cursor if any box
 *      overlaps the cursor's area.  Then draw the segments via the "actual"
 *      PolySegment routine.  Note that the endpoints of the segments are in no
 *      particular order, so we find the bounding box of the segment
 *      in two comparisons and use that to figure things out.
 */
static void
apPolySegment(pDraw, pGC, nseg, pSegs)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         nseg;
    xSegment    *pSegs;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    register xSegment   *pSeg;
    register int        nSeg;
    register int  	xmin, xmax, ymin, ymax;
    int			lw;
    BoxRec              cursorBox;
    Bool                nuke = FALSE;

    if (IsWorthChecking(pDraw) &&
        apCursorLoc (pDraw->pScreen, &cursorBox)) {
	    xmin = xmax = ((WindowPtr)pDraw)->absCorner.x;
	    ymin = ymax = ((WindowPtr)pDraw)->absCorner.y;
	    lw = pShadowGC->lineWidth >> 1;
	    xmin -= lw;
	    xmax += lw;
	    ymin -= lw;
	    ymax += lw;

            for (nSeg = nseg, pSeg = pSegs; nSeg--; pSeg++) {
                if (pSeg->x1 < pSeg->x2) {
                    if (pSeg->y1 < pSeg->y2) {
                        nuke = BOX_OVERLAP(&cursorBox,
                                           pSeg->x1+xmin,pSeg->y1+ymin,
                                           pSeg->x2+xmax,pSeg->y2+ymax);
                    } else {
                        nuke = BOX_OVERLAP(&cursorBox,
                                           pSeg->x1+xmin,pSeg->y2+ymin,
                                           pSeg->x2+xmax,pSeg->y1+ymax);
                    }
                } else if (pSeg->y1 < pSeg->y2) {
                    nuke = BOX_OVERLAP(&cursorBox,
                                       pSeg->x2+xmin,pSeg->y1+ymin,
                                       pSeg->x1+xmax,pSeg->y2+ymax);
                } else {
                    nuke = BOX_OVERLAP(&cursorBox,
                                       pSeg->x2+xmin,pSeg->y2+ymin,
                                       pSeg->x1+xmax,pSeg->y1+ymax);
                }
                if (nuke) {
                    apRemoveCursor();
                    break;
                }
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PolySegment) (pDraw, pShadowGC, nseg, pSegs);
}

/*
 * apPolyRectangle -- DDX interface (GC)
 *      Remove the cursor if it overlaps any of the rectangles to be
 *      drawn, then draw them using the "actual" PolyRectangle routine.
 */
static void
apPolyRectangle(pDraw, pGC, nrects, pRects)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         nrects;
    xRectangle  *pRects;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    register xRectangle *pRect;
    register int        nRect;
    register int        xorg,
                        yorg,
			lw;
    BoxRec              cursorBox;

    if (IsWorthChecking(pDraw) &&
        apCursorLoc (pDraw->pScreen, &cursorBox)) {
            xorg = ((WindowPtr)pDraw)->absCorner.x;
            yorg = ((WindowPtr)pDraw)->absCorner.y;
	    lw = pShadowGC->lineWidth >> 1;

            for (nRect = nrects, pRect = pRects; nRect--; pRect++) {
		if (ORG_OVERLAP(&cursorBox,xorg,yorg,
				pRect->x - lw, pRect->y - lw,
				(int)pRect->width + pShadowGC->lineWidth,
				(int)pRect->height + pShadowGC->lineWidth)){
                    apRemoveCursor();
                    break;
                }
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PolyRectangle) (pDraw, pShadowGC, nrects, pRects);
}

/*
 * apPolyArc -- DDX interface (GC)
 *      Using the bounding rectangle of each arc, remove the cursor
 *      if it overlaps any arc, then draw all the arcs using the
 *      "actual" PolyArc command.
 */
static void
apPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         narcs;
    xArc        *parcs;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    register xArc       *pArc;
    register int        nArc;
    register int        xorg,
                        yorg,
			lw;
    BoxRec              cursorBox;

    if (IsWorthChecking(pDraw) &&
        apCursorLoc (pDraw->pScreen, &cursorBox)) {
            xorg = ((WindowPtr)pDraw)->absCorner.x;
            yorg = ((WindowPtr)pDraw)->absCorner.y;
	    lw = pShadowGC->lineWidth >> 1;

            for (nArc = narcs, pArc = parcs; nArc--; pArc++) {
		if (ORG_OVERLAP(&cursorBox,xorg,yorg,
				pArc->x - lw, pArc->y - lw,
				(int)pArc->width + pShadowGC->lineWidth,
				(int)pArc->height + pShadowGC->lineWidth)){
                    apRemoveCursor();
                    break;
                }
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PolyArc) (pDraw, pShadowGC, narcs, parcs);
}

/*
 * apFillPolygon -- DDX interface (GC)
 *      Find the bounding box of the polygon to fill and remove the
 *      cursor if it overlaps this box.  Then fill the polygon using
 *      the "actual" FillPolygon routine.
 */
static void
apFillPolygon(pDraw, pGC, shape, mode, count, pPts)
    DrawablePtr         pDraw;
    register GCPtr      pGC;
    int                 shape, mode;
    register int        count;
    DDXPointPtr         pPts;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    BoxRec              cursorBox;

    if (IsWorthChecking(pDraw) &&
        apCursorLoc (pDraw->pScreen, &cursorBox)) {
            apSaveCursorBox(((WindowPtr)pDraw)->absCorner.x,
                             ((WindowPtr)pDraw)->absCorner.y,
                             mode,
			     0,
                             pPts,
                             count,
                             &cursorBox);
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->FillPolygon) (pDraw, pShadowGC, shape, mode, count, pPts);
}

/*
 * apPolyFillRect -- DDX interface (GC)
 *      Remove the cursor if it overlaps any of the filled rectangles
 *      to be drawn; then call the "actual" PolyFillRect routine to
 *      draw them.
 */
static void
apPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr       pGC;
    int         nrectFill;      /* number of rectangles to fill */
    xRectangle  *prectInit;     /* Pointer to first rectangle to fill */
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    register xRectangle *pRect;
    register int        nRect;
    register int        xorg,
                        yorg;
    BoxRec              cursorBox;

    if (IsWorthChecking(pDrawable) &&
        apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            xorg = ((WindowPtr)pDrawable)->absCorner.x;
            yorg = ((WindowPtr)pDrawable)->absCorner.y;

            for (nRect = nrectFill, pRect = prectInit; nRect--; pRect++) {
                if (ORGRECT_OVERLAP(&cursorBox,xorg,yorg,pRect)){
                    apRemoveCursor();
                    break;
                }
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PolyFillRect) (pDrawable, pShadowGC, nrectFill, prectInit);
}

/*
 * apPolyFillArc -- DDX interface (GC)
 *      See if the cursor overlaps any of the bounding boxes for the
 *      filled arcs and remove it if it does.  Then use the "actual"
 *      PolyFillArc to fill them.
 */
static void
apPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         narcs;
    xArc        *parcs;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    register xArc       *pArc;
    register int        nArc;
    register int        xorg,
                        yorg;
    BoxRec              cursorBox;

    if (IsWorthChecking(pDraw) &&
        apCursorLoc (pDraw->pScreen, &cursorBox)) {
            xorg = ((WindowPtr)pDraw)->absCorner.x;
            yorg = ((WindowPtr)pDraw)->absCorner.y;

            for (nArc = narcs, pArc = parcs; nArc--; pArc++) {
                if (ORGRECT_OVERLAP(&cursorBox,xorg,yorg,pArc)){
                    apRemoveCursor();
                    break;
                }
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PolyFillArc) (pDraw, pShadowGC, narcs, parcs);
}


/*
 * apText -- Driver internal code
 *      Find the extent of a text operation and remove the cursor if they
 *      overlap.  This is a helper for the following actual text operations.
 */
static int
apText(pDraw, pGC, x, y, count, chars, fontEncoding, drawFunc)
    DrawablePtr   pDraw;
    GCPtr         pGC;
    int           x,
                  y;
    int           count;
    char          *chars;
    FontEncoding  fontEncoding;
    void          (*drawFunc)();
{
    CharInfoPtr   *charinfo;
    unsigned int  n,
                  w = 0;
    register int  xorg,
                  yorg;
    ExtentInfoRec extents;
    BoxRec        cursorBox;

    charinfo = (CharInfoPtr *)ALLOCATE_LOCAL (count * sizeof(CharInfoPtr));
    if (charinfo == (CharInfoPtr *)NULL) {
        return x;
    }

    GetGlyphs(pGC->font, count, chars, fontEncoding, &n, charinfo);

    if (n != 0) {
	QueryGlyphExtents(pGC->font, charinfo, n, &extents);
	w = extents.overallWidth;

	if (apCursorLoc (pDraw->pScreen, &cursorBox)) {
	  xorg = ((WindowPtr) pDraw)->absCorner.x;
	  yorg = ((WindowPtr) pDraw)->absCorner.y;
    
	  if (BOX_OVERLAP(&cursorBox,
			  x + xorg + extents.overallLeft,
			  y + yorg - extents.overallAscent,
			  x + xorg + extents.overallRight,
			  y + yorg + extents.overallDescent)) {
	      apRemoveCursor();
	  }
	}

        (* drawFunc)(pDraw, pGC, x, y, n, charinfo, pGC->font->pGlyphs);
    }

    DEALLOCATE_LOCAL(charinfo);
    return x + w;
}

/*
 * apPolyText8 -- DDX interface (GC)
 *      If the text operation will overlap the cursor, remove the cursor.
 *      Then call the "actual" PolyText8 function to draw the text string.
 */
static int
apPolyText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         x, y;
    int         count;
    char        *chars;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;

    COMPARE_GCS(pGC,pShadowGC);
    if (((apDisplayData[pGC->pScreen->myNum].noCrsrChk & NO_CRSR_CHK_PTEXT) == 0) &&
        IsWorthChecking(pDraw)) {
        return apText (pDraw, pShadowGC, x, y, count, chars, Linear8Bit,
                        pShadowGC->PolyGlyphBlt);
    } else {
        return (* pShadowGC->PolyText8)(pDraw, pShadowGC, x, y, count, chars);
    }
}

/*
 * apPolyText16 -- DDX interface (GC)
 *      If the text operation will overlap the cursor, remove the cursor.
 *      Then call the "actual" PolyText16 function to draw the text string.
 */
static int
apPolyText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         x, y;
    int         count;
    unsigned short *chars;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;

    COMPARE_GCS(pGC,pShadowGC);
    if (((apDisplayData[pGC->pScreen->myNum].noCrsrChk & NO_CRSR_CHK_PTEXT) == 0) &&
        IsWorthChecking(pDraw)) {
        return apText (pDraw, pShadowGC, x, y, count, chars,
                        (pShadowGC->font->pFI->lastRow == 0 ?
                         Linear16Bit : TwoD16Bit),
                        pShadowGC->PolyGlyphBlt);
    } else {
        return (* pShadowGC->PolyText16) (pDraw, pShadowGC, x, y,
                                          count, chars);
    }
}

/*
 * apImageText8 -- DDX interface (GC)
 *      If the text operation will overlap the cursor, remove the cursor.
 *      Then call the "actual" ImageText8 function to draw the text string.
 */
static void
apImageText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         x, y;
    int         count;
    char        *chars;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;

    COMPARE_GCS(pGC,pShadowGC);
    if (((apDisplayData[pGC->pScreen->myNum].noCrsrChk & NO_CRSR_CHK_PTEXT) == 0) &&
        IsWorthChecking(pDraw)) {
        (void) apText (pDraw, pShadowGC, x, y, count, chars,
                        Linear8Bit, pShadowGC->ImageGlyphBlt);
    } else {
        (* pShadowGC->ImageText8) (pDraw, pShadowGC, x, y, count, chars);
    }
}

/*
 * apImageText16 -- DDX interface (GC)
 *      If the text operation will overlap the cursor, remove the cursor.
 *      Then call the "actual" ImageText16 function to draw the text string.
 */
static void
apImageText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         x, y;
    int         count;
    unsigned short *chars;
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;

    COMPARE_GCS(pGC,pShadowGC);
    if (((apDisplayData[pGC->pScreen->myNum].noCrsrChk & NO_CRSR_CHK_PTEXT) == 0) &&
        IsWorthChecking(pDraw)) {
        (void) apText (pDraw, pShadowGC, x, y, count, chars,
                        (pShadowGC->font->pFI->lastRow == 0 ?
                         Linear16Bit : TwoD16Bit),
                        pShadowGC->ImageGlyphBlt);
    } else {
        (* pShadowGC->ImageText16) (pDraw, pShadowGC, x, y, count, chars);
    }
}

/*
 * apImageGlyphBlt -- DDX interface (GC)
 *      If the ImageGlyphBlt operation will overlap the cursor, remove the cursor.
 *      Then call the "actual" ImageGlyphBlt function.
 *      Under gpr text, this gets called only if gpr can't handle it,
 *      and the mi[Image|Poly]Text[8|16] function is being used.
 */
static void
apImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC          *pGC;
    int         x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;          /* array of character info */
    pointer     pglyphBase;     /* start of array of glyphs */
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    BoxRec              cursorBox;
    ExtentInfoRec       extents;
    register int        xorg,
                        yorg;

    if (IsWorthChecking(pDrawable) &&
        apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            QueryGlyphExtents (pGC->font, ppci, nglyph, &extents);
            xorg = ((WindowPtr)pDrawable)->absCorner.x + x;
            yorg = ((WindowPtr)pDrawable)->absCorner.y + y;
            if (BOX_OVERLAP(&cursorBox,xorg+extents.overallLeft,
                            yorg-extents.overallAscent,
                            xorg+extents.overallRight,
                            yorg+extents.overallDescent)) {
                                apRemoveCursor();
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->ImageGlyphBlt) (pDrawable, pShadowGC, x, y, nglyph,
                                  ppci, pglyphBase);
}

/*
 * apPolyGlyphBlt -- DDX interface (GC)
 *      If the PolyGlyphBlt operation will overlap the cursor, remove the cursor.
 *      Then call the "actual" PolyGlyphBlt function.
 *      Under gpr text, this gets called only if gpr can't handle it,
 *      and the mi[Image|Poly]Text[8|16] function is being used.
 */
static void
apPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr       pGC;
    int         x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;          /* array of character info */
    char        *pglyphBase;    /* start of array of glyphs */
{
    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    BoxRec              cursorBox;
    ExtentInfoRec       extents;
    register int        xorg,
                        yorg;

    if (IsWorthChecking(pDrawable) &&
        apCursorLoc (pDrawable->pScreen, &cursorBox)) {
            QueryGlyphExtents (pGC->font, ppci, nglyph, &extents);
            xorg = ((WindowPtr)pDrawable)->absCorner.x + x;
            yorg = ((WindowPtr)pDrawable)->absCorner.y + y;
            if (BOX_OVERLAP(&cursorBox,xorg+extents.overallLeft,
                            yorg-extents.overallAscent,
                            xorg+extents.overallRight,
                            yorg+extents.overallDescent)){
                                apRemoveCursor();
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PolyGlyphBlt) (pDrawable, pShadowGC, x, y,
                                nglyph, ppci, pglyphBase);
}

/*
 * apPushPixels -- DDX interface (GC)
 *      Remove the cursor if it overlaps the bounds of the PushPixels operation.
 *      Then call the "actual" PushPixels routine.
 */
static void
apPushPixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr       pGC;
    PixmapPtr   pBitMap;
    DrawablePtr pDst;
    int         w, h, x, y;
{

    register GCPtr      pShadowGC = (GCPtr) pGC->devPriv;
    BoxRec              cursorBox;
    register int        xorg,
                        yorg;

    if (IsWorthChecking(pDst) &&
        apCursorLoc (pDst->pScreen, &cursorBox)) {
            xorg = ((WindowPtr)pDst)->absCorner.x + x;
            yorg = ((WindowPtr)pDst)->absCorner.y + y;

            if (BOX_OVERLAP(&cursorBox,xorg,yorg,xorg+w,yorg+h)){
                apRemoveCursor();
            }
    }

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->PushPixels) (pShadowGC, pBitMap, pDst, w, h, x, y);
}

/*
 * apLineHelper -- (sort of) DDX interface (GC)
 *      Just a pass-thru for the LineHelper GC routine.
 */
static void
apLineHelper (pDraw, pGC, caps, npt, pPts, xOrg, yOrg)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int         caps;
    int         npt;
    SppPointPtr pPts;
    int         xOrg, yOrg;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;

    COMPARE_GCS(pGC,pShadowGC);
    (* pShadowGC->LineHelper) (pDraw, pGC, caps, npt, pPts, xOrg, yOrg);
}

/*
 * apChangeClip -- (sort of) DDX interface (GC)
 *      Just a pass-thru for the ChangeClip GC routine.
 */
static void
apChangeClip (pGC, type, pValue, numRects)
    GCPtr         pGC;
    int           type;
    pointer       pValue;
    int           numRects;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;

    /* COMPARE_GCS(pGC,pShadowGC); */
    (* pShadowGC->ChangeClip) (pShadowGC, type, pValue, numRects);
    /*
     *        Now copy the clip info back from the shadow to the real
     *        GC so that if we use it as the source for a CopyGC,
     *        the clip info will get copied along with everything
     *        else.
     */
    pGC->clientClip = pShadowGC->clientClip;
    pGC->clientClipType = pShadowGC->clientClipType;
}

/*
 * apDestroyClip -- (sort of) DDX interface (GC)
 *      Just a pass-thru for the DestroyClip GC routine.
 */
static void
apDestroyClip (pGC)
    GCPtr   pGC;
{
    register GCPtr pShadowGC = (GCPtr) pGC->devPriv;

    /* COMPARE_GCS(pGC,pShadowGC); */
    (* pShadowGC->DestroyClip) (pShadowGC);
    /* presumably this is NULL,  or junk,  or something.... */
    pGC->clientClip = pShadowGC->clientClip;
    pGC->clientClipType = pShadowGC->clientClipType;
}

/*-
 *-----------------------------------------------------------------------
 * apCopyClip --
 *	Ditto for copying the clipping region of the GC. Note that it
 *	is possible for us to be called to copy the clip from a shadow
 *	GC to a top-level (one of our) GC (the backing-store scheme in
 *	MI will do such a thing), so we must be careful.
 *
 * Results:
 *    None.
 *
 * Side Effects:
 *    ???
 *-----------------------------------------------------------------------
 */
void
apCopyClip (pgcDst, pgcSrc)
    GCPtr   pgcDst, pgcSrc;
{
     register GCPtr pShadowSrcGC = (GCPtr) pgcSrc->devPriv;
     register GCPtr pShadowDstGC = (GCPtr) pgcDst->devPriv;

    /* COMPARE_GCS(pgcSrc, pShadowSrcGC); */
    /* COMPARE_GCS(pgcDst, pShadowDstGC); */
    if ((void (*)())pgcSrc->FillSpans != (void (*)())apFillSpans) {
	/*
	 * The source is a shadow GC in its own right, so don't try to
	 * reference through the devPriv field...
	 */
	pShadowSrcGC = pgcSrc;
    }
     (* pShadowDstGC->CopyClip) (pShadowDstGC, pShadowSrcGC);
     pgcDst->clientClip = pShadowDstGC->clientClip;
     pgcDst->clientClipType = pShadowDstGC->clientClipType;
   }

/*
 * apDestroyGC -- DDX interface (GCInterest)
 *      Function called when a GC is being freed. Simply unlinks and frees
 *      the GCInterest structure.
 */
static void
apDestroyGC (pGC, pGCI)
    GCPtr          pGC; /* GC pGCI is attached to */
    GCInterestPtr  pGCI;        /* GCInterest being destroyed */
{
    short         sn = pGC->pScreen->myNum;

    if (pGC->devPriv) {
        if ( (GCPtr)apDisplayData[sn].lastShadowGC == (GCPtr)pGC->devPriv )
           apDisplayData[sn].lastShadowGC = 0;
        FreeGC ((GCPtr)pGC->devPriv);
    }
    if ( apDisplayData[sn].lastGC == pGC )
           apDisplayData[sn].lastGC = 0;
    Xfree (pGCI);
}

/*
 * apValidateGC -- DDX interface (GCInterest)
 *      Called when a GC is about to be used for drawing. Copies all
 *      changes from the GC to its shadow and validates the shadow.
 */
static void
apValidateGC (pGC, pGCI, changes, pDrawable)
    GCPtr         pGC;
    GCInterestPtr pGCI;
    Mask          changes;
    DrawablePtr   pDrawable;
{
    GCPtr pShadowGC = (GCPtr) pGC->devPriv;
    short         sn = pGC->pScreen->myNum;

    /* here's the story -- we keep only one gpr attribute block in the whole world
       therefore anytime a different drawable/GC combination is used, we must revalidate --
       not just when a GC changes or when it's used with a different drawable.  So when
       we see a different GC come through, we clobber the serial number in the old one
       to force it to be revalidated later.  We do all this using the shadow GC because
       not all GCs are shadowed, so this code therefore needs to be in plfbgc as well,
       and can only see the shadow there */
    if (pShadowGC != apDisplayData[sn].lastShadowGC) {
        if (apDisplayData[sn].lastGC)
        {
            apDisplayData[sn].lastGC->serialNumber |= GC_CHANGE_SERIAL_BIT;  /* set high bit to force revalidate */
            apDisplayData[sn].lastShadowGC->serialNumber |= GC_CHANGE_SERIAL_BIT;  /* in both GCs */
        }
        apDisplayData[sn].lastGC = pGC;
        apDisplayData[sn].lastShadowGC = pShadowGC;
    }

    if ( pGC->depth != pDrawable->depth )
        FatalError( "apValidateGC: depth mismatch.\n" );

  /*
   *  apValidateGC copies the GC to the shadow.  Alas,
   *  apChangeClip has stored the new clip in the shadow,
   *  where it will be overwritten,  unless we pretend
   *  that the clip hasn't changed.
   */
      changes &= ~GCClipMask;

    CopyGC (pGC, pShadowGC, changes);
    pShadowGC->serialNumber = pGC->serialNumber;
    COMPARE_GCS(pGC,pShadowGC);
    ValidateGC (pDrawable, pShadowGC);
}
        
/*
 * apCopyGC -- DDX interface (GCInterest)
 *      Called when a GC with its shadow is the destination of a copy.
 *      Calls CopyGC to transfer the changes to the shadow GC as well.
 *      Should not be used for the CopyGCSource since we like to copy from
 *      the real GC to the shadow using CopyGC.
 */
static void
apCopyGC (pGCDst, pGCI, changes, pGCSrc)
    GCPtr         pGCDst;
    GCInterestPtr pGCI;
    int           changes;
    GCPtr         pGCSrc;
{
    CopyGC (pGCDst, (GCPtr) pGCDst->devPriv, (pGCDst->stateChanges|changes));
    COMPARE_GCS(pGCDst,(GCPtr) pGCDst->devPriv);
}

/*
 * Array of functions to replace the functions in the GC.
 * Caveat: Depends on the ordering of functions in the GC structure.
 */
static void (* apGCFuncs[]) () = {
    apFillSpans,
    apSetSpans,

    apPutImage,
    (void (*)()) apCopyArea,
    (void (*)()) apCopyPlane,
    apPolyPoint,
    apPolylines,
    apPolySegment,
    apPolyRectangle,
    apPolyArc,
    apFillPolygon,
    apPolyFillRect,
    apPolyFillArc,
    (void(*)())apPolyText8,
    (void(*)())apPolyText16,
    apImageText8,
    apImageText16,
    apImageGlyphBlt,
    apPolyGlyphBlt,
    apPushPixels,
    apLineHelper,
    apChangeClip,
    apDestroyClip,
    apCopyClip
};

/*
 * apCreateGC -- DDX interface (screen)
 *      This function is used to get our own validation hooks into each
 *      GC to preserve the cursor. It calls the regular creation routine
 *      for the screen and then, if that was successful, tacks another
 *      GCInterest structure onto the GC *after* the one placed on by
 *      the screen-specific CreateGC.
 *
 *      Returns TRUE if created OK, FALSE otherwise.
 */
Bool
apCreateGC (pGC)
    GCPtr       pGC;    /* The GC to play with */
{
    GCInterestPtr       pGCI;
    register GCPtr      pShadowGC;
    int                 i;
    
    if ((*apDisplayData[pGC->pScreen->myNum].CreateGC) (pGC)) {

        if (pGC->depth != pGC->pScreen->rootDepth) {
            /* This GC will never be used for painting the screen,  so no shadow needed */
            return TRUE;
        }

        pShadowGC = (GCPtr) Xalloc (sizeof (GC));
        if (pShadowGC == (GCPtr)NULL) {
            return FALSE;
        }
        
        *pShadowGC = *pGC;
        pGC->devPriv = (pointer)pShadowGC;
        bcopy (apGCFuncs, &pGC->FillSpans, sizeof (apGCFuncs));
        
        pGCI = (GCInterestPtr) Xalloc (sizeof (GCInterestRec));
        if (!pGCI) {
            return FALSE;
        }

        /*
         * Any structure being shared between these two GCs must have its
         * reference count incremented. This includes:
         *  font, tile, stipple.
         * Anything which doesn't have a reference count must be duplicated:
         *  pCompositeClip, pAbsClientRegion.
         * 
         */
        if (pGC->font) {
            pGC->font->refcnt++;
        }
        if (pGC->tile) {
            pGC->tile->refcnt++;
        }
        if (pGC->stipple) {
            pGC->stipple->refcnt++;
        }
        pShadowGC->dash = (unsigned char *)
                Xalloc(pGC->numInDashList * sizeof(unsigned char));
        for (i=0; i<pGC->numInDashList; i++)
                pShadowGC->dash[i] = pGC->dash[i];

#ifdef  notdef
        if (pGC->pCompositeClip) {
            pShadowGC->pCompositeClip =
                (* pGC->pScreen->RegionCreate) (NULL, 1);
            (* pGC->pScreen->RegionCopy) (pShadowGC->pCompositeClip,
                                          pGC->pCompositeClip);
        }
        if (pGC->pAbsClientRegion) {
            pShadowGC->pAbsClientRegion=
                (* pGC->pScreen->RegionCreate) (NULL, 1);
            (* pGC->pScreen->RegionCopy) (pShadowGC->pAbsClientRegion,
                                          pGC->pAbsClientRegion);
        }
#endif  notdef
        
        pGC->pNextGCInterest = pGCI;
        pGC->pLastGCInterest = pGCI;
        pGCI->pNextGCInterest = (GCInterestPtr) &pGC->pNextGCInterest;
        pGCI->pLastGCInterest = (GCInterestPtr) &pGC->pNextGCInterest;
        pGCI->length = sizeof(GCInterestRec);
        pGCI->owner = 0;                    /* server owns this */
        pGCI->ValInterestMask = ~0;         /* interested in everything */
        pGCI->ValidateGC = apValidateGC;
        pGCI->ChangeInterestMask = 0;       /* interested in nothing */
        pGCI->ChangeGC = (int (*)()) NULL;
        pGCI->CopyGCSource = (void (*)())NULL;
        pGCI->CopyGCDest = apCopyGC;
        pGCI->DestroyGC = apDestroyGC;
        

        /*
         * Because of this weird way of handling the GCInterest lists,
         * we need to modify the output library's GCInterest structure to
         * point to the pNextGCInterest field of the shadow GC...
         */
        pGCI = pShadowGC->pNextGCInterest;
        pGCI->pLastGCInterest = pGCI->pNextGCInterest =
            (GCInterestPtr) &pShadowGC->pNextGCInterest;

        return TRUE;
    } else {
        return FALSE;
    }
}
