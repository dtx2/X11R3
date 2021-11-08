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
static char *sid_ = "@(#)plxFill.c	1.18 09/01/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

void
plxPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
DrawablePtr pDrawable;
GCPtr pGC;
int nrectFill;				/* number of rectangles to fill */
xRectangle *prectInit;			/* Pointer to first rectangle to fill */
{
	int i,x,y,rop;
	register int height;
	register int width;
	register xRectangle *prect;
	short xorg,yorg,xt,yt;
	DDXPointPtr pptFirst;
	register DDXPointPtr ppt;
	int *pwFirst;
	register int *pw;

	ifdebug(2) printf("plxPolyFillRect(), nrect=%d\n", nrectFill);

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
			ErrorF("plxPolyFillRect: PIXMAP NOT IN CACHE\n");
			return;
		}
		/* translate X cordinate system origin to cache item origin */
		yorg = PTY(yorg);
		break;
	}

	if (xorg || yorg) {
		prect = prectInit;
		for (i = 0; i<nrectFill; i++, prect++) {
			prect->x += xorg;
			prect->y += yorg;
		}
	}

	/* reset xorg & yorg correct for clipping */
	if (pDrawable->type == DRAWABLE_WINDOW) {
		xorg = yorg = 0;
	}

	if (!plxclipdownload(pGC, xorg, yorg))
		return;
	plxMask(pDrawable, pGC);

	prect = prectInit;

	switch(pGC->fillStyle) {
	case FillSolid :
		rop = PLX_FROM_X_OP(pGC->alu);

		if (rop < 0) {
			while (--nrectFill >= 0) {
				ifdebug(2) printf("\tFillSolid pixel,x,y,w,h=%d,%d,%d,%d,%d\n", pGC->fgPixel, prect->x, prect->y, prect->width, prect->height);
				CLIPREG(p_box(pGC->fgPixel, prect->x, PTY(prect->y), prect->x+prect->width-1, PTY(prect->y+prect->height-1)));
				prect++;
			}
		} else {
			p_srop1(rop, ROP1_RMAP_TABLE, 0xff);
			while (--nrectFill >= 0) {
				ifdebug(2) printf("\tFillSolid x,y,w,h=%d,%d,%d,%d\n", prect->x, prect->y, prect->width, prect->height);
				CLIPREG(p_boxr1(ROP1_RMAP_TABLE, prect->x, PTY(prect->y), prect->x+prect->width-1, PTY(prect->y+prect->height-1)));
				prect++;
			}
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
AAA:		while (--nrectFill >= 0) {
			ifdebug(2) printf("\tFill*Stipple/Tiled x,y,w,h=%d,%d,%d,%d\n", prect->x, prect->y, prect->width, prect->height);
			CLIPREG(p_boxs(xt, yt, prect->x, PTY(prect->y), prect->x+prect->width-1, PTY(prect->y+(prect->height-1))));
			prect++;
		}
		break;
	}
	p_opaq(0);
	p_mask(0xffff);
}

/*
 * scanline filling for parallax graphics board.
 *
 * these routines all clip. they assume that anything that has called
 * them has already translated the points. (i.e. pGC->miTranslate is
 * non-zero, which is howit gets set in plxCreateGC().)
 */

void
plxSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GCPtr pGC;
int nInit;				/* number of spans to fill */
DDXPointPtr pptInit;			/* pointer to list of start points */
int *pwidthInit;			/* pointer to list of n widths */
int fSorted;
{
	int n;				/* number of spans to fill */
	register DDXPointPtr ppt;	/* pointer to list of start points */
	short xorg,yorg;
	int temp;
	short rop;

	ifdebug(2) printf("plxSolidFS(), n=%d\n", nInit);

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		xorg = yorg = 0;
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_WRITE, pDrawable, &xorg, &yorg)) {
			ErrorF("plxSolidFS: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}
	if (!plxclipdownload(pGC, xorg, yorg))
		return;
	plxMask(pDrawable, pGC);

	if (xorg || yorg) {
		n = nInit; 
		ppt = pptInit;
		while (n--) {
			ppt->x += xorg;
			ppt->y += yorg;
			ppt++;
		}
	}

	rop = PLX_FROM_X_OP(pGC->alu);

	while (nInit--) {
		ifdebug(2) printf("\tx,y,width=%d,%d,%d\n", pptInit->x, pptInit->y, *pwidthInit);

		if(rop < 0) {
			CLIPREG(p_box(pGC->fgPixel, pptInit->x, PTY(pptInit->y), pptInit->x + *pwidthInit - 1, PTY(pptInit->y)));
		} else {
			p_srop1(rop, ROP1_RMAP_TABLE, 0xff);
			CLIPREG(p_boxr1(ROP1_RMAP_TABLE, pptInit->x, PTY(pptInit->y), pptInit->x + *pwidthInit - 1, PTY(pptInit->y)));
		}
		pwidthInit++;
		pptInit++;
	}
	p_mask(0xffff);
}

void
plxTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GCPtr pGC;
int nInit;				/* number of spans to fill */
DDXPointPtr pptInit;			/* pointer to list of start points */
int *pwidthInit;			/* pointer to list of n widths */
int fSorted;
{
	int n;				/* number of spans to fill */
	register DDXPointPtr ppt;	/* pointer to list of start points */
	short xorg,yorg;
	int temp;
	short rop;
	short xt,yt;

	ifdebug(2) printf("plxTileFS(), n=%d\n", nInit);

	if (!plxpreparepattern(pGC->tile, &xt, &yt)) {
		ErrorF("plxTileFS: TILE NOT IN CACHE\n");
		return;
	}

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		xorg = yorg = 0;
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_WRITE, pDrawable, &xorg, &yorg)) {
			ErrorF("plxTileFS: PIXMAP NOT IN CACHE\n");
			return;
		}
		/* translate X cordinate system origin to cache item origin */
		yorg = PTY(yorg);
		break;
	}
	if (!plxclipdownload(pGC, xorg, yorg))
		return;
	plxMask(pDrawable, pGC);

	if (xorg || yorg) {
		n = nInit; 
		ppt = pptInit;
		while (n--) {
			ppt->x += xorg;
			ppt->y += yorg;
			ppt++;
		}
	}

	while (nInit--) {
		ifdebug(2) printf("\tx,y,width=%d,%d,%d\n", pptInit->x, pptInit->y, *pwidthInit);

		CLIPREG(p_boxs(xt, yt, pptInit->x, PTY(pptInit->y), pptInit->x + *pwidthInit - 1, PTY(pptInit->y)));
		pwidthInit++;
		pptInit++;
	}

	p_opaq(0);
	p_mask(0xffff);
}

/*
 * Fill spans with stipples
 */
void
plxStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GCPtr pGC;
int nInit;				/* number of spans to fill */
DDXPointPtr pptInit;			/* pointer to list of start points */
int *pwidthInit;			/* pointer to list of n widths */
int fSorted;
{
	int n;				/* number of spans to fill */
	register DDXPointPtr ppt;	/* pointer to list of start points */
	short xorg,yorg,xt,yt;
	int temp;
	short rop;

	ifdebug(2) printf("plxStippleFS(), n=%d\n", nInit);

	{
		plxPrivGCPtr pPriv = ((plxPrivGCPtr)(pGC->devPriv));

		if (!plxpreparepattern(pPriv->plxStipple, &xt, &yt)) {
			ErrorF("plxStippleFS: STIPPLE NOT IN CACHE\n");
			return;
		}
	}

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		xorg = yorg = 0;
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_WRITE, pDrawable, &xorg, &yorg)) {
			ErrorF("plxStippleFS: PIXMAP NOT IN CACHE\n");
			return;
		}
		/* translate X cordinate system origin to cache item origin */
		yorg = PTY(yorg);
		break;
	}
	if (!plxclipdownload(pGC, xorg, yorg))
		return;
	plxMask(pDrawable, pGC);

	if (xorg || yorg) {
		n = nInit; 
		ppt = pptInit;
		while (n--) {
			ppt->x += xorg;
			ppt->y += yorg;
			ppt++;
		}
	}

	switch (pGC->fillStyle) {
	case FillStippled:			/* only write foreground */
		p_opaq(STIPPLE_OPAQ_TABLE);
		p_opaqm(0xff, pGC->fgPixel);
		break;
	case FillOpaqueStippled :
		p_opaq(0);
		break;
	}

	while (nInit--) {
		ifdebug(2) printf("\tx,y,width=%d,%d,%d\n", pptInit->x, pptInit->y, *pwidthInit);

		CLIPREG(p_boxs(xt, yt, pptInit->x, PTY(pptInit->y), pptInit->x + *pwidthInit - 1, PTY(pptInit->y)));
		pwidthInit++;
		pptInit++;
	}

	p_opaq(0);
	p_mask(0xffff);
}
