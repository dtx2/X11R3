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
 *   Created by djb, May 1986 (adapted from Ray's QDline.c)
 *   edited by drewry, 13 june 1986: added clipping
 *   edited by kelleher, 27 june 1986: added pixeltypes, interp code.
 */

#include <sys/types.h>

#include "X.h"
#include "windowstr.h"
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

#define NLINES (req_buf_size/4)

/*
 * Each packet is repeated once per clip rectangle.  Separating clip rectangles
 * from GC lets caller pre-clip.
 */
tlzlines(pWin, pGC, nclip, pclip, nptInit, pPtsInit)
    WindowPtr	pWin;
    GCPtr	pGC;
    int		nclip;		/* ignore clip rectangles in GC */
    BoxPtr	pclip;		/* ignore clip rectangles in GC */
    int		nptInit;
    DDXPointRec	*pPtsInit;
{
    int nby;	/* size in shorts */
    register DDXPointRec *pPts;
    register struct DMAreq *pRequest;
    register int nlinesThisTime;
    register unsigned short *p;
    int		nlines;

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT(pWin->absCorner.x, pWin->absCorner.y);

    while ( nclip-- > 0) {
	pPts = pPtsInit;
	nlines = nptInit-1;

	/*
	 *  Break the polyline into reasonable size packets so we
	 *  have enough space in the dma buffer.
	 */
	Need_dma (16);	/* per-clip initialization */
	*p++ = JMPT_SETRGBPLANEMASK;
	*p++ = RED(pGC->planemask);
	*p++ = GREEN(pGC->planemask);
	*p++ = BLUE(pGC->planemask);
	*p++ = JMPT_SETVIPER24;
	*p++ = umtable[pGC->alu] | FULL_SRC_RESOLUTION;
	*p++ = JMPT_SETCLIP;
	*p++ = (pclip->x1) & 0x3fff;
	*p++ = (pclip->x2) & 0x3fff;
	*p++ = (pclip->y1) & 0x3fff;
	*p++ = (pclip->y2) & 0x3fff;
	*p++ = JMPT_SETRGBCOLOR;
	*p++ = RED(pGC->fgPixel);
	*p++ = GREEN(pGC->fgPixel);
	*p++ = BLUE(pGC->fgPixel);
	*p++ = JMPT_INITPOLYLINE;
	Confirm_dma();
	while ( nlines > 0) {
	    nlinesThisTime = min(nlines, NLINES);
	    nlines -= NLINES;
	    Need_dma(nlinesThisTime * 4);
	    while (nlinesThisTime-- > 0) {
		*p++ = pPts->x & 0x3fff;
		*p++ = pPts->y & 0x3fff;
		*p++ = ((pPts+1)->x - pPts->x) & 0x3fff;
		*p++ = ((pPts+1)->y - pPts->y) & 0x3fff;
		pPts++;
	    }
	    Confirm_dma();
	}
	Need_dma(1);
	*p++ = TEMPLATE_DONE;
	Confirm_dma();
	pclip++;
    }
}

