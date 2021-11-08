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
#include "miscstruct.h"
#include "pixmapstr.h"
#include "region.h"
#include "mistruct.h"
#include "gc.h"		/* for NEXT_SERIAL_NUMBER */

#include "Xmd.h"
#include "servermd.h"

#include "qd.h"

PixmapPtr
qdCreatePixmap( pScreen, width, height, depth)
    ScreenPtr pScreen;
    int width;
    int height;
    int depth;
{
    register PixmapPtr pPixmap;
    register QDPixPtr qdpix;
    int		nbytes;

    PixmapPtr mfbCreatePixmap();

    if ( depth == 1)
	return mfbCreatePixmap( pScreen, width, height, depth);

    /*
     * allocate the generic and the private pixmap structs in one chunk
     */
    pPixmap = (PixmapPtr) Xalloc( sizeof(PixmapRec) + sizeof(QDPixRec));
    if ( !pPixmap)
	return (PixmapPtr)NULL;

    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->width = width;
    pPixmap->height = height;
    pPixmap->devKind = 0;		/* not used */
    pPixmap->refcnt = 1;

    qdpix = (QDPixPtr) &pPixmap[1];
    qdpix->lastOrg.x = width;
    qdpix->lastOrg.y = height;
    qdpix->offscreen = NOTOFFSCREEN;
    pPixmap->devPrivate = (pointer)qdpix;

    nbytes = sizeQDdata( pPixmap);
    if (   nbytes <= 0
	|| (qdpix->data = (unsigned char *) Xalloc( nbytes)) == NULL)
    {
    	Xfree( pPixmap);
	return (PixmapPtr)NULL;
    }
    return pPixmap;
}

/*
 * DIX assumes DestroyPixmap may be called with NULL arg
 */
Bool
qdDestroyPixmap( pPixmap)
    PixmapPtr pPixmap;
{
    unsigned char *	data;

    if ( ! IS_VALID_PIXMAP(pPixmap))
	return TRUE;

    if ( pPixmap->drawable.depth == 1)
	return mfbDestroyPixmap( pPixmap);

    if ( --pPixmap->refcnt != 0)
	return TRUE;

    tlCancelPixmap(pPixmap->devPrivate);
    data = ((QDPixPtr) pPixmap->devPrivate)->data;
    if ( data)
	Xfree( data);
    Xfree( pPixmap);  /* frees the QDPixRec at the same time */
    return TRUE;
}


static
PixmapPtr
QD1toNplanes( pBit, fgpixel, bgpixel)
    PixmapPtr   pBit;
    int		fgpixel, bgpixel;
{
    register unsigned char *	newbytes;
    register unsigned char *	oldbits = ((QDPixPtr)pBit->devPrivate)->data;
    register int	r, c;
    register int	pixel;
    

    newbytes = (unsigned char *) Xalloc( sizeQDdata( pBit));
    /*
     * Now expand the bitmap and two colors into the full-depth pixmap
     */
    for ( r=0; r<pBit->height; r++)
	for ( c=0; c<pBit->width; c++)
	{
	    pixel = oldbits[ r*QPPADBYTES( pBit->width) + (c>>3)] & 1<<(c&0x7)
		? fgpixel
		: bgpixel;
	    newbytes[ r*pBit->width+c] = pixel;
#if NPLANES==24
	    newbytes[ (pBit->height+r)*pBit->width + c] = pixel >> 8;
	    newbytes[ ((pBit->height<<1)+r)*pBit->width + c] = pixel >> 16;
#endif
	}

    Xfree( ((QDPixPtr)pBit->devPrivate)->data);
    ((QDPixPtr)pBit->devPrivate)->data = newbytes;
    return pBit;	/* now a PixMap */
}

extern	int	Nchannels;

static
sizeQDdata( pPix)
    PixmapPtr	pPix;
{
    switch ( pPix->drawable.depth)
    {
      case 1:
	return PixmapWidthInPadUnits( pPix->width, 1);
      case NPLANES:
      default:
	return pPix->width * pPix->height * Nchannels;
    }
}
