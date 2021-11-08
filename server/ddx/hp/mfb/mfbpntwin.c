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
/* $XConsortium: mfbpntwin.c,v 1.2 88/09/06 15:20:40 jim Exp $ */
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

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "mfb.h"
#include "maskbits.h"
/*
NOTE
   PaintArea32() doesn't need to rotate the tile, since
mfbPositionWIndow() and mfbChangeWIndowAttributes() do it.
*/

/* Paint Window None -- just return */
void
mfbPaintWindowNone(pWin, pRegion, what)
    WindowPtr pWin;
    RegionPtr pRegion;
    int what;		
{
}

/* Paint Window Parent Relative -- Find first ancestor which isn't parent
 * relative and paint as it would, but with this region */ 
void
mfbPaintWindowPR(pWin, pRegion, what)
    WindowPtr pWin;
    RegionPtr pRegion;
    int what;		
{
    WindowPtr pParent;

    pParent = pWin->parent;
    while(pParent->backgroundTile == (PixmapPtr)ParentRelative)
	pParent = pParent->parent;

    if(what == PW_BORDER)
        (*pParent->PaintWindowBorder)(pParent, pRegion, what);
    else
	(*pParent->PaintWindowBackground)(pParent, pRegion, what);
}

void
mfbPaintWindowSolid(pWin, pRegion, what)
    WindowPtr pWin;
    RegionPtr pRegion;
    int what;		
{
    if (what == PW_BACKGROUND)
    {
	if (pWin->backgroundPixel)
	    mfbSolidWhiteArea(pWin, pRegion->numRects, pRegion->rects,
			      GXset, NullPixmap);
	else
	    mfbSolidBlackArea(pWin, pRegion->numRects, pRegion->rects,
			      GXclear, NullPixmap);
    } 
    else
    {
	if (pWin->borderPixel)
	    mfbSolidWhiteArea(pWin, pRegion->numRects, pRegion->rects,
			      GXset, NullPixmap);
	else
	    mfbSolidBlackArea(pWin, pRegion->numRects, pRegion->rects,
			      GXclear, NullPixmap);
    }
}


/* Tile Window with a 32 bit wide tile 
   this could call mfbTileArea32, but that has to do a switch on the
rasterop, which seems expensive.
*/
void
mfbPaintWindow32(pWin, pRegion, what)
    WindowPtr pWin;
    RegionPtr pRegion;
    int what;		
{
    int nbox;		/* number of boxes to fill */
    register BoxPtr pbox;	/* pointer to list of boxes to fill */
    int *psrc;		/* pointer to bits in tile */
    int tileHeight;	/* height of the tile */
    register int srcpix;	/* current row from tile */

    PixmapPtr pPixmap;
    mfbPrivPixmapPtr pPrivPixmap;
    int nlwScreen;	/* width in longwords of the screen's pixmap */
    int w;		/* width of current box */
    register int h;	/* height of current box */
    register int startmask;
    int endmask;	/* masks for reggedy bits at either end of line */
    int nlwMiddle;	/* number of longwords between sides of boxes */
    int nlwExtra;	/* to get from right of box to left of next span */
    
    register int nlw;	/* loop version of nlwMiddle */
    register unsigned int *p;	/* pointer to bits we're writing */
    int y;		/* current scan line */

    unsigned int *pbits;	/* pointer to start of screen */
    mfbPrivWin *pPrivWin;
    PixmapPtr Tile;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivate);

    if (what == PW_BACKGROUND)
    {
	tileHeight = pWin->backgroundTile->height;
	Tile = pPrivWin->pRotatedBackground;
    } 
    else
    {
        tileHeight = pWin->borderTile->height;
	Tile = pPrivWin->pRotatedBorder;
    } 
    psrc = (int *)(((mfbPrivPixmapPtr)(Tile->devPrivate))->bits);

    pPixmap = (PixmapPtr)
	(((mfbPrivScreenPtr)(pWin->drawable.pScreen->devPrivate))->pDrawable);
    pPrivPixmap = (mfbPrivPixmapPtr)(pPixmap->devPrivate);
    pbits = (unsigned int *)(pPrivPixmap->bits);
    nlwScreen = (pPrivPixmap->stride) >> 2;
    nbox = pRegion->numRects;
    pbox = pRegion->rects;

    while (nbox--)
    {
	w = pbox->x2 - pbox->x1;
	h = pbox->y2 - pbox->y1;
	y = pbox->y1;
	p = pbits + (pbox->y1 * nlwScreen) + (pbox->x1 >> 5);

	if ( ((pbox->x1 & 0x1f) + w) < 32)
	{
	    maskpartialbits(pbox->x1, w, startmask);
	    nlwExtra = nlwScreen;
	    while (h--)
	    {
		srcpix = psrc[y%tileHeight];
		y++;
		*p = (*p & ~startmask) | (srcpix & startmask);
		p += nlwExtra;
	    }
	}
	else
	{
	    maskbits(pbox->x1, w, startmask, endmask, nlwMiddle);
	    nlwExtra = nlwScreen - nlwMiddle;

	    if (startmask && endmask)
	    {
		nlwExtra -= 1;
		while (h--)
		{
		    srcpix = psrc[y%tileHeight];
		    y++;
		    nlw = nlwMiddle;
		    *p = (*p & ~startmask) | (srcpix & startmask);
		    p++;
		    while (nlw--)
			*p++ = srcpix;
		    *p = (*p & ~endmask) | (srcpix & endmask);
		    p += nlwExtra;
		}
	    }
	    else if (startmask && !endmask)
	    {
		nlwExtra -= 1;
		while (h--)
		{
		    srcpix = psrc[y%tileHeight];
		    y++;
		    nlw = nlwMiddle;
		    *p = (*p & ~startmask) | (srcpix & startmask);
		    p++;
		    while (nlw--)
			*p++ = srcpix;
		    p += nlwExtra;
		}
	    }
	    else if (!startmask && endmask)
	    {
		while (h--)
		{
		    srcpix = psrc[y%tileHeight];
		    y++;
		    nlw = nlwMiddle;
		    while (nlw--)
			*p++ = srcpix;
		    *p = (*p & ~endmask) | (srcpix & endmask);
		    p += nlwExtra;
		}
	    }
	    else /* no ragged bits at either end */
	    {
		while (h--)
		{
		    srcpix = psrc[y%tileHeight];
		    y++;
		    nlw = nlwMiddle;
		    while (nlw--)
			*p++ = srcpix;
		    p += nlwExtra;
		}
	    }
	}
        pbox++;
    }
}
