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
/* $XConsortium: mfbimage.c,v 1.4 88/09/06 15:21:05 jim Exp $ */

#include "X.h"

#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"

#include "mfb.h"
#include "mi.h"
#include "Xmd.h"

#include "maskbits.h"

#include "servermd.h"

/* Put and Get images on a monochrome frame buffer
 *
 *   we do this by creating a temporary pixmap and making its
 * pointer to bits point to the buffer read in from the client.
 * this works because of the padding rules specified at startup
 *
 * Note that CopyArea must know how to copy a bitmap into the server-format
 * temporary pixmap.
 *
 * For speed, mfbPutImage should allocate the temporary pixmap on the stack.
 *
 *     even though an XYBitmap and an XYPixmap have the same
 * format (for this device), PutImage has different semantics for the
 * two.  XYPixmap just does the copy; XYBitmap takes gc.fgPixel for
 * a 1 bit, gc.bgPixel for a 0 bit, which we notice is exactly
 * like CopyPlane.
 *
 *   written by drewry, september 1986
 */


void
mfbPutImage(dst, pGC, depth, x, y, w, h, leftPad, format, pImage)
    DrawablePtr dst;
    GCPtr	pGC;
    int		depth, x, y, w, h;
    int leftPad;
    unsigned int format;
    int 	*pImage;
{
    pointer	pbits;
    PixmapPtr	pFakePixmap;
    mfbPrivPixmapPtr pPrivFakePixmap;

    if (!(pGC->planemask & 1))
	return;

    /* 0 may confuse CreatePixmap, and will sometimes be
       passed by the mi text code
    */
    if ((w == 0) || (h == 0))
	return;

    /* mfb is always depth 1 */
    pFakePixmap = (PixmapPtr)mfbCreatePixmap(dst->pScreen, w+leftPad, h, 1);
    if (!pFakePixmap)
    {
	ErrorF( "mfbPutImage can't make temp pixmap\n");
	return;
    }

    pPrivFakePixmap = (mfbPrivPixmapPtr)(pFakePixmap->devPrivate);
    pbits = pPrivFakePixmap -> bits;
    pPrivFakePixmap->bits = (pointer)pImage;
    ((mfbPrivGC *)(pGC->devPriv))->fExpose = FALSE;
    if (format != XYBitmap)
	(*pGC->CopyArea)(pFakePixmap, dst, pGC, leftPad, 0, w, h, x, y);
    else
	(*pGC->CopyPlane)(pFakePixmap, dst, pGC, leftPad, 0, w, h, x, y, 1);
    ((mfbPrivGC*)(pGC->devPriv))->fExpose = TRUE;
    pPrivFakePixmap->bits = pbits;
    (*dst->pScreen->DestroyPixmap)(pFakePixmap);
}


/*
 * pdstLine points to space allocated by caller, which he can do since
 * he knows dimensions of the pixmap
 * we can call mfbDoBitblt because the dispatcher has promised not to send us
 * anything that would require going over the edge of the screen.
 *
 *	XYPixmap and ZPixmap are the same for mfb.
 *	For any planemask with bit 0 == 0, just fill the dst with 0.
 */
void
mfbGetImage( pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned int planeMask;
    pointer	pdstLine;
{
    int xorg, yorg;
    PixmapPtr pPixmap;
    mfbPrivPixmapPtr pPrivPixmap;
    BoxRec box;
    DDXPointRec ptSrc;
    RegionPtr prgnDst;
    pointer pspare;

    if (planeMask & 0x1)
    {
        if (pDrawable->type == DRAWABLE_WINDOW)
        {
	    xorg = ((WindowPtr)pDrawable)->absCorner.x;
	    yorg = ((WindowPtr)pDrawable)->absCorner.y;
        }
        else
        {
	    xorg = 0;
	    yorg = 0;
        }

	sx += xorg;
	sy += yorg;
	
	pPixmap = (PixmapPtr)mfbCreatePixmap(pDrawable->pScreen, w, h, 1);
	pPrivPixmap = (mfbPrivPixmapPtr)(pPixmap->devPrivate);
	pspare = pPrivPixmap->bits;
	pPrivPixmap->bits = pdstLine;
	ptSrc.x = sx;
	ptSrc.y = sy;
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = w;
	box.y2 = h;

	prgnDst = (*pDrawable->pScreen->RegionCreate)(&box, 1);
	mfbDoBitblt(pDrawable, pPixmap, GXcopy, prgnDst, &ptSrc);
 
 	if ((pDrawable->type == DRAWABLE_WINDOW) &&
 	    (((WindowPtr)pDrawable)->backingStore != NotUseful))
 	{
 	    miBSGetImage(pDrawable, pPixmap, sx, sy, w, h, format,
 			 planeMask, pdstLine);
 	}
 

        (*pDrawable->pScreen->RegionDestroy)(prgnDst);
	pPrivPixmap->bits = pspare;
	mfbDestroyPixmap(pPixmap);
    }
    else
    {
	bzero(pdstLine, PixmapBytePad(w, 1) * h);
    }
}
