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
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/


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

#include "X.h"
#include "Xprotostr.h"

#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "cfb.h"
#include "cfbmskbits.h"


/* CopyArea and CopyPlane for a color frame buffer


    clip the source rectangle to the source's available bits.  (this
avoids copying unnecessary pieces that will just get exposed anyway.)
this becomes the new shape of the destination.
    clip the destination region to the composite clip in the
GC.  this requires translating the destination region to (dstx, dsty).
    build a list of source points, one for each rectangle in the
destination.  this is a simple translation.
    go do the multiple rectangle copies
    do graphics exposures
*/

/* macro for bitblt to avoid a switch on the alu per scanline 
   comments are in the real code in cfbDoBitblt.
   we need tmpDst for things less than 1 word wide becuase
the destination may cross a word boundary, and we need to read
it all at once to do the rasterop.  (this perhaps argues for
sub-casing narrow things that don't cross a word boundary.)
*/
#define DOBITBLT(ALU) \
while (nbox--) \
{ \
    w = pbox->x2 - pbox->x1; \
    h = pbox->y2 - pbox->y1; \
    if (ydir == -1) \
    { \
        psrcLine = psrcBase + ((pptSrc->y+h-1) * -widthSrc); \
        pdstLine = pdstBase + ((pbox->y2-1) * -widthDst); \
    } \
    else \
    { \
        psrcLine = psrcBase + (pptSrc->y * widthSrc); \
        pdstLine = pdstBase + (pbox->y1 * widthDst); \
    } \
    if (w <= PPW) \
    { \
	int tmpDst; \
        int srcBit, dstBit; \
        pdstLine += (pbox->x1 >> PWSH); \
        psrcLine += (pptSrc->x >> PWSH); \
        psrc = psrcLine; \
        pdst = pdstLine; \
        srcBit = pptSrc->x & PIM; \
        dstBit = pbox->x1 & PIM; \
        while(h--) \
        { \
	    getbits(psrc, srcBit, w, tmpSrc) \
	    getbits(pdst, dstBit, w, tmpDst) \
	    tmpSrc = ALU(tmpSrc, tmpDst); \
/*XXX*/	    putbits(tmpSrc, dstBit, w, pdst, -1) \
	    pdst += widthDst; \
	    psrc += widthSrc; \
        } \
    } \
    else \
    { \
        register int xoffSrc; \
        int nstart; \
        int nend; \
        int srcStartOver; \
        maskbits(pbox->x1, w, startmask, endmask, nlMiddle) \
        if (startmask) \
	    nstart = PPW - (pbox->x1 & PIM); \
        else \
	    nstart = 0; \
        if (endmask) \
            nend = pbox->x2 & PIM; \
        else \
	    nend = 0; \
        xoffSrc = ((pptSrc->x & PIM) + nstart) & PIM; \
        srcStartOver = ((pptSrc->x & PIM) + nstart) > PLST; \
        if (xdir == 1) \
        { \
            pdstLine += (pbox->x1 >> PWSH); \
            psrcLine += (pptSrc->x >> PWSH); \
	    while (h--) \
	    { \
	        psrc = psrcLine; \
	        pdst = pdstLine; \
	        if (startmask) \
	        { \
		    getbits(psrc, (pptSrc->x & PIM), nstart, tmpSrc) \
		    tmpSrc = ALU(tmpSrc, *pdst); \
/*XXX*/		    putbits(tmpSrc, (pbox->x1 & PIM), nstart, pdst, -1) \
		    pdst++; \
		    if (srcStartOver) \
		        psrc++; \
	        } \
	        nl = nlMiddle; \
	        while (nl--) \
	        { \
		    getbits(psrc, xoffSrc, PPW, tmpSrc) \
		    *pdst = ALU(tmpSrc, *pdst); \
		    pdst++; \
		    psrc++; \
	        } \
	        if (endmask) \
	        { \
		    getbits(psrc, xoffSrc, nend, tmpSrc) \
		    tmpSrc = ALU(tmpSrc, *pdst); \
/*XXX*/		    putbits(tmpSrc, 0, nend, pdst, -1) \
	        } \
	        pdstLine += widthDst; \
	        psrcLine += widthSrc; \
	    } \
        } \
        else  \
        { \
            pdstLine += (pbox->x2 >> PWSH); \
            psrcLine += (pptSrc->x+w >> PWSH); \
	    if (xoffSrc + nend >= PPW) \
	        --psrcLine; \
	    while (h--) \
	    { \
	        psrc = psrcLine; \
	        pdst = pdstLine; \
	        if (endmask) \
	        { \
		    getbits(psrc, xoffSrc, nend, tmpSrc) \
		    tmpSrc = ALU(tmpSrc, *pdst); \
/*XXX*/		    putbits(tmpSrc, 0, nend, pdst, -1) \
	        } \
	        nl = nlMiddle; \
	        while (nl--) \
	        { \
		    --psrc; \
		    getbits(psrc, xoffSrc, PPW, tmpSrc) \
		    --pdst; \
		    *pdst = ALU(tmpSrc, *pdst); \
	        } \
	        if (startmask) \
	        { \
		    if (srcStartOver) \
		        --psrc; \
		    --pdst; \
		    getbits(psrc, (pptSrc->x & PIM), nstart, tmpSrc) \
		    tmpSrc = ALU(tmpSrc, *pdst); \
/*XXX*/		    putbits(tmpSrc, (pbox->x1 & PIM), nstart, pdst, -1) \
	        } \
	        pdstLine += widthDst; \
	        psrcLine += widthSrc; \
	    } \
        } \
    } \
    pbox++; \
    pptSrc++; \
}


/* #include "salloc.h" /* !!! tanks the optimizer */

/* DoBitblt() does multiple rectangle moves into the rectangles
   DISCLAIMER:
   this code can be made much faster; this implementation is
designed to be independent of byte/bit order, processor
instruction set, and the like.  it could probably be done
in a similarly device independent way using mask tables instead
of the getbits/putbits macros.  the narrow case (w<32) can be
subdivided into a case that crosses word boundaries and one that
doesn't.

   we have to cope with the dircetion on a per band basis,
rather than a per rectangle basis.  moving bottom to top
means we have to invert the order of the bands; moving right
to left requires reversing the order of the rectangles in
each band.

   if src or dst is a window, the points have already been
translated.
*/


/*
 * magic macro for copying longword aligned regions -
 * can easily be replaced with bcopy (from, to, count * 4)
 * with some reduction in performance...
 */

#ifdef vax /* or rather, if it has fast bcopy */
#define longcopy(from,to,count)\
{ \
      bcopy((char *) (from),(char *) (to),(count)<<2); \
      (from) += (count); \
      (to) += (count); \
}
#else /* else not vax; doesn't have fast bcopy */
/* we want to make sure that this stays in the cache */
#define longcopy(from,to,count)    \
{ \
    switch (count & 7) { \
          case 0:   *to++ = *from++; \
          case 7:   *to++ = *from++; \
          case 6:   *to++ = *from++; \
          case 5:   *to++ = *from++; \
          case 4:   *to++ = *from++; \
          case 3:   *to++ = *from++; \
          case 2:   *to++ = *from++; \
          case 1:   *to++ = *from++; \
    } \
    while ((count -= 8) > 0) { \
          *to++ = *from++; \
          *to++ = *from++; \
          *to++ = *from++; \
          *to++ = *from++; \
          *to++ = *from++; \
          *to++ = *from++; \
          *to++ = *from++; \
          *to++ = *from++; \
    } \
}
#endif /* vax */

cfbDoBitblt(pSrcDrawable, pDstDrawable, alu, prgnDst, pptSrc)
DrawablePtr pSrcDrawable;
DrawablePtr pDstDrawable;
int alu;
RegionPtr prgnDst;
DDXPointPtr pptSrc;
{
    int *psrcBase, *pdstBase;	/* start of src and dst bitmaps */
    int widthSrc, widthDst;	/* add to get to same position in next line */

    register BoxPtr pbox;
    int nbox;

    BoxPtr pboxTmp, pboxNext, pboxBase, pboxNewX, pboxNewY;
				/* temporaries for shuffling rectangles */
    DDXPointPtr pptTmp, pptNewX, pptNewY; /* shuffling boxes entails shuffling the
					     source points too */
    int w, h;
    int xdir;			/* 1 = left right, -1 = right left/ */
    int ydir;			/* 1 = top down, -1 = bottom up */

    int *psrcLine, *pdstLine;	/* pointers to line with current src and dst */
    register int *psrc;		/* pointer to current src longword */
    register int *pdst;		/* pointer to current dst longword */

				/* following used for looping through a line */
    int startmask, endmask;	/* masks for writing ends of dst */
    int nlMiddle;		/* whole longwords in dst */
    register int nl;		/* temp copy of nlMiddle */
    register int tmpSrc;	/* place to store full source word */

    if (pSrcDrawable->type == DRAWABLE_WINDOW)
    {
	psrcBase = (int *)
	 (((cfbPrivScreenPtr)(pSrcDrawable->pScreen->devPrivate))->bits);
	widthSrc = (int)
	   ((cfbPrivScreenPtr)(pSrcDrawable->pScreen->devPrivate))->stride
	       >> 2;
    }
    else
    {
	psrcBase = (int *)
	   (((cfbPrivPixmapPtr)(((PixmapPtr)pSrcDrawable)->devPrivate))->bits);
	widthSrc = (int)
	   (((cfbPrivPixmapPtr)(((PixmapPtr)pSrcDrawable)->devPrivate))->stride)
		>> 2;
    }

    if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (int *)
	 (((cfbPrivScreenPtr)(pDstDrawable->pScreen->devPrivate))->bits);
	widthDst = (int)
	   (((cfbPrivScreenPtr)(pDstDrawable->pScreen->devPrivate))->stride)
	       >> 2;
    }
    else
    {
	pdstBase = (int *)
	   (((cfbPrivPixmapPtr)(((PixmapPtr)pDstDrawable)->devPrivate))->bits);
	widthDst = (int)
	   (((cfbPrivPixmapPtr)(((PixmapPtr)pDstDrawable)->devPrivate))->stride)
		>> 2;
    }

    pbox = prgnDst->rects;
    nbox = prgnDst->numRects;

    pboxNewX = NULL;
    pboxNewY = NULL;
    pptNewX = NULL;
    pptNewY = NULL;
    if (pptSrc->y < pbox->y1) 
    {
        /* walk source botttom to top */
	ydir = -1;
	widthSrc = -widthSrc;
	widthDst = -widthDst;

	if (nbox > 1)
	{
	    /* keep ordering in each band, reverse order of bands */
#if SAOK
	    SALLOC(sizeof(BoxRec) * nbox); pboxNewY = (BoxPtr)SADDR;
	    SALLOC(sizeof(DDXPointRec)*nbox); pptNewY = (DDXPointPtr)SADDR;
#else
	    pboxNewY = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    if(!pboxNewY) return;
	    pptNewY = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pptNewY)  { DEALLOCATE_LOCAL(pboxNewY); return; }
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
    else
    {
	/* walk source top to bottom */
	ydir = 1;
    }

    if (pptSrc->x < pbox->x1)
    {
	/* walk source right to left */
        xdir = -1;

	if (nbox > 1)
	{
	    /* reverse order of rects ineach band */
#if SAOK
	    SALLOC(sizeof(BoxRec) * nbox); pboxNewX = (BoxPtr)SADDR;
	    SALLOC(sizeof(DDXPointRec) * nbox); pptNewX = (DDXPointPtr)SADDR;
#else
	    pboxNewX = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    pptNewX = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pboxNewX || !pptNewX)
	    {
		if (pptNewX) DEALLOCATE_LOCAL(pptNewX);
		if (pboxNewX) DEALLOCATE_LOCAL(pboxNewX);
		if (pboxNewY)
		  { DEALLOCATE_LOCAL(pptNewY); DEALLOCATE_LOCAL(pboxNewY); }
	        return NULL;
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
    else
    {
	/* walk source left to right */
        xdir = 1;
    }


    WAIT_READY_TO_RENDER(pSrcDrawable->pScreen);
    SET_REGISTERS_FOR_WRITING(pDstDrawable->pScreen, ~0, GXcopy);

    /*
     * XXX: This should be a switch so good compilers could do a calculated
     * jump, these constants all being in order. Unfortunately, Sun's 3.2
     * compiler can't handle this one because it's so huge, so the offsets
     * won't fit properly in 16 bits.
     */
    if (alu == GXcopy) {
	/*
	 * special case copy, to avoid some redundant moves
	 * into temporaries
	 */
	while (nbox--)
	{
	    w = pbox->x2 - pbox->x1;
	    h = pbox->y2 - pbox->y1;
	    
	    if (ydir == -1) /* start at last scanline of rectangle */
	    {
		psrcLine = psrcBase + ((pptSrc->y+h-1) * -widthSrc);
		pdstLine = pdstBase + ((pbox->y2-1) * -widthDst);
	    }
	    else /* start at first scanline */
	    {
		psrcLine = psrcBase + (pptSrc->y * widthSrc);
		pdstLine = pdstBase + (pbox->y1 * widthDst);
	    }
	    
	    /* x direction doesn't matter for < 1 longword */
	    if (w <= PPW)
	    {
		int srcBit, dstBit;	/* bit offset of src and dst */
		
		pdstLine += (pbox->x1 >> PWSH);
		psrcLine += (pptSrc->x >> PWSH);
		psrc = psrcLine;
		pdst = pdstLine;
		
		srcBit = pptSrc->x & PIM;
		dstBit = pbox->x1 & PIM;
		
		while(h--)
		{
		    getbits(psrc, srcBit, w, tmpSrc);
		    putbits(tmpSrc, dstBit, w, pdst, -1);
		    pdst += widthDst;
		    psrc += widthSrc;
		}
	    }
	    else
	    {
		register int xoffSrc;	/* offset (>= 0, < 32) from
					 * which to fetch whole
					 * longwords fetched in src */
		int nstart;		/* number of ragged bits 
					   at start of dst */
		int nend;		/* number of ragged bits at end 
					   of dst */
		int srcStartOver;	/* pulling nstart bits from src
					   overflows into the next word? */
		
		maskbits(pbox->x1, w, startmask, endmask, nlMiddle);
		if (startmask)
		    nstart = PPW - (pbox->x1 & PIM);
		else
		    nstart = 0;
		if (endmask)
		    nend = pbox->x2 & PIM;
		else
		    nend = 0;
		
		xoffSrc = ((pptSrc->x & PIM) + nstart) & PIM;
		srcStartOver = ((pptSrc->x & PIM) + nstart) > PLST;
		
		if (xdir == 1) /* move left to right */
		{
		    pdstLine += (pbox->x1 >> PWSH);
		    psrcLine += (pptSrc->x >> PWSH);
		    
		    while (h--)
		    {
			psrc = psrcLine;
			pdst = pdstLine;
			
			if (startmask)
			{
			    getbits(psrc, (pptSrc->x&PIM), nstart, tmpSrc);
			    putbits(tmpSrc, (pbox->x1 & PIM), nstart, pdst, 
				    -1);
			    pdst++;
			    if (srcStartOver)
				psrc++;
			}
			
                        /* special case for aligned copies (scrolling) */
                        if (xoffSrc == 0)
		        {
			  
                            if ((nl = nlMiddle) != 0)
			    {
                                longcopy (psrc, pdst, nl)
			    }
                        }
                        else
		        {
                            nl = nlMiddle + 1;
                            while (--nl)
			    {
                                getbits (psrc, xoffSrc, PPW, *pdst++);
                                psrc++;
                            }
                        }

			if (endmask)
			{
			    getbits(psrc, xoffSrc, nend, tmpSrc);
			    putbits(tmpSrc, 0, nend, pdst, -1);
			}
			
			pdstLine += widthDst;
			psrcLine += widthSrc;
		    }
		}
		else /* move right to left */
		{
		    pdstLine += (pbox->x2 >> PWSH);
		    psrcLine += (pptSrc->x+w >> PWSH);
		    /* if fetch of last partial bits from source crosses
		       a longword boundary, start at the previous longword
		       */
		    if (xoffSrc + nend >= PPW)
			--psrcLine;
		    
		    while (h--)
		    {
			psrc = psrcLine;
			pdst = pdstLine;
			
			if (endmask)
			{
			    getbits(psrc, xoffSrc, nend, tmpSrc);
			    putbits(tmpSrc, 0, nend, pdst, -1);
			}
			
			nl = nlMiddle;
			while (nl--)
			{
			    --psrc;
			    getbits(psrc, xoffSrc, PPW, tmpSrc);
			    *--pdst = tmpSrc;
			}
			
			if (startmask)
			{
			    if (srcStartOver)
				--psrc;
			    --pdst;
			    getbits(psrc, (pptSrc->x&PIM), nstart, tmpSrc);
			    putbits(tmpSrc, (pbox->x1&PIM), nstart, pdst, 
				    -1);
			}
			
			pdstLine += widthDst;
			psrcLine += widthSrc;
		    }
		} /* move right to left */
	    }
	    pbox++;
	    pptSrc++;
	} /* while (nbox--) */
    } else if (alu == GXclear) {
	DOBITBLT(fnCLEAR);
    } else if (alu == GXand) {
	DOBITBLT(fnAND);
    } else if (alu == GXandReverse) {
	DOBITBLT(fnANDREVERSE);
    } else if (alu == GXandInverted) {
	DOBITBLT(fnANDINVERTED);
    } else if (alu == GXxor) {
	DOBITBLT(fnXOR);
    } else if (alu == GXor) {
	DOBITBLT(fnOR);
    } else if (alu == GXnor) {
	DOBITBLT(fnNOR);
    } else if (alu == GXequiv) {
	DOBITBLT(fnEQUIV);
    } else if (alu == GXinvert) {
	DOBITBLT(fnINVERT);
    } else if (alu == GXorReverse) {
	DOBITBLT(fnORREVERSE);
    } else if (alu == GXcopyInverted) {
	DOBITBLT(fnCOPYINVERTED);
    } else if (alu == GXorInverted) {
	DOBITBLT(fnORINVERTED);
    } else if (alu == GXnand) {
	DOBITBLT(fnNAND);
    } else if (alu == GXset) {
	DOBITBLT(fnSET);
    }
#if !SAOK
    if (pboxNewX) { DEALLOCATE_LOCAL(pptNewX); DEALLOCATE_LOCAL(pboxNewX); }
    if (pboxNewY) { DEALLOCATE_LOCAL(pptNewY); DEALLOCATE_LOCAL(pboxNewY); }
#endif
}
