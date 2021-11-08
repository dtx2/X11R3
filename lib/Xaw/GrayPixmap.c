#ifndef lint
static char Xrcsid[] = "$XConsortium: GrayPixmap.c,v 1.21 88/09/27 16:42:05 swick Exp $";
#endif lint


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

#include <stdio.h>
#include <X11/IntrinsicP.h>

typedef struct _PixmapCache {
    Screen *screen;
    Pixmap pixmap;
    Pixel foreground, background;
    unsigned int depth;
    struct _PixmapCache *next;
  } CacheEntry;

static CacheEntry *pixmapCache = NULL;



Pixmap XmuCreateStippledPixmap(screen, fore, back, depth)
    Screen *screen;
    Pixel fore, back;
    unsigned int depth;
/*
 *	Creates a stippled pixmap of specified depth
 *	caches these so that multiple requests share the pixmap
 */
{
    register Display *display = DisplayOfScreen(screen);
    XImage image;
    CacheEntry *cachePtr;
    Pixmap stippled_pixmap;
    GC gc;
    XGCValues gcValues;
    static unsigned short pixmap_bits[] = {
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555,
	0xaaaa, 0xaaaa, 0x5555, 0x5555
    };


/*
 *	Creates a stippled pixmap of depth DefaultDepth(screen)
 *	caches these so that multiple requests share the pixmap
 */

#define pixmap_width 32
#define pixmap_height 32

    /* see if we already have a pixmap suitable for this screen */
    for (cachePtr = pixmapCache; cachePtr; cachePtr = cachePtr->next) {
	if (cachePtr->screen == screen && cachePtr->foreground == fore &&
	    cachePtr->background == back && cachePtr->depth == depth)
	    return( cachePtr->pixmap );
    }

    /* nope, we'll have to construct one now */
    XQueryBestStipple(display, RootWindowOfScreen(screen), pixmap_width,
		      pixmap_height, &image.width, &image.height);
    image.xoffset = 0;
    image.format = XYBitmap;
    image.data = (char*) pixmap_bits;
    image.byte_order = ImageByteOrder(display);
    image.bitmap_pad = BitmapPad(display);
    image.bitmap_bit_order = BitmapBitOrder(display);
    image.bitmap_unit = BitmapUnit(display);
    image.depth = 1;
    image.bytes_per_line = pixmap_width/8;
    image.obdata = NULL;

    stippled_pixmap = XCreatePixmap( display, RootWindowOfScreen(screen), 
				     image.width, image.height, depth);

    /* and insert it at the head of the cache */
    cachePtr = XtNew(CacheEntry);
    cachePtr->screen = screen;
    cachePtr->foreground = fore;
    cachePtr->background = back;
    cachePtr->depth = depth;
    cachePtr->pixmap = stippled_pixmap;
    cachePtr->next = pixmapCache;
    pixmapCache = cachePtr;

    /* now store the image into it */
    gcValues.foreground = fore;
    gcValues.background = back;
    gc = XCreateGC( display, stippled_pixmap,
		    GCForeground | GCBackground, &gcValues );

    XPutImage( display, stippled_pixmap, gc, &image, 0, 0, 0, 0,
	       image.width, image.height);

    XFreeGC( display, gc );

    return( stippled_pixmap );
}


Pixmap XtGrayPixmap (screen)
    Screen *screen;
{
    return XmuCreateStippledPixmap(screen, BlackPixelOfScreen(screen),
					   WhitePixelOfScreen(screen),
				   DefaultDepthOfScreen(screen));
}


Pixmap XtSimpleStippledPixmap (screen, fore, back)
    Screen *screen;
    Pixel fore, back;
{
    return XmuCreateStippledPixmap(screen, fore, back,
				   DefaultDepthOfScreen(screen));
}
