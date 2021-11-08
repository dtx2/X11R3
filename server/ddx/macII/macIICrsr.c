/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
/*-
 * macIICursor.c --
 *	Functions for maintaining the software cursor...
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/*
 * A cursor can be in one of 3 states:
 *	IN  	  It is actually in the frame buffer.
 *	OUT 	  It is not in the frame buffer and the screen should be
 *	    	  consulted for new screenBits before it is replaced.
 *	XING	  It is changing state. Used when calling mfb functions to
 *	    	  play with the cursor. Avoids infinite recursion, you know.
 *	
 * Two references for cursor coordinates are being used in this code. 
 * The outside world (dix code) and most of macIICursors use the cursor 
 * hot-spot as a reference for the cursor's location. macIICursor internal 
 * code uses the top-left corner of the cursor glyph as a reference 
 * for painting operations. 
 * 
 */

#define NEED_EVENTS
#include    "macII.h"
#include    <windowstr.h>
#include    <regionstr.h>
#include    <dix.h>
#include    <dixstruct.h>
#include    <opaque.h>

#include    <servermd.h>

static CursorPtr  currentCursor = NullCursor;	/* Cursor being displayed */
static BoxRec  	  currentLimits;		/* Box w/in which the hot spot
						 * must stay. */
/*
 * There are four window functions which bypass the usual GC validation
 * path (PaintWindow{Background,Border}, CopyWindow & ClearToBackground)
 * so we must go out of
 * our way to protect the cursor from them. This is accomplished by
 * intercepting the two screen calls which change the window vectors so
 * we can note when they do and substitute our own function which figures
 * out what's going to be nuked and makes sure the cursor isn't there.
 * The structure for the window is tracked in a somewhat sneaky way:
 * we create a new resource class (not type) and use that to associate
 * the WinPrivRec with the window (using the window's id) in the resource
 * table. This makes it easy to find and has the added benefit of freeing
 * the private data when the window is destroyed.
 */
typedef struct {
    void	(*PaintWindowBackground)();
    void	(*PaintWindowBorder)();
    void	(*CopyWindow)();
    void    	(*SaveAreas)();
    RegionPtr  	(*RestoreAreas)();
    void	(*DrawGuarantee)();
} WinPrivRec, *WinPrivPtr;

static int	wPrivClass;		/* Resource class for icky private
					 * window structure (WinPrivRec)
					 * needed to protect the cursor
					 * from background/border paintings */


/*
 * The following 6 procedures provide optimization for monochrome and
 * color cursors whose widths are 16 pixels or less. Tests in the
 * unoptimized versions dispatch to these special routines.
 * THESE PROCEDURES ARE MC680x0 DEPENDENT. In particular, in the monochrome
 * code, I've assumed that a long word of 32 bits can be read and stored
 * on an 8 bit boundary -- this is risky for RISC. Other assumptions:
 * 	
 *	- The width of the monochrome screen is a multiple of 8 pixels.
 *	  The width of a color screen is a multiple of 4 pixels.
 *
 *	- The pixel part of a Pixmap is long word aligned and padded
 *	  out with zeroes to a long word boundary. In the monochrome
 *	  case this leads to reading and writing with single long
 *	  word accesses.
 *
 *	- 680x0 bit and byte orderings.
 * 
 *	- Zero fill from the left and right on logical shifts.
 *
 *	- The coordinates of the upper left hand corner of a cursor
 *	  never exceed the width and height of the screen. (On this
 *	  assumption, this code will never touch memory off the
 *	  visible portion of the framebuffer.
 *
 * These optimizations use a different "buffering" strategy than the
 * original code. Here, a block 32 pixels wide and height twice the
 * cursor height is anchored horizontally to the rightmost 16 pixel 
 * boundary which allows enclosure of the cursor (the vertical anchor
 * is unchanged). But the bounding box never extends beyond the left
 * or right edges of the framebuffer.
 * Note that this strategy is biased in favor of the horizontal direction 
 * in which the cursor last moved, this appears to dominate the centered 
 * strategy.
 * 
 */
/*-
 *-----------------------------------------------------------------------
 * macIIMonoOptRemoveCursor --
 *	Remove the current cursor from the screen, if it hasn't been removed
 *	already.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the cursor is removed, the state field of the cursor-private
 *	structure is set to CR_OUT. In addition, macIIInputAvail is called.
 *
 *-----------------------------------------------------------------------
 */
void
macIIMonoOptRemoveCursor ()
{
    CrPrivPtr	  pPriv;    	/* Private data for this cursor */
    ScreenPtr	  pScreen;  	/* Screen on which the cursor is */
    PtrPrivPtr	  pPtrPriv;	/* XXX: Pointer private data */
    DevicePtr	  pDev;

    pDev = LookupPointerDevice();

    pPtrPriv = (PtrPrivPtr) pDev->devicePrivate;
    pScreen = pPtrPriv->pScreen;
    
    pPriv = (CrPrivPtr)currentCursor->devPriv[pScreen->myNum];

    if (pPriv->state == CR_IN) {
	/*
	 * XXX: Makes use of the devPrivate field of the screen. This *must*
	 * be a pixmap representing the entire screen. How else can I get a
	 * pixmap to draw to?
	 */

	/*
	 * Return the virgin screen bits to the framebuffer, thereby
	 * removing the cursor.
	 */
	{
	    register int h = pPriv->screenBits->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->screenBits->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)(((PixmapPtr)pScreen->devPrivate)->devPrivate);
	    register int widthDst = 
		(int)(((PixmapPtr)pScreen->devPrivate)->devKind) >> 2;

	    pDst += pPriv->scrY * widthDst;
	    pDst = (unsigned int *) /* wicked alignment hack! */
		((unsigned char *)pDst + (pPriv->scrX >> 3));

	    if (h > 0) do {
		*pDst = *pSrc++;
		pDst += widthDst;
	    } while (--h);
	}
	pPriv->state = CR_OUT;
	isItTimeToYield++;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIMonoOptPutCursor --
 *	Place the current cursor in the screen at the given coordinates.
 *	screenBits must already have been filled. If 'direct' is FALSE,
 *	the cursor will be drawn into the temp pixmap after the screenBits
 *	have been copied there. The temp pixmap will then be put onto
 *	the screen. Used for more flicker-free cursor motion...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The state field of the private data is set to CR_IN...
 *
 *-----------------------------------------------------------------------
 */

static void
macIIMonoOptPutCursor (pScreen, pPriv, hotX, hotY, direct)
    ScreenPtr	  	pScreen;    	/* Screen on which to put it */
    CrPrivPtr	  	pPriv;	    	/* Cursor-private data */
    int	    	  	hotX;
    int	    	  	hotY;
    Bool    	  	direct;	    	/* TRUE if should put the cursor */
					/* directly onto the screen, not */
					/* into pPriv->temp first... */
{
    register PixmapPtr	pPixmap;
    int			x = hotX - currentCursor->xhot;
    int 		y = hotY - currentCursor->yhot;
    

    if (!direct) {
	pPixmap = pPriv->temp;
	
	/*
	 * Duplicate the contents of the screenBits in pPriv->temp.
	 */
	{
	    register int h = pPriv->screenBits->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->screenBits->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)pPixmap->devPrivate;

	    if (h > 0) do { *pDst++ = *pSrc++; } while (--h);
	}
	
	/*
	 * Push foreground and background pixels through the source
	 * and inverse source bitmaps respectively.
	 */
	{
	    register int h = pPriv->source->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->source->devPrivate;
	    register unsigned int *pinvSrc = 
		(unsigned int *)pPriv->invSource->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)pPixmap->devPrivate;
	    register unsigned int fgFill = (pPriv->fg ? 0xffffffff : 0x0);
	    register unsigned int bgFill = (pPriv->bg ? 0xffffffff : 0x0);

	    y = y - pPriv->scrY;
	    if (y < 0) {
		h += y;
		pSrc += (-y);
		pinvSrc += (-y);
		y = 0;
	    }
	    if (y > (pPixmap->height - h)) h = pPixmap->height - y;
	    pDst += y;

	    x = x - pPriv->scrX;
	    if (h > 0) do {
		register unsigned int fgMask =
		    (x < 0 ? *pSrc++ << (-x) : *pSrc++ >> x);
		register unsigned int bgMask =
		    (x < 0 ? *pinvSrc++ << (-x) : *pinvSrc++ >> x);
		*pDst = (*pDst & ~fgMask & ~bgMask) |
			(fgFill & fgMask) | (bgFill & bgMask);
		pDst++;
	    } while (--h);
	}

	/*
	 * Now put the whole buffer onto the screen
	 */
	{
	    register int h = pPixmap->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPixmap->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)(((PixmapPtr)pScreen->devPrivate)->devPrivate);
	    register int widthDst = 
		(int)(((PixmapPtr)pScreen->devPrivate)->devKind) >> 2;

	    pDst += pPriv->scrY * widthDst;
	    pDst = (unsigned int *) /* wicked alignment hack! */
		((unsigned char *)pDst + (pPriv->scrX >> 3));

	    if (h > 0) do {
		*pDst = *pSrc++;
		pDst += widthDst;
	    } while (--h);
	}
	pPriv->state = CR_IN;
    } else {
	pPixmap = (PixmapPtr) pScreen->devPrivate;

	/*
	 * Push foreground and background pixels through the source
	 * and inverse source bitmaps respectively.
	 */
	{
	    register int h = pPriv->source->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->source->devPrivate;
	    register unsigned int *pinvSrc = 
		(unsigned int *)pPriv->invSource->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)(pPixmap->devPrivate);
	    register int widthDst = 
		(int)(pPixmap->devKind) >> 2;
	    register unsigned int fgFill = (pPriv->fg ? 0xffffffff: 0x0);
	    register unsigned int bgFill = (pPriv->bg ? 0xffffffff: 0x0);

	    if (y < 0) {
		h += y;
		pSrc += (-y);
		pinvSrc += (-y);
		y = 0;
	    }
	    if (y > pPixmap->height - h) h = pPixmap->height - y;
	    pDst += y * widthDst;
	    pDst = (unsigned int *) /* wicked alignment hack! */
		((unsigned char *)pDst + (pPriv->scrX >> 3));

	    x = x - pPriv->scrX;
	    if (h > 0) do {
		register unsigned int fgMask =
		(x < 0 ? *pSrc++ << (-x) : *pSrc++ >> x);
		register unsigned int bgMask =
		(x < 0 ? *pinvSrc++ << (-x) : *pinvSrc++ >> x);

		*pDst = (*pDst & ~fgMask & ~bgMask) |
			(fgFill & fgMask) | (bgFill & bgMask);
		pDst += widthDst;
	    } while (--h);
	}
	pPriv->state = CR_IN;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIMonoOptRestoreCursor --
 *	Redraw the cursor if it was removed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be replaced and the 'state' field changed.
 *
 *-----------------------------------------------------------------------
 */
void
macIIMonoOptRestoreCursor()
{
    PtrPrivPtr	  	pptrPriv;
    CrPrivPtr	  	pPriv;
    ScreenPtr	  	pScreen;
    DevicePtr		pDev;
    register PixmapPtr	screenBits;
    int	    	  	scrX, scrY;

    pDev = LookupPointerDevice();

    pptrPriv = (PtrPrivPtr) pDev->devicePrivate;
    pScreen = pptrPriv->pScreen;
    pPriv = (CrPrivPtr) currentCursor->devPriv[pScreen->myNum];
    screenBits = pPriv->screenBits;

    if (pPriv->state == CR_OUT) {
	scrX = ((pptrPriv->x - currentCursor->xhot) >> 4) << 4;
	scrX = min(scrX, ((PixmapPtr)pScreen->devPrivate)->width - 32);
        scrY = pptrPriv->y - currentCursor->yhot - screenBits->height / 4;
	scrY = min(scrY, ((PixmapPtr)pScreen->devPrivate)->height - 
			  screenBits->height);

	if (scrX < 0) {
	    scrX = 0;
	}
	if (scrY < 0) {
	    scrY = 0;
	}

	pPriv->scrX = scrX;
	pPriv->scrY = scrY;
	
	/*
	 * Copy virgin pixels from the screen to the screenBits pixmap.
	 */
	{
	    register int h = pPriv->screenBits->height;
	    register unsigned int *pDst = 
		(unsigned int *)pPriv->screenBits->devPrivate;
	    register unsigned int *pSrc =
		(unsigned int *)(((PixmapPtr)pScreen->devPrivate)->devPrivate);
	    register int widthSrc = 
		(int)(((PixmapPtr)pScreen->devPrivate)->devKind) >> 2;

	    pSrc += pPriv->scrY * widthSrc;
	    pSrc = (unsigned int *) /* wicked alignment hack! */
		((unsigned char *)pSrc + (pPriv->scrX >> 3));

	    if (h > 0) do {
		*pDst++ = *pSrc;
		pSrc += widthSrc;
	    } while (--h);
	}
	macIIMonoOptPutCursor (pScreen, pPriv, pptrPriv->x, pptrPriv->y, TRUE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorOptRemoveCursor --
 *	Remove the current cursor from the screen, if it hasn't been removed
 *	already.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the cursor is removed, the state field of the cursor-private
 *	structure is set to CR_OUT. In addition, macIIInputAvail is called.
 *
 *-----------------------------------------------------------------------
 */
void
macIIColorOptRemoveCursor ()
{
    CrPrivPtr	  pPriv;    	/* Private data for this cursor */
    ScreenPtr	  pScreen;  	/* Screen on which the cursor is */
    PtrPrivPtr	  pPtrPriv;	/* XXX: Pointer private data */
    DevicePtr	  pDev;

    pDev = LookupPointerDevice();

    pPtrPriv = (PtrPrivPtr) pDev->devicePrivate;
    pScreen = pPtrPriv->pScreen;
    
    pPriv = (CrPrivPtr)currentCursor->devPriv[pScreen->myNum];

    if (pPriv->state == CR_IN) {
	/*
	 * XXX: Makes use of the devPrivate field of the screen. This *must*
	 * be a pixmap representing the entire screen. How else can I get a
	 * pixmap to draw to?
	 */

	/*
	 * Return the virgin screen bits to the framebuffer, thereby
	 * removing the cursor.
	 */
	{
	    register int h = pPriv->screenBits->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->screenBits->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)(((PixmapPtr)pScreen->devPrivate)->devPrivate);
	    register int widthDst = 
		(int)(((PixmapPtr)pScreen->devPrivate)->devKind) >> 2;

	    pDst += (pPriv->scrY * widthDst) + (pPriv->scrX >> 2);

	    if (h > 0) do {
		*pDst++ = *pSrc++; *pDst++ = *pSrc++; 
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		pDst += widthDst - 8;
	    } while (--h);
	}
	pPriv->state = CR_OUT;
	isItTimeToYield++;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorOptPutCursor --
 *	Place the current cursor in the screen at the given coordinates.
 *	screenBits must already have been filled. If 'direct' is FALSE,
 *	the cursor will be drawn into the temp pixmap after the screenBits
 *	have been copied there. The temp pixmap will then be put onto
 *	the screen. Used for more flicker-free cursor motion...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The state field of the private data is set to CR_IN...
 *
 *-----------------------------------------------------------------------
 */

static int table[16] = {0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
			0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
			0xff000000, 0xff0000ff, 0xff00ff00, 0x0000ffff,
			0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff};

#define PFILL(x) (x | (x << 8) | (x << 16) | (x << 24))

static void
macIIColorOptPutCursor (pScreen, pPriv, hotX, hotY, direct)
    ScreenPtr	  	pScreen;    	/* Screen on which to put it */
    CrPrivPtr	  	pPriv;	    	/* Cursor-private data */
    int	    	  	hotX;
    int	    	  	hotY;
    Bool    	  	direct;	    	/* TRUE if should put the cursor */
					/* directly onto the screen, not */
					/* into pPriv->temp first... */
{
    register PixmapPtr	pPixmap;
    int			x = hotX - currentCursor->xhot;
    int 		y = hotY - currentCursor->yhot;
    

    if (!direct) {
	pPixmap = pPriv->temp;
	
	/*
	 * Duplicate the contents of the screenBits in pPriv->temp.
	 */
	{
	    register int h = pPriv->screenBits->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->screenBits->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)pPixmap->devPrivate;

	    if (h > 0) do { 
		*pDst++ = *pSrc++; *pDst++ = *pSrc++; 
		*pDst++ = *pSrc++; *pDst++ = *pSrc++; 
		*pDst++ = *pSrc++; *pDst++ = *pSrc++; 
		*pDst++ = *pSrc++; *pDst++ = *pSrc++; 
	    } while (--h);
	}
	
	/*
	 * Push foreground and background pixels through the source
	 * and inverse source bitmaps respectively.
	 */
	{
	    register int h = pPriv->source->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->source->devPrivate;
	    register unsigned int *pinvSrc = 
		(unsigned int *)pPriv->invSource->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)pPixmap->devPrivate;
	    register unsigned int fgFill = PFILL(pPriv->fg);
	    register unsigned int bgFill = PFILL(pPriv->bg);
	    int widthDst = 8; /* width in longwords */

	    y = y - pPriv->scrY;
	    if (y < 0) {
		h += y;
		pSrc += (-y);
		pinvSrc += (-y);
		y = 0;
	    }
	    if (y > (pPixmap->height - h)) h = pPixmap->height - y;
	    pDst += y * widthDst;

	    x = x - pPriv->scrX;
	    if (h > 0) do {
		unsigned int fgQuartets = 
		    (x < 0 ? *pSrc++ << (-x) : *pSrc++ >> x);
		unsigned int bgQuartets = 
		    (x < 0 ? *pinvSrc++ << (-x) : *pinvSrc++ >> x);
		register unsigned int fgMask, bgMask;
		register int shift = 28;

		do {
		    bgMask = table[(bgQuartets >> shift) & 0x0f];
		    fgMask = table[(fgQuartets >> shift) & 0x0f];
		    *pDst = (*pDst & ~bgMask & ~fgMask) | 
			    (bgFill & bgMask) | (fgFill & fgMask);
		    pDst++;
		    shift -= 4;
		} while (shift >= 0);
	    } while (--h);
	}

	/*
	 * Now put the whole buffer onto the screen
	 */
	{
	    register int h = pPixmap->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPixmap->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)(((PixmapPtr)pScreen->devPrivate)->devPrivate);
	    register int widthDst = 
		(int)(((PixmapPtr)pScreen->devPrivate)->devKind) >> 2;

	    pDst += (pPriv->scrY * widthDst) + (pPriv->scrX >> 2);

	    if (h > 0) do {
		*pDst++ = *pSrc++; *pDst++ = *pSrc++; 
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		pDst += widthDst - 8;
	    } while (--h);
	}
	pPriv->state = CR_IN;
    } else {
	pPixmap = (PixmapPtr) pScreen->devPrivate;

	/*
	 * Push foreground and background pixels through the source
	 * and inverse source bitmaps respectively.
	 */
	{
	    register int h = pPriv->source->height;
	    register unsigned int *pSrc = 
		(unsigned int *)pPriv->source->devPrivate;
	    register unsigned int *pinvSrc = 
		(unsigned int *)pPriv->invSource->devPrivate;
	    register unsigned int *pDst =
		(unsigned int *)pPixmap->devPrivate;
	    register unsigned int fgFill = PFILL(pPriv->fg);
	    register unsigned int bgFill = PFILL(pPriv->bg);
	    int widthDst = (int)(pPixmap->devKind) >> 2;

	    if (y < 0) {
		h += y;
		pSrc += (-y);
		pinvSrc += (-y);
		y = 0;
	    }
	    if (y > (pPixmap->height - h)) h = pPixmap->height - y;
	    pDst += y * widthDst + (pPriv->scrX >> 2);

	    x = x - pPriv->scrX;
	    if (h > 0) do {
		unsigned int fgQuartets = 
		(x < 0 ? *pSrc++ << (-x) : *pSrc++ >> x);
		unsigned int bgQuartets = 
		(x < 0 ? *pinvSrc++ << (-x) : *pinvSrc++ >> x);
		register unsigned int fgMask, bgMask;
		register int shift = 28;

		do {
		    bgMask = table[(bgQuartets >> shift) & 0x0f];
		    fgMask = table[(fgQuartets >> shift) & 0x0f];
		    *pDst = (*pDst & ~bgMask & ~fgMask) | 
			    (bgFill & bgMask) | (fgFill & fgMask);
		    pDst++;
		    shift -= 4;
		} while (shift >= 0);
		pDst += widthDst - 8;
	    } while (--h);
	}
	pPriv->state = CR_IN;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorOptRestoreCursor --
 *	Redraw the cursor if it was removed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be replaced and the 'state' field changed.
 *
 *-----------------------------------------------------------------------
 */
void
macIIColorOptRestoreCursor()
{
    PtrPrivPtr	  	pptrPriv;
    CrPrivPtr	  	pPriv;
    ScreenPtr	  	pScreen;
    DevicePtr		pDev;
    register PixmapPtr	screenBits;
    int	    	  	scrX, scrY;

    pDev = LookupPointerDevice();

    pptrPriv = (PtrPrivPtr) pDev->devicePrivate;
    pScreen = pptrPriv->pScreen;
    pPriv = (CrPrivPtr) currentCursor->devPriv[pScreen->myNum];
    screenBits = pPriv->screenBits;

    if (pPriv->state == CR_OUT) {
	scrX = ((pptrPriv->x - currentCursor->xhot) >> 4) << 4;
	scrX = min(scrX, ((PixmapPtr)pScreen->devPrivate)->width - 32);
        scrY = pptrPriv->y - currentCursor->yhot - screenBits->height / 4;
	scrY = min(scrY, ((PixmapPtr)pScreen->devPrivate)->height - 
			  screenBits->height);

	if (scrX < 0) {
	    scrX = 0;
	}
	if (scrY < 0) {
	    scrY = 0;
	}

	pPriv->scrX = scrX;
	pPriv->scrY = scrY;
	
	/*
	 * Copy virgin pixels from the screen to the screenBits pixmap.
	 */
	{
	    register int h = pPriv->screenBits->height;
	    register unsigned int *pDst = 
		(unsigned int *)pPriv->screenBits->devPrivate;
	    register unsigned int *pSrc =
		(unsigned int *)(((PixmapPtr)pScreen->devPrivate)->devPrivate);
	    register int widthSrc = 
		(int)(((PixmapPtr)pScreen->devPrivate)->devKind) >> 2;

	    pSrc += (pPriv->scrY * widthSrc) + (pPriv->scrX >> 2);

	    if (h > 0) do {
		*pDst++ = *pSrc++; *pDst++ = *pSrc++; 
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		*pDst++ = *pSrc++; *pDst++ = *pSrc++;
		pSrc += widthSrc - 8;
	    } while (--h);
	}
	macIIColorOptPutCursor (pScreen, pPriv, pptrPriv->x, pptrPriv->y, TRUE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIInitCursor --
 *	Initialize the cursor module...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	currentCursor is set to NullCursor.
 *
 *-----------------------------------------------------------------------
 */
void
macIIInitCursor ()
{

    currentCursor = NullCursor;
    wPrivClass = CreateNewResourceClass();
}

/*-
 *-----------------------------------------------------------------------
 * Stencil --
 *	Return the data for a bitmap made by squishing the source bitmap
 *	data through the mask bitmap data. If invert is TRUE, the source
 *	data are inverted before being squished through.
 *
 * Results:
 *	An array of data suitable for passing to PutImage
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static u_char *
macIIStencil (source, mask, w, h, invert)
    register u_char *source;  	/* Source data */
    register u_char *mask;    	/* Mask data */
    int	    	  w;	    	/* Width of both */
    int	    	  h;	    	/* Height of both */
    Bool    	  invert;   	/* Invert source before squishing */
{
    u_char    	  *result;
    register u_char *r;
    register int  nbytes;

    nbytes = h * PixmapBytePad (w, 1);
    result = (u_char *)Xalloc(nbytes);

    for (r = result; nbytes--; source++, r++) {
	*r = (invert ? ~ *source : *source) & (mask ? *mask++ : ~0);
    }

    return (result);
}

/*-
 *-----------------------------------------------------------------------
 * macIIGetPixel --
 *	Given an rgb value, find an equivalent pixel value for it.
 *	XXX - this is garbage - re-implement
 *
 * Results:
 *	The pixel value.
 *
 * Side Effects:
 *	A colormap entry might be allocated...
 *
 *-----------------------------------------------------------------------
 */
static Pixel
macIIGetPixel (pScreen, r, g, b)
    ScreenPtr	  pScreen;  /* Screen to allocate from */
    unsigned short  r;	    /* Red value to use */
    unsigned short  g;	    /* Green value to use */
    unsigned short  b;	    /* Blue value to use */
{
    ColormapPtr cmap = (ColormapPtr) LookupID(pScreen->defColormap, RT_COLORMAP, RC_CORE);
    Pixel       pix = 0;

    if (!cmap || AllocColor(cmap, &r, &g, &b, &pix, 0)) {
       ErrorF("macIIGetPixel: Can't alloc pixel (%d,%d,%d) in map 0x%x\n",
              r, g, b, pScreen->defColormap);
       return (pScreen->blackPixel);
     }
    return (pix);
}
/*-
 *-----------------------------------------------------------------------
 * macIIRealizeCursor --
 *	Realize a cursor for a specific screen. Eventually it will have
 *	to deal with the allocation of a special pixel from the system
 *	colormap, but for now it's fairly simple. Just have to create
 *	pixmaps.
 *
 * Results:
 *
 * Side Effects:
 *	A CrPrivRec is allocated and filled and stuffed into the Cursor
 *	structure given us.
 *
 *-----------------------------------------------------------------------
 */
Bool
macIIRealizeCursor (pScreen, pCursor)
    ScreenPtr	  pScreen;  	/* Screen for which the cursor should be */
				/* realized */
    CursorPtr	  pCursor;  	/* Cursor to realize */
{
    register CrPrivPtr pPriv = (CrPrivPtr)NULL;
    int         bufWidth;
    GC         *pGC = (GC *)NULL;     /* GC for initializing the source and */
    GC                *tGC1 = (GC *)NULL, *tGC2 = (GC *)NULL;
    /* invSource pixmaps... */
    u_char     *stencil;
    BITS32      status;
			  
    if (!(pGC = GetScratchGC(1, pScreen)))
      goto cleanup1;
    if (!(pPriv = (CrPrivPtr) Xalloc(sizeof(CrPrivRec))))
      goto cleanup2;

    pPriv->fg = macIIGetPixel(pScreen, pCursor->foreRed,
			    pCursor->foreGreen,
			    pCursor->foreBlue);
    pPriv->bg = macIIGetPixel(pScreen, pCursor->backRed,
			    pCursor->backGreen,
			    pCursor->backBlue);

    tGC1 = macIICreatePrivGC((DrawablePtr)pScreen->devPrivate,
      GCForeground, &pPriv->fg, &status);
    if (!tGC1)
      goto cleanup3;
    tGC2 = macIICreatePrivGC((DrawablePtr)pScreen->devPrivate,
      GCForeground, &pPriv->bg, &status);
    if (!tGC2) {
      FreeScratchGC(tGC1);
cleanup3:
      Xfree(pPriv);
      /* XXX - free up the pixels,  too */
cleanup2:
      FreeScratchGC(pGC);
cleanup1:
      return FALSE;
    }

    pPriv->srcGC = tGC1;
    pPriv->invSrcGC = tGC2;

    pPriv->state = CR_OUT;
    pCursor->devPriv[pScreen->myNum] = (pointer) pPriv;

    bufWidth = (pCursor->width <= 16 ? 32 : 2 * pCursor->width);

    /*
     * Create the two pixmaps for the off-screen manipulation of the
     * cursor image. The screenBits pixmap is used to hold the contents of
     * the screen before the cursor is put down and the temp pixmap exists
     * to avoid having to create a pixmap each time the cursor is rop'ed
     * in. Both are made the same depth as the screen, for obvious
     * reasons. 
     */
    pPriv->screenBits =
	(PixmapPtr) (*pScreen->CreatePixmap) (pScreen, bufWidth,
					      2 * pCursor->height,
					      pScreen->rootDepth);
    pPriv->temp =
	(PixmapPtr) (*pScreen->CreatePixmap) (pScreen, bufWidth,
					      2 * pCursor->height,
					      pScreen->rootDepth);

    /*
     * The source and invSource bitmaps are a bit trickier. The idea is to
     * use the source in a PushPixels operation on the foreground pixel of
     * the cursor and the invSource in a PushPixels on the background
     * pixel of the cursor. The tricky thing is both bitmaps must be
     * created from the source bits after being masked by the mask bits.
     * This must, sadly, be done by hand b/c the ddx interface isn't quite
     * rich enough to push a tile through a bitmap. 
     */
    pPriv->source =
	(PixmapPtr) (*pScreen->CreatePixmap) (pScreen, pCursor->width,
					      pCursor->height, 1);

    ValidateGC(pPriv->source, pGC);
    stencil = macIIStencil(pCursor->source, pCursor->mask,
			 pCursor->width, pCursor->height,
			 FALSE);
    (*pGC->PutImage) (pPriv->source, pGC, 1,
		      0, 0,
		      pCursor->width, pCursor->height,
		      0,
		      XYPixmap,
		      stencil);
    Xfree(stencil);

    ValidateGC((DrawablePtr) pScreen->devPrivate, pPriv->srcGC);

    pPriv->invSource =
	(PixmapPtr) (*pScreen->CreatePixmap) (pScreen, pCursor->width,
					      pCursor->height, 1);
    ValidateGC(pPriv->invSource, pGC);
    stencil = macIIStencil(pCursor->source, pCursor->mask,
			 pCursor->width, pCursor->height,
			 TRUE);
    (*pGC->PutImage) (pPriv->invSource, pGC, 1,
		      0, 0,
		      pCursor->width, pCursor->height,
		      0,
		      XYPixmap,
		      stencil);
    Xfree(stencil);

    ValidateGC((DrawablePtr) pScreen->devPrivate, pPriv->invSrcGC);

    FreeScratchGC( pGC );
    return TRUE;
}

/*-
 *-----------------------------------------------------------------------
 * macIIUnrealizeCursor --
 *	Free up the extra state created by macIIRealizeCursor.
 *
 * Results:
 *	TRUE.
 *
 * Side Effects:
 *	All the pixmaps etc. created by macIIRealizeCursor are destroyed
 *	and the private structure is freed.
 *
 *-----------------------------------------------------------------------
 */
Bool
macIIUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	  pScreen;  	/* Screen for which to unrealize the cursor */
    CursorPtr	  pCursor;  	/* Cursor to unrealize */
{
    CrPrivPtr   pPriv;

    if (currentCursor == pCursor) {
	macIIRemoveCursor();
	currentCursor = NullCursor;
    }
    pPriv = (CrPrivPtr) pCursor->devPriv[pScreen->myNum];

    (*pScreen->DestroyPixmap) (pPriv->source);
    (*pScreen->DestroyPixmap) (pPriv->invSource);
    (*pScreen->DestroyPixmap) (pPriv->screenBits);
    (*pScreen->DestroyPixmap) (pPriv->temp);
    /*
     * XXX: Deallocate pixels 
     */
    FreeScratchGC(pPriv->srcGC);
    FreeScratchGC(pPriv->invSrcGC);

    Xfree(pPriv);

    return TRUE;
}

/*-
 *-----------------------------------------------------------------------
 * macIISetCursorPosition --
 *	Alter the position of the current cursor. The x and y coordinates
 *	are assumed to be legal for the given screen.
 *
 * Results:
 *	TRUE.
 *
 * Side Effects:
 *	The pScreen, x, and y fields of the current pointer's private data
 *	are altered to reflect the new values. I.e. moving the cursor to
 *	a different screen moves the pointer there as well. Helpful...
 *
 *-----------------------------------------------------------------------
 */
Bool
macIISetCursorPosition (pScreen, hotX, hotY, generateEvent)
    ScreenPtr	  pScreen;  	/* New screen for the cursor */
    unsigned int  hotX;	    	/* New absolute X coordinate for the cursor */
    unsigned int  hotY;	    	/* New absolute Y coordinate for the cursor */
    Bool	  generateEvent;/* whether we generate a motion event */
{
    DevicePtr	  pDev;
    PtrPrivPtr	  pptrPriv;
    xEvent	  motion;
    extern int	macIISigIO;

    if (currentCursor)
	macIIDisplayCursor (pScreen, currentCursor);
    
    pDev = LookupPointerDevice();

    pptrPriv = (PtrPrivPtr)pDev->devicePrivate;

    pptrPriv->pScreen = pScreen;
    pptrPriv->x = hotX;
    pptrPriv->y = hotY;

    if (generateEvent)
    {
	if (macIISigIO)
	    ProcessInputEvents();
	motion.u.keyButtonPointer.rootX = hotX;
	motion.u.keyButtonPointer.rootY = hotY;
	motion.u.keyButtonPointer.time = lastEventTime;
	motion.u.u.type = MotionNotify;
	(*pDev->processInputProc) (&motion, pDev);
    }
    return TRUE;
}

/*-
 *-----------------------------------------------------------------------
 * macIICursorLimits --
 *	Return a box within which the given cursor may move on the given
 *	screen. We assume that the HotBox is actually on the given screen,
 *	since dix knows that size.
 *
 * Results:
 *	A box for the hot spot corner of the cursor.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
void
macIICursorLimits (pScreen, pCursor, pHotBox, pResultBox)
    ScreenPtr	  pScreen;  	/* Screen on which limits are desired */
    CursorPtr	  pCursor;  	/* Cursor whose limits are desired */
    BoxPtr  	  pHotBox;  	/* Limits for pCursor's hot point */
    BoxPtr  	  pResultBox;	/* RETURN: limits for hot spot */
{
    *pResultBox = *pHotBox;
}

/*-
 *-----------------------------------------------------------------------
 * macIIDisplayCursor --
 *	Set the current cursor.
 *
 * Results:
 *	TRUE.
 *
 * Side Effects:
 *	The value of currentCursor is altered. Note that the cursor is
 *	*not* placed in the frame buffer until RestoreCursor is called.
 *	Instead, the cursor is marked as out, which will cause it to
 *	be replaced.
 *
 *-----------------------------------------------------------------------
 */
/* ARGSUSED */
Bool
macIIDisplayCursor (pScreen, pCursor)
    ScreenPtr	  pScreen;  	/* Screen on which to display cursor */
    CursorPtr	  pCursor;  	/* Cursor to display */
{
    DevicePtr		  pDev;

    if (currentCursor) {
	macIIRemoveCursor();
    }

    currentCursor = pCursor;
    ((CrPrivPtr)pCursor->devPriv[pScreen->myNum])->state = CR_OUT;
    isItTimeToYield++;

    pDev = LookupPointerDevice();

    if (pScreen != ((PtrPrivPtr)pDev->devicePrivate)->pScreen) {
	/*XXX*/
	((PtrPrivPtr)pDev->devicePrivate)->pScreen = pScreen;
    }

    return TRUE;
}

/*-
 *-----------------------------------------------------------------------
 * macIIRecolorCursor --
 *	Change the color of a cursor.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the screen is a color one
 *
 *-----------------------------------------------------------------------
 */
void
macIIRecolorCursor (pScreen, pCursor, displayed)
    ScreenPtr	  pScreen;  	/* Screen for which the cursor is to be */
				/* recolored */
    CursorPtr	  pCursor;  	/* Cursor to recolor */
    Bool    	  displayed;	/* True if pCursor being displayed now */
{
    CrPrivPtr	  pPriv;
    /*
     * XXX: will have to alter the colormap entries of the foreground and
     * background pixels. For now, just change the pixels...
     */

    if (displayed) {
	macIIRemoveCursor();
    }

    pPriv = (CrPrivPtr)pCursor->devPriv[pScreen->myNum];
    pPriv->fg = macIIGetPixel(pScreen,
			    pCursor->foreRed,
			    pCursor->foreGreen,
			    pCursor->foreBlue);
    ChangeGC (pPriv->srcGC, GCForeground, &pPriv->fg);
    ValidateGC (pScreen->devPrivate, pPriv->srcGC);
    
    pPriv->bg = macIIGetPixel (pScreen,
			     pCursor->backRed,
			     pCursor->backGreen,
			     pCursor->backBlue);
    ChangeGC (pPriv->invSrcGC, GCForeground, &pPriv->bg);
    ValidateGC (pScreen->devPrivate, pPriv->invSrcGC);
}

/*-
 *-----------------------------------------------------------------------
 * macIIPointerNonInterestBox --
 *	Set up things to ignore uninteresting mouse events. Sorry.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
void
macIIPointerNonInterestBox (pScreen, pBox)
    ScreenPtr	  pScreen;  /* Screen for cursor */
    BoxPtr  	  pBox;	    /* Box outside of which motions are boring */
{
}

/*-
 *-----------------------------------------------------------------------
 * macIIConstrainCursor --
 *	Make it so the current pointer doesn't let the cursor's
 *      hot-spot wander out of the specified box.
 *	
 * Results:
 *	None.
 *
 * Side Effects:
 *	None, for now.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
void
macIIConstrainCursor (pScreen, pBox)
    ScreenPtr	  pScreen;  	/* Screen to which it should be constrained */
    BoxPtr  	  pBox;	    	/* Box in which... */
{
    currentLimits = *pBox;
}

/*-
 *-----------------------------------------------------------------------
 * macIIRemoveCursor --
 *	Remove the current cursor from the screen, if it hasn't been removed
 *	already.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the cursor is removed, the state field of the cursor-private
 *	structure is set to CR_OUT. In addition, macIIInputAvail is called.
 *
 *-----------------------------------------------------------------------
 */
void
macIIRemoveCursor ()
{
    CrPrivPtr	  pPriv;    	/* Private data for this cursor */
    ScreenPtr	  pScreen;  	/* Screen on which the cursor is */
    PtrPrivPtr	  pPtrPriv;	/* XXX: Pointer private data */
    DevicePtr	  pDev;
    GC	    	  *pGC;

    pDev = LookupPointerDevice();

    pPtrPriv = (PtrPrivPtr) pDev->devicePrivate;
    pScreen = pPtrPriv->pScreen;
    
    switch (pScreen->rootDepth) {
	case 1:
	    if (currentCursor->width <= 16) {
		macIIMonoOptRemoveCursor();
		return;
	    }
	    break;
	case 8:
	    if (currentCursor->width <= 16) {
		macIIColorOptRemoveCursor();
		return;
	    }
	    break;
	default:
	    ErrorF("Bad depth in macIIRemoveCursor!\n");
	    break;
    }

    pPriv = (CrPrivPtr)currentCursor->devPriv[pScreen->myNum];

    if (pPriv->state == CR_IN) {
	/*
	 * XXX: Makes use of the devPrivate field of the screen. This *must*
	 * be a pixmap representing the entire screen. How else can I get a
	 * pixmap to draw to?
	 */

	pGC = macIIFbs[pScreen->myNum].pGC;

	pPriv->state = CR_XING;
	pGC->stateChanges |= (GCForeground|GCBackground);	
		/* Need to set some bits */
	ValidateGC((DrawablePtr)pScreen->devPrivate, pGC);
	(* pGC->CopyArea) (pPriv->screenBits,
			   (PixmapPtr)pScreen->devPrivate,
			   pGC,
			   0, 0,
			   pPriv->screenBits->width,
			   pPriv->screenBits->height,
			   pPriv->scrX, pPriv->scrY);
	pPriv->state = CR_OUT;
	isItTimeToYield++;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIPutCursor --
 *	Place the current cursor in the screen at the given coordinates.
 *	screenBits must already have been filled. If 'direct' is FALSE,
 *	the cursor will be drawn into the temp pixmap after the screenBits
 *	have been copied there. The temp pixmap will then be put onto
 *	the screen. Used for more flicker-free cursor motion...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The state field of the private data is set to CR_IN...
 *
 *-----------------------------------------------------------------------
 */

static void
macIIPutCursor (pScreen, pPriv, hotX, hotY, direct)
    ScreenPtr	  	pScreen;    	/* Screen on which to put it */
    CrPrivPtr	  	pPriv;	    	/* Cursor-private data */
    int	    	  	hotX;
    int	    	  	hotY;
    Bool    	  	direct;	    	/* TRUE if should put the cursor */
					/* directly onto the screen, not */
					/* into pPriv->temp first... */
{
    register PixmapPtr	pPixmap;
    register GC	  	*pGC;
    int 	  	realW,
			realH;
    int			x = hotX - currentCursor->xhot;
    int 		y = hotY - currentCursor->yhot;
    
    switch (pScreen->rootDepth) {
	case 1:
	    if (currentCursor->width <= 16) {
		macIIMonoOptPutCursor(pScreen, pPriv, hotX, hotY, direct);
		return;
	    }
	    break;
	case 8:
	    if (currentCursor->width <= 16) {
		macIIColorOptPutCursor(pScreen, pPriv, hotX, hotY, direct);
		return;
	    }
	    break;
	default:
	    ErrorF("Bad depth in macIIPutCursor!\n");
	    break;
    }
    pGC = macIIFbs[pScreen->myNum].pGC;

    if (!direct) {
	pPixmap = pPriv->temp;
	
	/*
	 * Duplicate the contents of the screen
	 */
	pGC->stateChanges |= (GCForeground|GCBackground);
		/* Need to set some bits */
	ValidateGC(pPixmap, pGC);
	(* pGC->CopyArea) (pPriv->screenBits, pPixmap, pGC,
			   0, 0, pPixmap->width, pPixmap->height,
			   0, 0);
	
	/*
	 * First the foreground pixels...
	 * Warning: The srcGC is validated with the screen, so the clip list
	 * here will be wrong, but it shouldn't matter since we never go
	 * outside the pixmap...
	 */
	(* pPriv->srcGC->PushPixels) (pPriv->srcGC, pPriv->source, pPixmap,
				      pPriv->source->width,
				      pPriv->source->height,
				      x - pPriv->scrX, y - pPriv->scrY);

	/*
	 * Then the background pixels
	 */
	(* pPriv->invSrcGC->PushPixels) (pPriv->invSrcGC, pPriv->invSource,
					 pPixmap, pPriv->invSource->width,
					 pPriv->invSource->height,
					 x - pPriv->scrX, y - pPriv->scrY);

	/*
	 * Now put the whole buffer onto the screen
	 */
	pPriv->state = CR_XING;
	realW = min(pPixmap->width, pScreen->width - pPriv->scrX);
	realH = min(pPixmap->height,pScreen->height - pPriv->scrY);

	pGC->stateChanges |= (GCForeground|GCBackground);
		/* Need to set some bits */
	ValidateGC((DrawablePtr)pScreen->devPrivate, pGC);
	(* pGC->CopyArea) (pPixmap, (PixmapPtr)pScreen->devPrivate, pGC,
			   0, 0, realW, realH, pPriv->scrX, pPriv->scrY);
	
	pPriv->state = CR_IN;
    } else {
	pPixmap = (PixmapPtr) pScreen->devPrivate;

	/*
	 * PushPixels can't handle negative x and y.  Therefore
	 * we'll clip here on our own.
	 */
	realW = min(pPriv->source->width,pPixmap->width - x);
	realH = min(pPriv->source->height,pPixmap->height - y);

	pPriv->state = CR_XING;
	/*
	 * First the foreground pixels...
	 */
	(* pPriv->srcGC->PushPixels) (pPriv->srcGC, pPriv->source, pPixmap,
				      realW, realH, x, y);
	
	/*
	 * Then the background pixels
	 */
	(* pPriv->invSrcGC->PushPixels) (pPriv->invSrcGC, pPriv->invSource,
					 pPixmap, realW, realH, x, y);

	pPriv->state = CR_IN;
    }
}


/*-
 *-----------------------------------------------------------------------
 * macIICursorLoc --
 *	If the current cursor is both on and in the given screen,
 *	fill in the given BoxRec with the extent of the cursor and return
 *	TRUE. If the cursor is either on a different screen or not
 *	currently in the frame buffer, return FALSE.
 *
 * Results:
 *	TRUE or FALSE, as above.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
Bool
macIICursorLoc (pScreen, pBox)
    ScreenPtr	  pScreen;  	/* Affected screen */
    BoxRec	  *pBox;	/* Box in which to place the limits */
{
    PtrPrivPtr	  pptrPriv;
    CrPrivPtr	  pPriv;
    DevicePtr	  pDev;

    if (currentCursor == NullCursor) {
	/*
	 * Firewall: Might be called when initially putting down the cursor
	 */
	return FALSE;
    }

    pDev = LookupPointerDevice();
    pptrPriv = (PtrPrivPtr) pDev->devicePrivate;
    pPriv = (CrPrivPtr) currentCursor->devPriv[pScreen->myNum];

    if (pptrPriv->pScreen == pScreen && pPriv->state == CR_IN) {
	/*
	 * If the cursor is actually on the same screen, stuff the cursor's
	 * limits in the given BoxRec. Perhaps this should be done when
	 * the cursor is moved? Probably not important...
	 */

	pBox->x1 = pPriv->scrX;
	pBox->y1 = pPriv->scrY;
	pBox->x2 = pPriv->scrX + pPriv->screenBits->width;
	pBox->y2 = pPriv->scrY + pPriv->screenBits->height;
	return TRUE;
    } else {
	return FALSE;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIRestoreCursor --
 *	Redraw the cursor if it was removed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be replaced and the 'state' field changed.
 *
 *-----------------------------------------------------------------------
 */
void
macIIRestoreCursor()
{
    PtrPrivPtr	  	pptrPriv;
    CrPrivPtr	  	pPriv;
    ScreenPtr	  	pScreen;
    DevicePtr		pDev;
    GC	    	  	*pGC;
    register PixmapPtr	screenBits;
    int	    	  	scrX, scrY;

    pDev = LookupPointerDevice();

    pptrPriv = (PtrPrivPtr) pDev->devicePrivate;
    pScreen = pptrPriv->pScreen;

    switch (pScreen->rootDepth) {
	case 1:
	    if (currentCursor->width <= 16) {
		macIIMonoOptRestoreCursor();
		return;
	    }
	    break;
	case 8:
	    if (currentCursor->width <= 16) {
		macIIColorOptRestoreCursor();
		return;
	    }
	    break;
	default:
	    ErrorF("Bad depth in macIIRestoreCursor!\n");
	    break;
    }

    pPriv = (CrPrivPtr) currentCursor->devPriv[pScreen->myNum];
    screenBits = pPriv->screenBits;

    if (pPriv->state == CR_OUT) {
	/*
	 * Since the buffer pixmap is four times as large as the cursor and
	 * we would always like to center the thing so as to allow the same
	 * leeway for movement, in macIIMoveCursor, on each side, we place the
	 * top-left corner of the cursor at the intersection of the first
	 * quarter lines by shifting the position of the buffer pixmap
	 */
	scrX = pptrPriv->x - currentCursor->xhot - screenBits->width / 4;
	scrY = pptrPriv->y - currentCursor->yhot - screenBits->height / 4;

	/*
	 * In general we're trying to store some of the bits surrounding the
	 * cursor.  In general, we try to center the cursor in the area we're
         * saving; that is what the previous two lines are doing.  But if we're
         * at the top left already, we won't try to center the cursor; we'll just
         * save from [0,0].
	 */
	if (scrX < 0) {
	    scrX = 0;
	}
	if (scrY < 0) {
	    scrY = 0;
	}

	pPriv->scrX = scrX;
	pPriv->scrY = scrY;
	
	pGC = macIIFbs[pScreen->myNum].pGC;
	pGC->stateChanges |= (GCForeground|GCBackground);	
		/* Need to set some bits */
	ValidateGC(screenBits, pGC);
	(* pGC->CopyArea) ((DrawablePtr)pScreen->devPrivate,
			   (DrawablePtr)screenBits,
			   pGC,
			   scrX, scrY,
			   screenBits->width, screenBits->height,
			   0, 0);
	macIIPutCursor (pScreen, pPriv, pptrPriv->x, pptrPriv->y, TRUE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIMoveCursor --
 *	Shift a the current cursor by a given amount. If the change keeps
 *	the cursor within its screenBits pixmap, the whole thing is
 *	simply drawn over the old position. Otherwise, the cursor is
 *	removed and must be redrawn before we sleep. The pointer's
 *	coordinates need not have been updated before this is called.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed...
 *
 *-----------------------------------------------------------------------
 */
void
macIIMoveCursor (pScreen, hotX, hotY)
    ScreenPtr	  pScreen;  	/* Screen cursor's currently on */
    int	    	  hotX;	  	/* Cursor's new X coordinate */
    int	    	  hotY;	  	/* Cursor's new Y coordinate */
{
    CrPrivPtr	  pPriv;

    pPriv = (CrPrivPtr) currentCursor->devPriv[pScreen->myNum];

    if (pPriv->state == CR_OUT) {
	return;
    }

    if ((hotX - currentCursor->xhot >= pPriv->scrX) &&
	((hotX - currentCursor->xhot + currentCursor->width) <
	 (pPriv->scrX + pPriv->screenBits->width))&&
	(hotY - currentCursor->yhot >= pPriv->scrY) &&
	((hotY - currentCursor->yhot + currentCursor->height) <
	 (pPriv->scrY + pPriv->screenBits->height))) {
	     /*
	      * If the entire cursor at its new position remains inside the
	      * box buffered in the screenBits pixmap, then its ok to just
	      * place the cursor inside the box and draw the entire box
	      * onto the screen. The hope is that this redrawing, rather than
	      * removing the cursor and redrawing it, will cause it to flicker
	      * less than it did in V10...
	      */
	     macIIPutCursor (pScreen, pPriv, hotX, hotY, FALSE);
    } else {
	/*
	 * The cursor is no longer within the screenBits pixmap, so we just
	 * remove it. dix will RestoreCursor() it back onto the screen.
	 */
	macIIRemoveCursor();
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIConstrainXY --
 *	Given an X and Y coordinate, alter them to fit within the current
 *	cursor constraints.  Used by mouse processing code to adjust 
 * 	position reported by hardware.
 *	
 *
 * Results:
 *	The new constrained coordinates. Returns FALSE if the motion
 *	event should be discarded...
 *
 * Side Effects:
 *	guess what?
 *
 *-----------------------------------------------------------------------
 */
Bool
macIIConstrainXY (px, py)
    short	  *px;
    short     	  *py;	
{
    *px = max(currentLimits.x1,min(currentLimits.x2,*px));
    *py = max(currentLimits.y1,min(currentLimits.y2,*py));
    return TRUE;
}

/*-
 * macIIPaintWindowBackground --
 *	Paint the window's background while preserving the cursor
 */
#define macIIPaintWindowBorder macIIPaintWindowBackground
void
macIIPaintWindowBackground (pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    BoxRec	cursorBox;
    WinPrivPtr	pPriv;
    ScreenPtr	pScreen;

    pScreen = pWin->drawable.pScreen;

    if (macIICursorLoc (pScreen, &cursorBox)) {
	/*
	 * If the cursor is on the same screen as the window, check the
	 * region to paint for the cursor and remove it as necessary
	 */
	if ((* pScreen->RectIn) (pRegion, &cursorBox) != rgnOUT) {
	    macIIRemoveCursor();
	}
    }

    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    if (what == PW_BACKGROUND) {
	(* pPriv->PaintWindowBackground) (pWin, pRegion, what);
    } else {
	(* pPriv->PaintWindowBorder) (pWin, pRegion, what);
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIICopyWindow --
 *	Protect the cursor from window copies..
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed.
 *
 *-----------------------------------------------------------------------
 */
void
macIICopyWindow (pWin, ptOldOrg, prgnSrc)
    WindowPtr	  pWin;
    DDXPointRec	  ptOldOrg;
    RegionPtr	  prgnSrc;
{
    BoxRec	cursorBox;
    WinPrivPtr	pPriv;
    ScreenPtr	pScreen;

    pScreen = pWin->drawable.pScreen;

    if (macIICursorLoc (pScreen, &cursorBox)) {
	/*
	 * If the cursor is on the same screen, compare the box for the
	 * cursor against the original window clip region (prgnSrc) and
	 * the current window clip region (pWin->borderClip) and if it
	 * overlaps either one, remove the cursor. (Should it really be
	 * borderClip?)
	 */
	switch ((* pScreen->RectIn) (prgnSrc, &cursorBox)) {
	    case rgnOUT:
		if ((* pScreen->RectIn) (pWin->borderClip, &cursorBox) ==
		    rgnOUT) {
			break;
		}
	    case rgnIN:
	    case rgnPART:
		macIIRemoveCursor();
	}
    }

    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    (* pPriv->CopyWindow) (pWin, ptOldOrg, prgnSrc);
}

/*-
 *-----------------------------------------------------------------------
 * macIISaveAreas --
 *	Keep the cursor from getting in the way of any SaveAreas operation
 *	by backing-store.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed.
 *
 *-----------------------------------------------------------------------
 */
void
macIISaveAreas(pWin)
    WindowPtr	  pWin;
{
    BoxRec  	  cursorBox;
    WinPrivPtr	  pPriv;
    ScreenPtr	  pScreen;

    pScreen = pWin->drawable.pScreen;

    if (macIICursorLoc(pScreen, &cursorBox)) {
	/*
	 * If the areas are obscured because the window moved, we need to
	 * translate the box to the correct relationship with the region,
	 * which is at the new window coordinates.
	 */
	int dx, dy;

	dx = pWin->absCorner.x - pWin->backStorage->oldAbsCorner.x;
	dy = pWin->absCorner.y - pWin->backStorage->oldAbsCorner.y;

	if (dx || dy) {
	    cursorBox.x1 += dx;
	    cursorBox.y1 += dy;
	    cursorBox.x2 += dx;
	    cursorBox.y2 += dy;
	}
	if ((* pScreen->RectIn) (pWin->backStorage->obscured, &cursorBox) != rgnOUT) {
	    macIIRemoveCursor();
	}
    }
    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    (* pPriv->SaveAreas) (pWin);
}

/*-
 *-----------------------------------------------------------------------
 * macIIRestoreAreas --
 *	Keep the cursor from getting in the way of any RestoreAreas operation
 *	by backing-store.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed.
 *
 *-----------------------------------------------------------------------
 */
RegionPtr
macIIRestoreAreas(pWin)
    WindowPtr	  pWin;
{
    BoxRec  	  cursorBox;
    WinPrivPtr	  pPriv;
    ScreenPtr	  pScreen;

    pScreen = pWin->drawable.pScreen;

    if (macIICursorLoc(pScreen, &cursorBox)) {
	/*
	 * The exposed region is now window-relative, so we have to make the
	 * cursor box window-relative too.
	 */
	cursorBox.x1 -= pWin->absCorner.x;
	cursorBox.x2 -= pWin->absCorner.x;
	cursorBox.y1 -= pWin->absCorner.y;
	cursorBox.y2 -= pWin->absCorner.y;
	if ((* pScreen->RectIn) (pWin->exposed, &cursorBox) != rgnOUT) {
	    macIIRemoveCursor();
	}
    }
    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    return (* pPriv->RestoreAreas) (pWin);
}

/*-
 *-----------------------------------------------------------------------
 * macIIDrawGuarantee --
 *	Makes any DrawGuarantee operation see the shadow GC
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed.
 *
 *-----------------------------------------------------------------------
 */

void
macIIDrawGuarantee(pWin, pGC, guarantee)
    WindowPtr	  pWin;
    GCPtr	  pGC;
    int		  guarantee;
{
    WinPrivPtr	  pPriv;

    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    (* pPriv->DrawGuarantee) (pWin, (GCPtr) pGC->devPriv, guarantee);
}

/*-
 * macIICreateWindow --
 *	Allow the output library to do its thing and then make sure
 *	we intercept calls to PaintWindow{Border,Background} and
 *	ClearToBackground
 */
Bool
macIICreateWindow (pWin)
    WindowPtr	pWin;
{
    WinPrivPtr	pPriv;

    (* macIIFbs[((DrawablePtr)pWin)->pScreen->myNum].CreateWindow) (pWin);

    pPriv = (WinPrivPtr) Xalloc (sizeof (WinPrivRec));
    pPriv->PaintWindowBackground =  pWin->PaintWindowBackground;
    pPriv->PaintWindowBorder = 	    pWin->PaintWindowBorder;
    pPriv->CopyWindow =     	    pWin->CopyWindow;

    pWin->PaintWindowBackground =   macIIPaintWindowBackground;
    pWin->PaintWindowBorder = 	    macIIPaintWindowBorder;
    pWin->CopyWindow =	    	    macIICopyWindow;

    if (pWin->backStorage) {
	pPriv->SaveAreas = pWin->backStorage->SaveDoomedAreas;
	pPriv->RestoreAreas = pWin->backStorage->RestoreAreas;
	pPriv->DrawGuarantee = pWin->backStorage->DrawGuarantee;

	pWin->backStorage->SaveDoomedAreas = macIISaveAreas;
	pWin->backStorage->RestoreAreas = macIIRestoreAreas;
	pWin->backStorage->DrawGuarantee = macIIDrawGuarantee;
    }

    AddResource (pWin->wid, RT_WINDOW, (pointer)pPriv, Xfree, 
		 wPrivClass);
}

/*-
 * macIIChangeWindowAttributes --
 *	Catch the changing of the background/border functions
 */
Bool
macIIChangeWindowAttributes (pWin, mask)
    WindowPtr	pWin;
    Mask	mask;
{
    WinPrivPtr	pPriv;

    (* macIIFbs[((DrawablePtr)pWin)->pScreen->myNum].ChangeWindowAttributes)
	(pWin, mask);
    
    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);

    if (pPriv == (WinPrivPtr)0) {
      ErrorF ("macIIChangeWindowAttributes: Can't find priv data for 0x%x\n",
              pWin->wid);
      return (FALSE);

    }

    if ((void (*)())pWin->PaintWindowBackground != (void (*)())macIIPaintWindowBackground){
	    pPriv->PaintWindowBackground = pWin->PaintWindowBackground;
	    pWin->PaintWindowBackground = macIIPaintWindowBackground;
    }

    if ((void (*)())pWin->PaintWindowBorder != (void (*)())macIIPaintWindowBorder) {
	pPriv->PaintWindowBorder = pWin->PaintWindowBorder;
	pWin->PaintWindowBorder = macIIPaintWindowBorder;
    }

    if ((void (*)())pWin->CopyWindow != (void (*)())macIICopyWindow) {
	pPriv->CopyWindow = pWin->CopyWindow;
	pWin->CopyWindow = macIICopyWindow;
    }

    if (pWin->backStorage &&
	((void (*)())pWin->backStorage->SaveDoomedAreas != (void (*)())macIISaveAreas)){
	    pPriv->SaveAreas = pWin->backStorage->SaveDoomedAreas;
	    pWin->backStorage->SaveDoomedAreas = macIISaveAreas;
    }
    if (pWin->backStorage &&
	(pWin->backStorage->RestoreAreas != macIIRestoreAreas)){
	    pPriv->RestoreAreas = pWin->backStorage->RestoreAreas;
	    pWin->backStorage->RestoreAreas = macIIRestoreAreas;
    }
    if (pWin->backStorage &&
	((void (*) ())pWin->backStorage->DrawGuarantee != (void (*)())macIIDrawGuarantee)){
	    pPriv->DrawGuarantee = pWin->backStorage->DrawGuarantee;
	    pWin->backStorage->DrawGuarantee = macIIDrawGuarantee;
    }
    
    return (TRUE);
}

