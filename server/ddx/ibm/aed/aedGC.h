/***********************************************************
		Copyright IBM Corporation 1987

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* private field of GC */
typedef struct {
    unsigned char	rop;		/* reduction of rasterop to 1 of 3 */
    unsigned char	ropOpStip;	/* rop for opaque stipple */
    unsigned char	ropFillArea;	/* == alu, rop, or ropOpStip */
    short	fExpose;		/* callexposure handling ? */
    short	freeCompClip;
    PixmapPtr	pRotatedTile; 		/* tile/stipple  rotated to align */
    PixmapPtr	pRotatedStipple;	/* with window and using offsets */
    RegionPtr	pAbsClientRegion; 	/* client region in screen coords */
    RegionPtr	pCompositeClip; 	/* FREE_CC or REPLACE_CC */
    void 	(* FillArea)();		/* fills regions; look at the code */
    PixmapPtr   *ppPixmap;		/* points to the pixmapPtr to
				   	   use for tiles and stipples */
    short lastDrawableType;		/* was last drawable window or pixmap */
    } aedPrivGC;
typedef aedPrivGC	*aedPrivGCPtr;
