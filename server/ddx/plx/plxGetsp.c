/*
 *   Copyright (c) 1987, 88 by
 *   PARALLAX GRAPHICS, INCORPORATED, Santa Clara, California.
 *   All rights reserved
 *
 *   This software is furnished on an as-is basis, and may be used and copied
 *   only with the inclusion of the above copyright notice.
 *
 *   The information in this software is subject to change without notice.
 *   No committment is made as to the usability or reliability of this
 *   software.
 *
 *   Parallax Graphics, Inc.
 *   2500 Condensa Street
 *   Santa Clara, California  95051
 */

#ifndef lint
static char *sid_ = "@(#)plxGetsp.c	1.10 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

/*
 * GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
unsigned int *
plxGetSpans(pDrawable, wMax, ppt, pwidth, nspans)
DrawablePtr pDrawable;			/* drawable from which to get bits */
int wMax;				/* largest value of all *pwidths */
register DDXPointPtr ppt;		/* points to start copying from */
int *pwidth;				/* list of number of bits to copy */
int nspans;				/* number of scanlines to copy */
{
	register unsigned int *pdst;	/* where to put the bits */
	register DDXPointPtr pptLast;	/* one past last point to get */
	DDXPointPtr pptT;		/* temporary point */
	unsigned int *pdstStart;	/* start of storage for return data */
	short bump;
	short xorg, yorg;

	ifdebug(10) printf("plxGetSpans(), nspans=%d\n", nspans);

	pptLast = ppt + nspans;

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		xorg = yorg = 0;
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_READ, pDrawable, &xorg, &yorg)) {
			ErrorF("plxGetSpans: PIXMAP NOT IN CACHE\n");
			return (unsigned int *)0;
		}
		/* translate X cordinate system origin to cache item origin */
		yorg = PTY(yorg);
		break;
	}

	switch (pDrawable->depth) {
	case 1:
		p_opaq(UBIT_OPAQ_TABLE);
		break;
	case 8:
		p_opaq(0);
		break;
	default:
		ifdebug(1) printf("plxGetSpans() NULL, bad depth\n");
		return (unsigned int *)0;
	}

	/* translate points */
	pptT = ppt;
	while (pptT < pptLast) {
		pptT->x += xorg;
		pptT->y += yorg;
		pptT++;
	}

	pdstStart = (unsigned int *)Xalloc(nspans * PixmapBytePad(wMax, pDrawable->depth));
	pdst = pdstStart;
	bump = PixmapWidthInPadUnits(*pwidth, pDrawable->depth);

	while (ppt < pptLast) {
		ifdebug(10) printf("\tx,y,width=%d,%d,%d\n", ppt->x, ppt->y, *pwidth);

		switch (pDrawable->depth) {
		case 1:
			p_ubit(ppt->x, PTY(ppt->y), ppt->x + *pwidth - 1, PTY(ppt->y), pdst);
			break;
		case 8:
			p_uimg(ppt->x, PTY(ppt->y), ppt->x + *pwidth - 1, PTY(ppt->y), pdst);
			break;
		}
		pdst += bump;        /* bump up pointer and pad to scanline */
		ppt++;
		pwidth++;
	}

	p_opaq(0);

#ifdef notdef
	/*
	 * this is going to be difficult for the parallax board --
	 * no sort of off-board pixmap is supported for drawing
	 * operations. I'm not sure how we're going to do this...
	 */

	/*
	 * If the drawable is a window with some form of backing-store, consult
	 * the backing-store module to fetch any invalid spans from the window's
	 * backing-store. The pixmap is made into one long scanline and the
	 * backing-store module takes care of the rest. We do, however, have
	 * to tell the backing-store module exactly how wide each span is,
	 * padded to the correct boundary, so we allocate pwidthPadded and
	 * set those widths into it.
	 */
	if ((pDrawable->type == DRAWABLE_WINDOW) &&
		(((WindowPtr)pDrawable)->backingStore != NotUseful)) {
		PixmapPtr pPixmap;

		pPixmap = (PixmapPtr)ALLOCATE_LOCAL(sizeof(PixmapRec));
		if ((pPixmap != (PixmapPtr)NULL) && (pwidthPadded != (int *)NULL)) {
			pPixmap->drawable.type = DRAWABLE_PIXMAP;
			pPixmap->drawable.pScreen = pDrawable->pScreen;
			pPixmap->drawable.depth = pDrawable->depth;
			pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
			pPixmap->devKind = PixmapBytePad(wMax, PSZ) * nspans;
			pPixmap->width = (pPixmap->devKind >> 2) * PPW;
			pPixmap->height = 1;
			pPixmap->refcnt = 1;
			pPixmap->devPrivate = (pointer)pdstStart;
			miBSGetSpans(pDrawable, pPixmap, wMax, pptInit, pwidthInit, pwidthPadded, nspans);
		}
		DEALLOCATE_LOCAL(pPixmap);
	}
#endif /* notdef */
	return(pdstStart);
}
