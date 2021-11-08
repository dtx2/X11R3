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

/*
 * Expands a bitmap in the frame buffer to a full depth pixmap elsewhere
 * in the frame buffer.
 * The full depth pixels are either set to
 * foreground/destination (tlPlaneStipple)
 * or foreground/background (tlPlaneCopy).
 *
 * Calling sequence is the same as that of tlbitblt, except for appending
 * of a planemask argument.
 * tlOddSize() depends on this congruence,
 * as it uses a function pointer to point to either of them.
 */
#include "X.h"
#include "gcstruct.h"
#include "windowstr.h"

#include <sys/types.h>
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"

#include "qd.h"

#include "tltemplabels.h"
#include "tl.h"

#ifdef TWOCOLOR
# define PROCNAME tlPlaneCopy
# define QPROCNAME "tlPlaneCopy"	/* quoted procedure name */
#else
# define PROCNAME tlPlaneStipple
# define QPROCNAME "tlPlaneStipple"
#endif

void
PROCNAME( pGC, srcx, srcy, width, height, dstx, dsty, srcplane)
    GCPtr               pGC;
    int                 srcx, srcy;
    int                 width, height;
    int                 dstx, dsty;
    unsigned long       srcplane;
{
    RegionPtr	pgcclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    int		nshortcolor;
    int		x1clip, y1clip, x2clip, y2clip;
    int		nc;
    BoxPtr	pc;
    unsigned short *p;

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT(0,0);	/* this ok?  abs dst{x,y} assumed. */
    /*
     * soft clip the destination rectangle to the GC clip extents.
     */
    x1clip = max( dstx, pgcclip->extents.x1);
    y1clip = max( dsty, pgcclip->extents.y1);
    x2clip = min( dstx+width, pgcclip->extents.x2);
    y2clip = min( dsty+height, pgcclip->extents.y2);

    srcx += x1clip-dstx;
    srcy += y1clip-dsty;
    dstx = x1clip;
    dsty = y1clip;
    width = x2clip-x1clip;
    height = y2clip-y1clip;

    if (     pgcclip->numRects <= 0
	  || width <= 0
	  || height <= 0)
	return;

#if 1 /* NPLANES==24 */
# ifdef TWOCOLOR
    nshortcolor = ( 4  + 2*NCOLOR24SHORTS);
# else
    nshortcolor = ( 5  + 1*NCOLOR24SHORTS);
# endif
#else /* NPLANES==8 */
# ifdef TWOCOLOR
    nshortcolor = ( 3  + 2*NCOLORSHORTS);
# else
    nshortcolor = ( 3  + 1*NCOLORSHORTS);
# endif
#endif

    Need_dma(nshortcolor+9);
#if 1 /*NPLANES==24*/
# ifdef TWOCOLOR
    *p++ = JMPT_SETV24OPAQUE;
    *p++ = RED( pGC->planemask );
    *p++ = GREEN( pGC->planemask );
    *p++ = BLUE( pGC->planemask );
    SET24COLOR( p, ( pGC->fgPixel));
    SET24COLOR( p, ( pGC->bgPixel));
# else
    *p++ = JMPT_SETV24TRANS;
    *p++ = RED( pGC->planemask );
    *p++ = GREEN( pGC->planemask );
    *p++ = BLUE( pGC->planemask );
    *p++ = umtable[ pGC->alu] | FULL_SRC_RESOLUTION;
    SET24COLOR( p, ( pGC->fgPixel));
# endif

#else /* NPLANES==8 */

# ifdef TWOCOLOR
    *p++ = JMPT_SETVIPER;
    *p++ = umtable[ GXcopy] | FULL_SRC_RESOLUTION;
    *p++ = JMPT_SETFOREBACKCOLOR;
    SETCOLOR( p, pGC->fgPixel);
    SETCOLOR( p, pGC->bgPixel);
# else
    *p++ = JMPT_SETVIPER;
    *p++ = umtable[ pGC->alu] | FULL_SRC_RESOLUTION;
    *p++ = JMPT_SETCOLOR;
    SETCOLOR( p, pGC->fgPixel);
# endif
#endif

#ifdef TWOCOLOR
    *p++ = JMPT_INIT2COLORBITMAP;
#else
    *p++ = JMPT_INIT1COLORBITMAP;
#endif
    *p++ = srcplane;
    *p++ = srcx & 0x3fff;
    *p++ = srcy & 0x3fff;
    *p++ = dstx & 0x3fff;
    *p++ = dsty & 0x3fff;
    *p++ = width;
    *p++ = height;
    *p++ = JMPT_COLORBITMAP;
    Confirm_dma();
    for (   nc=pgcclip->numRects, pc=pgcclip->rects;
	    nc>0;
	    nc--, pc++)
    {
	Need_dma(4);
	*p++ = pc->x1 & 0x3fff;
	*p++ = pc->x2 & 0x3fff;
	*p++ = pc->y1 & 0x3fff;
	*p++ = pc->y2 & 0x3fff;
	Confirm_dma();
    }
}
