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

/*
 * Functions implementing Apollo monochrome-specific parts of the driver
 * having to do with the software cursor.
 */

#include "apollo.h"

/*
 * apMonoCursorUp -- Driver internal code
 *      Given a screen number and cursor pointer (cursor is assumed down),
 *      put it up on that screen, at the apEventPosition location.
 */
void
apMonoCursorUp(scrNum, pCurCursor)
    int             scrNum;
    CursorPtr       pCurCursor;
{
    apDisplayDataPtr    pDisp;
    apPrivCursPtr       pPrivC;
    apPrivPointrPtr     pPrivP;
    unsigned long       *bitsToSet;
    unsigned long       *bitsToClear;
    short               *pshortScr, *pshortTmp;
    short               *pshortSav;
    long                *plongScr;
    long                *plongSav;
    int                 wpl;
    int                 x, y;
    int                 nlines;
    int                 shift;
    unsigned long       mask;
    int                 i;

    pDisp = &apDisplayData[scrNum];
    pPrivC = (apPrivCursPtr) pCurCursor->devPriv[scrNum];
    pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;

    x = pPrivP->x - pCurCursor->xhot;
    y = pPrivP->y - pCurCursor->yhot;

    shift = x & 0x000F;
    pshortScr = (short *) pDisp->bitmap_ptr;
    wpl = pDisp->words_per_line;
    pshortScr =  &pshortScr[(y * wpl) + (x >> 4)];
    bitsToClear = (unsigned long *) pPrivC->pRealizedData;
    plongSav = (long *) pPrivC->pStateData;

    nlines = pPrivC->realizedHeight;
    if (y < 0)
    {
        nlines += y;
        pshortScr -= wpl * y;
        bitsToClear -= y;
        y = 0;
    }
    else if (y+nlines >= pDisp->display_char.y_visible_size)
        nlines = pDisp->display_char.y_visible_size - y;

    bitsToSet = bitsToClear + 16;

    /* cursorBox indicates to drawing code and cursorDown code
       what bits will be restored which may be a superset of
       those that were actually touched */
    pPrivC->cursorBox.y1 = y;
    pPrivC->cursorBox.y2 = y + nlines;

    if (x < 0)
    {
        mask = ((unsigned long) 0xFFFF0000) >> (-x);
        pshortTmp = pshortScr + 1;
        pshortSav = (short *) plongSav;
        *pshortSav++ = *pshortTmp;
        plongSav = (long *) pshortSav;
        *pshortTmp &= (~(*bitsToClear++)) >> shift;
        *pshortTmp |=   (*bitsToSet++) >> shift;
        pshortScr += wpl;
        nlines -= 1;
        pPrivC->cursorBox.x1 = 0;
        pPrivC->cursorBox.x2 = 16;
        pPrivC->alignment = 0;
    }
    else if (x+pPrivC->realizedWidth >=
             pDisp->display_char.x_visible_size)
    {
        mask = 0xFFFF0000 << (x + pPrivC->realizedWidth
                            - pDisp->display_char.x_visible_size);
        nlines -= 1;
        pshortTmp = pshortScr + (wpl * nlines);
        plongSav[nlines] = *pshortTmp << 16;
        *pshortTmp &= (~(bitsToClear[nlines])) >> (16+shift);
        *pshortTmp |=   (bitsToSet[nlines]) >> (16+shift);
        pPrivC->cursorBox.x2 = pDisp->display_char.x_visible_size;
        pPrivC->cursorBox.x1 = pPrivC->cursorBox.x2 - 16;
        pPrivC->alignment = 0;
    }
    else
    {
        mask = 0xFFFF0000;
        pPrivC->cursorBox.x1 = x & 0xFFF0;
        if (shift)
            pPrivC->cursorBox.x2 = pPrivC->cursorBox.x1 + 32;
        else
            pPrivC->cursorBox.x2 = pPrivC->cursorBox.x1 + 16;
        pPrivC->alignment = shift;
    }

    plongScr = (long *) pshortScr;
    for (i=0; i<nlines; i++, plongScr += (wpl>>1))
    {
        *plongSav++ = *plongScr;
        *plongScr &= ~(((bitsToClear[i])&mask) >> shift);
        *plongScr |=   ((bitsToSet[i])&mask) >> shift;
    }
}

/*
 * apMonoCursorDown -- Driver internal code
 *      Given a screen number and cursor pointer (cursor is assumed up
 *      on that screen), take it down.
 */
void
apMonoCursorDown (scrNum, pCurCursor)
    int         scrNum;
    CursorPtr   pCurCursor;
{
    apDisplayDataPtr    pDisp;
    apPrivCursPtr       pPrivC;
    short               *pshortScr;
    long                *plongScr;
    short               *pshortSav;
    long                *plongSav;
    int                 nlines;
    int                 wpl;
    int                 i;

    pDisp = &apDisplayData[scrNum];
    pPrivC = (apPrivCursPtr) pCurCursor->devPriv[scrNum];

    wpl = pDisp->words_per_line;

    pshortScr = (short *) pDisp->bitmap_ptr;
    pshortScr =  &pshortScr[(pPrivC->cursorBox.y1 * (wpl))
                          + (pPrivC->cursorBox.x1 >> 4)];
    nlines = pPrivC->cursorBox.y2 - pPrivC->cursorBox.y1;

    if (pPrivC->alignment)
    {
        plongSav = (long *) pPrivC->pStateData;
        plongScr = (long *) pshortScr;
        for (i=0; i<nlines; i++, plongScr += (wpl>>1))
            *plongScr = *plongSav++;
    }
    else    /* must avoid accessing extra words past end of bitmap */
    {
        pshortSav = (short *) pPrivC->pStateData;
        for (i=0; i<nlines; i++, pshortScr += wpl, pshortSav++)
            *pshortScr = *pshortSav++;
    }
}

/*
 * apMonoRealizeCurs --  Driver internal code
 *      Given a cursor record and the cursor-private record for a
 *      monochrome screen, initialize the cursor-private record,
 *      putting as much precomputed data in it as we can.
 */
Bool
apMonoRealizeCurs (pDisp, pCurs, pPrivC)
    apDisplayDataPtr    pDisp;
    CursorPtr       pCurs;
    apPrivCursPtr   pPrivC;
{
    unsigned long   *bitsToSet;
    unsigned long   *bitsToClear;
    unsigned long   *pSrcImg;
    unsigned long   *pSrcMsk;
    unsigned long   *pDstFg;
    unsigned long   *pDstBg;
    unsigned long   widthmask, temp;
    int             i;

    pPrivC->realizedWidth = (pCurs->width <= 16) ? pCurs->width : 16;
    pPrivC->realizedHeight = (pCurs->height <= 16) ? pCurs->height : 16;

    pPrivC->pRealizedData = (pointer) Xalloc (sizeof(long)*32);
    bitsToClear = (unsigned long *) pPrivC->pRealizedData;
    bitsToSet = bitsToClear + 16;
    pPrivC->pStateData = (pointer) Xalloc (sizeof(long)*16);

    bzero((char *)bitsToClear, sizeof(long)*32);

    pSrcImg = (unsigned long *) pCurs->source;
    pSrcMsk = (unsigned long *) pCurs->mask;
    temp = 32 - pPrivC->realizedWidth;
    widthmask = ~((1 << temp) - 1);

    pDstFg = (pCurs->foreRed) ? bitsToSet : bitsToClear;
    pDstBg = (pCurs->backRed) ? bitsToSet : bitsToClear;

    for (i=0; i<(pPrivC->realizedHeight); i++, pSrcImg++, pSrcMsk++)
    {
        *pDstFg++ |= ( (*pSrcImg) & *pSrcMsk) & widthmask;
        *pDstBg++ |= (~(*pSrcImg) & *pSrcMsk) & widthmask; 
    }
    return (TRUE);
}

/*
 * apMonoUnrealizeCurs --  Driver internal code
 *      Given a cursor record and the cursor-private record for a
 *      monochrome screen, deallocate the dynamic storage we allocated
 *      in apMonoRealizeCurs.
 */
Bool
apMonoUnrealizeCurs (pCurs, pPrivC)
    CursorPtr       pCurs;
    apPrivCursPtr   pPrivC;
{
    Xfree (pPrivC->pRealizedData);
    Xfree (pPrivC->pStateData);
    return (TRUE);
}


/* apMonoDisplayCurs -- Driver internal code
 *       Called by apDisplayCursor -- needed by color cursor support,
 *                                    but does nothing here
 */
void
apMonoDisplayCurs (pCurs, pPrivC)
    CursorPtr       pCurs;
    apPrivCursPtr   pPrivC;
{
}

