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

#if NPLANES==8
#define GETGREENBYTE( p)	((p)<<8)
#else
#define GETGREENBYTE( p)        (p)
#endif

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
#include <sys/types.h>
#include "gcstruct.h"
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"

#include "qd.h"

#include "tltemplabels.h"
#include "tl.h"


#if 1 /* if NPLANES==24 */ 
# ifdef BICHROME
#  define PTOBOVERHEAD	 (NMASK24SHORTS + 12 + 4 + 2*NCOLOR24SHORTS)
# else
#  define PTOBOVERHEAD	 (NMASK24SHORTS + 12 + 5 + 1*NCOLOR24SHORTS)
# endif
#else /* NPLANES==8 */
# ifdef BICHROME
#  define PTOBOVERHEAD	 (NMASKSHORTS + 12 + 3 + 2*NCOLORSHORTS)
# else
#  define PTOBOVERHEAD	 (NMASKSHORTS + 12 + 3 + 1*NCOLORSHORTS)
# endif
#endif

/*
 * Uses PTOBXY to broadcast a bitmap in the DMA queue to all planes
 *
 * Dragon clipping is set from a single box argument, not from the GC
 */
VOID
#ifdef BICHROME
 tlBitmapBichrome
#else
 tlBitmapStipple
#endif
	( pGC, qbit, fore, back, x0, y0, box)
    GCPtr	pGC;
    PixmapPtr	qbit;		/* x11 bitmap, with the mfb conventions */
    int		fore;
    int		back;		/* used in tlTileBichrome only */
    int		x0, y0;
    BoxPtr	box;		/* clipping */
 {
    unsigned short *	pshort;
    int			slwidthshorts;
    int 		nscansperblock;
    int 		nscansdone;

    extern int     req_buf_size;
    void doBitmap();

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT( 0, 0);
    /*
     * dest scanline may be one short wider than source
     */
    pshort = (unsigned short *)
	alloca( (qbit->devKind+2) * qbit->height); /* upper bound */ 

    /*
     * bitmap needs to be right-shifted to the same modulo-16 boundary
     * as x0
     */
    slwidthshorts = bitmapShiftRight( qbit->devPrivate, pshort,
			qbit->devKind>>1, qbit->width, qbit->height, x0&0xf);

    /*
     * Flush ensures complete buffer available; Save 50 words
     * for setup display list
     */
    dmafxns.flush (TRUE);
    nscansperblock = ( min( req_buf_size, MAXDMAPACKET / sizeof(short))
		     - 50 - PTOBOVERHEAD - slwidthshorts) / slwidthshorts;

    for (   nscansdone=0;
            qbit->height-nscansdone > nscansperblock;
            nscansdone+=nscansperblock, y0+=nscansperblock,
                                    pshort += nscansperblock*slwidthshorts)
        doBitmap( pGC, fore, back, x0, y0, box,
		qbit->width, nscansperblock,
		slwidthshorts, pshort);
    doBitmap( pGC, fore, back, x0, y0, box,
		qbit->width, qbit->height-nscansdone,
		slwidthshorts, pshort);
}

static void
doBitmap( pGC, fore, back, x0, y0, box, width, height, slwidthshorts, pshort)
    GCPtr	pGC;
    int		fore;
    int		back;		/* used in tlTileBichrome only */
    int		x0, y0;
    BoxPtr	box;		/* clipping */
    int		width;
    int		height;
    int		slwidthshorts;
    register unsigned short *	pshort;
{
    register unsigned	 	nshort;
    register unsigned short *p;
    register struct DMAreq *	pRequest;

    /*
     * dest scanline may be one short wider than source
     */

    nshort = slwidthshorts * height;

    /*
     * I believe this may all have to fit in one DMA partition,
     * because we program the dragon to do PTOB
     */
    Need_dma( PTOBOVERHEAD + nshort + 1);

#if 1 /* if NPLANES==24 */ 
# ifdef BICHROME
    *p++ = JMPT_SETV24OPAQUE;
    *p++ = RED( pGC->planemask );
    *p++ = GREEN( pGC->planemask );
    *p++ = BLUE( pGC->planemask );
    SET24COLOR( p, GETGREENBYTE( fore))
    SET24COLOR( p, GETGREENBYTE( back))
# else
    *p++ = JMPT_SETV24TRANS;
    *p++ = RED( pGC->planemask );
    *p++ = GREEN( pGC->planemask );
    *p++ = BLUE( pGC->planemask );
    *p++ = umtable[ pGC->alu] | FULL_SRC_RESOLUTION;
    SET24COLOR( p, GETGREENBYTE( fore))
# endif
#else /* NPLANES==8 */
# if BICHROME
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

    *p++ = JMPT_SETCLIP;
    *p++ = max( box->x1, x0);	/* clip to intersection of dest and arg clip */
    *p++ = min( box->x2, x0+width);
    *p++ = max( box->y1, y0);
    *p++ = min( box->y2, y0+height);
    *p++ = JMPT_PTOBXY24;
#if 1 /* NPLANES==24 */
    SET24PMASK( p, pGC->planemask )	/* 3 shorts for qZss. */
#else /* NPLANES==8 */
    SETPMASK( p, pGC->planemask )	/* 1 short for qd */
#endif
    *p++ = x0 & 0x03fff;
    *p++ = y0 & 0x03fff;
    *p++ = width & 0x03fff;
    *p++ = height & 0x03fff;
    /*
     * DGA magic bit pattern for PTB, see VCB02 manual pp.3-117
     */
    *p++ = 0x06000 | (0x01fff & -nshort);
    bcopy( pshort, p, nshort<<1);
    p += nshort;
    *p++ = JMPT_PTOBXY24CLEAN;
    Confirm_dma ();
}

#ifdef BICHROME		/* only one instance of bitmapShiftRight in library */
/*
 * Shift "right" in the frame buffer sense.  This is an algebraic left shift.
 * Pad each scan line of the destination to an integral number of shorts.
 * Source is known to be padded to an integral number of longwords.
 */
int
bitmapShiftRight( psrc, pdst, srcShorts, width, height, nbits)
    unsigned short *	psrc;
    unsigned short *	pdst;
    int			srcShorts;	/* source width in shorts */
    int			width;
    int			height;
    int			nbits;	/* number of bit places to shift: 0-15 */
{
    int		ir;	/* row index */
    int		ids;	/* index of destination short */
    int		dstShorts;

    dstShorts = (width + nbits + 15) >> 4;
    /*
     * for each scan line
     */
    for ( ir=0; ir<height; ir++, psrc+=srcShorts, pdst+=dstShorts)
    {
	/*
	 * for each short on the destination line,
	 * find the containing longword in the source and extract the bits
	 */
	pdst[ 0] = psrc[0] << nbits; 
	for ( ids=1; ids<dstShorts; ids++)
	    pdst[ ids] = *(unsigned *)&psrc[ids-1] >> 16-nbits; 
    }
    return dstShorts;
}
#endif
