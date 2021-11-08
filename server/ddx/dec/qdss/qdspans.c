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

#include "scrnintstr.h"

#include "pixmapstr.h"

#include <sys/types.h>

#include "X.h"
#include "windowstr.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "dixstruct.h"

#include "Xproto.h"
#include "Xprotostr.h"
#include "mi.h"
#include "Xmd.h"

/*
 * driver headers
 */
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdioctl.h"
#include "qdreg.h"

#include "qd.h"

#include "qdprocs.h"

/*
 * used as procedure vector when drawable is an undrawable window
 */
void
qdFSUndrawable()
{
}

#define	PIXtoDATA(x)	(((QDPixPtr) (x->devPrivate))->data)
#define	PIXDEPTH(x)	((x->drawable).depth)
/* ZCOPY - XXX replaced by does-everything function qddopixel! */
#if NPLANES==24
# define ZCOPY(s,d,sw,dw) \
       {*((unsigned char *) d) = *((unsigned char *) s); \
	*(((unsigned char *) d)+dw) = *(((unsigned char *) s)+sw); \
	*(((unsigned char *) d)+2*(dw)) = *(((unsigned char *) s)+2*(sw));}
#else	/* NPLANES == 8 */
# define ZCOPY(s,d,sw,dw) \
       {*((unsigned char *) d) = *((unsigned char *) s);}
#endif


#define u_char	unsigned char

/*
 * GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of longwords.
 */
/*
 * Currently, the caller does an Xfree of the returned value.
 *
 * If Drawable is the screen
 *	hand the bytes back a pixel at a time.
 * If Drawable is a pixmap
 *	hand the bytes back a pixel at a time.
 */

extern	int Nentries;
extern	int Nplanes;
extern	int Nchannels;

unsigned int *
qdGetSpans( pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
    static int *	qdGSPixFull();
    unsigned int *	pret;
    DDXPointPtr	  	pptInit;
    int	    	  	*pwidthInit;
    int	    	  	*pwidthPadded;
    int	    	  	i;

    if ( pDrawable->type == UNDRAWABLE_WINDOW)
	return (unsigned int *)NULL;

    if ( pDrawable->depth == 1 && pDrawable->type == DRAWABLE_PIXMAP)
	return mfbGetSpans( pDrawable, wMax, ppt, pwidth, nspans);

    pptInit = ppt;
    pwidthInit = pwidth;

    pwidthPadded = (int *)ALLOCATE_LOCAL(nspans * sizeof(int));
    if (pwidthPadded == (int *)NULL)
    {
	return (NULL);
    }

    /*
     * It's a full depth Pixmap or a window.
     * Use an upper bound for the number of bytes to allocate.
     */
    if ( pDrawable->depth == Nplanes)
	pret = (unsigned int *) Xalloc( wMax * nspans * Nchannels);
    else
	return (unsigned int *)NULL;

    if ( pDrawable->type == DRAWABLE_WINDOW)
    {
	int	ns;			/* span count */
	u_char *  pr = (u_char *) pret;

	for ( ns=0; ns<nspans; ns++, pwidth++, ppt++)
	{
	    /*
	     *  order these come back in: RGBRGB...
	     */
	    tlgetspan((WindowPtr) pDrawable, ppt->x, ppt->y, *pwidth, pr);
	    pr += *pwidth * Nchannels;
	    pwidthPadded[ns] = *pwidth * Nchannels;
	}
	/*
	 * If the drawable is a window with some form of backing-store, consult
	 * the backing-store module to fetch any invalid spans from the window's
	 * backing-store. The pixmap is made into one long scanline and the
	 * backing-store module takes care of the rest. We do, however, have
	 * to tell the backing-store module exactly how wide each span is, padded
	 * to the correct boundary, so we allocate pwidthPadded and set those
	 * widths into it.
	 */
	if (((WindowPtr)pDrawable)->backingStore != NotUseful)
	{
	    PixmapPtr pPixmap;
	    QDPixPtr  pQDPix;

	    pQDPix = (QDPixPtr)ALLOCATE_LOCAL(sizeof(QDPixRec));
	    pPixmap = (PixmapPtr)ALLOCATE_LOCAL(sizeof(PixmapRec));
	    if ((pPixmap != (PixmapPtr)NULL) && (pwidthPadded != (int *)NULL))
	    {
		pPixmap->drawable.type = DRAWABLE_PIXMAP;
		pPixmap->drawable.pScreen = pDrawable->pScreen;
		pPixmap->drawable.depth = pDrawable->depth;
		pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
		pPixmap->width = wMax;
		pPixmap->height = 1;
		pPixmap->refcnt = 1;
		pPixmap->devPrivate = (pointer)pQDPix;
		pQDPix->data = (u_char *) pret;
		pQDPix->lastOrg.x = wMax;
		pQDPix->lastOrg.y = 1;
		pQDPix->offscreen = NOTOFFSCREEN;
		miBSGetSpans(pDrawable, pPixmap, wMax, pptInit, pwidthInit,
			 pwidthPadded, nspans);
	    }
	    DEALLOCATE_LOCAL(pQDPix);
	    DEALLOCATE_LOCAL(pPixmap);
	}

	return  pret;
    }
    else	/* DRAWABLE_PIXMAP */
    {
	qdGSPixFull( (PixmapPtr)pDrawable, wMax, ppt, pwidth, nspans, pret);
	return pret;
    }
}

void
qdGSPixFull( pPix, wMax, ppt, pwidth, nspans, pret)
    PixmapPtr		pPix;
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    int *		pret;		/* return value */
{
    u_char *	ps = ((QDPixPtr)pPix->devPrivate)->data;
    register u_char	*psr, *psg, *psb;	/* red, green, blue */
    u_char *	pd = (u_char *)pret;
    int		ns;				/* span count */
    register int	sc;			/* source column */
    register int	pixwidth = pPix->width;

    psr = ps;
    psg = ps +   pixwidth*pPix->height;
    psb = ps + 2*pixwidth*pPix->height;

    for ( ns=0; ns<nspans; ns++, pwidth++, ppt++)
	for ( sc=0; sc<*pwidth; sc++)
	{
	    *pd++ = psr[ ppt->y*pixwidth + ppt->x + sc]; 
#if NPLANES==24
	    *pd++ = psg[ ppt->y*pixwidth + ppt->x + sc]; 
	    *pd++ = psb[ ppt->y*pixwidth + ppt->x + sc]; 
#endif
	}
}
void
qdSetSpans(pDraw, pGC, pPixels, pPoint, pWidth, n, fSorted)
    DrawablePtr pDraw;
    GC *pGC;
    pointer pPixels;
    DDXPointPtr pPoint;
    int *pWidth;
    int n;
    int fSorted;
{
    int		ispan;  /* counts spans */
    int		ipix;	/* indexes pPixels; knows about pixel size */
    int		j;
    int		ic;	/* clip rect index */
    BoxPtr      pc = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip->rects;
    DDXPointRec	qdPoint;

#ifdef DEBUG
    if ((pDraw->depth) == 1)
	FatalError("bitmap passed to qdss code\n");
#endif

    if (pWidth[0] <= 0 || n <= 0)
	return;

    switch ((char) (pDraw->type)) {
      case DRAWABLE_WINDOW:
	for (   ispan = ipix = 0;
		ispan < n;
		ipix += pWidth[ispan++]*Nchannels)
	{
#if NPLANES==24
	    /*
	     * convert from rgb, rgb, rgb to
	     * all red, all green, all blue for tlsetspan's convenience
	     */
	    unsigned char *	newpix;
	    int		j, off;
	    register int	isnf;	/* index to server-natural */

	    if ( pWidth[ispan] <= 0)
		continue;
	    newpix = (unsigned char *)
		alloca(pWidth[ispan] * 3 * sizeof(unsigned char));
	    for (j=0, isnf=ipix; j<pWidth[ispan]; j++) {
		for (off = 0; off < 3; off++) {
		    newpix[j+pWidth[ispan]*off] =
			pPixels[isnf++];
		}
	    }
	    tlsetspan((WindowPtr) pDraw, pGC, pPoint[ispan].x,
		pPoint[ispan].y, pWidth[ispan], newpix);
#else	/* NPLANES == 8 */
	    if ( pWidth[ispan] <= 0)
		continue;
	    tlsetspan((WindowPtr) pDraw, pGC, pPoint[ispan].x,
		pPoint[ispan].y, pWidth[ispan], &pPixels[ipix]);
#endif
	}
	break;
      case UNDRAWABLE_WINDOW:
	return;
      case DRAWABLE_PIXMAP:
	tlCancelPixmap( ((PixmapPtr) pDraw)->devPrivate);
	/*
	 * for each clipping rectangle
	 */
	for ( ic=0;
	      ic<((QDPrivGCPtr)pGC->devPriv)->pCompositeClip->numRects;
	      ic++, pc++)
	{
	    /*
	     * for each scan
	     */
	    for (   ispan = ipix = 0;
		    ispan < n;
		    ipix += pWidth[ispan++]*Nchannels)
	    {
		if (  pc->y2 <= pPoint[ispan].y || pc->y1 > pPoint[ispan].y)
		    continue;
		for ( j = 0; j < pWidth[ispan]; j++) /* each pt */
		{
		    if (	pc->x1 > pPoint[ispan].x+j
			     || pc->x2 <= pPoint[ispan].x+j)
			continue;
		    qdPoint.x = pPoint[ispan].x + j;
		    qdPoint.y = pPoint[ispan].y;
		    qddopixel( &pPixels[ipix+j*Nchannels],
				(PixmapPtr) pDraw,
				&qdPoint,
				pGC);
		}    /* for j (inc on scan) */
	    }    /* for ispan (scan) */
	}
	break;
    }    /* switch (type of dest object: window/pixmap) */
}


/*
 * FillSpans cases
 */

/*
 * For now, hack arg list and call qdFillRectOddSize.
 */
void
qdWinFSOddSize( pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr	pDraw;
    GC		*pGC;
    int		nInit;
    DDXPointPtr	pptInit;
    int		*pwidthInit;
    int		fSorted;
{
    xRectangle * prect;          /* Pointer to first rectangle to fill */
    int		nr;
    xRectangle * pr;

    pr = prect = (xRectangle *) alloca( nInit*sizeof(xRectangle));
    for (   nr=nInit, pr=prect;
	    nr>0;
	    nr--, pr++, pptInit++, pwidthInit++)
    {
	pr->x = pptInit->x;
	pr->y = pptInit->y;
	pr->width = *pwidthInit;
	pr->height = 1;
    }
    qdPolyFillRectOddSize( pDraw, pGC, nInit, prect);
}

#define	CLIPSPANS \
    register DDXPointPtr	ppt; \
    register int *		pwidth; \
    int		n; \
    n = nInit * miFindMaxBand(((QDPrivGCPtr) (pGC->devPriv))->pCompositeClip);\
    if ( n == 0) \
	return; \
    pwidth = (int *)alloca( n * sizeof(int)); \
    ppt = (DDXPointRec *)alloca( n * sizeof(DDXPointRec)); \
    if ( !ppt || !pwidth) \
	FatalError("alloca failed in qd FillSpans.\n"); \
    n = miClipSpans(((QDPrivGCPtr)(pGC->devPriv))->pCompositeClip, \
	pptInit, pwidthInit, nInit, ppt, pwidth, fSorted)

void
qdFSPixSolid(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr	pDraw;
    GC		*pGC;
    int		nInit;
    DDXPointPtr	pptInit;
    int		*pwidthInit;
    int		fSorted;
{
    CLIPSPANS;

    tlCancelPixmap( ((PixmapPtr) pDraw)->devPrivate);
    for ( ; n > 0; n--, pwidth++, ppt++)
    {
	for ( ; *pwidth; (*pwidth)--, (ppt->x)++)
	    qddopixel( &pGC->fgPixel, (PixmapPtr)pDraw, ppt, pGC);
    }
}

void
qdFSPixTiled(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr	pDraw;
    GC		*pGC;
    int		nInit;
    DDXPointPtr	pptInit;
    int		*pwidthInit;
    int		fSorted;
{
    register int	tbase, tinc, tbit;
    unsigned long	scratch;
    CLIPSPANS;

    tlCancelPixmap( ((PixmapPtr) pDraw)->devPrivate);
#ifdef DEBUG
    if (PIXDEPTH(pGC->tile) < Nplanes)
	FatalError("tile is not full-depth pixmap\n");
#endif
    for ( ; n > 0; n--, ppt++, pwidth++)
    {
	tbase = ((ppt->y) % (pGC->stipple->height))
		*QPPADBYTES(pGC->tile->width);
    	for ( ; *pwidth > 0; (ppt->x)++, *(pwidth)--)
	{
	    tinc = tbase + (ppt->x) % (pGC->tile->width);
	    ZCOPY(PIXtoDATA(pGC->tile) + tinc, &scratch,
		pGC->tile->width*pGC->tile->height, 1);
	    qddopixel(&scratch, (PixmapPtr) pDraw, ppt, pGC);
	}
    }
}

void
qdFSPixStippleorOpaqueStip( pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr	pDraw;
    GC		*pGC;
    int		nInit;
    DDXPointPtr	pptInit;
    int		*pwidthInit;
    int		fSorted;
{
    int	tbase;	/* address of first byte in a stipple row */
    int	tinc;	/* byte address within a stipple row */
    int	tbit;	/* bit address within a stipple byte */
    CLIPSPANS;

    tlCancelPixmap( ((PixmapPtr) pDraw)->devPrivate);
#ifdef DEBUG
    if (PIXDEPTH(pGC->stipple) != 1)
	FatalError("stipple is not bitmap\n");
#endif
    for ( ; n > 0; n--, ppt++, pwidth++)
    {
	tbase = UMOD(ppt->y-pGC->patOrg.y, pGC->stipple->height)
		    * QPPADBYTES(pGC->stipple->width);
    	for ( ; *pwidth > 0; ppt->x++, (*pwidth)--)
    	{
	    tinc = tbase + UMOD(ppt->x+pGC->patOrg.x, pGC->stipple->width)/8;
	    tbit = UMOD( ppt->x+pGC->patOrg.x, 8);
	    if ( pGC->fillStyle == FillStippled)
	    {
		if ((*((char *)pGC->stipple->devPrivate + tinc) >> tbit) & 1)
		    qddopixel( &pGC->fgPixel, pDraw, ppt, pGC);
	    }
	    else	/* FillOpaqueStippled */
	    {
		if ((*((char *)pGC->stipple->devPrivate + tinc) >> tbit) & 1)
		    qddopixel( &pGC->fgPixel, pDraw, ppt, pGC);
		else
		    qddopixel( &pGC->bgPixel, pDraw, ppt, pGC);
	    }
    	}
    }
}
