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
static char *sid_ = "@(#)plxMouse.c	1.22 07/30/88 Parallax Graphics Inc";
#endif

#define	PARALLAX_QEVENT
#include	"Xplx.h"

#define NEED_EVENTS
#include "Xproto.h"
#include "cursorstr.h"
#include "input.h"

extern int plxGetMotionEvents();
extern void plxChangePointerControl();

extern vsEventQueue *queue;
extern DevicePtr plxPointer;
extern int lastEventTime;

extern int plx_wfd;

/*
 * Cursor/Mouse code.
 */

Bool
plxRealizeCursor(pScreen, pCursor)
ScreenPtr pScreen;
CursorPtr pCursor;
{
	PixmapPtr pPixmap;
	short x, y;

	ifdebug(5) printf("plxRealizeCursor() 0x%08x\n", pCursor);

	plxclipinvalidate();

	pPixmap = (*pScreen->CreatePixmap)(pScreen, pCursor->width, pCursor->height, BITPLANES);
	if (!plxpixmapuse(PIXMAP_WRITE, pPixmap, &x, &y)) {
		ErrorF("plxRealizeCursor: CURSOR AREA NOT IN CACHE\n");
		return FALSE;
	}
	pl_cache_lock(((MapPriv *)(pPixmap->devPrivate))->plxcache);

	p_opaq(0);
	p_rmap(0);
	p_mask(0xff);
	p_box(0, x, y, x+(pCursor->width-1), y-(pCursor->height-1));

	p_rmap(LBIT_RMAP_TABLE);
	p_mask(1);
	p_lbitl(x, y, x+(pCursor->width-1), y-(pCursor->height-1), pCursor->source);
	p_mask(2);
	p_lbitl(x, y, x+(pCursor->width-1), y-(pCursor->height-1), pCursor->mask);

	p_rmap(0);
	p_mask(0xff);

	pCursor->devPriv[pScreen->myNum] = (pointer)pPixmap;

	/* make sure the save area is large enough */
	plxsetsavearea(pCursor->width, pCursor->height);

	return TRUE;
}

Bool
plxUnrealizeCursor(pScreen, pCursor)
ScreenPtr pScreen;
CursorPtr pCursor;
{
	ifdebug(5) printf("plxUnrealizeCursor() 0x%08x\n", pCursor);

	return ((*pScreen->DestroyPixmap)(pCursor->devPriv[pScreen->myNum]));
}

static BYTE mouse_mapping[4] = {
	0, 1, 2, 3,
};

int
plxMouseProc(pDev, onoff, argc, argv)
DevicePtr pDev;
char *argv[];
{
	int fd;

	ifdebug(5) printf("plxMouseProc(), onoff=%d\n", onoff);

	/*
	 * we remove the clip becuase the cursor state on the board
	 * will inherit the clip list
	 */
	plxclipinvalidate();

	switch (onoff) {
	case DEVICE_INIT:
		plxPointer = pDev;
		fd = px_mouse_init();
		pDev->devicePrivate = (pointer)&queue;
		InitPointerDeviceStruct(plxPointer, mouse_mapping, 3, plxGetMotionEvents, plxChangePointerControl);
#if defined(sun) || defined(interactive) || defined(motorola131)
		{ static long c1 = 1, c2 = 2;
			SetInputCheck(&c1, &c2);
		}
#else
		SetInputCheck(&queue->head, &queue->tail);
#endif
		pl_cursor_init();
		pl_cursor_report(1);		/* should this be 1 ? */
		plxsetsavearea(16, 16);
		pl_cursor_color(1, 0);
		break;
	case DEVICE_ON:
		pDev->on = TRUE;
		pl_cursor_active(1);
		AddEnabledDevice(plx_wfd);
		break;
	case DEVICE_OFF:
		pDev->on = FALSE;
		plxsetsavearea(-1, -1);
		pl_cursor_active(0);
		RemoveEnabledDevice(plx_wfd);
		break;
	case DEVICE_CLOSE:
		plxsetsavearea(-1, -1);
		pl_cursor_active(0);
		break;
	}
	return Success;
}

void
plxChangePointerControl(pDevice, ctrl)
DevicePtr pDevice;
PtrCtrl *ctrl;
{
	ifdebug(5) printf("plxChangePointerControl() num,den,threshold=%d,%d,%d\n", ctrl->num, ctrl->den, ctrl->threshold);

	pl_cursor_speed(min(ctrl->num / ctrl->den, 1), ctrl->threshold);
}

int
plxGetMotionEvents(buff, start, stop)
xTimecoord *buff;
CARD32 start, stop;
{
	ifdebug(5) printf("plxGetMotionEvents()\n");

	return (0);
}

Bool
plxSetCursorPosition(pScreen, hotX, hotY, generateEvent)
ScreenPtr pScreen;
unsigned int hotX, hotY;
Bool generateEvent;			/* do we generate a motion event? */
{
	ifdebug(5) printf("plxSetCursorPosition(), x,y=%d,%d\n", hotX, hotY);

	pl_cursor_position(PTX(hotX), PTY(hotY));

	if (generateEvent) {
		register DevicePtr pDev;
		xEvent motion;

		pDev = LookupPointerDevice();
		motion.u.keyButtonPointer.rootX = hotX;
		motion.u.keyButtonPointer.rootY = hotY;
		motion.u.keyButtonPointer.time = lastEventTime;
		motion.u.u.type = MotionNotify;
		(* pDev->processInputProc)(&motion, pDev);
	}
	return TRUE;
}

Pixel
plxgetpixel(pScreen, r, g, b)
ScreenPtr pScreen;				/* Screen to allocate from */
u_short r, g, b;
{
	ColormapPtr pColormap = (ColormapPtr)LookupID(pScreen->defColormap, RT_COLORMAP, RC_CORE);
	Pixel pixelvalue;

	if (!pColormap)
		FatalError("plxgetpixel: Can't get default colormap\n");
	if (AllocColor(pColormap, &r, &g, &b, &pixelvalue, 0))
		FatalError("plxgetpixel: Can't alloc pixel value\n");
	return (pixelvalue);
}

Bool
plxDisplayCursor(pScreen, pCursor)
ScreenPtr pScreen;
CursorPtr pCursor;
{
	int i;
	unsigned short x, y;			/* cursor pattern location */
	Pixel fg, bg;

	ifdebug(5) printf("plxDisplayCursor() 0x%08x\n", pCursor);

	if (!plxpixmapuse(PIXMAP_READ, pCursor->devPriv[pScreen->myNum], &x, &y)) {
		ErrorF("plxRealizeCursor: CURSOR AREA NOT IN CACHE\n");
		return FALSE;
	}
	ifdebug(5) printf("\tx,y,w,h=%d,%d,%d,%d\n", x, y, pCursor->width, pCursor->height);

	plxclipinvalidate();

	pl_cursor_sizes(x, y, pCursor->width, pCursor->height, pCursor->xhot, pCursor->xhot);
#ifdef notdef
	/* done in plxRealizeCursor by hand */
	pl_cursor_new(pCursor->source, pCursor->mask);
#endif

	fg = plxgetpixel(pScreen, pCursor->foreRed, pCursor->foreGreen, pCursor->foreBlue);
	bg = plxgetpixel(pScreen, pCursor->backRed, pCursor->backGreen, pCursor->backBlue);
	pl_cursor_color(fg, bg);

	pl_cursor_active(1);

	return TRUE;
}

void
plxPointerNonInterestBox(pScreen, pBox)
ScreenPtr pScreen;
BoxPtr pBox;
{
	ifdebug(5) printf("plxPointerNonInterestBox(), %d,%d,%d,%d\n",
				pBox->x1, pBox->y1, pBox->x2, pBox->y2);

	pl_cursor_window(
		0,
		PTX(min(pBox->x1, pBox->x2)),
		PTY(max(pBox->y1, pBox->y2)),
		PTX(max(pBox->x1, pBox->x2)),
		PTY(min(pBox->y1, pBox->y2)));
}

void
plxConstrainCursor(pScreen, pBox)
ScreenPtr pScreen;
BoxPtr pBox;
{
	ifdebug(5) printf("plxConstrainCursor(), %d,%d,%d,%d\n",
				pBox->x1, pBox->y1, pBox->x2, pBox->y2);

	pl_cursor_constrain(
		PTX(min(pBox->x1, pBox->x2)),
		PTY(max(pBox->y1, pBox->y2)),
		PTX(max(pBox->x1, pBox->x2)),
		PTY(min(pBox->y1, pBox->y2)));
}

void
plxCursorLimits(pScreen, pCursor, pHotBox, pBox)
ScreenPtr pScreen;
CursorPtr pCursor;
BoxPtr pHotBox;
BoxPtr pBox;				/* return value */
{
	pBox->x1 = max(pHotBox->x1, 0);
	pBox->y1 = max(pHotBox->y1, 0);
	pBox->x2 = min(pHotBox->x2, 1279);
	pBox->y2 = min(pHotBox->y2, 1023);

	ifdebug(5) printf("plxCursorLimits(), %d,%d,%d,%d\n",
				pBox->x1, pBox->y1, pBox->x2, pBox->y2);
}

plxsetsavearea(xsize, ysize)
{
	static plxCache *savearea = (plxCache *)0;
	static int oldxsize = 0, oldysize = 0;

	plxclipinvalidate();

	if (xsize == -1) {
		/* closing down cursor */
		if (savearea) {
			pl_cache_free(savearea, 0);
		}
		oldxsize = 0; oldysize = 0;
		savearea = (plxCache *)0;
		return;
	}
	if ((oldxsize >= xsize) && (oldysize >= ysize))
		return;

	if (savearea) {
		pl_cache_free(savearea, 0);
	}

	savearea = pl_cache_find(xsize, ysize);
	if (!savearea) {
		FatalError("plxMouse: no save area for cursor\n");
		return;
	}
	oldxsize = xsize;
	oldysize = ysize;
	pl_cache_lock(savearea);
	pl_cursor_savearea(savearea->x, savearea->y);
}
