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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: mfb.h,v 1.5 88/09/06 15:20:27 jim Exp $ */
/* Monochrome Frame Buffer definitions 
   written by drewry, september 1986
*/
#include "pixmap.h"
#include "region.h"
#include "gc.h"
#include "colormap.h"
#include "miscstruct.h"
#include "../hp/XInputExt.h"

#include "mibstore.h"

extern int InverseAlu[];

extern Bool mfbScreenInit();
extern void mfbQueryBestSize();
extern Bool mfbCreateWindow();
extern Bool mfbPositionWindow();
extern Bool mfbChangeWindowAttributes();
extern Bool mfbMapWindow();
extern Bool mfbUnmapWindow();
extern Bool mfbDestroyWindow();

extern Bool mfbRealizeFont();
extern Bool mfbUnrealizeFont();
extern Bool mfbRealizeCursor();
extern Bool mfbUnrealizeCursor();
extern Bool mfbScreenSaver();
extern Bool mfbCreateGC();

extern PixmapPtr mfbCreatePixmap();
extern Bool mfbDestroyPixmap();

extern void mfbCopyWindow();

extern void mfbSaveAreas();
extern void mfbRestoreAreas();

/* window painters */
extern void mfbPaintWindowNone();
extern void mfbPaintWindowPR();
extern void mfbPaintWindowSolid();
extern void mfbPaintWindow32();

/* rectangle painters */
extern void mfbSolidWhiteArea();
extern void mfbStippleWhiteArea();
extern void mfbSolidBlackArea();
extern void mfbStippleBlackArea();
extern void mfbSolidInvertArea();
extern void mfbStippleInvertArea();
extern void mfbTileArea32();


extern void mfbPolyFillRect();
extern RegionPtr mfbCopyArea();
extern void mfbPolyPoint();
extern RegionPtr mfbCopyPlane();

extern void mfbDestroyGC();
extern void mfbValidateGC();

extern void mfbSetSpans();
extern unsigned int *mfbGetSpans();
extern void mfbWhiteSolidFS();
extern void mfbBlackSolidFS();
extern void mfbInvertSolidFS();
extern void mfbWhiteStippleFS();
extern void mfbBlackStippleFS();
extern void mfbInvertStippleFS();
extern void mfbTileFS();
extern void mfbUnnaturalTileFS();
extern void mfbUnnaturalStippleFS();

extern void mfbGetImage();
extern void mfbPutImage();

extern void mfbLineSS();	/* solid single-pixel wide line */
				/* calls mfb{Bres|Horz|Vert}S() */
extern void mfbDashLine();	/* dashed zero-width line */
extern void mfbImageText8();
extern void mfbImageText16();
extern int mfbPolyText16();
extern int mfbPolyText8();
extern PixmapPtr mfbCopyPixmap();
extern RegionPtr mfbPixmapToRegion();
extern void mfbPushPixels();

/* text for glyphs <= 32 bits wide */
extern void mfbImageGlyphBltWhite();
extern void mfbImageGlyphBltBlack();
extern void mfbPolyGlyphBltWhite();
extern void mfbPolyGlyphBltBlack();
extern void mfbPolyGlyphBltInvert();

/* text for terminal emulator fonts */
extern void mfbTEGlyphBltWhite();	/* fg = 1, bg = 0 */
extern void mfbTEGlyphBltBlack();	/* fg = 0, bg = 1 */

extern void mfbChangeClip();
extern void mfbDestroyClip();
extern void mfbCopyClip();

extern int mfbListInstalledColormaps();
extern void mfbInstallColormap();
extern void mfbUninstallColormap();

extern void mfbResolveColor();

extern void mfbCopyGCDest(),  hpDoNothing();

#define PIXMAP_HOST_MEMORY 	1
#define	PIXMAP_FRAME_BUFFER	2

/* private field of pixmap */
typedef struct {
    pointer	bits;
    short	stride;	    /* width of the pixmap in bytes */
    pointer     pChunk;     /* description of off-screen memory (if used) */
} mfbPrivPixmap;
typedef mfbPrivPixmap	*mfbPrivPixmapPtr;

/* private field of screen. These fields must be in same order as in cfb. */
typedef struct {
    pointer 	bits;
    short	stride;
    DrawablePtr	pDrawable;
    pointer	pHardwareScreen;
    int		(*MoveBits)();
	/* cursor stuff */
    void (*MoveMouse)(), (*CursorOff)();
    short int
      hoffX, hoffY,	/* offset of hot spot in cursor rectangle */
      width, height,	/* of cursor rectangle */
      ssaveX, ssaveY,	/* where to save screen covered by cursor */
      srcX, srcY,	/* cursor source */
      maskX, maskY,	/* cursor mask */
      X,Y,		/* upper left corner of cursor rectangle */
      w,h;		/* chunk of cursor thats on screen */
    unsigned char
      cstate;		/* cursor state */
    /* allow two heads to work on same hardware (e.g. da Vinci) */
    void (*ChangeScreen)();
    Bool isBlank,isSaved;
} mfbPrivScreen;
typedef mfbPrivScreen	*mfbPrivScreenPtr;

/* private field of GC */
typedef struct {
    unsigned char	rop;		/* reduction of rasterop to 1 of 3 */
    unsigned char	ropOpStip;	/* rop for opaque stipple */
    unsigned char	ropFillArea;	/*  == alu, rop, or ropOpStip */
    short	fExpose;	/* callexposure handling ? */
    short	freeCompClip;
    PixmapPtr	pRotatedTile;		/* tile/stipple  rotated to align */
    PixmapPtr	pRotatedStipple;	/* with window and using offsets */
    RegionPtr	pAbsClientRegion;	/* client region in screen coords */
    RegionPtr	pCompositeClip;		/* free this based on freeCompClip
					   flag rather than NULLness */
    void 	(* FillArea)();		/* fills regions; look at the code */
    PixmapPtr   *ppPixmap;		/* points to the pixmapPtr to
					   use for tiles and stipples */
    } mfbPrivGC;
typedef mfbPrivGC	*mfbPrivGCPtr;

/* This macro is also defined in mfb.h */
#ifndef WAIT_READY_TO_RENDER
#define WAIT_READY_TO_RENDER(pScreen) \
    if (((mfbPrivScreenPtr)(pScreen->devPrivate))->MoveBits) \
      (*(((mfbPrivScreenPtr)(pScreen->devPrivate))->MoveBits)) \
        (pScreen, 0, GXnoop, 0, 0, 0, 0, 0, 0)
#endif

/* freeCompositeClip values */
#define REPLACE_CC	0		/* compsite clip is a copy of a
					   pointer, so it doesn't need to 
					   be freed; just overwrite it.
					   this happens if there is no
					   client clip and the gc has
					   ClipByChildren in it.
					*/
#define FREE_CC		1		/* composite clip is a real
					   region that we need to free
					*/

/* private field of window */
typedef struct {
    DeviceMasks dev;
    int		fastBorder;	/* non-zero if border tile is 32 bits wide */
    int		fastBackground;
    DDXPointRec	oldRotate;
    PixmapPtr	pRotatedBackground;
    PixmapPtr	pRotatedBorder;
    } mfbPrivWin;

/* precomputed information about each glyph for GlyphBlt code.
   this saves recalculating the per glyph information for each
box.
*/
#ifndef POS_CONFLICT
typedef struct _pos{
    int xpos;		/* xposition of glyph's origin */
    int xchar;		/* x position mod 32 */
    int leftEdge;
    int rightEdge;
    int topEdge;
    int bottomEdge;
    unsigned int *pdstBase;	/* longword with character origin */
    int widthGlyph;	/* width in bytes of this glyph */
} TEXTPOS;
#endif POS_CONFLICT
#define POS_CONFLICT

/* reduced raster ops for mfb */
#define RROP_BLACK	GXclear
#define RROP_WHITE	GXset
#define RROP_NOP	GXnoop
#define RROP_INVERT	GXinvert

/* out of clip region codes */
#define OUT_LEFT 0x08
#define OUT_RIGHT 0x04
#define OUT_ABOVE 0x02
#define OUT_BELOW 0x01

/* major axis for bresenham's line */
#define X_AXIS	0
#define Y_AXIS	1

/* optimization codes for FONT's devPrivate field */
#define FT_VARPITCH	0
#define FT_SMALLPITCH	1
#define FT_FIXPITCH	2

/* macros for mfbbitblt.c, mfbfillsp.c
   these let the code do one switch on the rop per call, rather
than a switch on the rop per item (span or rectangle.)
*/

#define fnCLEAR(src, dst)	(0)
#define fnAND(src, dst) 	(src & dst)
#define fnANDREVERSE(src, dst)	(src & ~dst)
#define fnCOPY(src, dst)	(src)
#define fnANDINVERTED(src, dst)	(~src & dst)
#define fnNOOP(src, dst)	(dst)
#define fnXOR(src, dst)		(src ^ dst)
#define fnOR(src, dst)		(src | dst)
#define fnNOR(src, dst)		(~(src | dst))
#define fnEQUIV(src, dst)	(~src ^ dst)
#define fnINVERT(src, dst)	(~dst)
#define fnORREVERSE(src, dst)	(src | ~dst)
#define fnCOPYINVERTED(src, dst)(~src)
#define fnORINVERTED(src, dst)	(~src | dst)
#define fnNAND(src, dst)	(~(src & dst))
#define fnSET(src, dst)		(~0)

/* Binary search to figure out what to do for the raster op.  It may
 * do 5 comparisons, but at least it does no function calls 
 * Special cases copy because it's so frequent 
 */
#define DoRop(alu, src, dst) \
( ((alu) == GXcopy) ? (src) : \
    (((alu) >= GXnor) ? \
     (((alu) >= GXcopyInverted) ? \
       (((alu) >= GXnand) ? \
         (((alu) == GXnand) ? ~((src) & (dst)) : ~0) : \
         (((alu) == GXcopyInverted) ? ~(src) : (~(src) | (dst)))) : \
       (((alu) >= GXinvert) ? \
	 (((alu) == GXinvert) ? ~(dst) : ((src) | ~(dst))) : \
	 (((alu) == GXnor) ? ~((src) | (dst)) : (~(src) ^ (dst)))) ) : \
     (((alu) >= GXandInverted) ? \
       (((alu) >= GXxor) ? \
	 (((alu) == GXxor) ? ((src) ^ (dst)) : ((src) | (dst))) : \
	 (((alu) == GXnoop) ? (dst) : (~(src) & (dst)))) : \
       (((alu) >= GXandReverse) ? \
	 (((alu) == GXandReverse) ? ((src) & ~(dst)) : (src)) : \
	 (((alu) == GXand) ? ((src) & (dst)) : 0)))  ) )


#define DoRRop(alu, src, dst) \
(((alu) == RROP_BLACK) ? ((dst) & ~(src)) : \
 ((alu) == RROP_WHITE) ? ((dst) | (src)) : \
 ((alu) == RROP_INVERT) ? ((dst) ^ (src)) : \
  (dst))
