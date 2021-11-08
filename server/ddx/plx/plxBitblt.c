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
static char *sid_ = "@(#)plxBitblt.c	1.16 09/01/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

extern RegionPtr	miHandleExposures ();

/* PLXCOPYAREA -- Greg Cockroft
 * Code heavily cribbed from X10 copy.c
 * and micopyarea.
 *
 * Copy rect from one Drawable to another.
 * Currently will only be used for copying between
 * drawables in Framebuffer. That is either windows
 * or cached pixmaps.
 */
RegionPtr
plxCopyArea(pDrawableSrc, pDrawableDst, pGC, xSrc, ySrc, wDst, hDst, xDst, yDst)
register DrawablePtr pDrawableSrc;
register DrawablePtr pDrawableDst;
GCPtr pGC;
int xSrc, ySrc;
int wDst, hDst;
int xDst, yDst;
{
	short xorg, yorg;
	int xSrcSave = xSrc, ySrcSave = ySrc, xDstSave = xDst, yDstSave = yDst;
	int op;
	RegionPtr	prgnExposed;

	ifdebug(12) printf("plxCopyArea() 0x%08x 0x%08x, x,ySrc=%d,%d out x,y,w,hDst=%d,%d,%d,%d\n", pDrawableSrc, pDrawableDst, xSrc, ySrc, xDst, yDst, wDst, hDst);

	switch (pDrawableSrc->type) {
	case DRAWABLE_WINDOW:
		if (!((WindowPtr)pDrawableDst)->realized) {
			return NULL;
		}
		if (pGC->miTranslate) {
			xorg = ((WindowPtr)pDrawableSrc)->absCorner.x;
			yorg = ((WindowPtr)pDrawableSrc)->absCorner.y;
		} else {
			xorg = yorg = 0;
		}
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_READ, pDrawableSrc, &xorg, &yorg)) {
			ErrorF("plxCopyArea: SRC PIXMAP NOT IN CACHE\n");
			return NULL;
		}
		yorg = PTY(yorg);
		break;
	}

	xSrc += xorg;
	ySrc += yorg;

	switch (pDrawableDst->type) {
	case DRAWABLE_WINDOW:
		if (pGC->miTranslate) {
			xorg = ((WindowPtr)pDrawableDst)->absCorner.x;
			yorg = ((WindowPtr)pDrawableDst)->absCorner.y;
		} else {
			xorg = yorg = 0;
		}
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_WRITE, pDrawableDst, &xorg, &yorg)) {
			ErrorF("plxCopyArea: DST PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	xDst += xorg;
	yDst += yorg;

	if (pDrawableDst->type == DRAWABLE_WINDOW) {
		xorg = yorg = 0;
	}

	if (!plxclipdownload(pGC, xorg, yorg))
		return NULL;
	plxMask(pDrawableDst, pGC);

	op = PLX_FROM_X_OP(pGC->alu);

	if (pGC->alu == GXinvert) {
		set_rop1(op,0xffff);
		op = -5;
	}

	switch (pGC->alu) {
	case GXcopy:
		/*
		 * a straight box copy, although source may overlap
		 * destination, so possibly reverse the blit direction
		 * to prevent blit feedback
		 */
		if ((ySrc > yDst) || ((ySrc == yDst) && (xSrc > xDst))) {
			CLIPREG(p_boxc(xSrc, PTY(ySrc), xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		} else {
			CLIPREG(p_boxc((xSrc+wDst-1), PTY(ySrc+hDst-1), (xDst+wDst-1), PTY(yDst+hDst-1), xDst,PTY(yDst)));
		}
		break;
	case GXinvert:
		CLIPREG(p_boxr1(ROP1_RMAP_TABLE, xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		break;
	case GXclear:
		CLIPREG(p_box(0, xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		break;
	case GXset:
		CLIPREG(p_box(0xffff, xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		break;
	default:
		if (op >= 0) {
			/* a ROP to destination copy */
			p_srop2(op);
			CLIPREG(p_boxr2(xSrc, PTY(ySrc), xDst, PTY(yDst), (xDst+wDst-1), PTY(yDst+hDst-1)));
		}
		break;
	}

	prgnExposed = miHandleExposures(pDrawableSrc, pDrawableDst, pGC, xSrcSave, ySrcSave, wDst, hDst, xDstSave, yDstSave, 0);
	p_mask(0xffff);
	return prgnExposed;
}

RegionPtr
plxCopyPlane(pDrawableSrc, pDrawableDst, pGC, xSrc, ySrc, wDst, hDst, xDst, yDst, plane)
register DrawablePtr pDrawableSrc;
register DrawablePtr pDrawableDst;
GCPtr pGC;
int xSrc, ySrc;
int wDst, hDst;
int xDst, yDst;
long plane;
{
	short xorg, yorg;
	int xSrcSave = xSrc, ySrcSave = ySrc, xDstSave = xDst, yDstSave = yDst;
	int op;
	RegionPtr	prgnExposed;

	ifdebug(12) printf("plxCopyPlane() 0x%08x 0x%08x, x,ySrc=%d,%d out x,y,w,hDst=%d,%d,%d,%d, plane=0x08x\n", pDrawableSrc, pDrawableDst, xSrc, ySrc, xDst, yDst, wDst, hDst, plane);

	switch (pDrawableSrc->type) {
	case DRAWABLE_WINDOW:
		if (!((WindowPtr)pDrawableDst)->realized) {
			return NULL;
		}
		if (pGC->miTranslate) {
			xorg = ((WindowPtr)pDrawableSrc)->absCorner.x;
			yorg = ((WindowPtr)pDrawableSrc)->absCorner.y;
		} else {
			xorg = yorg = 0;
		}
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_READ, pDrawableSrc, &xorg, &yorg)) {
			ErrorF("plxCopyArea: SRC PIXMAP NOT IN CACHE\n");
			return NULL;
		}
		yorg = PTY(yorg);
		break;
	}

	xSrc += xorg;
	ySrc += yorg;

	switch (pDrawableDst->type) {
	case DRAWABLE_WINDOW:
		if (pGC->miTranslate) {
			xorg = ((WindowPtr)pDrawableDst)->absCorner.x;
			yorg = ((WindowPtr)pDrawableDst)->absCorner.y;
		} else {
			xorg = yorg = 0;
		}
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_WRITE, pDrawableDst, &xorg, &yorg)) {
			ErrorF("plxCopyArea: DST PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	xDst += xorg;
	yDst += yorg;

	if (pDrawableDst->type == DRAWABLE_WINDOW) {
		xorg = yorg = 0;
	}

	if (!plxclipdownload(pGC, xorg, yorg))
		return NULL;
	plxMask(pDrawableDst, pGC);

	SetFontMaps(pGC->fgPixel, pGC->bgPixel, plane, 0);

	op = PLX_FROM_X_OP(pGC->alu);

	if (pGC->alu == GXinvert) {
		set_rop1(op,0xffff);
		op = -5;
	}

	switch (pGC->alu) {
	case GXcopy:
		/*
		 * a straight box copy, although source may overlap
		 * destination, so possibly reverse the blit direction
		 * to prevent blit feedback
		 */
		if ((ySrc > yDst) || ((ySrc == yDst) && (xSrc > xDst))) {
			CLIPREG(p_boxc(xSrc, PTY(ySrc), xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		} else {
			CLIPREG(p_boxc((xSrc+wDst-1), PTY(ySrc+hDst-1), (xDst+wDst-1), PTY(yDst+hDst-1), xDst,PTY(yDst)));
		}
		break;
	case GXinvert:
		CLIPREG(p_boxr1(ROP1_RMAP_TABLE, xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		break;
	case GXclear:
		CLIPREG(p_box(0, xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		break;
	case GXset:
		CLIPREG(p_box(0xffff, xDst, PTY(yDst), xDst+wDst-1, PTY(yDst+hDst-1)));
		break;
	default:
		if (op >= 0) {
			/* a ROP to destination copy */
			p_srop2(op);
			CLIPREG(p_boxr2(xSrc, PTY(ySrc), xDst, PTY(yDst), (xDst+wDst-1), PTY(yDst+hDst-1)));
		}
		break;
	}

	prgnExposed = miHandleExposures(pDrawableSrc, pDrawableDst, pGC, xSrcSave, ySrcSave, wDst, hDst, xDstSave, yDstSave, 0);
	p_rmap(0);
	p_mask(0xffff);
	return prgnExposed;
}
