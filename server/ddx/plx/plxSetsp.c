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
static char *sid_ = "@(#)plxSetsp.c	1.13 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

/* plxsetscanline -- copies the bits from psrc to the drawable starting at
 * (x, y) and continuing to (x+w-1, y).  xOrigin tells us where psrc 
 * starts on the scanline. (I.e., if this scanline passes through multiple
 * boxes, we may not want to start grabbing bits at psrc but at some offset
 * further on.) 
 */

void
plxsetscanline(depth, x, y, w, psrc, pGC, xorg, yorg)
int x, y, w;
register char *psrc;
GCPtr pGC;
{
	ifdebug(2) printf("plxsetscanline(), x,y,w=%d,%d,%d\n", x, y, w);

	switch(pGC->alu) {
		/* fills */
	case GXcopy:
		switch (depth) {
		case 1:
			plxbyteswap(0);
			CLIPREG(p_lbit(x, PTY(y), x + w - 1, PTY(y), psrc));
			plxbyteswap(1);
			break;
		case 8:
			CLIPREG(p_limg(x, PTY(y), x + w - 1, PTY(y), psrc));
			break;
		}
		break;
	case GXclear:
		CLIPREG(p_box(0, x, PTY(y), x + w - 1, PTY(y)));
		break;
	case GXset:
		CLIPREG(p_box(0xff, x, PTY(y), x + w - 1, PTY(y)));
		break;
		/* double rops */
	case GXand:
	case GXandReverse:
	case GXandInverted:
	case GXxor:
	case GXor:
	case GXnor:
	case GXequiv:
	case GXorReverse:
	case GXorInverted:
	case GXnand:
		switch (depth) {
		case 1:
			plxbyteswap(0);
			p_lbit(0, AREA1_TOP, w-1, AREA1_TOP, psrc);
			plxbyteswap(1);
			break;
		case 8:
			p_limg(0, AREA1_TOP, w-1, AREA1_TOP, psrc);
			break;
		}
		p_srop2(PLX_FROM_X_OP(pGC->alu));
		CLIPREG(p_boxr2(0, AREA1_TOP, x, PTY(y), x + w - 1, PTY(y)));
		break;
		/* single rops */
	case GXcopyInverted:
		switch (depth) {
		case 1:
			plxbyteswap(0);
			CLIPREG(p_lbit(x, PTY(y), x + w - 1, PTY(y), psrc));
			plxbyteswap(1);
			break;
		case 8:
			CLIPREG(p_limg(x, PTY(y), x + w - 1, PTY(y), psrc));
			break;
		}
	case GXinvert:
		p_srop1(7, ROP1_RMAP_TABLE, 0xff);	/* not destination */
		CLIPREG(p_boxr1(ROP1_RMAP_TABLE, x, PTY(y), x + w - 1, PTY(y)));
		break;
	case GXnoop:
		break;
	}
}

/* SetSpans -- for each span copy pwidth[i] bits from psrc to pDrawable at
 * ppt[i] using the raster op from the GC. If fSorted is TRUE, the scanlines
 * are in increasing Y order.
 * Source bit lines are server scanline padded so that they always begin
 * on a word boundary.
 */

void
plxSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
DrawablePtr pDrawable;
GCPtr pGC;
int *psrc;
register DDXPointPtr ppt;
int *pwidth;
int nspans;
int fSorted;
{
	int *pdstBase;			/* start of dst bitmap */
	int widthDst;			/* width of bitmap in words */
	register BoxPtr pbox, pboxLast, pboxTest;
	register DDXPointPtr pptLast;
	DDXPointPtr pptT;
	RegionPtr prgnDst;
	int xStart, xEnd, yMax;
	short bump,xorg,yorg;

	ifdebug(2) printf("plxSetSpans(), nspans=%d\n", nspans);

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		if (pGC->miTranslate) {
			xorg = ((WindowPtr)pDrawable)->absCorner.x;
			yorg = ((WindowPtr)pDrawable)->absCorner.y;
		} else {
			xorg = yorg = 0;
		}
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_WRITE, pDrawable, &xorg, &yorg)) {
			ErrorF("plxSetSpans: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	/* translate points */
	pptLast = ppt + nspans;
	pptT = ppt;
	while (pptT < pptLast) {
		pptT->x += xorg;
		pptT->y += yorg;
		pptT++;
	}

	/* reset xorg & yorg for clipping */
	if (pDrawable->type == DRAWABLE_WINDOW) {
		xorg = yorg = 0;
	}
	if (!plxclipdownload(pGC, xorg, yorg))
		return;
	plxMask(pDrawable, pGC);

	switch (pDrawable->depth) {
	case 1:
		p_rmap(LBIT_RMAP_TABLE);
		break;
	case 8:
		p_rmap(0);
		break;
	default:
		ifdebug(1) printf("plxSetSpans() NULL, bad depth\n");
		return;
	}

	pptLast = ppt + nspans;

	bump = PixmapWidthInPadUnits(*pwidth, pDrawable->depth);

	while (ppt < pptLast) {
		switch (pDrawable->depth) {
		case 1:
			plxsetscanline(1, ppt->x, ppt->y, *pwidth, psrc, pGC, xorg, yorg);
			break;
		case 8:
			plxsetscanline(8, ppt->x, ppt->y, *pwidth, psrc, pGC, xorg, yorg);
			break;
		}
		psrc += bump;
		ppt++;
		pwidth++;
	}
	p_rmap(0);
	p_mask(0xffff);
}
