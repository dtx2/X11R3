/*
Copyright (c) 1986, 1987, 1988 by Hewlett-Packard Company
HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.
*/
/* $XConsortium: hpCopyArea.c,v 1.4 88/09/30 14:16:22 jim Exp $ */

/* 
 * hpCopyArea.c : a copy area for HP displays
 * Author: C Durland  (aided and abetted by Mr. J. Daniels)
 */

#include "X.h"

#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "mi.h"

#include "cfb.h"
#include "hpbuf.h"


/* 
 MTOS : main memory to screen
 MTOM : main mem to main mem, screen to main mem
 STOS : screen to screen
			    DESTINATION
			       | screen | main memory
			window | pixmap | pixmap
	SOURCE	      .--------|--------|-------
   window	      |  STOS  |  STOS  | MTOM
   screen pixmap      |  STOS  |  STOS  | MTOM
   main memory pixmap |  MTOS  |  MTOS  | MTOM

			Dest clip list		Source clip list
   window		composite clip		inferiors or window clip list
   screen pixmap	maybe a client clip	no
   main memory pixmap	maybe a client clip	no
*/

#if 0

static unsigned char *addr;
static unsigned short int cnt;

#define Z (12*4)	/* 12 registers of 4 bytes each */

	/* a 68020 block mover */
void blkmov(to,from,n) unsigned char *to, *from; int n;
{
  addr = from;
  asm("mov.l _addr,  %a0");	/* a0 = from */
  addr = to;
  asm("mov.l _addr,  %a1");	/* a1 = to */
  
	/* move chunks of 48 bytes */
  cnt = n/Z;
  asm("mov.w _cnt, %d0");
  asm("beq $S10");		/* skip if zero */
  asm("movm.l %d1-%d7 / %a2-%a6,-(%sp)");	/* save registers */
  asm("bra $M20");
asm("$M10:");
  asm("movm.l (%a0)+,%d1-%d7 / %a2-%a6");
  asm("movm.l %d1-%d7 / %a2-%a6,(%a1)");
  asm("add.w &0x30,%a1");
  asm("$M20: dbf %d0,$M10");
  asm("movm.l (%sp)+,%d1-%d7 / %a2-%a6");	/* restore registers */

asm("$S10:");
	/* move chunks of 4 bytes */
  n = n -cnt*Z;
  cnt = n>>2;
  asm("mov.w _cnt, %d0");
  asm("beq $S20");		/* skip if zero */
  asm("bra $Q20");
  asm("$Q10: mov.l (%a0)+,(%a1)+");	/* move those long words */
  asm("$Q20: dbf %d0,$Q10");

asm("$S20:");
	/* move bytes */
  cnt = n & 3;
  asm("mov.w _cnt, %d0");
  asm("beq $S30");	/* skip if zero */
  asm("bra $B20");
  asm("$B10: mov.b (%a0)+,(%a1)+");	/* move those bytes */
  asm("$B20: dbf %d0,$B10");

asm("$S30:");
}

extern int XHP_QUADALIGN;	/* 0 => don't care about alignment */

#define COPYROW(to,from,n) \
	if (XHP_QUADALIGN) memcpy(to,from,n); else blkmov(to,from,n)

#else

#define COPYROW(to,from,n) memcpy(to,from,n)

#endif

#define PIXER(Drawable)  ((cfbPrivPixmapPtr)((PixmapPtr)Drawable)->devPrivate)
#define SCRMER(Drawable) ((cfbPrivScreenPtr)(Drawable->pScreen)->devPrivate)
#define DEVKIND(Drawable) ((PixmapPtr)Drawable)->devKind

#define SWEAT_PLANES_MASK(Drawable,planesmask)		\
  ((SCRMER(Drawable)->planesMask & planesmask) !=	\
    SCRMER(Drawable)->planesMask)

	/* Clip rectangle (x,y)-(x+w,y+h) against (a,b)-(c,d)
	 * Returns the cliped (x,y) and w,h
	 */
void clipper(x,y,w,h, a,b,c,d, cx,cy, cw,ch)
register int x,y,w,h, a,b,c,d; int *cx,*cy, *cw,*ch;
{
  *cw = min(x+w,c) -(*cx = max(x,a));
  *ch = min(y+h,d) -(*cy = max(y,b));
}

	/* Check to see if can copy area between two rectangles:
	 *   If the dest is above the source: can copy
	 *   else if they overlap:  Can't copy
	 * Have to check horizontal because of clip lists.  boxes is the sum
	 *  of the number of clip rectangles for the source and dest.
	 * Returns: FALSE if can copy area (ie any overlap don't matter)
	 *    true if got a overlap problem
	 */
static lapdog(srcx,srcy, w,h, dstx,dsty, boxes)
{
  return
    ( (srcy <= dsty) || ((boxes > 2) && (srcx < dstx)) ) &&
    ( ((min(srcx,dstx)+w -max(srcx,dstx)) >0) &&
      ((min(srcy,dsty)+h -max(srcy,dsty)) >0) );
}

	/* Copy rectangles from screen to screen using the block mover
	 * Overlapping moves:
	 *   Assumes that block mover can handle overlapping copies.
	 *   If there is a clip list, care must be taken so that copying
	 *     a block does not trash a block to be copied.
	 * (?tx, ?ty) are translation constants to convert pixmaps
	 *   to screen coordinates.  0 if not a pixmap.
	 */
void
ScreenToScreen(Drawable,gc, sx,sy,width,height, dx,dy,
	sbox,sboxes, dbox,dboxes, stx,sty, dtx,dty)
DrawablePtr Drawable;
GCPtr gc;
int sx,sy,width,height, dx,dy, sboxes, dboxes, stx,sty, dtx,dty;
BoxPtr sbox, dbox;	/* the clip lists */
{
  register BoxPtr btr;
  int j, x1,y1, w1,h1, x2,y2, w2,h2,   a,b,c,d, tx = sx-dx, ty = sy-dy;
  ScreenPtr Screen = Drawable->pScreen;

  if (gc->alu==GXnoop) return;		/* no op => don't do nothin */

  for (; sboxes--; sbox++)	/* for each source box */
  {
	  /* intersect source and source clip rectangle */
    clipper(sx,sy,width,height, sbox->x1,sbox->y1, sbox->x2,sbox->y2,
	&x1,&y1, &w1,&h1);
    if (w1<=0 || h1<=0) continue;
	/* translate box to dst coordinates */
    a = x1 -tx; b = y1 -ty; c = a +w1; d = b +h1;
    for (j = dboxes, btr = dbox; j--; btr++)	/* copy to the dest box */
    {
	  /* intersect dst and dst clip rectangles */
      clipper(dx,dy,width,height, btr->x1,btr->y1, btr->x2,btr->y2,
	&x2,&y2, &w2,&h2);
      if (w2<=0 || h2<=0) continue;
	  /* intersect clipped src and clipped dst rectangles */
      clipper(x2,y2,w2,h2, a,b,c,d, &x1,&y1, &w1,&h1);
      if (w1<=0 || h1<=0) continue;
      (*((cfbPrivScreenPtr)(Screen->devPrivate))->MoveBits)
	(Screen,gc->planemask,gc->alu,
	  x1+tx+stx,y1+ty+sty, x1+dtx,y1+dty, w1,h1);
    }  
  }
}

	/* copy a rectangle of bytes: no processing */
static void CopyRec0(dst,w,h,dst_stride, alu)
register char *dst;
int w,h,dst_stride, alu;
{
  register int z,j;

  switch (alu)
  {
    case GXclear: z = 0; break;		/* 0 */
    case GXset: z = ~0; break;		/* 1 */
  }
  while (h--)		/* copy h rows */
  {
#if 0
    j = w; while (j--) dst[j] = z;
#else
    memset(dst,z,w);
#endif
    dst += dst_stride;	/* move to next row */
  }
}

	/* copy a rectangle of bytes: only process src */
	/* two cases: GXcopy & GXcopyInverted */
static void CopyRec1(src,dst,w,h,src_stride,dst_stride, alu)
register char *src, *dst;
int w,h,src_stride,dst_stride, alu;
{
  register int j;

  if (alu==GXcopy)
    while (h--)		/* copy h rows */
    {
      COPYROW(dst,src,w);
      src += src_stride; dst += dst_stride;	/* move to next row */
    }
  else
    while (h--)		/* copy h rows */
    {
      j = w;
      while (j--) dst[j] = ~src[j];
      src += src_stride; dst += dst_stride;	/* move to next row */
    }
}

	/* copy a rectangle of bytes: process src & dst */
static void CopyRec2(src,dst,w,h,src_stride,dst_stride, alu,planesmask)
register char *src, *dst;
int w,h,src_stride,dst_stride, alu;
register long int planesmask;
{
  register char a, b;
  register int j;

 /*!!! slime bag note: if planesmask > 8 bits this routine no workie */

  while (h--)	/* copy h rows */
  {
    j = w;
    while (j--)		/* copy a row */
    {
      a = src[j]; b = dst[j];
      switch (alu)
      {
	case GXclear: a = 0; break;		/* 0 */
	case GXset: a = ~0; break;		/* 1 */

	case GXcopyInverted: b = ~a; break;	/* ~src */
/* 	case GXcopy: a = a; break;		/* src */

	case GXand: a = a & b; break;	      /* src AND dst */
	case GXandReverse: a = a & (~b); break;   /* src AND NOT dst */
	case GXandInverted: a = (~a) & b; break;  /* NOT src AND dst */
	case GXxor: a = a ^ b; break;	      /* src XOR dst */
	case GXor: a = a | b; break;	      /* src OR dst */
	case GXnor: a = (~a) & (~b); break;   /* NOT src AND NOT dst */
	case GXequiv: a = (~a) ^ b; break;    /* NOT src XOR dst */
	case GXinvert: a = ~b; break;	      /* NOT dst */
	case GXorReverse: a = a | (~b); break;    /* src OR NOT dst */
	case GXorInverted: a = (~a) | b; break;   /* NOT src OR dst */
	case GXnand: a = (~a) | (~b); break;	  /* NOT src OR NOT dst */
      }
      dst[j] = (a & planesmask) | (b & ~planesmask);
    }
    src += src_stride; dst += dst_stride;	/* move to next row */
  }
}

	/* move bytes from main memory to main memory
	 *   or move bytes from screen to main memory
	 * concerns: Replacement rule, overlapping copies,
	 * Only works for depths of 1 and multiples of 8.
	 */
void MemToMem(SrcDrawable,DstDrawable,gc, sx,sy,width,height, dx,dy,
	sbox,sboxes, dbox,dboxes)
DrawablePtr SrcDrawable,DstDrawable;
GCPtr gc;
int sx,sy,width,height, dx,dy, sboxes, dboxes;
BoxPtr sbox, dbox;	/* the clip lists */
{
  unsigned char *src, *dst, *presrc;
  register int src_stride, dst_stride;
  int
    j, x1,y1, w1,h1, x2,y2, w2,h2,   a,b,c,d, tx = sx-dx, ty = sy-dy,
    alu, fake_alu,
    DepthInBytes = SrcDrawable->depth/8;
  unsigned long int planesmask;
  register BoxPtr btr;

  if ((alu=gc->alu)==GXnoop) return;	/* no op => don't do nothin */

  fake_alu = alu; planesmask = gc->planemask;

  if (DepthInBytes<1)	/* moving bits */
  {
    DepthInBytes = 1;	/* fake around */
    if (planesmask==0) return;	/* no bits will be changed */
    planesmask = ~0;		/* all bits can be changed */
  }

	/* check to see if gotta sweat the planes mask */
  if (SWEAT_PLANES_MASK(SrcDrawable,planesmask)) fake_alu = 666;

  dst_stride = PIXER(DstDrawable)->stride;
  if (SrcDrawable->type ==DRAWABLE_WINDOW)	/* from screen */
  {
    src_stride = SCRMER(SrcDrawable)->stride;
    presrc = SCRMER(SrcDrawable)->bits;
  }
  else			/* from main memory or offscreen pixmap */
  {
    src_stride = PIXER(SrcDrawable)->stride;
    presrc = PIXER(SrcDrawable)->bits;
  }

  for (; sboxes--; sbox++)	/* for each source box */
  {
	/* intersect src and src clip rectangle */
    clipper(sx,sy,width,height, sbox->x1,sbox->y1,sbox->x2,sbox->y2,
	&x1,&y1, &w1,&h1);
    if (w1<=0 || h1<=0) continue;
	/* translate box to dst coordinates */
    a = x1 -tx; b = y1 -ty; c = a +w1; d = b +h1;
    for (j = dboxes, btr = dbox; j--; btr++)
    {
	  /* intersect dst and dst clip rectangles */
      clipper(dx,dy,width,height, btr->x1,btr->y1, btr->x2,btr->y2,
	&x2,&y2, &w2,&h2);
      if (w2<=0 || h2<=0) continue;
	  /* intersect clipped src and clipped dst rectangles */
      clipper(x2,y2,w2,h2, a,b,c,d, &x1,&y1, &w1,&h1);
      if (w1<=0 || h1<=0) continue;
      w1 *= DepthInBytes;
	/* convert rectangle to addresses */
      src = presrc + (x1+tx)*DepthInBytes +(y1+ty)*src_stride;
      dst = PIXER(DstDrawable)->bits +x1*DepthInBytes +y1*dst_stride;

      switch (fake_alu)
      {
	case GXclear: case GXset:
	  CopyRec0(dst,w1,h1,dst_stride, alu); break;
	case GXcopy: case GXcopyInverted:
	  CopyRec1(src,dst,w1,h1,src_stride,dst_stride, alu);
	  break;
	default: CopyRec2(src,dst,w1,h1,src_stride,dst_stride, alu,planesmask);
      }
    }
  }  
}


	/* Move bytes from main memory to screen memory.
	 * (dtx,dty) are translation constants to convert pixmaps
	 *   to screen coordinates.  0 if not a pixmap.
	 */
void MemToScreen(SrcDrawable,DstDrawable,gc, sx,sy,width,height, dx,dy,
   dboxes, dbox, dtx,dty)
DrawablePtr SrcDrawable,DstDrawable;
GCPtr gc;
int sx,sy,width,height, dx,dy, dboxes, dtx,dty;
BoxPtr dbox;	/* the dst clip list */
{
  register unsigned char *src, *dst;
  register int src_stride, dst_stride;
  int
    w,h, x1,y1, x2,y2,
    DepthInBytes = SrcDrawable->depth/8;

	/* setup hardware */
  SET_REGISTERS_FOR_WRITING(gc->pScreen,gc->planemask,gc->alu);
  src_stride = PIXER(SrcDrawable)->stride;
  dst_stride = SCRMER(DstDrawable)->stride;
  for (; dboxes--; dbox++)
  {
	/* intersect dst and clip rectangles */
    clipper(dx,dy,width,height, dbox->x1,dbox->y1,dbox->x2,dbox->y2,
	&x1,&y1, &w,&h);
    w *= DepthInBytes;
    if (w<=0 || h<=0) continue;
	/* convert rectangle to addresses */
    src = PIXER(SrcDrawable)->bits
	+(sx +(x1-dx))*DepthInBytes +(sy +(y1-dy))*src_stride;
    dst = SCRMER(DstDrawable)->bits
		+(x1+dtx)*DepthInBytes +(y1+dty)*dst_stride;
	/* copy rectangle */
    while (h--)
    {
      COPYROW(dst,src,w);
      src += src_stride; dst += dst_stride;	/* move to next row */
    }
  }
}

	/* reverse banding */
static void spud(list,n) BoxRec *list;
{
  register BoxRec box;
  register int i, j, k;

  for (j = 0, i = n, k = n/2; j<k; )
	{ box = list[j]; list[j++] = list[--i]; list[i] = box; }
}

	/* reverse horizontal banding */
static void spudd(list,n) register BoxRec *list;
{
  register int i,j;

  for (j=0; j<n; j = i)
  {
    for (i = j+1; i<n && list[i].y1==list[j].y1; i++) ;
    spud(&list[j],i-j);
  }
}

unsigned long int *Xalloc();
RegionPtr NotClippedByChildren();

RegionPtr hpcCopyArea(pSrcDrawable, pDstDrawable,
	       pGC, xIn, yIn, widthSrc, heightSrc, xOut, yOut)
DrawablePtr pSrcDrawable, pDstDrawable;
GCPtr pGC;
int xIn, yIn, widthSrc, heightSrc, xOut, yOut;
{
  BoxRec *scl, *dcl, sbox;
  int
    srcx,srcy, dstx,dsty, width,height, dboxes,sboxes,
    stx,sty, dtx,dty,
    expose = 0, lowlife = 0, x;
  RegionPtr sHitList = NULL;	/* the source hit list */
  RegionPtr prgnExposed = NULL;
  extern RegionPtr hpfbCopyArea (), mfbCopyArea ();

/* ignore UNDRAWABLE_WINDOWs in the hope DIX takes care of them */

  if ((pDstDrawable->type ==DRAWABLE_WINDOW) && 
	(!((WindowPtr)pDstDrawable)->realized))
  { return NULL; }

  dstx = xOut; dsty = yOut; width = widthSrc; height = heightSrc;
	/* clip the left and top edges of the source */
  if (xIn<0) { expose = 1; srcx = 0; width += xIn; }  else srcx = xIn;
  if (yIn<0) { expose = 1; srcy = 0; height += yIn; } else srcy = yIn;


	/* lookup or create the source clip lists */
  stx = sty = 0;
  if (pSrcDrawable->type ==DRAWABLE_PIXMAP)
  {
	/* clip right and bottom edges of source */
    if (width > ((PixmapPtr)pSrcDrawable)->width)
	{ expose = 1; width = ((PixmapPtr)pSrcDrawable)->width; }
    if (height > ((PixmapPtr)pSrcDrawable)->height)
	{ expose = 1; height = ((PixmapPtr)pSrcDrawable)->height; }
	/* if screen pixmap & going to use the block mover, translate */
    if (DEVKIND(pSrcDrawable) ==PIXMAP_FRAME_BUFFER)
    {
      stx = ((hpChunk *)PIXER(pSrcDrawable)->pChunk)->x;
      sty = ((hpChunk *)PIXER(pSrcDrawable)->pChunk)->y;
    }
    sbox.x2 = (sbox.x1 = srcx) +width; sbox.y2 = (sbox.y1 = srcy) +height;
    scl = &sbox; sboxes = 1;
  }
  else	/* source is a window */
  {
    expose = 1;	/* hard to figure out for a window so expose always */
	/* translate window to screen coordinates */
    srcx += ((WindowPtr)pSrcDrawable)->absCorner.x;
    srcy += ((WindowPtr)pSrcDrawable)->absCorner.y;
    if (pGC->subWindowMode == IncludeInferiors)
    {
	/* included window can write over parent => overlap problem
	 *  (if included window is source and parent is dest)
	 */
      if (pDstDrawable->type ==DRAWABLE_WINDOW) lowlife = 1;
      if (pSrcDrawable==pDstDrawable && pGC->clientClipType ==CT_NONE)
      {
	scl    = ((cfbPrivGCPtr)(pGC->devPriv))->pCompositeClip->rects;
	sboxes = ((cfbPrivGCPtr)(pGC->devPriv))->pCompositeClip->numRects;
      }
      else	/* gotta create a new clip list */
      {
	sHitList = NotClippedByChildren((WindowPtr)pSrcDrawable);
	scl = sHitList->rects; sboxes = sHitList->numRects;
      }
    }
    else
    {
      scl    = ((WindowPtr)pSrcDrawable)->clipList->rects;
      sboxes = ((WindowPtr)pSrcDrawable)->clipList->numRects;
    }
  }

	/* lookup the dest clip list and any translation */
  dcl    = ((cfbPrivGCPtr)(pGC->devPriv))->pCompositeClip->rects;
  dboxes = ((cfbPrivGCPtr)(pGC->devPriv))->pCompositeClip->numRects;
  dtx = dty = 0;
  if (pDstDrawable->type ==DRAWABLE_PIXMAP)
  {
    if (DEVKIND(pDstDrawable) ==PIXMAP_FRAME_BUFFER)
    {
      dtx = ((hpChunk *)PIXER(pDstDrawable)->pChunk)->x;
      dty = ((hpChunk *)PIXER(pDstDrawable)->pChunk)->y;
    }
    /* else dest is in main mem & composite clip is OK */
  }
  else	/* dest is a window */
  {
    if (pGC->miTranslate) /* translate window to screen coordinates */
    {
      dstx += ((WindowPtr)pDstDrawable)->absCorner.x;
      dsty += ((WindowPtr)pDstDrawable)->absCorner.y;
    }
  }
  
	/* figure out who to call to actually do the copy area */
  if (pDstDrawable->type ==DRAWABLE_PIXMAP &&
      DEVKIND(pDstDrawable) ==PIXMAP_HOST_MEMORY)		/* MTOM */
  {
    if (pSrcDrawable->depth==1)	/* can only handle some bitmaps */
    {
      if ((pSrcDrawable!=pDstDrawable ||
	   !lapdog(srcx,srcy, width,height, dstx,dsty, sboxes+dboxes)) &&
	  srcx==0 && srcy==0 && dstx==0 && dsty==0 && (width % 8)==0)
      {
	width /= 8; goto mtom;
      }
		/* let mfb handle most of the bit maps */
      return mfbCopyArea(pSrcDrawable, pDstDrawable,
	pGC, xIn, yIn, widthSrc, heightSrc, xOut, yOut);
    }
	/* can't handle overlapping moves unless overlap don't matter */
    if (pSrcDrawable==pDstDrawable &&
	lapdog(srcx,srcy,width,height,dstx,dsty,sboxes+dboxes) &&
	(SWEAT_PLANES_MASK(pSrcDrawable,pGC->planemask) ||
	 (pGC->alu!=GXclear && pGC->alu!=GXset && pGC->alu!=GXnoop)) )
    {
      return hpfbCopyArea(pSrcDrawable, pDstDrawable,
	pGC, xIn, yIn, widthSrc, heightSrc, xOut, yOut);
    }
  mtom:
    MemToMem(pSrcDrawable,pDstDrawable,pGC,
	srcx,srcy,width,height, dstx,dsty, scl,sboxes, dcl,dboxes);
  }
  else
    if (pSrcDrawable->type ==DRAWABLE_PIXMAP &&
      DEVKIND(pSrcDrawable) ==PIXMAP_HOST_MEMORY)		/* MTOS */
	MemToScreen(pSrcDrawable,pDstDrawable,pGC,
	  srcx,srcy,width,height, dstx,dsty, dboxes,dcl, dtx,dty);
    else							/* STOS */
      if ( (pSrcDrawable==pDstDrawable || lowlife) &&
	   (sboxes +dboxes >2) &&
	   lapdog(srcx,srcy, width,height, dstx,dsty, sboxes+dboxes) &&
	   (srcy<dsty || srcx<dstx) )			/* overlap */
      {
	BoxRec *sl, *dl;
	int i,j;

	sl = (BoxRec *)(Xalloc((unsigned long)	/* allocate new clip lists */
		((sboxes+dboxes)*sizeof(BoxRec))));
	dl = &sl[sboxes];
        for (j=0; j<sboxes; j++) sl[j] = scl[j];
        for (j=0; j<dboxes; j++) dl[j] = dcl[j];
		/* reverse vertical & horizontal banding */
	if (srcy<dsty || (srcy<dsty && srcx<dstx))
	  { spud(sl,sboxes); spud(dl,dboxes); }
	else		/* reverse horizontal banding */
	  { spudd(sl,sboxes); spudd(dl,dboxes); }
	ScreenToScreen(pSrcDrawable, pGC, srcx,srcy, width,height, dstx,dsty,
		sl,sboxes, dl,dboxes, stx,sty, dtx,dty);
	xfree((char *)sl);
      }
      else
	ScreenToScreen(pSrcDrawable, pGC, srcx,srcy, width,height, dstx,dsty,
		scl,sboxes, dcl,dboxes, stx,sty, dtx,dty);

	/* let miHandleExposures() handle all the exposure stuff 'cause
	 *   it knows lots more than I do.  It also sends noExpose
	 *   events if needbe.
	 * Note: Other CopyAreas use (cfbPrivGC *)(pGC->devPriv))->fExpose
	 *   instead of pGC->graphicsExposures.  This is because
	 *   mfbPutImage is brain damaged and since HP don't use it, I
	 *   can use graphicsExposures.
	 */
  if (pGC->graphicsExposures)
	prgnExposed = miHandleExposures(pSrcDrawable, pDstDrawable, pGC, xIn, yIn,
		widthSrc, heightSrc, xOut, yOut,0);
  if (sHitList) (*pGC->pScreen->RegionDestroy)(sHitList);
  return prgnExposed;
}
