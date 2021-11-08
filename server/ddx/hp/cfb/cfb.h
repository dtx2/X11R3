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
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "pixmap.h"
#include "region.h"
#include "gc.h"
#include "colormap.h"
#include "miscstruct.h"
#include "hpbuf.h"
#include "../hp/XInputExt.h"

extern Bool cfbScreenInit();
extern void cfbQueryBestSize();
extern Bool cfbCreateWindow();
extern Bool cfbPositionWindow();
extern Bool cfbChangeWindowAttributes();
extern Bool cfbMapWindow();
extern Bool cfbUnmapWindow();
extern Bool cfbDestroyWindow();

extern Bool cfbRealizeFont();
extern Bool cfbUnrealizeFont();
extern Bool cfbRealizeCursor();
extern Bool cfbUnrealizeCursor();
extern Bool cfbScreenSaver();
extern Bool cfbCreateGC();

extern PixmapPtr cfbCreatePixmap();
extern Bool cfbDestroyPixmap();

extern void cfbCopyWindow();
extern void cfbPaintWindowBorder();
extern void cfbPaintWindowBackground();
extern void cfbPaintAreaNone();
extern void cfbPaintAreaPR();
extern void cfbPaintAreaSolid();
extern void cfbPaintArea32();
extern void cfbPaintAreaOther();

extern Bool mrcfbScreenInit();
extern void mrcfbQueryBestSize();
extern Bool mrcfbCreateWindow();
extern Bool mrcfbPositionWindow();
extern Bool mrcfbChangeWindowAttributes();
extern Bool mrcfbMapWindow();
extern Bool mrcfbUnmapWindow();
extern Bool mrcfbDestroyWindow();

extern Bool mrcfbRealizeFont();
extern Bool mrcfbUnrealizeFont();
extern Bool mrcfbRealizeCursor();
extern Bool mrcfbUnrealizeCursor();
extern Bool mrcfbScreenSaver();
extern Bool mrcfbCreateGC();

extern PixmapPtr mrcfbCreatePixmap();
extern Bool mrcfbDestroyPixmap();

extern void mrcfbCopyWindow();
extern void mrcfbPaintWindowBorder();
extern void mrcfbPaintWindowBackground();
extern void mrcfbPaintAreaNone();
extern void mrcfbPaintAreaPR();
extern void mrcfbPaintAreaSolid();
extern void mrcfbPaintArea32();
extern void mrcfbPaintAreaOther();

extern void miPolyFillRect();
extern void miPolyFillArc();

extern void cfbDestroyGC();
extern void cfbValidateGC();
extern void cfbDestroyClip();
extern void cfbCopyClip();
extern void cfbChangeClip();
extern void cfbCopyGCDest();

extern void cfbSetSpans();
extern unsigned int *cfbGetSpans();
extern void cfbSolidFS();
extern void cfbUnnaturalTileFS();
extern void cfbUnnaturalStippleFS();

extern void hpDoNothing();
extern void cfbSaveAreas(), cfbRestoreAreas();

extern void mrcfbDestroyGC();
extern void mrcfbValidateGC();
extern void mrcfbDestroyClip();
extern void mrcfbCopyClip();
extern void mrcfbChangeClip();
extern void mrcfbCopyGCDest();

extern void mrcfbSetSpans();
extern unsigned int *mrcfbGetSpans();
extern void mrcfbSolidFS();
extern void mrcfbUnnaturalTileFS();
extern void mrcfbUnnaturalStippleFS();

extern void mrcfbSaveAreas(), mrcfbRestoreAreas();
/* included from mfb.h; we can't include mfb.h directly because of other 
 * conflicts */
extern void mfbSetSpans();
extern unsigned int *mfbGetSpans();
extern void mfbUnnaturalTileFS();
extern void mfbUnnaturalStippleFS();
extern Bool mfbRealizeFont();
extern Bool mfbUnrealizeFont();
extern RegionPtr mfbPixmapToRegion();

extern void miNotMiter();
extern void miMiter();
extern PixmapPtr cfbCopyPixmap();
extern void  cfbConvertRects();
extern PixmapPtr mrcfbCopyPixmap();
extern void  mrcfbConvertRects();
extern void  miPolyArc();
extern void  miFillPolyArc();

extern void miPutImage();
extern void miGetImage();
extern RegionPtr miCopyArea();
extern RegionPtr miCopyPlane();
extern void miPolyPoint();
extern void miPushPixels();

#ifdef	STATIC_COLOR
extern void cfbInstallColormap();
extern void cfbUninstallColormap();
extern int cfbListInstalledColormaps();
extern void cfbResolveStaticColor();

extern void mrcfbInstallColormap();
extern void mrcfbUninstallColormap();
extern int mrcfbListInstalledColormaps();
extern void mrcfbResolveStaticColor();
#endif

/*
   private filed of pixmap
   pixmap.devPrivate = (unsigned int *)pointer_to_bits
   pixmap.devKind = width_of_pixmap_in_bytes
*/
#define PIXMAP_HOST_MEMORY 	1
#define	PIXMAP_FRAME_BUFFER	2

/* size of scratch pixmap in off-screen memory. */
#define PRIV_PIX_WIDTH       32
#define PRIV_PIX_HEIGHT      32

/* private field of pixmap */
typedef struct {
    pointer	bits;
    short	stride;	    /* width of the pixmap in bytes */
    pointer     pChunk;     /* description of off-screen memory (if used) */
} cfbPrivPixmap;
typedef cfbPrivPixmap	*cfbPrivPixmapPtr;

/* allowed types for the gcid */

#define GCID_GATORBOX		8
#define GCID_TOPCAT		9
#define GCID_RENAISSANCE	10
#define GCID_DAVINCI		14
#define GCID_CATSEYE		9	/** what should this be, really? **/
#define GCID_FIREEYE		11

/*
 * Parameters for the two functions in the structure are as
 * follows:
 *   (*MaskConfig)(pScreen, writeEnableMask, replacementRule);
 *   (*MoveBits)(pScreen, planeMask, replacementRule, sourceX, sourceY,
 *   		destX, destY, width, height);
 */

/* private field of screen. Initial fields must be in same order as in mfb */
typedef struct {
    pointer 	bits;
    short	stride;
    DrawablePtr	pDrawable;
    pointer	pHardwareScreen;
    void	(*MoveBits)();
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
    /* fields above are the same as mfbPrivScreen */
    void	(*MaskConfig)();
    pointer     pBufAllocInfo;
    pointer     pTmpPixmap; /* scratch off-screen Pixmap used by cfb code */
    short	memHeight;
    short	memWidth;
    short	fd;
    short	gcid;
    unsigned long	minor_num;
    unsigned char	screenSaver;
    unsigned char	planesMask;
} cfbPrivScreen;
typedef cfbPrivScreen	*cfbPrivScreenPtr;

/* private field of GC */
typedef struct {
    unsigned char       rop;            /* reduction of rasterop to 1 of 3 */
    unsigned char       ropOpStip;      /* rop for opaque stipple */
    unsigned char       ropFillArea;    /*  == alu, rop, or ropOpStip */
    short		fExpose;	/* callexposure handling ? */
    short		freeCompClip;
    PixmapPtr		pRotatedTile;	/* tile/stipple  rotated to align with window */
    PixmapPtr		pRotatedStipple;/* and using offsets */
    RegionPtr		pAbsClientRegion;/* client region in screen coords */
    RegionPtr		pCompositeClip; /* FREE_CC or REPLACE_CC */
    } cfbPrivGC;
typedef cfbPrivGC	*cfbPrivGCPtr;

/* This macro is also defined in cfb.h */
#ifndef WAIT_READY_TO_RENDER
#define WAIT_READY_TO_RENDER(pScreen) \
    if (((cfbPrivScreenPtr)(pScreen->devPrivate))->MoveBits) \
      (*(((cfbPrivScreenPtr)(pScreen->devPrivate))->MoveBits)) \
        (pScreen, 0, GXnoop, 0, 0, 0, 0, 0, 0)
#endif

#ifndef SET_REGISTERS_FOR_WRITING
#define SET_REGISTERS_FOR_WRITING(pScreen, writeEnableMask, replacementRule) \
    if (((cfbPrivScreenPtr)((pScreen)->devPrivate))->MaskConfig)\
      (*(((cfbPrivScreenPtr)((pScreen)->devPrivate))->MaskConfig)) \
      ((pScreen), (writeEnableMask), (replacementRule))
#endif

#ifndef getPlanesMask
#define getPlanesMask(pScreen)                                          \
    (((cfbPrivScreenPtr)((pScreen)->devPrivate))->planesMask)
#endif

/* freeCompositeClip values */
#define REPLACE_CC	0		/* compsite clip is a copy of a
					pointer, so it doesn't need to 
					be freed; just overwrite it.
					this happens if there is no
					client clip and the gc is
					clipped by children 
					*/
#define FREE_CC		1		/* composite clip is a real
					   region that we need to free
					*/

/* private field of window */
typedef struct {
    DeviceMasks dev;
    int		fastBorder;	/* non-zero if border is 32 bits wide */
    int		fastBackground;
    DDXPointRec	oldRotate;
    PixmapPtr	pRotatedBackground;
    PixmapPtr	pRotatedBorder;
    } cfbPrivWin;

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
    int *pdstBase;	/* longword with character origin */
    int widthGlyph;	/* width in bytes of this glyph */
} TEXTPOS;
#endif POS_CONFLICT
#define POS_CONFLICT

/* reduced raster ops for cfb */
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

/* macros for cfbbitblt.c, cfbfillsp.c
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
 * XXX - can't use this in many cases because it has no plane mask.
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
