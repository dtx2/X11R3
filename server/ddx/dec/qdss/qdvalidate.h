/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/*
 * This scheme seems way too complex; I think it should be replaced with
 * the one used for mfb.		dwm
 */
#ifdef QDVALIDATE
;-=+_*()))(({}{}}}}{{{{@#$@#$^%^*$%^&*!#$$^%&#($&%#(!@!!~```\';:",,":die!
#else
#define	QDVALIDATE
#endif

extern void tlSolidSpans(), tlTiledSpans(), tlStipSpans(), tlOpStipSpans();
extern void tlSolidRects(), tlTiledRects(), tlStipRects(), tlOpStipRects();
extern void qdFSPixSolid(), qdFSPixTiled(), qdFSPixStippleorOpaqueStip(),
		qdFSPixStippleorOpaqueStip();
extern void miPolyFillRect();

/* qdvalidate.h
 *	validation lookup tables
 */

#define	VECFillSpans		0
#define	VECSetSpans		1
#define	VECPutImage		2
#define	VECCopyArea		3
#define	VECCopyPlane		4
#define	VECPolyPoint		5
#define	VECPolylines		6
#define	VECPolySegment		7
#define	VECPolyRectangle	8
#define	VECPolyArc		9
#define	VECFillPolygon		10
#define	VECPolyFillRect		11
#define	VECPolyFillArc		12
#define	VECPolyText8		13
#define	VECPolyText16		14
#define	VECImageText8		15
#define	VECImageText16		16
#define	VECImageGlyphBlt	17
#define	VECPolyGlyphBlt		18
#define	VECPushPixels		19
#define	VECLineHelper		20
/*	THESE MUST BE SET UP AT CREATE TIME
 * #define	VECChangeClip		21
 * #define	VECDestroyClip		22
 */
#define	VECMAX			21
#define	VECNULL			21

#define	VECMOST	10	/* most vectors indexes in one entry */

/* indexed by GC field-changed bits
 *	returns the index of the vector that may be changed.
 */
typedef struct _tlchangevecs
{
	char 	pivec[VECMAX+1];
}	tlchangevecs;
tlchangevecs tlChangeVecs[] =
{
/* GCFunction */		{VECNULL},
/* GCPlaneMask */		{VECNULL},
/* GCForeground */		{VECNULL},
/* GCBackground */		{VECNULL},
/* GCLineWidth */		{VECPolylines, VECNULL},
/* GCLineStyle */		{VECPolylines, VECNULL},
/* GCCapStyle */		{VECNULL},
/* GCJoinStyle */		{VECLineHelper, VECNULL},
/* GCFillStyle */		{VECImageText8, VECPolyText8, VECFillSpans,
				VECPolyFillRect, VECNULL},
/* GCFillRule */		{VECNULL},
/* GCTile */			{VECFillSpans, VECPolyFillRect, VECNULL},
/* GCStipple */			{VECFillSpans, VECPolyFillRect, VECNULL},
/* GCTileStipXOrigin */		{VECNULL},
/* GCTileStipYOrigin */		{VECNULL},
/* GCFont */			{VECImageText8, VECPolyText8, VECNULL},
/* GCSubwindowMode */		{VECNULL},
/* GCGraphicsExposures */	{VECNULL},
/* GCClipXOrigin */		{VECNULL},
/* GCClipYOrigin */		{VECNULL},
/* GCClipMask */		{VECNULL},
/* GCDashOffset */		{VECNULL},
/* GCDashList */		{VECNULL},
/* GCArcMode */			{VECNULL},
	/* THESE ARE devPrivate->GCstate CHANGE BITS: */
/* VSFullReset */
{VECFillSpans, VECSetSpans, VECPutImage, VECCopyArea, VECCopyPlane,
 VECPolyPoint, VECPolylines, VECPolySegment, VECPolyRectangle, VECPolyArc,
 VECFillPolygon, VECPolyFillRect, VECPolyFillArc, VECPolyText8, VECPolyText16,
 VECImageText8, VECImageText16, VECImageGlyphBlt, VECPolyGlyphBlt,
 VECPushPixels, VECLineHelper, VECNULL},
/* VSDest  */
{VECPutImage, VECCopyArea, VECFillPolygon, VECPolyFillRect, VECImageText8,
 VECPolyText8, VECPushPixels, VECPolylines, VECFillSpans, VECNULL},
};

/* Validation Factors
 *	these are sloppy, but they'll have to do.
 *	requires a switch in qdvalidategc.
 * N.B.: these values should be sequential with a small range.
 *	ALWAYS include definitions for all possible values.
 *	you may be hosed if you miss one.
 * the choice of entries is quite arbitrary.  these seemed interesting.
 * 	add more if you wish, but adjust the VFACMAX value.
 */
#define	VFAClineWidth	0
#define	VFAClineStyle	1
#define	VFACcapStyle	2
#define	VFACjoinStyle	3
#define	VFACfillStyle	4
#define	VFACfillRule	5
#define	VFACarcMode	6
#define	VFACdest	7	/* window or pixmap destination */
#define	VFACfont	8	/* can font be loaded offscreen? */
#define	VFACpat		9	/* tile or stipple change */
#define	VFACMAX		10
#define	VFACNULL	10

#define	VFUNCMAX	72

/* indexed by vector indexes (VEC*)
 *	returns a weird command language for validate
 */
typedef	void 	(* PFN)();
typedef struct _tlvecchoice
{
	char	vfacs[VFACMAX+1];
	PFN	function[VFUNCMAX];
}	tlvecchoice;
tlvecchoice tlVecChoice[] =
{
/* VECFillSpans */	{ {VFACfillStyle,VFACdest,VFACpat,VFACNULL},
{(PFN) tlSolidSpans, (PFN) tlTiledSpans, (PFN) tlStipSpans,
	(PFN) tlOpStipSpans,
 (PFN) qdFSPixSolid, (PFN) qdFSPixTiled, (PFN) qdFSPixStippleorOpaqueStip,
	(PFN) qdFSPixStippleorOpaqueStip,
 (PFN) qdFSUndrawable, (PFN) qdFSUndrawable, (PFN) qdFSUndrawable,
	(PFN) qdFSUndrawable,
 (PFN) tlSolidSpans, (PFN) qdWinFSOddSize, (PFN) qdWinFSOddSize,
	(PFN) qdWinFSOddSize,
 (PFN) qdFSPixSolid, (PFN) qdFSPixTiled, (PFN) qdFSPixStippleorOpaqueStip,
	(PFN) qdFSPixStippleorOpaqueStip,
 (PFN) qdFSUndrawable, (PFN) qdFSUndrawable, (PFN) qdFSUndrawable,
	(PFN) qdFSUndrawable}},
/* VECSetSpans */	{ {VFACNULL}, {(PFN) qdSetSpans}},
/* VECPutImage */	{ {VFACdest,VFACNULL},
{(PFN) qdWinPutImage, (PFN) qdPixPutImage, (PFN) miPutImage}},
/* VECCopyArea */	{ {VFACdest,VFACNULL},
{(PFN) qdCopyArea, (PFN) miCopyArea, (PFN) miCopyArea}},
/* VECCopyPlane */	{ {VFACNULL}, {(PFN) miCopyPlane}},
/* VECPolyPoint */	{ {VFACNULL}, {(PFN) miPolyPoint}},
/* VECPolylines */	{
{VFAClineWidth,VFAClineStyle,VFACfillStyle,VFACdest,VFACNULL},
{(PFN) qdPolylines, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash,
 (PFN) miZeroLine, (PFN) miWideLine, (PFN) miWideDash, (PFN) miWideDash,
	(PFN) miWideDash, (PFN) miWideDash}},
/* VECPolySegment */	{ {VFACNULL}, {(PFN) miPolySegment}},
/* VECPolyRectangle */	{ {VFACNULL}, {(PFN) miPolyRectangle}},
/* VECPolyArc */	{ {VFACNULL}, {(PFN) miPolyArc}},
/* VECFillPolygon */	{ {VFACdest,VFACNULL},
{(PFN) qdFillPolygon, (PFN) miFillPolygon, (PFN) miFillPolygon}},
/* VECPolyFillRect */	{ {VFACfillStyle,VFACdest,VFACpat,VFACNULL},
{(PFN) tlSolidRects, (PFN) tlTiledRects, (PFN) tlStipRects,
	(PFN) tlOpStipRects,
     miPolyFillRect, miPolyFillRect, miPolyFillRect, miPolyFillRect,
     miPolyFillRect, miPolyFillRect, miPolyFillRect, miPolyFillRect,
(PFN) tlSolidRects, (PFN) qdPolyFillRectOddSize, (PFN) qdPolyFillRectOddSize,
	(PFN) qdPolyFillRectOddSize,
     miPolyFillRect, miPolyFillRect, miPolyFillRect, miPolyFillRect,
     miPolyFillRect, miPolyFillRect, miPolyFillRect, miPolyFillRect}},
/* VECPolyFillArc */	{ {VFACNULL}, {miPolyFillArc}},
/* VECPolyText8 */	{ {VFACfillStyle,VFACdest,VFACfont,VFACNULL},
{(PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8,
 (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8,
 (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8,
 (PFN) qdPolyText8, (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8,
 (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8,
 (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8, (PFN) miPolyText8}},
/* VECPolyText16 */	{ {VFACNULL}, {(PFN) miPolyText16}},
/* VECImageText8 */	{ {VFACdest,VFACfillStyle,VFACfont,VFACNULL},
{(PFN) miImageText8, (PFN) miImageText8, (PFN) miImageText8,
	(PFN) miImageText8,
 (PFN) miImageText8, (PFN) miImageText8, (PFN) miImageText8,
	(PFN) miImageText8,
 (PFN) miImageText8, (PFN) miImageText8, (PFN) miImageText8,
	(PFN) miImageText8,
 (PFN) qdImageText8, (PFN) miImageText8, (PFN) miImageText8,
	(PFN) miImageText8,
 (PFN) miImageText8, (PFN) miImageText8, (PFN) miImageText8,
	(PFN) miImageText8,
 (PFN) miImageText8, (PFN) miImageText8, (PFN) miImageText8,
	(PFN) miImageText8}},
/* VECImageText16 */	{ {VFACNULL}, {(PFN) miImageText16}},
/* VECImageGlyphBlt */	{ {VFACNULL}, {(PFN) miImageGlyphBlt}},
/* VECPolyGlyphBlt */	{ {VFACNULL}, {(PFN) miPolyGlyphBlt}},
/* VECPushPixels */	{ {VFACdest,VFACNULL},
{(PFN) qdPushPixels, (PFN) miPushPixels, (PFN) miPushPixels}},
/* VECLineHelper */	{ {VFACjoinStyle,VFACNULL},
{(PFN) miMiter, (PFN) miNotMiter, (PFN) miNotMiter}},
};

