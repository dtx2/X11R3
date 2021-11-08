/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
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
/* $XConsortium: mfbgetsp.c,v 1.3 88/09/06 15:20:25 jim Exp $ */
#include "X.h"
#include "Xmd.h"

#include "misc.h"
#include "region.h"
#include "gc.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "mfb.h"
#include "maskbits.h"

#include "servermd.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
unsigned int *
mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
    register unsigned int	*pdst;	/* where to put the bits */
    register unsigned int	*psrc;	/* where to get the bits */
    register unsigned int	tmpSrc;	/* scratch buffer for bits */
    unsigned int		*psrcBase;	/* start of src bitmap */
    int			widthSrc;	/* width of pixmap in bytes */
    register DDXPointPtr pptLast;	/* one past last point to get */
    int         	xEnd;		/* last pixel to copy from */
    register int	nstart; 
    int	 		nend; 
    int	 		srcStartOver; 
    int	 		startmask, endmask, nlMiddle, nl, srcBit;
    int			w;
    unsigned int	*pdstStart;
    unsigned int	*pdstNext;
    DDXPointPtr	  	pptInit;
    int	    	  	*pwidthInit;
    int	    	  	*pwidthPadded;
    int	    	  	i;

    pptLast = ppt + nspans;
    pptInit = ppt;
    pwidthInit = pwidth;

    /*
     * XXX: Should probably only do this if going to use backing-store
     */
    pwidthPadded = (int *)ALLOCATE_LOCAL(nspans * sizeof(int));
    if (pwidthPadded == (int *)NULL)
    {
	return(NULL);
    }

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	psrcBase = (unsigned int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	widthSrc = (int)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride);
    }
    else
    {
	psrcBase = (unsigned int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	widthSrc = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride);
    }
    pdstStart = (unsigned int *)Xalloc(nspans * PixmapBytePad(wMax, 1));
    pdst = pdstStart;

    i = 0;
    while(ppt < pptLast)
    {
	xEnd = min(ppt->x + *pwidth, widthSrc << 3);
	pwidth++;
	psrc = psrcBase + (ppt->y * (widthSrc >> 2)) + (ppt->x >> 5); 
	w = xEnd - ppt->x;
	srcBit = ppt->x & 0x1f;
	/* This shouldn't be needed */
	pdstNext = pdst + PixmapWidthInPadUnits(w, 1);

	pwidthPadded[i] = PixmapBytePad(w, 1) << 3;
	i++;

	if (srcBit + w <= 32) 
	{ 
	    getbits(psrc, srcBit, w, tmpSrc);
	    putbits(tmpSrc, 0, w, pdst); 
	    pdst++;
	} 
	else 
	{ 

	    maskbits(ppt->x, w, startmask, endmask, nlMiddle);
	    if (startmask) 
		nstart = 32 - srcBit; 
	    else 
		nstart = 0; 
	    if (endmask) 
		nend = xEnd & 0x1f; 
	    srcStartOver = srcBit + nstart > 31;
	    if (startmask) 
	    { 
		getbits(psrc, srcBit, nstart, tmpSrc);
		putbits(tmpSrc, 0, nstart, pdst);
		if(srcStartOver)
		    psrc++;
	    } 
	    nl = nlMiddle; 
	    while (nl--) 
	    { 
		tmpSrc = *psrc;
		putbits(tmpSrc, nstart, 32, pdst);
		psrc++;
		pdst++;
	    } 
	    if (endmask) 
	    { 
		getbits(psrc, 0, nend, tmpSrc);
		putbits(tmpSrc, nstart, nend, pdst);
		if(nstart + nend > 32)
		    pdst++;
	    } 
	    pdst++; 
	    while(pdst < pdstNext)
	    {
		*pdst++ = 0;
	    }
	} 
        ppt++;
    }

    /*
     * If the drawable is a window with some form of backing-store, consult
     * the backing-store module to fetch any invalid spans from the window's
     * backing-store. The pixmap is made into one long scanline and the
     * backing-store module takes care of the rest. 
     */
    if ((pDrawable->type == DRAWABLE_WINDOW) &&
	(((WindowPtr)pDrawable)->backingStore != NotUseful))
    {
	PixmapPtr pPixmap;

#if 0
	pPixmap = (PixmapPtr)ALLOCATE_LOCAL(sizeof(PixmapRec));
	if ((pPixmap != (PixmapPtr)NULL) && (pwidthPadded != (int *)NULL))
	{
	    pPixmap->drawable.type = DRAWABLE_PIXMAP;
	    pPixmap->drawable.pScreen = pDrawable->pScreen;
	    pPixmap->drawable.depth = 1;
	    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    pPixmap->devKind = PixmapBytePad(wMax, 1) * nspans;
	    pPixmap->width = pPixmap->devKind << 3;
	    pPixmap->height = 1;
	    pPixmap->refcnt = 1;
	    pPixmap->devPrivate = (pointer)pdstStart;
	    miBSGetSpans(pDrawable, pPixmap, wMax, pptInit, pwidthInit,
			 pwidthPadded, nspans);
	}
	DEALLOCATE_LOCAL(pPixmap);
#else
      if (pPixmap = mfbCreatePixmap(pDrawable->pScreen,	/* !!!XXX outta mem? */
	  (PixmapBytePad(wMax,1)*nspans)<<3, 1, pDrawable->depth))
      {
	miBSGetSpans(pDrawable, pPixmap, wMax, pptInit, pwidthInit,
		pwidthPadded, nspans);
	mfbDestroyPixmap(pPixmap);
      }
#endif
    }

    DEALLOCATE_LOCAL(pwidthPadded);
	
    return(pdstStart);
}

