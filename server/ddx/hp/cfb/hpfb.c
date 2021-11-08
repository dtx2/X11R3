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
/* $XConsortium: hpfb.c,v 1.7 88/09/06 15:18:15 jim Exp $ */
/* Author: Todd Newman  (aided and abetted by Mr. Drewry) */

#include "X.h"
#include "Xprotostr.h"

#include "misc.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "regionstr.h"
#include "Xmd.h"
#include "../mfb/maskbits.h"
#include "servermd.h"
#include "cfb.h"

/*static*/
void UFFRHelper(pGC, pTile, prectIn, offset)
    GCPtr	pGC;
    PixmapPtr   pTile;	/* pointer to tile we want to fill with */
    xRectangle  *prectIn; 
    DDXPointPtr	offset;

{
    int         yT, yB, xL, xR, xLWidth, yTHeight,
                fullX, fullXLimit, fullY, fullYLimit,
                xCoord, yCoord;
    register 	rop;
    register xRectangle *prect = prectIn; 
    register hpChunk *pTileChunk; /* descriptive info for tiles bits */
    void         (*bitMover)();
    ScreenPtr pScreen = pGC->pScreen;
    int planemask = pGC->planemask;
    int xRemaining;
    int yRemaining;
    
    pTileChunk = (hpChunk *)
	(((cfbPrivPixmapPtr) (pTile->devPrivate))->pChunk);
    rop = pGC->alu;
    bitMover = ((cfbPrivScreenPtr)
		(pScreen->devPrivate))->MoveBits;

    /* offset into tile on left edge */
    xL = (prect->x - offset->x) % pTile->width;
    if (xL<0) xL += pTile->width;
    /* number of pixels used on left */
    xLWidth = (xL) ? (pTile->width - xL) : 0; 

    if (xLWidth >= prect->width) { /* one tile covers full x range */
	xLWidth = prect->width;
	xR = 0;
    }
    else { /* see if there's a partial tile on right edge */
        xR = (prect->x + prect->width - offset->x) % pTile->width;
        if (xR<0) xR += pTile->width;
    }
    
    /* y offset in tile at top */
    yT = (prect->y - offset->y) % pTile->height; 
    if (yT<0) yT += pTile->height; 
    /* number of pixels used on top */
    yTHeight = (yT) ? (pTile->height - yT) : 0;
    if (yTHeight >= prect->height) { /* tile covers full y range */
	yTHeight = prect->height;
	yB = 0;
    }
    else { /* portion of tile used on bottom */
        yB = (prect->y + prect->height - offset->y) % pTile->height; 
        if (yB<0) yB += pTile->height; 
    }

    fullX = (prect->width - xLWidth) / pTile->width; /* full tiles across */
    fullY = (prect->height - yTHeight) / pTile->height; /* full vertical tiles */
    fullXLimit = prect->x + xLWidth + fullX * pTile->width; 
    fullYLimit = prect->y + yTHeight + fullY * pTile->height;

    if(rop != GXcopy) {
      /* Now the actual work...
         First, fill partial horizontal band across top of rectangle */
      if (yT) {
        if(xL) { /* fill the top left corner if it's a partial tile */
          (*bitMover)(pScreen, planemask, rop,
	 	      pTileChunk->x + xL, pTileChunk->y + yT,
		      prect->x, prect->y,
		      xLWidth, yTHeight);
        }
	/* fill full width words in middle */
	for (xCoord = prect->x + xLWidth;
	     xCoord < fullXLimit;
	     xCoord += pTile->width)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x, pTileChunk->y + yT,
			xCoord, prect->y,
			pTile->width, yTHeight);
	/* finally, partial tile on right edge. */
	if (xR)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x, pTileChunk->y + yT,
			xCoord, prect->y,
			xR, yTHeight);
      }
      /* fill full tile height bands in middle */
      for (yCoord = prect->y + yTHeight;
	 yCoord < fullYLimit;
	 yCoord += pTile->height) {
	/* partial vertical band on left edge of rectangle */
	if (xL)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x + xL, pTileChunk->y,
			prect->x, yCoord,
			xLWidth, pTile->height);
	/* now fill full width words in middle */
	for (xCoord = prect->x + xLWidth;
	     xCoord < fullXLimit;
	     xCoord += pTile->width)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x, pTileChunk->y,
			xCoord, yCoord,
			pTile->width, pTile->height);
	/* finally, partial tile on right edge. */
	if (xR)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x, pTileChunk->y,
			xCoord, yCoord,
			xR, pTile->height);
        }
        /* finally, partial height band across the bottom */
        if (yB) {
	/* partial vertical band on left edge of rectangle */
	if (xL)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x + xL, pTileChunk->y,
			prect->x, yCoord,
			xLWidth, yB);
	/* now fill full width words in middle */
	for (xCoord = prect->x + xLWidth;
	     xCoord < fullXLimit;
	     xCoord += pTile->width)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x, pTileChunk->y,
			xCoord, yCoord,
			pTile->width, yB);
	/* finally, partial tile on right edge. */
	if (xR)
	    (*bitMover)(pScreen, planemask, rop,
			pTileChunk->x, pTileChunk->y,
			xCoord, yCoord,
			xR, yB);
      }
    }
    else { /* gxcopy so we can use on-screen bits as the source */
      /*
       * We know that we have at least one whole tile to put out
       * in either the X or Y direction.
       *
       * x,y of repeat cell. Start with the cell for horiz. repeat.
       * These same vars will be used for the vertical repeat also
       */
      int cellx = prect->x + xLWidth; /* x of horiz. repeat tile */
      int celly = prect->y + yTHeight;/* y of vert. repeat tile */
      int cellWidth = pTile->width;
      int cellHeight = pTile->height;

      xCoord = prect->x + xLWidth; 
      yCoord = prect->y + yTHeight;
      xRemaining = prect->width;
      yRemaining = prect->height;

      if(xL && yT) {
	/*
	 * fill the whole corner with pieces of the tile
	 */

        (*bitMover)(pScreen, planemask, rop,
	 	    pTileChunk->x + xL, pTileChunk->y + yT,
		    prect->x, prect->y,
		    xLWidth, yTHeight);
	xRemaining -= xLWidth;
	yRemaining -= yTHeight;

        if(!xRemaining && !yRemaining) return; /* if one piece was enough */

	if(xRemaining < cellWidth) cellWidth = xRemaining;
	if(yRemaining < cellHeight) cellHeight = yRemaining;

        /* fill left edge */
        if(yRemaining) {
	  (*bitMover)(pScreen, planemask, rop,
		      pTileChunk->x + xL, pTileChunk->y,
		      prect->x, prect->y + yTHeight,
		      xLWidth, cellHeight);
	}

	/* fill top edge */
	if (xRemaining) {
	  (*bitMover)(pScreen, planemask, rop,
		      pTileChunk->x, pTileChunk->y + yT,
		      prect->x + xLWidth, prect->y,
		      cellWidth, yTHeight);
        }

	if(xRemaining && yRemaining) {
          (*bitMover)(pScreen, planemask, rop,
	              pTileChunk->x, pTileChunk->y,
	              prect->x + xLWidth, prect->y + yTHeight,
		      cellWidth, cellHeight);
	}
	xRemaining -= cellWidth;
	yRemaining -= cellHeight;
        if(!xRemaining && !yRemaining) return;
	cellHeight += yTHeight;
      }
      else if(xL) {
        /* fill left edge */
	if(yRemaining < cellHeight) cellHeight = yRemaining;
	(*bitMover)(pScreen, planemask, rop,
		    pTileChunk->x + xL, pTileChunk->y,
		    prect->x, prect->y,
		    xLWidth, cellHeight);
	xRemaining -= xLWidth;
	yRemaining -= cellHeight;

	if(xRemaining) {
	  if(xRemaining < cellWidth) cellWidth = xRemaining;
	  (*bitMover)(pScreen, planemask, rop,
		      pTileChunk->x, pTileChunk->y,
		      prect->x + xLWidth, prect->y,
		      cellWidth, cellHeight);
	  xRemaining -= cellWidth;
        }
      }
      else if(yT) {
	/* fill top edge */
	if(xRemaining < cellWidth) cellWidth = xRemaining;
	(*bitMover)(pScreen, planemask, rop,
		    pTileChunk->x, pTileChunk->y + yT,
		    prect->x, prect->y,
		    cellWidth, yTHeight);
	yRemaining -= yTHeight;
	xRemaining -= cellWidth;

        if(yRemaining) {
	  if(yRemaining < cellHeight) cellHeight = yRemaining;
          (*bitMover)(pScreen, planemask, rop,
	              pTileChunk->x, pTileChunk->y,
	              prect->x, prect->y + yTHeight,
		      cellWidth, cellHeight);
	  yRemaining -= cellHeight;
	  cellHeight += yTHeight;
	}
	else {
	  cellHeight = yTHeight;
	}
      }
      else { /* tile is aligned w/ upper left corner */
	if(xRemaining < cellWidth) cellWidth = xRemaining;
	if(yRemaining < cellHeight) cellHeight = yRemaining;
	(*bitMover)(pScreen, planemask, rop,
		    pTileChunk->x, pTileChunk->y,
		    prect->x, prect->y,
		    cellWidth, cellHeight);
	xRemaining -= cellWidth;
	yRemaining -= cellHeight;
      }

      /*
       * Now all the funky corner stuff is filled in.  We can go on to
       * replicate the horizontal repeat cell across the screen
       */

      /* fill the rest of the first row */
      xCoord = prect->x + xLWidth + cellWidth;
      cellx = prect->x + xLWidth;
      while (xRemaining) {
        if (cellWidth > xRemaining) cellWidth = xRemaining;
        (*bitMover)(pScreen, planemask, rop,
		    cellx, prect->y,
		    xCoord, prect->y,
		    cellWidth, cellHeight);
        xRemaining -= cellWidth;
        xCoord += cellWidth;
        cellWidth += cellWidth;
      }

      /* fill the rest of the vertical space */
      cellx = prect->x;
      celly = prect->y + yTHeight;

      yCoord = prect->y + cellHeight;
      cellHeight -= yTHeight;

      while(yRemaining) {
        if(cellHeight > yRemaining) cellHeight = yRemaining;
        (*bitMover)(pScreen, planemask, rop,
		 cellx, celly,
		 prect->x, yCoord,
		 prect->width, cellHeight);
        yRemaining -= cellHeight;
        yCoord += cellHeight;
        cellHeight += cellHeight;
      }
    }
}

#define PIXER(Drawable)  ((cfbPrivPixmapPtr)((PixmapPtr)Drawable)->devPrivate)

/* Fill rectangle with tiles that aren't 32 bits wide */
static void
hpfbUnnaturalFastFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */

{
  register    xRectangle *prect; 
  xRectangle  realRect;
  DDXPointRec	offset;
  RegionPtr cliplist = (RegionPtr)((cfbPrivGC *)pGC->devPriv)->pCompositeClip;
  register    BoxRec     *pClipBox;
  PixmapPtr   pTile;	/* pointer to tile we want to fill with */
  register int j, wtx = 0, wty = 0, ptx = 0, pty = 0;
  int x,y,w,h;


#if 0	/* we already did this test in hpfbPolyFillRect() */
  switch (pDrawable->depth)
  {
    case 8: break;
    default: FatalError("hpfbUnnaturalFastFillRect: invalid depth\n");
  }
#endif

  if (!(pGC->planemask)) return;

  pTile = (pGC->fillStyle == FillTiled) ? pGC->tile : pGC->stipple;

  if (pDrawable->type==DRAWABLE_PIXMAP)	/* offscreen memory pixmap */
  {
    ptx = ((hpChunk *)PIXER(pDrawable)->pChunk)->x;
    pty = ((hpChunk *)PIXER(pDrawable)->pChunk)->y;
  }
  else		/* window */
  {
    wtx = ((WindowPtr)pDrawable)->absCorner.x;
    wty = ((WindowPtr)pDrawable)->absCorner.y;
  }

  prect = prectInit;
  for (; nrectFill--; prect++)	/* for each rectangle */
  {
    pClipBox = cliplist->rects;
    for (j = cliplist->numRects; j--; pClipBox++) /* and each clip rectangle */
    {
      clipper(
	    prect->x +wtx, prect->y +wty, prect->width,prect->height,
	    pClipBox->x1, pClipBox->y1, pClipBox->x2, pClipBox->y2,
	    &x,&y, &w,&h);
      if (w>0 && h>0)
      {
	realRect.x = x +ptx; realRect.y = y +pty;
	realRect.width = w; realRect.height = h;
	offset.x = wtx + ptx + pGC->patOrg.x;
	offset.y = wty + pty + pGC->patOrg.y;
	UFFRHelper(pGC, pTile, &realRect, &offset);	/* draw it */
      }
    }
  }
}

/* HPFBPOLYFILLRECT -- public entry for PolyFillRect request
 * very straight forward: translate rectangles if necessary
 * then call FastFillRect to fill each rectangle.  We let
 * FastFillRect worry about clipping to the destination.
 */
void
hpfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{

  switch (pDrawable->depth) {
	case 1:
	    miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
	    return;
	case 8: break;
	default:
	    FatalError("hpfbPolyFillRect: invalid depth\n");
    }

  if ((pDrawable->type==DRAWABLE_WINDOW ||
       ((PixmapPtr)pDrawable)->devKind ==PIXMAP_FRAME_BUFFER) &&
      ((PixmapPtr)pGC->tile)->devKind ==PIXMAP_FRAME_BUFFER )
    hpfbUnnaturalFastFillRect(pDrawable, pGC, nrectFill, prectInit);
  else		/* we got called for a main-memory pixmap or tile */
    miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
}


#include "salloc.h"

/* HPFBCOPYAREA -- public entry for the CopyArea request 
 * For requests operating within a single topcat frame buffer.
 * For each rectangle in the source region
 *   move rectangle using topcat pixel mover hardware
 */
RegionPtr
hpfbCopyArea(pSrcDrawable, pDstDrawable,
	       pGC, xIn, yIn, widthSrc, heightSrc, xOut, yOut)
    register DrawablePtr 	pSrcDrawable;
    register DrawablePtr 	pDstDrawable;
    GCPtr 			pGC;
    int 			xIn, yIn;
    int 			widthSrc, heightSrc;
    int 			xOut, yOut;
{
    DDXPointPtr		ppt, pptFirst;
    unsigned int	*pwidthFirst, *pwidth, *pbits;
    BoxRec 		srcBox, *prect;
    			/* may be a new region, or just a copy */
    RegionPtr 		prgnSrcClip, prgnDstClip;
    			/* non-0 if we've created a src clip */
    int 		realSrcClip = 0,
                        useOrdering = 0;
    int			srcx, srcy, dstx, dsty, i, j, y, width, height,
    			xMin, xMax, yMin, yMax;
    unsigned int        *ordering;
    RegionPtr		prgnExposed;

    /* clip the left and top edges of the source */
    if (xIn < 0)
    {
        widthSrc += xIn;
        srcx = 0;
    }
    else
	srcx = xIn;
    if (yIn < 0)
    {
        heightSrc += yIn;
        yIn = 0;
    }
    else
	srcy = yIn;

    /* clip the source */

    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	BoxRec box;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = ((PixmapPtr)pSrcDrawable)->width;
	box.y2 = ((PixmapPtr)pSrcDrawable)->height;

	prgnSrcClip = (*pGC->pScreen->RegionCreate)(&box, 1);
	realSrcClip = 1;
    }
    else
    {
        if ((pDstDrawable->type == DRAWABLE_WINDOW) && 
	    (!((WindowPtr)pDstDrawable)->realized))
	{
	    return NULL;
	}
	srcx += ((WindowPtr)pSrcDrawable)->absCorner.x;
	srcy += ((WindowPtr)pSrcDrawable)->absCorner.y;
	prgnSrcClip = ((WindowPtr)pSrcDrawable)->clipList;
    }

    srcBox.x1 = srcx;
    srcBox.y1 = srcy;
    srcBox.x2 = srcx + widthSrc;
    srcBox.y2 = srcy + heightSrc;

    dstx = xOut;
    dsty = yOut;
    if (pDstDrawable->type == DRAWABLE_WINDOW) {
	if (pGC->miTranslate) {
	    dstx += ((WindowPtr)pDstDrawable)->absCorner.x;
	    dsty += ((WindowPtr)pDstDrawable)->absCorner.y;
	}
    }

    prgnDstClip = ((cfbPrivGC *) (pGC->devPriv))->pCompositeClip;

#if SAOK
    SALLOC(prgnSrcClip->numRects*sizeof(unsigned int));
    ordering = (unsigned int *)SADDR;
#else
    ordering = (unsigned int *)
        ALLOCATE_LOCAL(prgnSrcClip->numRects * sizeof(unsigned int));
#endif
    if(!ordering) return;

    /* If not the same drawable then order of move doesn't matter.
       Following assumes that prgnSrcClip->rects are sorted from top
       to bottom and left to right.
    */
    if (pSrcDrawable != pDstDrawable)
      for (i=0; i < prgnSrcClip->numRects; i++)
	ordering[i] = i;
    else { /* within same drawable, must sequence moves carefully! */
      useOrdering = 1; /* must pay attention to this ordering later! */
      if (dsty <= srcBox.y1) { /* Scroll up or stationary vertical.
				  Vertical order OK */
	if (dstx <= srcBox.x1) /* Scroll left or stationary horizontal.
				  Horizontal order OK as well */
	  for (i=0; i < prgnSrcClip->numRects; i++)
	    ordering[i] = i;
        else { /* scroll right. must reverse horizontal banding of rects. */
	  for (i=0, j=1, xMax=0;
	       i < prgnSrcClip->numRects;
	       j=i+1, xMax=i) {
	    /* find extent of current horizontal band */
	    y=prgnSrcClip->rects[i].y1; /* band has this y coordinate */
	    while ((j < prgnSrcClip->numRects) &&
		   (prgnSrcClip->rects[j].y1 == y))
	      j++;
	    /* reverse the horizontal band in the output ordering */
	    for (j-- ; j >= xMax; j--, i++)
	      ordering[i] = j;
          }
        }
      }
      else { /* Scroll down. Must reverse vertical banding. */
	if (dstx < srcBox.x1) { /* Scroll left. Horizontal order OK. */
	  for (i=prgnSrcClip->numRects-1, j=i-1, yMin=i, yMax=0;
	       i >= 0;
	       j=i-1, yMin=i) {
	    /* find extent of current horizontal band */
	    y=prgnSrcClip->rects[i].y1; /* band has this y coordinate */
	    while ((j >= 0) &&
		   (prgnSrcClip->rects[j].y1 == y))
	      j--;
	    /* reverse the horizontal band in the output ordering */
	    for (j++ ; j <= yMin; j++, i--, yMax++)
	      ordering[yMax] = j;
	  }
	}
	else /* Scroll right or horizontal stationary.
		Reverse horizontal order as well (if stationary, horizontal
		order can be swapped without penalty and this is faster
		to compute). */
	  for (i=0, j=prgnSrcClip->numRects-1;
	       i < prgnSrcClip->numRects;
	       i++, j--)
	      ordering[i] = j;
      }
    }

    if ((pSrcDrawable->pScreen == pDstDrawable->pScreen) &&
	
	(((pSrcDrawable->type == DRAWABLE_PIXMAP) &&
	  (((PixmapPtr)pSrcDrawable)->devKind == PIXMAP_FRAME_BUFFER)) ||
	 (pSrcDrawable->type == DRAWABLE_WINDOW)) &&
	
	(((pDstDrawable->type == DRAWABLE_PIXMAP) &&
	  (((PixmapPtr)pDstDrawable)->devKind == PIXMAP_FRAME_BUFFER)) ||
	 (pDstDrawable->type == DRAWABLE_WINDOW))
	) {
	
	/* Copy area within portions of a single screens frame buffer.
	 * For each visible portion of source, move into visible
	 * portions of destination utilizing area mover.
	 */
	BoxRec  dstBox,  *prect2;
	int     sxMin, sxMax, syMin, syMax,  /* source for actual move */
	dxMin, dxMax, dyMin, dyMax;  /* dest for actual move */
	
	register cfbPrivScreenPtr pPrivScreen;
	void (*bitMover)(), (*maskConfig)();

	pPrivScreen = (cfbPrivScreenPtr)(pGC->pScreen->devPrivate);
	bitMover = pPrivScreen->MoveBits;
	maskConfig = pPrivScreen->MaskConfig;

	if (pSrcDrawable->type == DRAWABLE_PIXMAP) { /* make screen relative */
	    register hpChunk *pixChunk = (hpChunk *)
		((cfbPrivPixmapPtr) (((PixmapPtr)pSrcDrawable)->devPrivate))->pChunk;
	    prgnSrcClip->rects[0].x1 += pixChunk->x;
	    prgnSrcClip->rects[0].y1 += pixChunk->y;
	    prgnSrcClip->rects[0].x2 += pixChunk->x;
	    prgnSrcClip->rects[0].y2 += pixChunk->y;
	    
	    srcBox.x1 += pixChunk->x;
	    srcBox.y1 += pixChunk->y;
	    srcBox.x2 += pixChunk->x;
	    srcBox.y2 += pixChunk->y;
	    
	    srcx += pixChunk->x;
	    srcy += pixChunk->y;
	}
	
	if (pDstDrawable->type == DRAWABLE_PIXMAP) { /* make screen relative */
	    register hpChunk *pixChunk = (hpChunk *)
		((cfbPrivPixmapPtr) (((PixmapPtr)pDstDrawable)->devPrivate))->pChunk;
	    
	    prgnDstClip->rects[0].x1 += pixChunk->x;
	    prgnDstClip->rects[0].y1 += pixChunk->y;
	    prgnDstClip->rects[0].x2 += pixChunk->x;
	    prgnDstClip->rects[0].y2 += pixChunk->y;
	    
	    dstx += pixChunk->x;
	    dsty += pixChunk->y;
	}
	
	for(i = 0;
	    i < prgnSrcClip->numRects;
	    i++)
	    {
		prect = &prgnSrcClip->rects[ordering[i]];
		/* find portion of move contained in this visible portion of window */
		xMin = max(prect->x1, srcBox.x1);
		xMax = min(prect->x2, srcBox.x2);
		yMin = max(prect->y1, srcBox.y1);
		yMax = min(prect->y2, srcBox.y2);
		/* exit loop unless there is something visible */
		if(xMax <= xMin || yMax <= yMin)
		    continue;
		
		/* destination box for visible portion of source */
		dstBox.x1 = xMin - (srcx - dstx);
		dstBox.y1 = yMin - (srcy - dsty);
		dstBox.x2 = dstBox.x1 + xMax - xMin;
		dstBox.y2 = dstBox.y1 + yMax - yMin;
		
		/* find visible portions of destination */
		prect2 = prgnDstClip->rects;
		for(j = 0;
		    j < prgnDstClip->numRects;
		    j++) {
		    if (useOrdering)
			prect2 = &prgnDstClip->rects[ordering[j]];
		    else
			prect2 = &prgnDstClip->rects[j];
		    dxMin = max(prect2->x1, dstBox.x1);
		    dxMax = min(prect2->x2, dstBox.x2);
		    dyMin = max(prect2->y1, dstBox.y1);
		    dyMax = min(prect2->y2, dstBox.y2);
		    /* any portion of destination visible in this area? */
		    if(dxMax <= dxMin || dyMax <= dyMin)
			continue;
		    
		    /* will further clip source if destination was also clipped */
		    sxMin = xMin + max((prect2->x1 - dstBox.x1), 0);
		    syMin = yMin + max((prect2->y1 - dstBox.y1), 0);
		    sxMax = xMax + min((prect2->x2 - dstBox.x2), 0);
		    syMax = yMax + min((prect2->y2 - dstBox.y2), 0);
		    
		    (*bitMover)(pGC->pScreen, pGC->planemask, pGC->alu,
				sxMin, syMin, dxMin, dyMin,
				(sxMax - sxMin), (syMax - syMin));
		}
	    }
    }
    else {  /* no place for hardware assist */
#if SAOK
      SALLOC(heightSrc * sizeof(DDXPointRec));
      pptFirst = ppt = (DDXPointPtr)SADDR;
      SALLOC(heightSrc * sizeof(unsigned int));
      pwidthFirst = pwidth = (unsigned int *)SADDR;
#else
      pptFirst = ppt = (DDXPointPtr)
        ALLOCATE_LOCAL(heightSrc * sizeof(DDXPointRec));
      pwidthFirst = pwidth = (unsigned int *)
        ALLOCATE_LOCAL(heightSrc * sizeof(unsigned int));
      if(!pptFirst || !pwidthFirst)
	{
	  if (pptFirst)
	    DEALLOCATE_LOCAL(pptFirst);
	  if (pwidthFirst)
	    DEALLOCATE_LOCAL(pwidthFirst);
	  return;
	}
#endif
      for(i = 0;
	  i < prgnSrcClip->numRects;
	  i++)
	{
	  prect = &prgnSrcClip->rects[ordering[i]];
	  xMin = max(prect->x1, srcBox.x1);
	  xMax = min(prect->x2, srcBox.x2);
	  yMin = max(prect->y1, srcBox.y1);
	  yMax = min(prect->y2, srcBox.y2);
	  /* is there anything visible here? */
	  if(xMax <= xMin || yMax <= yMin)
	    continue;
	  
	  ppt = pptFirst;
	  pwidth = pwidthFirst;
	  y = yMin;
	  height = yMax - yMin;
	  width = xMax - xMin;
	  
	  for(j = 0; j < height; j++)
	    {
	      ppt->x = xMin;
	      ppt++->y = y++;
	      *pwidth++ = width;
	    }
	  pbits = (*pSrcDrawable->pScreen->GetSpans)(pSrcDrawable, width, 
						     pptFirst, pwidthFirst, height);
	  ppt = pptFirst;
	  pwidth = pwidthFirst;
	  xMin -= (srcx - dstx);
	  y = yMin - (srcy - dsty);
	  for(j = 0; j < height; j++)
	    {
	      ppt->x = xMin;
	      ppt++->y = y++;
	      *pwidth++ = width;
	    }
	  
	  (*pGC->SetSpans)(pDstDrawable, pGC, pbits, pptFirst, pwidthFirst,
			   height, TRUE);
	  Xfree(pbits);
	}
#if !SAOK
      DEALLOCATE_LOCAL(pptFirst);
      DEALLOCATE_LOCAL(pwidthFirst);
#endif
    }
#if !SAOK    
    DEALLOCATE_LOCAL(ordering);
#endif
    prgnExposed = miHandleExposures(pSrcDrawable, pDstDrawable, pGC, xIn, yIn,
		      widthSrc, heightSrc, xOut, yOut, 0);
    if(realSrcClip)
      (*pGC->pScreen->RegionDestroy)(prgnSrcClip);
    return prgnExposed;
}


/* DoBitblt() does multiple rectangle moves into the rectangles

   we have to cope with the direction on a per band basis,
rather than a per rectangle basis.  moving bottom to top
means we have to invert the order of the bands; moving right
to left requires reversing the order of the rectangles in
each band.

   if src or dst is a window, the points have already been
translated.
*/

hpfbDoBitblt(pSrcDrawable, pDstDrawable, alu, prgnDst, pptSrc)
DrawablePtr pSrcDrawable;
DrawablePtr pDstDrawable;
int alu;
RegionPtr prgnDst;
DDXPointPtr pptSrc;
{
    register BoxPtr pbox;
    int nbox;

    BoxPtr pboxTmp, pboxNext, pboxBase, pboxNewY, pboxNewX;
    /* temporaries for shuffling rectangles */
    DDXPointPtr pptTmp, pptNewY, pptNewX; /* shuffling boxes entails shuffling the
					   source points too */
    int w, h;

    void (*bitMover)();

    /* check whether we should use this routine... call cfb code if not. */

    if (((pSrcDrawable->type == DRAWABLE_PIXMAP) &&
	 (((PixmapPtr)(pSrcDrawable))->devKind != PIXMAP_FRAME_BUFFER)) &&
	(pSrcDrawable->type != DRAWABLE_WINDOW)) {
	cfbDoBitblt(pSrcDrawable, pDstDrawable, alu, prgnDst, pptSrc);
	return;
    }

    if (((pDstDrawable->type == DRAWABLE_PIXMAP) &&
	 (((PixmapPtr)(pDstDrawable))->devKind != PIXMAP_FRAME_BUFFER)) &&
	(pDstDrawable->type != DRAWABLE_WINDOW)) {
	cfbDoBitblt(pSrcDrawable, pDstDrawable, alu, prgnDst, pptSrc);
	return;
    }

    if (pSrcDrawable->pScreen != pDstDrawable->pScreen) {
	cfbDoBitblt(pSrcDrawable, pDstDrawable, alu, prgnDst, pptSrc);
	return;
    }

    pbox = prgnDst->rects;
    nbox = prgnDst->numRects;

    pboxNewY = 0;
    pptNewY = 0;
    pboxNewX = 0;
    pptNewX = 0;
    if (pptSrc->y < pbox->y1) 
    {
        /* walk source botttom to top */
	if (nbox > 1)
	{
	    /* keep ordering in each band, reverse order of bands */
#if SAOK
	    SALLOC(sizeof(BoxRec) * nbox); pboxNewY = (BoxPtr)SADDR;
	    SALLOC(sizeof(DDXPointRec) * nbox); pptNewY = (DDXPointPtr)SADDR;
#else
	    pboxNewY = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    pptNewY = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pboxNewY || !pptNewY)
	    {
	        DEALLOCATE_LOCAL(pptNewY);
	        DEALLOCATE_LOCAL(pboxNewY);
	        return;
	    }
#endif
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox)
	    {
	        while ((pboxNext >= pbox) && 
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
	        pboxTmp = pboxNext+1;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp <= pboxBase)
	        {
		    *pboxNewY++ = *pboxTmp++;
		    *pptNewY++ = *pptTmp++;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNewY -= nbox;
	    pbox = pboxNewY;
	    pptNewY -= nbox;
	    pptSrc = pptNewY;
        }
    }

    if (pptSrc->x < pbox->x1)
    {
	/* walk source right to left */

	if (nbox > 1)
	{
	    /* reverse order of rects in each band */
#if SAOK
	    SALLOC(sizeof(BoxRec) * nbox); pboxNewX = (BoxPtr)SADDR;
	    SALLOC(sizeof(DDXPointRec) * nbox); pptNewX = (DDXPointPtr)SADDR;
#else
	    pboxNewX = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    pptNewX = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pboxNewX || !pptNewX)
	    {
	        DEALLOCATE_LOCAL(pptNewX);
	        DEALLOCATE_LOCAL(pboxNewX);
	        return;
	    }
#endif
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox)
	    {
	        while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
	        pboxTmp = pboxNext;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp != pboxBase)
	        {
		    *pboxNewX++ = *--pboxTmp;
		    *pptNewX++ = *--pptTmp;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNewX -= nbox;
	    pbox = pboxNewX;
	    pptNewX -= nbox;
	    pptSrc = pptNewX;
	}
    }

    bitMover = ((cfbPrivScreenPtr)
		(pSrcDrawable->pScreen->devPrivate))->MoveBits;

    while (nbox--)
        {
	    w = pbox->x2 - pbox->x1;
	    h = pbox->y2 - pbox->y1;
	    
	    (*bitMover)(pSrcDrawable->pScreen, ~0, alu,
			pptSrc->x, pptSrc->y,
			pbox->x1, pbox->y1,
			w, h);

	    pbox++;
	    pptSrc++;
        } /* while (nbox--) */
#if !SAOK    
    if (pptNewY) DEALLOCATE_LOCAL(pptNewY);
    if (pboxNewY) DEALLOCATE_LOCAL(pboxNewY);
    if (pptNewX) DEALLOCATE_LOCAL(pptNewX);
    if (pboxNewX) DEALLOCATE_LOCAL(pboxNewX);
#endif
}
