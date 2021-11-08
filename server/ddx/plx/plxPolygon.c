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
static char *sid_ = "@(#)plxPolygon.c	1.7 09/01/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

short *plxpointslist = (short *)0;
int plxpointslistsize = 0;			/* # of points */

void
plxFillPolygon(pDrawable, pGC, shape, mode, nptInit, pptInit)
DrawablePtr pDrawable;
GCPtr pGC;
int shape, mode;
int nptInit;
xPoint *pptInit;
{
	short xorg, yorg, xt, yt;
	register xPoint *ppt;
	register int npt;
	register short *p;
	int rop;
	int func = pGC->alu;

	ifdebug(2) printf("plxFillPolygon(), npoints=%d, shape=%d\n", nptInit, shape);

	switch(pGC->fillStyle) {
	case FillStippled :
	case FillOpaqueStippled :
		{
			plxPrivGCPtr pPriv = ((plxPrivGCPtr)(pGC->devPriv));

			if (!plxpreparepattern(pPriv->plxStipple, &xt, &yt)) {
				ErrorF("plxPolyFillRect: STIPPLE NOT IN CACHE\n");
				return;
			}
		}
		break;
	case FillTiled :
		if (!plxpreparepattern(pGC->tile, &xt, &yt)) {
			ErrorF("plxPolyFillRect: TILE NOT IN CACHE\n");
			return;
		}
		break;
	}

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
			*p++ = PTX((ppt->x += xorg));
			*p++ = PTY((ppt->y += yorg));
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

	ppt = pptInit;
	npt = nptInit;

	switch(pGC->fillStyle) {
	case FillSolid :
		rop = PLX_FROM_X_OP(pGC->alu);

		if (rop < 0) {
			ifdebug(2) printf("\tFillSolid pixel=%d\n", pGC->fgPixel);
			CLIPREG(p_poly(pGC->fgPixel, npt, plxpointslist));
		} else {
			ifdebug(2) printf("\tFillSolid\n");
			set_rop1(rop, pGC->fgPixel);
			CLIPREG(p_polyr1(ROP1_RMAP_TABLE, npt, plxpointslist));
		}
		break;
	case FillStippled :
		p_opaq(STIPPLE_OPAQ_TABLE);
		p_opaqm(0xff, pGC->fgPixel);
		goto AAA;
	case FillOpaqueStippled :
		p_opaq(0);
		goto AAA;
	case FillTiled :
		p_opaq(0);
AAA:
		ifdebug(2) printf("\tFill*Stipple/Tiled\n");
		CLIPREG(p_polys(xt, yt, npt, plxpointslist));
		break;
	}

	p_opaq(0);
	p_mask(0xffff);
}
