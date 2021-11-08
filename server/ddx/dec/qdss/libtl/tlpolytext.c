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
 * SETVIPER does not yet initialize LU_R4, upon which INITTEXT* depends,
 * so we do the following hacks:				XX
 */
#if NPLANES==8
#define GETGREENBYTE( p)	((p)<<8)
#else
#define GETGREENBYTE( p)        (p)
#endif

#include "X.h"
#include "Xproto.h"		/* required by fontstruct.h */

#include "gcstruct.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "dixfontstr.h"
#include "fontstruct.h"

#include "qd.h"

#include <sys/types.h>
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"

#include "tl.h"
#include "tltemplabels.h"

#define LOG2OF1024	10
#define u_char unsigned char

#ifdef IMAGE
# define PROCNAME tlImageText
# define QPROCNAME "tlImageText"	/* quoted procedure name */
#else
# define PROCNAME tlPolyText
# define QPROCNAME "tlPolyText"
#endif

extern int	Nplanes;

#ifdef IMAGE
void
#else	/* PolyText returns width */
int
#endif
PROCNAME( pWin, pGC, x0, y0, nChars, pStr, fontPmask, fontY)
    WindowPtr	pWin;		/* destination */
    GCPtr       pGC;		/* needed for planemask */
    int		x0, y0;		/* origin */
    int		nChars;
    char *	pStr;
    int		fontPmask;	/* plane address of off-screen font */
    int		fontY;		/* Y address of off-screen font */
{
    FontPtr		pFont = pGC->font;
    FontInfoPtr		pfi = pFont->pFI;
    int			chfirst = pfi->firstCol;
    int			chlast = pfi->lastCol;
    CharInfoPtr 	pCI = &pFont->pCI[-chfirst];
    RegionPtr		pcompclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    struct DMAreq *	pRequest;
    int			nshortInit;
    BoxPtr		pboxes;	/* a temporary */
    BoxPtr		pboxes2;/* a temporary */
    BoxPtr		clip;	/* list of clip rects after trivial-reject */
    int			nclip;  /* number of clip rects */
    QDFontPtr		qdfont =
			    (QDFontPtr) pFont->devPriv[ pGC->pScreen->myNum];
    register unsigned short *p;
    register int	nxbits = LOG2OF1024-qdfont->log2dx;
    register int	xmask = (1<<nxbits)-1;
    int			width = (max(  pfi->maxbounds.metrics.rightSideBearing,
				 pfi->maxbounds.metrics.characterWidth
				    + pfi->maxbounds.metrics.leftSideBearing)
			- pfi->minbounds.metrics.leftSideBearing) & 0x3fff;
    int 		height = pfi->maxbounds.metrics.ascent
			+ pfi->maxbounds.metrics.descent; /*adder dy*/
    int			ic;
    register int	xc;
    int			maxcharblits = MAXDMAWORDS/3;
    unsigned int planemask = ((Nplanes == 4) ? (0x0f) : (0xff));

    INVALID_SHADOW;	/* XXX */
    /*
     * do trivial-reject Y clipping by pruning the list of boxes
     */
    clip = (BoxPtr) alloca( pcompclip->numRects * sizeof( BoxRec));
    for ( ic=0, pboxes=pcompclip->rects, pboxes2=clip;
          ic<pcompclip->numRects;
          ic++, pboxes++)
    {
        if (   pWin->absCorner.y + y0 - pfi->maxbounds.metrics.ascent
								>= pboxes->y2
	    || pWin->absCorner.y + y0 + pfi->maxbounds.metrics.descent
								< pboxes->y1) 
	    continue;
	*pboxes2++ = *pboxes;
    }
    nclip = pboxes2 - clip;
    /*
     * loops may not deal gracefully with nChars == 0 or nclip == 0
     */
    if ( nChars==0 || nclip==0)
	return;

    nshortInit = (
#if 1 /* NPLANES==24 */
# ifdef IMAGE
				4  + 2*NCOLOR24SHORTS);
# else
				5  + 1*NCOLOR24SHORTS);
# endif
#else /* NPLANES==8 */
# ifdef IMAGE
				3  + 2*NCOLORSHORTS);
# else
				3  + 1*NCOLORSHORTS);
# endif
#endif

    SETTRANSLATEPOINT( pWin->absCorner.x, pWin->absCorner.y);

    Need_dma(nshortInit);
#if 1 /* NPLANES==24 */
# ifdef IMAGE
    *p++ = JMPT_SETV24OPAQUE;
    *p++ = RED( pGC->planemask );
    *p++ = GREEN( pGC->planemask );
    *p++ = BLUE( pGC->planemask );
    SET24COLOR( p, GETGREENBYTE( pGC->fgPixel));
    SET24COLOR( p, GETGREENBYTE( pGC->bgPixel));
# else
    *p++ = JMPT_SETV24TRANS;
    *p++ = RED( pGC->planemask );
    *p++ = GREEN( pGC->planemask );
    *p++ = BLUE( pGC->planemask );
    *p++ = umtable[ pGC->alu] | FULL_SRC_RESOLUTION;
    SET24COLOR( p, GETGREENBYTE( pGC->fgPixel));
# endif

#else /* NPLANES==8 */

# ifdef IMAGE
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

    for ( ic=0, pboxes = clip;
	  ic<nclip;
	  ic++, pboxes++)
    {
	int			nch = nChars;
	register u_char *	pstr = (u_char *) pStr;
	unsigned short *	px2clip;
 
	xc = x0;

	Confirm_dma();
	Need_dma( 10);
	*p++ = JMPT_SETCLIP;
	*p++ = pboxes->x1;
	px2clip = p;		/* set up for ultimate sleazoid hack */
	*p++ = pboxes->x2;
	*p++ = pboxes->y1;
	*p++ = pboxes->y2;

#ifdef IMAGE
	*p++ = JMPT_INITTEXTTERM;
#else
	*p++ = JMPT_INITTEXTMASK;
#endif
	*p++ = width & 0x3fff;					/*dx*/
	*p++ = height & 0x3fff;					/*dy*/
	*p++ = fontPmask & planemask;
	*p++ = y0 - pfi->maxbounds.metrics.ascent & 0x3fff;
	while( nch)
	{
	    int n = min( nch, maxcharblits);

	    nch -= n;
	    Confirm_dma();
	    Need_dma( 3*n);
	    while ( n--)
	    {
		register unsigned int ch = *pstr;

		if ( ch < chfirst || ch > chlast)
		{
#ifdef DEBUG
		    ErrorF( "%s: character out of range", QPROCNAME);
#endif
		    ch = pfi->chDefault;		/* step on the arg */
		}
		*p++ = (ch & xmask) << qdfont->log2dx;	/* source X */
		*p++ = (ch>>nxbits) * height + fontY;	/* source Y */
		*p++ = xc + pCI[ ch].metrics.leftSideBearing & 0x3fff;
							    /* destination X */
		xc += pCI[ ch].metrics.characterWidth;
		pstr++;
	    }
	}
#ifdef IMAGE
	/*
	 * Ultimate sleazoid hack to clip off ragged, background-colored,
	 * right-hand side of glyphs.
	 */
	*px2clip = min( *px2clip,
		    pWin->absCorner.x + xc
		    - pCI[ *(pstr-1)].metrics.characterWidth
		    + pCI[ *(pstr-1)].metrics.rightSideBearing);
#endif
    }
    Confirm_dma();

#ifndef IMAGE
    return xc;
#endif
}
