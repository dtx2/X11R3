/* $XConsortium: hpcursor.c,v 1.5 88/09/06 15:26:07 jim Exp $ */
/*
 * hpcursor.c : hp soft cursor routines
 * C Durland, C Amacher
 */

/*
Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.
*/

#define NEED_EVENTS	/* hack hack hack */

#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "cfb.h"
#include "resource.h"
#include "input.h"
#include "sun.h"
#include "XHPproto.h"

DevicePtr LookupPointerDevice();

	/* cstate flags */
#define CURSOR_OFF 0
#define CURSOR_ON  1

#define MAXCX 64	/* max cursor rectangle width */
#define MAXCY 64	/* max cursor rectangle height */

#ifdef XTESTEXT1
/*
 * defined in xtestext1di.c
 */
extern int	on_steal_input;
/*
 * defined in xtestext1di.c
 */
extern short	xtest_mousex;
/*
 * defined in xtestext1di.c
 */
extern short	xtest_mousey;
#endif /* XTESTEXT1 */

	/* restore screen from off screen mem */
void hpCursorOff(pScreen) ScreenPtr pScreen;
{
  register cfbPrivScreenPtr cfb = (cfbPrivScreenPtr)(pScreen->devPrivate);

  if (cfb->cstate == CURSOR_OFF) return;

  cfb->cstate = CURSOR_OFF;
  (*cfb->MoveBits)(pScreen, ~0, GXcopy,
	cfb->ssaveX,cfb->ssaveY, cfb->X,cfb->Y, cfb->w,cfb->h);
}

	/* copy area cursor covers to off screen mem, copy cursor to screen */
void hpCursorOn(pScreen,dx,dy)
ScreenPtr pScreen;
register int dx,dy;	/* the cursor hot spot */
{
  register cfbPrivScreenPtr cfb = (cfbPrivScreenPtr)(pScreen->devPrivate);
  register int
    srcx = cfb->srcX, srcy = cfb->srcY,
    maskx = cfb->maskX, masky = cfb->maskY,
    w,h;

  if (cfb->cstate ==CURSOR_ON) return;		/* else cursor is offscreen */
  dx -= cfb->hoffX; dy -= cfb->hoffY;	/* hotspot to upper left corner */
		/* clip cursor rectangle */
  w = min(cfb->width,  pScreen->width -dx);
  h = min(cfb->height, pScreen->height -dy);
  if (dx<0) { srcx -= dx; maskx -= dx; w += dx; dx = 0; }
  if (dy<0) { srcy -= dy; masky -= dy; h += dy; dy = 0; }

  (*cfb->MoveBits)(pScreen, ~0, GXcopy,	/* save screen area */
	dx,dy, cfb->ssaveX,cfb->ssaveY, w,h);
  cfb->X = dx; cfb->Y = dy; cfb->w = w; cfb->h = h;

	/* mask out cursor shape, put cursor in hole */
  (*cfb->MoveBits)(pScreen, ~0, GXand,	maskx,masky, dx,dy, w,h);
  (*cfb->MoveBits)(pScreen, ~0, GXor,	srcx,srcy, dx,dy, w,h);

  cfb->cstate = CURSOR_ON;	/* cursor is on screen */
}

	/* move screen cursor hotspot to (hotX,hotY) from wherever it is */
void hpMoveMouse(pScreen, hotX,hotY, forceit)
ScreenPtr pScreen; register int hotX, hotY;
{
#ifdef XTESTEXT1
	if (on_steal_input)
	{
		/*
		 * only call if the mouse position has actually moved
		 */
		if ((hotX != xtest_mousex) || (hotY != xtest_mousey))
		{
			XTestStealMotionData((hotX - xtest_mousex),
					     (hotY - xtest_mousey),
					     MOUSE,
					     xtest_mousex,
					     xtest_mousey);
		}
	}
#endif /* XTESTEXT1 */

  if (forceit) hpCursorOff(pScreen);
  if (0<=hotX && 0<=hotY && hotX<pScreen->width && hotY<pScreen->height)
	hpCursorOn(pScreen, hotX,hotY);
}

/*
 * sunGetPixel --
 *	Given an rgb value, find an equivalent pixel value for it.
 *	!!! - this is garbage - re-implement
 * Side Effects:
 *	A colormap entry might be allocated...
 */
Pixel hpGetPixel(pScreen, r,g,b)
  ScreenPtr pScreen;
  unsigned short r,g,b;    /* Red, Green, Blue */
{
    ColormapPtr
      cmap = (ColormapPtr)LookupID(pScreen->defColormap,RT_COLORMAP,RC_CORE);
    Pixel pix = 0;

    if (!cmap) FatalError("Can't find default colormap in sunGetPixel\n");
    if (AllocColor(cmap, &r, &g, &b, &pix, 0))
	FatalError("Can't alloc pixel (%d,%d,%d) in sunGetPixel\n");
    return pix;
}

#define tcXY(x,y) /* address of offscreen mem */ \
	(unsigned char *)(cfb->bits +(y)*cfb->stride +(x))

Bool hpDisplayCursor(pScreen,pCursor) ScreenPtr pScreen; CursorPtr pCursor;
{
  extern	PtrPrivRec	*other_p[];
  register cfbPrivScreenPtr cfb = (cfbPrivScreenPtr)(pScreen->devPrivate);
  register unsigned char *ctr, *mtr, *ptr, *qtr, z, startbit;
  register short int x,y, w,h;
  int xstart, ystart;
  Pixel fgcolor, bgcolor;

  w = pCursor->width; h = pCursor->height;
	/* scale cursor if bigger than max */
  cfb->width = min(w,MAXCX); cfb->height = min(h,MAXCY);
  xstart = (w<=MAXCX) ? 0 : pCursor->xhot -MAXCX +((w -pCursor->xhot)*MAXCX)/w;
  startbit = 0x80>>(xstart%8); cfb->hoffX = pCursor->xhot -xstart; xstart /= 8;
  ystart = (h<=MAXCY) ? 0 : pCursor->yhot -MAXCY +((h -pCursor->yhot)*MAXCY)/h;
  cfb->hoffY = pCursor->yhot -ystart;

	/* convert colors to display specific colors */
  fgcolor = hpGetPixel(pScreen, pCursor->foreRed,
			pCursor->foreGreen, pCursor->foreBlue);
  bgcolor = hpGetPixel(pScreen, pCursor->backRed,
			pCursor->backGreen,pCursor->backBlue);
  SET_REGISTERS_FOR_WRITING(pScreen,~0,GXcopy);	  /* setup hardware */
	/* bytes in each row of pixmaps (including padding) */
  w = (w+BITMAP_SCANLINE_PAD-1)/BITMAP_SCANLINE_PAD*(BITMAP_SCANLINE_PAD/8);
  for (y=0; y<cfb->height; y++)
  {
    ptr = pCursor->source +w*(ystart+y) +xstart;
    qtr = pCursor->mask +w*(ystart+y) +xstart;
    ctr = tcXY(cfb->srcX, y+cfb->srcY);	/* address of offscreen cursor */
    mtr = tcXY(cfb->maskX,y+cfb->maskY); /* address of offscreen cursor mask */
    for (z = startbit, x=0; x<cfb->width; x++)
    {
	/* cursor mask: all planes = xor(or(all planes)) */
      *mtr++ = (*qtr & z) ? 0 : 0xFF;
	/* squeeze cursor through mask */
      *ctr++ = (*qtr & z) ? ((*ptr & z) ? fgcolor : bgcolor) : 0;
      if ((z>>=1)==0) { z = 0x80; ptr++; qtr++; }
    }
  }
  hpMoveMouse(pScreen, other_p[XPOINTER]->x,other_p[XPOINTER]->y, 1);
  return TRUE;
}

void hpDoNothing() {  }	/* one of the more exciting routines */

extern hpChunk *hpBufAlloc();

Bool hpInitCursor(pScreen) ScreenPtr pScreen;
{
  register cfbPrivScreenPtr cfb = (cfbPrivScreenPtr)(pScreen->devPrivate);
  hpChunk *chunkie;

  cfb->MoveMouse = hpMoveMouse; cfb->CursorOff = hpCursorOff;
  cfb->cstate = CURSOR_OFF;
	/* allocate 3 max size cursor retangles: 
	 * cursor, cursor mask and screen save area
	 */
  chunkie = hpBufAlloc(pScreen, MAXCX,MAXCY);		/* save area */
  cfb->ssaveX = chunkie->x; cfb->ssaveY = chunkie->y;
  chunkie = hpBufAlloc(pScreen, MAXCX,MAXCY);		/* cursor */
  cfb->srcX = chunkie->x; cfb->srcY = chunkie->y;
  chunkie = hpBufAlloc(pScreen, MAXCX,MAXCY);		/* mask */
  cfb->maskX = chunkie->x; cfb->maskY = chunkie->y;

	/* create a dummy cursor to avoid trashing screen */
  cfb->hoffX = 0; cfb->hoffY = 0; cfb->width = 1; cfb->height = 1;

  return TRUE;
}

Bool hpRealizeCursor(pScreen,pCursor)
ScreenPtr pScreen; CursorPtr pCursor;
{
  return TRUE;
}

Bool hpUnrealizeCursor(pScreen,pCursor)
ScreenPtr pScreen; CursorPtr pCursor;
{
  return TRUE;
}

/*-
 * hpSetCursorPosition --
 *	Alter the position of the current cursor. The x and y coordinates
 *	are assumed to be legal for the given screen.
 *
 * Side Effects:
 *	The pScreen, x, and y fields of the current pointer's private data
 *	are altered to reflect the new values. I.e. moving the cursor to
 *	a different screen moves the pointer there as well. Helpful...
 *
 */
Bool hpSetCursorPosition(pScreen, hotX,hotY, generateEvent)
  register ScreenPtr pScreen;  	/* New screen for the cursor */
  unsigned int hotX, hotY;    	/* New absolute x,y coordinates for cursor */
  Bool generateEvent;	/* whether we generate a motion event */
{
  register cfbPrivScreenPtr cfb = (cfbPrivScreenPtr)(pScreen->devPrivate);
  DevicePtr pDev;
  PtrPrivPtr pptrPriv;
  xEvent motion;

  (*cfb->MoveMouse)(pScreen,hotX,hotY,1);

  pDev = LookupPointerDevice();
  pptrPriv = (PtrPrivPtr)pDev->devicePrivate;

  pptrPriv->pScreen = pScreen; pptrPriv->x = hotX; pptrPriv->y = hotY;

  if (generateEvent)
  {
    motion.u.keyButtonPointer.rootX = hotX;
    motion.u.keyButtonPointer.rootY = hotY;
    motion.u.keyButtonPointer.time = lastEventTime;
    motion.u.u.type = MotionNotify;
    (*pDev->processInputProc)(&motion, pDev);
  }
  return TRUE;
}

/*
 * hpCursorLimits --
 *	Return a box within which the given cursor may move on the given
 *	screen. We assume that the HotBox is actually on the given screen,
 *	since dix knows that size.
 *
 * Results:
 *	A box for the hot spot corner of the cursor.
 */
void hpCursorLimits(pScreen, pCursor, pHotBox, pResultBox)
  ScreenPtr pScreen;  	/* Screen on which limits are desired */
  CursorPtr pCursor;  	/* Cursor whose limits are desired */
  BoxPtr pHotBox,  	/* Limits for pCursor's hot point */
    pResultBox;		/* RETURN: limits for hot spot */
{
  *pResultBox = *pHotBox;
  pResultBox->x2 = min(pResultBox->x2,pScreen->width);
  pResultBox->y2 = min(pResultBox->y2,pScreen->height);
}

	/* hpRecolorCursor -- Change the color of a cursor
	 * Do this by creating a new cursor that has the new colors
	 */
void hpRecolorCursor(pScreen, pCursor, displayed)
  ScreenPtr pScreen;  	/* Screen for which the cursor is to be recolored */
  CursorPtr pCursor;  	/* Cursor to recolor */
  Bool displayed;	/* True if pCursor being displayed now */
{
  if (displayed) hpDisplayCursor(pScreen,pCursor);
}

/*
 * hpPointerNonInterestBox --
 *	Set up things to ignore uninteresting mouse events. Sorry.
 */
void hpPointerNonInterestBox(pScreen,pBox)
  ScreenPtr pScreen;
  BoxPtr    pBox;	    /* Box outside of which motions are boring */
{
}

/* static */	/* shared with sunCursor.c -- yukk!!! */
BoxRec currentLimits;	/* Box w/in which the hot spot must stay */

/*
 * hpConstrainCursor --
 *	Make it so the current pointer doesn't let the cursor's
 *      hot-spot wander out of the specified box.
 */
void hpConstrainCursor(pScreen,pBox)
  ScreenPtr pScreen;  	/* Screen to which it should be constrained */
  BoxPtr   pBox;	/* Box in which... */
{
  currentLimits = *pBox;
}

/*
 * sunConstrainXY --
 *   Given an X and Y coordinate, alter them to fit within the current
 *	cursor constraints.  Used by mouse processing code to adjust 
 * 	position reported by hardware.
 *
 * Results:
 *   The new constrained coordinates. Returns FALSE if the motion
 *	event should be discarded.
 */
Bool hpConstrainXY(px,py) short int *px, *py;
{
  *px = max(currentLimits.x1, min(currentLimits.x2 -1,*px));
  *py = max(currentLimits.y1, min(currentLimits.y2 -1,*py));
  return TRUE;
}

/*-
 *-----------------------------------------------------------------------
 * hpCursorLoc --
 *  If the current cursor is both on and in the given screen,
 *    fill in the given BoxRec with the extent of the cursor and return
 *    TRUE. If the cursor is either on a different screen or not
 *    currently in the frame buffer, return FALSE.
 *-----------------------------------------------------------------------
 */
Bool hpCursorLoc(pScreen, pBox)
  ScreenPtr	  pScreen;  	/* Affected screen */
  register BoxRec *pBox;	/* Box in which to place the limits */
{
  register cfbPrivScreenPtr cfb = (cfbPrivScreenPtr)(pScreen->devPrivate);
  DevicePtr	  pDev;
  PtrPrivPtr	  pptrPriv;

  pDev = LookupPointerDevice();
  pptrPriv = (PtrPrivPtr) pDev->devicePrivate;

  if (pptrPriv->pScreen==pScreen && cfb->cstate==CURSOR_ON)
  {
	/*
	 * If the cursor is actually on the same screen, stuff the cursor's
	 * limits in the given BoxRec. Perhaps this should be done when
	 * the cursor is moved? Probably not important...
	 */
    pBox->x1 = cfb->X; pBox->x2 = cfb->X +cfb->w;
    pBox->y1 = cfb->Y; pBox->y2 = cfb->Y +cfb->h;
    return TRUE;
  }
  return FALSE;
}
