/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.

                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

#include "apollo.h"
#include "pixmap.h"
#include "region.h"
#include "gc.h"
#include "colormap.h"
#include "miscstruct.h"
#include "mibstore.h"

extern Bool apcScreenInit();
extern void apcQueryBestSize();
extern Bool apcCreateWindow();
extern Bool apcPositionWindow();
extern Bool apcChangeWindowAttributes();
extern Bool apcMapWindow();
extern Bool apcUnmapWindow();
extern Bool apcDestroyWindow();
extern void apcCopyWindow();

extern Bool apcCreateGC();
extern void apcDestroyGC();
extern void apcValidateGC();

extern PixmapPtr apcCreatePixmap();
extern Bool apcDestroyPixmap();

extern void apcSetSpans();
extern unsigned int *apcGetSpans();
extern void apcSolidFS();
extern void apcUnnaturalTileFS();
extern void apcUnnaturalStippleFS();
extern void apcUnnaturalOpaqueStippleFS();

extern RegionPtr apcPixmapToRegion();
extern void apcPushPixels();

extern void apcChangeClip();
extern void apcDestroyClip();
extern void apcCopyClip();

extern void apcCopyGCDest();

extern void apcPushPixels();
extern RegionPtr apcCopyArea();
extern void apcSaveAreas();
extern void apcRestoreAreas();

extern void apcValidateTile();
extern void apcPolyFillRect();
extern void apcPaintWindow();
extern void apcZeroLine();
extern void apcPolySegment();

extern void miNotMiter();
extern void miMiter();
extern void miPolyArc();

extern void miPolyFillRect();
extern void miPolyFillArc();

extern void miPutImage();
extern void miGetImageWithBS();
extern RegionPtr miCopyPlane();
extern void miPolyPoint();

extern Bool mfbRealizeFont();
extern Bool mfbUnrealizeFont();

/*
   private filed of pixmap
   pixmap.devPrivate = (unsigned int *)pointer_to_bits
   pixmap.devKind = width_of_pixmap_in_bytes

   private field of screen
   a pixmap, for which we allocate storage.  devPrivate is a pointer to
the bits in the hardware framebuffer.  note that devKind can be poked to
make the code work for framebuffers that are wider than their
displayable screen (e.g. the early vsII, which displayed 960 pixels
across, but was 1024 in the hardware.)  this is left to the
code that calls apcScreenInit(), rather than having apcScreenInit()
take yet another parameter.

   private field of GC 
	pAbsClientRegion is always a real region, although perhaps
an empty one.
	Freeing pCompositeClip is done based on the value of
freeCompClip; if freeCompClip is not carefully maintained, we will end
up losing storage or freeing something that isn't ours.
*/

typedef struct {
    short	fExpose;		/* callexposure handling ? */
    short	freeCompClip;
    RegionPtr	pAbsClientRegion;	/* client region in screen coords */
    RegionPtr	pCompositeClip;		/* free this based on freeCompClip
					   flag rather than NULLness */
    int     polyTextVal;   /* desired text color for gpr implementation of polytext */
    } apcPrivGC;
typedef apcPrivGC	*apcPrivGCPtr;

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

/* private field of PIXMAP */
typedef struct {
    int			    *bitmap_ptr;    /* pointer to the bitmap */
    gpr_$bitmap_desc_t      bitmap_desc;    /* gpr bitmap descriptor */
    int 		    width;          /* width of bitmap in words */
    gpr_$offset_t 	    size;           /* bitmap size */
    } apcPrivPM;
typedef apcPrivPM	*apcPrivPMPtr;   

/* precomputed information about each glyph for GlyphBlt code.
   this saves recalculating the per glyph information for each
   box.
*/
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

/* out of clip region codes */
#define OUT_LEFT 0x08
#define OUT_RIGHT 0x04
#define OUT_ABOVE 0x02
#define OUT_BELOW 0x01

/* reduced raster ops */
#define RROP_BLACK	GXclear
#define RROP_WHITE	GXset
#define RROP_NOP	GXnoop
#define RROP_INVERT	GXinvert

/* optimization codes for FONT's devPrivate field */
#define FT_VARPITCH	0
#define FT_SMALLPITCH	1
#define FT_FIXPITCH	2

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
