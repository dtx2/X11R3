/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.
     
                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
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
#include "apc.h"
#include "Xmd.h"
#include "misc.h"
#include "region.h"
#include "gc.h"
#include "pixmapstr.h"

#include "apcmskbits.h"
#include "servermd.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
 /*  HACK ALERT  
	THIS IS CODED ASSUMING 8BITS PER PIXEL, even though it
        looks up the bits per pixel in the screen info record.  
        The pdst pointer is an unsigned char pointer. */

unsigned int *
apcGetSpans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    DDXPointPtr         ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
    int	                *psrc;	/* where to get the bits */
    int			tmpSrc;	/* scratch buffer for bits */
    int                 *psrcBase;	/* start of src bitmap */
    int			widthSrc;	/* width of pixmap in bytes */
    DDXPointPtr      pptLast;	/* one past last point to get */
    int         	xEnd;		/* last pixel to copy from */
    int	                nstart; 
    int	 		nend; 
    int	 		srcStartOver; 
    int	 		startmask, endmask, nlMiddle, srcBit;
    int			w;
    unsigned int	*pdstStart;
    unsigned int	*pdstNext;
    DDXPointPtr	  	pptInit;
    int	    	  	*pwidthInit;
    int	    	  	*pwidthPadded;
    int	    	  	i;

    int	                *BaseBase;
    short               width;
    DDXPointPtr         ppttemp;
    int                 *pwidthtemp;
    int                 pdstoffset, nspanstemp, mainmemoffset;
    gpr_$offset_t 	size;
    gpr_$rgb_plane_t    depth, depthsave;
    status_$t		status;
    Bool             thru_once;
                                   
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

    depth = (gpr_$rgb_plane_t)pDrawable->depth;
    depthsave = depth;

    gpr_$enable_direct_access( status);
    if (depth == 1 ) {
        DDXPointPtr     pptLast;	/* one past last point to get */
        int 		srcStartOver; 
        unsigned int	*pdst;	/* where to put the bits */

        pptLast = ppt + nspans;

        if (pDrawable->type == DRAWABLE_WINDOW) {
	    psrcBase = (int *)(apDisplayData[pDrawable->pScreen->myNum].bitmap_ptr);
	    widthSrc = (int )(apDisplayData[pDrawable->pScreen->myNum].words_per_line) >> 1;
            }
        else {
	    psrcBase = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_ptr;
            widthSrc = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->width >> 1;
            }

        pdstStart = (unsigned int *)Xalloc(nspans * PixmapBytePad(wMax, 1));
        pdstNext = pdstStart;

        i = 0;
        while(ppt < pptLast) {
	    pdst = pdstNext;
            pdstNext += (PixmapBytePad(wMax, 1)>>2);
	    xEnd = min(ppt->x + *pwidth, widthSrc << 5);
	    pwidth++;
	    psrc = psrcBase + (ppt->y * widthSrc) + (ppt->x >> 5); 
	    w = xEnd - ppt->x;
	    srcBit = ppt->x & 0x1f;

           pwidthPadded[i] = PixmapBytePad(w, 1) << 3;
           i++;

	    if (srcBit + w <= 32) { 
	        getbits(psrc, srcBit, w, tmpSrc);
	        putbits(tmpSrc, 0, w, pdst); 
 	        } 
	    else { 

	        maskbits(ppt->x, w, startmask, endmask, nlMiddle);
	        if (startmask) 
		    nstart = 32 - srcBit; 
	        else 
		    nstart = 0; 
	        if (endmask) 
		    nend = xEnd & 0x1f; 
	        srcStartOver = srcBit + nstart > 31;
	        if (startmask) { 
		    getbits(psrc, srcBit, nstart, tmpSrc);
		    putbits(tmpSrc, 0, nstart, pdst);
		    if(srcStartOver)
		        psrc++;
	            } 
	        while (nlMiddle--) { 
		    tmpSrc = *psrc;
		    putbits(tmpSrc, nstart, 32, pdst);
		    psrc++;
		    pdst++;
	            } 
	        if (endmask) { 
		    getbits(psrc, 0, nend, tmpSrc);
		    putbits(tmpSrc, nstart, nend, pdst);
	            } 
	        } 
            ppt++;
            }
        }
    else {
        char	*pdst;	/* where to put the bits */

        if (pDrawable->type == DRAWABLE_WINDOW) {
	    psrcBase = (int *)(apDisplayData[pDrawable->pScreen->myNum].bitmap_ptr);
	    widthSrc = (int )(apDisplayData[pDrawable->pScreen->myNum].words_per_line) >> 1;
            }
        else {
	    BaseBase = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->bitmap_ptr;
            widthSrc = ((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->width >> 1;
            mainmemoffset = widthSrc*((apcPrivPMPtr)(((PixmapPtr)pDrawable)->devPrivate))->size.y_size;
            }

        pdstoffset = PixmapBytePad(wMax, depth);
        pdstStart = (unsigned int *)Xalloc(nspans * pdstoffset);
        pdstoffset = pdstoffset >> 2;
    
        pwidthtemp = pwidth;
        ppttemp = ppt;

        thru_once = FALSE;
        while (depth--) {
            if (pDrawable->type == DRAWABLE_WINDOW) gpr_$remap_color_memory(depth, status);
            else psrcBase = (int *)(mainmemoffset*depth+BaseBase);

	    pwidth = pwidthtemp;
            ppt = ppttemp;
            nspanstemp = nspans;
            pdstNext = pdstStart;

            i = 0;
            while(nspanstemp--) {
	        xEnd = min(ppt->x + *pwidth, widthSrc << 5);
	        psrc = psrcBase + ppt->y * widthSrc + (ppt->x >> 5); 
	        w = xEnd - ppt->x;
	        srcBit = ppt->x & 0x1f;

               if ( !thru_once ) {
                  pwidthPadded[i] = PixmapBytePad(w, (depth+1)) << 3;
                  i++;
               }

                pdst = (char *)pdstNext;
	        pdstNext = pdstNext + pdstoffset;

	        if (srcBit + w <= 32) { 
	            getbits(psrc, srcBit, w, tmpSrc);
		    putbitsapc(depth, tmpSrc, w, pdst);
	            } 
	        else { 
 	            maskbits(ppt->x, w, startmask, endmask, nlMiddle);
	            if (startmask) 
		        nstart = 32 - srcBit; 
	            else 
		        nstart = 0;                                                              
	            if (endmask) 
		        nend = xEnd & 0x1f; 
        	    srcStartOver = srcBit + nstart > 31;
	            if (startmask) { 
		       getbits(psrc, srcBit, nstart, tmpSrc);
		       tmpSrc >>= srcBit;
                       putbitsapc(depth, tmpSrc, nstart, pdst);
                       if(srcStartOver)
		           psrc++;
	               } 
	            while (nlMiddle--) { 
		        tmpSrc = *psrc++;
                        putbitsapc(depth, tmpSrc, 32, pdst);
	                } 
	            if (endmask) { 
		        getbits(psrc, 0, nend, tmpSrc);
        	        tmpSrc >>= 32-nend;
                        putbitsapc(depth, tmpSrc, nend, pdst);
	                } 
                    /* assuming no need to zero remainder of span memory */
	            } 
                ppt++;
	        pwidth++;
	        }
            }
            thru_once = TRUE;
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

	pPixmap = (PixmapPtr)ALLOCATE_LOCAL(sizeof(PixmapRec));
	if ((pPixmap != (PixmapPtr)NULL) && (pwidthPadded != (int *)NULL))
	{
	    pPixmap->drawable.type = DRAWABLE_PIXMAP;
	    pPixmap->drawable.pScreen = pDrawable->pScreen;
	    pPixmap->drawable.depth = 1;
	    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    pPixmap->devKind = PixmapBytePad(wMax, depthsave) * nspans;
	    pPixmap->width = pPixmap->devKind << 3;
	    pPixmap->height = 1;
	    pPixmap->refcnt = 1;
	    pPixmap->devPrivate = (pointer)pdstStart;
	    miBSGetSpans(pDrawable, pPixmap, wMax, pptInit, pwidthInit,
			 pwidthPadded, nspans);
	}
	DEALLOCATE_LOCAL(pPixmap);
    }

    DEALLOCATE_LOCAL(pwidthPadded);
	
    return(pdstStart);
}

