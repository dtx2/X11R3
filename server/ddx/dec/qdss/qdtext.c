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
#include "gcstruct.h"
#include "windowstr.h"

void
qdImageText8( pDraw, pGC, x0, y0, nChars, pStr)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x0, y0;
    int		nChars;
    char *	pStr;
{
    int		fontPmask;
    int		fontY;

    /*
     * If pDraw is not the screen
     */
    if ( pDraw->type == UNDRAWABLE_WINDOW)
        return;
    if ( pDraw->type == DRAWABLE_PIXMAP)
    {
	miImageText8( pDraw, pGC, x0, y0, nChars, pStr);
	return;
    }

    fontPmask = LoadFont( pDraw->pScreen, pGC->font, &fontY);
    tlImageText( (WindowPtr)pDraw, pGC, x0, y0,
					    nChars, pStr, fontPmask, fontY);
    return;
}

int
qdPolyText8( pDraw, pGC, x0, y0, nChars, pStr)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x0, y0;
    int 	nChars;
    char *	pStr;
{
    int		fontPmask;
    int		fontY;

    /*
     * If pDraw is not the screen
     */
    if ( pDraw->type == UNDRAWABLE_WINDOW)
        return 0;
    if ( pDraw->type == DRAWABLE_PIXMAP)
	return miPolyText8( pDraw, pGC, x0, y0, nChars, pStr);

    fontPmask = LoadFont( pDraw->pScreen, pGC->font, &fontY);
    return tlPolyText( (WindowPtr)pDraw, pGC, x0, y0,
					nChars, pStr, fontPmask, fontY);
}
