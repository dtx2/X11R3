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
/* pixmap management
   written by drewry, september 1986

   on a monchrome device, a pixmap is a bitmap.
*/

#include "Xmd.h"
#include "servermd.h"
#include "pixmapstr.h"
#include "cfbmskbits.h"

#include "cfb.h"
#include "mi.h"
#include "scrnintstr.h"
#include "gcstruct.h"

PixmapPtr cfbCreatePixHdr(pScreen, width, height, depth, data)
    ScreenPtr	pScreen;
    int		width, height, depth;
    pointer	data;
{
    register PixmapPtr pPixmap;

#ifdef notdef
    pPixmap = (PixmapPtr)Xalloc(sizeof(PixmapRec));
    pPixmap->devPrivate = (pointer)Xalloc(sizeof(cfbPrivPixmap));
#else
    if(!(pPixmap = (PixmapPtr)malloc(sizeof(PixmapRec)))) 
        return (PixmapPtr) NULL;
    if(!(pPixmap->devPrivate = (pointer)malloc(sizeof(cfbPrivPixmap)))) {
	free(pPixmap);
        return (PixmapPtr) NULL;
    }
#endif
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->width = width;
    pPixmap->height = height;
    pPixmap->refcnt = 1;
    ((cfbPrivPixmapPtr) pPixmap->devPrivate)->stride =
		PixmapBytePad(width, depth);
	/* in case weez called from the outside world ... */
    pPixmap->devKind = PIXMAP_HOST_MEMORY;
    ((cfbPrivPixmapPtr) pPixmap->devPrivate)->bits = data;

    return pPixmap;
}

PixmapPtr
cfbCreatePixmap (pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    register PixmapPtr pPixmap;
    cfbPrivPixmapPtr pPrivPixmap;
    int size;

    if (depth != 1 && depth != PSZ) return (PixmapPtr)NULL;

#if 0
    pPixmap = (PixmapPtr)Xalloc(sizeof(PixmapRec));
    pPixmap->devPrivate = (pointer)Xalloc(sizeof(cfbPrivPixmap));
    pPrivPixmap = (cfbPrivPixmapPtr) pPixmap->devPrivate;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->width = width;
    pPixmap->height = height;
    pPixmap->refcnt = 1;
    pPrivPixmap->stride = PixmapBytePad(width, depth);
#else
    if(!(pPixmap = cfbCreatePixHdr(pScreen, width, height, 
	 depth, (pointer)NULL))) 
        return (PixmapPtr) NULL;
    pPrivPixmap = (cfbPrivPixmapPtr) pPixmap->devPrivate;
#endif
    if ((depth == PSZ) &&
	(pPrivPixmap->pChunk = (pointer)hpBufAlloc(pScreen,
						   pPrivPixmap->stride,
						   height))) {
	pPrivPixmap->stride =
	    ((cfbPrivScreenPtr)(pScreen->devPrivate))->stride;
	pPixmap->devKind = PIXMAP_FRAME_BUFFER;
	pPrivPixmap->bits = (pointer)
	    (((cfbPrivScreenPtr)(pScreen->devPrivate))->bits) +
            (((hpChunk *)(pPrivPixmap->pChunk))->y * pPrivPixmap->stride) +
            ((hpChunk *)(pPrivPixmap->pChunk))->x;
    }
    else { /* depth == 1 or no off-screen memory */
	pPixmap->devKind = PIXMAP_HOST_MEMORY;
	size = height * pPrivPixmap->stride;
#ifdef notdef
	if ( !(pPrivPixmap->bits = (pointer)Xalloc(size)))
	    {
		Xfree(pPrivPixmap);
		Xfree(pPixmap);
		return (PixmapPtr)NULL;
	    }
#else
	if ( !(pPrivPixmap->bits = (pointer)malloc(size)))
	    {
		free(pPrivPixmap);
		free(pPixmap);
		return (PixmapPtr)NULL;
	    }
#endif
	else
	    bzero((char *)pPrivPixmap->bits, size);
    }
    return pPixmap;
}
PixmapPtr
cfbCreateOffscreenPixmap (pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    register PixmapPtr pPixmap;
    cfbPrivPixmapPtr pPrivPixmap;
    int size;

    if (depth != 1 && depth != PSZ) return (PixmapPtr)NULL;

    if(!(pPixmap = cfbCreatePixHdr(pScreen, width, height, 
	 depth, (pointer)NULL))) 
        return (PixmapPtr) NULL;
    pPrivPixmap = (cfbPrivPixmapPtr) pPixmap->devPrivate;
    if ((depth == PSZ) &&
	(pPrivPixmap->pChunk = (pointer)hpBufAlloc(pScreen,
						   pPrivPixmap->stride,
						   height))) {
	pPrivPixmap->stride =
	    ((cfbPrivScreenPtr)(pScreen->devPrivate))->stride;
	pPixmap->devKind = PIXMAP_FRAME_BUFFER;
	pPrivPixmap->bits = (pointer)
	    (((cfbPrivScreenPtr)(pScreen->devPrivate))->bits) +
            (((hpChunk *)(pPrivPixmap->pChunk))->y * pPrivPixmap->stride) +
            ((hpChunk *)(pPrivPixmap->pChunk))->x;
    }
    else { /* Cannot create offscreen pixmap */
		free(pPrivPixmap);
		free(pPixmap);
		pPixmap = (PixmapPtr)NULL;
    }
    return pPixmap;
}

#ifdef notdef
#define FREEPIXHDR() Xfree(pPixmap->devPrivate); Xfree(pPixmap);
#else
#define FREEPIXHDR() free(pPixmap->devPrivate); free(pPixmap);
#endif

void cfbDestroyPixHdr(pPixmap) PixmapPtr pPixmap; { FREEPIXHDR(); }

Bool cfbDestroyPixmap(pPixmap) PixmapPtr pPixmap;
{
/* BOGOSITY ALERT */
    if ((unsigned)pPixmap < 42)
	return TRUE;

    if(--pPixmap->refcnt) return TRUE;
    if (pPixmap->devKind == PIXMAP_FRAME_BUFFER)
	hpBufFree ((pPixmap->drawable.pScreen),
		   ((hpChunk *)
		    ((cfbPrivPixmapPtr)pPixmap->devPrivate)->pChunk));
    else
#ifdef notdef
	Xfree(((cfbPrivPixmapPtr)pPixmap->devPrivate)->bits);
#else
	free(((cfbPrivPixmapPtr)pPixmap->devPrivate)->bits);
#endif
    FREEPIXHDR();
    return TRUE;
}

PixmapPtr
cfbCopyPixmap(pSrc)
    register PixmapPtr	pSrc;
{
    register PixmapPtr		 pDst;
    register int		*pSrcInt, *pDstInt, *pSrcI, *pDstI, *pDstMax;
    register cfbPrivPixmapPtr    pPrivDst, pPrivSrc;
    int		                 copyWords, i;
    void			(*bitMover)();

    pPrivSrc = (cfbPrivPixmapPtr) pSrc->devPrivate;
    pDst = cfbCreatePixmap(pSrc->drawable.pScreen, pSrc->width,
			   pSrc->height, pSrc->drawable.depth,
			   0);
    if (!pDst)
	return NullPixmap;
    pPrivDst = (cfbPrivPixmapPtr) pDst->devPrivate;
    

    pSrcInt = (int *)pPrivSrc->bits;
    pDstInt = (int *)pPrivDst->bits;
    if (pSrc->drawable.depth == 1) { /* bit per pixel */
	pDstMax = pDstInt + ((pDst->height * pPrivDst->stride) >> 2);
	/* Copy words */
	while(pDstInt < pDstMax)
	    {
		*pDstInt++ = *pSrcInt++;
	    }
    }
    else { /* byte per pixel */
	if ((pSrc->devKind == PIXMAP_FRAME_BUFFER) &&
	    (pDst->devKind == PIXMAP_FRAME_BUFFER) &&
	    (pSrc->drawable.pScreen == pDst->drawable.pScreen)) {
	    bitMover = ((cfbPrivScreenPtr)
			(pSrc->drawable.pScreen->devPrivate))->MoveBits;
	    (*bitMover)(pSrc->drawable.pScreen, ~0, GXcopy,
			((hpChunk *)(pPrivSrc->pChunk))->x,
			((hpChunk *)(pPrivSrc->pChunk))->y,
			((hpChunk *)(pPrivDst->pChunk))->x,
			((hpChunk *)(pPrivDst->pChunk))->y,
			pSrc->width, pSrc->height);
	}
	else
	    {
	    if (pSrc->devKind == PIXMAP_FRAME_BUFFER)
		WAIT_READY_TO_RENDER(pSrc->drawable.pScreen);
	    if (pDst->devKind == PIXMAP_FRAME_BUFFER)
                SET_REGISTERS_FOR_WRITING(pDst->drawable.pScreen, ~0, GXcopy);
	    copyWords = (pDst->width + 3) >> 2; /* width will be in bytes */
	    for (i=0;
		 i < pDst->height;
		 pSrcInt = (int *)((char *)pSrcInt + pPrivSrc->stride),
		 pDstInt = (int *)((char *)pDstInt + pPrivDst->stride),
		 i++) {
		pDstMax = pDstInt + copyWords;
		pSrcI = pSrcInt;
		pDstI = pDstInt;
		while (pDstI < pDstMax)
		    *pDstI++ = *pSrcI++;
	    }
	}
    }
    
    return pDst;
}


/* replicates a pattern to be a full 32 bits wide.
   relies on the fact that each scnaline is longword padded.
   doesn't do anything if pixmap is not a factor of 32 wide.
   changes width field of pixmap if successful, so that the fast
	cfbXRotatePixmap code gets used if we rotate the pixmap later.
	cfbXRotatePixmap code gets used if we rotate the pixmap later.

   calculate number of times to repeat
   for each scanline of pattern
      zero out area to be filled with replicate
      left shift and or in original as many times as needed

   returns TRUE iff pixmap was, or could be padded to be, 32 bits wide.
*/
Bool
cfbPadPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    /* do nothing for now; eventually pad to a 4-byte boundary. */
    return( FALSE );
}


#ifdef notdef
/*
 * cfb debugging routine -- assumes pixmap is 1 byte deep 
 */
static cfbdumppixmap(pPix)
    PixmapPtr	pPix;
{
    unsigned int *pw;
    char *psrc, *pdst;
    int	i, j;
    char	line[66];

    ErrorF(  "pPixmap: 0x%x\n", pPix);
    ErrorF(  "%d wide %d high\n", pPix->width, pPix->height);
    if (pPix->width > 64)
    {
	ErrorF(  "too wide to see\n");
	return;
    }

    pw = (unsigned int *)(((cfbPrivPixmapPtr)(pPix->devPrivate))->bits);
    psrc = (char *) pw;

/*
    for ( i=0; i<pPix->height; ++i )
	ErrorF( "0x%x\n", pw[i] );
*/

    for ( i = 0; i < pPix->height; ++i ) {
	pdst = line;
	for(j = 0; j < pPix->width; j++) {
	    *pdst++ = *psrc++ ? 'X' : ' ' ;
	}
	*pdst++ = '\n';
	*pdst++ = '\0';
	ErrorF( "%s", line);
    }
}
#endif notdef

/* Rotates pixmap pPix by w pixels to the right on the screen. Assumes that
 * words are 32 bits wide, and that the least significant bit appears on the
 * left.
 */
cfbXRotatePixmap(pPix, rw)
    PixmapPtr	pPix;
    register int rw;
{
    register long	*pw, *pwFinal, *pwNew;
    register unsigned long	t;
    int			size, i;
    cfbPrivPixmapPtr	pPrivPix;

    if (pPix == NullPixmap)
        return;

    pPrivPix = (cfbPrivPixmapPtr)(pPix->devPrivate);

    switch (((DrawablePtr) pPix)->depth) {
	case PSZ:
	    break;
	case 1:
	    cfbXRotateBitmap(pPix, rw);
	    return;
	default:
	    ErrorF("cfbXRotatePixmap: unsupported depth %d\n", ((DrawablePtr) pPix)->depth);
	    return;
    }
    pw = (long *)(pPrivPix->bits);
    rw %= pPix->width;
    if (rw < 0)
	rw += pPix->width;
    if (rw == 0)
	return;
    if (pPix->devKind == PIXMAP_HOST_MEMORY) {
	if(pPix->width == PPW)
	    {
		pwFinal = pw + pPix->height;
		while(pw < pwFinal)
		    {
			t = *pw;
			*pw++ = SCRRIGHT(t, rw) | 
			    (SCRLEFT(t, (PPW-rw)) & cfbendtab[rw]);
		    }
	    }
	else
	    {
		pwNew = (long *) Xalloc( pPix->height * pPrivPix->stride);
		
		/* o.k., divide pw (the pixmap) in two vertically at (w - rw)
		 * pick up the part on the left and make it the right of the new
		 * pixmap.  then pick up the part on the right and make it the left
		 * of the new pixmap.
		 * now hook in the new part and throw away the old. All done.
		 */
		size = PixmapBytePad(pPix->width, PSZ) >> 2;
		cfbQuickBlt(pw, pwNew, 0, 0, rw, 0, pPix->width - rw, pPix->height,
			    size, size);
		cfbQuickBlt(pw, pwNew, pPix->width - rw, 0, 0, 0, rw, pPix->height,
			    size, size);
		pPrivPix->bits = (pointer) pwNew;
		Xfree((char *) pw);

	    }
    }
    else  /* PIXMAP_FRAME_BUFFER */
	{
	    if(pPix->width == PPW)
		{
                    SET_REGISTERS_FOR_WRITING(pPix->drawable.pScreen, 
					      ~0, GXcopy);
		    for (i=0; i<pPix->height; i++, pw += pPrivPix->stride) {
			t = *pw;
			*pw = SCRRIGHT(t, rw) | 
			    (SCRLEFT(t, (PPW-rw)) & cfbendtab[rw]);
		    }
		}
	    else
		{
		    /* o.k., divide pw (the pixmap) in two vertically at (w - rw)
		     * pick up the part on the left and make it the right of the temp
		     * pixmap.  then pick up the part on the right and make it the left
		     * of the original pixmap. now copy in the part saved in temp and
		     * discard temp. All done.
		     */
		    PixmapPtr pTemp;
		    short realTemp = 0;

                    if (((pPix->width - rw) > PRIV_PIX_WIDTH) ||
			(pPix->height > PRIV_PIX_HEIGHT)) {
			pTemp = (* pPix->drawable.pScreen->CreatePixmap)
			    (pPix->drawable.pScreen, pPix->width - rw,
			     pPix->height, pPix->drawable.depth);
			realTemp = 1;
		    }
		    else { /* use pre-allocated pixmap */
			pTemp =
			    (PixmapPtr)
				((cfbPrivScreen *)
				 (pPix->drawable.pScreen->devPrivate))->pTmpPixmap;
		    };

		    if (pTemp->devKind == PIXMAP_FRAME_BUFFER) {

			register hpChunk *pixChunk;
			register hpChunk *tempChunk;
			register void (*bitMover)();

			tempChunk = (hpChunk *)
			    ((cfbPrivPixmapPtr)(pTemp->devPrivate))->pChunk;
			pixChunk = ((hpChunk *)(pPrivPix->pChunk));

			bitMover = ((cfbPrivScreenPtr)
				    (pPix->drawable.pScreen->devPrivate))->MoveBits;

			(*bitMover)(pPix->drawable.pScreen, ~0, GXcopy,
				    pixChunk->x,
				    pixChunk->y,
				    tempChunk->x,
				    tempChunk->y,
				    pPix->width - rw, pPix->height);

			(*bitMover)(pPix->drawable.pScreen, ~0, GXcopy,
				    (pixChunk->x + pPix->width - rw),
				    pixChunk->y,
				    pixChunk->x,
				    pixChunk->y,
				    rw, pPix->height);

			(*bitMover)(pPix->drawable.pScreen, ~0, GXcopy,
				    tempChunk->x,
				    tempChunk->y,
				    (pixChunk->x + rw),
				    pixChunk->y,
				    pPix->width - rw, pPix->height);
			if (realTemp)
			    (* pPix->drawable.pScreen->DestroyPixmap)(pTemp);
		    }
		    else /* forced to use generic code */
			{
			    GCPtr pGC;
			    CARD32 attribute;

			    pGC = GetScratchGC(pPix->drawable.depth, 
					       pPix->drawable.pScreen);
			    
			    attribute = GXcopy;
			    ChangeGC(pGC, GCFunction, &attribute, 1);
			    ValidateGC(pTemp, pGC);
			    
			    (* pGC->CopyArea)(pPix, pTemp, pGC,
					      0, 0,
					      pPix->width - rw, pPix->height, 
					      0, 0);
			    ValidateGC(pPix, pGC);
			    (* pGC->CopyArea)(pPix, pPix, pGC,
					      pPix->width - rw, 0,
					      rw, pPix->height, 
					      0, 0);
			    (* pGC->CopyArea)(pTemp, pPix, pGC,
					      0, 0,
					      pPix->width - rw, pPix->height,
					      rw, 0);
			    (* pPix->drawable.pScreen->DestroyPixmap)(pTemp);
			    FreeScratchGC(pGC);
			}
		}
	}
}
/* Rotates pixmap pPix by h lines.  Assumes that h is always less than
   pPix->height
   works on any width.
 */
cfbYRotatePixmap(pPix, rh)
    register PixmapPtr	pPix;
    int	rh;
{
    int nbyDown;	/* bytes to move down to row 0; also offset of
			   row rh */
    int nbyUp;		/* bytes to move up to line rh; also
			   offset of first line moved down to 0 */
    char *pbase;
    char *ptmp;
    cfbPrivPixmapPtr pPrivPix;

    if (pPix == NullPixmap)
	return;

    pPrivPix = (cfbPrivPixmapPtr)(pPix->devPrivate);

    switch (((DrawablePtr) pPix)->depth) {
	case PSZ:
	    break;
	case 1:
	    mfbYRotatePixmap(pPix, rh);
	    return;
	default:
	    ErrorF("cfbYRotatePixmap: unsupported depth %d\n", ((DrawablePtr) pPix)->depth);
	    return;
    }

    rh %= pPix->height;
    if (rh < 0)
	rh += pPix->height;
    if (rh == 0)
	return;
    if (pPix->devKind == PIXMAP_HOST_MEMORY) {
	pbase = (char *)(pPrivPix->bits);
	nbyDown = rh * pPrivPix->stride;
	nbyUp = (pPrivPix->stride * pPix->height) - nbyDown;
	if(!(ptmp = (char *)ALLOCATE_LOCAL(nbyUp)))
	    return;
	
	bcopy(pbase, ptmp, nbyUp);		/* save the low rows */
	bcopy(pbase+nbyUp, pbase, nbyDown);	/* slide the top rows down */
	bcopy(ptmp, pbase+nbyDown, nbyUp);	/* move lower rows up to row rh */
	DEALLOCATE_LOCAL(ptmp);
    }
    else  /* PIXMAP_FRAME_BUFFER */
	{
	    /* o.k., divide the pixmap in two horizontally at (h - rh)
	     * pick up the part on the top and make it the temp
	     * pixmap.  then pick up the part on the bottom and make it the top
	     * of the original pixmap. now copy in the part saved in temp and
	     * discard temp. All done.
	     */
	    PixmapPtr pTemp;
	    short realTemp = 0;
	    
	    if ((pPix->width > PRIV_PIX_WIDTH) ||
		((pPix->height - rh) > PRIV_PIX_HEIGHT)) {
		pTemp = (* pPix->drawable.pScreen->CreatePixmap)
		    (pPix->drawable.pScreen, pPix->width,
		     pPix->height - rh, pPix->drawable.depth);
		realTemp = 1;
	    }
	    else { /* use pre-allocated pixmap */
		pTemp =
		    (PixmapPtr)
			((cfbPrivScreen *)
			 (pPix->drawable.pScreen->devPrivate))->pTmpPixmap;
	    };

	    if (pTemp->devKind == PIXMAP_FRAME_BUFFER) {

		register hpChunk *pixChunk;
		register hpChunk *tempChunk;
		register void (*bitMover)();
		
		tempChunk = (hpChunk *)
		    ((cfbPrivPixmapPtr)(pTemp->devPrivate))->pChunk;
		pixChunk = ((hpChunk *)(pPrivPix->pChunk));
		
		bitMover = ((cfbPrivScreenPtr)
			    (pPix->drawable.pScreen->devPrivate))->MoveBits;

		(*bitMover)(pPix->drawable.pScreen, ~0, GXcopy,
			    pixChunk->x,
			    pixChunk->y,
			    tempChunk->x,
			    tempChunk->y,
			    pPix->width, pPix->height - rh);
		
		(*bitMover)(pPix->drawable.pScreen, ~0, GXcopy,
			    pixChunk->x,
			    (pixChunk->y + pPix->height - rh),
			    pixChunk->x,
			    pixChunk->y,
			    pPix->width, rh);
		
		(*bitMover)(pPix->drawable.pScreen, ~0, GXcopy,
			    tempChunk->x,
			    tempChunk->y,
			    pixChunk->x,
			    (pixChunk->y + rh),
			    pPix->width, pPix->height - rh);
		if (realTemp)
		    (* pPix->drawable.pScreen->DestroyPixmap)(pTemp);
	    }
	    else /* forced to use generic code */
		{
		    GCPtr pGC;
		    CARD32 attribute;
		    
		    pGC = GetScratchGC(pPix->drawable.depth, 
				       pPix->drawable.pScreen);
		    
		    attribute = GXcopy;
		    ChangeGC(pGC, GCFunction, &attribute, 1);
		    ValidateGC(pTemp, pGC);
		    
		    (* pGC->CopyArea)(pPix, pTemp, pGC,
				      0, 0,
				      pPix->width, pPix->height - rh, 
				      0, 0);
		    ValidateGC(pPix, pGC);
		    (* pGC->CopyArea)(pPix, pPix, pGC,
				      0, pPix->height - rh,
				      pPix->width, rh, 
				      0, 0);
		    (* pGC->CopyArea)(pTemp, pPix, pGC,
				      0, 0,
				      pPix->width, pPix->height - rh,
				      0, rh);
		    (* pPix->drawable.pScreen->DestroyPixmap)(pTemp);
		    FreeScratchGC(pGC);
		}
	}
}

