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
#include "Xmd.h"
#include "misc.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"

#include "apcmskbits.h"
#include "servermd.h"

/* apcSetScanline -- copies the bits from psrc to the drawable starting at
 * (xStart, y) and continuing to (xEnd, y).  xOrigin tells us where psrc 
 * starts on the scanline. (I.e., if this scanline passes through multiple
 * boxes, we may not want to start grabbing bits at psrc but at some offset
 * further on.) 
 */
apcSetScanline(y, xOrigin, xStart, xEnd, psrct, alu, pdstBase, widthDst, depth, onebit)
    int                 y;
    int                 xOrigin;        /* where this scanline starts */
    int                 xStart;         /* first bit to use from scanline */
    int                 xEnd;           /* last bit to use from scanline + 1 */
    int                 *psrct;
    int                 alu;            /* raster op */
    int                 *pdstBase;      /* start of the drawable */
    int                 widthDst;       /* width of drawable in words */
    gpr_$rgb_plane_t    depth;          /* plane were working on */
    int                 onebit;
{
    int                 w;              /* width of scanline in bits */
    int                 *pdst;          /* where to put the bits */
    int                 tmpSrc;         /* scratch buffer to collect bits in */
    int                 dstBit;         /* offset in bits from beginning of 
                                         * word */
    int                 nstart;         /* number of bits from first partial */
    int                 nend;           /* " " last partial word */
    int                 startmask, endmask, nlMiddle, nl;
               
                              
    if ( onebit ) {
        int             offSrc;
        int             *psrc;

        pdst = pdstBase + (y * widthDst) + (xStart >> 5); 
        psrc = psrct + ((xStart - xOrigin) >> 5);
        offSrc = (xStart - xOrigin) & 0x1f;
        w = xEnd - xStart;
        dstBit = xStart & 0x1f;

        if (dstBit + w <= 32) { 
            getbits(psrc, offSrc, w, tmpSrc);
            putbitsrop(tmpSrc, dstBit, w, pdst, alu); 
            } 
        else { 

            maskbits(xStart, w, startmask, endmask, nlMiddle);
            if (startmask) 
                nstart = 32 - dstBit; 
            else 
                nstart = 0; 
            if (endmask) 
                nend = xEnd & 0x1f; 
            else 
                nend = 0; 
            if (startmask) { 
                getbits(psrc, offSrc, nstart, tmpSrc);
                putbitsrop(tmpSrc, dstBit, nstart, pdst, alu);
                pdst++; 
                offSrc += nstart;
                if (offSrc > 31) {
                    psrc++;
                    offSrc -= 32;
                    }
                } 
            nl = nlMiddle; 
            while (nl--) { 
                getbits(psrc, offSrc, 32, tmpSrc);
                *pdst = DoRop(alu, tmpSrc, *pdst); 
                pdst++; 
                psrc++; 
                } 
            if (endmask) { 
                getbits(psrc, offSrc, nend, tmpSrc);
                putbitsrop(tmpSrc, 0, nend, pdst, alu);
                } 
            } 
        }
    else {
        short               i;
        unsigned char       *psrc;
        unsigned char       bitsper;

        psrc = (unsigned char *)psrct;
        psrc += (xStart - xOrigin) >> 3;

        pdst = pdstBase + (y * widthDst) + (xStart >> 5); 
        w = xEnd - xStart;
        dstBit = xStart & 0x1f;

        if (dstBit + w <= 32) { 
            getbitsapc(depth, psrc, bitsper, w, tmpSrc);
            putbitsrop(tmpSrc, dstBit, w, pdst, alu); 
            } 
        else { 

            maskbits(xStart, w, startmask, endmask, nlMiddle);
            if (startmask) 
                nstart = 32 - dstBit; 
            else 
                nstart = 0; 
            if (endmask) 
                nend = xEnd & 0x1f; 
            else 
                nend = 0; 
            if (startmask) { 
                getbitsapc(depth, psrc, bitsper, nstart, tmpSrc);
                putbitsrop(tmpSrc, dstBit, nstart, pdst, alu);
                pdst++; 
                } 
            nl = nlMiddle; 
            while (nl--) { 
                getbitsapc(depth, psrc, bitsper, 32, tmpSrc);
                *pdst = DoRop(alu, tmpSrc, *pdst); 
                pdst++; 
                } 
            if (endmask) { 
               getbitsapc(depth, psrc, bitsper, nend, tmpSrc);
               putbitsrop(tmpSrc, 0, nend, pdst, alu);
               } 
            } 
        }
}



/* SetSpans -- for each span copy pwidth[i] bits from psrc to pDrawable at
 * ppt[i] using the raster op from the GC.  If fSorted is TRUE, the scanlines
 * are in increasing Y order.
 * Source bit lines are server scanline padded so that they always begin
 * on a word boundary.
 */ 
void
apcSetSpans(pDrawable, pGC, psrcsave, pptsave, pwidthsave, nspans, fSorted)
     DrawablePtr        pDrawable;
     GCPtr              pGC;
     int                *psrcsave;
     DDXPointPtr        pptsave;
     int                *pwidthsave;
     int                nspans;
     int                fSorted;
{
  int                 *pdstBase;       /* start of dst bitmap */
  int                 widthDst;        /* width of bitmap in words */
  
  DDXPointPtr         ppt, pptLast;    /* Next 7 parameters used to clip */
  BoxPtr              pbox, pboxLast, pboxTest;  
  int                 *pwidth;
  int                 yMax;
  
  RegionPtr           prgnDst;
  int                 xStart, xEnd;
  
  int		      *psrc;
  int                 spanoffset;
  int                 mainmemoffset;
  int                 *BaseBase;
  short               width;
  int                 onebit;
  gpr_$rgb_plane_t    depth;
  status_$t           status;
  
  depth = (gpr_$rgb_plane_t)pDrawable->depth;
  onebit = 0;
  if ( depth == 1 )
    onebit = 1;
  
  spanoffset = PixmapWidthInPadUnits(*pwidthsave, pDrawable->depth);
  
  pptLast = pptsave + nspans;
  pboxTest = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
  pboxLast = pboxTest + ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->numRects;
  
  if (pDrawable->type == DRAWABLE_WINDOW) {
    pdstBase = (int *)(apDisplayData[pDrawable->pScreen->myNum].bitmap_ptr);
    widthDst = (int )(apDisplayData[pDrawable->pScreen->myNum].words_per_line) >> 1;
    yMax = (int)((WindowPtr)pDrawable)->clientWinSize.height +
      ((WindowPtr)pDrawable)->absCorner.y;
  }
  else {
    BaseBase = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_ptr;
    widthDst = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->width >> 1;
    mainmemoffset = widthDst*((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->size.y_size;
    yMax = ((PixmapPtr)pDrawable)->height;
  }
  
  gpr_$enable_direct_access( status);
  while (depth--) {
    
    if (pDrawable->type == DRAWABLE_WINDOW) 
      gpr_$remap_color_memory(depth, status);
    else pdstBase = (int *)(mainmemoffset*depth+BaseBase);
    psrc = psrcsave;
    ppt = pptsave;
    pwidth = pwidthsave;
    pboxTest = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
    pbox = pboxTest;
    
    for (; ppt < pptLast; pwidth++, ppt++) {
      if(fSorted) {
        /* scan lines sorted in ascending order. Because they are sorted, we
         * don't have to check each scanline against each clip box.  We can be
         * sure that this scanline only has to be clipped to boxes at or after
         * the beginning of this y-band
         */
        if(ppt->y >= yMax)
          break;
        pbox = pboxTest;
      }
      if(*pwidth == 0)
        continue;
      while(pbox < pboxLast) {
        
        if(pbox->y1 > ppt->y) {
          /* scanline is before clip box */
          break;
        }
        else if(pbox->y2 <= ppt->y) {
          /* clip box is before scanline */
          pboxTest = ++pbox;
          continue;
        }
        else if(pbox->x1 > ppt->x + *pwidth) {
          /* clip box is to right of scanline */
          break;
        }   
        else if(pbox->x2 <= ppt->x) {
          /* scanline is to right of clip box */
          pbox++;
          continue;
        }
        
        /* at least some of the scanline is in the current clip box */
        xStart = max(pbox->x1, ppt->x);
        xEnd = min(ppt->x + *pwidth, pbox->x2);
        apcSetScanline(ppt->y, ppt->x, xStart, xEnd, psrc, pGC->alu,
                        pdstBase, widthDst, depth, onebit);
        if(ppt->x + *pwidth <= pbox->x2) {
          /* End of the line, as it were */
          break;
        }
        else
          pbox++;
      }
      /* We've tried this line against every box; it must be outside them
       * all.  move on to the next point */
      psrc += spanoffset;
    }
  }
}
