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
static char *sid_ = "@(#)plxBstore.c	1.2 09/06/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

static WindowPtr saveWin;

static void (*globalSaveAreas)();
static RegionPtr (*globalRestoreAreas)();
static void plxPreSaveAreas();
static RegionPtr plxPreRestoreAreas();

/*
 * exchange SaveDoomedAreas and RestoreAreas
 * for our private wrappers, saving the old routines
 * so we can call them inside. This lets us
 * tell how to set the DAM bits.
 */

plxSwapSaveAndRestoreAreas(pWindow)
WindowPtr pWindow;
{
	if (pWindow->backStorage) {
		if ((int (*)()) pWindow->backStorage->SaveDoomedAreas != (int (*)()) plxPreSaveAreas) {
			globalSaveAreas = pWindow->backStorage->SaveDoomedAreas;
			pWindow->backStorage->SaveDoomedAreas = plxPreSaveAreas;
		}

		if (pWindow->backStorage->RestoreAreas != plxPreRestoreAreas) {
			globalRestoreAreas = pWindow->backStorage->RestoreAreas;
			pWindow->backStorage->RestoreAreas = plxPreRestoreAreas;
		}
	}
}

/*
 * simple wrapper to save the associated window
 */

static void
plxPreSaveAreas(pWin)
WindowPtr pWin;
{
	saveWin = pWin;
	globalSaveAreas(pWin);
}

void
plxSaveAreas(pPixmap, pRegion, xorg, yorg)
PixmapPtr pPixmap;
RegionPtr pRegion;	/* pixmap relative coordinates */
int xorg, yorg;		/* window offset */
{
	short pixx, pixy;
	int i;
	BoxPtr pBox;
	register MapPriv *mp = (MapPriv *)saveWin->devPrivate;

	if (!plxpixmapuse(PIXMAP_WRITE, pPixmap, &pixx, &pixy)) {
		ErrorF("plxCopyArea: DST PIXMAP NOT IN CACHE\n");
		return;
	}
	pixy = PTY(pixy);
	p_mask(0xffff);
	p_clipd();
	if (mp->video)
		p_damvv();
	else
		p_damvg();
	for (i=0,pBox=pRegion->rects;i<pRegion->numRects;i++,pBox++) {
		p_boxc(
			pBox->x1 + xorg, PTY(pBox->y1 + yorg),
			pBox->x1 + pixx, PTY(pBox->y1 + pixy),
			pBox->x2 + pixx - 1, PTY(pBox->y2 + pixy - 1));
	}
	p_clipe();
	p_damvx();
}

static WindowPtr restoreWin;

/*
 * simple wrapper to save the associated window
 */

static RegionPtr
plxPreRestoreAreas(pWin)
WindowPtr pWin;
{
	extern RegionPtr miRestoreAreas();
	restoreWin = pWin;
	return globalRestoreAreas(pWin);
}

plxRestoreAreas(pPixmap, pRegion, xorg, yorg)
PixmapPtr pPixmap;
RegionPtr pRegion;	/* screen relative coordinates */
int xorg, yorg;		/* window offset */
{
	short pixx, pixy;
	int i;
	BoxPtr pBox;
	register MapPriv *mp = (MapPriv *)restoreWin->devPrivate;

	if (!plxpixmapuse(PIXMAP_READ, pPixmap, &pixx, &pixy)) {
		ErrorF("plxCopyArea: DST PIXMAP NOT IN CACHE\n");
		return;
	}
	pixy = PTY(pixy);
	p_mask(0xffff);
	p_clipd();
	if (mp->video)
		p_damvv();
	else
		p_damvg();
	for (i=0,pBox=pRegion->rects;i<pRegion->numRects;i++,pBox++) {
		p_boxc(
			pBox->x1 - xorg + pixx, PTY(pBox->y1 - yorg + pixy),
			pBox->x1, PTY(pBox->y1),
			pBox->x2 - 1, PTY(pBox->y2 - 1));
	}
	p_clipe();
	p_damvx();
}
