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
static char *sid_ = "@(#)plxWindow.c	1.19 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

#include	"mi.h"

Bool plxCreateWindow(pWindow)
WindowPtr pWindow;
{
	register MapPriv *mp;

	ifdebug(3) printf("plxCreateWindow()\n");

	pWindow->ClearToBackground = miClearToBackground;
	pWindow->PaintWindowBackground = plxPaintAreaNone;
	pWindow->PaintWindowBorder = plxPaintAreaPR;
	pWindow->CopyWindow = plxCopyWindow;

	mp = (MapPriv *)Xalloc(sizeof(MapPriv));
	mp->video = 0;
	pWindow->devPrivate = (pointer)mp;

	return TRUE;
}

/*
 * plxCopyWindow copies only the parts of the destination that are
 * visible in the source.
 */

static Bool any_video;

static
plxvideocheckstop(pWindow, garbage)
WindowPtr pWindow;
char *garbage;
{
	register MapPriv *mp = (MapPriv *)pWindow->devPrivate;

	if (mp->video) {
		plxVideoStop(pWindow, (GCPtr) 0);
		any_video = TRUE;
	}
	return WT_WALKCHILDREN;
}

static Bool
plxstopsubvideo(pWindow)
WindowPtr pWindow;
{
	any_video = FALSE;
	TraverseTree(pWindow, plxvideocheckstop, (char *) 0);
	return any_video;
}

static
plxclearonevideo(pWindow, garbage)
WindowPtr pWindow;
char *garbage;
{
	register MapPriv *mp = (MapPriv *)pWindow->devPrivate;

	if (mp->video)
		(*pWindow->ClearToBackground)(pWindow, 0, 0, 0, 0, TRUE);
	return WT_WALKCHILDREN;
}

static
plxclearsubvideo(pWindow)
WindowPtr pWindow;
{
	TraverseTree(pWindow, plxclearonevideo, (char *) 0);
}

void 
plxCopyWindow(pWindow, lastposition, pRegionSrc)
WindowPtr pWindow;
DDXPointRec lastposition;
RegionPtr pRegionSrc;
{
	RegionPtr pRegionDst;
	register BoxPtr pbox;
	register int dx, dy;
	int nbox;
	Bool hasVideo;

	register MapPriv *mp = (MapPriv *)pWindow->devPrivate;

	ifdebug(3) printf("plxCopyWindow()\n");

	pRegionDst = (* pWindow->drawable.pScreen->RegionCreate)(NULL, pWindow->borderClip->numRects);

	dx = lastposition.x - pWindow->absCorner.x;
	dy = lastposition.y - pWindow->absCorner.y;
	(* pWindow->drawable.pScreen->TranslateRegion)(pRegionSrc, -dx, -dy);
	(* pWindow->drawable.pScreen->Intersect)(pRegionDst, pWindow->borderClip, pRegionSrc);

	plxclipinvalidate ();

	p_opaq(0);
	p_rmap(0);
	p_mask(0xffff);

	hasVideo = plxstopsubvideo(pWindow);

	p_damvg ();
	/*
	 * a straight box copy, although source may overlap
	 * destination, so possibly reverse the blit direction
	 * to prevent blit feedback
	 */
	if ((dy > 0) || ((dy == 0) && (dx > 0))) {
		nbox = pRegionDst->numRects;
		pbox = pRegionDst->rects;

		while (--nbox >= 0) {
			p_boxc(pbox->x1 + dx, PTY(pbox->y1 + dy),
				pbox->x1, PTY(pbox->y1),
				pbox->x2 - 1, PTY(pbox->y2-1));
			pbox++;
		}
	} else {
		nbox = pRegionDst->numRects;
		pbox = pRegionDst->rects + nbox;

		while (--nbox >= 0) {
			pbox--;
			p_boxc(pbox->x2 + dx - 1, PTY(pbox->y2 + dy - 1),
				pbox->x2 - 1, PTY(pbox->y2 - 1),
				pbox->x1, PTY(pbox->y1));
		}
	}
	if (hasVideo)
		plxclearsubvideo(pWindow);
	(* pWindow->drawable.pScreen->RegionDestroy)(pRegionDst);
}

/*
 * swap in correct PaintWindow* routine. 
 * These functions are in plxpntwin.c. 
 */
Bool
plxChangeWindowAttributes(pWindow, vmask)
WindowPtr pWindow;
long vmask;
{
	register int idx;
	extern void	plxSaveAreas (), plxRestoreAreas ();

	ifdebug(3) printf("plxChangeWindowAttributes()\n");

	while(vmask) {
		idx = ffs(vmask) -1;
		vmask &= ~(idx = (1 << idx));
		switch(idx) {
		case CWBackingStore:
#ifndef X11R2
			if (pWindow->backingStore != NotUseful) {
				miInitBackingStore(pWindow, plxSaveAreas, plxRestoreAreas);
				plxSwapSaveAndRestoreAreas(pWindow);
			} else {
				miFreeBackingStore(pWindow);
			}
			/*
			 * XXX: The changing of the backing-store status of a
			 * window is serious enough to warrant a validation,
			 * since otherwise the backing-store stuff won't work.
			 */
			pWindow->drawable.serialNumber = NEXT_SERIAL_NUMBER;
#endif /* not X11R2 */
			break;
		case CWBackPixmap:
			switch((int)pWindow->backgroundTile) {
			case None:
				pWindow->PaintWindowBackground = plxPaintAreaNone;
				break;
			case ParentRelative:
				pWindow->PaintWindowBackground = plxPaintAreaPR;
				break;
			default:
				pWindow->PaintWindowBackground = plxPaintArea;
				break;
			}
			break;
		case CWBackPixel:
			pWindow->PaintWindowBackground = plxPaintAreaSolid;
			break;
		case CWBorderPixmap:
			switch((int)pWindow->borderTile) {
			case None:
				pWindow->PaintWindowBorder = plxPaintAreaNone;
				break;
			case ParentRelative:
				pWindow->PaintWindowBorder = plxPaintAreaPR;
				break;
			default:
				pWindow->PaintWindowBorder = plxPaintArea;
				break;
			}
			break;
		case CWBorderPixel:
			pWindow->PaintWindowBorder = plxPaintAreaSolid;
			break;
		}
	}
}

Bool
plxDestroyWindow(pWindow)
WindowPtr pWindow;
{
	register MapPriv *mp = (MapPriv *)pWindow->devPrivate;

	ifdebug(3) printf("plxDestroyWindow()\n");

	if (mp->video) {
		plxVideoStop(pWindow, (GCPtr)0);
	}
	Xfree(pWindow->devPrivate);
#ifndef X11R2
	if (pWindow->backingStore != NotUseful) {
		miFreeBackingStore(pWindow);
	}
#endif /* not X11R2 */
	return(TRUE);
}

Bool plxPositionWindow(pWindow, x, y)
WindowPtr pWindow;
int x, y;
{
	ifdebug(3) printf("plxPositionWindow() x,y = %d,%d\n", x, y);

	/*
	 * deal with rotation of Window's background and border Tile
	 */
	if (IS_VALID_PIXMAP(pWindow->backgroundTile))
		plxrotatepixmap(pWindow->backgroundTile, x, y);
	if (IS_VALID_PIXMAP(pWindow->borderTile))
		plxrotatepixmap(pWindow->borderTile, x, y);
}

Bool plxRealizeWindow(pWindow)
WindowPtr pWindow;
{
	ifdebug(3) printf("plxRealizeWindow()\n");
}

Bool plxUnrealizeWindow(pWindow)
WindowPtr pWindow;
{
	register MapPriv *mp = (MapPriv *)pWindow->devPrivate;

	ifdebug(3) printf("plxUnrealizeWindow()\n");

	if (mp->video) {
		plxVideoStop(pWindow, (GCPtr)0);
	}
}
