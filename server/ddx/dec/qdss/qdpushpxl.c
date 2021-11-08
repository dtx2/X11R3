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

#include "X.h"
#include "Xmd.h"
#include "gcstruct.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"

#include "../../mfb/maskbits.h"

#include "qd.h"

/*
 * xxPushPixels
 * pBitMap is a stencil (dx by dy of it is used, it may
 * be bigger) which is placed on the drawable at xOrg, yOrg.  Where a 1 bit
 * is set in the bitmap, the foreground pattern is put onto the drawable using
 * the GC's logical function. The drawable is not changed where the bitmap
 * has a zero bit or outside the area covered by the stencil.
*/
void
qdPushPixels( pGC, pBitMap, pDrawable, dx, dy, xOrg, yOrg)
    GCPtr		pGC;
    PixmapPtr		pBitMap;
    DrawablePtr 	pDrawable;
    int			dx, dy, xOrg, yOrg;
{
    void		qdPPpixmap();

    WindowPtr		pWin;		/* used when pDrawable is a window */
    PixmapPtr		pBMoutput;
    RegionPtr		pcl;
    int			ic;

    /*
     * If pDrawable is not the screen
     */
    if ( pDrawable->type == UNDRAWABLE_WINDOW)
	return;
    if ( pDrawable->type == DRAWABLE_PIXMAP)
    {
	miPushPixels( pGC, pBitMap, pDrawable, dx, dy, xOrg, yOrg);
	return;
    }

    /*
     * output to the screen
     *
     * This implementation is based on the paint library's processor-to-bitmap
     * text output code, and uses the p-to-b text template routine with
     * no changes.
     *
     * It currently relies on the paint library; these dependencies must be
     * removed eventually.  XXX
     */
    pWin = (WindowPtr)pDrawable;

    /*
     * create a temporary copy of the bitmap and use mfb code to stipple it,
     * if fileStyle requires it
     */
    switch (pGC->fillStyle)
    {
      case FillOpaqueStippled:	/* bag out for two-color tiles
					pushed through the bitmap */
	miPushPixels( pGC, pBitMap, pDrawable, dx, dy, xOrg, yOrg);
	return;
      case FillStippled:  /* use mfb code to OR the stipple with the bitmap */
	{
	GCPtr	ptempGC = GetScratchGC( 1, pDrawable->pScreen);
	GC	cgc;	/* helps change ptempGC */
        PixmapPtr	psavestipple;

	pBMoutput = (* ptempGC->pScreen->CreatePixmap)
			    ( ptempGC->pScreen, dx, dy, 1 /*depth*/);
	cgc.alu = GXcopy;
	cgc.fgPixel = 1;		/* mfb white */
	cgc.fillStyle = FillStippled;
	cgc.stipple = pGC->stipple;
	cgc.stateChanges = GCFunction|GCForeground|GCFillStyle|GCStipple;
	QDChangeGCHelper( ptempGC, &cgc);
	ValidateGC( pBMoutput, ptempGC);
	(* ptempGC->PushPixels)( ptempGC, pBitMap, pBMoutput, dx, dy, 0, 0);
	(* ptempGC->pScreen->DestroyPixmap)( pBMoutput);
	FreeScratchGC( ptempGC);
	}
	break;
      case FillSolid:
	pBMoutput = pBitMap;
	break;
      case FillTiled:	/* bag out for tiles pushed through the bitmap */
	miPushPixels( pGC, pBitMap, pDrawable, dx, dy, xOrg, yOrg);
	return;
    }

    /*
     * This can eventually be done in ValidateGC	XXX
     */
    xOrg += pWin->absCorner.x;
    yOrg += pWin->absCorner.y;

    /*
     * now do the real work
     */
    pcl = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    for (   ic=0;
	    ic<pcl->numRects;
	    ic++)
    {
	switch (pGC->fillStyle)
	{
	  case FillStippled:
	  case FillSolid:
	    {
	    BoxRec	cb;
	
	    cb.x1 = max( pcl->rects[ic].x1, xOrg);
	    cb.y1 = max( pcl->rects[ic].y1, yOrg);
	    cb.x2 = min( pcl->rects[ic].x2, xOrg+dx);
	    cb.y2 = min( pcl->rects[ic].y2, yOrg+dy);

	    if (   cb.x1 < cb.x2
		&& cb.y1 < cb.y2)
		tlBitmapStipple( pGC, pBMoutput, pGC->fgPixel, pGC->bgPixel,
			    xOrg, yOrg, &cb);
	    break;
	    }
	}
    }
}

/*
 * This should be faster than miPushPixels to a Pixmap, because this
 * routine can take advantage of knowledge of the dragon-specific
 * pixmap representation.
 *	DON'T NEED SPEED YET, SO THIS IS NOT CALLED	XXX
 */
#define MAX3( a, b, c)	( (a)>(b)&&(a)>(c) ? (a) : (((b)>(c))?(b):(c)))
#define MIN3( a, b, c)	( (a)<(b)&&(a)<(c) ? (a) : (((b)<(c))?(b):(c)))

extern	int	Nplanes;
extern	int	Nentries;
extern	int	Nchannels;

static void
qdPPpixmap( pGC, pBitMap, pDrawable, dx, dy, xOrg, yOrg)
    GCPtr		pGC;
    PixmapPtr		pBitMap;
    PixmapPtr	 	pDrawable;
    int			dx, dy, xOrg, yOrg;
{
    PixmapPtr		pDp = (PixmapPtr) pDrawable; /* destination Pixmap */
    int			ch;	/* channel index */
    int			ic;	/* clip box index */

    /*
     * for each channel in destination pixmap
     */
    for ( ch=0; ch<Nchannels; ch++)
    {
	RegionPtr		pcl;
	unsigned char *pd =
	    ((QDPixPtr)pDp->devPrivate)->data + ch*pDp->width*pDp->height;
	 
	/*
	 * for each clipping rectangle
	 */
	pcl = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
	for ( ic=0; ic<pcl->numRects; ic++)
	{
	    BoxPtr	p1box = &pcl->rects[ic];

	    qdPPPix1chan( pGC, pBitMap, pDrawable, ch, 
		    MAX3( 0, xOrg, p1box->x1),
		    MAX3( 0, yOrg, p1box->y1),
		    MIN3( pDrawable->width, xOrg+dx, p1box->x2),
		    MIN3( pDrawable->height, yOrg+dy, p1box->y2),
		    xOrg, yOrg);
	}
    }
}


#define FATAL(x)    \
	{ fprintf(stderr, "fatal error in qdspans, %s\n", x); exit(1) }
#define PIXDEPTH(x) ((x->drawable).depth)

/*
 * box clipped by caller
 * bitmap could be clipped too
 */
static
qdPPPix1chan( pGC, pBitMap, pPixmap, ch, x1, y1, x2, y2, xOrg, yOrg)
    GCPtr		pGC;		/* note that clipping is already done */
    PixmapPtr		pBitMap;
    PixmapPtr		pPixmap;
    int			ch;		/* which group of 8 planes? */
    int			x1, y1, x2, y2;	/* clipping rectangle in pixmap coords*/
    int                 xOrg, yOrg;     /* origin of bitmap within pixmap */
{
#ifdef undef
    int			mask[];

    register int	chmask = 0xff << (ch*8);
    u_char *		destorig = ((QDPixPtr)pPixmap->devPrivate)->data
				    [ nch * pPixmap->width * pPixmap->height]; 
    int			swlongs = 
		pBitMap->devKind>>2;	/* width of source bitmap in longs */
    unsigned *		psr;		/* pointer to bitmap source row */
    int			sr;		/* bitmap source row */
    int			sc;		/* bitmap source column */
    int			dr;		/* destination row */
    int			dc;		/* destination column */
    int			bit;		/* effectively, a boolean */
    char		bytetab[256];


    switch (pGC->fillStyle) {
      case FillOpaqueStippled:
      case FillStippled:
      case FillSolid:
        {
	initializeLookup( bytetab, pGC);/*accounts for output state in the GC*/

	/*
	 * for each row in source bitmap
	 */
	for (	sr = y1-yOrg, psr = (unsigned *)pBitmap->devPrivate+swlongs*sr;
		sr < y2-yOrg;
		sr++, psr += swlongs)
	    /*
	     * for each column in source bitmap
	     */
	    for ( sc=x1-xOrg; sc < x2-xOrg; sc++)
	    {
		/*
		 * examine each bit.  "mask" is from maskbits.c
		 */
		bit = psr[ sc>>5] & mask[ sc&0x1f] ? 1 : 0;
		/*
		 * apply alu function and write result
		 */
		pdr[ dc] = bytetab[ pdr[ dc]] [ bit];
	    }
	}
	break;
      case FillTiled:
	break;
    }
#endif
}


static
initializeLookup( bytetab, pGC)
    char	bytetab[];
    GCPtr               pGC;

{
}
