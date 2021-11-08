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

#include "Xmd.h"
#include "servermd.h"
#include "misc.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "scrnintstr.h"

#include "qd.h"
#include "qdprocs.h"

extern	int	Nplanes;

void
qdGetImage( pDraw, x, y, w, h, format, planemask, pImage)
    DrawablePtr		pDraw;
    int			x, y, w, h;
    unsigned int	format;
    unsigned long	planemask;
    unsigned char	*pImage;
{
    switch( format)
    {
      case XYPixmap:
	miGetImage( pDraw, x, y, w, h, format, planemask, pImage);
	break;
      case ZPixmap:
	if ( pDraw->type == DRAWABLE_WINDOW)
	    tlgetimage( pDraw, x, y, w, h, planemask, pImage);
	else
	    miGetImage( pDraw, x, y, w, h, format, planemask, pImage);
	break;
    }
    if ((pDraw->type == DRAWABLE_WINDOW) &&
	(((WindowPtr)pDraw)->backingStore != NotUseful))
    {
	miBSGetImage( pDraw, (PixmapPtr) 0, x, y, w, h, format, planemask, pImage);
    }
}

void
qdPixPutImage( pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage)
    DrawablePtr		pDraw;
    GCPtr		pGC;
    int			depth, x, y, w, h, leftPad;
    unsigned int	format;
    unsigned char	*pImage;
{
    switch( format)
    {
      case XYBitmap:
	if ( pDraw->depth == NPLANES)
	{
	    bitmapToPixmap( (PixmapPtr)pDraw, pGC, x, y, w, h,
							leftPad, pImage);
	    break;
	}
	/* otherwise, fall through */
      case XYPixmap:
	miPutImage( pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage);
	break;
      case ZPixmap:
	imageToPixmap( pDraw, pGC, x, y, w, h, pImage );
	break;
    }
}

void
qdWinPutImage( pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage)
    WindowPtr		pDraw;
    GCPtr		pGC;
    int			depth, x, y, w, h, leftPad;
    unsigned int	format;
    unsigned char	*pImage;
{
    switch( format)
    {
      case XYBitmap:
	if (leftPad == 0 && pGC->alu == GXcopy)
	{
	    bitmapToScr( (WindowPtr)pDraw, pGC, x, y, w, h, leftPad, pImage);
	    break;
	}
      case XYPixmap:
	miPutImage( pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage);
	break;
      case ZPixmap:
	tlputimage( pDraw, pGC, x, y, w, h, pImage );
	break;
    }
}

/*
 * use the bitmap to select back or fore color from GC
 */
static
bitmapToScr( pWin, pGC, x, y, w, h, leftPad, pImage)
    WindowPtr		pWin;
    GCPtr               pGC;
    int                 x, y, w, h, leftPad;
    unsigned char       *pImage;
{
    PixmapPtr	ptempBMap;
    char *	pUnusedBits;
    int         ic;     /* clip rect index */

    x += pWin->absCorner.x;
    y += pWin->absCorner.y;

    /*
     * create a temporary bitmap and transplant pImage into it
     */
    ptempBMap = mfbCreatePixmap( pWin->drawable.pScreen, w+leftPad, h, 1);
    pUnusedBits = (char *)ptempBMap->devPrivate; /* knowledge of
						mfb implementation */
    ptempBMap->devPrivate = pImage;

    for ( ic=0; ic<((QDPrivGCPtr)pGC->devPriv)->pCompositeClip->numRects; ic++)
	tlBitmapBichrome( pGC, ptempBMap, pGC->fgPixel, pGC->bgPixel,
	    x, y, &((QDPrivGCPtr)pGC->devPriv)->pCompositeClip->rects[ic]);

    /*
     * clean up the pointers and destroy the bitmap
     */
    ptempBMap->devPrivate = (pointer) pUnusedBits;
    mfbDestroyPixmap( ptempBMap);
}

/*
 * use the bitmap to select back or fore color from GC
 */
static
bitmapToPixmap( pPix, pGC, x, y, w, h, leftPad, pImage)
    PixmapPtr           pPix;
    GCPtr               pGC;
    int                 x, y, w, h, leftPad;
    unsigned char       *pImage;
{
    int			ic;	/* clip rect index */
    BoxPtr		pc = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip->rects;

    int			br;	/* bitmap row */
    register int	bc;	/* bitmap column */
    register unsigned *	pbr;	/* pointer to bitmap row */
    int			bwlongs = PixmapWidthInPadUnits( w, 1); /* bitmap width
							    in longs */

    DDXPointRec		mp;	/* pixmap point */

    /*
     * for each clipping rectangle
     */
    for ( ic=0; ic<((QDPrivGCPtr)pGC->devPriv)->pCompositeClip->numRects; ic++, pc++)
    {
	int	brlast = min( h, pc->y2-y);  /* last bitmap row to paint */
	  
	/*
	 * for each row in the intersection of bitmap source and dest clip
	 */
	for ( br = max( 0, pc->y1-y), mp.y = y + br,
		pbr = (unsigned *) &pImage[ br*(bwlongs<<2) ];
	      br < brlast;
	      br++, mp.y++, pbr+=bwlongs
	    )
	{
	    int	bclast = leftPad + min( w, pc->x2-x);

	    /*
	     * for each column in the intersection
	     */
	    for ( bc = leftPad + max( 0, pc->x1-x), mp.x = x + (bc-leftPad);
		  bc < bclast;
		  bc++, mp.x++)
	    {
		/*
                 * examine each bit.
                 */
		/* XXX - now uses qddopixel below
		unsigned char	pixel;
                pixel = ( pbr[ bc>>5] & (1 << (bc&0x1f)))
			    ? pGC->fgPixel
			    : pGC->bgPixel;
		*/
		qddopixel((pbr[ bc>>5] & (1 << (bc&0x1f)))?
		    (unsigned char *) &pGC->fgPixel:
		    (unsigned char *) &pGC->bgPixel, pPix, &mp, pGC);
	    }
	}
    }
}

/*
 * transfer an image into a pixmap
 */
static
imageToPixmap( pPix, pGC, x, y, w, h, pImage)
    PixmapPtr           pPix;
    GCPtr               pGC;
    int                 x, y, w, h;
    unsigned char       *pImage;
{
    register unsigned char	*pcolor;	/* current z-val */
    register int		minx, miny, maxx, maxy;
    RegionPtr	pSaveGCclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    register BoxPtr	pclip = pSaveGCclip->rects;
    register int	nclip = pSaveGCclip->numRects;
    DDXPointRec		mp;	/* pixmap point */

    /*
     * for each clipping rectangle
     */
    for ( ; nclip > 0; nclip--, pclip++)
    {
	minx = max(x,	pclip->x1);
	miny = max(y,	pclip->y1);
	maxx = min(x+w,	pclip->x2);
	maxy = min(y+h,	pclip->y2);
	for (mp.y = miny; mp.y < maxy; mp.y++)
	{
	    pcolor = pImage + (NPLANES/8)*((mp.y-y)*pPix->width + (minx-x));
	    for (mp.x = minx; mp.x < maxx; mp.x++)
	    {
		qddopixel(pcolor, pPix, &mp, pGC);
		pcolor += (NPLANES/8);
	    }
	}
    }
}

