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
 * Functions implementing Apollo clrchrome-specific parts of the driver
 * having to do with the software cursor.
 */

#include "apollo.h"

#define savex APCURSOR_SIZE
#define savey 0
#define imagex 0
#define imagey 0

/*
 * apClrCursorUp -- Driver internal code
 *      Given a screen number and cursor pointer, if the cursor is down,
 *      put it up on that screen, at the apEventPosition location.
 */
void
apClrCursorUp(scrNum, pCurCursor)
    int             scrNum;
    CursorPtr       pCurCursor;
{
    apDisplayDataPtr    pDisp;
    apPrivCursPtr       pPrivC;
    apPrivPointrPtr     pPrivP;
    gpr_$bitmap_desc_t  dbm;                             /* incoming bitmap */
    gpr_$window_t       sw, cw;
    short               cx, cy;
    status_$t		    st;

    pDisp = &apDisplayData[scrNum];
    pPrivC = (apPrivCursPtr) pCurCursor->devPriv[scrNum];
    pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;

    gpr_$set_clipping_active( false, st );
    gpr_$inq_bitmap( dbm, st );

    sw.window_base.x_coord = pPrivP->x - pCurCursor->xhot;
    sw.window_base.y_coord = pPrivP->y - pCurCursor->yhot;
    cw.window_base.x_coord = savex;
    cw.window_base.y_coord = savey;
    sw.window_size.x_size = pPrivC->realizedWidth;
    sw.window_size.y_size = pPrivC->realizedHeight;
    cx = imagex;
    cy = imagey;

    /* clip it to screen */
    /* note that the saved image is always at savex,savey to simplify restoring it */
    if (sw.window_base.x_coord < 0) {
        cx -= sw.window_base.x_coord;   /* cursor source coordinate for later */
        sw.window_size.x_size += sw.window_base.x_coord;
        sw.window_base.x_coord = 0;
    }
    else if (sw.window_base.x_coord+sw.window_size.x_size > pDisp->display_char.x_visible_size)
        sw.window_size.x_size = pDisp->display_char.x_visible_size - sw.window_base.x_coord;
    if (sw.window_base.y_coord < 0) {
        cy -= sw.window_base.y_coord;   /* cursor source coordinate for later */
        sw.window_size.y_size += sw.window_base.y_coord;
        sw.window_base.y_coord = 0;
    }
    else if (sw.window_base.y_coord+sw.window_size.y_size > pDisp->display_char.y_visible_size)
        sw.window_size.y_size = pDisp->display_char.y_visible_size - sw.window_base.y_coord;

    cw.window_size = sw.window_size;

    pPrivC->cursorBox.x1 = sw.window_base.x_coord;
    pPrivC->cursorBox.y1 = sw.window_base.y_coord;
    pPrivC->cursorBox.x2 = sw.window_base.x_coord + sw.window_size.x_size - 1;
    pPrivC->cursorBox.y2 = sw.window_base.y_coord + sw.window_size.y_size - 1;

    /* save current contents */
    gpr_$set_bitmap( pDisp->HDMCursor, st );
    gpr_$set_plane_mask_32( (gpr_$mask_32_t) 0xFFFFFFFF, st );
    gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF, (gpr_$raster_op_t) GXcopy, st );
    gpr_$pixel_blt( pDisp->display_bitmap, sw, cw.window_base, st );

    /* clear all mask bits */
    cw.window_base.x_coord = cx;
    cw.window_base.y_coord = cy;
    gpr_$set_bitmap( pDisp->display_bitmap, st );
    gpr_$set_plane_mask_32( (gpr_$mask_32_t) 0xFFFFFFFF, st );
    gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF, (gpr_$raster_op_t) 4, st );
    gpr_$additive_blt( pDisp->HDMCursor, cw, (gpr_$plane_t) 1, sw.window_base, st );

    /* or in image bits */
    gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF, (gpr_$raster_op_t) 7, st );
    gpr_$pixel_blt( pDisp->HDMCursor, cw, sw.window_base, st );

    /* put back everything we changed */
    if (dbm != pDisp->display_bitmap)
        gpr_$set_bitmap( dbm, st );
    gpr_$set_clipping_active( true, st );
    if ( pDisp->lastGC ) {
        gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF,
                             (gpr_$raster_op_t) pDisp->lastGC->alu, st );
        gpr_$set_plane_mask_32( pDisp->lastGC->planemask, st );
    }
}

/*
 * apClrCursorDown -- Driver internal code
 *      Given a screen number and cursor pointer, if the cursor is up
 *      on that screen, take it down.
 */
void
apClrCursorDown (scrNum, pCurCursor)
    int         scrNum;
    CursorPtr   pCurCursor;
{
    apDisplayDataPtr    pDisp;
    apPrivCursPtr       pPrivC;
    gpr_$bitmap_desc_t  dbm;                             /* incoming bitmap */
    gpr_$window_t       cw;
    gpr_$position_t     sp;
    status_$t		    st;

    pDisp = &apDisplayData[scrNum];
    pPrivC = (apPrivCursPtr) pCurCursor->devPriv[scrNum];

    gpr_$inq_bitmap( dbm, st );
    gpr_$set_clipping_active( false, st );

    sp.x_coord = pPrivC->cursorBox.x1;
    sp.y_coord = pPrivC->cursorBox.y1;
    cw.window_size.x_size = pPrivC->cursorBox.x2 - pPrivC->cursorBox.x1 + 1;
    cw.window_size.y_size = pPrivC->cursorBox.y2 - pPrivC->cursorBox.y1 + 1;
    cw.window_base.x_coord = savex;
    cw.window_base.y_coord = savey;

    /* restore current contents */
    if (dbm != pDisp->display_bitmap)
        gpr_$set_bitmap( pDisp->display_bitmap, st );
    gpr_$set_plane_mask_32( (gpr_$mask_32_t) 0xFFFFFFFF, st );
    gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF, (gpr_$raster_op_t) GXcopy, st );
    gpr_$pixel_blt( pDisp->HDMCursor, cw, sp, st );

    /* put back everything we changed */
    if (dbm != pDisp->display_bitmap)
        gpr_$set_bitmap( dbm, st );
    gpr_$set_clipping_active( true, st );
    if ( pDisp->lastGC ) {
        gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF,
                             (gpr_$raster_op_t) pDisp->lastGC->alu, st );
        gpr_$set_plane_mask_32( pDisp->lastGC->planemask, st );
    }

}

/*
 * apClrDisplayCursor -- Driver internal code
 *      Given a screen number and cursor pointer, initialize the hdm cursor
 *      bitmap, and set the foreground and background colors
 */
void
apClrDisplayCursor(scrNum, pCurCursor)
    int         scrNum;
    CursorPtr   pCurCursor;
{
    apDisplayDataPtr    pDisp;
    apPrivCursPtr       pPrivC;
    gpr_$bitmap_desc_t  dbm;                             /* incoming bitmap */
    gpr_$window_t       cw;
    unsigned long       *sbits, *cbits;
    int                 i, lwpl;
    gpr_$color_t        cColors[2];
    status_$t		    st;

    pDisp = &apDisplayData[scrNum];
    pPrivC = (apPrivCursPtr) pCurCursor->devPriv[scrNum];

    gpr_$inq_bitmap( dbm, st );

    gpr_$set_bitmap( pDisp->HDMCursor, st );
    gpr_$set_clipping_active( false, st );
    gpr_$set_plane_mask_32( (gpr_$mask_32_t) 0xFFFFFFFF, st );
    gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF, (gpr_$raster_op_t) GXcopy, st );

    /* blt the cursor image into hdm from the mm bitmap */
    cw.window_size.x_size = pPrivC->realizedWidth;
    cw.window_size.y_size = pPrivC->realizedHeight;
    cw.window_base.x_coord = 0;
    cw.window_base.y_coord = 0;
    gpr_$pixel_blt( pPrivC->pRealizedData, cw, cw.window_base, st );

    /* put cursor colors into color map */
    cColors[0] = (pCurCursor->backBlue >> 8)
               | (pCurCursor->backGreen & 0xFF00)
               | ((pCurCursor->backRed & 0xFF00) << 8);
    cColors[1] = (pCurCursor->foreBlue >> 8)
               | (pCurCursor->foreGreen & 0xFF00)
               | ((pCurCursor->foreRed & 0xFF00) << 8);
    gpr_$set_color_map( pDisp->cursBackPix, (short) 2, *cColors, st );

    /* put back everything we changed */
    gpr_$set_bitmap( dbm, st );
    gpr_$set_clipping_active( true, st );
    if ( pDisp->lastGC ) {
        gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF,
                             (gpr_$raster_op_t) pDisp->lastGC->alu, st );
        gpr_$set_plane_mask_32( pDisp->lastGC->planemask, st );
    }

}

/*
 * apClrRealizeCursor -- DDX interface (screen)
 *      Given a clrchrome screen and cursor, realize the given cursor
 *      for the given clrchrome screen.  We allocate the cursor private
 *      structure, and put as much precomputed data in it as we can.
 */
Bool
apClrRealizeCursor(pDisp, pCurs, pPrivC)
    apDisplayDataPtr    pDisp;
    CursorPtr           pCurs;
    apPrivCursPtr       pPrivC;
{
    status_$t           st;
    gpr_$offset_t       size;
    unsigned long       *cbits, *sbits;
    short               wpl;
    int                 i, j;

    pPrivC->realizedWidth = (pCurs->width <= APCURSOR_SIZE) ? pCurs->width : APCURSOR_SIZE;
    pPrivC->realizedHeight = (pCurs->height <= APCURSOR_SIZE) ? pCurs->height : APCURSOR_SIZE;

    size.x_size = pPrivC->realizedWidth;
    size.y_size = pPrivC->realizedHeight;
    gpr_$allocate_bitmap( size, (gpr_$rgb_plane_t)(pDisp->depth-1), gpr_$nil_attribute_desc,
                          pPrivC->pRealizedData, st );
    gpr_$inq_bitmap_pointer( pPrivC->pRealizedData, sbits, wpl, st );

    /* source bits go in plane zero */
    cbits = (unsigned long *) pCurs->source;
    wpl = wpl >> 1;
    for (i=0; i<pPrivC->realizedHeight; i++ ) {
        *sbits = *cbits++;
        sbits += wpl;
    }
    /* mask bits go in planes 1 thru depth-1 */
    for (j=1; j<pDisp->depth; j++) {
        cbits = (unsigned long *) pCurs->mask;
        for (i=0; i<pPrivC->realizedHeight; i++ ) {
            *sbits = *cbits++;
            sbits += wpl;
        }
    }

    return (TRUE);
}

/*
 * apClrUnrealizeCursor -- DDX interface (screen)
 *      Given a cursor record and the cursor-private record for a
 *      monochrome screen, deallocate the dynamic storage we allocated
 *      in apMonoRealizeCurs.
 */
Bool
apClrUnrealizeCursor (pCurs, pPrivC)
    CursorPtr       pCurs;
    apPrivCursPtr   pPrivC;
{
    status_$t           st;

    gpr_$deallocate_bitmap( pPrivC->pRealizedData, st );

    return (TRUE);
}
