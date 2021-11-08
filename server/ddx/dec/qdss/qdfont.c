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

#include "X.h"
#include "Xproto.h"

#include "scrnintstr.h"
#include "gcstruct.h"
#include "fontstruct.h"
#include "dixfontstr.h"

#include "qd.h"
#define LOG2OF1024	10

/*
 * must be called only once per font, otherwise storage leak
 */
qdRealizeFont( pscr, pfont)
    ScreenPtr	pscr;
    FontPtr	pfont;
{
    pfont->devPriv[ pscr->myNum] = (pointer)NULL;
}

qdUnrealizeFont( pscr, pfont)
    ScreenPtr	pscr;
    FontPtr	pfont;
{
    QDFontPtr	pqdfont = (QDFontPtr)pfont->devPriv[ pscr->myNum];

    if ( pqdfont != (QDFontPtr)NULL
      && pqdfont != (QDFontPtr)QDSLOWFONT)
    {
	tlCancelPixmap( pfont);		/* x11 font address is used as ID */
	mfbDestroyPixmap( pqdfont->pPixmap);	/* always depth 1 */
	Xfree( pqdfont);
    }
}

/*
 * Called by qdValidateGC to create a QDFontRec for fast output.
 *
 * Creates a single plane pixmap in memory, writes the characters of the
 * font into it, and loads the bitmap into the off-screen cache.
 *     
 * pFont must satisfy the various criteria for being loaded off-screen,
 * otherwise the font private field will be set to QDSLOWFONT.
 * The criteria are:
 *	one-byte character code
 *	can be packed side-to-side in one row of off-screen memory
 * 
 * Return Bool indicates whether or not fast output is possible.
 */
Bool
QDCreateFont( pscr, pFont)
    ScreenPtr	pscr;
    FontPtr	pFont;
{
    unsigned int ic;
    CharInfoPtr	pci;		/* will be biased by -chfirst */
    FontInfoPtr pfi;
    QDFontPtr	pqdfont;
    GCPtr	pgc;		/* handed to mfb, to build font */
    GC		cgc;		/* helps to change values in pgc */
    int		chfirst;
    int		chlast;
    int		paddedbreadth;	/* no character is fatter than this */
    int		cellheight;	/* no character is taller than this */
    int		nrows;
    int         nxbits;
    int         xmask;

    PixmapPtr	mfbCreatePixmap();

    /*
     * check that this is the first call to QDCreateFont for this font
     * and that the font is suitable for fast output
     */
    if (   pFont == NULL
	|| pFont->devPriv[ pscr->myNum] == (pointer)QDSLOWFONT)
	return FALSE;
    if ( pFont->devPriv[ pscr->myNum] != (pointer)NULL)
	return TRUE;
    /* if ( ! pFont->pFI->linear)	NOT SET BY DIX		XXX */
    if ( pFont->pFI->firstRow != pFont->pFI->lastRow)
	return FALSE;

    /*
     * Create the font bitmap for off-screen caching.
     * Leave spaces for the unused character codes from 0 to chfirst,
     * to simplify the arithmetic.
     */
    chfirst = pFont->pFI->firstCol;
    chlast = pFont->pFI->lastCol;
    pci = &pFont->pCI[-chfirst];
    pfi = pFont->pFI;

    cellheight =  pfi->maxbounds.metrics.ascent
	        + pfi->maxbounds.metrics.descent;
    paddedbreadth = power2ceiling(
	max(  pfi->maxbounds.metrics.rightSideBearing,
	      pfi->maxbounds.metrics.characterWidth
			+ pfi->maxbounds.metrics.leftSideBearing)
	    - pfi->minbounds.metrics.leftSideBearing);
    nrows = paddedbreadth * (chlast+1) / pscr->width +
	   (paddedbreadth * (chlast+1) % pscr->width ? 1 : 0);

    /*
     * POLICY: if the font would be too tall to fit off-screen without crowding
     * the full-depth pixmaps, back out.
     */
    if ( nrows * cellheight > (2048-864)>>1)
    {
	pFont->devPriv[ pscr->myNum] = (pointer)QDSLOWFONT;
	return FALSE;
    }

    /*
     * allocate the QDFontRec
     * create a depth 1 pixmap and write the characters into it
     */
    pqdfont = (QDFontPtr) Xalloc( sizeof(QDFontRec));
    pFont->devPriv[ pscr->myNum] = (pointer)pqdfont;
    pqdfont->log2dx = ffs( paddedbreadth) - 1;
    pqdfont->pPixmap = mfbCreatePixmap( pscr, pscr->width, nrows*cellheight, 1);
    nxbits = LOG2OF1024-pqdfont->log2dx;
    xmask = (1<<nxbits)-1;

    pgc = GetScratchGC( 1, pscr);
    cgc.alu = GXcopy;		cgc.stateChanges  = GCFunction;
    cgc.fgPixel = 1;		cgc.stateChanges |= GCForeground;
    cgc.bgPixel = 0;		cgc.stateChanges |= GCBackground;
    cgc.fillStyle = FillSolid;	cgc.stateChanges |= GCFillStyle;
    cgc.font = pFont;		cgc.stateChanges |= GCFont;
    QDChangeGCHelper( pgc, &cgc);
    ValidateGC( pqdfont->pPixmap, pgc);
    for ( ic=0; ic<=chlast; ic++)
    {
	(* pgc->PolyText8)( pqdfont->pPixmap, pgc,
		paddedbreadth * (ic & xmask)
					- pci[ic].metrics.leftSideBearing,
		cellheight * (ic >> nxbits)
					+ pfi->maxbounds.metrics.ascent,
		1, (char *)&ic);
    }
    FreeScratchGC( pgc);
    return TRUE;
}

/*
 * Called by fast text output routines.
 *
 * Check to see if the font is already off-screen.  If not, create a temporary
 * paint pixmap, load it off-screen, and destroy the paint pixmap.
 *
 * address of the x11 font is used as the unique ID for the off-screen bitmap
 *
 * returns planemask of loaded font
 */
int
LoadFont( pscr, pFont, yaddr)
    ScreenPtr	pscr;
    FontPtr	pFont;
    int*	yaddr;	/* RETURN */
{
    PixmapPtr	pxpix = ((QDFontPtr)pFont->devPriv[ pscr->myNum])->pPixmap;
    int		planemask;

    if ( planemask = tlConfirmPixmap( pFont, (PixmapPtr) NULL, yaddr))
	return planemask;

    /*
     * Hand the memory bitmap to the off-screen mem. allocator
     */
    return tlConfirmPixmap( pFont, pxpix, yaddr);
}
