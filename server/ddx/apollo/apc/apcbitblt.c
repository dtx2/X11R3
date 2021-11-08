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
#include "Xprotostr.h"

#include "misc.h"
#include "pixmapstr.h"
#include "Xmd.h"
#include "servermd.h"
    
RegionPtr miHandleExposures();

/* Sortrects -- sort list of clip rectangles, depending
 *      on blt direction. 
 */
void
Sortrects( order, pClip, dstx, dsty, Box) 
    unsigned int	*order;
    RegionPtr 		pClip;
    int			dstx, dsty;
    BoxRec		Box;
{
    int                 xMax, i, j, y, yMin, yMax;

    if (dsty <= Box.y1) { /* Scroll up or stationary vertical.
                                  Vertical order OK */
      if (dstx <= Box.x1) /* Scroll left or stationary horizontal.
                                  Horizontal order OK as well */
        for (i=0; i < pClip->numRects; i++)
          order[i] = i;
      else { /* scroll right. must reverse horizontal banding of rects. */
        for (i=0, j=1, xMax=0;
             i < pClip->numRects;
             j=i+1, xMax=i) {
          /* find extent of current horizontal band */
          y=pClip->rects[i].y1; /* band has this y coordinate */
          while ((j < pClip->numRects) &&
                 (pClip->rects[j].y1 == y))
            j++;
          /* reverse the horizontal band in the output ordering */
          for (j-- ; j >= xMax; j--, i++)
            order[i] = j;
        }
      }
    }
    else { /* Scroll down. Must reverse vertical banding. */
      if (dstx < Box.x1) { /* Scroll left. Horizontal order OK. */
        for (i=pClip->numRects-1, j=i-1, yMin=i, yMax=0;
             i >= 0;
             j=i-1, yMin=i) {
          /* find extent of current horizontal band */
          y=pClip->rects[i].y1; /* band has this y coordinate */
          while ((j >= 0) &&
                 (pClip->rects[j].y1 == y))
            j--;
          /* reverse the horizontal band in the output ordering */
          for (j++ ; j <= yMin; j++, i--, yMax++)
            order[yMax] = j;
        }
      }
      else /* Scroll right or horizontal stationary.
              Reverse horizontal order as well (if stationary, horizontal
              order can be swapped without penalty and this is faster
              to compute). */
        for (i=0, j=pClip->numRects-1;
             i < pClip->numRects;
             i++, j--)
            order[i] = j;
    }
}

/* APCCOPYAREA -- public entry for the CopyArea request 
 */
RegionPtr
apcCopyArea(pSrcDrawable, pDstDrawable,
	    pGC, xIn, yIn, widthSrc, heightSrc, xOut, yOut)
    DrawablePtr 		pSrcDrawable;
    DrawablePtr 		pDstDrawable;
    GCPtr 			pGC;
    int 			xIn, yIn;
    int 			widthSrc, heightSrc;
    int 			xOut, yOut;
{
    BoxRec 		srcBox, *prect;
    			/* may be a new region, or just a copy */
    RegionPtr 		prgnSrcClip, prgnDstClip;
    			/* non-0 if we've created a src clip */
    int 		realSrcClip = 0;
    int			srcx, srcy, dstx, dsty, i, j, width, height,
    			xMin, xMax, yMin, yMax;
    unsigned int	*srcordering, *dstordering;

    int			x1,x2,y1,y2;
    BoxPtr 	        pbox;
    gpr_$bitmap_desc_t  srcBitmap, dstBitmap;
    gpr_$window_t       src_win;
    gpr_$position_t     dest_origin;
    status_$t		status;

    gpr_$window_t window;
    boolean active;
    gpr_$mask_t mask;

    srcx = xIn;
    srcy = yIn;

    /* If the destination isn't realized, this is easy */
    if (pDstDrawable->type == DRAWABLE_WINDOW &&
	!((WindowPtr)pDstDrawable)->realized) {
	return NULL;
    }
    else {

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
	    srcBitmap = ((apcPrivPMPtr)(((PixmapPtr)pSrcDrawable)->devPrivate))->bitmap_desc;
        }
        else
        {
	    srcBitmap = apDisplayData[pSrcDrawable->pScreen->myNum].display_bitmap;
	    srcx += ((WindowPtr)pSrcDrawable)->absCorner.x;
	    srcy += ((WindowPtr)pSrcDrawable)->absCorner.y;
	    if (pGC->subWindowMode == IncludeInferiors)
	    {
	        prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
	        realSrcClip = 1;
	    }
	    else
	    {
	        prgnSrcClip = ((WindowPtr)pSrcDrawable)->clipList;
	    }
        }

        /* If the src drawable is a window, we need to translate the srcBox so
         * that we can compare it with the window's clip region later on. */
        srcBox.x1 = srcx;
        srcBox.y1 = srcy;
        srcBox.x2 = srcx  + widthSrc;
        srcBox.y2 = srcy  + heightSrc;

        if (pGC->miTranslate && (pDstDrawable->type == DRAWABLE_WINDOW) )
        {
	    dstx = xOut + ((WindowPtr)pDstDrawable)->absCorner.x;
            dsty = yOut + ((WindowPtr)pDstDrawable)->absCorner.y;
            dstBitmap = apDisplayData[pDstDrawable->pScreen->myNum].display_bitmap;
        }
        else
        {
            dstBitmap = ((apcPrivPMPtr)(((PixmapPtr)pDstDrawable)->devPrivate))->bitmap_desc;
	    dstx = xOut;
	    dsty = yOut;
        }
                                             
        apc_$set_bitmap( dstBitmap);

        srcordering = NULL; /* assume source is in order */
        dstordering = NULL; /* assume destination is in order */

        /* If not the same drawable then order of move doesn't matter.
           Following assumes that prgnSrcClip->rects are sorted from top
           to bottom and left to right.
        */
        prgnDstClip = ((apcPrivGC *)(pGC->devPriv))->pCompositeClip;
        if (pSrcDrawable == pDstDrawable) { 

            if (prgnSrcClip->rects != prgnDstClip->rects) {
	        /* different list of clip rects for source and dest.
	           Must sort both of these. Do dest here, and default
                   to always sorting source.*/

                if (prgnDstClip->numRects != 1) {
                   dstordering = (unsigned int *)
                      ALLOCATE_LOCAL(prgnDstClip->numRects * sizeof(unsigned int));
	           Sortrects( dstordering, prgnDstClip, dstx, dsty, srcBox);          
	           }
                }

            if (prgnSrcClip->numRects != 1) {
                srcordering = (unsigned int *)
                   ALLOCATE_LOCAL(prgnSrcClip->numRects * sizeof(unsigned int));
                Sortrects( srcordering, prgnSrcClip, dstx, dsty, srcBox);          
	    }
        }
     
        if (prgnDstClip->numRects != 1) gpr_$set_clipping_active(false, status);

        for(i = 0;
            i < prgnSrcClip->numRects;
            i++)
        {
            if (srcordering == NULL) prect = &prgnSrcClip->rects[i];
	    else prect = &prgnSrcClip->rects[srcordering[i]];

  	    x1 = max(prect->x1, srcBox.x1);
  	    x2 = min(prect->x2, srcBox.x2);
  	    y1 = max(prect->y1, srcBox.y1);
	    y2 = min(prect->y2, srcBox.y2);
	    /* is there anything visible here? */
	    if(!(x2 <= x1 && y2 <= y1)) {
	        for( j=0; j< prgnDstClip->numRects; j++) {
		    if ((dstordering == NULL) && (srcordering == NULL)) pbox = &prgnDstClip->rects[j];
		    else if (dstordering == NULL) pbox = &prgnDstClip->rects[srcordering[j]];
		    else pbox =  &prgnDstClip->rects[dstordering[j]];
	            xMin = max(x1-(srcx-dstx), pbox->x1);
	            xMax = min(x2-(srcx-dstx), pbox->x2);
	            yMin = max(y1-(srcy-dsty), pbox->y1);
	            yMax = min(y2-(srcy-dsty), pbox->y2);
	            /* is there anything visible here? */
	            if(xMax <= xMin || yMax <= yMin)
	               continue;
                                
                    src_win.window_base.x_coord = (xMin-(dstx-srcx));
                    src_win.window_base.y_coord = (yMin-(dsty-srcy));
                    src_win.window_size.x_size = (xMax-xMin);
                    src_win.window_size.y_size = (yMax-yMin);
                                    
		    dest_origin.x_coord = (xMin);
                    dest_origin.y_coord = (yMin);

                    gpr_$pixel_blt( srcBitmap, src_win, dest_origin, status);
	            }
                }
        }

        if (prgnDstClip->numRects != 1) gpr_$set_clipping_active(true, status);

        if (srcordering == NULL) DEALLOCATE_LOCAL(srcordering);
        if (dstordering == NULL) DEALLOCATE_LOCAL(dstordering);
        if(realSrcClip)
	    (*pGC->pScreen->RegionDestroy)(prgnSrcClip);
    }
    return miHandleExposures(pSrcDrawable, pDstDrawable, pGC, xIn, yIn,
                      widthSrc, heightSrc, xOut, yOut);
}
