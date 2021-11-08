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

/* tlprimitive
 *	used by tldrawshapes for generic packet building
 */

void
PROCNAME (pWin, pGC,
#ifdef	D_SPANS
	    nshapes, pshape, pWidth, fSorted)
#else	/* rects */
	    nshapes, pshape)
#endif

/* parameters declarations: */
    WindowPtr	pWin;
    GCPtr	pGC;
#ifdef	D_SPANS
    int         nshapes;
    DDXPointPtr pshape;
    int         *pWidth;
    int         fSorted;
#else	/* rects */
    int			nshapes;
    xRectangle *	pshape;
#endif

{
    register unsigned short     *p;
    RegionPtr pSaveGCclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    register BoxPtr pclip = pSaveGCclip->rects;	/* step through GC clip */
    int		nclip = pSaveGCclip->numRects;	/* number of clips */
    int		mask;	/* planes used by offscreen dragon tile */
    register DDXPointPtr	trans = &(pWin->absCorner);
#ifdef	D_SPANS
    register short	newx1;	/* used in pre-clipping */
    register int	*pwid;	/* step through widths */
    register DDXPointPtr
#else	/* rects */
    register short	newx1, newy1;	/* used in pre-clipping */
    register xRectangle *
#endif
	parg;		/* step through argument shapes */
    int	narg;		/* number of argument shapes */
#ifdef	D_TILE
    DDXPointRec		tileOrg;	/* off-screen rotated tile address */
    int			magic;
#endif
    dUpdateRec		dsprim;

#ifdef	D_TILE
# ifdef	D_MASK	/* x11 stip */
#  define	DTILE	stipple
# else	/* x11 tile */
#  define	DTILE	tile
# endif
# ifdef	D_ODD
    mask = tlConfirmPixmap(pGC->DTILE->devPrivate, pGC->DTILE, &(tileOrg.y));
# else	/* natural size..align to word boundary */
    tlrotile(pGC->DTILE, pGC->patOrg, &tileOrg, &mask, &magic);
# endif
#endif

#ifdef	D_MASK
# ifdef	D_FG	/* stipple: fgmask */
    dsprim.planemask = pGC->planemask & BITS_PLANEMASK;
    dsprim.adder.clip.x1 = dsprim.adder.clip.y1 = 0;
    dsprim.adder.clip.x2 = 1024;
    dsprim.adder.clip.y2 = 2048;
    dsprim.adder.translate.x = trans->x;
    dsprim.adder.translate.y = trans->y;
    dsprim.adder.rastermode = DST_WRITE_ENABLE|DST_INDEX_ENABLE|NORMAL;
    dsprim.stipmask = mask & BITS_STIPMASK;
    dsprim.common.alu = umtable[(pGC->alu)];
    dsprim.common.mask = BITS_MASK;
    dsprim.common.source = pGC->fgPixel & BITS_FGPIXEL;
    dsprim.common.bgpixel = 0;
    dsprim.common.fgpixel = BITS_FGPIXEL;
    tldchange (	m_RASTERMODE|m_TRANSLATE|m_CLIP|m_PLANEMASK|
		m_FGMASK|m_ALU|m_MASK|m_SOURCE|m_BGPIXEL|m_FGPIXEL,
		&dsprim);
    dsprim.template = ILL_TEMPLATE;
    tldchange(m_TEMPLATE|m_NOOP, &dsprim);
# else		/* OpStip: fgbgmask */
    dsprim.planemask = pGC->planemask & BITS_PLANEMASK;
    dsprim.adder.clip.x1 = dsprim.adder.clip.y1 = 0;
    dsprim.adder.clip.x2 = 1024;
    dsprim.adder.clip.y2 = 2048;
    dsprim.adder.translate.x = trans->x;
    dsprim.adder.translate.y = trans->y;
    dsprim.adder.rastermode = DST_WRITE_ENABLE|DST_INDEX_ENABLE|NORMAL;
    dsprim.stipmask = mask & BITS_STIPMASK;
    dsprim.common.alu = umtable[(pGC->alu)];
    dsprim.common.mask = BITS_MASK;
    dsprim.common.source = BITS_SOURCE;
    dsprim.common.bgpixel = pGC->bgPixel & BITS_BGPIXEL;
    dsprim.common.fgpixel = pGC->fgPixel & BITS_FGPIXEL;
    tldchange (	m_RASTERMODE|m_TRANSLATE|m_CLIP|m_PLANEMASK|
		m_FGBGMASK|m_ALU|m_MASK|m_SOURCE|m_BGPIXEL|m_FGPIXEL,
		&dsprim);
    dsprim.template = ILL_TEMPLATE;
    tldchange(m_TEMPLATE|m_NOOP, &dsprim);
# endif
#else
# ifdef	D_TILE	/* tile */
    dsprim.planemask = pGC->planemask & BITS_PLANEMASK;
    dsprim.adder.clip.x1 = dsprim.adder.clip.y1 = 0;
    dsprim.adder.clip.x2 = 1024;
    dsprim.adder.clip.y2 = 2048;
    dsprim.adder.translate.x = trans->x;
    dsprim.adder.translate.y = trans->y;
    dsprim.adder.rastermode = DST_WRITE_ENABLE|DST_INDEX_ENABLE|NORMAL;
    dsprim.common.alu = umtable[(pGC->alu)];
    dsprim.common.mask = BITS_MASK;
    dsprim.common.bgpixel = 0;
    dsprim.common.fgpixel = BITS_FGPIXEL;
    dsprim.common.ocr[OCRINDEX(i_SRC2OCRB)] =
	EXT_NONE|INT_SOURCE|NO_ID|BAR_SHIFT_DELAY;
    tldchange (	m_RASTERMODE|m_TRANSLATE|m_CLIP|m_PLANEMASK|
		m_ALU|m_MASK|m_BGPIXEL|m_FGPIXEL|m_SRC2OCRB,
		&dsprim);

    /* keep planemask the same */
    dsprim.template = ILL_TEMPLATE;
    dsprim.common.source = ILL_SOURCE;
    tldchange(m_TEMPLATE|m_SOURCE|m_NOOP, &dsprim);
# else		/* fg solid */
    dsprim.planemask = pGC->planemask & BITS_PLANEMASK;
    dsprim.adder.clip.x1 = dsprim.adder.clip.y1 = 0;
    dsprim.adder.clip.x2 = 1024;
    dsprim.adder.clip.y2 = 2048;
    dsprim.adder.translate.x = trans->x;
    dsprim.adder.translate.y = trans->y;
    dsprim.adder.rastermode = DST_WRITE_ENABLE|DST_INDEX_ENABLE|NORMAL;
    dsprim.common.alu = umtable[(pGC->alu)];
    dsprim.common.mask = BITS_MASK;
    dsprim.common.source = pGC->fgPixel & BITS_SOURCE;
    dsprim.common.bgpixel = 0;
    dsprim.common.fgpixel = BITS_FGPIXEL;
#  ifdef	D_SPANS
    dsprim.template = JMPT_BASICSPAN;
#  else		/* rects */
    dsprim.template = JMPT_BASICRECT;
#  endif
    tldchange ( m_TEMPLATE|m_RASTERMODE|m_TRANSLATE|m_CLIP|m_PLANEMASK|
		m_ALU|m_MASK|m_SOURCE|m_BGPIXEL|m_FGPIXEL,
		&dsprim);
# endif
#endif

/* rasterop setup and initialization */
#ifdef	D_TILE
# ifdef	D_ODD	/* unnatural size..use linear pattern mode */
    Need_dma(6);
#  ifdef	D_MASK	/* x11 stip */
    *p++ = JMPT_INIT1STIPPLE;
#  else	/* x11 tile */
    *p++ = JMPT_INIT1TILE;
#  endif
    *p++ = pGC->DTILE->width;
    *p++ = pGC->DTILE->height;
    *p++ = 0;
    *p++ = tileOrg.y;
#  ifdef	D_SPANS
    *p++ = JMPT_PATTERNSPAN;
#  else	/* rects */
    *p++ = JMPT_PATTERNRECT;
#  endif
    Confirm_dma();
# else	/* natural sized tile..use source 2 tiling */
    Need_dma(5);
#  ifdef	D_MASK	/* x11 stip */
    *p++ = JMPT_INIT2STIPPLE;
#  else	/* x11 tile */
    *p++ = JMPT_INIT2TILE;
#  endif
    *p++ = tileOrg.x;
    *p++ = tileOrg.y;
    *p++ = magic;
#  ifdef	D_SPANS
    *p++ = JMPT_TILESPAN;
#  else	/* rects */
    *p++ = JMPT_TILERECT;
#  endif
    Confirm_dma();
# endif
#endif

#define	ZERORLESS(x)	\
	(((short) (x)) <= 0)

#ifdef	D_SPANS
Need_dma(3);
    for ( ; nclip > 0; nclip--, pclip++)
    {
        narg = nshapes;
        parg = pshape;
for (pwid = pWidth; narg > 0; narg--, pwid++, parg++) {
  *p++ = (newx1 = (parg->x+trans->x < pclip->x1)
	? pclip->x1-trans->x : parg->x);
  if (parg->y+trans->y < pclip->y1
	|| parg->y+trans->y >= pclip->y2)
    p -= 1;
  else {
    *p++ = parg->y;
    if ( ZERORLESS(*p++ = ((parg->x+(*pwid)+trans->x < pclip->x2)
	? parg->x+(*pwid) : pclip->x2-trans->x) - newx1) )
      p -= 3;
    else {	/* yes, i needed that dma */
      Confirm_dma();
      Need_dma(3);
    }
  }
}
#else	/* rects */
Need_dma(4);
    for ( ; nclip > 0; nclip--, pclip++)
    {
        narg = nshapes;
        parg = pshape;
for ( ; narg > 0; narg--, parg++) {
  *p++ = (newx1 = (parg->x+trans->x < pclip->x1)
	? pclip->x1-trans->x : parg->x);
  *p++ = (newy1 = (parg->y+trans->y < pclip->y1)
	? pclip->y1-trans->y : parg->y);
  if ( ZERORLESS(*p++ = ((parg->x+(int)parg->width+trans->x<pclip->x2)
	? parg->x+(int)parg->width : pclip->x2-trans->x) - newx1) )
    p -= 3;
  else {
    if ( ZERORLESS(*p++=((parg->y+(int)parg->height+trans->y<pclip->y2)
	? parg->y+(int)parg->height : pclip->y2-trans->y) - newy1) )
      p -= 4;
    else {	/* yes, i needed that dma */
      Confirm_dma();
      Need_dma(4);
    }
  }
}
#endif
    }
    Confirm_dma();
}
#undef	DTILE	/* so compiler doesn't complain */
#undef	PROCNAME
