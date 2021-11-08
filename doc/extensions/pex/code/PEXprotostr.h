
/*
 *      $Header: PEXprotostr.h,v 1.35 88/09/27 15:39:59 todd Exp $
 */

/* Definitions for PEX graphics extension likely to be used by applications */

#ifndef PEXPROTOSTR_H
#define PEXPROTOSTR_H

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
#include "Xmd.h"			/* defines things like CARD32 */

#include "floatdef.h"			/* defines what "FLOAT" means */

typedef CARD32  pexAsfAttribute;
typedef CARD32	pexAsfValue;
typedef CARD32	pexBitmask;
typedef CARD16	pexBitmaskShort;
typedef CARD16	pexColorType; 	/* direct, indirect */
typedef CARD16  pexCoordType; 	/* rational, nonrational */
typedef CARD16	pexComposition;
typedef CARD16	pexCullMode;
typedef BYTE 	pexDynamicType;
typedef INT16	pexEnumTypeIndex;
typedef XID 	pexLookupTable;
typedef CARD32 	pexName;
typedef XID 	pexNameSet;
typedef XID	pexPC;
typedef XID	pexFont;
typedef FLOAT	pexMatrix[4][4];
typedef FLOAT 	pexMatrix3X3[3][3];
typedef XID	pexPhigsWks;
typedef XID	pexPickMeasure;
typedef XID	pexRenderer;
typedef XID	pexSC;
typedef XID	pexStructure;
typedef CARD8	pexSwitch;
typedef CARD16	pexTableIndex;
typedef CARD16	pexTableType;	/* could be smaller if it ever helps */
typedef CARD16	pexTextHAlignment;
typedef CARD16	pexTextVAlignment;
typedef CARD16	pexTypeOrTableIndex;

/* included in others */
typedef struct {
    CARD16	length B16;
    CARD16	pad B16;
    /* list of CARD8 -- don't swap */
} pexString;

typedef struct {
	FLOAT	x B32;
	FLOAT	y B32;
} pexVector2D;

typedef struct {
	FLOAT	x B32;
	FLOAT	y B32;
	FLOAT	z B32;
} pexVector3D;

/* Coord structures */

typedef struct {
	FLOAT	x B32;
	FLOAT	y B32;
} pexCoord2D;

typedef struct {
	FLOAT	x B32;
	FLOAT	y B32;
	FLOAT	z B32;
} pexCoord3D;

typedef struct {
	FLOAT	x B32;
	FLOAT	y B32;
	FLOAT	z B32;
	FLOAT	w B32;
} pexCoord4D;

/*
 * An awful solution, we waste as much space as if everything were a 
 * 4D coordinate 
 */
 typedef struct {
    union {
	pexCoord2D	c2d;
	pexCoord3D	c3d;
	pexCoord4D	c4d;
    } c;
} pexCoord;


/* Color structures */
typedef struct {
    FLOAT	red B32;
    FLOAT	green B32;
    FLOAT	blue B32;
} pexRgbFloatColor;

typedef struct {
    FLOAT	hue B32;
    FLOAT	saturation B32;
    FLOAT	value B32;
} pexHsvColor;

typedef struct {
    FLOAT	hue B32;
    FLOAT	lightness B32;
    FLOAT	saturation B32;
} pexHlsColor;

typedef struct {
    FLOAT	x B32;
    FLOAT	y B32;
    FLOAT	z B32;
} pexCieColor;

typedef struct {
    CARD8	red;
    CARD8	green;
    CARD8	blue;
    CARD8	pad;
} pexRgb8Color;

typedef struct {
    CARD16	red B16;
    CARD16	green B16;
    CARD16	blue B16;
    CARD16	pad B16;
} pexRgb16Color;

typedef struct {
    pexTableIndex	index B16;
    CARD16		pad B16;
} pexIndexedColor;

typedef struct {
    union {
	pexIndexedColor		indexed;
	pexRgb8Color		rgb8;
	pexRgb16Color		rgb16;
	pexRgbFloatColor	rgbFloat;
	pexHsvColor		hsvFloat;
	pexHlsColor		hlsFloat;
	pexCieColor		cieFloat;
    } format;
} pexColor;

typedef struct {
    union {
	pexRgb8Color		rgb8;
	pexRgb16Color		rgb16;
	pexRgbFloatColor	rgbFloat;
	pexHsvColor		hsvFloat;
	pexHlsColor		hlsFloat;
	pexCieColor		cieFloat;
    } format;
} pexDirectColor;


typedef struct {
    CARD16		colorType B16;	/* Indexed or Direct */
    CARD16		pad B16;
    pexColor		color;
} pexColorSpecifier;


typedef struct {
    pexEnumTypeIndex	approxMethod B16;
    CARD16		pad B16;
    FLOAT		tolerance B32;
} pexCurveApprox;

typedef struct {
    CARD16	x B16;
    CARD16	y B16;
    FLOAT	z B32;
} pexDrawableCoord;

typedef struct {
    CARD16	elementType B16;
    CARD16	length B16;
} pexElementInfo;

typedef struct {
    CARD16	whence B16;
    CARD16	pad B16;
    INT32	offset B32;
} pexElementPos;

typedef struct {
    pexElementPos	position1;
    pexElementPos	position2;
} pexElementRange;

typedef struct {
    pexStructure	structure B32;
    CARD32		offset B32;
} pexElementRef;

typedef struct {
    pexEnumTypeIndex	fpFormat B16;
    pexEnumTypeIndex	directColorFormat B16;
} pexFormat;

typedef struct {
    pexCoord3D	point;
    pexVector3D	vector;
} pexHalfSpace;

typedef struct {
    pexCoord2D	point;
    pexVector2D	vector;
} pexHalfSpace2D;


typedef struct {
    CARD16	composition B16;
    CARD16	pad B16;
    pexMatrix	matrix;
} pexLocalTransform3DData;

typedef struct {
    CARD16		composition B16;
    CARD16		pad B16;
    pexMatrix3X3	matrix;
} pexLocalTransform2DData;

typedef struct {
    pexCoord3D	minval;
    pexCoord3D	maxval;
} pexNpcSubvolume;


/* this is, of course, quite wrong */
typedef struct {
    pexColor	color;
    pexCoord3D	normal;
    pexSwitch	edges;
    CARD8	pad1;
    CARD16	pad B16;
} pexOptData;


typedef struct {
    pexStructure	sid B32;
    CARD32		offset B32;
    CARD32		pickid B32;
} pexPickPath;



typedef struct {
    pexTextVAlignment		vertical B16;
    pexTextHAlignment		horizontal B16;
} pexTextAlignmentData;

typedef struct {
    FLOAT	s B32;
    FLOAT	t B32;
} pexTolerance;

typedef struct {
    pexCoordType	type;
    pexSwitch		visibility;
    CARD8		pad1;
    CARD32		order;
    FLOAT		tMin;
    FLOAT		tMax;
    CARD32		numCoord;
    CARD32		numKnots;
    /* LISTof Coord(numCoord) */
    /* LISTof Float(numKnots) -- length = order + number of coords */
} pexTrimCurve;

typedef struct {
    FLOAT               ambient;
    FLOAT               diffuse;
    FLOAT               specular;
    FLOAT               specularConc;
    FLOAT               transmission;  /* 0.0 = opaque, 1.0 = transparent */
    pexColorSpecifier   specularColor;
} pexReflectionAttr;

typedef struct {
    pexEnumTypeIndex	approxMethod B16;
    CARD16		pad B16;
    FLOAT		sTolerance B32;
    FLOAT		tTolerance B32;
} pexSurfaceApprox;


typedef struct {
    pexCoord3D	point;
    pexOptData	data;
} pexVertex;

typedef struct {
    pexDrawableCoord	minval;
    pexDrawableCoord	maxval;
    INT16		useDrawable B16;
    CARD16		pad B16;
} pexViewport;

typedef struct {
    CARD16		clipFlags B16;
    CARD16		pad B16;
    pexNpcSubvolume	clipLimits;
    pexMatrix		orientation;
    pexMatrix		mapping;
} pexViewEntry;

typedef struct {
    pexTableIndex	index B16;
    CARD16		pad B16;
    pexViewEntry	view;
} pexViewRep;

typedef CARD8	pexVisualState;

/*
 * typedefs for lookup tables
 */

typedef struct {
    CARD16	definableEntries B16;
    CARD16	numPredefined B16;
    CARD16	predefinedMin B16;
    CARD16	predefinedMax B16;
} pexTableInfo;

typedef struct {
    pexEnumTypeIndex	lineType B16;
    pexEnumTypeIndex	polylineInterp B16;
    pexCurveApprox	curveApprox;
    FLOAT		lineWidth B32;
    pexColorSpecifier	lineColor;
} pexLineBundleEntry;

typedef struct {
    pexEnumTypeIndex	markerType B16;
    INT16		pad B16;
    FLOAT		markerScale B32;
    pexColorSpecifier	markerColor;
} pexMarkerBundleEntry;

typedef struct {
    CARD16		textFontIndex B16;
    CARD16		textPrecision B16;
    FLOAT		charExpansion B32;
    FLOAT		charSpacing B32;
    pexColorSpecifier	textColor;
} pexTextBundleEntry;

typedef struct {
    pexEnumTypeIndex	interiorStyle B16;
    INT16		interiorStyleIndex B16;
    pexEnumTypeIndex	reflectionModel B16;
    pexEnumTypeIndex	surfaceInterp B16;
    pexEnumTypeIndex	bfInteriorStyle B16;
    INT16		bfInteriorStyleIndex B16;
    pexEnumTypeIndex	bfReflectionModel B16;
    pexEnumTypeIndex	bfSurfaceInterp B16;
    pexSurfaceApprox	surfaceApprox;
    pexCurveApprox	trimCurveApprox;
    pexColorSpecifier	surfaceColor;
    pexReflectionAttr	reflectionAttr;
    pexColorSpecifier	bfSurfaceColor;
    pexReflectionAttr	bfReflectionAttr;
} pexInteriorBundleEntry;

typedef struct {
    CARD16		edges B16;
    pexEnumTypeIndex	edgeType B16;
    FLOAT		edgeWidth B16;
    pexColorSpecifier	edgeColor B16;
} pexEdgeBundleEntry;

typedef struct {
    pexColorType	colorType;
    CARD16		numx B16;
    CARD16		numy B16;
    CARD16		unused B16;
    /* LISTof Color(numx, numy) 2D array of colors */
} pexPatternEntry;

/* a pexColorEntry is just a pexDirectColor */

/* a pexTextFontEntry is just a pexFont or a Font */

/* a pexViewEntry is defined above */

typedef struct {
    pexEnumTypeIndex	lightType B16;
    INT16		pad B16;
    pexVector3D		direction;
    pexCoord3D		point;
    FLOAT		concentration B32;
    FLOAT		spreadAngle B32;
    FLOAT		attenuation1 B32;
    FLOAT		attenuation2 B32;
    pexColorSpecifier	lightColor;
} pexLightEntry;

typedef struct {
    pexSwitch		mode B16;
    CARD8		pad1;
    CARD16		pad B16;
    FLOAT		frontPlane B32;
    FLOAT		backPlane B32;
    FLOAT		frontScaling B32;
    FLOAT		backScaling B32;
    pexColorSpecifier	depthCueColor;
} pexDepthCueEntry;

typedef struct {
    CARD16	redMax B16;
    CARD16	greenMax B16;
    CARD16	blueMax B16;
    CARD16	dither B16;
    CARD32	redMult B32;
    CARD32	greenMult B32;
    CARD32	blueMult B32;
    CARD32	basePixel B32;
} pexRgbApproxEntry;

typedef struct {
    CARD32	basePixel B32;
    CARD16	intensityMax B16;
    CARD16	dither B16;
} pexIntensityApproxEntry;
#endif /* PEXPROTOSTR_H */
