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
static char *sid_ = "@(#)plxGlyph.c	1.19 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

#include	"Xproto.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"

/*
 * machine-independent glyph blt.
 * assumes that glyph bits in snf are written in bytes,
 * have same bit order as the server's bitmap format,
 * and are byte padded. this corresponds to the snf distributed
 * with the sample server.
 * 
 */

void
plxImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
DrawablePtr pDrawable;
GC *pGC;
int x, y;
unsigned int nglyph;
CharInfoPtr *ppci;		/* array of character info */
unsigned char *pglyphBase;	/* start of array of glyphs */
{
	int width, height;
	short xorg, yorg;
	FontRec *pfont = pGC->font;
	short idx = pDrawable->pScreen->myNum;
	struct plxfontpriv *privPtr = (struct plxfontpriv *) pfont->devPriv[idx];
	register CharInfoPtr pci;	/* currect char info */
	register unsigned char *pglyph;	/* pointer bits in glyph */
	int gWidth, gHeight;		/* width and height of glyph */
	register int nbyGlyphWidth;	/* bytes per scanline of glyph */
	int nbyPadGlyph;		/* server padded line of glyph */
	int tx,ty;

	ifdebug(14) printf("plxImageGlyphBlt(), 0x%08x, nglyph=%d\n", pDrawable, nglyph);

	if (!privPtr->cached) {
		ErrorF("plxImageGlyphBlt: FONT NOT IN CACHE\n");
		return;
	}

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		if (pGC->miTranslate) {
			xorg = ((WindowPtr)pDrawable)->absCorner.x;
			yorg = ((WindowPtr)pDrawable)->absCorner.y;
		} else {
			xorg = 0;
			yorg = 0;
		}
		break;
	case DRAWABLE_PIXMAP:
		if (!plxpixmapuse(PIXMAP_WRITE, pDrawable, &xorg, &yorg)) {
			ErrorF("plxImageGlyphBlt: PIXMAP NOT IN CACHE\n");
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

	SetFontMaps(pGC->fgPixel, pGC->bgPixel, 1 << privPtr->plane, 0);
	while (nglyph--) {
		pci = *ppci++;
		idx = (short) (pci - pfont->pCI);
		gWidth = GLYPHWIDTHPIXELS(pci);
		gHeight = GLYPHHEIGHTPIXELS(pci);

		tx = x + pci->metrics.leftSideBearing;
		ty = PTY(y - pci->metrics.ascent);
		ifdebug(14) printf("\tleft,top=%d,%d x,y=%d,%d\n", privPtr->lefts[idx], privPtr->tops[idx], tx, ty);
		CLIPREG(p_boxc(privPtr->lefts[idx], privPtr->tops[idx], tx, ty, ((tx + gWidth) - 1), ((ty - gHeight) + 1)));
		x += pci->metrics.characterWidth;
	}

	p_rmap(0);
	p_opaq(0);
	p_mask(0xffff);
}

void
plxPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
DrawablePtr pDrawable;
GC *pGC;
int x, y;
unsigned int nglyph;
CharInfoPtr *ppci;		/* array of character info */
unsigned char *pglyphBase;	/* start of array of glyphs */
{
	int width, height;
	short xorg, yorg;
	FontRec *pfont = pGC->font;
	short idx = pDrawable->pScreen->myNum;
	struct plxfontpriv *privPtr = (struct plxfontpriv *) pfont->devPriv[idx];
	register CharInfoPtr pci;	/* currect char info */
	register unsigned char *pglyph;	/* pointer bits in glyph */
	int gWidth, gHeight;		/* width and height of glyph */
	register int nbyGlyphWidth;	/* bytes per scanline of glyph */
	int nbyPadGlyph;		/* server padded line of glyph */
	int tx,ty;

	ifdebug(14) printf("plxPolyGlyphBlt(), 0x%08x, nglyph=%d\n", pDrawable, nglyph);

	if (!privPtr->cached) {
		ErrorF("plxPolyGlyphBlt: FONT NOT IN CACHE\n");
		return;
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
			ErrorF("plxPolyGlyphBlt: PIXMAP NOT IN CACHE\n");
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

	while (nglyph--) {
		pci = *ppci++;
		idx = (short) (pci - pfont->pCI);
		gWidth = GLYPHWIDTHPIXELS(pci);
		gHeight = GLYPHHEIGHTPIXELS(pci);

		tx = x + pci->metrics.leftSideBearing;
		ty = y - pci->metrics.ascent;
		/* squeeze the glyph with fill style */
		ifdebug(14) printf("\tleft,top=%d,%d x,y=%d,%d\n", privPtr->lefts[idx], privPtr->tops[idx], tx, ty);
		plxdrawthrustencil(pGC, pDrawable, gWidth, gHeight, tx, ty, privPtr->lefts[idx], privPtr->tops[idx], xorg, yorg, 1 << privPtr->plane);
		x += pci->metrics.characterWidth;
	}

	p_mask(0xffff);
}
