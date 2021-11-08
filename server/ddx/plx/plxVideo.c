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
static char *sid_ = "@(#)plxVideo.c	1.17 09/01/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

#include "plxvideo.h"

#if defined(sun) || defined(is68k) || defined(motorola131) || defined(hpux)
#define WE_HAVE_VME_BUS
#include	"plxvme.h"
#endif

plxCache *plxvideosa = (plxCache *)0;
WindowPtr plxvideolivewindow = (WindowPtr)0;

#define	VIDEO_MAX_WIDTH		640
#define	VIDEO_MAX_HEIGHT	482

#define	VIDEO_NO		0
#define	VIDEO_STILL		1
#define	VIDEO_LIVE		2

/*
 * this is a copy of the current video state, move it into a GC
 * sometime
 */
struct plxvideomodes {
	int framemode;		/* frame or field */
	int framestart;		/* even, odd or dont care */
	struct {
		int luma;	/* dither control */
		int chroma;
	} dither;
	int input;		/* input 1 or input 2 */
	int priority;		/* 0,1,2,3 relative pri gived to video */
	short vparam[6];	/* sat, cont, lgain, cgain, hue, brit */
} plxvideomodes;

extern GCPtr CreateScratchGC();

plxVideoProc(pDrawable, pGC, videoReqType, xva)
register DrawablePtr pDrawable;
register GCPtr pGC;
register xVideoArgs *xva;
{
	short xorg, yorg;
	Bool goingtodraw = FALSE;
	BoxRec r;

	ifdebug(15) printf("plxVideoProc() request=%d\n", videoReqType);

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
			ErrorF("plxVideo: PIXMAP NOT IN CACHE\n");
			return (Success);
		}
		yorg = PTY(yorg);
		break;
	}

	switch (videoReqType) {
	case X_VideoSet:
		goingtodraw = TRUE;
		break;
	case X_VideoLive:
	case X_VideoStill:
		goingtodraw = TRUE;
		xva->vw = VIDEO_MAX_WIDTH;
		xva->vh = VIDEO_MAX_HEIGHT;
		if (xva->w > VIDEO_MAX_WIDTH)
			xva->w = VIDEO_MAX_WIDTH;
		if (xva->h > VIDEO_MAX_HEIGHT)
			xva->h = VIDEO_MAX_HEIGHT;

		break;
	case X_VideoScale:
		goingtodraw = TRUE;
		break;
	case X_VideoStop:
		break;
	case X_VideoFrameMode:
	case X_VideoFrameStart:
	case X_VideoDitherControl:
	case X_VideoPriority:
	case X_VideoInput:
		break;
	}

	if (goingtodraw) {
		xva->x += xorg;
		xva->y += yorg;

		/* this region is used for the video clip */
		r.x1 = xva->x;
		r.y1 = xva->y;
		r.x2 = xva->x + xva->w;
		r.y2 = xva->y + xva->h;

		/* reset xorg & yorg for clipping */
		if (pDrawable->type == DRAWABLE_WINDOW) {
			xorg = yorg = 0;
			/*
			 * for video operations, we adjust the clip before
			 * setting it in the the board.
			 * the reason is that the clip should be rounded
			 * to mod16
			 */
			if (!plxvclipdownload(pDrawable, pGC, &r))
				return (Success);
		} else {
			if (!plxclipdownload(pDrawable, pGC, xorg, yorg))
				return (Success);
		}

		plxMask(pDrawable, pGC);
	}

	switch (videoReqType) {
	case X_VideoSet:
		/* plxvideoforce(pDrawable, pGC, xorg, yorg); */
		return (Success);
		break;
	case X_VideoLive:
		plxvideoroundup(xva);
		return (plxVideoLive(pDrawable, pGC, xva->vx, xva->vy, xva->x, xva->y, xva->w, xva->h));
		break;
	case X_VideoStill:
		plxvideoroundup(xva);
		return (plxVideoStill(pDrawable, pGC, xva->vx, xva->vy, xva->x, xva->y, xva->w, xva->h));
		break;
	case X_VideoScale:
		plxvideoroundup(xva);
		return (plxVideoScale(pDrawable, pGC, xva->vx, xva->vy, xva->vw, xva->vh, xva->x, xva->y, xva->w, xva->h));
		break;
	case X_VideoStop:
		return (plxVideoStop(pDrawable, pGC));
		break;
	case X_VideoFrameMode:
		/* this should be a GC saved value */
		plxvideomodes.framemode = xva->xva_arg1;
		break;
	case X_VideoFrameStart:
		/* this should be a GC saved value */
		plxvideomodes.framestart = xva->xva_arg1;
		break;
	case X_VideoDitherControl:
		/* this should be a GC saved value */
		plxvideomodes.dither.luma = xva->xva_arg1;
		plxvideomodes.dither.chroma = xva->xva_arg2;
		if (plxvideomodes.dither.luma) {
			if (plxvideomodes.dither.chroma) {
				p_dthrlc();
			} else {
				p_dthrl();
			}
		} else {
			if (plxvideomodes.dither.chroma) {
				p_dthrc();
			} else {
				p_dthroff();
			}
		}
		break;
	case X_VideoPriority:
		/* this should be a GC saved value */
		plxvideomodes.priority = 3 - ((3 * xva->xva_arg1 + 49) / 100);
		break;
#ifdef WE_HAVE_VME_BUS
	case X_VideoInput:
		/* this should be a GC saved value */
		plxvideomodes.input = xva->xva_arg1;
		switch (plxvideomodes.input) {
		case 1:
			p_vid1();
			break;
		case 2:
			p_vid2();
			break;
		default:
			return (BadValue);
			break;
		}
		break;
#endif /* WE_HAVE_VME_BUS */
	}
	return (BadRequest);
}

plxVideoInit(pScreen)
{
	GCPtr pGC = CreateScratchGC(pScreen, 1);

	if (pGC) {
		RegisterProc(VIDEOPROCEDURES, pGC, plxVideoProc);
		FreeScratchGC(pGC);
	}

	plxvideolivewindow = (WindowPtr)0;

	/*
	 * find the scale video cache flash area
	 */
	plxvideosa = pl_cache_find(VIDEO_MAX_WIDTH, VIDEO_MAX_HEIGHT);
	if (!plxvideosa) {
		ErrorF("plxVideoInit: can't get video scale area\n");
		return (BadRequest);
	}
	pl_cache_lock(plxvideosa);

	/*
	 * set video defaults for board
	 */
	plxvideomodes.framemode = 0;		/* frame */
	plxvideomodes.framestart = 1;		/* even */
	plxvideomodes.dither.luma = 0;
	plxvideomodes.dither.chroma = 1;
	plxvideomodes.input = 1;
	plxvideomodes.priority = 3;

	p_dthron();
	p_flsp(plxvideomodes.priority);
#ifdef WE_HAVE_VME_BUS
	p_vid1();
/*
	p_vpu(&(plxvideomodes.vparam));
*/
	p_vpu(plxvideomodes.vparam);
#endif

	return (Success);
}

plxVideoLive(pDrawable, pGC, vx, vy, x, y, w, h)
DrawablePtr pDrawable;
GCPtr pGC;
{
	register MapPriv *mp;

	ifdebug(15) printf("plxVideoLive() vx,vy=%d,%d x,y,w,h=%d,%d,%d,%d\n", vx, vy, x, y, w, h);

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		{
			register WindowPtr pWindow = (WindowPtr)pDrawable;

			mp = (MapPriv *)pWindow->devPrivate;
		}
		break;
	case DRAWABLE_PIXMAP:
		return (BadValue);
		break;
	}

	if (plxvideolivewindow) {
		/* we do a proxy video stop on the other window */
		(void)plxVideoStop(plxvideolivewindow, pGC);
	}

	plxvideolivewindow = (WindowPtr) pDrawable;
	mp->video = VIDEO_LIVE;

	pl_cursor_active(0);

	p_flsp(plxvideomodes.priority);

	/* clear overlay graphics plane */
	p_mask(0x01);
	CLIPREG(p_box(0, x, PTY(y), (x+w-1), PTY(y+h-1)));

	/* start live video in other planes */
	p_mask(0xfe);
	px_flush();
	switch (plxvideomodes.framemode) {
	case 0:			/* frame capture */
		switch (plxvideomodes.framestart) {
		case 0:		/* any field first */
			CLIPREG(p_flc(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 1:		/* even field first */
			CLIPREG(p_flec(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 2:		/* odd field first */
			CLIPREG(p_floc(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		}
		break;
	case 1:			/* field capture */
		switch (plxvideomodes.framestart) {
		case 0:
			CLIPREG(p_flfc(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 1:
			CLIPREG(p_flfec(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 2:
			CLIPREG(p_flfoc(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		}
		break;
	}
	px_flush();

	p_mask(0xffff);
	pl_cursor_active(1);

	return (Success);
}

plxVideoStill(pDrawable, pGC, vx, vy, x, y, w, h)
DrawablePtr pDrawable;
GCPtr pGC;
{
	register MapPriv *mp;

	ifdebug(15) printf("plxVideoStill() vx,vy=%d,%d x,y,w,h=%d,%d,%d,%d\n", vx, vy, x, y, w, h);

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		{
			register WindowPtr pWindow = (WindowPtr)pDrawable;

			mp = (MapPriv *)pWindow->devPrivate;
		}
		break;
	case DRAWABLE_PIXMAP:
		{
			register PixmapPtr pPixmap = (PixmapPtr)pDrawable;

			mp = (MapPriv *)pPixmap->devPrivate;
		}
		break;
	}

	/* we do a proxy video stop on this window in case its live */
	(void)plxVideoStop((WindowPtr)pDrawable, pGC);

	mp->video = VIDEO_STILL;

	pl_cursor_active(0);

	p_mask(0xff);
	px_flush();
	switch (plxvideomodes.framemode) {
	case 0:			/* frame capture */
		switch (plxvideomodes.framestart) {
		case 0:
			CLIPREG(p_fl(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 1:
			CLIPREG(p_fle(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 2:
			CLIPREG(p_flo(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		}
		break;
	case 1:			/* field capture */
		switch (plxvideomodes.framestart) {
		case 0:
			CLIPREG(p_flf(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 1:
			CLIPREG(p_flfe(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		case 2:
			CLIPREG(p_flfo(vx, 0x1e1 - vy, x, PTY(y), (x+w-1), PTY(y+h-1)));
			break;
		}
		break;
	}
	px_flush();

	p_mask(0xffff);
	pl_cursor_active(1);

	return (Success);
}

plxVideoScale(pDrawable, pGC, vx, vy, vw, vh, x, y, w, h)
DrawablePtr pDrawable;
GCPtr pGC;
{
	register MapPriv *mp;

	ifdebug(15) printf("plxVideoScale() vx,vy,vw,vh=%d,%d,%d,%d x,y,w,h=%d,%d,%d,%d\n", vx, vy, vw, vh, x, y, w, h);

	if (((vw/w) >= 256) || ((w/vw) >= 8)) {
		ErrorF("plxVideoScale: video scale overflow\n");
		return (BadValue);
	}

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		{
			register WindowPtr pWindow = (WindowPtr)pDrawable;

			mp = (MapPriv *)pWindow->devPrivate;
		}
		break;
	case DRAWABLE_PIXMAP:
		{
			register PixmapPtr pPixmap = (PixmapPtr)pDrawable;

			mp = (MapPriv *)pPixmap->devPrivate;
		}
		break;
	}

	/* we do a proxy video stop on this window in case its live */
	(void)plxVideoStop((WindowPtr)pDrawable, pGC);

	mp->video = VIDEO_STILL;

	/*
	 * flash into an off screen area
	 */
	p_clipd();			/* REMEMBER TO TURN ON AGAIN */

	p_mask(0xff);
	px_flush();
	switch (plxvideomodes.framemode) {
	case 0:			/* frame capture */
		switch (plxvideomodes.framestart) {
		case 0:
			p_fl(vx, 0x1e1 - vy,
				plxvideosa->x, plxvideosa->y,
				plxvideosa->x+vw-1, plxvideosa->y-(vh-1));
			break;
		case 1:
			p_fle(vx, 0x1e1 - vy,
				plxvideosa->x, plxvideosa->y,
				plxvideosa->x+vw-1, plxvideosa->y-(vh-1));
			break;
		case 2:
			p_flo(vx, 0x1e1 - vy,
				plxvideosa->x, plxvideosa->y,
				plxvideosa->x+vw-1, plxvideosa->y-(vh-1));
			break;
		}
		break;
	case 1:			/* field capture */
		switch (plxvideomodes.framestart) {
		case 0:
			p_flf(vx, 0x1e1 - vy,
				plxvideosa->x, plxvideosa->y,
				plxvideosa->x+vw-1, plxvideosa->y-(vh-1));
			break;
		case 1:
			p_flfe(vx, 0x1e1 - vy,
				plxvideosa->x, plxvideosa->y,
				plxvideosa->x+vw-1, plxvideosa->y-(vh-1));
			break;
		case 2:
			p_flfo(vx, 0x1e1 - vy,
				plxvideosa->x, plxvideosa->y,
				plxvideosa->x+vw-1, plxvideosa->y-(vh-1));
			break;
		}
		break;
	}
	px_flush();

	p_clipe();			/* WE REMEMBERED */

	p_damvv();
	CLIPREG(p_boxzv(plxvideosa->x, plxvideosa->y,
			plxvideosa->x+vw-1, plxvideosa->y-(vh-1),
			x, PTY(y),
			x+w-1, PTY(y+h-1)));

	p_mask(0xffff);
	return (Success);
}

plxVideoStop(pDrawable, pGC)
DrawablePtr pDrawable;
GCPtr pGC;
{
	register MapPriv *mp;

	ifdebug(15) printf("plxVideoStop()\n");

	if (!plxvideolivewindow || ((WindowPtr) pDrawable != plxvideolivewindow))
		return (Success);		/* always a success */

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		{
			register WindowPtr pWindow = (WindowPtr)pDrawable;

			mp = (MapPriv *)pWindow->devPrivate;
		}
		break;
	case DRAWABLE_PIXMAP:
		return (BadValue);
		break;
	}

	p_floff();
	plxvideolivewindow = (WindowPtr)0;
	mp->video = VIDEO_STILL;

	return (Success);
}

/*
 * plxvideoforce is used to allow pixmaps to be saved and restored
 * in video format. the whole window is placed in dam (video) mode, 
 * and as the pixmap is laoded with p_damvx() in operation, the video
 * will stay on
 */
plxvideoforce(pDrawable, pGC, xorg, yorg)
DrawablePtr pDrawable;
GCPtr pGC;
short xorg, yorg;
{
	register MapPriv *mp;
	register BoxPtr pbox, pboxLast;
	register RegionPtr pRegion;

	return;

#ifdef notdef
	ifdebug(15) printf("plxvideoforce()\n");

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		{
			register WindowPtr pWindow = (WindowPtr)pDrawable;

			mp = (MapPriv *)pWindow->devPrivate;
		}
		break;
	case DRAWABLE_PIXMAP:
		{
			register PixmapPtr pPixmap = (PixmapPtr)pDrawable;

			mp = (MapPriv *)pPixmap->devPrivate;
		}
		break;
	}

	if (mp->video)
		return;

	mp->video = VIDEO_STILL;
	/*
	 * now turn on the dam bits for the whole window
	 */
	p_damvv();
	p_mask(0);	/* only draw into the DAM in video mode */

	pRegion = ((plxPrivGC *)pGC->devPriv)->pCompositeClip;
	pbox = pRegion->rects;
	pboxLast = pbox + pRegion->numRects;
	for (;pbox<pboxLast;pbox++) {
		ifdebug(15) printf("\t%d,%d,%d,%d\n", pbox->x1, pbox->y1, pbox->x2-1, pbox->y2-1);
		CLIPREG(p_box(0,
				ROUNDUP16(pbox->x1 + xorg),
				PTY(pbox->y1 + yorg-1),
				ROUNDDN16(pbox->x2 + xorg)-1,
				PTY(pbox->y2 + yorg-1)));
	}
	p_mask(0xffff);
#endif
}

plxvideoroundup(xva)
register xVideoArgs *xva;
{
	ifdebug(15) printf("plxvideoroundup(i) vx,vy,vw,vh=%d,%d,%d,%d x,y,w,h=%d,%d,%d,%d\n", xva->vx, xva->vy, xva->vw, xva->vh, xva->x, xva->y, xva->w, xva->h);

	/*
	 * deal with the boards video display size
	 */
	if ((xva->vx + xva->vw) > VIDEO_MAX_WIDTH)
		xva->vw = VIDEO_MAX_WIDTH - xva->vx;
	if ((xva->vy + xva->vh) > VIDEO_MAX_HEIGHT)
		xva->vh = VIDEO_MAX_HEIGHT - xva->vy;

	if (!ISROUND16(xva->x)) {
		int newx;

		newx = ROUNDUP16(xva->x);
		xva->w -= newx - xva->x;
		xva->x = newx;
	}
	xva->w = ROUNDDN16(xva->w);

	ifdebug(15) printf("plxvideoroundup(o) vx,vy,vw,vh=%d,%d,%d,%d x,y,w,h=%d,%d,%d,%d\n", xva->vx, xva->vy, xva->vw, xva->vh, xva->x, xva->y, xva->w, xva->h);
}

plxvclipdownload(pWindow, pGC, pRect)
WindowPtr pWindow;
GCPtr pGC;
BoxPtr pRect;
{
	register RegionPtr pRegion1, pRegion2, pRegionClip;
	register ScreenPtr pScreen;
	register BoxPtr pbox;
	int rval, nbox, tmp;
	register MapPriv *mp;

	mp = (MapPriv *)pWindow->devPrivate;

	ifdebug(15) printf("plxvclipdownload() r=%d,%d,%d,%d\n",
			pRect->x1,
			pRect->y1,
			pRect->x2,
			pRect->y2);

	pRegionClip = ((plxPrivGC *)pGC->devPriv)->pCompositeClip;

	pScreen = pWindow->drawable.pScreen;

	pRegion1 = (* pScreen->RegionCreate)(NULL, pRegionClip->numRects);
	(* pScreen->RegionCopy)(pRegion1, pRegionClip);
	pRegion2 = (* pScreen->RegionCreate)(pRect, 1);
	(* pScreen->Intersect)(pRegion2, pRegion1, pRegion2);
	/* region2 contains the area to be "painted" */
	(* pScreen->RegionCopy)(pRegion1, pRegion2);

	pbox = pRegion2->rects;
	nbox = pRegion2->numRects;

	while (--nbox >= 0) {
		pbox->x1 = ROUNDUP16(pbox->x1);
		if (!ISROUND16(pbox->x2 - 1 + 1)) {
			pbox->x2 = ROUNDDN16(pbox->x2);
		}
		if (pbox->x1 > pbox->x2)
			pbox->x1 = pbox->x2;		/* XXX */
		ifdebug(15) printf("\t x1,y1,x2,y2=%d,%d,%d,%d\n",
				pbox->x1, pbox->y1,
				pbox->x2, pbox->y2);
		pbox++;
	}
	/* region2 now follows the parallax rules */

	(* pScreen->Subtract)(pRegion1, pRegion1, pRegion2);
	/* region1 now contains the parts that are not to be painted */
	tmp = mp->video;
	mp->video = VIDEO_NO;
	pWindow->PaintWindowBackground(pWindow, pRegion1, PW_BACKGROUND);
	mp->video = tmp;

	plxclipinvalidate();
	rval = plxclipdownloadregion(pRegion2, 0, 0);

	(* pScreen->RegionDestroy)(pRegion1);
	(* pScreen->RegionDestroy)(pRegion2);

	return (rval);
}

void
plxVideoReshape(pWindow, dx, dy)
WindowPtr pWindow;
int	dx, dy;
{

	if (plxvideolivewindow != pWindow)
		return;

	p_floff();
	plxvideolivewindow = (WindowPtr) 0;

	plxclipinvalidate();

	/*
	 * only do this if the window hasn't been moved --
	 * when it's moved, plxCopyWindow will deal with
	 * the problem there
	 */
	if (dx == 0 && dy == 0) {
		/*
		 * this will force the client to start the video again
	 	 */
		(*pWindow->ClearToBackground) (pWindow, 0, 0, 0, 0, TRUE);
	}
}
