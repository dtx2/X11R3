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
static char *sid_ = "@(#)plxPaint.c	1.21 08/05/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

/*
 * CLIPPING:
 * we could set clip to the rectangle list, in these routines, and
 * fill the entire window.
 * But it is probably easier and quicker to just do the fills.
 */

void
plxPaintAreaSolid(pWin, pRegion, what)
WindowPtr pWin;
RegionPtr pRegion;
int what;
{
	register int nbox;		/* number of boxes to fill */
	register BoxPtr pbox;		/* pointer to list of boxes to fill */
	int pix;			/* pixel value to draw */
	register MapPriv *mp = (MapPriv *)pWin->devPrivate;

	ifdebug(16) printf("plxPaintAreaSolid(), what=%d\n", what);

	plxMask(pWin, 0);
	if (!mp->video) {
		p_damvg();
	} else {
		p_damvv();
	}

	switch (what) {
	case PW_BACKGROUND:
		pix = pWin->backgroundPixel;
		break;
	case PW_BORDER:
		pix = pWin->borderPixel;
		break;
	}

	p_rmap(0);
	p_opaq(0);

	p_clipd();			/* REMEMBER TO TURN ON AGAIN */

	nbox = pRegion->numRects;
	pbox = pRegion->rects;
	while (nbox--) {
		ifdebug(16) printf("\tpix,x1,y1,x2,y2=%d,%d,%d,%d,%d\n", pix, pbox->x1, pbox->y1, pbox->x2-1, pbox->y2-1);
		p_box(pix, pbox->x1, PTY(pbox->y1), pbox->x2-1, PTY(pbox->y2-1));
		pbox++;
	}
	p_clipe();			/* WE REMEMBERED */
	p_damvx();
}

/*
 * plxPaintArea()
 * -- is a composite of PaintWindowBackground and PaintWindowBorder.
 * "what" is either the constant PW_BACKGROUND or PW_BORDER saying
 * which of the window's border or fill style to use.
 *	Each fill style is a tile pointer and a pixel value.
 * If the tile pointer is NULL, that means that the background is the
 * solid pixel value. In which case this routine shouldn't have been called.
 */

void
plxPaintArea(pWin, pRegion, what)
WindowPtr pWin;
RegionPtr pRegion;
int what;
{
	register int nbox;		/* number of boxes to fill */
	register BoxPtr pbox;		/* pointer to list of boxes to fill */
	unsigned long pixel;
	short xt, yt, xorg, yorg;
	PixmapPtr pPixmap = (PixmapPtr)0;
	register MapPriv *mp = (MapPriv *)pWin->devPrivate;

	ifdebug(16) printf("plxPaintArea(), what=%d\n", what);

	switch (what) {
	case PW_BACKGROUND:
		pPixmap = pWin->backgroundTile;
		break;
	case PW_BORDER:
		pPixmap = pWin->borderTile;
		break;
	}

	if (!plxpreparepattern(pPixmap, &xt, &yt)) {
		ErrorF("plxPaintArea: TILE NOT IN CACHE\n");
		return;
	}

	plxMask(pWin, 0);
	if (!mp->video) {
		p_damvg();
	} else {
		p_damvv();
	}

	p_rmap(0);
	p_opaq(0);

	plxclipinvalidate();			/* because of slow stipple */

	nbox = pRegion->numRects;
	pbox = pRegion->rects;
	while (nbox--) {
		ifdebug(16) printf("\tx1,y1,x2,y2=%d,%d,%d,%d\n", pbox->x1, pbox->y1, pbox->x2-1, pbox->y2-1);
		if ((pPixmap->width > 16) || (pPixmap->height > 16)) {
			plxslowstipple(pWin, xt, yt, pPixmap->width, pPixmap->height, pbox->x1, pbox->y1, pbox->x2-1, pbox->y2-1);
		} else {
			p_boxs(xt, yt, pbox->x1, PTY(pbox->y1), pbox->x2-1, PTY(pbox->y2-1));
		}
		pbox++;
	}
	p_damvx();
}

plxslowstipple(pWin, xt, yt, w, h, x1, y1, x2, y2)
WindowPtr pWin;
{
	int xorg, yorg;
	register int x, y;

	ifdebug(16) printf("plxslowstipple(), xt,yt=%d,%d w,h=%d,%d x1,y1,x2,y2=%d,%d,%d,%d\n", xt, yt, w, h, x1, y1, x2, y2);

	p_clip(x1, PTY(y1), x2, PTY(y2));

	if (pWin->absCorner.x <= x1)
		xorg = x1 - (x1 - pWin->absCorner.x) % w;
	else
		xorg = x1 + (pWin->absCorner.x - x1) % w + w;
	if (pWin->absCorner.y <= y1)
		yorg = y1 - (y1 - pWin->absCorner.y) % h;
	else
		yorg = y1 + (pWin->absCorner.y - y1) % h + h;

	for (x=xorg;x<=x2;x += w) {
		for (y=yorg;y<=y2;y += h) {
			p_boxc(xt, yt+h-1,
				x, PTY(y),
				(((x+w-1)>x2)?x2:x+w-1), PTY(((y+h-1)>y2)?y2:y+h-1));
		}
	}
}

void
plxPaintAreaNone(pWin, pRegion, what)
WindowPtr pWin;
RegionPtr pRegion;
int what;
{
	ifdebug(16) printf("plxPaintAreaNone(), what=%d\n", what);
}

/*
 * Paint Area Parent Relative -- Find first ancestor which isn't parent
 * relative and paint as it would, but with this region
 */
void
plxPaintAreaPR(pWin, pRegion, what)
WindowPtr pWin;
RegionPtr pRegion;
int what;
{
	WindowPtr pParent;
	register MapPriv *mp = (MapPriv *)pWin->devPrivate;

	ifdebug(16) printf("plxPaintAreaPR(), what=%d\n", what);

#ifdef notdef
	plxMask(pWin, 0);
	if (!mp->video) {
		p_damvg();
	} else {
		p_damvv();
	}
#endif

	pParent = pWin->parent;
	while(pParent->backgroundTile == (PixmapPtr)ParentRelative)
		pParent = pParent->parent;

	switch (what) {
	case PW_BACKGROUND:
		(*pParent->PaintWindowBackground)(pParent, pRegion, what);
		break;
	case PW_BORDER:
		(*pParent->PaintWindowBorder)(pParent, pRegion, what);
		break;
	}
}
