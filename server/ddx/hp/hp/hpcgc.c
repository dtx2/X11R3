/* hpcgc.c : hp soft cursor GC stuff - a munged and hacked sunGC.c */

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

/*-
 * sunGC.c --
 *	Functions to support the meddling with GC's we do to preserve
 *	the software cursor...
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */
#ifndef lint
#ifdef VERBOSE_REV_INFO
static char rcsid[] =
	"$XConsortium: hpcgc.c,v 1.3 88/09/06 15:24:51 jim Exp $ SPRITE (Berkeley)";
#endif
#endif lint

#include    "sun.h"

#include    "X.h"
#include    "Xmd.h"
#include    "extnsionst.h"
#include    "regionstr.h"
#include    "windowstr.h"
#include    "fontstruct.h"
#include    "dixfontstr.h"
#include    "../cfb/cfb.h"

#define RemoveCursor(pDrawable) \
  (*((cfbPrivScreenPtr)pDrawable->pScreen->devPrivate)->CursorOff) \
	(pDrawable->pScreen)

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
    ORG_OVERLAP((pCbox),(xorg),(yorg),(pRect)->x,(pRect)->y,(pRect)->width, \
		(pRect)->height)
/*
 * Overlap BoxPtr and horizontal span
 */
#define SPN_OVERLAP(pCbox,y,x,w) BOX_OVERLAP((pCbox),(x),(y),(x)+(w),(y))

void hpText();

/*-
 * sunSaveCursorBox --
 *	Given an array of points, figure out the bounding box for the
 *	series and remove the cursor if it overlaps that box.
 *
 */
static Bool
hpSaveCursorBox (xorg, yorg, mode, pPts, nPts, pCursorBox)
  register int xorg,   	    /* X-Origin for points */
	yorg;   	    /* Y-Origin for points */
  int	mode;	/* CoordModeOrigin or * CoordModePrevious */
  register DDXPointPtr	pPts;   	    /* Array of points */
  int			nPts;   	    /* Number of points */
  register BoxPtr 	pCursorBox;	    /* Bounding box for cursor */
{
    register int minx, miny, maxx, maxy;

    minx = maxx = pPts->x + xorg; miny = maxy = pPts->y + yorg;
    pPts++; nPts--;

    if (mode == CoordModeOrigin) {
	while (nPts--) {
	    minx = min(minx, pPts->x + xorg);
	    maxx = max(maxx, pPts->x + xorg);
	    miny = min(miny, pPts->y + yorg);
	    maxy = max(maxy, pPts->y + yorg);
	    pPts++;
	}
    } else {
	xorg = minx; yorg = miny;
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
  return BOX_OVERLAP(pCursorBox,minx,miny,maxx,maxy);
}

/*-
 * sunFillSpans --
 *	Remove the cursor if any of the spans overlaps the area covered
 *	by the cursor. This assumes the points have been translated
 *	already, though perhaps it shouldn't...
 */
void hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  if (pDrawable->type == DRAWABLE_WINDOW)
  {
    BoxRec	  cursorBox;

    if (hpCursorLoc (pDrawable->pScreen, &cursorBox))
    {
      register DDXPointPtr  pts;
      register int    	    *widths, nPts;

      for (pts = pptInit, widths = pwidthInit, nPts = nInit;
	nPts--; pts++, widths++)
      {
	if (SPN_OVERLAP(&cursorBox,pts->y,pts->x,*widths))
		{ RemoveCursor(pDrawable); break; }
      }
    }
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunSetSpans --
 *	Remove the cursor if any of the horizontal segments overlaps
 *	the area covered by the cursor. This also assumes the spans
 *	have been translated from the window's coordinates to the
 *	screen's.
 *-----------------------------------------------------------------------
 */
void hpSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc, *pwidth, nspans, fSorted;
    register DDXPointPtr ppt;
{
  if (pDrawable->type == DRAWABLE_WINDOW)
  {
    BoxRec	  cursorBox;

    if (hpCursorLoc (pDrawable->pScreen, &cursorBox))
    {
      register DDXPointPtr    pts;
      register int    	    *widths, nPts;

      for (pts = ppt, widths = pwidth, nPts = nspans;
	nPts--; pts++, widths++)
      {
	if (SPN_OVERLAP(&cursorBox,pts->y,pts->x,*widths))
		{ RemoveCursor(pDrawable); break; }
      }
    }
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunGetSpans --
 *	Remove the cursor if any of the desired spans overlaps the cursor.
 *-----------------------------------------------------------------------
 */
void hpGetSpans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
  if (pDrawable->type == DRAWABLE_WINDOW)
  {
    BoxRec	  cursorBox;

    if (hpCursorLoc (pDrawable->pScreen, &cursorBox))
    {
      register DDXPointPtr    pts;
      register int    	    *widths, nPts, xorg,yorg;

      xorg = ((WindowPtr)pDrawable)->absCorner.x;
      yorg = ((WindowPtr)pDrawable)->absCorner.y;

      for (pts = ppt, widths = pwidth, nPts = nspans;
	nPts--; pts++, widths++)
      {
	if (SPN_OVERLAP(&cursorBox,pts->y+yorg, pts->x+xorg,*widths))
		{ RemoveCursor(pDrawable); break; }
      }
    }
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunPutImage --
 *	Remove the cursor if it is in the way of the image to be
 *	put down...
 *-----------------------------------------------------------------------
 */
void hpPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int		  depth, x, y, w, h, format;
    char    	  *pBits;
{
  if (pDst->type == DRAWABLE_WINDOW)
  {
    BoxRec	  cursorBox;

    if (hpCursorLoc (pDst->pScreen, &cursorBox))
    {
      register WindowPtr pWin = (WindowPtr)pDst;

      if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,x,y,w,h))
	RemoveCursor(pDst);
    }
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunGetImage --
 *	Remove the cursor if it overlaps the image to be gotten.
 *-----------------------------------------------------------------------
 */
void hpGetImage (pSrc, x, y, w, h, format, planeMask, pBits)
    DrawablePtr	  pSrc;
    int	    	  x, y, w, h, *pBits;
    unsigned int  format, planeMask;
{
  if (pSrc->type == DRAWABLE_WINDOW)
  {
    BoxRec	  cursorBox;

    if (hpCursorLoc(pSrc->pScreen, &cursorBox))
    {
      register WindowPtr	pWin = (WindowPtr)pSrc;

      if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,x,y,w,h))
	RemoveCursor(pSrc);
    }
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunCopyArea --
 *	Remove the cursor if it overlaps either the source or destination
 *	drawables, then call the screen-specific CopyArea routine.
 *-----------------------------------------------------------------------
 */
void hpCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc, pDst;
    GCPtr   	  pGC;
    int	    	  srcx, srcy, w, h, dstx, dsty;
{
  BoxRec	  cursorBox;
  register WindowPtr	pWin;
  int	    	  out = FALSE;

  if (pSrc->type == DRAWABLE_WINDOW &&
	hpCursorLoc(pSrc->pScreen, &cursorBox))
  {
    pWin = (WindowPtr)pSrc;
    if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
    	srcx, srcy, w, h)) { RemoveCursor(pSrc); out = TRUE; }
  }

  if (!out && pDst->type == DRAWABLE_WINDOW &&
	hpCursorLoc(pDst->pScreen, &cursorBox))
  {
    pWin = (WindowPtr)pDst;
    if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
	dstx, dsty, w, h)) RemoveCursor(pDst);
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunCopyPlane --
 *	Remove the cursor as necessary and call the screen-specific
 *	CopyPlane function.
 *-----------------------------------------------------------------------
 */
void hpCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc, pDst;
    register GC   *pGC;
    int     	  srcx, srcy, w,h, dstx,dsty;
    unsigned int  plane;
{
  BoxRec	  cursorBox;
  register WindowPtr	pWin;
  int	    	  out = FALSE;

  if (pSrc->type == DRAWABLE_WINDOW &&
	hpCursorLoc(pSrc->pScreen, &cursorBox))
  {
    pWin = (WindowPtr)pSrc;
    if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
	srcx, srcy, w, h)) { RemoveCursor(pSrc); out = TRUE; }
    }

    if (!out && pDst->type == DRAWABLE_WINDOW &&
	hpCursorLoc(pDst->pScreen, &cursorBox))
    {
      pWin = (WindowPtr)pDst;
      if (ORG_OVERLAP(&cursorBox,pWin->absCorner.x,pWin->absCorner.y,
	dstx, dsty, w, h)) RemoveCursor(pDst);
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyPoint --
 *	See if any of the points lies within the area covered by the
 *	cursor and remove the cursor if one does. Then put the points
 *	down.
 *-----------------------------------------------------------------------
 */
void hpPolyPoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
  register GCPtr 	pShadowGC = (GCPtr) pGC->devPriv;
  register xPoint 	*pts;
  register int  	nPts, xorg, yorg;
  BoxRec  	  	cursorBox;

  if (pDrawable->type == DRAWABLE_WINDOW &&
	hpCursorLoc (pDrawable->pScreen, &cursorBox))
  {
    xorg = ((WindowPtr)pDrawable)->absCorner.x;
    yorg = ((WindowPtr)pDrawable)->absCorner.y;

    if (mode == CoordModeOrigin)
    {
      for (pts = pptInit, nPts = npt; nPts--; pts++)
	if (ORG_OVERLAP(&cursorBox,xorg,yorg,pts->x,pts->y,0,0))
	  { RemoveCursor(pDrawable); break; }
    }
    else
      for (pts = pptInit, nPts = npt; nPts--; pts++)
	if (ORG_OVERLAP(&cursorBox,xorg,yorg,pts->x,pts->y,0,0))
	  { RemoveCursor(pDrawable); break; }
        else { xorg += pts->x; yorg += pts->y; }
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunPolylines --
 *	Find the bounding box of the lines and remove the cursor if
 *	the box overlaps the area covered by the cursor. Then call
 *	the screen's Polylines function to draw the lines themselves.
 *-----------------------------------------------------------------------
 */
void hpPolylines (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  BoxRec  	  cursorBox;

  if (pDrawable->type == DRAWABLE_WINDOW &&
    hpCursorLoc (pDrawable->pScreen, &cursorBox) &&
    hpSaveCursorBox(((WindowPtr)pDrawable)->absCorner.x,
	((WindowPtr)pDrawable)->absCorner.y,mode, pptInit, npt, &cursorBox))
      RemoveCursor(pDrawable);
}

/*-
 * sunPolySegment --
 *	Treat each segment as a box and remove the cursor if any box
 *	overlaps the cursor's area. Then draw the segments. Note that
 *	the endpoints of the segments are in no way guaranteed to be
 *	in the right order, so we find the bounding box of the segment
 *	in two comparisons and use that to figure things out.
 */
void hpPolySegment(pDraw, pGC, nseg, pSegs)
    DrawablePtr pDraw;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
  register xSegment	*pSeg;
  register int  	nSeg, xorg,yorg;
  BoxRec  	  	cursorBox;
  Bool    	  	nuke = FALSE;

  if (pDraw->type == DRAWABLE_WINDOW &&
	hpCursorLoc (pDraw->pScreen, &cursorBox))
  {
    xorg = ((WindowPtr)pDraw)->absCorner.x;
    yorg = ((WindowPtr)pDraw)->absCorner.y;

    for (nSeg = nseg, pSeg = pSegs; nSeg--; pSeg++)
    {
      if (pSeg->x1 < pSeg->x2)
      {
	if (pSeg->y1 < pSeg->y2)
	  nuke = BOX_OVERLAP(&cursorBox,
	    pSeg->x1+xorg,pSeg->y1+yorg, pSeg->x2+xorg,pSeg->y2+yorg);
	else
	  nuke = BOX_OVERLAP(&cursorBox,
	    pSeg->x1+xorg,pSeg->y2+yorg, pSeg->x2+xorg,pSeg->y1+yorg);
      }
      else
	if (pSeg->y1 < pSeg->y2)
	  nuke = BOX_OVERLAP(&cursorBox,
	    pSeg->x2+xorg,pSeg->y1+yorg, pSeg->x1+xorg,pSeg->y2+yorg);
	else
	  nuke = BOX_OVERLAP(&cursorBox,
	    pSeg->x2+xorg,pSeg->y2+yorg, pSeg->x1+xorg,pSeg->y1+yorg);
      if (nuke) { RemoveCursor(pDraw); break; }
    }
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyRectangle --
 *	Remove the cursor if it overlaps any of the rectangles to be
 *	drawn, then draw them.
 *-----------------------------------------------------------------------
 */
void hpPolyRectangle(pDraw, pGC, nrects, pRects)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
    register xRectangle	*pRect;
    register int  	nRect, xorg,yorg;
    BoxRec  	  	cursorBox;

    if (pDraw->type == DRAWABLE_WINDOW &&
	hpCursorLoc (pDraw->pScreen, &cursorBox)) {
	    xorg = ((WindowPtr)pDraw)->absCorner.x;
	    yorg = ((WindowPtr)pDraw)->absCorner.y;

	    for (nRect = nrects, pRect = pRects; nRect--; pRect++) {
		if (ORGRECT_OVERLAP(&cursorBox,xorg,yorg,pRect))
		{ RemoveCursor(pDraw); break; }
	    }
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyArc --
 *	Using the bounding rectangle of each arc, remove the cursor
 *	if it overlaps any arc, then draw all the arcs.
 *-----------------------------------------------------------------------
 */
void hpPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc	*pArc;
    register int  	nArc, xorg,yorg;
    BoxRec  	  	cursorBox;

    if (pDraw->type == DRAWABLE_WINDOW &&
	hpCursorLoc (pDraw->pScreen, &cursorBox)) {
	    xorg = ((WindowPtr)pDraw)->absCorner.x;
	    yorg = ((WindowPtr)pDraw)->absCorner.y;

	    for (nArc = narcs, pArc = parcs; nArc--; pArc++) {
		if (ORGRECT_OVERLAP(&cursorBox,xorg,yorg,pArc))
		{ RemoveCursor(pDraw); break; }
	    }
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunFillPolygon --
 *	Find the bounding box of the polygon to fill and remove the
 *	cursor if it overlaps this box...
 *-----------------------------------------------------------------------
 */
void hpFillPolygon(pDraw, pGC, shape, mode, count, pPts)
    DrawablePtr		pDraw;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
  BoxRec cursorBox;

  if (pDraw->type == DRAWABLE_WINDOW &&
    hpCursorLoc (pDraw->pScreen, &cursorBox) &&
    hpSaveCursorBox(((WindowPtr)pDraw)->absCorner.x,
	((WindowPtr)pDraw)->absCorner.y, mode, pPts, count, &cursorBox))
      RemoveCursor(pDraw);
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyFillRect --
 *	Remove the cursor if it overlaps any of the filled rectangles
 *	to be drawn by the output routines.
 *-----------------------------------------------------------------------
 */
void hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    register xRectangle	*pRect;
    register int  	nRect, xorg,yorg;
    BoxRec  	  	cursorBox;

    if (pDrawable->type == DRAWABLE_WINDOW &&
	hpCursorLoc (pDrawable->pScreen, &cursorBox)) {
	    xorg = ((WindowPtr)pDrawable)->absCorner.x;
	    yorg = ((WindowPtr)pDrawable)->absCorner.y;

	    for (nRect = nrectFill, pRect = prectInit; nRect--; pRect++) {
		if (ORGRECT_OVERLAP(&cursorBox,xorg,yorg,pRect))
		{ RemoveCursor(pDrawable); break; }
	    }
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyFillArc --
 *	See if the cursor overlaps any of the bounding boxes for the
 *	filled arc and remove it if it does.
 *-----------------------------------------------------------------------
 */
void hpPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc	*pArc;
    register int  	nArc, xorg,yorg;
    BoxRec  	  	cursorBox;

    if (pDraw->type == DRAWABLE_WINDOW &&
	hpCursorLoc (pDraw->pScreen, &cursorBox)) {
	    xorg = ((WindowPtr)pDraw)->absCorner.x;
	    yorg = ((WindowPtr)pDraw)->absCorner.y;

	    for (nArc = narcs, pArc = parcs; nArc--; pArc++) {
		if (ORGRECT_OVERLAP(&cursorBox,xorg,yorg,pArc))
		{ RemoveCursor(pDraw); break; }
	    }
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyText8 --
 *-----------------------------------------------------------------------
 */
void hpPolyText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    char	*chars;
{
  if (pDraw->type == DRAWABLE_WINDOW) 
    hpText (pDraw, pGC, x,y, count, chars, Linear8Bit);
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyText16 --
 *-----------------------------------------------------------------------
 */
void hpPolyText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    unsigned short *chars;
{
  if (pDraw->type == DRAWABLE_WINDOW)
    hpText (pDraw, pGC, x, y, count, chars,
	(pGC->font->pFI->lastRow == 0 ? Linear16Bit : TwoD16Bit));
}

/*-
 *-----------------------------------------------------------------------
 * sunImageText8 --
 *-----------------------------------------------------------------------
 */
void hpImageText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    char	*chars;
{
  if (pDraw->type ==DRAWABLE_WINDOW)
    hpText(pDraw, pGC, x, y, count, chars, Linear8Bit);
}

/*-
 *-----------------------------------------------------------------------
 * sunImageText16 --
 *-----------------------------------------------------------------------
 */
void hpImageText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    unsigned short *chars;
{
  if (pDraw->type == DRAWABLE_WINDOW) 
    hpText (pDraw, pGC, x, y, count, chars,
	(pGC->font->pFI->lastRow == 0 ? Linear16Bit : TwoD16Bit));
}

/*-
 *-----------------------------------------------------------------------
 * sunImageGlyphBlt --
 *-----------------------------------------------------------------------
 */
void hpImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
  BoxRec  	  	cursorBox;
  ExtentInfoRec 	extents;
  register int  	xorg, yorg;

  if (pDrawable->type == DRAWABLE_WINDOW &&
      hpCursorLoc (pDrawable->pScreen, &cursorBox))
  {
    QueryGlyphExtents (pGC->font, ppci, nglyph, &extents);
    xorg = ((WindowPtr)pDrawable)->absCorner.x + x;
    yorg = ((WindowPtr)pDrawable)->absCorner.y + y;
    if (BOX_OVERLAP(&cursorBox,xorg+extents.overallLeft,
		    yorg+extents.overallAscent,
		    xorg+extents.overallRight,
		    yorg+extents.overallDescent)) RemoveCursor(pDrawable);
  }
}

/*-
 *-----------------------------------------------------------------------
 * sunPolyGlyphBlt --
 *-----------------------------------------------------------------------
 */
void hpPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
    BoxRec  	  	cursorBox;
    ExtentInfoRec 	extents;
    register int  	xorg, yorg;

    if (pDrawable->type == DRAWABLE_WINDOW &&
	hpCursorLoc (pDrawable->pScreen, &cursorBox)) {
	    QueryGlyphExtents (pGC->font, ppci, nglyph, &extents);
	    xorg = ((WindowPtr)pDrawable)->absCorner.x + x;
	    yorg = ((WindowPtr)pDrawable)->absCorner.y + y;
	    if (BOX_OVERLAP(&cursorBox,xorg+extents.overallLeft,
			    yorg+extents.overallAscent,
			    xorg+extents.overallRight,
			    yorg+extents.overallDescent)){
				RemoveCursor(pDrawable);
	    }
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunPushPixels --
 *-----------------------------------------------------------------------
 */
void hpPushPixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
  BoxRec  	  	cursorBox;
  register int  	xorg,yorg;

  if (pDst->type==DRAWABLE_WINDOW && hpCursorLoc(pDst->pScreen, &cursorBox))
  {
    xorg = ((WindowPtr)pDst)->absCorner.x + x;
    yorg = ((WindowPtr)pDst)->absCorner.y + y;

    if (BOX_OVERLAP(&cursorBox,xorg,yorg,xorg+w,yorg+h)) RemoveCursor(pDst);
  }
}

#include "salloc.h"

/*-
 *-----------------------------------------------------------------------
 * sunText --
 *	Find the extent of a text operation and remove the cursor if they
 *	overlap. pDraw is assumed to be a window.
 *-----------------------------------------------------------------------
 */
static void hpText(pDraw, pGC, x,y, count, chars, fontEncoding)
    DrawablePtr   pDraw;
    GCPtr	  pGC;
    int		  x, y, count;
    char 	  *chars;
    FontEncoding  fontEncoding;
{
  CharInfoPtr *charinfo;
  unsigned int n, w;
  register int xorg=0, yorg=0;
  ExtentInfoRec extents;
  BoxRec      cursorBox;

#if SAOK
  SALLOC(count * sizeof(CharInfoPtr)); charinfo = (CharInfoPtr *)SADDR;
#else
  charinfo = (CharInfoPtr *) ALLOCATE_LOCAL(count * sizeof(CharInfoPtr));
  if (charinfo == (CharInfoPtr *) NULL) { RemoveCursor(pDraw); return; }
#endif
  GetGlyphs(pGC->font, count, chars, fontEncoding, &n, charinfo);

  if (n != 0)
  {
    QueryGlyphExtents(pGC->font, charinfo, n, &extents);
    w = extents.overallWidth;

    if (hpCursorLoc(pDraw->pScreen, &cursorBox))
    {
      if(pGC->miTranslate) 
      {
        xorg = ((WindowPtr) pDraw)->absCorner.x;
        yorg = ((WindowPtr) pDraw)->absCorner.y;
      }

      if (BOX_OVERLAP(&cursorBox,
	x + xorg + extents.overallLeft,  y + yorg - extents.overallAscent,
	x + xorg + extents.overallRight, y + yorg + extents.overallDescent))
	    RemoveCursor(pDraw);
    }
  }
#if !SAOK
  DEALLOCATE_LOCAL(charinfo);
#endif
}
