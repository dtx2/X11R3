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

#ifndef QD_H
#define QD_H

#include "pixmapstr.h"
#include "regionstr.h"

extern RegionPtr mfbPixmapToRegion();

extern Bool qdScreenInit();

extern void	qdSaveAreas ();
extern void	qdRestoreAreas ();

extern int	DragonPix;	/* max y size for offscreen pixmap */

/*
 * On the VAX with pcc
 *      (unsigned)0xffffffff % 2 == 0xffffffff
 * UMOD does not have a discontinuity at 0, as % does
 */
#define UMOD( dend, dor) \
        ( (dend)%(dor) + (((dend) & 0x80000000) ? (dor) : 0))

/*
 * private field of GC
 */
typedef struct {
    unsigned long	mask;		/* see values below, i.e.: QD_*	*/
    int			igreen;		/* index to green table */
    unsigned char *	ptresult;	/* pointer to lookup table */
    RegionPtr		pAbsClientRegion; /* non-NULL if and only if
					   * clientClipType in the GC is
					   * CT_REGION */
    RegionPtr   	pCompositeClip;   /* always valid */
    int			freeCompClip;	  /* FREE_CC or REPLACE_CC */
    short		lastDest; /* any of the drawable types (-1,0,1) */
    unsigned long	GCstate;	/* added to pGC->stateChanges */
} QDPrivGCRec, *QDPrivGCPtr;

/*
 * mask values:  (to be or-ed together)
 */
#define	QD_LOOKUP	(1L)
#define	QD_SRCBIT	(1L<<1)
#define	QD_DSTBYTE	(1L<<2)
#define	QD_NEWLOGIC	(1L<<3)

/*
 * GCstate values:	(to be or-ed together)
 */
#define	VSFullResetBit	0
#define	VSFullReset	(1L<<VSFullResetBit)
#define	VSDestBit	1		/* never set, but looked-up */
#define	VSDest		(1L<<VSDestBit)
#define	VSNone		0
#define	VSNewBits	2

/*
 * freeCompositeClip values
 */
#define REPLACE_CC      0               /* composite clip is a copy of a
                                         * pointer, so it doesn't need to
                                         * be freed; just overwrite it.
                                         * this happens if clientClipType
                                         * is CT_NONE and the gc is clipped 
                                         * by children (GCSubwindowMode).
                                         */
#define FREE_CC         1               /* composite clip is a real
                                         * region that we need to free
                                         */


/*
 * private data in a PIXMAP
 *
 * "data" must be an unsigned char to be able to point to bytes in
 * full-depth image
 */
typedef struct _QDPix {
    unsigned char	*data;		/* pointer to bits */
    DDXPointRec		lastOrg;	/* last patOrg for rotile */
    int			offscreen;	/* y at offscreen location */
} QDPixRec, *QDPixPtr;
#define	NOTOFFSCREEN	(0)

typedef QDPixRec QDPixmapRec;
typedef QDPixPtr QDPixmapPtr;

/*
 * There is only one representation for depth 1 pixmaps (bitmaps).
 *
 * Pad out to 32-bit boundary, like monochrome servers,
 * even though the dragon DMA engine can transfer 16-bit-padded bitmaps.
 */
/*
 * A convenient macro which incorporates the bitmap padding rule:
 */
#define QPPADBYTES( w)	( ((w)+31)>>3 & ~3)	/* return units of bytes */

/*
 * There is only one representation for full-depth pixmaps.
 * This is used when the "format"
 * argument to CreatePixmap() is either XYPixmap or ZPixmap.
 * We can do this because DDX's representation of the pixmap data is hidden
 * DIX code.
 *
 * Pixel data is stored in the order that is natural for
 * the Dragon's DMA engine in Z mode: red bytes, green bytes, then blue bytes.
 * Note that there are no padding bytes whatever.
 */


#if NPLANES==24
#define RED(p)		(p & 0xff)
#define GREEN(p)	(p>>8 & 0xff)
#define BLUE(p)		(p>>16 & 0xff)
#define	TORED(p)	(p & 0xff)
#define	TOGREEN(p)	(p<<8 & 0xff00)
#define	TOBLUE(p)	(((unsigned long) p)<<16 & 0xff0000)
#else	/* NPLANES == 8 */
#define RED(p)		(p & 0xff)
#define GREEN(p)	(p & 0xff)
#define BLUE(p)		(p & 0xff)
#define	TORED(p)	(p & 0xff)
#define	TOGREEN(p)	(p & 0xff)
#define	TOBLUE(p)	(p & 0xff)
#endif


/*
 * For fast text output, we need both a depth 1 pixmap containing
 * the strike-order font, and a horizontal offset table.
 */
typedef struct {
    PixmapPtr	pPixmap;
    int		log2dx;		/* log2 of x dimension of all char cells */
} QDFontRec, *QDFontPtr;

#define QDSLOWFONT	1	/* a QDFontPtr may take this value */

#endif	/* QD_H */

/* ISDRAGONTILE
 *	sets (int) pow2 = 1 if the pixmap is in [4,512] and a power of 2
 *	in width and height.
 */
#define	ISDRAGONTILE(ppixmap,pow2)	\
{	\
	register int	shifted;	\
	pow2 = 1;	/* Yes, is power of 2 */	\
	/* check if tile w,h (belong-to) [4,512] */	\
	if ((ppixmap)->width > 512 || (ppixmap)->height > 512 ||	\
		(ppixmap)->width < 4 || (ppixmap)->height < 4)	\
	    pow2 = 0;	\
	for (shifted = 1; ((ppixmap)->width >> shifted) > 0;	\
		shifted += 1)	\
	{	\
	    if (((ppixmap)->width >> shifted) << shifted	\
		    != (ppixmap)->width)	\
		pow2 = 0;	\
	}	\
	for (shifted = 1; ((ppixmap)->height >> shifted) > 0;	\
		shifted += 1)	\
	{	\
	    if (((ppixmap)->height >> shifted) << shifted	\
		    != (ppixmap)->height)	\
		pow2 = 0;	\
	}	\
}

