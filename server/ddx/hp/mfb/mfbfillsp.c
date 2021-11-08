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
/* $XConsortium: mfbfillsp.c,v 1.3 88/09/06 15:20:17 jim Exp $ */
#include "X.h"
#include "Xmd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mfb.h"
#include "maskbits.h"

#include "servermd.h"

/* scanline filling for monochrome frame buffer
   written by drewry, oct 1986

   these routines all clip.  they assume that anything that has called
them has already translated the points (i.e. pGC->miTranslate is
non-zero, which is howit gets set in mfbCreateGC().)

   the number of new scnalines created by clipping ==
MaxRectsPerBand * nSpans.

    FillSolid is overloaded to be used for OpaqueStipple as well,
if fgPixel == bgPixel.  


    FillTiled is overloaded to be used for OpaqueStipple, if
fgPixel != bgPixel.  based on the fill style, it uses
{RotatedTile, gc.alu} or {RotatedStipple, PrivGC.ropOpStip}
*/


void mfbBlackSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int *addrlBase;		/* pointer to start of bitmap */
    int nlwidth;		/* width in longwords of bitmap */
    register int *addrl;	/* pointer to current longword in bitmap */
    register int startmask;
    register int endmask;
    register int nlmiddle;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit,
		    ppt, pwidth, fSorted);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
	    (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		>> 2;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
	     >> 2;
    }

    while (n--)
    {
        addrl = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);

	if (*pwidth)
	{
	    if ( ((ppt->x & 0x1f) + *pwidth) < 32)
	    {
		/* all bits inside same longword */
		maskpartialbits(ppt->x, *pwidth, startmask);
		    *addrl &= ~startmask;
	    }
	    else
	    {
		maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
		if (startmask)
		    *addrl++ &= ~startmask;
		while (nlmiddle--)
		    *addrl++ = 0x0;
		if (endmask)
		    *addrl &= ~endmask;
	    }
	}
	pwidth++;
	ppt++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}



void mfbWhiteSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int *addrlBase;		/* pointer to start of bitmap */
    int nlwidth;		/* width in longwords of bitmap */
    register int *addrl;	/* pointer to current longword in bitmap */
    register int startmask;
    register int endmask;
    register int nlmiddle;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit,
		    ppt, pwidth, fSorted);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
	        (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		    >> 2;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
	     >> 2;
    }

    while (n--)
    {
        addrl = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);

	if (*pwidth)
	{
	    if ( ((ppt->x & 0x1f) + *pwidth) < 32)
	    {
		/* all bits inside same longword */
		maskpartialbits(ppt->x, *pwidth, startmask);
		*addrl |= startmask;
	    }
	    else
	    {
		maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
		if (startmask)
		    *addrl++ |= startmask;
		while (nlmiddle--)
		    *addrl++ = 0xffffffff;
		if (endmask)
		    *addrl |= endmask;
	    }
	}
	pwidth++;
	ppt++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}



void mfbInvertSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int *addrlBase;		/* pointer to start of bitmap */
    int nlwidth;		/* width in longwords of bitmap */
    register int *addrl;	/* pointer to current longword in bitmap */
    register int startmask;
    register int endmask;
    register int nlmiddle;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit,
		    ppt, pwidth, fSorted);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
	      (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
	      (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		  >> 2;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
		>> 2;
    }

    while (n--)
    {
        addrl = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);

	if (*pwidth)
	{
	    if ( ((ppt->x & 0x1f) + *pwidth) < 32)
	    {
		/* all bits inside same longword */
		maskpartialbits(ppt->x, *pwidth, startmask);
		*addrl ^= startmask;
	    }
	    else
	    {
		maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
		if (startmask)
		    *addrl++ ^= startmask;
		while (nlmiddle--)
		    *addrl++ ^= 0xffffffff;
		if (endmask)
		    *addrl ^= endmask;
	    }
	}
	pwidth++;
	ppt++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


void 
mfbWhiteStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC *pGC;
int nInit;			/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int *addrlBase;		/* pointer to start of bitmap */
    int nlwidth;		/* width in longwords of bitmap */
    register int *addrl;	/* pointer to current longword in bitmap */
    register int startmask;
    register int endmask;
    register int nlmiddle;
    PixmapPtr pStipple;
    int *psrc;
    int src;
    int tileHeight;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit, 
		    ppt, pwidth, fSorted);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
	        (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		    >> 2;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
		>> 2;
    }

    pStipple = ((mfbPrivGC *)(pGC->devPriv))->pRotatedStipple;
    tileHeight = pStipple->height;
    psrc = (int *)(((mfbPrivPixmapPtr)(pStipple->devPrivate))->bits);

    while (n--)
    {
        addrl = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);
	src = psrc[ppt->y % tileHeight];

        /* all bits inside same longword */
        if ( ((ppt->x & 0x1f) + *pwidth) < 32)
        {
	    maskpartialbits(ppt->x, *pwidth, startmask);
	    *addrl |= (src & startmask);
        }
        else
        {
	    maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
	    if (startmask)
		*addrl++ |= (src & startmask);
	    while (nlmiddle--)
		*addrl++ |= src;
	    if (endmask)
		*addrl |= (src & endmask);
        }
	pwidth++;
	ppt++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


void 
mfbBlackStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC *pGC;
int nInit;			/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int *addrlBase;		/* pointer to start of bitmap */
    int nlwidth;		/* width in longwords of bitmap */
    register int *addrl;	/* pointer to current longword in bitmap */
    register int startmask;
    register int endmask;
    register int nlmiddle;
    PixmapPtr pStipple;
    int *psrc;
    int src;
    int tileHeight;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit, 
		    ppt, pwidth, fSorted);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		    >> 2;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
	     >> 2;
    }

    pStipple = ((mfbPrivGC *)(pGC->devPriv))->pRotatedStipple;
    tileHeight = pStipple->height;
    psrc = (int *)(((mfbPrivPixmapPtr)(pStipple->devPrivate))->bits);

    while (n--)
    {
        addrl = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);
	src = psrc[ppt->y % tileHeight];

        /* all bits inside same longword */
        if ( ((ppt->x & 0x1f) + *pwidth) < 32)
        {
	    maskpartialbits(ppt->x, *pwidth, startmask);
	    *addrl &= ~(src & startmask);
        }
        else
        {
	    maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
	    if (startmask)
		*addrl++ &= ~(src & startmask);
	    while (nlmiddle--)
		*addrl++ &= ~src;
	    if (endmask)
		*addrl &= ~(src & endmask);
        }
	pwidth++;
	ppt++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


void 
mfbInvertStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC *pGC;
int nInit;			/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int *addrlBase;		/* pointer to start of bitmap */
    int nlwidth;		/* width in longwords of bitmap */
    register int *addrl;	/* pointer to current longword in bitmap */
    register int startmask;
    register int endmask;
    register int nlmiddle;
    PixmapPtr pStipple;
    int *psrc;
    int src;
    int tileHeight;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit, 
		    ppt, pwidth, fSorted);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
	        (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		    >> 2;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
		>> 2;
    }

    pStipple = ((mfbPrivGC *)(pGC->devPriv))->pRotatedStipple;
    tileHeight = pStipple->height;
    psrc = (int *)(((mfbPrivPixmapPtr)(pStipple->devPrivate))->bits);

    while (n--)
    {
        addrl = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);
	src = psrc[ppt->y % tileHeight];

        /* all bits inside same longword */
        if ( ((ppt->x & 0x1f) + *pwidth) < 32)
        {
	    maskpartialbits(ppt->x, *pwidth, startmask);
	    *addrl ^= (src & startmask);
        }
        else
        {
	    maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
	    if (startmask)
		*addrl++ ^= (src & startmask);
	    while (nlmiddle--)
		*addrl++ ^= src;
	    if (endmask)
		*addrl ^= (src & endmask);
        }
	pwidth++;
	ppt++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


/* this works with tiles of width == 32 */
#define FILLSPAN32(ROP) \
    while (n--) \
    { \
	if (*pwidth) \
	{ \
            addrl = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5); \
	    src = psrc[ppt->y % tileHeight]; \
            if ( ((ppt->x & 0x1f) + *pwidth) < 32) \
            { \
	        maskpartialbits(ppt->x, *pwidth, startmask); \
	        *addrl = (*addrl & ~startmask) | \
		         (ROP(src, *addrl) & startmask); \
            } \
            else \
            { \
	        maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle); \
	        if (startmask) \
	        { \
	            *addrl = (*addrl & ~startmask) | \
			     (ROP(src, *addrl) & startmask); \
		    addrl++; \
	        } \
	        while (nlmiddle--) \
	        { \
		    *addrl = ROP(src, *addrl); \
		    addrl++; \
	        } \
	        if (endmask) \
	            *addrl = (*addrl & ~endmask) | \
			     (ROP(src, *addrl) & endmask); \
            } \
	} \
	pwidth++; \
	ppt++; \
    }



void mfbTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC *pGC;
int nInit;			/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int *addrlBase;		/* pointer to start of bitmap */
    int nlwidth;		/* width in longwords of bitmap */
    register int *addrl;	/* pointer to current longword in bitmap */
    register int startmask;
    register int endmask;
    register int nlmiddle;
    PixmapPtr pTile;
    int *psrc;
    register int src;
    int tileHeight;
    int rop;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit, 
		    ppt, pwidth, fSorted);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
	        (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		    >> 2;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
		>> 2;
    }

    if (pGC->fillStyle == FillTiled)
    {
	pTile = ((mfbPrivGC *)(pGC->devPriv))->pRotatedTile;
	tileHeight = pTile->height;
	psrc = (int *)(((mfbPrivPixmapPtr)(pTile->devPrivate))->bits);
	rop = pGC->alu;
    }
    else
    {
	pTile = ((mfbPrivGC *)(pGC->devPriv))->pRotatedStipple;
	tileHeight = pTile->height;
	psrc = (int *)(((mfbPrivPixmapPtr)(pTile->devPrivate))->bits);
	rop = ((mfbPrivGC *)(pGC->devPriv))->ropOpStip;
    }


    switch(rop)
    {
      case GXclear:
	FILLSPAN32(fnCLEAR)
	break;
      case GXand:
	FILLSPAN32(fnAND)
	break;
      case GXandReverse:
	FILLSPAN32(fnANDREVERSE)
	break;
      case GXcopy:
	FILLSPAN32(fnCOPY)
	break;
      case GXandInverted:
	FILLSPAN32(fnANDINVERTED)
	break;
      case GXnoop:
	break;
      case GXxor:
	FILLSPAN32(fnXOR)
	break;
      case GXor:
	FILLSPAN32(fnOR)
	break;
      case GXnor:
	FILLSPAN32(fnNOR)
	break;
      case GXequiv:
	FILLSPAN32(fnEQUIV)
	break;
      case GXinvert:
	FILLSPAN32(fnINVERT)
	break;
      case GXorReverse:
	FILLSPAN32(fnORREVERSE)
	break;
      case GXcopyInverted:
	FILLSPAN32(fnCOPYINVERTED)
	break;
      case GXorInverted:
	FILLSPAN32(fnORINVERTED)
	break;
      case GXnand:
	FILLSPAN32(fnNAND)
	break;
      case GXset:
	FILLSPAN32(fnSET)
	break;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


/* Fill spans with tiles that aren't 32 bits wide */
void
mfbUnnaturalTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC		*pGC;
int		nInit;		/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
    int		iline;		/* first line of tile to use */
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int		*addrlBase;	/* pointer to start of bitmap */
    int		 nlwidth;	/* width in longwords of bitmap */
    register int *pdst;		/* pointer to current word in bitmap */
    register int *psrc;		/* pointer to current word in tile */
    register int startmask;
    register int nlMiddle;
    PixmapPtr	pTile;		/* pointer to tile we want to fill with */
    mfbPrivPixmapPtr pPrivTile;
    int		w, width, x, xSrc, ySrc, tmpSrc, srcStartOver, nstart, nend;
    int 	endmask, tlwidth, rem, tileWidth, *psrcT, endinc, rop;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit, 
		    ppt, pwidth, fSorted);

    if (pGC->fillStyle == FillTiled)
    {
	pTile = pGC->tile;
	rop = pGC->alu;
    }
    else
    {
	pTile = pGC->stipple;
	rop = ((mfbPrivGC *)(pGC->devPriv))->ropOpStip;
    }

    pPrivTile = (mfbPrivPixmapPtr) pTile -> devPrivate;
    tlwidth = pPrivTile->stride >> 2;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		    >> 2;
	xSrc = ((WindowPtr) pDrawable)->absCorner.x;
	ySrc = ((WindowPtr) pDrawable)->absCorner.y;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
		>> 2;
	xSrc = 0;
	ySrc = 0;
    }

    tileWidth = pTile->width;

    /* this replaces rotating the tile. Instead we just adjust the offset
     * at which we start grabbing bits from the tile.
     * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
     * so that iline and rem always stay within the tile bounds.
     */
    xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
    ySrc += (pGC->patOrg.y % pTile->height) - pTile->height;

    while (n--)
    {
	iline = (ppt->y - ySrc) % pTile->height;
        pdst = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);
        psrcT = ((int *)(pPrivTile->bits)) + (iline * tlwidth);
	x = ppt->x;

	if (*pwidth)
	{
	    width = *pwidth;
	    while(width > 0)
	    {
		psrc = psrcT;
	        w = min(tileWidth, width);
		if((rem = (x - xSrc)  % tileWidth) != 0)
		{
		    /* if we're in the middle of the tile, get
		       as many bits as will finish the span, or
		       as many as will get to the left edge of the tile,
		       or a longword worth, starting at the appropriate
		       offset in the tile.
		    */
		    w = min(min(tileWidth - rem, width), BITMAP_SCANLINE_UNIT);
		    endinc = rem / BITMAP_SCANLINE_UNIT;
		    getbits(psrc + endinc, rem & 0x1f, w, tmpSrc);
		    putbitsrop(tmpSrc, (x & 0x1f), w, pdst, rop);
		    if((x & 0x1f) + w >= 0x20)
			pdst++;
		}
		else if(((x & 0x1f) + w) < 32)
		{
		    /* doing < 32 bits is easy, and worth special-casing */
		    getbits(psrc, 0, w, tmpSrc);
		    putbitsrop(tmpSrc, x & 0x1f, w, pdst, rop);
		}
		else
		{
		    /* start at the left edge of the tile,
		       and put down as much as we can
		    */
		    maskbits(x, w, startmask, endmask, nlMiddle);

	            if (startmask)
		        nstart = 32 - (x & 0x1f);
	            else
		        nstart = 0;
	            if (endmask)
	                nend = (x + w)  & 0x1f;
	            else
		        nend = 0;

	            srcStartOver = nstart > 31;

		    if(startmask)
		    {
			getbits(psrc, 0, nstart, tmpSrc);
			putbitsrop(tmpSrc, (x & 0x1f), nstart, pdst, rop);
			pdst++;
			if(srcStartOver)
			    psrc++;
		    }
		     
		    while(nlMiddle--)
		    {
			    getbits(psrc, nstart, 32, tmpSrc);
			    *pdst = DoRop(rop, tmpSrc, *pdst);
			    pdst++;
			    psrc++;
		    }
		    if(endmask)
		    {
			getbits(psrc, nstart, nend, tmpSrc);
			putbitsrop(tmpSrc, 0, nend, pdst, rop);
		    }
		 }
		 x += w;
		 width -= w;
	    }
	}
	ppt++;
	pwidth++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


/* Fill spans with stipples that aren't 32 bits wide */
void
mfbUnnaturalStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC		*pGC;
int		nInit;		/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
				/* next three parameters are post-clip */
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    int		iline;		/* first line of tile to use */
    int		*addrlBase;	/* pointer to start of bitmap */
    int		 nlwidth;	/* width in longwords of bitmap */
    register int *pdst;		/* pointer to current word in bitmap */
    register int *psrc;		/* pointer to current word in tile */
    register int startmask;
    register int nlMiddle;
    PixmapPtr	pTile;		/* pointer to tile we want to fill with */
    mfbPrivPixmapPtr pPrivTile;
    int		w, width,  x, xSrc, ySrc, tmpSrc, srcStartOver, nstart, nend;
    int 	endmask, tlwidth, rem, tileWidth, *psrcT, endinc, rop;
    int *pwidthFree;		/* copies of the pointers to free */
    DDXPointPtr pptFree;

    if (!(pGC->planemask & 1))
	return;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPriv))->pCompositeClip,
		    pptInit, pwidthInit, nInit, 
		    ppt, pwidth, fSorted);

    pTile = pGC->stipple;
    pPrivTile = (mfbPrivPixmapPtr) pTile->devPrivate;
    rop = ((mfbPrivGC *)(pGC->devPriv))->rop;
    tlwidth = pPrivTile->stride >> 2;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrlBase = (int *)
		(((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->bits);
	nlwidth = (int)
	        (((mfbPrivScreenPtr)(pDrawable->pScreen->devPrivate))->stride)
		    >> 2;
	xSrc = ((WindowPtr)pDrawable)->absCorner.x;
	ySrc = ((WindowPtr)pDrawable)->absCorner.y;
    }
    else
    {
	addrlBase = (int *)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->bits);
	nlwidth = (int)
	    (((mfbPrivPixmapPtr)(((PixmapPtr)pDrawable)->devPrivate))->stride)
		>> 2;
	xSrc = 0;
	ySrc = 0;
    }

    tileWidth = pTile->width;

    /* this replaces rotating the stipple.  Instead, we just adjust the offset
     * at which we start grabbing bits from the stipple.
     * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
     * so that iline and rem always stay within the tile bounds.
     */
    xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
    ySrc += (pGC->patOrg.y % pTile->height) - pTile->height;
    while (n--)
    {
	iline = (ppt->y - ySrc) % pTile->height;
        pdst = addrlBase + (ppt->y * nlwidth) + (ppt->x >> 5);
        psrcT = ((int *)(pPrivTile->bits)) + (iline * tlwidth);
	x = ppt->x;

	if (*pwidth)
	{
	    width = *pwidth;
	    while(width > 0)
	    {
		psrc = psrcT;
	        w = min(tileWidth, width);
		if((rem = (x - xSrc) % tileWidth) != 0)
		{
		    /* if we're in the middle of the tile, get
		       as many bits as will finish the span, or
		       as many as will get to the left edge of the tile,
		       or a longword worth, starting at the appropriate
		       offset in the tile.
		    */
		    w = min(min(tileWidth - rem, width), BITMAP_SCANLINE_UNIT);
		    endinc = rem / BITMAP_SCANLINE_UNIT;
		    getbits(psrc + endinc, rem & 0x1f, w, tmpSrc);
		    putbitsrrop(tmpSrc, (x & 0x1f), w, pdst, rop);
		    if((x & 0x1f) + w >= 0x20)
			pdst++;
		}

		else if(((x & 0x1f) + w) < 32)
		{
		    /* doing < 32 bits is easy, and worth special-casing */
		    getbits(psrc, 0, w, tmpSrc);
		    putbitsrrop(tmpSrc, x & 0x1f, w, pdst, rop);
		}
		else
		{
		    /* start at the left edge of the tile,
		       and put down as much as we can
		    */
		    maskbits(x, w, startmask, endmask, nlMiddle);

	            if (startmask)
		        nstart = 32 - (x & 0x1f);
	            else
		        nstart = 0;
	            if (endmask)
	                nend = (x + w)  & 0x1f;
	            else
		        nend = 0;

	            srcStartOver = nstart > 31;

		    if(startmask)
		    {
			getbits(psrc, 0, nstart, tmpSrc);
			putbitsrrop(tmpSrc, (x & 0x1f), nstart, pdst, rop);
			pdst++;
			if(srcStartOver)
			    psrc++;
		    }
		     
		    while(nlMiddle--)
		    {
			    getbits(psrc, nstart, 32, tmpSrc);
			    *pdst = DoRRop(rop, tmpSrc, *pdst);
			    pdst++;
			    psrc++;
		    }
		    if(endmask)
		    {
			getbits(psrc, nstart, nend, tmpSrc);
			putbitsrrop(tmpSrc, 0, nend, pdst, rop);
		    }
		 }
		 x += w;
		 width -= w;
	    }
	}
	ppt++;
	pwidth++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}
