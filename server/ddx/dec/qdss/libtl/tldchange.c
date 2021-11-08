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

#include	<sys/types.h>
#include	"Ultrix2.0inc.h"
#include	"miscstruct.h"
#include	"tldstate.h"
#include	"tltemplabels.h"

#include	"qd.h"
#include	"qduser.h"
#include	"qdreg.h"
#include	"tl.h"

#define	NOTYET	FatalError("tldchange: not implemented yet...sorry\n");
/* NOMORE - returns truth iff b has no bits true not true in a */
#define	NOMORE(a,b)	\
	(((a) | (b)) == (a))

/* this function takes the queue as a command list,
 * and verifies the named viper state for the vipers included in planemask.
 */
void
tldchange ( mcom, pds )
register unsigned long	mcom;	/* commands mask */
register dUpdatePtr	pds;
{
    register unsigned short	*p;
    register int		icom;	/* index of current command */
    register u_long		fullmask	= m_PLANEMASK;
    register u_long		singlemask	= m_PLANEMASK;
    static dUpdateRec		dstip;


    /* everything is hosed */
    if (dState.planemask == ILL_PLANEMASK)
	Hose_Shadow();

    for (icom = i_NULL - 1; icom >= 0; icom--)
    {	/* n.b.: these are processed in reverse order. */
	register int	whichocr;

	if (!(mcom & (1<<icom)))	/* command not masked */
	    continue;	/* don't do this command...go on to next */
	switch ( icom )
/* These are processed in reverse order.  This is essential for	*
 *   the planemask in particular, or no vipers are awake.		*/
	{
	  case i_TEMPLATE:
	    if (dState.template == pds->template)
		break;
	    dState.template = pds->template;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(1);
		*p++ = pds->template;
		Confirm_dma();
	    }
	    break;
	  case i_ALU:
	    if ( NOMORE(dState.planes[PLANEINDEX(i_ALU)], dState.planemask)
		&& dState.common.alu == pds->common.alu)
		break;	/* state ok */
/* honors "remain" backing */
	    /* if all planes are covered in planemasks, transfer */
	    if (dState.planemask | dState.planes[PLANEINDEX(i_ALU)] == BITS_ALU)
		dState.remain.alu = dState.common.alu;
	    else
		dState.remain.alu = ILL_ALU;
	    dState.planes[PLANEINDEX(i_ALU)] = dState.planemask;
	    dState.common.alu = pds->common.alu;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(2);
		*p++ = dState.template = JMPT_SETALU;
		*p++ = pds->common.alu;
		Confirm_dma();
	    }
	    break;
	  case i_MASK:
	    if ( NOMORE(dState.planes[PLANEINDEX(i_MASK)], dState.planemask)
		&& dState.common.mask == pds->common.mask)
		break;	/* state ok */
	    dState.planes[PLANEINDEX(i_MASK)] = dState.planemask;
	    dState.common.mask = pds->common.mask;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(2);
		*p++ = dState.template = JMPT_SETMASK;
		*p++ = pds->common.mask;
		Confirm_dma();
	    }
	    break;
	  case i_SOURCE:
	    if ( NOMORE(dState.planes[PLANEINDEX(i_SOURCE)], dState.planemask)
		&& dState.common.source == pds->common.source)
		break;	/* state ok */
	    dState.planes[PLANEINDEX(i_SOURCE)] = dState.planemask;
	    dState.common.source = pds->common.source;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(4);
		*p++ = dState.template = JMPT_SETRGBCOLOR;
		*p++ =   RED(pds->common.source);
		*p++ = GREEN(pds->common.source);
		*p++ =  BLUE(pds->common.source);
		Confirm_dma();
	    }
	    break;
	  case i_BGPIXEL:
	    if ( NOMORE(dState.planes[PLANEINDEX(i_BGPIXEL)], dState.planemask)
		&& dState.common.bgpixel == pds->common.bgpixel)
		break;	/* state ok */
	    dState.planes[PLANEINDEX(i_BGPIXEL)] = dState.planemask;
	    dState.common.bgpixel = pds->common.bgpixel;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(4);
		*p++ = dState.template = JMPT_SETRGBBACKCOLOR;
		*p++ =   RED(pds->common.bgpixel);
		*p++ = GREEN(pds->common.bgpixel);
		*p++ =  BLUE(pds->common.bgpixel);
		Confirm_dma();
	    }
	    break;
	  case i_FGPIXEL:
	    if ( NOMORE(dState.planes[PLANEINDEX(i_FGPIXEL)], dState.planemask)
		&& dState.common.fgpixel == pds->common.fgpixel)
		break;	/* state ok */
	    dState.planes[PLANEINDEX(i_FGPIXEL)] = dState.planemask;
	    dState.common.fgpixel = pds->common.fgpixel;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(4);
		*p++ = dState.template = JMPT_SETRGBFORECOLOR;
		*p++ =   RED(pds->common.fgpixel);
		*p++ = GREEN(pds->common.fgpixel);
		*p++ =  BLUE(pds->common.fgpixel);
		Confirm_dma();
	    }
	    break;
	  case i_SRC1OCRA:	case i_SRC2OCRA:	case i_DSTOCRA:
	  case i_SRC1OCRB:	case i_SRC2OCRB:	case i_DSTOCRB:
	    whichocr = OCRINDEX(icom);
	    if ( NOMORE(dState.planes[PLANEINDEX(icom)], dState.planemask)
		&& dState.common.ocr[whichocr] == pds->common.ocr[whichocr])
		break;	/* state ok */
/* honors "remain" backing */
	    /* if all planes are covered in planemasks, transfer */
	    if (dState.planemask | dState.planes[PLANEINDEX(icom)] == BITS_OCR)
		dState.remain.ocr[whichocr] = dState.common.ocr[whichocr];
	    else
		dState.remain.ocr[whichocr] = ILL_OCR;
	    dState.planes[PLANEINDEX(icom)] = dState.planemask;
	    dState.common.ocr[whichocr] = pds->common.ocr[whichocr];
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(2);
		switch (whichocr)
		{
		  case 0:
		    *p++ = dState.template = JMPT_SETSRC1OCRA;
		    break;
		  case 1:
		    *p++ = dState.template = JMPT_SETSRC2OCRA;
		    break;
		  case 2:
		    *p++ = dState.template = JMPT_SETDSTOCRA;
		    break;
		  case 3:
		    *p++ = dState.template = JMPT_SETSRC1OCRB;
		    break;
		  case 4:
		    *p++ = dState.template = JMPT_SETSRC2OCRB;
		    break;
		  case 5:
		    *p++ = dState.template = JMPT_SETDSTOCRB;
		    break;
#ifdef	DEBUG
		  default:
		    FatalError("Illegal dragon state ocr index: %d.\n",
			whichocr);
		    break;
#endif
		}
		*p++ = pds->common.ocr[whichocr];
		Confirm_dma();
	    }
	    break;
	  case i_PLANEMASK:
	    if ( dState.planemask == pds->planemask)
		break;	/* state ok */
	    dState.planemask = pds->planemask;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(4);
		*p++ = dState.template = JMPT_SETRGBPLANEMASK;
		*p++ =   RED(pds->planemask);
		*p++ = GREEN(pds->planemask);
		*p++ =  BLUE(pds->planemask);
		Confirm_dma();
	    }
	    break;
	  case i_CLIP:
	    if ( dState.adder.clip.x1 != pds->adder.clip.x1
	      || dState.adder.clip.y1 != pds->adder.clip.y1
	      || dState.adder.clip.x2 != pds->adder.clip.x2
	      || dState.adder.clip.y2 != pds->adder.clip.y2 )
	    {
		dState.adder.clip.x1 = pds->adder.clip.x1;
		dState.adder.clip.y1 = pds->adder.clip.y1;
		dState.adder.clip.x2 = pds->adder.clip.x2;
		dState.adder.clip.y2 = pds->adder.clip.y2;
		if (!(mcom & m_NOOP))
		{
		    if ( pds->adder.clip.x1 != 0
		      || pds->adder.clip.y1 != 0
		      || pds->adder.clip.x2 != 1024
		      || pds->adder.clip.y2 != 2048 )
		    {
			Need_dma(5);
			*p++ = dState.template = JMPT_SETCLIP;
			*p++ = pds->adder.clip.x1;
			*p++ = pds->adder.clip.y1;
			*p++ = pds->adder.clip.x2;
			*p++ = pds->adder.clip.y2;
			Confirm_dma();
		    } else
		    {
			Need_dma(1);
			*p++ = dState.template = JMPT_RESETCLIP;
			Confirm_dma();
		    }
		}
	    }
	    break;
	  case i_TRANSLATE:
	    if ( dState.adder.translate.x != pds->adder.translate.x
	      || dState.adder.translate.y != pds->adder.translate.y )
	    {
		dState.adder.translate.x = pds->adder.translate.x;
		dState.adder.translate.y = pds->adder.translate.y;
		if (!(mcom & m_NOOP))
		{
		    /* XXX - yukky hack! */
		    _tlTranslate.x = pds->adder.translate.x;
		    _tlTranslate.y = pds->adder.translate.y;
		    Need_dma(7);
		    *p++ = dState.template = JMPT_SETTRANSLATE;
		    *p++ = pds->adder.translate.x & 0x3fff;
		    *p++ = pds->adder.translate.y & 0x3fff;
		    *p++ = pds->adder.translate.x & 0x3fff;
		    *p++ = pds->adder.translate.y & 0x3fff;
		    *p++ = pds->adder.translate.x & 0x3fff;
		    *p++ = pds->adder.translate.y & 0x3fff;
		    Confirm_dma();
		}
	    }
	    break;
	  case i_RASTERMODE:
	    if ( dState.adder.rastermode == pds->adder.rastermode )
		break;	/* state ok */
	    dState.adder.rastermode = pds->adder.rastermode;
	    if (!(mcom & m_NOOP))
	    {
		Need_dma(2);
		*p++ = dState.template = JMPT_SETRASTERMODE;
		*p++ = pds->adder.rastermode;
		Confirm_dma();
	    }
	    break;
	  case i_FGMASK:
	  case i_FGBGMASK:
	    /* unselected alu's = dest	*/
	    /* alu's selected = planemask	*/
	    if (dState.remain.alu != LF_D | FULL_SRC_RESOLUTION | BITS_ALU
	     ||	dState.planes[PLANEINDEX(i_ALU)] != pds->planemask)
		fullmask |= m_ALU; /* XXX - could load only these planes */
	    /* receiving ocr's = ext_source	*/
	    if ( dState.remain.ocr[OCRINDEX(i_SRC2OCRB)]
	      != (icom == i_FGMASK ? EXT_M1_M2 : EXT_SOURCE)
		  |NO_ID|BAR_SHIFT_DELAY	)
		fullmask |= m_SRC2OCRB;
	    /* transmitting ocr = mask	*/
	    if (dState.planes[PLANEINDEX(i_SRC2OCRB)] != TOGREEN(pds->stipmask))
	    {
		fullmask |= m_SRC2OCRB;
		singlemask |= m_SRC2OCRB;
	    }
	    /* transmitting ocr = int_source | id	*/
	    if ( dState.common.ocr[OCRINDEX(i_SRC2OCRB)]
	      != (icom == i_FGMASK ? EXT_M1_M2 : EXT_SOURCE)
		  |ID|BAR_SHIFT_DELAY		)
		singlemask |= m_SRC2OCRB;

	    /* all planes receive src2	*/
	    /* all planes alu = no-op	*/
	    dstip.planemask = pds->planemask;
	    dstip.common.alu = LF_D | FULL_SRC_RESOLUTION | BITS_ALU;
	    dstip.common.ocr[OCRINDEX(i_SRC2OCRB)]
		= (icom == i_FGMASK ? EXT_M1_M2 : EXT_SOURCE)
		   |NO_ID|BAR_SHIFT_DELAY;
	    dstip.common.ocr[OCRINDEX(i_DSTOCRB)] = NO_BAR_SHIFT_DELAY;
#ifdef notdef
	    if (fullmask & (~(m_PLANEMASK)))	/* real output */
		tldchange(fullmask, &dstip);
#endif

	    /* one plane transmits src2	*/
	    dstip.planemask = TOGREEN(pds->stipmask)
		& BITS_PLANEMASK; /* in green byte */
	    dstip.common.ocr[OCRINDEX(i_SRC2OCRB)]
		= (icom == i_FGMASK ? EXT_M1_M2 : EXT_SOURCE)
		   |ID|BAR_SHIFT_DELAY;
	    if (singlemask & (~(m_PLANEMASK)))	/* real output */
		tldchange(singlemask, &dstip);

	    break;
#ifdef	DEBUG
	  default:
	    FatalError("Illegal dragon state command load index: %d.\n", icom);
#endif
	}
    }
    if (mcom & (m_FGMASK | m_FGBGMASK))
    {	/* munge planemask - otherwise transmit viper may be off */
#ifdef	DEBUG
	if (mcom & m_NOOP)
	    FatalError("Illegal: fg_ or fgbgmask dragon state with m_NOOP\n");
#endif
	if (mcom & m_FGMASK)
	{
	    dState.common.mask = ILL_MASK;
	    dState.planes[PLANEINDEX(i_MASK)] = BITS_PLANEMASK;
	}
	else	/* m_FGBGMASK */
	{
	    dState.common.source = ILL_SOURCE;
	    dState.planes[PLANEINDEX(i_SOURCE)] = BITS_PLANEMASK;
	}
	if ( dState.planemask == BITS_PLANEMASK)
	    return;	/* state ok */
	dState.planemask = BITS_PLANEMASK;
	Need_dma(4);
	*p++ = dState.template = JMPT_SETRGBPLANEMASK;
	*p++ =   RED(dState.planemask);
	*p++ = GREEN(dState.planemask);
	*p++ =  BLUE(dState.planemask);
	Confirm_dma();
    }
}

