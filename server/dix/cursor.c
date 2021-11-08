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


/* $XConsortium: cursor.c,v 1.30 88/09/06 15:40:29 jim Exp $ */

#include "X.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "dixfont.h"	/* for CreateRootCursor */
#include "resource.h"	/* for CreateRootCursor */

#include "Xmd.h"
#include "dix.h"	/* for CreateRootCursor  */
#include "opaque.h"	/* for CloseFont  */

/*
 * To be called indirectly by DeleteResource; must use exactly two args
 */
/*ARGSUSED*/
int
FreeCursor( pCurs, cid)
    CursorPtr 	pCurs;
    Cursor 	cid;	
{
    int		nscr;

    ScreenPtr	pscr;

    if ( --pCurs->refcnt > 0)
	return(Success);

    for ( nscr=0, pscr=screenInfo.screen;
	  nscr<screenInfo.numScreens;
	  nscr++, pscr++)
    {
        if ( pscr->UnrealizeCursor)
	    ( *pscr->UnrealizeCursor)( pscr, pCurs);
    }
    xfree( pCurs->source);
    xfree( pCurs->mask);
    xfree( pCurs);
    return(Success);
}

/*
 * does nothing about the resource table, just creates the data structure.
 * allocates no storage.
 */
CursorPtr 
AllocCursor( psrcbits, pmaskbits, cm,
	    foreRed, foreGreen, foreBlue, backRed, backGreen, backBlue)
    unsigned char *	psrcbits;		/* server-defined padding */
    unsigned char *	pmaskbits;		/* server-defined padding */
    CursorMetricPtr	cm;
    unsigned	int foreRed, foreGreen, foreBlue;
    unsigned	int backRed, backGreen, backBlue;
{
    CursorPtr 	pCurs;
    int		nscr;
    ScreenPtr 	pscr;

    pCurs = (CursorPtr )xalloc( sizeof(CursorRec)); 

    pCurs->source = psrcbits;
    pCurs->mask = pmaskbits;

    pCurs->width = cm->width;
    pCurs->height = cm->height;

    pCurs->refcnt = 1;		
    pCurs->xhot = cm->xhot;
    pCurs->yhot = cm->yhot;

    pCurs->foreRed = foreRed;
    pCurs->foreGreen = foreGreen;
    pCurs->foreBlue = foreBlue;

    pCurs->backRed = backRed;
    pCurs->backGreen = backGreen;
    pCurs->backBlue = backBlue;

    /*
     * realize the cursor for every screen
     */
    for ( nscr=0, pscr=screenInfo.screen;
	  nscr<screenInfo.numScreens;
	  nscr++, pscr++)
    {
        if ( pscr->RealizeCursor)
	    ( *pscr->RealizeCursor)( pscr, pCurs);
    }
    return pCurs;
}


/***********************************************************
 * CreateRootCursor
 *
 * look up the name of a font
 * open the font
 * add the font to the resource table
 * make bitmaps from glyphs "glyph" and "glyph + 1" of the font
 * make a cursor from the bitmaps
 * add the cursor to the resource table
 *************************************************************/

CursorPtr 
CreateRootCursor(pfilename, glyph)
    char *		pfilename;
    unsigned short	glyph;
{
    CursorPtr 	curs;
    FontPtr 	cursorfont;
    unsigned char *psrcbits;
    unsigned char *pmskbits;
    CursorMetricRec cm;
    XID		fontID;

    fontID = FakeClientID(0);
    if (cursorfont = OpenFont( (unsigned)strlen( pfilename), pfilename))
	AddResource(
	   fontID, RT_FONT, (pointer)cursorfont, CloseFont, RC_CORE);
    else
	return NullCursor;

    if (!CursorMetricsFromGlyph(cursorfont, glyph+1, &cm))
	return NullCursor;

    if (ServerBitsFromGlyph(fontID, cursorfont, glyph, &cm, &psrcbits))
	return NullCursor;
    if (ServerBitsFromGlyph(fontID, cursorfont, glyph+1, &cm, &pmskbits))
	return NullCursor;

    curs = AllocCursor( psrcbits, pmskbits, &cm, 0, 0, 0, ~0, ~0, ~0);

    AddResource(FakeClientID(0), RT_CURSOR, (pointer)curs, FreeCursor, RC_CORE);
    return curs;
}


