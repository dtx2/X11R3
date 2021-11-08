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
 *  setspan.c
 *
 *  written by Kelleher, july 1986
 *  (adapted from code written by drewry)
 *
 *  Set a span (type determined by srcpixtype) in the framebuffer
 *  to the values given.
 *
 *  NOTES
 *  this will work only if each channel has <8K pixels
 */

#include <sys/types.h>
#include <stdio.h>	/* debug */

#include "X.h"
#include "windowstr.h"
#include "regionstr.h"
#include "pixmap.h"
#include "gcstruct.h"
#include "dixstruct.h"

#include "Xproto.h"
#include "Xprotostr.h"
#include "mi.h"
#include "Xmd.h"

/*
 * driver headers
 */
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdioctl.h"
#include "qdreg.h"

#include	"tl.h"
#include "qd.h"
#include "tltemplabels.h"

extern	int	Nchannels;

#define	pixelsize	(1)

tlsetspan( pWin, pGC, x, y, width, pcolorInit)
    WindowPtr	pWin;
    GCPtr	pGC;
    int		x,y;
    int		width;
    unsigned char	*pcolorInit;
{
    register unsigned short *p;
    register int npix;
    register int zblock;
    int		i;
    RegionPtr	pSaveGCclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    int		nclip = pSaveGCclip->numRects;
    register BoxPtr	pclip = pSaveGCclip->rects;
    register unsigned char	*pcolor;

    extern struct DMAreq *DMArequest;
    extern u_short *DMAbuf;

    INVALID_SHADOW;	/* XXX */
    if ( width == 0)
	return;
    if ( width > MAXDGAWORDS)
    {
	ErrorF( "tlsetspan: width > MAXDGAWORDS\n");
	width = MAXDGAWORDS;
    }

    SETTRANSLATEPOINT(pWin->absCorner.x, pWin->absCorner.y);

    /*
     *  Just set up some state and fall through to code below
     *  for all RGB pixel data.  The state we are interested in is
     *  the number of zblocks, and the zblock identifiers, as well
     *  as the size of pixel data (for bopping through the array).
     *  Z buffer data is handled seperately.
     */

    while (nclip-- > 0) {
	Need_dma(11);
	*p++ = JMPT_SETRGBPLANEMASK;
	*p++ = 0xff;
	*p++ = 0xff;
	*p++ = 0xff;
	*p++ = JMPT_SETVIPER24;
	*p++ = umtable[pGC->alu] | FULL_SRC_RESOLUTION;
	*p++ = JMPT_SETCLIP;
	*p++ = (pclip->x1) & 0x3fff;
	*p++ = (pclip->x2) & 0x3fff;
	*p++ = (pclip->y1) & 0x3fff;
	*p++ = (pclip->y2) & 0x3fff;
	Confirm_dma();

	for (i=0, pcolor = pcolorInit; i<Nchannels; i++) {
#if NPLANES==8
	    zblock = ZGREEN;
#else	/* NPLANES == 24 */
	    switch (i)
	    {
		case 0:
	    	    zblock = ZRED;
		    break;
		case 1:
	    	    zblock = ZGREEN;
		    break;
		case 2:
	    	    zblock = ZBLUE;
		    break;
	    }
#endif

	    /*
	     * assuming:
	     * 	24--all red, all green, all blue; NOT r/g/b,r/g/b,r/g/b,...
	     * 	8--all grey (actually green)
	     */

	    Need_dma(11 + width);	/* planes+initptob */
	    *p++ = JMPT_SETRGBPLANEMASK;
	    switch(zblock) {
	      case ZGREEN:
		*p++ = 0x00;
		*p++ = GREEN(pGC->planemask);
		*p++ = 0x00;
		break;
	     case ZRED:
		*p++ = RED(pGC->planemask);
		*p++ = 0x00;
		*p++ = 0x00;
		break;
	     case ZBLUE:
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = BLUE(pGC->planemask);
		break;
	    }
	    *p++ = JMPT_INITPTOB;
	    *p++ = x & 0x3fff;
	    *p++ = y & 0x3fff;
	    *p++ = width &0x3fff;
	    *p++ = 1;

	    *p++ = PTBZ | zblock;
	    /* DGA magic bit pattern for PTB */
	    *p++ = 0x6000 |(0x1fff & -width);
	    npix = width;
	    while (npix--) {
		*p++ = *pcolor;
		pcolor += pixelsize;
	    }
	    Confirm_dma ();
	}
	pclip++;
    }
}
