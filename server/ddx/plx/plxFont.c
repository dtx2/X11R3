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
static char *sid_ = "@(#)plxFont.c	1.23 09/01/88 Parallax Graphics Inc";
#endif

#include "Xplx.h"

#include "Xproto.h"
#include "fontstruct.h"
#include "dixfontstr.h"

/*
 * Font area control.
 *
 * the fonts are kept in box's that are part of the cache, there are
 * a up to 8 bitplanes used for each font. the box's collected from the
 * cache have a refcount and should that count reach 0 the box is returned
 * to the cache.
 *
 * plxfontmeminit()		initiallizes font memory 
 * plxfontmemrequestplanes(n)	returns plane with n regions free (-1 if fail)
 * plxfontmemrequest(p, font)	assigns a region in plane p to font.
 * plxfontmemfree(font)		Frees all regions assigned to font
 */

#define	FONT_AREA_WIDTH		256		/* 256x256 box's */
#define	FONT_AREA_HEIGHT	256
#define	FONT_SECTION_NUMBER	11		/* only allow 11 boxs */

struct plxfontmem {
	FontPtr *font[BITPLANES];
	plxCache *el;
	int ref_count;
} plxfontmem[FONT_SECTION_NUMBER];

void
plxfontmeminit()
{
	register int i, plane;

	ifdebug(14) printf("plxfontmeminit()\n");

	for (i=0;i<FONT_SECTION_NUMBER;i++) {
		plxfontmem[i].ref_count = 0;
		plxfontmem[i].el = (plxCache *)0;
		for (plane=0;plane<BITPLANES;plane++) {
			plxfontmem[i].font[plane] = (FontPtr *)0;
		}
	}
}

plxfontmemrequestplanes(planes)
int planes;				/* number of regions requested */
{
	int sn = -1, ln = FONT_SECTION_NUMBER + 1, lp = -1, sp = -1;
	register int i, n, p;
	register plxCache *el;

	ifdebug(14) printf("plxfontmemrequestplanes(), n=%d\n", planes);

	for (p=0;p<BITPLANES;p++) {
		n = 0;
		for (i=0;i<FONT_SECTION_NUMBER;i++) {
			if (!plxfontmem[i].el)
				continue;
			if (plxfontmem[i].font[p] == (FontPtr *)0)
				n++;
		}
		if ((n>=planes) && (n<ln)) {
			lp = p;
			ln = n;
		}
		if (n > sn) {
			sp = p;
			sn = n;
		}
	}
	if (lp >= 0)
		return(lp);
	/*
	 * need more planes
	 */
	i = 0;
	while (sn < planes) {
		while (i<FONT_SECTION_NUMBER) {
			if (!plxfontmem[i].el) {
				break;
			}
			i++;
		}
		if (i == FONT_SECTION_NUMBER) {
			ErrorF("X: Parallax: ran out of font area\n");
			return (-1);
		}
		el = pl_cache_find(FONT_AREA_WIDTH, FONT_AREA_HEIGHT);
		if (!el) {
			ErrorF("X: Parallax: can't get font cache area\n");
			return (-1);
		}
		pl_cache_lock(el);
		plxfontmem[i].el = el;
		plxfontmem[i].ref_count = 0;
		/* clear cache box */
		p_box(0, el->x, el->y, el->x + FONT_AREA_WIDTH - 1, el->y - (FONT_AREA_HEIGHT - 1));
		i++;
		sn++;
	}
	return (sp);
}

struct plxfontmem *
plxfontmemrequest(plane, pFont)
FontPtr *pFont;
{
	register int i;

	ifdebug(14) printf("plxfontmemrequest(), 0x%08x, plane=%d\n", pFont, plane);

	for (i=0;i<FONT_SECTION_NUMBER;i++) {
		if (plxfontmem[i].font[plane] == (FontPtr *)0) {
			plxfontmem[i].font[plane] = pFont;
			plxfontmem[i].ref_count++;
			return (&plxfontmem[i]);
		}
	}
	return((struct plxfontmem *)0);
}

void
plxfontmemfree(pFont)
FontPtr *pFont;
{
	register int i, plane;

	ifdebug(14) printf("plxfontmemfree(), 0x%08x\n", pFont);

	for (i=0;i<FONT_SECTION_NUMBER;i++) {
		for (plane=0;plane<BITPLANES;plane++) {
			if (plxfontmem[i].font[plane] == pFont) {
				plxfontmem[i].font[plane] = (FontPtr *)0;
				if (--plxfontmem[i].ref_count <= 0) {
					pl_cache_free(plxfontmem[i].el);
					plxfontmem[i].el = (plxCache *)0;
				}
			}
		}
	}
}

Bool
plxRealizeFont(pScreen, pFont)
ScreenPtr pScreen;
register FontPtr pFont;
{
	register int idx = pScreen->myNum;
	register int sects, planenum;
	register struct plxfontpriv *privPtr;

	ifdebug(14) printf("plxRealizeFont(), 0x%08x, first,last,glyphs=%d,%d,%d\n", pFont, pFont->pFI->firstCol, pFont->pFI->lastCol, n1dChars(pFont->pFI));

	/* setup our private data structure */
	privPtr = (struct plxfontpriv *) Xalloc(sizeof(struct plxfontpriv));
	privPtr->lefts = (short *)Xalloc(sizeof(short) * n1dChars(pFont->pFI));
	privPtr->tops = (short *)Xalloc(sizeof(short) * n1dChars(pFont->pFI));
	privPtr->cached = TRUE;
	pFont->devPriv[idx] = (pointer)privPtr;

	plxclipinvalidate();

	p_mask(0xffff);
	p_opaq(0);
	p_rmap(0);
	p_damvg();

	/*
	 * pass1: count number of font boxs needed to hold font
	 */
	sects = plxfontdownload(FALSE, pFont, privPtr);
	if (sects == 0) {
		ifdebug(14) printf("plxRealizeFont(), FAILED font glyph too big\n");
		privPtr->cached = FALSE;
		return (FALSE);
	}

	/* request that many sections, they must be on the same bitplane */
	planenum = plxfontmemrequestplanes(sects);
	if (planenum == -1) {	/* we can't catch. Change flag and return */
		ifdebug(14) printf("plxRealizeFont(), FAILED not enought space\n");
		privPtr->cached = FALSE;
		return (FALSE);
	}

	privPtr->plane = planenum;
	p_mask(1<<planenum);
	p_rmap(LBIT_RMAP_TABLE);

	plxbyteswap(0);

	ifdebug(14) printf("\tsects,planenum=%d,%d\n", sects, planenum);

	/*
	 * pass2: actually download the fonts
	 */
	sects = plxfontdownload(TRUE, pFont, privPtr);

	plxbyteswap(1);

	p_rmap(0);			/* reset */
	p_damvx();
	p_mask(0xffff);
	return (TRUE);
}

plxfontdownload(do_load, pFont, privPtr)
int do_load;
FontPtr pFont;
struct plxfontpriv *privPtr;
{
	register int i, yoffset, xoffset, sects, lines, ylineheight;
	register CharInfoPtr cif = pFont->pCI;
	register struct plxfontmem *fmp;
	register int left_x, bot_y, top_y;
	register char *glyphbits = pFont->pGlyphs;
	int glyphs = n1dChars(pFont->pFI);
#ifdef notdef
	static int height[1024];			/* XXXX */
#endif

	xoffset = yoffset = 0;
#ifdef notdef
	ylineheight = 0;
#else
	ylineheight = GLYPHHEIGHTPIXELS((&(pFont->pFI->maxbounds)));
#endif
	sects = lines = 0;
	for (i=0;i<glyphs;i++,cif++) {
		if (GLYPHWIDTHPIXELS(cif) > FONT_AREA_WIDTH) {
			/* font will not fit in the cache */
			return (0);
		}
		if (GLYPHHEIGHTPIXELS(cif) > FONT_AREA_HEIGHT) {
			/* font will not fit in the cache */
			return (0);
		}
		if ((lines == 0) || ((xoffset + GLYPHWIDTHPIXELS(cif)) > FONT_AREA_WIDTH)) {
#ifdef notdef
			/* no room on this line for this glyph */
			height[lines] = ylineheight;
#endif
			xoffset = 0;
			if ((sects == 0) || ((yoffset+ylineheight) > FONT_AREA_HEIGHT)) {
				/* it over runs the font box */
				yoffset = ylineheight;
#ifdef notdef
				ylineheight = 0;
#endif
				sects++;
				if (do_load) {
					fmp = plxfontmemrequest(privPtr->plane, pFont);
					left_x = fmp->el->x;
					bot_y = fmp->el->y - (FONT_AREA_HEIGHT - 1);
				}
			} else {
				/* still room in this font box */
				yoffset += ylineheight;
#ifdef notdef
				ylineheight = 0;
#endif
				if (do_load) {
#ifdef notdef
					bot_y += height[lines];
#else
					bot_y += ylineheight;
#endif
				}
			}
			lines++;
			if (do_load) {
				/* as a we are on a new line, get new top_y */
#ifdef notdef
				top_y = bot_y + (height[lines] - 1);
#else
				top_y = bot_y + (ylineheight - 1);
#endif
			}
		}
		/* we now have room for the glyph in this line in this sect */
		if (do_load) {
			privPtr->lefts[i] = left_x + xoffset;
			privPtr->tops[i] = top_y;
			p_clip(left_x + xoffset, top_y, left_x + xoffset + GLYPHWIDTHPIXELS(cif) - 1, top_y-(GLYPHHEIGHTPIXELS(cif)-1));

#if GLYPHPADBYTES == 0 || GLYPHPADBYTES == 1
			/* vax machines */
			p_lbit(left_x + xoffset, top_y, left_x + xoffset + GLYPHWIDTHPIXELS(cif) - 1, bot_y, &glyphbits[cif->byteOffset]);
#endif /* 0 || 1 */
#if GLYPHPADBYTES == 2
			/* hpux, apollo machines */
			p_lbitw(left_x + xoffset, top_y, left_x + xoffset + GLYPHWIDTHPIXELS(cif) - 1, bot_y, &glyphbits[cif->byteOffset]);
#endif	/* 2 */
#if GLYPHPADBYTES == 4
			/* sun, ibm032 machines */
			p_lbitl(left_x + xoffset, top_y, left_x + xoffset + GLYPHWIDTHPIXELS(cif) - 1, bot_y, &glyphbits[cif->byteOffset]);
#endif	/* 4 */
		}
		xoffset += GLYPHWIDTHPIXELS(cif);
#ifdef notdef
		if (GLYPHHEIGHTPIXELS(cif) > ylineheight)
			ylineheight = GLYPHHEIGHTPIXELS(cif);
#endif
	}

#ifdef notdef
	/* take care of the stragglers */
	height[lines] = ylineheight;
#endif

	return (sects);
}

Bool
plxUnrealizeFont(pScreen, pFont)
ScreenPtr pScreen;
FontPtr pFont;
{
	int idx = pScreen->myNum;
	struct plxfontpriv *privPtr;

	ifdebug(14) printf("plxUnrealizeFont(), 0x%08x\n", pFont);

	plxfontmemfree(pFont);
	privPtr = (struct plxfontpriv *)pFont->devPriv[idx];

	Xfree(privPtr->lefts);
	Xfree(privPtr->tops);
	Xfree(privPtr);
}
