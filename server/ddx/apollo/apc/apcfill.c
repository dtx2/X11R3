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

/* apcFill.c, jah, 01/19/88      */

/* Calls to gpr to support X server fill operations */

#include "apc.h"
#include "Xprotostr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmap.h"
#include "pixmapstr.h"
#include "misc.h"


static Bool
set_tile( pPM, sNum, xorg, yorg )
    PixmapPtr pPM;
    int       sNum;
    int       xorg, yorg;
{
    int       xscale, yscale;
    int       i, j;
    gpr_$bitmap_desc_t  sbm = ((apcPrivPMPtr) pPM->devPrivate)->bitmap_desc;   /* source bitmap */
    gpr_$bitmap_desc_t  tbm;                                                    /* tile bitmap */
    gpr_$bitmap_desc_t  dbm;                                                    /* destination bitmap */
    gpr_$window_t       swin;
    gpr_$position_t     dpos;
    status_$t           st;


    xscale = TILE_SIZE / pPM->width;
    if (pPM->width * xscale != TILE_SIZE) return (FALSE);
    yscale = TILE_SIZE / pPM->height;
    if (pPM->height * yscale != TILE_SIZE) return (FALSE);

    xorg &= pPM->width - 1;
    yorg &= pPM->height - 1;

    if (xscale == 1 && yscale == 1 && xorg == 0 && yorg == 0) {
        /* bitmap is perfect as is */
        gpr_$set_fill_pattern( sbm, (short) 1, st );
        if (st.all == status_$ok)
            return( TRUE );
    }

    if (pPM->drawable.depth == 1)
        tbm = apDisplayData[sNum].opStip_bitmap;
    else
        tbm = apDisplayData[sNum].tile_bitmap;

    gpr_$inq_bitmap( dbm, st );
    apc_$set_bitmap( tbm );

    if (xorg) {
        xscale++;
        xorg -= pPM->width;
    }
    if (yorg) {
        yscale++;
        yorg -= pPM->height;
    }

    swin.window_base.x_coord = 0;
    swin.window_base.y_coord = 0;
    swin.window_size.x_size = pPM->width;
    swin.window_size.y_size = pPM->height;
    dpos.y_coord = yorg;
    for (i=0; i<yscale; i++) {
        dpos.x_coord = xorg;
        for (j=0; j<xscale; j++) {
            gpr_$pixel_blt( sbm, swin, dpos, st );
            if (st.all != status_$ok) return(FALSE);
            dpos.x_coord += swin.window_size.x_size;
        }
        dpos.y_coord += swin.window_size.y_size;
    }

    apc_$set_bitmap( dbm );  /* put this back now */

    gpr_$set_fill_pattern( tbm, (short) 1, st );
    if (st.all != status_$ok) return(FALSE);

    return( TRUE );
}


void
apcValidateTile( pDraw, pGC )
    DrawablePtr     pDraw;
    GCPtr           pGC;
{
    PixmapPtr           tsp;   /* tile/stipple pixmap */
    short               xorg, yorg;
    status_$t           st;

    switch (pGC->fillStyle) {
	case FillSolid:
        pGC->PolyFillRect = apcPolyFillRect;
        gpr_$set_fill_pattern( gpr_$nil_bitmap_desc, (short) 1, st );
	    break;
	case FillTiled:
	case FillOpaqueStippled:
        tsp = (pGC->fillStyle==FillTiled) ? pGC->tile : pGC->stipple;
        xorg = pGC->patOrg.x;
        yorg = pGC->patOrg.y;
        if (pDraw->type == DRAWABLE_WINDOW) {
            xorg += ((WindowPtr)pDraw)->absCorner.x;
            yorg += ((WindowPtr)pDraw)->absCorner.y;
        }
        if (set_tile( tsp, pGC->pScreen->myNum, xorg, yorg ))
            pGC->PolyFillRect = apcPolyFillRect;
        else
            pGC->PolyFillRect = miPolyFillRect;
	    break;
	case FillStippled:
        pGC->PolyFillRect = miPolyFillRect;
	    break;
	default:
	    FatalError("apcValidateGC: illegal fillStyle\n");
	}
}


void
apcPolyFillRect( pDraw, pGC, nrectFill, pRects )
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*pRects;  	/* Pointer to first rectangle to fill */
{
    short               i, j;
    short               xorg, yorg;
    Bool                weclip;
    status_$t           st;
    RegionPtr           pClipRgn = ((apcPrivGCPtr)(pGC->devPriv))->pCompositeClip;
    BoxPtr              pClipRct;
    gpr_$window_t       grect;

    /* turn off clipping since we clip ourselves -- its faster */
    weclip = pClipRgn->numRects != 1;
    if (weclip) gpr_$set_clipping_active( false, st );

    if (pDraw->type == DRAWABLE_WINDOW) {
        xorg = ((WindowPtr)pDraw)->absCorner.x;
        yorg = ((WindowPtr)pDraw)->absCorner.y;
    }
    else {
        xorg = 0;
        yorg = 0;
    }

    for (i=0; i<nrectFill; i++) {
        if (weclip) {
            pClipRct = pClipRgn->rects;
            for (j=0; j<pClipRgn->numRects; j++) {
                grect.window_base.x_coord = max(pRects->x+xorg, pClipRct->x1);
                grect.window_base.y_coord = max(pRects->y+yorg, pClipRct->y1);
                grect.window_size.x_size = min( pRects->x+xorg+pRects->width, pClipRct->x2 )
                                         - grect.window_base.x_coord;
                grect.window_size.y_size = min( pRects->y+yorg+pRects->height, pClipRct->y2 )
                                         - grect.window_base.y_coord;
                if (grect.window_size.x_size > 0 && grect.window_size.y_size > 0)
                    gpr_$rectangle( grect, st );
                pClipRct++;
            }
        }
        else {
            grect.window_base.x_coord = pRects->x + xorg;
            grect.window_base.y_coord = pRects->y + yorg;
            grect.window_size.x_size  = pRects->width;
            grect.window_size.y_size  = pRects->height;
            gpr_$rectangle( grect, st );
        }
        pRects++;
    }

    if (weclip) gpr_$set_clipping_active( true, st );

}


void
apcPaintWindow( pWin, prgn, what )
WindowPtr pWin;
RegionPtr prgn;
int what;
{
    gpr_$window_t       grect;
    short               i;
    BoxPtr              prect;
    status_$t           st;

    /* invalidate the currently validated GC, since we will trash its fill info and rops and bitmap */
    if (apDisplayData[pWin->pScreen->myNum].lastGC)
       apDisplayData[pWin->pScreen->myNum].lastGC->serialNumber |= GC_CHANGE_SERIAL_BIT;

    apc_$set_bitmap( apDisplayData[pWin->pScreen->myNum].display_bitmap );

    gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF, (gpr_$raster_op_t) GXcopy, st );
    if (what == PW_BACKGROUND)
    {
        if (pWin->backgroundTile == None)
            return;
        else if ((int)pWin->backgroundTile == ParentRelative)
        {
            (*pWin->parent->PaintWindowBackground)(pWin->parent, prgn, what);
            return;
        }
        else if ((int)pWin->backgroundTile == USE_BACKGROUND_PIXEL)
        {
            gpr_$set_fill_pattern( gpr_$nil_bitmap_desc, (short) 1, st );
            gpr_$set_fill_value( pWin->backgroundPixel, st );
        }
        else /* an actual tile pixmap */
        {
            if (!set_tile( pWin->backgroundTile, pWin->pScreen->myNum,
                           pWin->absCorner.x, pWin->absCorner.y ))
            {
                miPaintWindow( pWin, prgn, what );
                return;
            }
        }
    }
    else /* what == PW_BORDER */
    {
        if (pWin->borderTile == None)
            return;
        else if ((int)pWin->borderTile == USE_BORDER_PIXEL)
        {
            gpr_$set_fill_pattern( gpr_$nil_bitmap_desc, (short) 1, st );
            gpr_$set_fill_value( pWin->borderPixel, st );
        }
        else
        {
            if (!set_tile( pWin->borderTile, pWin->pScreen->myNum,
                           pWin->absCorner.x, pWin->absCorner.y ))
            {
                miPaintWindow( pWin, prgn, what );
                return;
            }
        }
    }
    
    gpr_$set_clipping_active( false, st );
    gpr_$set_plane_mask_32( (gpr_$mask_32_t) 0xFFFFFFFF, st );

    prect = prgn->rects;
    for (i=0; i<prgn->numRects; i++)
    {
        grect.window_base.x_coord = prect->x1;
        grect.window_base.y_coord = prect->y1;
        grect.window_size.x_size  = prect->x2 - prect->x1;
        grect.window_size.y_size  = prect->y2 - prect->y1;
        gpr_$rectangle( grect, st );
        prect++;
    }

    gpr_$set_clipping_active( true, st );

}
