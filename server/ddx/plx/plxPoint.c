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
static char *sid_ = "@(#)plxPoint.c	1.5 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

void
plxPolyPoint(pDrawable, pGC, mode, nptInit, pptInit)
DrawablePtr pDrawable;
GCPtr pGC;
int mode;
int nptInit;
xPoint *pptInit;
{
	short xorg, yorg;
	register xPoint *ppt;
	register int npt;
	int func = pGC->alu;
	int srcpix = pGC->fgPixel;

	ifdebug(9) printf("plxPolyPoint(), npoints=%d\n", nptInit);

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
			ErrorF("plxZeroLine: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	/* handle coordinate mode */
	ppt = pptInit;
	npt = nptInit;
	switch (mode) {
	case CoordModeOrigin:
		while (--npt >= 0) {
			ppt->x += xorg;
			ppt->y += yorg;
			ppt++;
		}
		break;
	case CoordModePrevious:
		ppt->x += xorg;
		ppt->y += yorg;
		ppt++;
		--npt;
		while (--npt >= 0) {
			ppt->x += (ppt-1)->x;
			ppt->y += (ppt-1)->y;
			ppt++;
		}
		break;
	}

	/* reset xorg & yorg for clipping */
	if (pDrawable->type == DRAWABLE_WINDOW) {
		xorg = yorg = 0;
	}
	if (!plxclipdownload(pGC, xorg, yorg))
		return;
	plxMask(pDrawable, pGC);

	/* set up rop function stuff */
	switch (func) {
	case GXclear:
		func = GXcopy;
		srcpix = 0;
		break;
	case GXset:
		func = GXcopy;
		srcpix = 0xff;
		break;
	case GXcopyInverted:
		func = GXcopy;
		srcpix ^= 0xff;
		break;
	case GXequiv:
		func = GXxor;
		srcpix ^= 0xff;
		break;
	case GXinvert:
		func = GXxor;
		srcpix = 0xff;
		break;
	}

	ppt = pptInit;
	npt = nptInit;

	switch(func) {
	case GXcopy:
		while (--npt >= 0) {
			CLIPREG(p_box(srcpix, ppt->x, PTY(ppt->y), ppt->x, PTY(ppt->y)));
			ppt++;
		}
		break;
	case GXxor:
		set_rop1(PLX_FROM_X_OP(pGC->alu), srcpix);
		while (--npt >= 0) {
			CLIPREG(p_boxr1(ROP1_RMAP_TABLE, ppt->x, PTY(ppt->y), ppt->x, PTY(ppt->y)));
			ppt++;
		}
		break;
	case GXnoop:
		break;
	default:
		ErrorF("X: Parallax: Unsupported Point Rop: %d, %d\n", func, pGC->alu);
		break;
	}

	p_mask(0xffff);
}
