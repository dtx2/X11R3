/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/* incls-ed on 7-17 */

#include <sys/types.h>

#include "X.h"
#include "gcstruct.h"

/*
 * driver headers
 */
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"


#include "qd.h"

#include "tl.h"
#include "tltemplabels.h"

/*
 *  screen to screen move
 */

/*
   the dma buffer has
   JMPT_SETVIPER24
   rop | FULL_SRC_RESOLUTION
   JMPT_INITBITBLT
   srcdx,srcdy
   srcx,srcy,dstx,dsty,w,h
   {clipx_min,clipx_max,clipy_min,clipy_max,}*
   JMPT_BITBLTDONE
srxdx and srcdy are loaded only for the sign bit.
*/

/* must be passed absolute dst{x,y}.  no translation! */
tlbitblt( pGC, dstX, dstY, dstW, dstH, srcX, srcY)
    GCPtr	pGC;
    int		dstX, dstY;
    int		dstW;
    register int dstH;	/* used in inner loop */
    int	srcX, srcY;
{
    RegionPtr	pgcclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    register unsigned short     *p;
    register BoxPtr	prect = pgcclip->rects;
    register int	nrects = pgcclip->numRects;
    int		x1clip, y1clip, x2clip, y2clip;
    int		xoff, yoff;
    int		iclip;	/* index into clip rect list */
    int		maxrects = MAXDMAWORDS/4;
    register int	nboxesThisTime;

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT(0,0);
    if (nrects == 0)
	return;

    /*
     * The X toolkit passes in outrageously big rectangles to be blitted,
     * using clipping to restrict the blit to a reasonable size.
     * This is slow on the adder, which generates all the clipped-out
     * pixel addresses, so we soft clip the rectangle.
     */
    x1clip = max( dstX, pgcclip->extents.x1);
    y1clip = max( dstY, pgcclip->extents.y1);
    x2clip = min( dstX+dstW, pgcclip->extents.x2);
    y2clip = min( dstY+dstH, pgcclip->extents.y2);

    srcX += x1clip-dstX;
    srcY += y1clip-dstY;
    dstX = x1clip;
    dstY = y1clip;
    dstW = x2clip-x1clip;
    dstH = y2clip-y1clip;

    yoff = dstY - srcY;
    xoff = dstX - srcX;
    if ((yoff > 0) && (yoff < dstH) && (abs(xoff) < dstW)) {
	/* reverse y scan direction */
	srcY += dstH-1;
	dstY += dstH-1;
	dstH = (-dstH);
	prect += nrects-1;
    }
    else if ((yoff == 0) && (xoff > 0) && (xoff < dstW)) {
	/* reverse x scan direction */
	srcX += dstW-1;
	dstX += dstW-1;
	dstW = (-dstW);
    }

    Need_dma(15);	/* base initialization */

    *p++ = JMPT_SETRGBPLANEMASK;
    *p++ = RED(pGC->planemask);
    *p++ = GREEN(pGC->planemask);
    *p++ = BLUE(pGC->planemask);
    *p++ = JMPT_SETVIPER24;
    *p++ = umtable[(pGC->alu)] | FULL_SRC_RESOLUTION;
    *p++ = JMPT_INITBITBLT;
    *p++ = dstW & 0x3fff;
    *p++ = dstH & 0x3fff;
    *p++ = srcX & 0x3fff;
    *p++ = srcY & 0x3fff;
    *p++ = dstX & 0x3fff;
    *p++ = dstY & 0x3fff;
    *p++ = dstW & 0x3fff;
    *p++ = dstH & 0x3fff;
    Confirm_dma();
    for (   nrects = pgcclip->numRects;
	    nrects>0;
	    nrects -= min( nrects, maxrects))
    {
	nboxesThisTime = min( nrects, maxrects);

	Need_dma( nboxesThisTime<<2);
	while ( nboxesThisTime-- > 0)
	{
	    *p++ = prect->x1;	/* always non-negative */
	    *p++ = prect->x2;	/*   "	*/
	    *p++ = prect->y1;	/*   "	*/
	    *p++ = prect->y2;	/*   "	*/
	    if ( dstH < 0)	/* assume dstH is on only negative if inverted
				   in this routine */
		prect--;
	    else
		prect++;
	}
	Confirm_dma();
    }
}
