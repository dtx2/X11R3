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
static char *sid_ = "@(#)plxZeroln.c	1.15 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

/*
 * from plxPolygon.c
 */
extern short *plxpointslist;
extern int plxpointslistsize;

void
plxZeroPolylines(pDrawable, pGC, mode, nptInit, pptInit)
DrawablePtr pDrawable;
GCPtr pGC;
int mode;
int nptInit;
xPoint *pptInit;
{
	short xorg, yorg;
	register xPoint *ppt;
	register int npt;
	register short *p;
	int func = pGC->alu;
	int srcpix = pGC->fgPixel;

	ifdebug(9) printf("plxZeroPolylines(), npoints=%d\n", nptInit);

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
			ErrorF("plxZeroPolylines: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	if (plxpointslistsize < nptInit) {
		plxpointslistsize = nptInit;
		plxpointslist = (short *)Xrealloc(plxpointslist, plxpointslistsize*sizeof(short)*2);
	}
	p = plxpointslist;

	/* handle coordinate mode */
	ppt = pptInit;
	npt = nptInit;

	switch (mode) {
	case CoordModeOrigin:
		while (--npt >= 0) {
			*p++ = PTX((ppt->x + xorg));
			*p++ = PTY((ppt->y + yorg));
			ppt++;
		}
		break;
	case CoordModePrevious:
		*p++ = PTX((ppt->x += xorg));
		*p++ = PTY((ppt->y += yorg));
		ppt++;
		--npt;
		while (--npt >= 0) {
			*p++ = PTX((ppt->x += (ppt-1)->x));
			*p++ = PTY((ppt->y += (ppt-1)->y));
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
		CLIPREG(p_vectm(srcpix, npt, plxpointslist));
		break;
	case GXxor:
		p = plxpointslist;
		while (--npt > 0) {
			CLIPREG(p_vectx(srcpix, *(p+0), *(p+1), *(p+2), *(p+3)));
			p += 2;
		}
		break;
	case GXnoop:
		break;
	default:
		ErrorF("X: Parallax: Unsupported Vector Rop: %d, %d\n", func, pGC->alu);
	}

	p_mask(0xffff);
}

void
plxZeroPolySegment(pDrawable, pGC, nsgInit, psgInit)
DrawablePtr pDrawable;
GCPtr pGC;
int nsgInit;
xSegment *psgInit;
{
	short xorg, yorg;
	register xSegment *psg;
	register int nsg;
	register short *p;
	int func = pGC->alu;
	int srcpix = pGC->fgPixel;

	ifdebug(9) printf("plxZeroPolySegment(), nsegments=%d\n", nsgInit);

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
			ErrorF("plxZeroPolySegment: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	if (plxpointslistsize < (nsgInit*2)) {
		plxpointslistsize = nsgInit*2;
		plxpointslist = (short *)Xrealloc(plxpointslist, plxpointslistsize*sizeof(short)*2);
	}
	p = plxpointslist;

	/* handle coordinate mode */
	psg = psgInit;
	nsg = nsgInit;

	while (--nsg >= 0) {
		*p++ = PTX((psg->x1 + xorg));
		*p++ = PTY((psg->y1 + yorg));
		*p++ = PTX((psg->x2 + xorg));
		*p++ = PTY((psg->y2 + yorg));
		psg++;
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

	psg = psgInit;
	nsg = nsgInit;

	switch(func) {
	case GXcopy:
		p = plxpointslist;
		while (--nsg >= 0) {
			CLIPREG(p_vect(srcpix, *(p+0), *(p+1), *(p+2), *(p+3)));
			p += 4;
		}
		break;
	case GXxor:
		p = plxpointslist;
		while (--nsg >= 0) {
			CLIPREG(p_vectx(srcpix, *(p+0), *(p+1), *(p+2), *(p+3)));
			p += 4;
		}
		break;
	case GXnoop:
		break;
	default:
		ErrorF("X: Parallax: Unsupported Vector Rop: %d, %d\n", func, pGC->alu);
	}

	p_mask(0xffff);
}

void
plxZeroPolyRectangle(pDrawable, pGC, nrectInit, prectInit)
DrawablePtr pDrawable;
GCPtr pGC;
int nrectInit;
xRectangle *prectInit;
{
	short xorg, yorg;
	register xRectangle *prect;
	register int nrect;
	register short *p;
	int func = pGC->alu;
	int srcpix = pGC->fgPixel;

	ifdebug(9) printf("plxZeroPolyRectangle(), nrect=%d\n", nrectInit);

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
			ErrorF("plxZeroPolyRectangle: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	if (plxpointslistsize < (nrectInit*2)) {
		plxpointslistsize = nrectInit*2;
		plxpointslist = (short *)Xrealloc(plxpointslist, plxpointslistsize*sizeof(short)*2);
	}
	p = plxpointslist;

	/* handle coordinate mode */
	prect = prectInit;
	nrect = nrectInit;

	while (--nrect >= 0) {
		*p++ = PTX((prect->x + xorg));
		*p++ = PTY((prect->y + yorg));
		*p++ = PTX((prect->x + prect->width - 1 + xorg));
		*p++ = PTY((prect->y + prect->height - 1 + yorg));
		prect++;
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

	prect = prectInit;
	nrect = nrectInit;

	switch(func) {
	case GXcopy:
		p = plxpointslist;
		while (--nrect >= 0) {
			CLIPREG(p_boxo(srcpix, *(p+0), *(p+1), *(p+2), *(p+3)));
			p += 4;
		}
		break;
	case GXxor:
		p = plxpointslist;
		while (--nrect >= 0) {
			CLIPREG(p_vectx(srcpix, *(p+0), *(p+1), *(p+2), *(p+1)));
			CLIPREG(p_vectx(srcpix, *(p+2), *(p+1), *(p+2), *(p+3)));
			CLIPREG(p_vectx(srcpix, *(p+2), *(p+3), *(p+0), *(p+3)));
			CLIPREG(p_vectx(srcpix, *(p+0), *(p+3), *(p+0), *(p+1)));
			p += 4;
		}
		break;
	case GXnoop:
		break;
	default:
		ErrorF("X: Parallax: Unsupported Vector Rop: %d, %d\n", func, pGC->alu);
	}

	p_mask(0xffff);
}
