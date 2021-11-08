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

#include "apc.h"
#include "Xmd.h"
#include "servermd.h"
#include "pixmapstr.h"

PixmapPtr
apcCreatePixmap (pScreen, width, height, depth, format)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
    int		format;
{
    PixmapPtr   pPixmap;
    gpr_$offset_t               size;
    gpr_$attribute_desc_t	attrib;
    gpr_$rgb_plane_t            hi_plane;
    gpr_$bitmap_desc_t          bitmap;
    short			gprwidth;
    status_$t   status;

    pPixmap = (PixmapPtr)Xalloc(sizeof(PixmapRec));
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->width = width;
    pPixmap->height = height;
    pPixmap->devKind = PixmapBytePad(width, depth);
    pPixmap->refcnt = 1;

    if ( !(pPixmap->devPrivate = (pointer)Xalloc(sizeof(apcPrivPM)))) {
	Xfree(pPixmap);
	return (PixmapPtr)NULL;
        }

    size.x_size = width;
    size.y_size = height;
    hi_plane = (gpr_$rgb_plane_t)(depth-1);
    gpr_$allocate_bitmap( size, hi_plane, apDisplayData[pScreen->myNum].attribute_block, 
                          bitmap, status);

    ((apcPrivPMPtr)(pPixmap->devPrivate))->bitmap_desc = bitmap;
    ((apcPrivPMPtr)(pPixmap->devPrivate))->size = size;
          
    gpr_$inq_bitmap_pointer(bitmap, ((apcPrivPMPtr)(pPixmap->devPrivate))->bitmap_ptr, gprwidth, status);

    ((apcPrivPMPtr)(pPixmap->devPrivate))->width = (int)gprwidth;
    return pPixmap;
}

Bool
apcDestroyPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    status_$t status;
/* BOGOSITY ALERT */
    if ((unsigned)pPixmap < 42)
	return TRUE;

    if(--pPixmap->refcnt)
	return TRUE;
    
    gpr_$deallocate_bitmap(((apcPrivPMPtr)(pPixmap->devPrivate))->bitmap_desc, status);

    Xfree(pPixmap->devPrivate);
    Xfree(pPixmap);
    return TRUE;
}
