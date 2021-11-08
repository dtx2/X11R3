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
static char *sid_ = "@(#)plxPushpix.c	1.22 09/01/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

/* plxPushPixels -- squeegees the fill style of pGC through pBitMap
 * into pDrawable.  pBitMap is a stencil (width by height of it is used, it may
 * be bigger) which is placed on the drawable at xOrg, yOrg.  Where a 1 bit
 * is set in the bitmap, the fill style is put onto the drawable using
 * the GC's logical function. The drawable is not changed where the bitmap
 * has a zero bit or outside the area covered by the stencil.
 */

void
plxPushPixels(pGC, pBitMap, pDrawable, width, height, x, y)
GCPtr pGC;
PixmapPtr pBitMap;
DrawablePtr pDrawable;
int width, height;
int x, y;					/* position in pDrawable */
{
	short xorg, yorg, stenx, steny;

	ifdebug(2) printf("plxPushPixels()\n");

	/* prepare the stencil */
	if (!plxpixmapuse(PIXMAP_READ, pBitMap, &stenx, &steny)) {
		ErrorF("plxPushPixels: STENCIL NOT IN CACHE\n");
		return;
	}
	steny = PTY(steny);

	/* prepare the destination */
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
			ErrorF("plxPushPixels: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);	/* offset in X coordinates */
		break;
	}

	x += xorg;
	y += yorg;

	/* reset xorg & yorg for clipping */
	if (pDrawable->type == DRAWABLE_WINDOW) {
		xorg = yorg = 0;
	}
	if (!plxclipdownload(pGC, xorg, yorg))
		return;
	plxMask(pDrawable, pGC);

	/*
	 * the 0x00000001 is the planemask for bitplane 0, lets hope
	 * that all bitmap are kept that way for now
	 */
	plxdrawthrustencil(pGC, pDrawable, width, height, x, y, stenx, steny, xorg, yorg, 0x00000001);

	p_mask(0xffff);
}

/*
 * Given the upper left corner of a stencil, stencil a drawable
 * with the current fill style.
 * Both the size of the drawable, and the stencil, must be small
 * enough to fit in cache.
 */

plxdrawthrustencil(pGC, pDrawable, width, height, x, y, stenx, steny, xorg, yorg, planemask)
GCPtr pGC;
DrawablePtr pDrawable;
int width, height;
int x,y;			/* position in drawable to stencil */
short stenx,steny;		/* upper left corner of stencil */
short xorg, yorg;
int planemask;			/* source plane for mask */
{
	short rop;
	short xt, yt;

	ifdebug(2) printf("plxdrawthrustencil() x,y=%d,%d\n", x, y);

	/*
	 * special case the easy most common case of solid fill and
	 * copy GXcopy
	 */
	if ((pGC->fillStyle == FillSolid) && (pGC->alu == GXcopy)) {
		SetFontMaps(pGC->fgPixel, pGC->bgPixel, planemask, 1);
		CLIPREG(p_boxc(stenx, steny, x, PTY(y), x+width-1,  PTY(y+height-1)));
		p_rmap(0); 
		p_opaq(0);
		return;
	}

	switch(pGC->fillStyle) {
	case FillStippled:
	case FillOpaqueStippled:
		{
			plxPrivGCPtr pPriv = ((plxPrivGCPtr)(pGC->devPriv));

			if (!plxpreparepattern(pPriv->plxStipple, &xt, &yt)) {
				ErrorF("plxdrawthrustencil: STIPPLE NOT IN CACHE\n");
				return;
			}
		}
		break;
	case FillTiled:
		if (!plxpreparepattern(pGC->tile, &xt, &yt)) {
			ErrorF("plxdrawthrustencil: TILE NOT IN CACHE\n");
			return;
		}
		break;
	}

	p_clipd();			/* REMEMBER TO TURN ON AGAIN */
	p_mask(0);
	p_box(STENCIL_COLOR, 0, AREA1_TOP, width-1, AREA1_TOP - height + 1);
	/*
	 * set mask value again, pixmap routines set it back to all bits
	 */
	plxMask(pDrawable, pGC);

	/*
	 * fill AREA1 with the fill style ropped 
	 * if necessary with the data in the destination.
	 */

	switch(pGC->fillStyle) {
	case FillSolid:
		rop = PLX_FROM_X_OP(pGC->alu);

		if(rop < 0) {
			p_box(pGC->fgPixel, 0, AREA1_TOP, width-1, AREA1_TOP - height + 1);
		} else {
			p_srop1(rop, ROP1_RMAP_TABLE, 0xff);
			p_rmap(ROP1_RMAP_TABLE);
			p_boxc(x, PTY(y), 0, AREA1_TOP, width-1, AREA1_TOP - height + 1);
		}
		break;
	case FillStippled:
		p_opaq(STIPPLE_OPAQ_TABLE);
		p_opaqm(0xff, pGC->fgPixel);
		goto AAA;
	case FillOpaqueStippled:
		p_opaq(0);
		goto AAA;
	case FillTiled:
		p_opaq(0);
AAA:
		/*
		 * for stipples >16 by 16 we use a slow stipple.
		 */
		p_boxs(xt, yt, 0, AREA1_TOP, width-1, AREA1_TOP - height + 1);
		break;
	}

	/*
	 * apply the areas of the stencil that should cause "don't draws" to
	 * copy in AREA1 in color STENCIL_COLOR.
	 */
	SetFontMaps(STENCIL_COLOR, STENCIL_COLOR, planemask, 1);
	p_opaq(FONT_OPAQ_TABLE + 1);		/* only draw background */
	p_boxc(stenx,steny,0,AREA1_TOP,width-1,AREA1_TOP-height+1);

	/*
	 * Draw the result back to the destination except
	 * color STENCIL_COLOR
	 */
	p_rmap(0);
	p_opaq(ONLY_STENCIL_COLOR_OPAQ_TABLE+1);

	p_clipe();			/* WE REMEMBERED */
	CLIPREG(p_boxc(0,AREA1_TOP,x,PTY(y),x+width-1, PTY(y+height-1)));

	p_opaq(0);
}
