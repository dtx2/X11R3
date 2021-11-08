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
/* $XConsortium: mfbpixmap.c,v 1.3 88/09/06 15:20:35 jim Exp $ */

/* pixmap management
   written by drewry, september 1986

   on a monchrome device, a pixmap is a bitmap.
*/

#include "Xmd.h"
#include "pixmapstr.h"
#include "maskbits.h"

#include "mfb.h"
#include "mi.h"

#include "servermd.h"

PixmapPtr
mfbCreatePixmap (pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    register PixmapPtr pPixmap;
    mfbPrivPixmapPtr pPrivPixmap;
    int size;

    if (depth != 1)
	return (PixmapPtr)NULL;

#ifdef notdef
    pPixmap = (PixmapPtr)Xalloc(sizeof(PixmapRec));
    pPixmap->devPrivate = (pointer)Xalloc(sizeof(mfbPrivPixmap));
#else
    if(!(pPixmap = (PixmapPtr)malloc(sizeof(PixmapRec)))) 
	return (PixmapPtr) NULL;
    if(!(pPixmap->devPrivate = (pointer)malloc(sizeof(mfbPrivPixmap)))) {
	free(pPixmap);
	return (PixmapPtr) NULL;
    }
#endif
    pPrivPixmap = (mfbPrivPixmapPtr)pPixmap->devPrivate;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = 1;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->width = width;
    pPixmap->height = height;
    pPixmap->devKind = PIXMAP_HOST_MEMORY;
    pPrivPixmap->stride = PixmapBytePad(width, 1);
    pPixmap->refcnt = 1;
    size = height * pPrivPixmap->stride;

#ifdef notdef
    if ( !(pPrivPixmap->bits = (pointer)Xalloc(size)))
#else
    if ( !(pPrivPixmap->bits = (pointer)malloc(size)))
#endif
    {
	Xfree(pPrivPixmap);
	Xfree(pPixmap);
	return (PixmapPtr)NULL;
    }
    else
        bzero((char *)pPrivPixmap->bits, size);
    return pPixmap;
}

Bool
mfbDestroyPixmap(pPixmap)
    PixmapPtr pPixmap;
{
/* BOGOSITY ALERT */
    if ((unsigned)pPixmap < 42)
	return TRUE;

    if(--pPixmap->refcnt)
	return TRUE;
#ifdef notdef
    Xfree(((mfbPrivPixmapPtr)pPixmap->devPrivate)->bits);
    Xfree(pPixmap->devPrivate);
    Xfree(pPixmap);
#else
    free(((mfbPrivPixmapPtr)pPixmap->devPrivate)->bits);
    free(pPixmap->devPrivate);
    free(pPixmap);
#endif
    return TRUE;
}


PixmapPtr
mfbCopyPixmap(pSrc)
    register PixmapPtr	pSrc;
{
    register PixmapPtr	pDst;
    register int	*pSrcInt, *pDstInt, *pDstMax;
    register mfbPrivPixmapPtr    pPrivDst, pPrivSrc;
    int		size;

    pPrivSrc = (mfbPrivPixmapPtr) pSrc->devPrivate;
    pDst = (PixmapPtr) Xalloc(sizeof(PixmapRec));
    pDst->devPrivate = (pointer) Xalloc(sizeof(mfbPrivPixmap));
    pPrivDst = (mfbPrivPixmapPtr) pDst->devPrivate;
    pDst->drawable.type = pSrc->drawable.type;
    pDst->drawable.pScreen = pSrc->drawable.pScreen;
    pDst->width = pSrc->width;
    pDst->height = pSrc->height;
    pDst->drawable.depth = pSrc->drawable.depth;
    pDst->devKind = PIXMAP_HOST_MEMORY;
    pPrivDst->stride = pPrivSrc->stride;
    pDst->refcnt = 1;

    size = pDst->height * pPrivDst->stride;
    pPrivDst->bits = (pointer) Xalloc(size);
    if (!(pPrivDst->bits))
    {
	Xfree(pPrivDst);
	Xfree(pDst);
	return NullPixmap;
    }
    else
	bzero((char *)pPrivDst->bits, size);

    pSrcInt = (int *)pPrivSrc->bits;
    pDstInt = (int *)pPrivDst->bits;
    pDstMax = pDstInt + (size >> 2);
    /* Copy words */
    while(pDstInt < pDstMax)
    {
        *pDstInt++ = *pSrcInt++;
    }

    return pDst;
}


/* replicates a pattern to be a full 32 bits wide.
   relies on the fact that each scnaline is longword padded.
   doesn't do anything if pixmap is not a factor of 32 wide.
   changes width field of pixmap if successful, so that the fast
	XRotatePixmap code gets used if we rotate the pixmap later.

   calculate number of times to repeat
   for each scanline of pattern
      zero out area to be filled with replicate
      left shift and or in original as many times as needed

   returns TRUE iff pixmap was, or could be padded to be, 32 bits wide.
*/
Bool
mfbPadPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    register int width = pPixmap->width;
    register int h;
    register int mask;
    register unsigned int *p;
    register unsigned int bits;	/* real pattern bits */
    register int i;
    int rep;			/* repeat count for pattern */

    if (width == 32)
	return(TRUE);
    if (width > 32)
	return(FALSE);

    rep = 32/width;
    if (rep*width != 32)
	return(FALSE);

    mask = endtab[width];

    p = (unsigned int *)(((mfbPrivPixmapPtr)(pPixmap->devPrivate))->bits);
    for (h=0; h < pPixmap->height; h++)
    {
	*p &= mask;
	bits = *p;
	for(i=1; i<rep; i++)
	{
	    bits = SCRRIGHT(bits, width);
	    *p |= bits;
	}
	p++;
    }
    pPixmap->width = 32;
    return(TRUE);
}

/* Rotates pixmap pPix by w pixels to the right on the screen. Assumes that
 * words are 32 bits wide, and that the least significant bit appears on the
 * left.
 */
mfbXRotatePixmap(pPix, rw)
    PixmapPtr	pPix;
    register int rw;
{
    register long	*pw, *pwFinal;
    register unsigned long	t;

    if (pPix == NullPixmap)
        return;

    pw = (long *)(((mfbPrivPixmapPtr)(pPix->devPrivate))->bits);
    rw %= pPix->width;
    if (rw < 0)
	rw += pPix->width;
    if(pPix->width == 32)
    {
        pwFinal = pw + pPix->height;
	while(pw < pwFinal)
	{
	    t = *pw;
	    *pw++ = SCRRIGHT(t, rw) | 
		    (SCRLEFT(t, (32-rw)) & endtab[rw]);
	}
    }
    else
    {
	/* We no longer do this.  Validate doesn't try to rotate odd-size
	 * tiles or stipples.  mfbUnnatural<tile/stipple>FS works directly off
	 * the unrotate tile/stipple in the GC
	 */
        ErrorF("X internal error: trying to rotate odd-sized pixmap.\n");
    }

}
/* Rotates pixmap pPix by h lines.  Assumes that h is always less than
   pPix->height
   works on any width.
 */
mfbYRotatePixmap(pPix, rh)
    register PixmapPtr	pPix;
    int	rh;
{
    int nbyDown;	/* bytes to move down to row 0; also offset of
			   row rh */
    int nbyUp;		/* bytes to move up to line rh; also
			   offset of first line moved down to 0 */
    char *pbase;
    char *ptmp;
    mfbPrivPixmapPtr pPrivPix;

    if (pPix == NullPixmap)
	return;

    pPrivPix = (mfbPrivPixmapPtr)pPix->devPrivate;
    rh %= pPix->height;
    if (rh < 0)
	rh += pPix->height;

    pbase = (char *)pPrivPix->bits;

    nbyDown = rh * pPrivPix->stride;
    nbyUp = (pPrivPix->stride * pPix->height) - nbyDown;
    if(!(ptmp = (char *)ALLOCATE_LOCAL(nbyUp)))
	return;

    bcopy(pbase, ptmp, nbyUp);		/* save the low rows */
    bcopy(pbase+nbyUp, pbase, nbyDown);	/* slide the top rows down */
    bcopy(ptmp, pbase+nbyDown, nbyUp);	/* move lower rows up to row rh */
    DEALLOCATE_LOCAL(ptmp);
}

