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

/* "@(#)Xplx.h	1.31 09/01/88 Parallax Graphics Inc" */

#include	<stdio.h>
#include	<sys/types.h>

#define	PC_FASTCALLS
#include	"plxlib.h"
#include	"plxeis.h"
#include	"pl_cache.h"

#ifdef PARALLAX_QEVENT
#ifdef vax
#include	"vaxuba/qevent.h"
#else
#include	"local/qevent.h"
#endif /* vax */
#endif /* PARALLAX_QEVENT */

#include "Xmd.h"
#include "X.h"

#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "windowstr.h"
#include "gcstruct.h"

struct plxfontpriv {
	short cached;	/* true if it is cached */
	short *lefts;	/* if cached, characters left position on screen */
	short *tops;	/* if cached, characters top position on screen */
	short plane;	/* plane cached on */
};

/* private field of GC */
typedef struct {
	short		freeCompClip;
	RegionPtr	pAbsClientRegion;/* client region in screen coords */
	RegionPtr	pCompositeClip; /* FREE_CC or REPLACE_CC */
	PixmapPtr	plxRotatedTile;	/* tile/stipple rotated to align with window or pixmap */
	PixmapPtr	plxStipple;	/* stipple with correct colors */
} plxPrivGC;
typedef plxPrivGC	*plxPrivGCPtr;

/*
 * freeCompositeClip values
 */

/*
 * REPLACE_CC
 *	compsite clip is a copy of a pointer, so it doesn't need to be
 *	freed; just overwrite it.  this happens if there is no client clip and
 *	the gc is clipped by children.
 *
 * FREE_CC
 *	composite clip is a real region that we need to free.
 */
#define	REPLACE_CC	0
#define	FREE_CC		1


typedef struct _plxtile {
	Bool expand;			/* true when expaned */
	Bool rotate;			/* true if needs rotation */
	int x, y;			/* x & y rotation */
	PixmapPtr original;		/* the 'base' tile/stipple */
} plxTile;
	
typedef struct _mappriv {
	plxCache *plxcache;		/* cache element */
	char *plxmemorycopy;		/* host memory copy */
	plxTile plxtile;		/* for rotated tile/stipple */
	int video;			/* == TRUE for video area */
} MapPriv;

extern short Plx_From_X_Op[];
#define PLX_FROM_X_OP(func)	(Plx_From_X_Op[func])

#define BITPLANES 8

#define CURSOR_BG_PSEUDOCOLOR	0xc1
#define CURSOR_FG_PSEUDOCOLOR	0xc3

#define GETIMG_OPAQ_TABLE		12
#define	UBIT_OPAQ_TABLE			14
#define	ONLY_STENCIL_COLOR_OPAQ_TABLE	16	
#define	STIPPLE_OPAQ_TABLE		18
#define	FONT_OPAQ_TABLE			20
#define	DASH_OPAQ_TABLE			22
#define	ONLY1_OPAQ_TABLE		24
#define	ONLY0_OPAQ_TABLE		26
#define	CURSOR_OPAQ_TABLE		28

#define	LBITCOL_RMAP_TABLE		8
#define	LBIT_RMAP_TABLE			9
#define	BITMAP_RMAP_TABLE		10
#define	FONT_RMAP_TABLE			11
#define	ROP1_RMAP_TABLE			12
#define	Zto1_RMAP_TABLE			13
#define	CURSOR_RMAP_TABLE		14

#define	STENCIL_COLOR			255		/* XXX */

/* memory usage:
 *
 *      0      256     512     768    1024    1280     1536    1792   2048
 * 2048 *-------*-------*-------*-------*-------*-------*-------*-------*
 *      *                                       *                       *
 * 1792 *                                       *                       *
 *      *                                       *                       *
 * 1536 *            DISPLAY (1280x1024)        *        CACHE #2       * 
 *      *                                       *                       *
 * 1280 *                                       *                       *
 *      *                                       *                       *
 * 1024 *-------*-------*-------*-------*-------*                       *
 *      *                                       *                       *
 *  768 *                                       *                       *
 *      *                                       *                       *
 *  512 *                                       *                       *
 *      *            CACHE #1                   *                       *
 *  256 *-------*-------*-------*-------*-------*                       *
 *      *             AREA #1                   *                       *
 *    0 *-------*-------*-------*-------*-------*-------*-------*-------*
 */

#define	CACHE_1_X		0
#define	CACHE_1_Y		128
#define	CACHE_1_WIDTH		1280
#define	CACHE_1_HEIGHT		(2048 - 1024 - 128)

#define	CACHE_2_X		1280
#define	CACHE_2_Y		0
#define	CACHE_2_WIDTH		(2048 - 1280)
#define	CACHE_2_HEIGHT		(2048 - 0)

#define	AREA1_TOP		(127 & ~0xf)
#define	AREA1_HEIGHT		(AREA1_TOP - 0 - 1)

/*
 * for plxpixmapuse()
 */
#define	PIXMAP_WRITE	0
#define	PIXMAP_READ	1

/*
 * Screen location macros
 * all screen access goes thru these macros, except when things
 * like clip lists are kept around.
 *
 * these work because "p_pan(0, 0);" is done at init time.
 */

#define	PTX(x)	(x)
#define	PTY(y)	(-(y))
#define	PTW(w)	((w) - 1)
#define	PTH(h)	(1 - (h))

/*
 * various rounding macros
 */

#define	ROUNDUP4(x)	(((x)+3)&(~3))		/* round up */
#define	ROUNDDN4(x)	((x)&(~3))		/* round down */

#define	ROUNDUP16(x)	(((x)+15)&(~15))	/* round up */
#define	ROUNDDN16(x)	((x)&(~15))		/* round down */
#define	ISROUND16(x)	(((x)&0x000f) == 0)	/* already on a 16 boundry */

/*
 * Clip macros, this is now a noop.
 */


#define	CLIPREG(execute_code) \
{ \
	execute_code ; \
}

/*
 * Debug macros
 * the debug control is based on a mask, you can turn on many
 * at a time.
 */

extern int plx_debug;
#define	ifdebug(mask)	if (plx_debug & (1<<mask))


extern unsigned int *plxGetSpans();
extern void plxGetImage(), plxPutImage();
extern void plxSetSpans();
extern void plxSolidFS(), plxTileFS(), plxStippleFS();

extern Bool plxCreateWindow();
extern void plxCopyWindow();
extern Bool plxChangeWindowAttributes();

extern void plxPaintArea();
extern void plxPaintAreaNone(), plxPaintAreaPR(), plxPaintAreaSolid();

extern void plxZeroPolylines(), plxZeroPolySegment(), plxZeroPolyRectangle();

extern RegionPtr plxCopyArea(), plxCopyPlane();

extern void plxPolyFillRect();
extern void plxPolyPoint();
extern void plxFillPolygon();
extern void plxPushPixels();

extern Bool plxRealizeFont(), plxUnrealizeFont();
extern void plxImageGlyphBlt(), plxPolyGlyphBlt();

extern PixmapPtr plxCreatePixmap(), plxCopyPixmap();
extern Bool plxDestroyPixmap();

extern Bool plxRealizeCursor(), plxUnrealizeCursor(), plxDisplayCursor();
extern Bool plxSetCursorPosition(), plxCloseScreen(), plxSaveScreen();
extern void plxCursorLimits();
extern void plxPointerNonInterestBox();
extern void plxConstrainCursor();
extern void plxQueryBestSize();

extern Bool plxRealizeWindow(), plxUnrealizeWindow();
extern Bool plxDestroyWindow(), plxPositionWindow();

extern int plxListInstalledColormaps();
extern void plxCreateColormap(), plxDestroyColormap();
extern void plxInstallColormap(), plxUninstallColormap();
extern void plxResolveColor(), plxStoreColors();

extern Bool plxCreateGC();
extern void plxValidateGC(), plxDestroyGC(), plxCopyGCDest();

extern void plxDestroyClip(), plxChangeClip(), plxCopyClip();

extern void plxRegionCopy(), plxRegionDestroy();
extern void plxRegionReset(), plxTranslateRegion();
extern int plxIntersect(), plxInverse(), plxSubtract(), plxUnion();
extern void plxRegionEmpty();
extern Bool plxRegionNotEmpty();
