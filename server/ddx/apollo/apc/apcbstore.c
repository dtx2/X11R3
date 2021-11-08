/******************************************************************
Copyright 1988 by Apollo Computer Inc., Chelmsford, Massachusetts.
   
                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/* apcbstore.c, dmm, 10/05/88      */

#include    "apc.h"
#include    "X.h"
#include    "mibstore.h"
#include    "regionstr.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "windowstr.h"


void
apcDoBitblt(pDisp, src, dst, rop, prgn, xorg, yorg)
    apDisplayDataPtr    pDisp;          /* for pDisp->lastGC ... */
    gpr_$bitmap_desc_t  src, dst;       /* source, destination bitmaps */
    gpr_$raster_op_t    rop;            /* raster op */
    RegionPtr           prgn;           /* destination-relative region */
    int                 xorg, yorg;     /* source offset */
{
    gpr_$bitmap_desc_t  dbm;            /* incoming bitmap */
    gpr_$window_t       win;            /* src window */
    gpr_$position_t     pos;            /* dst position */
    status_$t		st;

    register BoxPtr	pBox;
    register int	i;

    gpr_$set_clipping_active( false, st );
    gpr_$inq_bitmap( dbm, st );
    gpr_$set_bitmap( dst, st );
    gpr_$set_plane_mask_32( (gpr_$mask_32_t) 0xFFFFFFFF, st );
    gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF, rop, st );

    for (pBox = prgn->rects, i = prgn->numRects; i > 0; i--, pBox++)
    {
        pos.x_coord = pBox->x1;                         /* dst x */
        pos.y_coord = pBox->y1;                         /* dst y */
        win.window_base.x_coord = pBox->x1 + xorg;      /* src x */
        win.window_base.y_coord = pBox->y1 + yorg;      /* src y */
        win.window_size.x_size = pBox->x2 - pBox->x1;   /* x size */
        win.window_size.y_size = pBox->y2 - pBox->y1;   /* y size */

        gpr_$pixel_blt( src, win, pos, st );
    }

    /* put back everything we changed */
    gpr_$set_bitmap( dbm, st );
    gpr_$set_clipping_active( true, st );
    if (pDisp->lastGC)
    {
        gpr_$set_raster_op_mask( (gpr_$mask_32_t) 0xFFFFFFFF,
                                 (gpr_$raster_op_t) pDisp->lastGC->alu, st );
        gpr_$set_plane_mask_32( pDisp->lastGC->planemask, st );
    }
}


/*-
 *-----------------------------------------------------------------------
 * apcSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap. This is very simple to do, since
 *	apcDoBitblt is designed for this very thing. The region to save is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the screen
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the screen into the pixmap.
 *
 *-----------------------------------------------------------------------
 */
void
apcSaveAreas(pPixmap, prgnSave, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnSave; 	/* Region to save (pixmap-relative) */
    int	    	  	xorg;	    	/* X origin of region */
    int	    	  	yorg;	    	/* Y origin of region */
{
    apDisplayDataPtr    pDisp;
    gpr_$bitmap_desc_t  src, dst;

    pDisp = &apDisplayData[pPixmap->drawable.pScreen->myNum];
    src = pDisp->display_bitmap;
    dst = ((apcPrivPMPtr)(pPixmap->devPrivate))->bitmap_desc;

    apcDoBitblt(pDisp, src, dst, (gpr_$raster_op_t)GXcopy, prgnSave, xorg, yorg);
}

/*-
 *-----------------------------------------------------------------------
 * apcRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap. This is very simple to do, since
 *	apcDoBitblt is designed for this very thing. The region to restore is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the pixmap
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the pixmap into the screen.
 *
 *-----------------------------------------------------------------------
 */
void
apcRestoreAreas(pPixmap, prgnRestore, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnRestore; 	/* Region to restore (screen-relative)*/
    int	    	  	xorg;	    	/* X origin of window */
    int	    	  	yorg;	    	/* Y origin of window */
{
    apDisplayDataPtr    pDisp;
    gpr_$bitmap_desc_t  src, dst;

    pDisp = &apDisplayData[pPixmap->drawable.pScreen->myNum];
    src = ((apcPrivPMPtr)(pPixmap->devPrivate))->bitmap_desc;
    dst = pDisp->display_bitmap;

    apcDoBitblt(pDisp, src, dst, (gpr_$raster_op_t)GXcopy, prgnRestore, -xorg, -yorg);
}
