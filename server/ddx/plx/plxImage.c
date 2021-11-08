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
static char *sid_ = "@(#)plxImage.c	1.13 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

void
plxGetImage(pDrawable, xin, yin, w, h, format, planeMask, pImage)
DrawablePtr pDrawable;
int xin, yin, w, h;
long format;
unsigned long planeMask;
unsigned long *pImage;
{
	short xorg, yorg;
	register int x, y;

	ifdebug(10) printf("plxGetImage() 0x%08x, x,y,w,h=%d,%d,%d,%d mask=0x%08x format=%d depth=%d\n", pDrawable, xin, yin, w, h, planeMask, format, pDrawable->depth);

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		if (/*pGC->miTranslate*/ 1) {
			xorg = ((WindowPtr)pDrawable)->absCorner.x;
			yorg = ((WindowPtr)pDrawable)->absCorner.y;
		} else {
			xorg = yorg = 0;
		}
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_READ, pDrawable, &xorg, &yorg)) {
			ErrorF("plxGetImage: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
		break;
	}

	x = xin + xorg;
	y = yin + yorg;

	switch (format) {
	case XYBitmap:
		p_opaq(UBIT_OPAQ_TABLE);
		p_opaqm(planeMask, planeMask);
		p_ubitl(x, PTY(y), x+w-1, PTY(y+h-1), pImage);
		p_opaqm(1, 1);
		break;
	case XYPixmap:
		switch (pDrawable->depth) {
		case 1:
			p_opaq(UBIT_OPAQ_TABLE);
#ifdef notdef
			p_opaqm(planeMask, planeMask);
#endif
			p_ubitl(x, PTY(y), x+w-1, PTY(y+h-1), pImage);
			break;
		case 8:
			if ((planeMask & 0xff) == 0xff) {
				register int i, bump;
				register unsigned long *p;

				p = pImage;
				bump = h * PixmapWidthInPadUnits(w, pDrawable->depth);

				for (i=0;i<8;i++) {
					p_opaq(UBIT_OPAQ_TABLE);
					p_opaqm(1<<i, 1<<i);
					p_ubitl(x, PTY(y), x+w-1, PTY(y+h-1), p);
					p += bump;
				}
				p_opaqm(1, 1);
			} else {
				ifdebug(10) printf("plxGetImage() XYPixmap & bad planemask, using miGetImage()\n");
				miGetImage(pDrawable, xin, yin, w, h, format, planeMask, pImage);
			}
			break;
		default:
			ErrorF("plxGetImage(): XYPixmap depth mismatch %d\n", pDrawable->depth);
			break;
		}
		break;
	case ZPixmap:
		switch (pDrawable->depth) {
		case 1:
			p_opaq(UBIT_OPAQ_TABLE);
#ifdef notdef
			p_opaqm(planeMask, planeMask);
#endif
			p_ubitl(x, PTY(y), x+w-1, PTY(y+h-1), pImage);
			break;
		case 8:
			if ((planeMask & 0xff) == 0xff) {
#if BITMAP_SCANLINE_PAD == 8
				p_uimg(x, PTY(y), x+w-1, PTY(y+h-1), pImage);
#endif
#if BITMAP_SCANLINE_PAD == 16
				p_uimgw(x, PTY(y), x+w-1, PTY(y+h-1), pImage);
#endif
#if BITMAP_SCANLINE_PAD == 32
				p_uimgl(x, PTY(y), x+w-1, PTY(y+h-1), pImage);
#endif
			} else {
				ifdebug(10) printf("plxGetImage() ZPixmap & bad planemask, using miGetImage()\n");
				miGetImage(pDrawable, xin, yin, w, h, format, planeMask, pImage);
			}
			break;
		default:
			ErrorF("plxGetImage(): ZPixmap depth mismatch %d\n", pDrawable->depth);
			break;
		}
		break;
	default:
		ErrorF("plxGetImage(): Illegal Format %d\n", format);
		break;
	}

	p_opaq(0);				/* reset */
	p_mask(0xffff);
#ifndef X11R2
	/*
	 * consult backing store for the rest of the bits
	 */
	if ((pDrawable->type == DRAWABLE_WINDOW) &&
	    (((WindowPtr)pDrawable)->backingStore != NotUseful))
	{
	    miBSGetImage(pDrawable, (PixmapPtr) 0, xin, yin, w, h, format,
			 planeMask, pImage);
	}
#endif /* not X11R2 */
}

void
plxPutImage(pDrawable, pGC, depth, x, y, w, h, leftPad, format, pImage)
DrawablePtr pDrawable;
GCPtr pGC;
int depth, x, y, w, h, leftPad;
unsigned int format;
unsigned long *pImage;
{
	short xorg, yorg;

	ifdebug(10) printf("plxPutImage() 0x%08x, x,y,w,h=%d,%d,%d,%d\n", pDrawable, x, y, w, h);

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
			ErrorF("plxPutImage: PIXMAP NOT IN CACHE\n");
			return;
		}
		yorg = PTY(yorg);
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

	switch (pDrawable->depth) {
	case 1:
		switch (depth) {
		case 1:
			p_rmap(LBIT_RMAP_TABLE);
			plxbyteswap(0);
#if BITMAP_SCANLINE_PAD == 8
			CLIPREG(p_lbit(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
#if BITMAP_SCANLINE_PAD == 16
			CLIPREG(p_lbitw(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
#if BITMAP_SCANLINE_PAD == 32
			CLIPREG(p_lbitl(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
			plxbyteswap(1);
			break;
		default:
			ErrorF("plxPutImage(), DEPTH MISMATCH %d %d\n", pDrawable->depth, depth);
			break;
		}
		break;
	case 8:
		switch (depth) {
		case 1:
			switch(format) {
			case XYBitmap:
				set_lbmrmap(pGC->fgPixel, pGC->bgPixel);
				plxbyteswap(0);
#if BITMAP_SCANLINE_PAD == 8
				CLIPREG(p_lbit(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
#if BITMAP_SCANLINE_PAD == 16
				CLIPREG(p_lbitw(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
#if BITMAP_SCANLINE_PAD == 32
				CLIPREG(p_lbitl(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
				plxbyteswap(1);
				break;
			case XYPixmap:
			case ZPixmap:
				ErrorF("plxPutImage(), BAD FORMAT %d %d %d\n", pDrawable->depth, depth, format);
				break;
			}
			break;
		case 8:
			switch(format) {
			case XYBitmap:
				ErrorF("plxPutImage(), BAD FORMAT %d %d %d\n", pDrawable->depth, depth, format);
				break;
			case XYPixmap:
				ErrorF("plxPutImage(), UNCODED FORMAT %d %d %d\n", pDrawable->depth, depth, format);
				break;
			case ZPixmap:
				p_rmap(0);
#if BITMAP_SCANLINE_PAD == 8
				CLIPREG(p_limg(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
#if BITMAP_SCANLINE_PAD == 16
				CLIPREG(p_limgw(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
#if BITMAP_SCANLINE_PAD == 32
				CLIPREG(p_limgl(x, PTY(y), x+w-1, PTY(y+h-1), pImage));
#endif
			}
			break;
		default:
			ErrorF("plxPutImage(), DEPTH MISMATCH %d %d\n", pDrawable->depth, depth);
			break;
		}
		break;
	default:
		ErrorF("plxPutImage(), DEPTH MISMATCH %d %d\n", pDrawable->depth, depth);
		break;
	}

	p_rmap(0);
	p_mask(0xffff);
}
