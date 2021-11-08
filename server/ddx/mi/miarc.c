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
/* $XConsortium: miarc.c,v 1.67 88/12/09 13:25:56 keith Exp $ */
/* Author: Keith Packard */

#include "X.h"
#include "Xprotostr.h"
#include "misc.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "mifpoly.h"
#include "mi.h"

/*
 * some interesting sematic interpretation of the protocol:
 *
 * Self intersecting arcs (i.e. those spanning 360 degrees) 
 *  never join with other arcs, and are drawn without caps
 *  (unless on/off dashed, in which case each dash segment
 *  is capped, except when the last segment meets the
 *  first segment, when no caps are drawn)
 *
 * double dash arcs are drawn in two parts, first the
 *  odd dashes (drawn in background) then the even dashes
 *  (drawn in foreground).  This means that overlapping
 *  sections of foreground/background are drawn twice,
 *  first in background then in foreground.  The double-draw
 *  occurs even when the function uses the destination values
 *  (e.g. xor mode).  This is the same way the wide-line
 *  code works and should be "fixed".
 *
 * the wide arc code will never be "correct" -- the protocol
 *  document specifies exact pixelization which is impossible
 *  when calculating pixel positions with complicated floating-
 *  point expressions.
 */

extern double sqrt(), cos(), sin(), atan();

/* these are from our <math.h>, but I'm told some systems don't have
 * math.h and that they're not in all versions of math.h. */

# define torad(xAngle)	(((double) (xAngle)) / 64.0 * M_PI/180.0)

#define M_PI	3.14159265358979323846
#define M_PI_2	1.57079632679489661923

#ifndef X_AXIS
# define X_AXIS 0
# define Y_AXIS 1
#endif X_AXIS

/* 360 degrees * 64 sub-degree positions */
#define FULLCIRCLE (64 * 360)

# define RIGHT_END	0
# define LEFT_END	1

typedef struct _miArcJoin {
	int	arcIndex0, arcIndex1;
	int	phase0, phase1;
	int	end0, end1;
} miArcJoinRec, *miArcJoinPtr;

typedef struct _miArcCap {
	int		arcIndex;
	int		end;		
} miArcCapRec, *miArcCapPtr;

typedef struct _miArcFace {
	SppPointRec	clock;
	SppPointRec	center;
	SppPointRec	counterClock;
} miArcFaceRec, *miArcFacePtr;

typedef struct _miArcData {
	xArc		arc;
	int		render;		/* non-zero means render after drawing */
	int		join;		/* related join */
	int		cap;		/* related cap */
	int		selfJoin;	/* final dash meets first dash */
	miArcFaceRec	bounds[2];
	double		x0, y0, x1, y1;
} miArcDataRec, *miArcDataPtr;

/*
 * This is an entire sequence of arcs, computed and categorized according
 * to operation.  miDashArcs generates either one or two of these.
 */

typedef struct _miPolyArc {
	int		narcs;
	miArcDataPtr	arcs;
	int		ncaps;
	miArcCapPtr	caps;
	int		njoins;
	miArcJoinPtr	joins;
} miPolyArcRec, *miPolyArcPtr;

static miPolyArcPtr miComputeArcs ();

#define GCValsFunction		0
#define GCValsForeground 	1
#define GCValsBackground 	2
#define GCValsLineWidth 	3
#define GCValsCapStyle 		4
#define GCValsJoinStyle		5
#define GCValsMask		(GCFunction | GCForeground | GCBackground | \
				 GCLineWidth | GCCapStyle | GCJoinStyle)
static CARD32 gcvals[6];

/*
 * draw one segment of the arc using the arc spans generation routines
 */

static void
miArcSegment(pDraw, pGC, tarc, right, left)
    DrawablePtr   pDraw;
    GCPtr         pGC;
    xArc          tarc;
    miArcFacePtr	right, left;
{
    int l = pGC->lineWidth;
    int	w, h;
    int a0, a1, startAngle, endAngle;
    int st, ct;
    miArcFacePtr	temp;

    if (tarc.width == 0 || tarc.height == 0) {
    	drawZeroArc (pDraw, pGC, tarc, left, right);
	return;
    }

    if (pGC->miTranslate && (pDraw->type == DRAWABLE_WINDOW)) {
	tarc.x += ((WindowPtr) pDraw)->absCorner.x;
	tarc.y += ((WindowPtr) pDraw)->absCorner.y;
    }

    if (l < 1)
	l = 1;		/* for 0-width arcs */

    a0 = tarc.angle1;
    a1 = tarc.angle2;
    if (a1 > FULLCIRCLE)
	a1 = FULLCIRCLE;
    else if (a1 < -FULLCIRCLE)
	a1 = -FULLCIRCLE;
    if (a1 < 0) {
    	startAngle = a0 + a1;
	endAngle = a0;
	temp = right;
	right = left;
	left = temp;
    } else {
	startAngle = a0;
	endAngle = a0 + a1;
    }
    /*
     * bounds check the two angles
     */
    if (startAngle < 0)
	startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
    if (startAngle >= FULLCIRCLE)
	startAngle = startAngle % FULLCIRCLE;
    if (endAngle < 0)
	endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
    if (endAngle > FULLCIRCLE)
	endAngle = (endAngle-1) % FULLCIRCLE + 1;
    if (startAngle == endAngle) {
	startAngle = 0;
	endAngle = FULLCIRCLE;
    }

    drawArc ((int) tarc.x, (int) tarc.y,
             (int) tarc.width, (int) tarc.height, l, startAngle, endAngle,
	     right, left);
}

/*
 * miPolyArc strategy:
 *
 * If there's only 1 arc, or if the arc is draw with zero width lines, we 
 * don't have to worry about the rasterop or join styles.   
 * Otherwise, we set up pDrawTo and pGCTo according to the rasterop, then
 * draw using pGCTo and pDrawTo.  If the raster-op was "tricky," that is,
 * if it involves the destination, then we use PushPixels to move the bits
 * from the scratch drawable to pDraw. (See the wide line code for a
 * fuller explanation of this.)
 */

void
miPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register int		i;
    int				xMin, xMax, yMin, yMax;
    int				dx, dy;
    int				xOrg, yOrg;
    double			helperDx, helperDy;
    int				ifirst, count, width;
    Bool			fTricky;
    DrawablePtr			pDrawTo;
    unsigned long		fg, bg;
    GCPtr			pGCTo;
    miPolyArcPtr		polyArcs;
    int				cap[2], join[2];
    int				iphase;

    width = pGC->lineWidth;
    if(width == 0 && pGC->lineStyle == LineSolid)
    {
	for(i = 0; i < narcs; i++)
	    miArcSegment( pDraw, pGC, parcs[i],
 	    (miArcFacePtr) 0, (miArcFacePtr) 0 );
	fillSpans (pDraw, pGC);
    }
    else 
    {
	/* Set up pDrawTo and pGCTo based on the rasterop */
	switch(pGC->alu)
	{
	  case GXclear:		/* 0 */
	  case GXcopy:		/* src */
	  case GXcopyInverted:	/* NOT src */
	  case GXset:		/* 1 */
	    fTricky = FALSE;
	    pDrawTo = pDraw;
	    pGCTo = pGC;
	    break;
	  default:
	    fTricky = TRUE;

	    xMin = yMin = MAXSHORT;
	    xMax = yMax = MINSHORT;

	    for(i = 0; i < narcs; i++)
	    {
		xMin = min (xMin, parcs[i].x);
		yMin = min (yMin, parcs[i].y);
		xMax = max (xMax, (parcs[i].x + (int) parcs[i].width));
		yMax = max (yMax, (parcs[i].y + (int) parcs[i].height));
	    }

	    pGCTo = GetScratchGC(1, pDraw->pScreen);
	    gcvals[GCValsFunction] = GXcopy;
	    gcvals[GCValsForeground] = 1;
	    gcvals[GCValsBackground] = 0;
	    gcvals[GCValsLineWidth] = pGC->lineWidth;
	    gcvals[GCValsCapStyle] = pGC->capStyle;
	    gcvals[GCValsJoinStyle] = pGC->joinStyle;
	    DoChangeGC(pGCTo, GCValsMask, gcvals, 0);
    
    	    xOrg = xMin - (width + 1)/2;
	    yOrg = yMin - (width + 1)/2;
	    dx = (xMax - xMin) + width + 1;
	    dy = (yMax - yMin) + width + 1;
	    for(i = 0; i < narcs; i++)
	    {
		parcs[i].x -= xOrg;
		parcs[i].y -= yOrg;
	    }
	    if (pGC->miTranslate && (pDraw->type == DRAWABLE_WINDOW))
	    {
		xOrg += (double) ((WindowPtr)pDraw)->absCorner.x;
		yOrg += (double) ((WindowPtr)pDraw)->absCorner.y;
	    }

	    /* allocate a 1 bit deep pixmap of the appropriate size, and
	     * validate it */
	    pDrawTo = (DrawablePtr)(*pDraw->pScreen->CreatePixmap)
					(pDraw->pScreen, dx, dy, 1, XYBitmap);
	    ValidateGC(pDrawTo, pGCTo);
	    miClearDrawable(pDrawTo, pGCTo);
	}

	fg = pGCTo->fgPixel;
	bg = pGCTo->bgPixel;
	polyArcs = miComputeArcs (parcs, narcs,
 	    		!(pGC->lineStyle == LineSolid),
 			pGC->lineStyle == LineDoubleDash,
			pGC->dash, pGC->numInDashList, pGC->dashOffset);

	if (!polyArcs)
	{
	    if (fTricky) {
		(*pDraw->pScreen->DestroyPixmap) (pDrawTo);
		FreeScratchGC (pGCTo);
	    }
	    return;
	}

	cap[0] = cap[1] = 0;
	join[0] = join[1] = 0;
	for (iphase = ((pGC->lineStyle == LineDoubleDash) ? 1 : 0);
 	     iphase >= 0;
	     iphase--)
	{
	    if (iphase == 1) {
		gcvals[0] = bg;
		gcvals[1] = fg;
		DoChangeGC (pGC, GCForeground|GCBackground, gcvals, 0);
		ValidateGC (pDraw, pGC);
	    } else if (pGC->lineStyle == LineDoubleDash) {
		gcvals[0] = fg;
		gcvals[1] = bg;
		DoChangeGC (pGC, GCForeground|GCBackground, gcvals, 0);
		ValidateGC (pDraw, pGC);
	    }
	    for (i = 0; i < polyArcs[iphase].narcs; i++) {
		miArcDataPtr	arcData;
		int		j;

		arcData = &polyArcs[iphase].arcs[i];
		miArcSegment(pDrawTo, pGCTo, arcData->arc,
			     &arcData->bounds[RIGHT_END],
			     &arcData->bounds[LEFT_END]);
		if (polyArcs[iphase].arcs[i].render) {
		    fillSpans (pDrawTo, pGCTo);
		    /*
		     * don't cap self-joining arcs
		     */
		    if (polyArcs[iphase].arcs[i].selfJoin &&
		        cap[iphase] < polyArcs[iphase].arcs[i].cap)
		    	cap[iphase]++;
		    while (cap[iphase] < polyArcs[iphase].arcs[i].cap) {
			int	arcIndex, end;
			miArcDataPtr	arcData0;

			arcIndex = polyArcs[iphase].caps[cap[iphase]].arcIndex;
			end = polyArcs[iphase].caps[cap[iphase]].end;
			arcData0 = &polyArcs[iphase].arcs[arcIndex];
			miArcCap (pDrawTo, pGCTo,
 				  &arcData0->bounds[end], end,
				  arcData0->arc.x, arcData0->arc.y,
				  (double) arcData0->arc.width / 2.0,
 				  (double) arcData0->arc.height / 2.0);
			++cap[iphase];
		    }
		    while (join[iphase] < polyArcs[iphase].arcs[i].join) {
			int	arcIndex0, arcIndex1, end0, end1;
			int	phase0, phase1;
			miArcDataPtr	arcData0, arcData1;
			miArcJoinPtr	joinp;

			joinp = &polyArcs[iphase].joins[join[iphase]];
			arcIndex0 = joinp->arcIndex0;
			end0 = joinp->end0;
			arcIndex1 = joinp->arcIndex1;
			end1 = joinp->end1;
			phase0 = joinp->phase0;
			phase1 = joinp->phase1;
			arcData0 = &polyArcs[phase0].arcs[arcIndex0];
			arcData1 = &polyArcs[phase1].arcs[arcIndex1];
			miArcJoin (pDrawTo, pGCTo,
				  &arcData0->bounds[end0],
 				  &arcData1->bounds[end1],
				  arcData0->arc.x, arcData0->arc.y,
				  (double) arcData0->arc.width / 2.0,
 				  (double) arcData0->arc.height / 2.0,
				  arcData1->arc.x, arcData1->arc.y,
				  (double) arcData1->arc.width / 2.0,
 				  (double) arcData1->arc.height / 2.0);
			++join[iphase];
		    }
		    if (fTricky) {
		    	(*pGC->PushPixels) (pGC, pDrawTo, pDraw, dx,
					    dy, xOrg, yOrg);
			miClearDrawable ((DrawablePtr) pDrawTo, pGCTo);
		    }
		}
	    }
	}
	for (iphase = ((pGC->lineStyle == LineDoubleDash) ? 1 : 0);
 	     iphase >= 0;
	     iphase--)
	{
	    if (polyArcs[iphase].narcs > 0)
		Xfree((pointer) polyArcs[iphase].arcs);
	    if (polyArcs[iphase].njoins > 0)
		Xfree ((pointer) polyArcs[iphase].joins);
	    if (polyArcs[iphase].ncaps > 0)
		Xfree ((pointer) polyArcs[iphase].caps);
	}
	Xfree((pointer) polyArcs);

	if(fTricky)
	{
	    (*pGCTo->pScreen->DestroyPixmap)((PixmapPtr)pDrawTo);
	    FreeScratchGC(pGCTo);
	}
    }
}

static double
angleBetween (center, point1, point2)
	SppPointRec	center, point1, point2;
{
	double	atan2 (), a1, a2, a;
	
	/*
	 * reflect from X coordinates back to elipse
	 * coordinates -- y increasing upwards
	 */
	a1 = atan2 (- (point1.y - center.y), point1.x - center.x);
	a2 = atan2 (- (point2.y - center.y), point2.x - center.x);
	a = a2 - a1;
	if (a < -M_PI)
		a += 2 * M_PI;
	else if (a > M_PI)
		a -= 2 * M_PI;
	return a;
}

static
translateBounds (b, x, y, fx, fy)
miArcFacePtr	b;
int		x, y;
double		fx, fy;
{
	b->clock.x -= x + fx;
	b->clock.y -= y + fy;
	b->center.x -= x + fx;
	b->center.y -= y + fy;
	b->counterClock.x -= x + fx;
	b->counterClock.y -= y + fy;
}

miArcJoin (pDraw, pGC, pLeft, pRight,
	   xOrgLeft, yOrgLeft, xFtransLeft, yFtransLeft,
	   xOrgRight, yOrgRight, xFtransRight, yFtransRight)
	DrawablePtr	*pDraw;
	GCPtr		pGC;
	miArcFacePtr	pRight, pLeft;
	int		xOrgRight, yOrgRight;
	double		xFtransRight, yFtransRight;
	int		xOrgLeft, yOrgLeft;
	double		xFtransLeft, yFtransLeft;
{
	SppPointRec	center, corner, otherCorner, end;
	SppPointRec	poly[5], e;
	SppPointPtr	pArcPts;
	int		cpt;
	SppArcRec	arc;
	miArcFaceRec	Right, Left;
	int		polyLen;
	int		xOrg, yOrg;
	double		xFtrans, yFtrans;
	double		angle[4];
	double		a;
	double		ae, ac2, ec2, bc2, de;
	double		width;
	
	xOrg = (xOrgRight + xOrgLeft) / 2;
	yOrg = (yOrgRight + yOrgLeft) / 2;
	xFtrans = (xFtransLeft + xFtransRight) / 2;
	yFtrans = (yFtransLeft + yFtransRight) / 2;
	Right = *pRight;
	translateBounds (&Right, xOrg - xOrgRight, yOrg - yOrgRight,
				 xFtrans - xFtransRight, yFtrans - yFtransRight);
	Left = *pLeft;
	translateBounds (&Left, xOrg - xOrgLeft, yOrg - yOrgLeft,
				 xFtrans - xFtransLeft, yFtrans - yFtransLeft);
	pRight = &Right;
	pLeft = &Left;

	if (pRight->clock.x == pLeft->counterClock.x &&
	    pRight->clock.y == pLeft->counterClock.y)
		return;
	center = pRight->center;
	if (0 <= (a = angleBetween (center, pRight->clock, pLeft->counterClock))
 	    && a <= M_PI)
 	{
		corner = pRight->clock;
		otherCorner = pLeft->counterClock;
	} else {
		a = angleBetween (center, pLeft->clock, pRight->counterClock);
		corner = pLeft->clock;
		otherCorner = pRight->counterClock;
	}
	switch (pGC->joinStyle) {
	case JoinRound:
		width = (pGC->lineWidth ? pGC->lineWidth : 1);

		arc.x = center.x - width/2;
		arc.y = center.y - width/2;
		arc.width = width;
		arc.height = width;
		arc.angle1 = -atan2 (corner.y - center.y, corner.x - center.x);
		arc.angle2 = a;
		pArcPts = (SppPointPtr) Xalloc (sizeof (SppPointRec));
		pArcPts->x = center.x;
		pArcPts->y = center.y;
		if( cpt = miGetArcPts(&arc, 1, &pArcPts))
		{
			/* by drawing with miFillSppPoly and setting the endpoints of the arc
			 * to be the corners, we assure that the cap will meet up with the
			 * rest of the line */
			miFillSppPoly(pDraw, pGC, cpt, pArcPts, xOrg, yOrg, xFtrans, yFtrans);
			Xfree((pointer)pArcPts);
		}
		return;
	case JoinMiter:
		/*
		 * don't miter arcs with less than 11 degrees between them
		 */
		if (a < 169 * M_PI / 180.0) {
			poly[0] = corner;
			poly[1] = center;
			poly[2] = otherCorner;
			bc2 = (corner.x - otherCorner.x) * (corner.x - otherCorner.x) +
			      (corner.y - otherCorner.y) * (corner.y - otherCorner.y);
			ec2 = bc2 / 4;
			ac2 = (corner.x - center.x) * (corner.x - center.x) +
			      (corner.y - center.y) * (corner.y - center.y);
			ae = sqrt (ac2 - ec2);
			de = ec2 / ae;
			e.x = (corner.x + otherCorner.x) / 2;
			e.y = (corner.y + otherCorner.y) / 2;
			poly[3].x = e.x + de * (e.x - center.x) / ae;
			poly[3].y = e.y + de * (e.y - center.y) / ae;
			poly[4] = corner;
			polyLen = 5;
			break;
		}
	case JoinBevel:
		poly[0] = corner;
		poly[1] = center;
		poly[2] = otherCorner;
		poly[3] = corner;
		polyLen = 4;
		break;
	}
	miFillSppPoly (pDraw, pGC, polyLen, poly, xOrg, yOrg, xFtrans, yFtrans);
}

miArcCap (pDraw, pGC, pFace, end, xOrg, yOrg, xFtrans, yFtrans)
	DrawablePtr	*pDraw;
	GCPtr		pGC;
	miArcFacePtr	pFace;
	int		end;
	int		xOrg, yOrg;
	double		xFtrans, yFtrans;
{
	SppPointRec	corner, otherCorner, center, endPoint, poly[5];

	corner = pFace->clock;
	otherCorner = pFace->counterClock;
	center = pFace->center;
	switch (pGC->capStyle) {
	case CapProjecting:
		poly[0].x = otherCorner.x;
		poly[0].y = otherCorner.y;
		poly[1].x = corner.x;
		poly[1].y = corner.y;
		poly[2].x = corner.x -
 				(center.y - corner.y);
		poly[2].y = corner.y +
 				(center.x - corner.x);
		poly[3].x = otherCorner.x -
 				(otherCorner.y - center.y);
		poly[3].y = otherCorner.y +
 				(otherCorner.x - center.x);
		poly[4].x = otherCorner.x;
		poly[4].y = otherCorner.y;
		miFillSppPoly (pDraw, pGC, 5, poly, xOrg, yOrg, xFtrans, yFtrans);
		break;
	case CapRound:
		/*
		 * miRoundCap just needs these to be unequal.
		 */
		endPoint = center;
		endPoint.x = endPoint.x + 100;
		miRoundCap (pDraw, pGC, center, endPoint, corner, otherCorner, 0,
			    -xOrg, -yOrg, xFtrans, yFtrans);
		break;
	}
}

/* MIPOLYFILLARC -- The public entry for the PolyFillArc request.
 * Since we don't have to worry about overlapping segments, we can just
 * fill each arc as it comes.  As above, we convert the arc into a set of
 * line segments and then fill the resulting polygon.
 */
void
miPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    int	i, cpt;
    SppPointPtr pPts;
    SppArcRec	sppArc;
    int		angle1, angle2;

    for(i = 0; i < narcs; i++)
    {
	angle1 = parcs[i].angle1;
	if (angle1 >= FULLCIRCLE)
		angle1 = angle1 % FULLCIRCLE;
	else if (angle1 <= -FULLCIRCLE)
		angle1 = - (-angle1 % FULLCIRCLE);
	angle2 = parcs[i].angle2;
	if (angle2 > FULLCIRCLE)
		angle2 = FULLCIRCLE;
	else if (angle2 < -FULLCIRCLE)
		angle2 = -FULLCIRCLE;
	sppArc.x = parcs[i].x;
	sppArc.y = parcs[i].y;
	sppArc.width = parcs[i].width;
	sppArc.height = parcs[i].height;
	sppArc.angle1 = torad (angle1);
	sppArc.angle2 = torad (angle2);
	/* We do this test every time because a full circle PieSlice isn't
	 * really a slice, but a full pie, and the Chord code (below) should
	 * handle it better */
        if(pGC->arcMode == ArcPieSlice && parcs[i].angle2 < FULLCIRCLE)
	{
	    if (!(pPts = (SppPointPtr)Xalloc(sizeof(SppPointRec))))
		return;
	    if(cpt = miGetArcPts(&sppArc, 1, &pPts))
	    {
		pPts[0].x = sppArc.x + sppArc.width/2;
		pPts[0].y = sppArc.y + sppArc.height /2;
		miFillSppPoly(pDraw, pGC, cpt + 1, pPts, 0, 0, 0.0, 0.0);
		Xfree((pointer) pPts);
	    }
	}
        else /* Chord */
	{
	    pPts = (SppPointPtr)NULL;
	    if(cpt = miGetArcPts(&sppArc, 0, &pPts))
	    {
		miFillSppPoly(pDraw, pGC, cpt, pPts, 0, 0, 0.0, 0.0);
		Xfree((pointer) pPts);
	    }
	}
    }
}

#define REALLOC_STEP 10		/* how often to realloc */
/* MIGETARCPTS -- Converts an arc into a set of line segments -- a helper
 * routine for filled arc and line (round cap) code.
 * Returns the number of points in the arc.  Note that it takes a pointer
 * to a pointer to where it should put the points and an index (cpt).
 * This procedure allocates the space necessary to fit the arc points.
 * Sometimes it's convenient for those points to be at the end of an existing
 * array. (For example, if we want to leave a spare point to make sectors
 * instead of segments.)  So we pass in the Xalloc()ed chunk that contains the
 * array and an index saying where we should start stashing the points.
 * If there isn't an array already, we just pass in a null pointer and 
 * count on Xrealloc() to handle the null pointer correctly.
 */
int
miGetArcPts(parc, cpt, ppPts)
    SppArcPtr	parc;	/* points to an arc */
    int		cpt;	/* number of points already in arc list */
    SppPointPtr	*ppPts; /* pointer to pointer to arc-list -- modified */
{
    double 	st,	/* Start Theta, start angle */
                et,	/* End Theta, offset from start theta */
		dt,	/* Delta Theta, angle to sweep ellipse */
		cdt,	/* Cos Delta Theta, actually 2 cos(dt) */
    		x0, y0,	/* the recurrence formula needs two points to start */
		x1, y1,
		x2, y2, /* this will be the new point generated */
		xc, yc, /* the center point */
                xt, yt;	/* possible next point */
    int		count, i, axis, npts = 2; /* # points used thus far */
    double      asin(), fmax (), sin(), cos ();
    SppPointPtr	poly;
    DDXPointRec last;		/* last point on integer boundaries */

    /* The spec says that positive angles indicate counterclockwise motion.
     * Given our coordinate system (with 0,0 in the upper left corner), 
     * the screen appears flipped in Y.  The easiest fix is to negate the
     * angles given */
    
    st = - parc->angle1;

    et = - parc->angle2;

    /* Try to get a delta theta that is within 1/2 pixel.  Then adjust it
     * so that it divides evenly into the total.
     * I'm just using cdt 'cause I'm lazy.
     */
    cdt = fmax(parc->width, parc->height)/2.0;
    if(cdt <= 0)
	return 0;
    dt = asin( 1.0 / cdt ); /* minimum step necessary */
    count = et/dt;
    count = abs(count) + 1;
    dt = et/count;	
    count++;

    cdt = 2 * cos(dt);
#ifdef NOARCCOMPRESSION
    if (!(poly = (SppPointPtr) Xrealloc((pointer)*ppPts,
					(cpt + count) * sizeof(SppPointRec))))
	return(0);
    *ppPts = poly;
#else				/* ARCCOMPRESSION */
    if (!(poly = (SppPointPtr) Xrealloc((pointer)*ppPts,
					(cpt + 2) * sizeof(SppPointRec))))
	return(0);
#endif				/* ARCCOMPRESSION */

    xc = parc->width/2.0;		/* store half width and half height */
    yc = parc->height/2.0;
    axis = (xc >= yc) ? X_AXIS : Y_AXIS;
    
    x0 = xc * cos(st);
    y0 = yc * sin(st);
    x1 = xc * cos(st + dt);
    y1 = yc * sin(st + dt);
    xc += parc->x;		/* by adding initial point, these become */
    yc += parc->y;		/* the center point */

    poly[cpt].x = (xc + x0);
    poly[cpt].y = (yc + y0);
    last.x = ROUNDTOINT( poly[cpt + 1].x = (xc + x1) );
    last.y = ROUNDTOINT( poly[cpt + 1].y = (yc + y1) );

    for(i = 2; i < count; i++)
    {
	x2 = cdt * x1 - x0;
	y2 = cdt * y1 - y0;

#ifdef NOARCCOMPRESSION
 	poly[cpt + i].x = (xc + x2);
 	poly[cpt + i].y = (yc + y2);
#else				/* ARCCOMPRESSION */
	xt = xc + x2;
	yt = yc + y2;
 	if (((axis == X_AXIS) ?
	     (ROUNDTOINT(yt) != last.y) :
	     (ROUNDTOINT(xt) != last.x)) ||
	    i > count - 3)	/* insure 2 at the end */
 	{
	    /* allocate more space if we are about to need it */
	    /* include 1 extra in case minor axis swaps */
 	    if ((npts - 2) % REALLOC_STEP == 0)
	    {
 		if (!(poly = (SppPointPtr)
		      Xrealloc((pointer) poly,
			       ((npts + REALLOC_STEP + cpt) *
				sizeof(SppPointRec)))))
		    return(0);
	    }
	    /* check if we just switched direction in the minor axis */
	    if (((poly[cpt + npts - 2].y - poly[cpt + npts - 1].y > 0.0) ?
		 (yt - poly[cpt + npts - 1].y > 0.0) :
		 (poly[cpt + npts - 1].y - yt > 0.0)) ||
		((poly[cpt + npts - 2].x - poly[cpt + npts - 1].x > 0.0) ?
		 (xt - poly[cpt + npts - 1].x > 0.0) :
		 (poly[cpt + npts - 1].x - xt > 0.0)))
	    {
		/* Since the minor axis direction just switched, the final *
		 * point before the change must be included, or the        *
		 * following segment will begin before the minor swap.     */
		poly[cpt + npts].x = xc + x1;
		poly[cpt + npts].y = yc + y1;
		npts++;
		if ((npts - 2) % REALLOC_STEP == 0)
		{
		    if (!(poly = (SppPointPtr)
			  Xrealloc((pointer) poly,
				   ((npts + REALLOC_STEP + cpt) *
				    sizeof(SppPointRec)))))
			return(0);
		}
	    }
 	    last.x = ROUNDTOINT( poly[cpt + npts].x = xt );
 	    last.y = ROUNDTOINT( poly[cpt + npts].y = yt );
 	    npts++;
 	}
#endif				/* ARCCOMPRESSION */

	x0 = x1; y0 = y1;
	x1 = x2; y1 = y2;
    }
#ifndef NOARCCOMPRESSION	/* i.e.:  ARCCOMPRESSION */
    count = i = npts;
#endif				/* ARCCOMPRESSION */
    /* adjust the last point */
    if (abs(parc->angle2) >= FULLCIRCLE)
	poly[cpt +i -1] = poly[0];
    else {
	poly[cpt +i -1].x = (cos(st + et) * parc->width/2.0 + xc);
	poly[cpt +i -1].y = (sin(st + et) * parc->height/2.0 + yc);
    }

#ifndef NOARCCOMPRESSION	/* i.e.:  ARCCOMPRESSION */
    *ppPts = poly;		/* may have changed during reallocs */
#endif				/* ARCCOMPRESSION */
    return(count);
}

struct arcData {
	double	x0, y0, x1, y1;
	int	selfJoin;
};

# define ADD_REALLOC_STEP	20

addCap (capsp, ncapsp, sizep, end, arcIndex)
	miArcCapPtr	*capsp;
	int		*ncapsp, *sizep;
	int		end, arcIndex;
{
	miArcCapPtr	cap;

	if (*ncapsp == *sizep)
		*capsp = (miArcCapPtr) Xrealloc (*capsp,
 				(*sizep += ADD_REALLOC_STEP) * sizeof (**capsp));
	cap = &(*capsp)[*ncapsp];
	cap->end = end;
	cap->arcIndex = arcIndex;
	++*ncapsp;
}

addJoin (joinsp, njoinsp, sizep, end0, index0, phase0, end1, index1, phase1)
	miArcJoinPtr	*joinsp;
	int		*njoinsp, *sizep;
	int		end0, index0, end1, index1;
{
	miArcJoinPtr	join;

	if (*njoinsp == *sizep)
		*joinsp = (miArcJoinPtr) Xrealloc (*joinsp,
 				(*sizep += ADD_REALLOC_STEP) * sizeof (**joinsp));
	join = &(*joinsp)[*njoinsp];
	join->end0 = end0;
	join->arcIndex0 = index0;
	join->phase0 = phase0;
	join->end1 = end1;
	join->arcIndex1 = index1;
	join->phase1 = phase1;
	++*njoinsp;
}

miArcDataPtr
addArc (arcsp, narcsp, sizep, xarc)
	miArcDataPtr	*arcsp;
	int		*narcsp, *sizep;
	xArc		xarc;
{
	miArcDataPtr	arc;

	if (*narcsp == *sizep)
		*arcsp = (miArcDataPtr) Xrealloc (*arcsp,
 				(*sizep += ADD_REALLOC_STEP) * sizeof (**arcsp));
	arc = &(*arcsp)[*narcsp];
	arc->arc = xarc;
	++*narcsp;
	return arc;
}

/*
 * this routine is a bit gory
 */

static miPolyArcPtr
miComputeArcs (parcs, narcs, isDashed, isDoubleDash, pDash, nDashes, dashOffset)
	xArc	*parcs;
	int	narcs;
	int	isDashed, isDoubleDash;
	unsigned char	*pDash;
	int	nDashes, dashOffset;
{
	miPolyArcPtr	arcs;
	int		start, i, j, k, nexti, nextk;
	int		joinSize[2];
	int		capSize[2];
	int		arcSize[2];
	int		angle2;
	double		x0, y0, x1, y1, a0, a1, xc, yc;
	struct arcData	*data;
	miArcDataPtr	arc;
	xArc		xarc;
	int		iphase, prevphase, joinphase;
	int		arcsJoin;
	int		selfJoin;

	int		iDash, dashRemaining;
	int		iDashStart, dashRemainingStart, iphaseStart;
	int		startAngle, spanAngle, endAngle, backwards;
	int		prevDashAngle, dashAngle;
	static int	computeAngleFromPath ();

	arcs = (miPolyArcPtr) Xalloc (sizeof (*arcs) * (isDoubleDash ? 2 : 1));
	data = (struct arcData *) ALLOCATE_LOCAL (narcs * sizeof (struct arcData));

	for (i = 0; i < narcs; i++) {
		a0 = torad (parcs[i].angle1);
		angle2 = parcs[i].angle2;
		if (angle2 > FULLCIRCLE)
			angle2 = FULLCIRCLE;
		else if (angle2 < -FULLCIRCLE)
			angle2 = -FULLCIRCLE;
		data[i].selfJoin = angle2 == FULLCIRCLE || angle2 == -FULLCIRCLE;
		a1 = torad (parcs[i].angle1 + angle2);
		data[i].x0 = parcs[i].x + (double) parcs[i].width / 2 * (1 + cos (a0));
		data[i].y0 = parcs[i].y + (double) parcs[i].height / 2 * (1 - sin (a0));
		data[i].x1 = parcs[i].x + (double) parcs[i].width / 2 * (1 + cos (a1));
		data[i].y1 = parcs[i].y + (double) parcs[i].height / 2 * (1 - sin (a1));
	}

	for (iphase = 0; iphase < (isDoubleDash ? 2 : 1); iphase++) {
		arcs[iphase].njoins = 0;
		arcs[iphase].joins = 0;
		joinSize[iphase] = 0;
		
		arcs[iphase].ncaps = 0;
		arcs[iphase].caps = 0;
		capSize[iphase] = 0;
		
		arcs[iphase].narcs = 0;
		arcs[iphase].arcs = 0;
		arcSize[iphase] = 0;
	}

	iphase = 0;
	if (isDashed) {
		iDash = 0;
		dashRemaining = pDash[0];
	 	while (dashOffset > 0) {
			if (dashOffset >= dashRemaining) {
				dashOffset -= dashRemaining;
				iphase = iphase ? 0 : 1;
				iDash++;
				dashRemaining = pDash[iDash];
			} else {
				dashRemaining -= dashOffset;
				dashOffset = 0;
			}
		}
		iDashStart = iDash;
		dashRemainingStart = dashRemaining;
	}
	iphaseStart = iphase;

	for (i = narcs - 1; i >= 0; i--) {
		j = i + 1;
		if (j == narcs)
			j = 0;
		if (data[i].selfJoin || 
		     (UNEQUAL (data[i].x1, data[j].x0) ||
		      UNEQUAL (data[i].y1, data[j].y0)))
 		{
			if (iphase == 0 || isDoubleDash)
				addCap (&arcs[iphase].caps, &arcs[iphase].ncaps,
	 				&capSize[iphase], RIGHT_END, 0);
			break;
		}
	}
	start = i + 1;
	if (start == narcs)
		start = 0;
	i = start;
	for (;;) {
		j = i + 1;
		if (j == narcs)
			j = 0;
		nexti = i+1;
		if (nexti == narcs)
			nexti = 0;
		if (isDashed) {
			/*
			 * compute each individual dash segment using the path
			 * length function
			 */
			startAngle = parcs[i].angle1;
			spanAngle = parcs[i].angle2;
			if (spanAngle > FULLCIRCLE)
				spanAngle = FULLCIRCLE;
			else if (spanAngle < -FULLCIRCLE)
				spanAngle = -FULLCIRCLE;
			if (startAngle < 0)
				startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
			if (startAngle >= FULLCIRCLE)
				startAngle = startAngle % FULLCIRCLE;
			endAngle = startAngle + spanAngle;
			backwards = spanAngle < 0;
			prevDashAngle = startAngle;
			selfJoin = data[i].selfJoin &&
 				    (iphase == 0 || isDoubleDash);
			while (prevDashAngle != endAngle) {
				dashAngle = computeAngleFromPath
 						(prevDashAngle, endAngle,
		 				  parcs[i].width, parcs[i].height,
						  &dashRemaining, backwards);
				if (iphase == 0 || isDoubleDash) {
					xarc = parcs[i];
					xarc.angle1 = prevDashAngle;
					if (backwards) {
						spanAngle = dashAngle - prevDashAngle;
						if (dashAngle > prevDashAngle)
							spanAngle = - 360 * 64 + spanAngle;
					} else {
						spanAngle = dashAngle - prevDashAngle;
						if (dashAngle < prevDashAngle)
							spanAngle = 360 * 64 + spanAngle;
					}
					xarc.angle2 = spanAngle;
					arc = addArc (&arcs[iphase].arcs, &arcs[iphase].narcs,
 							&arcSize[iphase], xarc);
					/*
					 * cap each end of an on/off dash
					 */
					if (!isDoubleDash) {
						if (prevDashAngle != startAngle) {
							addCap (&arcs[iphase].caps,
 								&arcs[iphase].ncaps,
 								&capSize[iphase], RIGHT_END,
 								arc - arcs[iphase].arcs);
							
						}
						if (dashAngle != endAngle) {
							addCap (&arcs[iphase].caps,
 								&arcs[iphase].ncaps,
 								&capSize[iphase], LEFT_END,
 								arc - arcs[iphase].arcs);
						}
					}
					arc->cap = arcs[iphase].ncaps;
					arc->join = arcs[iphase].njoins;
					arc->render = 0;
					arc->selfJoin = 0;
					if (dashAngle == endAngle)
						arc->selfJoin = selfJoin;
				}
				prevphase = iphase;
				if (dashRemaining <= 0) {
					++iDash;
					if (iDash == nDashes)
						iDash = 0;
					iphase = iphase ? 0:1;
					dashRemaining = pDash[iDash];
				}
				prevDashAngle = dashAngle;
			}
		} else {
			arc = addArc (&arcs[iphase].arcs, &arcs[iphase].narcs,
 				      &arcSize[iphase], parcs[i]);
			arc->join = arcs[iphase].njoins;
			arc->cap = arcs[iphase].ncaps;
			arc->selfJoin = data[i].selfJoin;
			prevphase = iphase;
		}
		if (prevphase == 0 || isDoubleDash)
			k = arcs[prevphase].narcs - 1;
		if (iphase == 0 || isDoubleDash)
			nextk = arcs[iphase].narcs;
		if (nexti == start) {
			nextk = 0;
			if (isDashed) {
				iDash = iDashStart;
				iphase = iphaseStart;
				dashRemaining = dashRemainingStart;
			}
		}
		arcsJoin = narcs > 1 && 
	 		    ISEQUAL (data[i].x1, data[j].x0) &&
			    ISEQUAL (data[i].y1, data[j].y0) &&
			    !data[i].selfJoin && !data[j].selfJoin;
		if (arcsJoin)
			arc->render = 0;
		else
			arc->render = 1;
		if (arcsJoin &&
		    (prevphase == 0 || isDoubleDash) &&
		    (iphase == 0 || isDoubleDash))
 		{
			joinphase = iphase;
			if (isDoubleDash) {
				if (nexti == start)
					joinphase = iphaseStart;
				/*
				 * if the join is right at the dash,
				 * draw the join in foreground
				 * This is because the foreground
				 * arcs are computed second, the results
				 * of which are needed to draw the join
				 */
				if (joinphase != prevphase)
					joinphase = 0;
			}
			if (joinphase == 0 || isDoubleDash) {
				addJoin (&arcs[joinphase].joins,
 					 &arcs[joinphase].njoins,
 					 &joinSize[joinphase],
	 				 LEFT_END, k, prevphase,
	 				 RIGHT_END, nextk, iphase);
				arc->join = arcs[prevphase].njoins;
			}
		} else {
			/*
			 * cap the left end of this arc
			 * unless it joins itself
			 */
			if ((prevphase == 0 || isDoubleDash) &&
			    !arc->selfJoin)
			{
				addCap (&arcs[prevphase].caps, &arcs[prevphase].ncaps,
 					&capSize[prevphase], LEFT_END, k);
				arc->cap = arcs[prevphase].ncaps;
			}
			if (isDashed && !arcsJoin) {
				iDash = iDashStart;
				iphase = iphaseStart;
				dashRemaining = dashRemainingStart;
			}
			nextk = arcs[iphase].narcs;
			if (nexti == start) {
				nextk = 0;
				iDash = iDashStart;
				iphase = iphaseStart;
				dashRemaining = dashRemainingStart;
			}
			/*
			 * cap the right end of the next arc.  If the
			 * next arc is actually the first arc, only
			 * cap it if it joins with this arc.  This
			 * case will occur when the final dash segment
			 * of an on/off dash is off.  Of course, this
			 * cap will be drawn at a strange time, but that
			 * hardly matters...
			 */
			if ((iphase == 0 || isDoubleDash) &&
			    (nexti != start || arcsJoin && isDashed))
				addCap (&arcs[iphase].caps, &arcs[iphase].ncaps,
 					&capSize[iphase], RIGHT_END, nextk);
		}
		i = nexti;
		if (i == start)
			break;
	}
	/*
	 * make sure the last section is rendered
	 */
	for (iphase = 0; iphase < (isDoubleDash ? 2 : 1); iphase++)
		if (arcs[iphase].narcs > 0) {
			arcs[iphase].arcs[arcs[iphase].narcs-1].render = 1;
			arcs[iphase].arcs[arcs[iphase].narcs-1].join =
			         arcs[iphase].njoins;
			arcs[iphase].arcs[arcs[iphase].narcs-1].cap =
			         arcs[iphase].ncaps;
		}
	return arcs;
}

/* b > 0 only */

# define mod(a,b)	((a) >= 0 ? (a) % (b) : (b) - (-a) % (b))

/*
 * compute the angle of an elipse which cooresponds to
 * the given path length.  Note that the correct solution
 * to this problem is an eliptic integral, we'll punt and
 * approximate (it's only for dashes anyway).  The approximation
 * used is a diamond (well, sort of anyway)
 *
 * The remaining portion of len is stored in *lenp -
 * this will be negative if the arc extends beyond
 * len and positive if len extends beyond the arc.
 */

static int
computeAngleFromPath (startAngle, endAngle, w, h, lenp, backwards)
	int	startAngle, endAngle;	/* normalized absolute angles in *64 degrees;
	int	w, h;		/* elipse width and height */
	int	*lenp;
	int	backwards;
{
	double	len;
	double	t0, t1, t, l, x0, y0, x1, y1, sidelen;
	int	a, startq, endq, q;
	int	a0, a1;
	double	atan2 (), floor (), acos (), asin ();

	a0 = startAngle;
	a1 = endAngle;
	len = *lenp;
	if (backwards) {
		a0 = FULLCIRCLE - a0;
		a1 = FULLCIRCLE - a1;
	}
	if (a1 < a0)
		a1 += FULLCIRCLE;
	startq = floor ((double) a0 / (90.0 * 64.0));
	endq = floor ((double) a1 / (90.0 * 64.0));
	a = a0;
	a0 = a0 - startq * 90 *64;
	a1 = a1 - endq * 90 * 64;
	for (q = startq; q <= endq && len > 0; ++q) {
		/*
		 * compute the end points of this arc
		 * in this quadrant
		 */
		if (q == startq && a0 != 0) {
			t0 = torad (a0 + startq * 90 * 64);
			x0 = (double) w / 2 * cos (t0);
			y0 = (double) h / 2* sin (t0);
		} else {
			x0 = 0;
			y0 = 0;
			switch (mod (q, 4)) {
			case 0: x0 = (double) w/2;	break;
			case 2:	x0 = - (double) w/2;	break;
			case 1:	y0 = (double) h/2;	break;
			case 3:	y0 = -(double) h/2;	break;
			}
		}
		if (q == endq) {
 			if (a1 == 0) {
				x1 = 0;
				y1 = 0;
				switch (mod (q, 4)) {
				case 0: x1 = (double) w/2;	break;
				case 2:	x1 = - (double) w/2;	break;
				case 1:	y1 = (double) h/2;	break;
				case 3:	y1 = -(double) h/2;	break;
				}
			} else {
				t1 = torad (a1 + endq * 90 * 64);
				x1 = (double) w / 2 * cos(t1);
				y1 = (double) h / 2 * sin(t1);
			}
		} else {
			x1 = 0;
			y1 = 0;
			switch (mod (q, 4)) {
			case 0:	y1 = (double) h/2;	break;
			case 2:	y1 = - (double) h/2;	break;
			case 1:	x1 = -(double) w/2;	break;
			case 3:	x1 = (double) w/2;	break;
			}
		}
		/*
		 * compute the "length" of the arc in this quadrant --
		 * this should be the eliptic integral, we'll
		 * punt and assume it's close to a straight line
		 */
		sidelen = sqrt ((x1-x0)*(x1-x0) + (y1-y0) * (y1-y0));
		if (sidelen >= len) {
			/*
			 * compute the distance to the next axis
			 */
			x1 = 0;
			y1 = 0;
			switch (mod (q, 4)) {
			case 0:	y1 = (double) h/2;	break;
			case 2:	y1 = -(double) h/2;	break;
			case 1:	x1 = -(double) w/2;	break;
			case 3:	x1 = (double) w/2;	break;
			}
			sidelen = sqrt ((x1-x0) * (x1-x0) + (y1-y0) * (y1-y0));
			/*
			 * now pick the point "len" away from x0,y0
			 */
			y1 = y0 + (y1 - y0) * len / sidelen;
			x1 = x0 + (x1 - x0) * len / sidelen;
			/*
			 * translate the point to the angle on the
			 * elipse
			 * match the actual angle)
			 */
			if (x1 == (double) w/2 && y1 == 0)
				a1 = 0;
			else if (x1 == -(double) w/2 && y1 == 0)
				a1 = 180 * 64;
			else if (y1 == (double) h/2 && x1 == 0)
				a1 = 90 * 64;
			else if (y1 == -(double) h/2 && x1 == 0)
				a1 = 270 * 64;
			else {
				if (w == 0) {
					t1 = asin (y1 / ((double) h/2));
					switch (mod (q, 4)) {
					case 1:
					case 2:
						t1 = M_PI - t1;
					}
				} else if (h == 0) {
					t1 = acos (x1 / ((double) w/2));
					switch (mod (q, 4)) {
					case 2:
					case 3:
						t1 = 2 * M_PI - t1;
					}
 				} else {
					/*
					 * for round arcs, convert
					 * to eliptical angles
					 */
					t1 = atan2 (y1 * w, x1 * h);
				}
				a1 = (t1 * 180/M_PI) * 64.0;
				if (a1 < 0)
					a1 += FULLCIRCLE;
			}
 			a1 -= mod (q, 4) * 90 * 64;
			len = 0;
		} else
			len -= sidelen;
	}
	*lenp = len;
	a1 = a1 + (q-1) * (90*64);
	if (backwards)
		a1 = FULLCIRCLE - a1;
	return a1;
}

/*
 * To avoid inaccuracy at the cardinal points, use trig functions
 * which are exact for those angles
 */

# define Dsin(d)	((d) == 0.0 ? 0.0 : ((d) == 90.0 ? 1.0 : sin(d*M_PI/180.0)))
# define Dcos(d)	((d) == 0.0 ? 1.0 : ((d) == 90.0 ? 0.0 : cos(d*M_PI/180.0)))

double
FullDcos (a)
double	a;
{
	double	cos ();
	int	i;

	if (floor (a/90) == a/90) {
		i = (int) (a/90.0);
		switch (mod (i, 4)) {
		case 0: return 1;
		case 1: return 0;
		case 2: return -1;
		case 3: return 0;
		}
	}
	return cos (a * M_PI / 180.0);
}

double
FullDsin (a)
double	a;
{
	double	sin ();
	int	i;

	if (floor (a/90) == a/90) {
		i = (int) (a/90.0);
		switch (mod (i, 4)) {
		case 0: return 0;
		case 1: return 1;
		case 2: return 0;
		case 3: return -1;
		}
	}
	return sin (a * M_PI / 180.0);
}

/*
 * scan convert wide arcs.
 */

#undef fabs
#undef min
#undef max

extern double	ceil (), floor (), fabs (), sin (), cos (), sqrt (), pow ();

/*
 * draw zero width/height arcs
 */

drawZeroArc (pDraw, pGC, tarc, left, right)
    DrawablePtr   pDraw;
    GCPtr         pGC;
    xArc          tarc;
    miArcFacePtr	right, left;
{
	double	x0, y0, x1, y1, w, h;
	int	a0, a1;
	double	startAngle, endAngle;
	double	l;

	l = pGC->lineWidth;
	if (l == 0)
		l = 1;
	l /= 2;
	a0 = tarc.angle1;
	a1 = tarc.angle2;
	if (a1 > FULLCIRCLE)
		a1 = FULLCIRCLE;
	else if (a1 < -FULLCIRCLE)
		a1 = -FULLCIRCLE;
	w = tarc.width / 2.0;
	h = tarc.height / 2.0;
	/*
	 * play in X coordinates right away
	 */
	startAngle = - ((double) a0 / 64.0);
	endAngle = - ((double) (a0 + a1) / 64.0);
	
	x0 = w * FullDcos(startAngle);
	y0 = h * FullDsin(startAngle);
	x1 = w * FullDcos(endAngle);
	y1 = h * FullDsin(endAngle);

	if (y0 != y1) {
		if (y0 < y1) {
			x0 = -l;
			x1 = l;
		} else {
			x0 = l;
			x1 = -l;
		}
	} else {
		if (x0 < x1) {
			y0 = -l;
			y1 = l;
		} else {
			y0 = l;
			y1 = -l;
		}
	}
	if (x1 != x0 && y1 != y0) {
		int	minx, maxx, miny, maxy, y, t;
		xRectangle  rect;

		minx = ceil (x0 + w) + tarc.x;
		maxx = ceil (x1 + w) + tarc.x;
		if (minx > maxx) {
			t = minx;
			minx = maxx;
			maxx = t;
		}
		miny = ceil (y0 + h) + tarc.y;
		maxy = ceil (y1 + h) + tarc.y;
		if (miny > maxy) {
			t = miny;
			miny = maxy;
			maxy = t;
		}
		rect.x = minx;
		rect.y = miny;
		rect.width = maxx - minx;
		rect.height = maxy - miny;
		(*pGC->PolyFillRect) (pDraw, pGC, 1, &rect);
/*
		for (y = miny; y < maxy; y++)
			newFinalSpan (y, minx, maxx);
*/
	}
	if (right) {
		if (h != 0) {
			right->clock.x = x1;
			right->clock.y = y0;
			right->center.x = 0;
			right->center.y = y0;
			right->counterClock.x = x0;
			right->counterClock.y = y0;
		} else {
			right->clock.x = x0;
			right->clock.y = y0;
			right->center.x = x0;
			right->center.y = 0;
			right->counterClock.x = x0;
			right->counterClock.y = y1;
		}
	}
	if (left) {
		if (h != 0) {
			left->clock.x = x0;
			left->clock.y = y1;
			left->center.x = 0;
			left->center.y = y1;
			left->counterClock.x = x1;
			left->counterClock.y = y1;
		} else {
			left->clock.x = x1;
			left->clock.y = y1;
			left->center.x = x1;
			left->center.y = 0;
			left->counterClock.x = x1;
			left->counterClock.y = y0;
		}
	}
}

# define BINARY_LIMIT	(0.1)
# define NEWTON_LIMIT	(0.0000001)

struct bound {
	double	min, max;
};

struct line {
	double	m, b;
	int	valid;
};

/*
 * these are all y value bounds
 */

struct arc_bound {
	struct bound	elipse;
	struct bound	inner;
	struct bound	outer;
	struct bound	right;
	struct bound	left;
};

struct accelerators {
	double		tail_y;
	double		h2;
	double		w2;
	double		h4;
	double		w4;
	double		h2mw2;
	double		wh2mw2;
	double		wh4;
	struct line	left, right;
};

struct arc_def {
	double	w, h, l;
	double	a0, a1;
};

double
Sqrt (x)
double	x;
{
	if (x < 0) {
		if (x > -NEWTON_LIMIT)
			return 0;
		else
			abort ();
	}
	return sqrt (x);
}

double
fmax (a, b)
double	a,b;
{
	return a > b? a : b;
}

double
fmin (a, b)
double	a, b;
{
	return a < b ? a : b;
}

#define boundedLt(value, bounds) \
	((bounds).min <= (value) && (value) < (bounds).max)

#define boundedLe(value, bounds)\
	((bounds).min <= (value) && (value) <= (bounds).max)

/*
 * this computes the elipse y value associated with the
 * bottom of the tail.
 */

# define CUBED_ROOT_2	1.2599210498948732038115849718451499938964
# define CUBED_ROOT_4	1.5874010519681993173435330390930175781250

double
tailElipseY (w, h, l)
	double	w, h, l;
{
	extern double	Sqrt (), pow ();
	double		t;

	if (w != h) {
		t = (pow (h * l * w, 2.0/3.0) - CUBED_ROOT_4 * h*h) /
		    (w*w - h*h);
		if (t < 0)
			return 0;	/* no tail */
		return h / CUBED_ROOT_2 * Sqrt (t);
	} else
		return 0;
}

/*
 * inverse functions -- compute edge coordinates
 * from the elipse
 */

double
outerXfromXY (x, y, def, acc)
	double			x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return x + (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x *acc->h4 + y*y * acc->w4));
}

double
outerXfromY (y, def, acc)
	double			y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);

	return x + (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x *acc->h4 + y*y * acc->w4));
}

double
outerYfromXY (x, y, def, acc)
	double		x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return y + (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
outerYfromY (y, def, acc)
	double	y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);

	return y + (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerXfromXY (x, y, def, acc)
	double			x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return x - (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerXfromY (y, def, acc)
	double			y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);
	
	return x - (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerYfromXY (x, y, def, acc)
	double			x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return y - (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerYfromY (y, def, acc)
	double	y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);

	return y - (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

computeLine (x1, y1, x2, y2, line)
	double		x1, y1, x2, y2;
	struct line	*line;
{
	if (y1 == y2)
		line->valid = 0;
	else {
		line->m = (x1 - x2) / (y1 - y2);
		line->b = x1  - y1 * line->m;
		line->valid = 1;
	}
}

double
intersectLine (y, line)
	double		y;
	struct line	*line;
{
	return line->m * y + line->b;
}

/*
 * compute various accelerators for an elipse.  These
 * are simply values that are used repeatedly in
 * the computations
 */

computeAcc (def, acc)
	struct arc_def		*def;
	struct accelerators	*acc;
{
	acc->h2 = def->h * def->h;
	acc->w2 = def->w * def->w;
	acc->h4 = acc->h2 * acc->h2;
	acc->w4 = acc->w2 * acc->w2;
	acc->h2mw2 = acc->h2 - acc->w2;
	acc->wh2mw2 = def->w * acc->h2mw2;
	acc->wh4 = def->w * acc->h4;
	acc->tail_y = tailElipseY (def->w, def->h, def->l);
}
		
/*
 * compute y value bounds of various portions of the arc,
 * the outer edge, the elipse and the inner edge.
 */

computeBound (def, bound, acc, right, left)
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	miArcFacePtr		right, left;
{
	double		t, elipseX ();
	double		innerTaily;
	double		tail_y;
	struct bound	innerx, outerx;
	struct bound	elipsex;

	bound->elipse.min = Dsin (def->a0) * def->h;
	bound->elipse.max = Dsin (def->a1) * def->h;
	if (def->a0 == 45 && def->w == def->h)
		elipsex.min = bound->elipse.min;
	else
		elipsex.min = Dcos (def->a0) * def->w;
	if (def->a1 == 45 && def->w == def->h)
		elipsex.max = bound->elipse.max;
	else
		elipsex.max = Dcos (def->a1) * def->w;
	bound->outer.min = outerYfromXY (elipsex.min, bound->elipse.min, def, acc);
	bound->outer.max = outerYfromXY (elipsex.max, bound->elipse.max, def, acc);
	bound->inner.min = innerYfromXY (elipsex.min, bound->elipse.min, def, acc);
	bound->inner.max = innerYfromXY (elipsex.max, bound->elipse.max, def, acc);

	outerx.min = outerXfromXY (elipsex.min, bound->elipse.min, def, acc);
	outerx.max = outerXfromXY (elipsex.max, bound->elipse.max, def, acc);
	innerx.min = innerXfromXY (elipsex.min, bound->elipse.min, def, acc);
	innerx.max = innerXfromXY (elipsex.max, bound->elipse.max, def, acc);
	
	/*
	 * save the line end points for the
	 * cap code to use.  Careful here, these are
	 * in cartesean coordinates (y increasing upwards)
	 * while the cap code uses inverted coordinates
	 * (y increasing downwards)
	 */

	if (right) {
		right->counterClock.y = bound->outer.min;
		right->counterClock.x = outerx.min;
		right->center.y = bound->elipse.min;
		right->center.x = elipsex.min;
		right->clock.y = bound->inner.min;
		right->clock.x = innerx.min;
	}

	if (left) {
		left->clock.y = bound->outer.max;
		left->clock.x = outerx.max;
		left->center.y = bound->elipse.max;
		left->center.x = elipsex.max;
		left->counterClock.y = bound->inner.max;
		left->counterClock.x = innerx.max;
	}

	bound->left.min = bound->inner.max;
	bound->left.max = bound->outer.max;
	bound->right.min = bound->inner.min;
	bound->right.max = bound->outer.min;

	computeLine (innerx.min, bound->inner.min, outerx.min, bound->outer.min,
		      &acc->right);
	computeLine (innerx.max, bound->inner.max, outerx.max, bound->outer.max,
		     &acc->left);

	if (bound->inner.min > bound->inner.max) {
		t = bound->inner.min;
		bound->inner.min = bound->inner.max;
		bound->inner.max = t;
	}
	tail_y = acc->tail_y;
	if (tail_y > bound->elipse.max)
		tail_y = bound->elipse.max;
	else if (tail_y < bound->elipse.min)
		tail_y = bound->elipse.min;
	innerTaily = innerYfromY (tail_y, def, acc);
	if (bound->inner.min > innerTaily)
		bound->inner.min = innerTaily;
	if (bound->inner.max < innerTaily)
		bound->inner.max = innerTaily;
}

/*
 * using newtons method and a binary search, compute the elipse y value
 * associated with the given edge value (either outer or
 * inner).  This is the heart of the scan conversion code and
 * is generally called three times for each span.  It should
 * be optimized further.
 *
 * the general idea here is to solve the equation:
 *
 *                               w^2 * l
 *   edge_y = y + y * -------------------------------
 *                    2 * sqrt (x^2 * h^4 + y^2 * w^4)
 *
 * for y.  (x, y) is a point on the elipse, so x can be
 * found from y:
 *
 *                ( h^2 - y^2 )
 *   x = w * sqrt ( --------- )
 *                (    h^2    )
 *
 * The information given at the start of the search
 * is two points which are known to bound the desired
 * solution, a binary search starts with these two points
 * and converges close to a solution, which is then
 * refined with newtons method.  Newtons method
 * cannot be used in isolation as it does not always
 * converge to the desired solution without a close
 * starting point, the binary search simply provides
 * that point.  Increasing the solution interval for
 * the binary search will certainly speed up the
 * solution, but may generate a range which causes
 * the newtons method to fail.  This needs study.
 */

double
elipseY (edge_y, def, bound, acc, outer, y0, y1)
	double			edge_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	register double		y0, y1;
{
	register double	w, h, l, h2, h4, w2, w4, x, y2;
	double		newtony, binaryy;
	double		value0, value1, valuealt;
	double		newtonvalue, binaryvalue;
	double		minY, maxY;
	double		binarylimit;
	double		(*f)();
	
	/*
	 * compute some accelerators
	 */
	w = def->w;
	h = def->h;
	f = outer ? outerYfromY : innerYfromY;
	l = outer ? def->l : -def->l;
	h2 = acc->h2;
	h4 = acc->h4;
	w2 = acc->w2;
	w4 = acc->w4;

	value0 = f (y0, def, acc) - edge_y;
	if (value0 == 0)
		return y0;
	value1 = f (y1, def, acc) - edge_y;
	maxY = y1;
	minY = y0;
	if (y0 > y1) {
		maxY = y0;
		minY = y1;
	}
	if (value1 == 0)
		return y1;
	if (value1 > 0 == value0 > 0)
		return -1.0;	/* an illegal value */
	binarylimit = fabs ((value1 - value0) / 25.0);
	if (binarylimit < BINARY_LIMIT)
		binarylimit = BINARY_LIMIT;
	/*
	 * binary search for a while
	 */
	do {
		if (y0 == y1 || value0 == value1)
			return maxY+1;
		binaryy = (y0 + y1) / 2;

		/*
		 * inline expansion of the function
		 */

		y2 = binaryy*binaryy;
		x = w * Sqrt ((h2 - (y2)) / h2);

		binaryvalue = ( binaryy + (binaryy * w2 * l) /
			      (2 * Sqrt (x*x * h4 + y2 * w4))) - edge_y;

		if (binaryvalue > 0 == value0 > 0) {
			y0 = binaryy;
			value0 = binaryvalue;
		} else {
			y1 = binaryy;
			value1 = binaryvalue;
		}
	} while (fabs (value1) > binarylimit);
	if (binaryvalue == 0)
		return binaryy;

	/*
	 * clean up the estimate with newtons method
	 */

	while (fabs (value1) > NEWTON_LIMIT) {
		newtony = y1 - value1 * (y1 - y0) / (value1 - value0);
		if (newtony > maxY)
			newtony = maxY;
		if (newtony < minY)
			newtony = minY;
		/*
		 * inline expansion of the function
		 */

		y2 = newtony*newtony;
		x = w * Sqrt ((h2 - (y2)) / h2);

		newtonvalue = ( newtony + (newtony * w2 * l) /
			      (2 * Sqrt (x*x * h4 + y2 * w4))) - edge_y;

		if (newtonvalue == 0)
			return newtony;
		if (fabs (value0) > fabs (value1)) {
			y0 = newtony;
			value0 = newtonvalue;
		} else {
			y1 = newtony;
			value1 = newtonvalue;
		}
	}
	return y1;
}

double
elipseX (elipse_y, def, acc)
	double			elipse_y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return def->w / def->h * Sqrt (acc->h2 - elipse_y * elipse_y);
}

double
outerX (outer_y, def, bound, acc)
	register double		outer_y;
	register struct arc_def	*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
{
	double	y;

	/*
	 * special case for circles
	 */
	if (def->w == def->h) {
		register double	x;

		x = def->w + def->l/2.0;
		x = Sqrt (x * x - outer_y * outer_y);
		return x;
	}
	if (outer_y == bound->outer.min)
		y = bound->elipse.min;
	if (outer_y == bound->outer.max)
		y = bound->elipse.max;
	else
		y = elipseY (outer_y, def, bound, acc, 1,
 			     bound->elipse.min, bound->elipse.max);
	return outerXfromY (y, def, acc);
}

/*
 * this equation has two solutions -- it's not a function
 */

innerXs (inner_y, def, bound, acc, innerX1p, innerX2p)
	register double		inner_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	double			*innerX1p, *innerX2p;
{
	register double	x1, x2;
	double		xalt, y0, y1, altY, elipse_y1, elipse_y2;

	/*
	 * special case for circles
	 */
	if (def->w == def->h) {
		x1 = def->w - def->l/2.0;
		x2 = Sqrt (x1 * x1 - inner_y * inner_y);
		if (x1 < 0)
			x2 = -x2;
		*innerX1p = *innerX2p = x2;
		return;
	}
	if (boundedLe (acc->tail_y, bound->elipse)) {
		if (def->h > def->w) {
			y0 = bound->elipse.min;
			y1 = acc->tail_y;
			altY = bound->elipse.max;
		} else {
			y0 = bound->elipse.max;
			y1 = acc->tail_y;
			altY = bound->elipse.min;
		}
		elipse_y1 = elipseY (inner_y, def, bound, acc, 0, y0, y1);
		elipse_y2 = elipseY (inner_y, def, bound, acc, 0, y1, altY);
		if (elipse_y1 == -1.0)
			elipse_y1 = elipse_y2;
		if (elipse_y2 == -1.0)
			elipse_y2 = elipse_y1;
	} else {
		elipse_y1 = elipseY (inner_y, def, bound, acc, 0,
				     bound->elipse.min, bound->elipse.max);
		elipse_y2 = elipse_y1;
	}
	x2 = x1 = innerXfromY (elipse_y1, def, acc);
	if (elipse_y1 != elipse_y2)
		x2 = innerXfromY (elipse_y2, def, acc);
	if (acc->left.valid && boundedLe (inner_y, bound->left)) {
		xalt = intersectLine (inner_y, &acc->left);
		if (xalt < x2 && xalt < x1)
			x2 = xalt;
		if (xalt < x1)
			x1 = xalt;
	}
	if (acc->right.valid && boundedLe (inner_y, bound->right)) {
		xalt = intersectLine (inner_y, &acc->right);
		if (xalt < x2 && xalt < x1)
			x2 = xalt;
		if (xalt < x1)
			x1 = xalt;
	}
	*innerX1p = x1;
	*innerX2p = x2;
}

/*
 * this section computes the x value of the span at y 
 * intersected with the specified face of the elipse.
 *
 * this is the min/max X value over the set of normal
 * lines to the entire elipse,  the equation of the
 * normal lines is:
 *
 *     elipse_x h^2                   h^2
 * x = ------------ y + elipse_x (1 - --- )
 *     elipse_y w^2                   w^2
 *
 * compute the derivative with-respect-to elipse_y and solve
 * for zero:
 *    
 *       (w^2 - h^2) elipse_y^3 + h^4 y
 * 0 = - ----------------------------------
 *       h w elipse_y^2 sqrt (h^2 - elipse_y^2)
 *
 *            (   h^4 y     )
 * elipse_y = ( ----------  ) ^ (1/3)
 *            ( (h^2 - w^2) )
 *
 * The other two solutions to the equation are imaginary.
 *
 * This gives the position on the elipse which generates
 * the normal with the largest/smallest x intersection point.
 *
 * Now compute the second derivative to check whether
 * the intersection is a minimum or maximum:
 *
 *    h (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 * -  -------------------------------------------
 *          w y0^3 (sqrt (h^2 - y^2)) ^ 3
 *
 * as we only care about the sign,
 *
 * - (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 *
 * or (to use accelerators),
 *
 * y0^3 (h^2 - w^2) - h^2 y (3y0^2 - 2h^2) 
 *
 */

/*
 * computes the position on the elipse whose normal line
 * intersects the given scan line maximally
 */

double
hookElipseY (scan_y, def, bound, acc, left)
	double			scan_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
{
	double	ret;

	if (acc->h2mw2 == 0) {
		if (scan_y > 0 && !left || scan_y < 0 && left)
			return bound->elipse.min;
		return bound->elipse.max;
	}
	ret = (acc->h4 * scan_y) / (acc->h2mw2);
	if (ret >= 0)
		return pow (ret, 1.0/3.0);
	else
		return -pow (-ret, 1.0/3.0);
}

/*
 * computes the X value of the intersection of the
 * given scan line with the right side of the lower hook
 */

double
hookX (scan_y, def, bound, acc, left)
	double			scan_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	int			left;
{
	double	elipse_y, elipse_x, x, xalt;
	double	maxMin;

	if (def->w != def->h) {
		elipse_y = hookElipseY (scan_y, def, bound, acc, left);
		if (boundedLe (elipse_y, bound->elipse)) {
			/*
		 	 * compute the value of the second
		 	 * derivative
		 	 */
			maxMin = elipse_y*elipse_y*elipse_y * acc->h2mw2 -
		 	 acc->h2 * scan_y * (3 * elipse_y*elipse_y - 2*acc->h2);
			if ((left && maxMin > 0) || (!left && maxMin < 0)) {
				if (elipse_y == 0)
					return def->w + left ? -def->l/2 : def->l/2;
				x = (acc->h2 * scan_y - elipse_y * acc->h2mw2) *
					Sqrt (acc->h2 - elipse_y * elipse_y) /
			 		(def->h * def->w * elipse_y);
				return x;
			}
		}
	}
	if (left) {
		if (acc->left.valid && boundedLe (scan_y, bound->left)) {
			x = intersectLine (scan_y, &acc->left);
		} else {
			if (acc->right.valid)
				x = intersectLine (scan_y, &acc->right);
			else
				x = def->w - def->l/2;
		}
	} else {
		if (acc->right.valid && boundedLe (scan_y, bound->right)) {
			x = intersectLine (scan_y, &acc->right);
		} else {
			if (acc->left.valid)
				x = intersectLine (scan_y, &acc->left);
			else
				x = def->w - def->l/2;
		}
	}
	return x;
}

/*
 * generate the set of spans with
 * the given y coordinate
 */

arcSpan (y, def, bounds, acc)
	double			y;
	struct arc_def		*def;
	struct arc_bound	*bounds;
	struct accelerators	*acc;
{
	double	innerx1, innerx2, outerx1, outerx2;

	if (boundedLe (y, bounds->inner)) {
		/*
		 * intersection with inner edge
		 */
		innerXs (y, def, bounds, acc, &innerx1, &innerx2);
	} else {
		/*
		 * intersection with left face
		 */
		innerx2 = innerx1 = hookX (y, def, bounds, acc, 1);
		if (acc->right.valid && boundedLe (y, bounds->right))
		{
			innerx2 = intersectLine (y, &acc->right);
			if (innerx2 < innerx1)
				innerx1 = innerx2;
		}
	}
	if (boundedLe (y, bounds->outer)) {
		/*
		 * intersection with outer edge
		 */
		outerx1 = outerx2 = outerX (y, def, bounds, acc);
	} else {
		/*
		 * intersection with right face
		 */
		outerx2 = outerx1 = hookX (y, def, bounds, acc, 0);
		if (acc->left.valid && boundedLe (y, bounds->left))
 		{
			outerx2 = intersectLine (y, &acc->left);
			if (outerx2 < outerx1)
				outerx2 = outerx1;
		}
	}
	/*
	 * there are a very few cases when two spans will be
	 * generated.
	 */
	if (innerx1 < outerx1 && outerx1 < innerx2 && innerx2 < outerx2) {
		span (innerx1, outerx1);
		span (innerx2, outerx2);
	} else
		span (innerx1, outerx2);
}

/*
 * create whole arcs out of pieces.  This code is
 * very bad.
 */

static double	arcXcenter, arcYcenter;
static int	arcXoffset, arcYoffset;

static struct finalSpan	**finalSpans;
static int		finalMiny, finalMaxy;
static int		finalSize;

static int		nspans;		/* total spans, not just y coords */

struct finalSpan {
	struct finalSpan	*next;
	int			min, max;	/* x values */
};

static struct finalSpan    *freeFinalSpans, *tmpFinalSpan;

# define allocFinalSpan()   (freeFinalSpans ?\
				((tmpFinalSpan = freeFinalSpans), \
				 (freeFinalSpans = freeFinalSpans->next), \
				 (tmpFinalSpan->next = 0), \
				 tmpFinalSpan) : \
			     realAllocSpan ())

# define SPAN_CHUNK_SIZE    128

struct finalSpanChunk {
	struct finalSpan	data[SPAN_CHUNK_SIZE];
	struct finalSpanChunk	*next;
};

static struct finalSpanChunk	*chunks;

struct finalSpan *
realAllocSpan ()
{
	register struct finalSpanChunk	*newChunk;
	register struct finalSpan	*span;
	register int			i;

	newChunk = (struct finalSpanChunk *) Xalloc (sizeof (struct finalSpanChunk));
	if (!newChunk)
		return (struct finalSpan *) Xalloc (sizeof (struct finalSpan));
	newChunk->next = chunks;
	chunks = newChunk;
	freeFinalSpans = span = newChunk->data + 1;
	for (i = 1; i < SPAN_CHUNK_SIZE-1; i++) {
		span->next = span+1;
		span++;
	}
	span->next = 0;
	span = newChunk->data;
	span->next = 0;
	return span;
}

disposeFinalSpans ()
{
	struct finalSpanChunk	*chunk, *next;

	for (chunk = chunks; chunk; chunk = next) {
		next = chunk->next;
		Xfree (chunk);
	}
	chunks = 0;
	freeFinalSpans = 0;
}

fillSpans (pDrawable, pGC)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
{
	register struct finalSpan	*span;
	register DDXPointPtr		xSpan;
	register int			*xWidth;
	register int			i;
	register struct finalSpan	**f;
	register int			spany;
	DDXPointPtr			xSpans;
	int				*xWidths;

	if (nspans == 0)
		return;
	xSpan = xSpans = (DDXPointPtr) Xalloc (nspans * sizeof (DDXPointRec));
	xWidth = xWidths = (int *) Xalloc (nspans * sizeof (int));
	i = 0;
	f = finalSpans;
	for (spany = finalMiny; spany < finalMaxy; spany++, f++) {
		for (span = *f; span; span=span->next) {
			if (span->max <= span->min) {
				printf ("span width: %d\n", span->max-span->min);
				continue;
			}
			xSpan->x = span->min;
			xSpan->y = spany;
			++xSpan;
			*xWidth++ = span->max - span->min;
			++i;
		}
	}
	disposeFinalSpans ();
	Xfree (finalSpans);
	(*pGC->FillSpans) (pDrawable, pGC, i, xSpans, xWidths, TRUE);
	Xfree (xSpans);
	Xfree (xWidths);
	finalSpans = 0;
	finalMiny = 0;
	finalMaxy = 0;
	finalSize = 0;
	nspans = 0;
}

# define SPAN_REALLOC	1024

# define findSpan(y) ((finalMiny <= (y) && (y) < finalMaxy) ? \
			  &finalSpans[(y) - finalMiny] : \
			  realFindSpan (y))

struct finalSpan **
realFindSpan (y)
{
	struct finalSpan	**newSpans;
	int			newSize, newMiny, newMaxy;
	int			change;
	int			i;

	if (y < finalMiny || y >= finalMaxy) {
		if (y < finalMiny)
			change = finalMiny - y;
		else
			change = y - finalMaxy;
		if (change > SPAN_REALLOC)
			change += SPAN_REALLOC;
		else
			change = SPAN_REALLOC;
		newSize = finalSize + change;
		newSpans = (struct finalSpan **) Xalloc
 					(newSize * sizeof (struct finalSpan *));
		newMiny = finalMiny;
		newMaxy = finalMaxy;
		if (y < finalMiny)
			newMiny = finalMiny - change;
		else
			newMaxy = finalMaxy + change;
		if (finalSpans) {
			bcopy ((char *) finalSpans,
	 		       ((char *) newSpans) + (finalMiny-newMiny) * sizeof (struct finalSpan *),
			       finalSize * sizeof (struct finalSpan *));
			Xfree (finalSpans);
		}
		if ((i = finalMiny - newMiny) > 0)
			bzero (newSpans, i * sizeof (struct finalSpan *));
		if ((i = newMaxy - finalMaxy) > 0)
			bzero (newSpans + finalMaxy - newMiny,
			       i * sizeof (struct finalSpan *));
		finalSpans = newSpans;
		finalMaxy = newMaxy;
		finalMiny = newMiny;
		finalSize = newSize;
	}
	return &finalSpans[y - finalMiny];
}

newFinalSpan (y, xmin, xmax)
int		y;
register int	xmin, xmax;
{
	register struct finalSpan	*x;
	register struct finalSpan	**f;
	struct finalSpan		*oldx;
	struct finalSpan		*prev;

	f = findSpan (y);
	oldx = 0;
	for (;;) {
		prev = 0;
		for (x = *f; x; x=x->next) {
			if (x == oldx) {
				prev = x;
				continue;
			}
			if (x->min <= xmax && xmin <= x->max) {
				if (oldx) {
					oldx->min = min (x->min, xmin);
					oldx->max = max (x->max, xmax);
					if (prev)
						prev->next = x->next;
					else
						*f = x->next;
					--nspans;
				} else {
					x->min = min (x->min, xmin);
					x->max = max (x->max, xmax);
					oldx = x;
				}
				xmin = oldx->min;
				xmax = oldx->max;
				break;
			}
			prev = x;
		}
		if (!x)
			break;
	}
	if (!oldx) {
		x = allocFinalSpan ();
		x->min = xmin;
		x->max = xmax;
		x->next = *f;
		*f = x;
		++nspans;
	}
}

mirrorSppPoint (quadrant, sppPoint)
	int		quadrant;
	SppPointPtr	sppPoint;
{
	switch (quadrant) {
	case 0:
		break;
	case 1:
		sppPoint->x = -sppPoint->x;
		break;
	case 2:
		sppPoint->x = -sppPoint->x;
		sppPoint->y = -sppPoint->y;
		break;
	case 3:
		sppPoint->y = -sppPoint->y;
		break;
	}
	/*
	 * and translate to X coordinate system
	 */
	sppPoint->y = -sppPoint->y;
}

static double	spanY;

static int	quadrantMask;

span (left, right)
	double	left, right;
{
	register int	mask = quadrantMask, bit;
	register double	min, max, y;
	int	xmin, xmax, spany;

	while (mask) {
		bit = lowbit (mask);
		mask &= ~bit;
		switch (bit) {
		case 1:
			min = left;
			max = right;
			y = spanY;
			break;
		case 2:
			min = -right;
			max = -left;
			y = spanY;
			break;
		case 4:
			min = -right;
			max = -left;
			y = -spanY;
			break;
		case 8:
			min = left;
			max = right;
			y = -spanY;
			break;
		default:
			abort ();
		}
		xmin = (int) ceil (min + arcXcenter) + arcXoffset;
		xmax = (int) ceil (max + arcXcenter) + arcXoffset;
		spany = (int) (ceil (arcYcenter - y)) + arcYoffset;

		if (xmax > xmin)
			newFinalSpan (spany, xmin, xmax);
	}
}

/*
 * split an arc into pieces which are scan-converted
 * in the first-quadrant and mirrored into position.
 * This is necessary as the scan-conversion code can
 * only deal with arcs completely contained in the
 * first quadrant.
 */

drawArc (x0, y0, w, h, l, a0, a1, right, left)
	int	x0, y0, w, h, l, a0, a1;
	miArcFacePtr	right, left;	/* save end line points */
{
	struct arc_def		def;
	struct accelerators	acc;
	struct span		*result;
	int			startq, endq, curq;
	int			rightq, leftq, righta, lefta;
	miArcFacePtr		passRight, passLeft;
	int			q0, q1, mask;
	struct band {
		int	a0, a1;
		int	mask;
	}	band[5], sweep[20];
	int			bandno, sweepno;
	int			i, j, k;
	int			flipRight = 0, flipLeft = 0;			

	def.w = ((double) w) / 2;
	def.h = ((double) h) / 2;
	arcXoffset = x0;
	arcYoffset = y0;
	arcXcenter = def.w;
	arcYcenter = def.h;
	def.l = (double) l;
	if (a1 < a0)
		a1 += 360 * 64;
	startq = a0 / (90 * 64);
	endq = (a1-1) / (90 * 64);
	bandno = 0;
	curq = startq;
	for (;;) {
		switch (curq) {
		case 0:
			if (a0 > 90 * 64)
				q0 = 0;
			else
				q0 = a0;
			if (a1 < 360 * 64)
				q1 = min (a1, 90 * 64);
			else
				q1 = 90 * 64;
			if (curq == startq && a0 == q0) {
				righta = q0;
				rightq = curq;
			}
			if (curq == endq && a1 == q1) {
				lefta = q1;
				leftq = curq;
			}
			break;
		case 1:
			if (a1 < 90 * 64)
				q0 = 0;
			else
				q0 = 180 * 64 - min (a1, 180 * 64);
			if (a0 > 180 * 64)
				q1 = 90 * 64;
			else
				q1 = 180 * 64 - max (a0, 90 * 64);
			if (curq == startq && 180 * 64 - a0 == q1) {
				righta = q1;
				rightq = curq;
			}
			if (curq == endq && 180 * 64 - a1 == q0) {
				lefta = q0;
				leftq = curq;
			}
			break;
		case 2:
			if (a0 > 270 * 64)
				q0 = 0;
			else
				q0 = max (a0, 180 * 64) - 180 * 64;
			if (a1 < 180 * 64)
				q1 = 90 * 64;
			else
				q1 = min (a1, 270 * 64) - 180 * 64;
			if (curq == startq && a0 - 180*64 == q0) {
				righta = q0;
				rightq = curq;
			}
			if (curq == endq && a1 - 180 * 64 == q1) {
				lefta = q1;
				leftq = curq;
			}
			break;
		case 3:
			if (a1 < 270 * 64)
				q0 = 0;
			else
				q0 = 360 * 64 - min (a1, 360 * 64);
			q1 = 360 * 64 - max (a0, 270 * 64);
			if (curq == startq && 360 * 64 - a0 == q1) {
				righta = q1;
				rightq = curq;
			}
			if (curq == endq && 360 * 64 - a1 == q0) {
				lefta = q0;
				leftq = curq;
			}
			break;
		}
		band[bandno].a0 = q0;
		band[bandno].a1 = q1;
		band[bandno].mask = 1 << curq;
		bandno++;
		if (curq == endq)
			break;
		curq++;
		if (curq == 4) {
			a0 = 0;
			a1 -= 360 * 64;
			curq = 0;
			endq -= 4;
		}
	}
	sweepno = 0;
	for (;;) {
		q0 = 90 * 64;
		mask = 0;
		/*
		 * find left-most point
		 */
		for (i = 0; i < bandno; i++)
			if (band[i].a0 < q0) {
				q0 = band[i].a0;
				q1 = band[i].a1;
				mask = band[i].mask;
			}
		if (!mask)
			break;
		/*
		 * locate next point of change
		 */
		for (i = 0; i < bandno; i++)
			if (!(mask & band[i].mask)) {
				if (band[i].a0 == q0) {
					if (band[i].a1 < q1)
						q1 = band[i].a1;
					mask |= band[i].mask;
 				} else if (band[i].a0 < q1)
					q1 = band[i].a0;
			}
		/*
		 * create a new sweep
		 */
		sweep[sweepno].a0 = q0;
		sweep[sweepno].a1 = q1;
		sweep[sweepno].mask = mask;
		sweepno++;
		/*
		 * subtract the sweep from the affected bands
		 */
		for (i = 0; i < bandno; i++)
			if (band[i].a0 == q0) {
				band[i].a0 = q1;
				/*
				 * check if this band is empty
				 */
				if (band[i].a0 == band[i].a1)
					band[i].a1 = band[i].a0 = 90 * 64;
			}
	}
	computeAcc (&def, &acc);
	for (j = 0; j < sweepno; j++) {
		mask = sweep[j].mask;
		passRight = passLeft = 0;
 		if (mask & (1 << rightq)) {
			if (sweep[j].a0 == righta)
				passRight = right;
			if (sweep[j].a1 == righta) {
				passLeft = right;
				flipRight = 1;
			}
		}
		if (mask & (1 << leftq)) {
			if (sweep[j].a0 == lefta) {
				passRight = left;
				flipLeft = 1;
			}
			if (sweep[j].a1 == lefta)
				passLeft = left;
		}
		drawQuadrant (&def, &acc, sweep[j].a0, sweep[j].a1, mask, 
 			      passRight, passLeft);
	}
	/*
	 * mirror the coordinates generated for the
	 * faces of the arc
	 */
	if (right) {
		mirrorSppPoint (rightq, &right->clock);
		mirrorSppPoint (rightq, &right->center);
		mirrorSppPoint (rightq, &right->counterClock);
		if (flipRight) {
			SppPointRec	temp;

			temp = right->clock;
			right->clock = right->counterClock;
			right->counterClock = temp;
		}
	}
	if (left) {
		mirrorSppPoint (leftq,  &left->counterClock);
		mirrorSppPoint (leftq,  &left->center);
		mirrorSppPoint (leftq,  &left->clock);
		if (flipLeft) {
			SppPointRec	temp;

			temp = left->clock;
			left->clock = left->counterClock;
			left->counterClock = temp;
		}
	}
}

drawQuadrant (def, acc, a0, a1, mask, right, left)
	struct arc_def		*def;
	struct accelerators	*acc;
	int			a0, a1;
	int			mask;
	miArcFacePtr		right, left;
{
	struct arc_bound	bound;
	double			miny, maxy, y;
	int			minIsInteger;

	def->a0 = ((double) a0) / 64.0;
	def->a1 = ((double) a1) / 64.0;
	computeBound (def, &bound, acc, right, left);
	y = fmin (bound.inner.min, bound.outer.min);
	miny = ceil(y) +  def->w - floor (def->w);
	minIsInteger = y == miny;
	y = fmax (bound.inner.max, bound.outer.max);
	maxy = floor (y) +  def->w - floor (def->w);
	for (y = miny; y <= maxy; y = y + 1.0) {
		if (y == miny && minIsInteger)
			quadrantMask = mask & 0xc;
		else
			quadrantMask = mask;
		spanY = y;
		arcSpan (y, def, &bound, acc);
	}
	/*
	 * add the pixel at the top of the arc
	 */
	if (a1 == 90 * 64 && (mask & 1) && ((int) (def->w * 2 + def->l)) & 1) {
		quadrantMask = 1;
		spanY = def->h + def->l/2;
		span (0.0, 1.0);
	}
}

max (x, y)
{
	return x>y? x:y;
}

min (x, y)
{
	return x<y? x:y;
}

