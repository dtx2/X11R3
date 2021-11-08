/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.

                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/* ap_text.c, jah, 12/15/87      */

/*   code to call gpr_$text in support of X server text   */

#include    "apollo.h"
#include	"Xmd.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include    "screenint.h"
#include    "region.h"
#include    "window.h"
#include    "gc.h"
#include    "miscstruct.h"
#include    "misc.h"
#include    "apc.h"
#include    "ap_text.h"

/* these routines all make internal copies of the text, of maximum size MAXCHRS, and
   then they process larger strings in batches */
#define MAXCHRS 100

static int curTextFG = -3;
static int curTextBG = -3;

static gpr_$window_t nocrect = { 0, 0, 0, 0 };


/* check for intersection of this text drawing and remove the cursor if it is up */

static void check_crsr( cBox, pGC, x, y, count )
    BoxRec cBox;
    GCPtr  pGC;
    int    x, y;
    int    count;
{
    int         i;
    FontInfoPtr pfi = pGC->font->pFI;
    RegionPtr   pclip;
    BoxPtr      prect;

    /* rather than compute the real bounding box, we compute a safely larger one, but not too awfully
       much larger.  We use the maxbounds metrics as though each glyph had those bounds, rather than
       examining the bounds of each actual glyph.  Furthermore, we assume imagetext, in which the
       cleared background may in principle be larger than the bounds of the text itself */

    cBox.y1 = max( y - max( pfi->fontAscent, pfi->maxbounds.metrics.ascent ), cBox.y1 );
    cBox.y2 = min( y + max( pfi->fontDescent, pfi->maxbounds.metrics.descent ) - 1, cBox.y2 );
    if (cBox.y1 > cBox.y2) return;
    cBox.x1 = max( x + min( 0, pfi->minbounds.metrics.leftSideBearing ), cBox.x1 );
    cBox.x2 = min( x + (count-1) * pfi->maxbounds.metrics.characterWidth
              + max( pfi->maxbounds.metrics.characterWidth, pfi->maxbounds.metrics.rightSideBearing ), cBox.x2 );
    if (cBox.x1 > cBox.x2) return;

    /* if we get here, we know that the cursor box and the text bounding box intersect, ignoring
       clipping */

    pclip = ((apcPrivGCPtr) (pGC->devPriv)) -> pCompositeClip;
    if (pclip->numRects < 5) { /* if there are really a lot of clip rectangles, don't bother -- assume one
                                  of them will intersect */
        prect = pclip->rects;
        for (i=0; i<pclip->numRects; i++) {
            if (prect->x1 <= cBox.x2 &&
                prect->y1 <= cBox.y2 &&
                prect->x2 >= cBox.x1 &&
                prect->y2 >= cBox.y1) {
                apRemoveCursor();
                return;
            }
            prect++;
        }
    }
    else
        apRemoveCursor();
}


/* Examine each glyph for (a) existence, and (b) width.  Return overall width, plus
   a new string in which missing characters are replaced by chDefault.  This code is
   cloned from (dix)getGlyphs which doesn't do what is needed */

static int rep_dfltch( pGC, fontEncoding, chars, cpp, count )
    GCPtr	pGC;
    FontEncoding fontEncoding;
    unsigned char *chars, *cpp;
    int count;
{
    CharInfoPtr		pCI = pGC->font->pCI;
    FontInfoPtr		pFI = pGC->font->pFI;
    unsigned int	firstCol = pFI->firstCol;
    unsigned int	numCols = pFI->lastCol - firstCol + 1;
    unsigned int	firstRow = pFI->firstRow;
    unsigned int	numRows = pFI->lastRow - firstRow + 1;
    unsigned int	chDefault = pFI->chDefault;
    int	            i;
    unsigned int	c;
    int             width;

    if (fontEncoding == Linear16Bit && pFI->lastRow > 0)
        fontEncoding = TwoD16Bit;

    width = 0;

    switch (fontEncoding) {

	case Linear8Bit:
	case TwoD8Bit:
	    for (i=0; i < count; i++) {

    		c = *chars - firstCol;
    		if (c < numCols && pCI[c].exists) {
                *cpp++ = *chars++;
                width += pCI[c].metrics.characterWidth;
    		}
            else {
                *cpp++ = chDefault;
                chars++;
        		c = chDefault - firstCol;
        		if (c < numCols && pCI[c].exists)
                    width += pCI[c].metrics.characterWidth;
            }
	    }
	    break;

	case Linear16Bit:
	    for (i=0; i < count; i++) {

    		if (*chars++) 
                c = chDefault;
            else
                c = *chars++;
            *cpp++ = 0;

            *cpp = c;
    		c -= firstCol;
    		if (c < numCols && pCI[c].exists) {
                cpp++;
                width += pCI[c].metrics.characterWidth;
    		}
            else {
                *cpp++ = chDefault;
        		c = chDefault - firstCol;
        		if (c < numCols && pCI[c].exists)
                    width += pCI[c].metrics.characterWidth;
            }
	    }
	    break;

	case TwoD16Bit:
	    for (i=0; i < count; i++) {
    		unsigned int row;
    		unsigned int col;

    		row = (*chars) - firstRow;
    		col = (*chars) - firstCol;
    		if ((row < numRows) && (col < numCols)) {
    		    c = row*numCols + col;
    		    if (pCI[c].exists) {
                    *cpp++ = *chars++;
                    *cpp++ = *chars++;
                    width += pCI[c].metrics.characterWidth;
                    continue;
                }
    		}

            *cpp++ = chDefault >> 8;
            *cpp++ = chDefault;
            chars += 2;

    		row = (chDefault >> 8)-firstRow;
    		col = (chDefault & 0xff)-firstCol;
    		if ((row < numRows) && (col < numCols)) {
    		    c = row*numCols + col;
    		    if (pCI[c].exists)
                    width += pCI[c].metrics.characterWidth;
    		}
	    }
	    break;
    }
    return( width );
}


/* gprText8 does the work for both gprPolyText8 and gprImageText8 -- they differ only
   in the colors they use  */

static int gprText8(pGC, x, y, count, chars, fgcolor, bgcolor, crect)
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
    int     fgcolor, bgcolor;
    gpr_$window_t *crect;
{
    status_$t       st;
    apcPrivGCPtr   dpp = (apcPrivGCPtr) pGC->devPriv;
    gprFIDPtr       gfp = (gprFIDPtr) (pGC->font->devPriv[pGC->pScreen->myNum]);
    short           clipx, cx;
    gpr_$window_t   gwin;
    BoxPtr          pclip;
    short           finuse, fwanted;
    char            *startp, *curp;
    short           newx, newy;

    /* set colors if necessary */
    if (curTextFG != fgcolor) {
        gpr_$set_text_value( fgcolor, st );
        curTextFG = fgcolor;
    }
    if (curTextBG != bgcolor) {
        gpr_$set_text_background_value( bgcolor, st );
        curTextBG = bgcolor;
    }

    pclip = dpp->pCompositeClip->rects;
    for ( clipx = 0; clipx < dpp->pCompositeClip->numRects; clipx++ ) {

        gpr_$move( (short) x, (short) (y - 1), st );

        if (dpp->pCompositeClip->numRects != 1) {
            gpr_$window_t gwin;
            gwin.window_base.x_coord = pclip->x1;
            gwin.window_base.y_coord = pclip->y1;
            gwin.window_size.x_size = pclip->x2 - gwin.window_base.x_coord;
            gwin.window_size.y_size = pclip->y2 - gwin.window_base.y_coord;
            gpr_$set_clip_window( gwin, st );
            pclip++;
        }

        if (crect->window_size.x_size)
            gpr_$rectangle( *crect, st );

        if (gfp->nGprFonts == 1)
            gpr_$text( *chars, (short) count, st );

        else {   /* must build separate strings and use different fonts */
            startp = chars;
            curp = chars;
            finuse = -1;
            for ( cx=0; cx<count; cx++ ) {
                fwanted = *curp >> 7;
                if (fwanted != finuse) {
                    if (startp != curp) {
                        gpr_$text( *startp, (short) (curp-startp), st );
                        startp = curp;
                    }
                    gpr_$set_text_font( gfp->fontIds[fwanted], st );
                    finuse = fwanted;
                }
                curp++;
            }
            if (startp != curp)
                gpr_$text( *startp, (short) (curp-startp), st );

        }
    }
    gpr_$inq_cp( newx, newy, st );

    gpr_$enable_direct_access( st );

    return( newx );
}

int gprPolyText8(pDraw, pGC, x, y, count, chars)
    WindowPtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    FontInfoPtr pfi = pGC->font->pFI;
    BoxRec    cursorBox;
    int       rval = 0;
    int       i;
    char      chrcopy[MAXCHRS];
    short     xorg;

    /* handle long strings */
    while (count > MAXCHRS) {
        rval += gprPolyText8(pDraw, pGC, x+rval, y, MAXCHRS, chars);
        count -= MAXCHRS;
        chars += MAXCHRS;
    }

    if (pDraw->type == DRAWABLE_WINDOW) {
        /* apply coordinate origin */
        xorg = pDraw->absCorner.x;
        x += xorg;
        y += pDraw->absCorner.y;
        /* remove cursor if necessary */
        if (apCursorLoc (pDraw->pScreen, &cursorBox))
            check_crsr( cursorBox, pGC, x, y, count );
    }
    else xorg = 0;

    /* rval will be set if we have called ourselves already. */
    /* if this is so, then we want to use rval as x, not what came in */
    if ( rval )
      x = rval;

    /* replace missing characters with chDefault */
    if (pfi->allExist) {
        for (i=0; i<count; i++)
            if (chars[i] >= pfi->firstCol &&
                chars[i] <= pfi->lastCol)
                chrcopy[i] = chars[i];
            else
                chrcopy[i] = pfi->chDefault;
    }
    else
        rep_dfltch( pGC, Linear8Bit, chars, chrcopy, count );

    /* go actually draw the text */
    return( rval - xorg + gprText8( pGC, x, y, count, chrcopy,
            ((apcPrivGCPtr)(pGC->devPriv))->polyTextVal, gpr_$transparent, &nocrect ));
}


int gprImageText8(pDraw, pGC, x, y, count, chars)
    WindowPtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    FontInfoPtr     pfi = pGC->font->pFI;
    int             i;
    status_$t       st;
    gpr_$window_t   gwin;
    BoxRec          cursorBox;
    int             rval = 0;
    char            chrcopy[MAXCHRS];
    short           xorg;

    /* handle long strings */
    while (count > MAXCHRS) {
        rval += gprImageText8(pDraw, pGC, x+rval, y, MAXCHRS, chars);
        count -= MAXCHRS;
        chars += MAXCHRS;
    }

    if (pDraw->type == DRAWABLE_WINDOW) {
        /* apply coordinate origin */
        xorg = pDraw->absCorner.x;
        x += xorg;
        y += pDraw->absCorner.y;
        /* remove cursor if necessary */
        if (apCursorLoc (pDraw->pScreen, &cursorBox))
            check_crsr( cursorBox, pGC, x, y, count );
    }
    else xorg = 0;

    /* rval will be set if we have called ourselves already. */
    /* if this is so, then we want to use rval as x, not what came in */
    if ( rval )
      x = rval;
 

    /* replace missing characters with chDefault */
    if (pfi->allExist && pfi->constantMetrics) {
        for (i=0; i<count; i++)
            if (chars[i] >= pfi->firstCol &&
                chars[i] <= pfi->lastCol)
                chrcopy[i] = chars[i];
            else
                chrcopy[i] = pfi->chDefault;
    }
    else
        gwin.window_size.x_size = rep_dfltch( pGC, Linear8Bit, chars, chrcopy, count );

    /* fill the background rectangle, if necessary */

    if (pfi->terminalFont)
        return( rval + gprText8( pGC, x, y, count, chrcopy, pGC->fgPixel, pGC->bgPixel, &nocrect ));

    else {  /* must fill background */
        if (pfi->constantMetrics)
            gwin.window_size.x_size = count * pfi->maxbounds.metrics.characterWidth;

     /* else width got computed above */

        gwin.window_base.x_coord = x;
        gwin.window_base.y_coord = y - pfi->fontAscent;
        gwin.window_size.y_size = pfi->fontAscent + pfi->fontDescent;
        gpr_$set_fill_value( pGC->bgPixel, st );

        rval += gprText8( pGC, x, y, count, chrcopy, pGC->fgPixel, gpr_$transparent, &gwin )
                - xorg;
        gpr_$set_fill_value( pGC->fgPixel, st );  /* put this back when done */
        return( rval );
    }
}


/* gprText16 does the work for both gprPolyText16 and gprImageText16 -- they differ only
   in the colors they use  */

static int gprText16(pGC, x, y, count, chars, fgcolor, bgcolor, crect)
    GCPtr	pGC;
    int		x, y;
    int 	count;
    unsigned short *chars;
    int     fgcolor, bgcolor;
    gpr_$window_t *crect;
{
    status_$t       st;
    apcPrivGCPtr   dpp = (apcPrivGCPtr) pGC->devPriv;
    gprFIDPtr       gfp = (gprFIDPtr) pGC->font->devPriv[pGC->pScreen->myNum];
    short           clipx, cx;
    BoxPtr          pclip;
    short           finuse, fwanted;
    unsigned short  *curp;
    short           bcx;
    short           newx, newy;
    char            charStr[MAXCHRS];

    /* set colors if necessary */
    if (curTextFG != fgcolor) {
        gpr_$set_text_value( fgcolor, st );
        curTextFG = fgcolor;
    }
    if (curTextBG != bgcolor) {
        gpr_$set_text_background_value( bgcolor, st );
        curTextBG = bgcolor;
    }

    pclip = dpp->pCompositeClip->rects;
    for ( clipx = 0; clipx < dpp->pCompositeClip->numRects; clipx++ ) {

        gpr_$move( (short) x, (short) (y - 1), st );

        if (dpp->pCompositeClip->numRects != 1) {
            gpr_$window_t gwin;
            gwin.window_base.x_coord = pclip->x1;
            gwin.window_base.y_coord = pclip->y1;
            gwin.window_size.x_size = pclip->x2 - gwin.window_base.x_coord;
            gwin.window_size.y_size = pclip->y2 - gwin.window_base.y_coord;
            gpr_$set_clip_window( gwin, st );
            pclip++;
        }

        if (crect->window_size.x_size)
            gpr_$rectangle( *crect, st );

        bcx = 0;
        curp = chars;
        finuse = -1;
        for ( cx=0; cx<count; cx++ ) {
            fwanted = *curp >> 7;
            if (fwanted != finuse) {
                if (bcx > 0) {
                    if (finuse < gfp->nGprFonts) gpr_$text( *charStr, bcx, st );
                    bcx = 0;
                }
                if (fwanted < gfp->nGprFonts) gpr_$set_text_font( gfp->fontIds[fwanted], st );
                finuse = fwanted;
            }
            charStr[bcx++] = *curp++ & 0x7F;
        }
        if (bcx > 0 && finuse < gfp->nGprFonts) gpr_$text( *charStr, bcx, st );
    }
    gpr_$inq_cp( newx, newy, st );
    gpr_$enable_direct_access( st );
    return( newx );
}


int gprPolyText16(pDraw, pGC, x, y, count, chars)
    WindowPtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    unsigned short 	*chars;
{
    FontInfoPtr pfi = pGC->font->pFI;
    BoxRec    cursorBox;
    int       rval = 0;
    int       i;
    short     xorg;
    unsigned short  chrcopy[MAXCHRS];

    /* handle long strings */
    while (count > MAXCHRS) {
        rval += gprPolyText16(pDraw, pGC, x+rval, y, MAXCHRS, chars);
        count -= MAXCHRS;
        chars += MAXCHRS;
    }

    if (pDraw->type == DRAWABLE_WINDOW) {
        /* apply coordinate origin */
        xorg = pDraw->absCorner.x;
        x += xorg;
        y += pDraw->absCorner.y;
        /* remove cursor if necessary */
        if (apCursorLoc (pDraw->pScreen, &cursorBox))
            check_crsr( cursorBox, pGC, x, y, count );
    }
    else xorg = 0;

    /* rval will be set if we have called ourselves already. */
    /* if this is so, then we want to use rval as x, not what came in */
    if ( rval )
      x = rval;

    /* replace missing characters with chDefault */
    if (pfi->allExist) {
        for (i=0; i<count; i++)
            if (chars[i] >= pfi->firstCol &&
                chars[i] <= pfi->lastCol)
                chrcopy[i] = chars[i];
            else
                chrcopy[i] = pfi->chDefault;
    }
    else
        rep_dfltch( pGC, Linear16Bit, chars, chrcopy, count );

    /* go actually draw the text */
    return( rval - xorg + gprText16( pGC, x, y, count, chrcopy,
            ((apcPrivGCPtr)(pGC->devPriv))->polyTextVal, gpr_$transparent, &nocrect ));
}


int gprImageText16(pDraw, pGC, x, y, count, chars)
    WindowPtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short 	*chars;
{
    FontInfoPtr     pfi = pGC->font->pFI;
    int             i;
    status_$t       st;
    gpr_$window_t   gwin;
    BoxRec          cursorBox;
    int             rval = 0;
    unsigned short  chrcopy[MAXCHRS];
    short           xorg;

    /* handle long strings */
    while (count > MAXCHRS) {
        rval += gprImageText16(pDraw, pGC, x+rval, y, MAXCHRS, chars);
        count -= MAXCHRS;
        chars += MAXCHRS;
    }

    if (pDraw->type == DRAWABLE_WINDOW) {
        /* apply coordinate origin */
        xorg = pDraw->absCorner.x;
        x += xorg;
        y += pDraw->absCorner.y;
        /* remove cursor if necessary */
        if (apCursorLoc (pDraw->pScreen, &cursorBox))
            check_crsr( cursorBox, pGC, x, y, count );
    }
    else xorg = 0;

    /* rval will be set if we have called ourselves already. */
    /* if this is so, then we want to use rval as x, not what came in */
    if ( rval )
      x = rval;

    /* replace missing characters with chDefault */
    if (pfi->allExist && pfi->constantMetrics) {
        for (i=0; i<count; i++)
            if (chars[i] >= pfi->firstCol &&
                chars[i] <= pfi->lastCol)
                chrcopy[i] = chars[i];
            else
                chrcopy[i] = pfi->chDefault;
    }
    else
        gwin.window_size.x_size = rep_dfltch( pGC, Linear16Bit, chars, chrcopy, count );

    /* fill the background rectangle, if necessary */

    if (pfi->terminalFont)
        return( rval + gprText16( pGC, x, y, count, chrcopy, pGC->fgPixel, pGC->bgPixel, &nocrect ));

    else {  /* must fill background */
        if (pfi->constantMetrics)
            gwin.window_size.x_size = count * pfi->maxbounds.metrics.characterWidth;

     /* else width got computed above */

        gwin.window_base.x_coord = x;
        gwin.window_base.y_coord = y - pfi->fontAscent;
        gwin.window_size.y_size = pfi->fontAscent + pfi->fontDescent;
        gpr_$set_fill_value( pGC->bgPixel, st );

        rval += gprText16( pGC, x, y, count, chrcopy, pGC->fgPixel, gpr_$transparent, &gwin )
                - xorg;
        gpr_$set_fill_value( pGC->fgPixel, st );  /* put this back when done */
        return( rval );
    }
}


int nopText(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    return(x);  /* return value doesn't really matter - its only used within dispatcher,
                   never stored */
}

