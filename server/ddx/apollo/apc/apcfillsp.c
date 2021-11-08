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
#include "window.h"
#include "pixmapstr.h"

#include "apcmskbits.h"
#include "servermd.h"
 
void
apcSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
     DrawablePtr           pDrawable;
     GCPtr                 pGC;
     int                   nInit;        /* number of spans to fill */
     DDXPointPtr           pptInit;      /* pointer to list of start points */
     int                   *pwidthInit;  /* pointer to list of n widths */
     int                   fSorted;
{
  DDXPointPtr           pptLast;         /* These 5 parameters used to clip */
  BoxPtr                pbox, pboxLast;
  BoxPtr                pboxTest;
  int                   yMax;
  
  int                   i;
  gpr_$bitmap_desc_t    bitmap_id, bitmap;
  gpr_$window_t         rect;
  gpr_$coordinate_t     xcoords[1000], ycoords[1000];
  status_$t             status;
  gpr_$window_t         win;
  boolean               clip_act;
  gpr_$mask_t           plane_mask;
  gpr_$pixel_value_t    draw_value;
  gpr_$raster_op_array_t   raster_ops;
  
  if (!(pGC->planemask))
    return;
  
  if (pDrawable->type == DRAWABLE_WINDOW) 
    bitmap_id = (apDisplayData[pDrawable->pScreen->myNum].display_bitmap);
  else 
    bitmap_id = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_desc;
  
  gpr_$inq_bitmap (bitmap, status);
  apc_$set_bitmap( bitmap_id);    
  if (((apcPrivGC *) (pGC->devPriv))->pCompositeClip->numRects != 1)
        gpr_$set_clipping_active( false, status);
  
  pptLast = pptInit + nInit;
  pboxTest = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
  pboxLast = pboxTest + ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->numRects;
  yMax = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->extents.y2;
  
  for (i=0; pptInit < pptLast; pwidthInit++, pptInit++) {
    if(fSorted) {
      if(pptInit->y >= yMax)
        break;
      pbox = pboxTest;
    }
    else {
      if(pptInit->y >= yMax)
        continue;
      pbox = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
    }
    if(*pwidthInit == 0)
      continue;
    while(pbox < pboxLast) {
      
      if(pbox->y1 > pptInit->y) {
        /* scanline is before clip box */
        break;
      }
      
      if(pbox->y2 <= pptInit->y) {
        /* clip box is before scanline */
        pboxTest = ++pbox;
        continue;
      }
      
      if(pbox->x1 >= pptInit->x + *pwidthInit) {
        /* clip box is to right of scanline */
        break;
      }
      
      if(pbox->x2 <= pptInit->x) {
        /* scanline is to right of clip box */
        pbox++;
        continue;
      }
      
      /* at least some of the scanline is in the current clip box */
      xcoords[i<<1] = (gpr_$coordinate_t) (max(pbox->x1, pptInit->x));
      xcoords[(i<<1)+1] = (gpr_$coordinate_t) (xcoords[i<<1] + ((min(pptInit->x + *pwidthInit, pbox->x2)) - xcoords[i<<1])-1);
      ycoords[(i<<1)] = (gpr_$coordinate_t)pptInit->y;
      ycoords[(i<<1)+1] = (gpr_$coordinate_t)pptInit->y;
      pbox++;
      i++;
      if (i == 500) {
        gpr_$multiline( xcoords, ycoords, (short)(i*2), status);
        i = 0;
	if (pptInit->x + *pwidthInit <= pbox->x2)
	  break;                /* We hit the end of the line */
      }
    }                           /* End of while loop */
  }                             /* End of for loop */
  if (i != 0)                   /* Process leftovers */
    gpr_$inq_constraints( win, clip_act, plane_mask, status);
    gpr_$inq_draw_value( draw_value, status);
    gpr_$inq_raster_ops( raster_ops, status);
    gpr_$multiline( xcoords, ycoords, (short)(i*2), status);
  
  if (((apcPrivGC *) (pGC->devPriv))->pCompositeClip->numRects != 1)
        gpr_$set_clipping_active( true, status);

}

/* Fill spans with tiles that aren't 32 bits wide */
void
apcUnnaturalTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
     DrawablePtr        pDrawable;
     GC                 *pGC;
     int                nInit;           /* number of spans to fill */
     DDXPointPtr        pptInit;         /* pointer to list of start points */
     int                *pwidthInit;     /* pointer to list of n widths */
     int                fSorted;
{
  int                   iline;           /* first line of tile to use */

  DDXPointPtr           ppt, pptLast;    /* Next 7 parameters used to clip */
  BoxPtr                pbox, pboxLast, pboxTest;
  int                   yMax;
  int                   *pwidth;         /* pointer to list of n widths */
  
  int                   *addrlBase;      /* pointer to start of bitmap */
  int                   nlwidth;         /* width in longwords of bitmap */
  int                   *pdst;           /* pointer to current word in bitmap */
  int                   *psrc;           /* pointer to current word in tile */
  int                   startmask;
  int                   nlMiddle;
  PixmapPtr             pTile;       /* pointer to tile we want to fill with */
  int                   w, width, x, xSrc, ySrc, tmpSrc;
  int                   srcStartOver, nstart, nend;
  int                   endmask, tlwidth, rem, tileWidth, *psrcT, endinc, rop;
  int                   *tbase;
  int                   *BaseBase;       /* pointer to start of bitmap */
  int                   mainmemoffset;
  gpr_$offset_t         tsize, size;
  gpr_$rgb_plane_t      depth;
  gpr_$bitmap_desc_t    bitmap_id, cur_bitmap;
  status_$t             status;
  
  if (!(pGC->planemask)) return;
  
  depth = (gpr_$rgb_plane_t)pGC->depth;
  
  pptLast = pptInit + nInit;             /* Init clip stuff */
  pboxTest = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
  pboxLast = pboxTest + ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->numRects;
  yMax = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->extents.y2;
  
  pTile = pGC->tile;
  tileWidth = pTile->width;
  psrcT = ((apcPrivPM *)pTile->devPrivate)->bitmap_ptr;
  
  /* tlwidth is number of long words to next scan line of Tile */
  tlwidth = (int)(((apcPrivPM *)pTile->devPrivate)->width) >> 1;
  tsize = ((apcPrivPM *)pTile->devPrivate)->size;
  
  rop = pGC->alu;
  
  if (pDrawable->type == DRAWABLE_WINDOW) {
    addrlBase = (int *)apDisplayData[pDrawable->pScreen->myNum].bitmap_ptr;
    nlwidth = (int)(apDisplayData[pDrawable->pScreen->myNum].words_per_line) >> 1;
  }
  else {
    BaseBase = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_ptr;
    nlwidth = (int)(((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->width) >> 1;
    size = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->size;
    mainmemoffset = nlwidth*size.y_size;
  }
  
  gpr_$enable_direct_access( status);
  while ((depth--) && ((pGC->planemask >> depth) &1)) {
    if (pDrawable->type == DRAWABLE_WINDOW) gpr_$remap_color_memory(depth, status);
    else addrlBase = (int *)(mainmemoffset*depth+BaseBase);
    
    pwidth = pwidthInit;
    ppt = pptInit;
    tbase = psrcT + (tsize.y_size * tlwidth * depth);
    
    if (pDrawable->type == DRAWABLE_WINDOW) {
      xSrc = ((WindowPtr) pDrawable)->absCorner.x;
      ySrc = ((WindowPtr) pDrawable)->absCorner.y;
    }
    else {
      xSrc = 0;
      ySrc = 0;
    }       
    /* this replaces rotating the tile. Instead we just adjust the offset
     * at which we start grabbing bits from the tile */
    xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
    ySrc += (pGC->patOrg.y % pTile->height) - pTile->height;
    
    for (; ppt < pptLast; pwidth++, ppt++) {
      if(fSorted) {
        if(ppt->y >= yMax)
          break;
        pbox = pboxTest;
      }
      else {
        if(ppt->y >= yMax)
          continue;
        pbox = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
      }
      if(*pwidth == 0)
        continue;
      while(pbox < pboxLast) {
        
        if(pbox->y1 > ppt->y) {
          /* scanline is before clip box */
          break;
        }
        
        if(pbox->y2 <= ppt->y) {
          /* clip box is before scanline */
          pboxTest = ++pbox;
          continue;
        }
        
        if(pbox->x1 >= ppt->x + *pwidth) {
          /* clip box is to right of scanline */
          break;
        }
        
        if(pbox->x2 <= ppt->x) {
          /* scanline is to right of clip box */
          pbox++;
          continue;
        }
        /* at least some of the scanline is in the current clip box */
        x = max(pbox->x1, ppt->x);
        width = min(ppt->x+ *pwidth, pbox->x2) - x;
        iline = (ppt->y - ySrc) % pTile->height;
        pdst = addrlBase + (ppt->y * nlwidth) + (x >> 5);
        
        while(width > 0) {
          psrc = tbase + (iline * tlwidth); 
          w = min(tileWidth, width);
          if((rem = (x - xSrc)  % tileWidth) != 0) {
            /* if we're in the middle of the tile, get
               as many bits as will finish the span, or
               as many as will get to the left edge of the tile,
               or a longword worth, starting at the appropriate
               offset in the tile.
             */
            w = min(min(tileWidth - rem, width), BITMAP_SCANLINE_UNIT);
            endinc = rem / BITMAP_SCANLINE_UNIT;
            getbits(psrc + endinc, rem & 0x1f, w, tmpSrc);
            putbitsrop(tmpSrc, (x & 0x1f), w, pdst, rop);
            if((x & 0x1f) + w >= 0x20)
              pdst++;
          }
          else if(((x & 0x1f) + w) < 32) {
            /* doing < 32 bits is easy, and worth special-casing */
            getbits(psrc, 0, w, tmpSrc);
            putbitsrop(tmpSrc, x & 0x1f, w, pdst, rop);
          }
          else {
            /* start at the left edge of the tile,
               and put down as much as we can
             */
            maskbits(x, w, startmask, endmask, nlMiddle);
            
            if (startmask)
              nstart = 32 - (x & 0x1f);
            else
              nstart = 0;
            if (endmask)
              nend = (x + w)  & 0x1f;
            else
              nend = 0;
            
            srcStartOver = nstart > 31;
            
            if(startmask) {
              getbits(psrc, 0, nstart, tmpSrc);
              putbitsrop(tmpSrc, (x & 0x1f), nstart, pdst, rop);
              pdst++;
              if(srcStartOver)
                psrc++;
            }
            
            while(nlMiddle--) {
              getbits(psrc, nstart, 32, tmpSrc);
              *pdst = DoRop(rop, tmpSrc, *pdst);
              pdst++;
              psrc++;
            }
            if(endmask) {
              getbits(psrc, nstart, nend, tmpSrc);
              putbitsrop(tmpSrc, 0, nend, pdst, rop);
            }
          }
          x += w;
          width -= w;
        }
	if (pptInit->x + *pwidthInit <= pbox->x2)
          break;                /* We hit EOL */
	else
	  pbox++;
      }     /* End of clip while loop */
    }       /* End of clip for loop */
  }         /* End of while depth-- */
}

/* Fill spans with stipples that aren't 32 bits wide */
void
apcUnnaturalStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
     DrawablePtr         pDrawable;
     GC                  *pGC;
     int                 nInit;          /* number of spans to fill */
     DDXPointPtr         pptInit;        /* pointer to list of start points */
     int                 *pwidthInit;    /* pointer to list of n widths */
     int                 fSorted;
{
  DDXPointPtr         ppt, pptLast;      /* Next 7 parameters used to clip */
  BoxPtr              pbox, pboxLast, pboxTest;
  int                 yMax;
  int                 *pwidth;           /* pointer to list of n widths */
  
  int                 iline;             /* first line of tile to use */
  int                 *addrlBase;        /* pointer to start of bitmap */
  int                 nlwidth;           /* width in longwords of bitmap */
  int                 *pdst;             /* pointer to current word in bitmap */
  int                 *psrc;             /* pointer to current word in tile */
  int                 startmask;
  int                 nlMiddle;
  PixmapPtr           pTile;             /* pointer to tile we want to fill with */
  int                 w, width,  x, xSrc, ySrc, tmpSrc;
  int                 srcStartOver, nstart, nend;
  int                 endmask, tlwidth, rem, tileWidth, *psrcT, endinc, rop;
  
  int                 tmpmask; 
  int                 t1, t2; 
  int                 *tbase;
  int                 *BaseBase;
  int                 mainmemoffset;
  gpr_$offset_t       tsize, size;
  gpr_$rgb_plane_t    depth;
  gpr_$bitmap_desc_t  cur_bitmap, bitmap_id;
  status_$t           status;
  
  if (!(pGC->planemask))
    return;
  
  depth = (gpr_$rgb_plane_t)pGC->depth;
  
  pptLast = pptInit + nInit;             /* Init clip stuff */
  pboxTest = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
  pboxLast = pboxTest + ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->numRects;
  yMax = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->extents.y2;
  
  pTile = pGC->stipple;
  tileWidth = pTile->width;
  
  tbase = ((apcPrivPM *)pTile->devPrivate)->bitmap_ptr;
  
  /* tlwidth is number of long words to next scan line of Stipple */
  tlwidth = (int)(((apcPrivPM *)pTile->devPrivate)->width) >> 1;
  tsize = ((apcPrivPM *)pTile->devPrivate)->size;
  
  if (pDrawable->type == DRAWABLE_WINDOW) {
    addrlBase = (int *)apDisplayData[pDrawable->pScreen->myNum].bitmap_ptr;
    nlwidth = (int)(apDisplayData[pDrawable->pScreen->myNum].words_per_line) >> 1;
  }
  else {
    BaseBase = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_ptr;
    nlwidth = (int)(((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->width) >> 1;
    size = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->size;
    mainmemoffset = nlwidth*size.y_size;
  }
  
  gpr_$enable_direct_access( status);
  while ((depth--) && ((pGC->planemask >> depth) &1)) {
    
    if (pDrawable->type == DRAWABLE_WINDOW) gpr_$remap_color_memory(depth, status);
    else addrlBase = (int *)(mainmemoffset*depth+BaseBase);
    
    pwidth = pwidthInit;
    ppt = pptInit;
    
    if( (pGC->fgPixel >> depth) &1 ) {
      rop = 0x4 | ( pGC->alu & 3 );
    }
    else {
      rop = 0x4 | (pGC->alu >> 2);
    }
    
    if (pDrawable->type == DRAWABLE_WINDOW) {
      xSrc = ((WindowPtr) pDrawable)->absCorner.x;
      ySrc = ((WindowPtr) pDrawable)->absCorner.y;
    }
    else {
      xSrc = 0;
      ySrc = 0;
    }       
    /* this replaces rotating the tile. Instead we just adjust the offset
     * at which we start grabbing bits from the tile */
    xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
    ySrc += (pGC->patOrg.y % pTile->height) - pTile->height;
    
    for (; ppt < pptLast; pwidth++, ppt++) {
      if(fSorted) {
        if(ppt->y >= yMax)
          break;
        pbox = pboxTest;
      }
      else {
        if(ppt->y >= yMax)
          continue;
        pbox = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
      }
      if(*pwidth == 0)
        continue;
      while(pbox < pboxLast) {
        
        if(pbox->y1 > ppt->y) {
          /* scanline is before clip box */
          break;
        }
        
        if(pbox->y2 <= ppt->y) {
          /* clip box is before scanline */
          pboxTest = ++pbox;
          continue;
        }
        
        if(pbox->x1 >= ppt->x + *pwidth) {
          /* clip box is to right of scanline */
          break;
        }
        
        if(pbox->x2 <= ppt->x) {
          /* scanline is to right of clip box */
          pbox++;
          continue;
        }
        /* at least some of the scanline is in the current clip box */
        x = max(pbox->x1, ppt->x);
        width = min(ppt->x+ *pwidth, pbox->x2) - x;
        iline = (ppt->y - ySrc) % pTile->height;
        pdst = addrlBase + (ppt->y * nlwidth) + (x >> 5);
        psrcT = tbase + (iline * tlwidth);                                
        
        while(width > 0) {
          psrc = psrcT;
          w = min(tileWidth, width);
          if((rem = (x - xSrc)  % tileWidth) != 0) {
            /* if we're in the middle of the tile, get
               as many bits as will finish the span, or
               as many as will get to the left edge of the tile,
               or a longword worth, starting at the appropriate
               offset in the tile.
             */
            w = min(min(tileWidth - rem, width), BITMAP_SCANLINE_UNIT);
            endinc = rem / BITMAP_SCANLINE_UNIT;
            getbits(psrc + endinc, rem & 0x1f, w, tmpSrc);
            putbitsrop(tmpSrc, (x & 0x1f), w, pdst, rop);
            if((x & 0x1f) + w >= 0x20)
              pdst++;
          }
          else if(((x & 0x1f) + w) < 32) {
            /* doing < 32 bits is easy, and worth special-casing */
            getbits(psrc, 0, w, tmpSrc);
            putbitsrop(tmpSrc, x & 0x1f, w, pdst, rop); 
          }
          else {
            /* start at the left edge of the tile,
               and put down as much as we can
             */
            maskbits(x, w, startmask, endmask, nlMiddle);
            
            if (startmask)
              nstart = 32 - (x & 0x1f);
            else
              nstart = 0;
            if (endmask)
              nend = (x + w)  & 0x1f;
            else
              nend = 0;
            
            srcStartOver = nstart > 31;
            
            if(startmask) {
              getbits(psrc, 0, nstart, tmpSrc);
              putbitsrop(tmpSrc, (x & 0x1f), nstart, pdst, rop);
              pdst++;
              if(srcStartOver)
                psrc++;
            }
            while(nlMiddle--) {
              getbits(psrc, nstart, 32, tmpSrc);
              *pdst = DoRop(rop, tmpSrc, *pdst);
              pdst++;
              psrc++;
            }
            if(endmask) {
              getbits(psrc, nstart, nend, tmpSrc);
              putbitsrop(tmpSrc, 0, nend, pdst, rop);
            }
          }
          x += w;
          width -= w;
        }
	if (pptInit->x + *pwidthInit <= pbox->x2)
          break;                /* We hit EOL */
        else
	  pbox++;
      }     /* End of clip while loop */
    }       /* End of clip for loop */
  }         /* End of while depth-- */
}

/* Fill spans with opaque stipples that aren't 32 bits wide */
void
apcUnnaturalOpaqueStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
     DrawablePtr         pDrawable;
     GC                  *pGC;
     int                 nInit;          /* number of spans to fill */
     DDXPointPtr         pptInit;        /* pointer to list of start points */
     int                 *pwidthInit;    /* pointer to list of n widths */
     int                 fSorted;
{
  DDXPointPtr         ppt, pptLast;      /* Next 7 parameters are to clip */
  BoxPtr              pbox, pboxLast, pboxTest;
  int                 yMax;
  int                 *pwidth;           /* pointer to list of n widths */
  
  int                 iline;             /* first line of tile to use */
  int                 *addrlBase;        /* pointer to start of bitmap */
  int                 nlwidth;           /* width in longwords of bitmap */
  int                 *pdst;             /* pointer to current word in bitmap */
  int                 *psrc;             /* pointer to current word in tile */
  int                 startmask;
  int                 nlMiddle;
  PixmapPtr           pTile;             /* pointer to tile we want to fill with */
  int                 w, width,  x, xSrc, ySrc, tmpSrc;
  int                 srcStartOver, nstart, nend;
  int                 endmask, tlwidth, rem, tileWidth, *psrcT, endinc, rop;
  
  int                 *baseT;
  int                 mainmemoffset;
  int                 frgrnd, bckgrnd;
  int                 *BaseBase;
  gpr_$offset_t       tsize, size;
  gpr_$rgb_plane_t    depth;
  gpr_$bitmap_desc_t  bitmap_id;
  status_$t           status;
  
  if (!(pGC->planemask))
    return;
  
  depth = (gpr_$rgb_plane_t)pGC->depth;
  
  pptLast = pptInit + nInit;             /* Init clip stuff */
  pboxTest = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
  pboxLast = pboxTest + ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->numRects;
  yMax = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->extents.y2;
  
  pTile = pGC->stipple;
  tileWidth = pTile->width;
  
  baseT = ((apcPrivPM *)pTile->devPrivate)->bitmap_ptr;
  
  /* tlwidth is number of long words to next scan line of Stipple */
  tlwidth = (int)(((apcPrivPM *)pTile->devPrivate)->width) >> 1;
  tsize = ((apcPrivPM *)pTile->devPrivate)->size;
  
  rop = pGC->alu;
  
  if (pDrawable->type == DRAWABLE_WINDOW) {
    addrlBase = (int *)apDisplayData[pDrawable->pScreen->myNum].bitmap_ptr;
    nlwidth = (int)(apDisplayData[pDrawable->pScreen->myNum].words_per_line) >> 1;
  }
  else {
    BaseBase = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_ptr;
    nlwidth = (int)(((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->width) >> 1;
    size = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->size;
    mainmemoffset = nlwidth*size.y_size;
  }
  
  if (pDrawable->type == DRAWABLE_WINDOW) {
    xSrc = ((WindowPtr) pDrawable)->absCorner.x;
    ySrc = ((WindowPtr) pDrawable)->absCorner.y;
  }
  else {
    xSrc = 0;
    ySrc = 0;
  }       
  /* this replaces rotating the tile. Instead we just adjust the offset
   * at which we start grabbing bits from the tile */
  xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
  ySrc += (pGC->patOrg.y % pTile->height) - pTile->height;
  
  gpr_$enable_direct_access( status);
  while ((depth--) && ((pGC->planemask >> depth) &1)) {
    
    if (pDrawable->type == DRAWABLE_WINDOW) gpr_$remap_color_memory(depth, status);
    else addrlBase = (int *)(mainmemoffset*depth+BaseBase);
    
    pwidth = pwidthInit;
    ppt = pptInit;
    
    frgrnd = (pGC->fgPixel >> depth)&1;
    bckgrnd = (pGC->bgPixel >> depth)&1;
    if (frgrnd == bckgrnd) {
      /* since foreground = background, no reading of the stipple is
         necessary.  The fill source IS the foreground/background */
      if (frgrnd) {
        for (; ppt < pptLast; pwidth++, ppt++) {
          if(fSorted) {
            if(ppt->y >= yMax)
              break;
            pbox = pboxTest;
          }
          else {
            if(ppt->y >= yMax)
              continue;
            pbox = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
          }
          if(*pwidth == 0)
            continue;
          while(pbox < pboxLast) {
            
            if(pbox->y1 > ppt->y) {
              /* scanline is before clip box */
              break;
            }
            
            if(pbox->y2 <= ppt->y) {
              /* clip box is before scanline */
              pboxTest = ++pbox;
              continue;
            }
            
            if(pbox->x1 >= ppt->x + *pwidth) {
              /* clip box is to right of scanline */
              break;
            }
            
            if(pbox->x2 <= ppt->x) {
              /* scanline is to right of clip box */
              pbox++;
              continue;
            }
            /* at least some of the scanline is in the current clip box */
            x = max(pbox->x1, ppt->x);
            width = min(ppt->x+ *pwidth, pbox->x2) - x;
            pdst = addrlBase + (ppt->y * nlwidth) + (x >> 5);
            
            if ( ((x & 0x1f) + width) < 32) {
              /* all bits inside same longword */
              maskpartialbits(x, width, startmask);
              *pdst |= startmask;
            }
            else {
              maskbits(x, width, startmask, endmask, nlMiddle);
              if (startmask)
                *pdst++ |= startmask;
              while (nlMiddle--)
                *pdst++ = 0xffffffff;
              if (endmask)
                *pdst |= endmask;
            }
            if (pptInit->x + *pwidthInit <= pbox->x2)
              break;                /* We hit EOL */
            else
              pbox++;
          }
        }
      }
      else {
        for (; ppt < pptLast; pwidth++, ppt++) {
          if(fSorted) {
            if(ppt->y >= yMax)
              break;
            pbox = pboxTest;
          }
          else {
            if(ppt->y >= yMax)
              continue;
            pbox = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
          }
          if(*pwidth == 0)
            continue;
          while(pbox < pboxLast) {
            
            if(pbox->y1 > ppt->y) {
              /* scanline is before clip box */
              break;
            }
            
            if(pbox->y2 <= ppt->y) {
              /* clip box is before scanline */
              pboxTest = ++pbox;
              continue;
            }
            
            if(pbox->x1 >= ppt->x + *pwidth) {
              /* clip box is to right of scanline */
              break;
            }
            
            if(pbox->x2 <= ppt->x) {
              /* scanline is to right of clip box */
              pbox++;
              continue;
            }
            /* at least some of the scanline is in the current clip box */
            x = max(pbox->x1, ppt->x);
            width = min(ppt->x+ *pwidth, pbox->x2) - x;
            pdst = addrlBase + (ppt->y * nlwidth) + (x >> 5);
            
            if ( ((x & 0x1f) + width) < 32) {
              /* all bits inside same longword */
              maskpartialbits(x, width, startmask);
              *pdst &= ~startmask;
            }
            else {
              maskbits(x, width, startmask, endmask, nlMiddle);
              if (startmask)
                *pdst++ &= ~startmask;
              while (nlMiddle--)
                *pdst++ = 0x0;
              if (endmask)
                *pdst &= ~endmask;
            }
            if (pptInit->x + *pwidthInit <= pbox->x2)
              break;                /* We hit EOL */
            else
              pbox++;
          }
        }
      }
    }
    /*  else the foreground and background are different  */
    /*  we either write in the stipple or inverse stipple */
    else {
      
      for (; ppt < pptLast; pwidth++, ppt++) {
        if(fSorted) {
          if(ppt->y >= yMax)
            break;
          pbox = pboxTest;
        }
        else {
          if(ppt->y >= yMax)
            continue;
          pbox = ((apcPrivGC *) (pGC->devPriv))->pCompositeClip->rects;
        }
        if(*pwidth == 0)
          continue;
        while(pbox < pboxLast) {
          
          if(pbox->y1 > ppt->y) {
            /* scanline is before clip box */
            break;
          }
          
          if(pbox->y2 <= ppt->y) {
            /* clip box is before scanline */
            pboxTest = ++pbox;
            continue;
          }
          
          if(pbox->x1 >= ppt->x + *pwidth) {
            /* clip box is to right of scanline */
            break;
          }
          
          if(pbox->x2 <= ppt->x) {
            /* scanline is to right of clip box */
            pbox++;
            continue;
          }
          /* at least some of the scanline is in the current clip box */
          x = max(pbox->x1, ppt->x);
          width = min(ppt->x+ *pwidth, pbox->x2) - x;
          iline = (ppt->y - ySrc) % pTile->height;
          pdst = addrlBase + (ppt->y * nlwidth) + (x >> 5);
          psrcT = baseT + (iline * tlwidth); 
          
          while(width > 0) {
            psrc = psrcT;
            w = min(tileWidth, width);
            if((rem = (x - xSrc)  % tileWidth) != 0) {
              /* if we're in the middle of the tile, get
                 as many bits as will finish the span, or
                 as many as will get to the left edge of the tile,
                 or a longword worth, starting at the appropriate
                 offset in the tile.
               */
              w = min(min(tileWidth - rem, width), BITMAP_SCANLINE_UNIT);
              endinc = rem / BITMAP_SCANLINE_UNIT;
              getbits(psrc + endinc, rem & 0x1f, w, tmpSrc);
              if (bckgrnd) tmpSrc = !tmpSrc;
              putbitsrop(tmpSrc, (x & 0x1f), w, pdst, rop);
              if((x & 0x1f) + w >= 0x20)
                pdst++;
            }
            else if(((x & 0x1f) + w) < 32) {
              /* doing < 32 bits is easy, and worth special-casing */
              getbits(psrc, 0, w, tmpSrc);
              if (bckgrnd) tmpSrc = !tmpSrc;
              putbitsrop(tmpSrc, x & 0x1f, w, pdst, rop);
            }
            else {
              /* start at the left edge of the tile,
                 and put down as much as we can
               */
              maskbits(x, w, startmask, endmask, nlMiddle);
              
              if (startmask)
                nstart = 32 - (x & 0x1f);
              else
                nstart = 0;
              if (endmask)
                nend = (x + w)  & 0x1f;
              else
                nend = 0;
              
              srcStartOver = nstart > 31;
              
              if(startmask) {
                getbits(psrc, 0, nstart, tmpSrc);
                if (bckgrnd) tmpSrc = !tmpSrc;
                putbitsrop(tmpSrc, (x & 0x1f), nstart, pdst, rop);
                pdst++;
                if(srcStartOver)
                  psrc++;
              }
              
              while(nlMiddle--) {
                getbits(psrc, nstart, 32, tmpSrc);
                if (bckgrnd) tmpSrc = !tmpSrc;
                *pdst = DoRRop(rop, tmpSrc, *pdst);
                pdst++;
                psrc++;
              }
              if(endmask) {
                getbits(psrc, nstart, nend, tmpSrc);
                if (bckgrnd) tmpSrc = !tmpSrc;
                putbitsrop(tmpSrc, 0, nend, pdst, rop);
              }
            }
            x += w;
            width -= w;
          }
          if (pptInit->x + *pwidthInit <= pbox->x2)
            break;                /* We hit EOL */
          else
            pbox++;
        }
      }
    }
  }
}
