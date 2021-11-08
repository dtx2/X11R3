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
/* $XConsortium: miwidedash.c,v 1.23 88/10/15 17:49:34 keith Exp $ */
/* Author: Todd "Mr. Wide Line" Newman */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "mistruct.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "mifpoly.h"

#define GCVALSALU	0
#define GCVALSFORE	1
#define GCVALSBACK	2
#define GCVALSWIDTH	3
#define GCVALSCAPSTYLE	4
#define GCVALSJOINSTYLE 5
#define GCVALSMASK	(GCFunction | GCForeground | GCBackground | \
			 GCLineWidth | GCCapStyle | GCJoinStyle)
static int gcvals[] = {GXcopy, 1, 0, 0, 0, 0};

/* Neither Fish nor Fowl, it's a Wide Dashed Line. */
/* Actually, wide, dashed lines Are pretty foul. (You knew that was coming,
 * didn't you.) */

/* MIWIDEDASH -- Public entry for PolyLine Requests when the GC speicifies
 * that we must be dashing (All Hail Errol Flynn)
 *
 * We must use the raster op to decide how whether we will draw directly into
 * the Drawable or squeegee bits through a scratch pixmap to avoid the dash
 * interfering with itself.
 * 
 * miDashLine will convert the poly line we were called with into the
 * appropriate set of line segments. 
 * Based on the dash style we then draw the segments. For OnOff dashes we
 * draw every other segment and cap each segment.  For DoubleDashes, we
 * draw every other segment starting with the first in the foreground color,
 * then draw every other segment starting with the second in the background
 * color.  Then we cap the first and last segments "by hand."
 */
void
miWideDash(pDraw, pGC, mode, npt, pPtsIn)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr pPtsIn;
{

    SppPointPtr	pPts, ppt;
    int		nseg, which, whichPrev, i, j, xOrg, yOrg, width,
    		fTricky, xMin, xMax, yMin, yMax, dxi, dyi;
    unsigned long oldfore = pGC->fgPixel, newfore = pGC->bgPixel;
    miDashPtr	dashes;
    double	dy, dx, m;
    Bool	IsDoubleDash = (pGC->lineStyle == LineDoubleDash),
		fXmajor;
    SppPointRec pt, PointStash[4], PolyPoints[4];
    DDXPointPtr pPtIn;
    DrawablePtr	pDrawTo, pDrawToBG;
    GCPtr	pGCTo;
    int         initerr = 0, edges_match;
    int         gcinvert[3];	/* for swapping fg and bg of gc */

    m = EPSILON;
    if (mode == CoordModePrevious)
    {
	DDXPointPtr pptT;
	int nptTmp;

	pptT = pPtsIn + 1;
	nptTmp = npt - 1;
	while (nptTmp--)
	{
	    pptT->x += (pptT-1)->x;
	    pptT->y += (pptT-1)->y;
	    pptT++;
	}
    }

    width = (pGC->lineWidth ? pGC->lineWidth : 1);
    switch(pGC->alu)
    {
      case GXnoop:
	return;
	break;
      case GXclear:		/* 0 */
      case GXcopy:		/* src */
      case GXcopyInverted:	/* NOT src */
      case GXset:		/* 1 */
	fTricky = FALSE;
        xOrg = yOrg = 0;
	pDrawToBG = pDrawTo = pDraw;
	pGCTo = pGC;
	break;
      case GXand:		/* src AND dst */
      case GXandReverse:	/* src AND NOT dst */
      case GXandInverted:	/* NOT src AND dst */
      case GXxor:		/* src XOR dst */
      case GXor	:		/* src OR dst */
      case GXnor:		/* NOT src AND NOT dst */
      case GXequiv:		/* NOT src XOR dst */
      case GXinvert:		/* NOT dst */
      case GXorReverse:		/* src OR NOT dst */
      case GXorInverted:	/* NOT src OR dst */
      case GXnand:		/* NOT src OR NOT dst */
	fTricky = TRUE;
	yMin = yMax = pPtsIn[0].y;
	xMin = xMax = pPtsIn[0].x;

	for (i = 1; i < npt; i++)
	{
	    xMin = min(xMin, pPtsIn[i].x);
	    xMax = max(xMax, pPtsIn[i].x);
	    yMin = min(yMin, pPtsIn[i].y);
	    yMax = max(yMax, pPtsIn[i].y);
	}
	xOrg = xMin - D2SECANT * width;
	yOrg = yMin - D2SECANT * width;
	dxi = xMax - xMin + D2SECANT * width * 2.0 + 1;
	dyi = yMax - yMin + D2SECANT * width * 2.0 + 1;
	pDrawTo = (DrawablePtr) (*pDraw->pScreen->CreatePixmap)
	  (pDraw->pScreen, dxi, dyi, 1);
	if (IsDoubleDash)
	    pDrawToBG = (DrawablePtr) (*pDraw->pScreen->CreatePixmap)
		(pDraw->pScreen, dxi, dyi, 1);
	pGCTo =  GetScratchGC(1, pDraw->pScreen);
	gcvals[GCVALSWIDTH] = width;
	gcvals[GCVALSCAPSTYLE] = pGC->capStyle;
	gcvals[GCVALSJOINSTYLE] = pGC->joinStyle;
	DoChangeGC(pGCTo, GCVALSMASK, gcvals, 0);
	if (IsDoubleDash) {
	    ValidateGC(pDrawToBG, pGCTo);
	    miClearDrawable(pDrawToBG, pGCTo);
	}
	ValidateGC(pDrawTo, pGCTo);
	miClearDrawable(pDrawTo, pGCTo);
    }
    gcinvert[0] = newfore;	/* original background */
    gcinvert[1] = oldfore;	/* original foreground */
    gcinvert[2] = newfore;	/* original background */

    dashes = miDashLine(npt, pPtsIn,
               pGC->numInDashList, pGC->dash, pGC->dashOffset, &nseg);
    if (nseg < 2)
    {
	/* note we only need one scratch pixmap here; only one color */
	if ((!fTricky) && dashes[0].which == ODD_DASH)
	{			/* swap colors for background dash */
	    DoChangeGC(pGCTo, GCForeground | GCBackground, gcinvert);
	    ValidateGC(pDrawTo, pGCTo);
	}
	PointStash[0].x = (double) dashes[0].pt.x;
	PointStash[0].y = (double) dashes[0].pt.y;
	if (nseg == 1)
	{
	    dx = (double) (dashes[1].pt.x - dashes[0].pt.x);
	    dy = (double) (dashes[1].pt.y - dashes[0].pt.y);
	    fXmajor = (fabs(dx) > fabs(dy));
	    if(fXmajor)
		m = sign(dy)/(!ISZERO(dx) ? 2.0*fabs(dx) : EPSILON);
	    else
		m = sign(dx)/(!ISZERO(dy) ? 2.0*fabs(dy) : EPSILON);
	    PointStash[1].x = (double) dashes[1].pt.x +
		(fXmajor ? 0.0 : (double)(dashes[1].e)*m);
	    PointStash[1].y = (double) dashes[1].pt.y +
		(fXmajor ? (double)(dashes[1].e)*m : 0.0);
	}
	miOneSegWide(pDrawTo, pGCTo, nseg+1, PointStash, TRUE, xOrg, yOrg);
	if(fTricky) {
	    if (pGC->miTranslate && (pDraw->type == DRAWABLE_WINDOW) )
	    {
		xOrg += ((WindowPtr)pDraw)->absCorner.x;
		yOrg += ((WindowPtr)pDraw)->absCorner.y;
	    }
	    if (dashes[0].which == ODD_DASH)
		DoChangeGC(pGC, GCForeground | GCBackground, gcinvert);
	    ValidateGC(pDraw, pGC);
	    (*pGC->PushPixels)(pGC, pDrawTo, pDraw, dxi, dyi, xOrg, yOrg);
	    (*pDraw->pScreen->DestroyPixmap)(pDrawTo);
	    if (IsDoubleDash)
		(*pDraw->pScreen->DestroyPixmap)(pDrawToBG);
	    FreeScratchGC(pGCTo);
	}
	if (dashes[0].which == ODD_DASH)
	{
	    DoChangeGC(pGC, GCForeground | GCBackground, &gcinvert[1]);
	    ValidateGC(pDraw, pGC);	/* just in case */
	}
	Xfree(dashes);
	return;
    }
    if(!(pPts = (SppPointPtr) Xalloc((nseg + 1) * sizeof(SppPointRec))))
    {
	if(fTricky) {
	    (*pDraw->pScreen->DestroyPixmap)(pDrawTo);
	    if (IsDoubleDash)
		(*pDraw->pScreen->DestroyPixmap)(pDrawToBG);
	    FreeScratchGC(pGCTo);
	}
	return;
    }
    ppt = pPts;
    pPtIn = pPtsIn;
    whichPrev = EVEN_DASH;

    edges_match = npt>1 && nseg>=2 && PtEqual(pPtsIn[0], pPtsIn[npt-1]);

    j = 0;
    for(i = 0; i <= nseg; i++)
    {
	if(dashes[i].newLine)
	{
	    /* calculate slope of the line */
	    dx = (double) ((pPtIn + 1)->x - pPtIn->x);
	    dy = (double) ((pPtIn + 1)->y - pPtIn->y);
	    pPtIn++;
	    /* calculate initial error term */
	    initerr=dashes[i].e;
	    /* use slope of line to figure out how to use error term */
	    fXmajor = (fabs(dx) > fabs(dy));
	    if(fXmajor)
		m = sign(dy)/(!ISZERO(dx) ? 2.0*fabs(dx) : EPSILON);
	    else
		m = sign(dx)/(!ISZERO(dy) ? 2.0*fabs(dy) : EPSILON);
	}
	/* Add this point to our list, adjusting the error term as needed */
	ppt->x = (double) dashes[i].pt.x +
	    (fXmajor ? 0.0 : (double)(dashes[i].e-initerr)*m);
	ppt->y = (double) dashes[i].pt.y +
	    (fXmajor ? (double)(dashes[i].e-initerr)*m : 0.0);

	if (i < 2 || i > nseg - 2)
	    PointStash[j++] = *ppt;
	ppt++;
	which = dashes[i].which;
	if((which != whichPrev) || (i == nseg))
	{
	    if (whichPrev == EVEN_DASH)
	    {
		/* Display the collect line */
		(*pGC->LineHelper)(pDrawTo, pGCTo, !IsDoubleDash,
				   ppt - pPts, pPts, xOrg, yOrg);
	    }
	    /* Reset the line and start a new dash */
	    pPts[0] = ppt[-1];
	    ppt = &pPts[1];
	    whichPrev = which;
	}

    }

    if (edges_match)
    { /* Join the first and last line segment by drawing last dash and
	 first dash.  If last dash is odd, it will later be overwritten */
	SppPointRec joinPts[3];
	joinPts[0].x = PointStash[2].x;
	joinPts[0].y = PointStash[2].y;
	joinPts[1].x = PointStash[0].x;
	joinPts[1].y = PointStash[0].y;
	joinPts[2].x = PointStash[1].x;
	joinPts[2].y = PointStash[1].y;
	/* the color choice here is quite arbitrary.  read the protocol. */
	if (IsDoubleDash) 
	{
	    if (dashes[0].which == ODD_DASH && dashes[nseg].which == ODD_DASH)
		/* if both ends odd and double dashing, dash the bg */
	    {
		if (!fTricky)
		    DoChangeGC(pGCTo, GCForeground | GCBackground,
			       gcinvert, 0); /* invert the colors */
		ValidateGC(pDrawToBG, pGCTo);
		(*pGC->LineHelper)(pDrawToBG, pGCTo, FALSE, 3, joinPts,
				   xOrg, yOrg);
	    } else		/* otherwise, use fg for no real reason */
		(*pGC->LineHelper)(pDrawTo, pGCTo, FALSE, 3, joinPts,
				   xOrg, yOrg);
	} else			/* on-off dashed */
	    if (dashes[0].which == EVEN_DASH &&
		dashes[nseg].which == EVEN_DASH) /* both ends are "on" */
		(*pGC->LineHelper)(pDrawTo, pGCTo, FALSE, 3, joinPts,
				   xOrg, yOrg);
    }
    
    if(IsDoubleDash)
    {
        ppt = pPts;
        pPtIn = pPtsIn;
	whichPrev = EVEN_DASH;

	if (nseg == 2) {
	    PointStash[3] = PointStash[2];
	    PointStash[2] = PointStash[1];
	}
	if (!edges_match) {
	    /* cap first (and maybe the last) line segment(s) appropriately */
	    if(pGC->capStyle == CapProjecting)
	    {
		if (dashes[0].which == EVEN_DASH)
		{
		    pt = miExtendSegment(PointStash[0], PointStash[1],
					 width/2);
		    miGetPts(pt, PointStash[0],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miFillSppPoly(pDrawTo, pGCTo, 4, PolyPoints, -xOrg, -yOrg, 0.0, 0.0);
		}
		if(dashes[nseg].which == EVEN_DASH)
		{
		    pt = miExtendSegment(PointStash[3], PointStash[2],
					 width/2);
		    miGetPts(pt, PointStash[3],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miFillSppPoly(pDrawTo, pGCTo, 4, PolyPoints, -xOrg, -yOrg, 0.0, 0.0);
		}
	    } else if (pGC->capStyle == CapRound)
	    {
		if (dashes[0].which == EVEN_DASH)
		{
		    miGetPts(PointStash[0], PointStash[1],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miRoundCap(pDrawTo, pGCTo, PointStash[0], PointStash[1],
			       PolyPoints[3], PolyPoints[0], FirstEnd,
			       xOrg, yOrg, 0.0, 0.0);
		}
		if(dashes[nseg].which == EVEN_DASH)
		{
		    miGetPts(PointStash[3], PointStash[2],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miRoundCap(pDrawTo, pGCTo, PointStash[3], PointStash[2],
			       PolyPoints[3], PolyPoints[0], SecondEnd,
			       xOrg, yOrg, 0.0, 0.0);
		}
	    }
	    if (!fTricky)
		/* invert the colors to bg in front */
		DoChangeGC(pGCTo, GCForeground | GCBackground, gcinvert, 0);
	    ValidateGC(pDrawToBG, pGCTo);
	} else {
	    if (!fTricky && (dashes[0].which == EVEN_DASH ||
			     dashes[nseg].which == EVEN_DASH))
		/* invert the colors to bg in front */
		DoChangeGC(pGCTo, GCForeground | GCBackground, gcinvert, 0);
	    ValidateGC(pDrawToBG, pGCTo);
	}

	for(i = 0; i <= nseg; i++)
	{
	    if(dashes[i].newLine)
	    {
		/* calculate slope of the line */
		dx = (double) ((pPtIn + 1)->x - pPtIn->x);
		dy = (double) ((pPtIn + 1)->y - pPtIn->y);
		pPtIn++;
		/* calculate initial error term */
		initerr=dashes[i].e;
		/* use slope of line to figure out how to use error term */
		fXmajor = (fabs(dx) > fabs(dy));
		if(fXmajor)
		    m = sign(dy)/(!ISZERO(dx) ? 2.0*fabs(dx) : EPSILON);
		else
		    m = sign(dx)/(!ISZERO(dy) ? 2.0*fabs(dy) : EPSILON);
	    }
	    /* Add this point to our list */
	    ppt->x = (double) dashes[i].pt.x +
		(fXmajor ? 0.0 : (double)(dashes[i].e-initerr)*m);
	    ppt->y = (double) dashes[i].pt.y +
		(fXmajor ? (double)(dashes[i].e-initerr)*m : 0.0);
	    ppt++;
	    which = dashes[i].which;
	    if((which != whichPrev) || (i == nseg))
	    {
		if(whichPrev == ODD_DASH)
		{
		    /* Display the collected line */
		    (*pGC->LineHelper)(pDrawToBG, pGCTo, FALSE,
				       ppt - pPts, pPts, xOrg, yOrg);
		}
		/* Reset the line  and start a new dash */
		pPts[0] = ppt[-1];
		ppt = &pPts[1];
		whichPrev = which;
	    }
	}

	/* cap the last line segments appropriately */
	if (!edges_match) {
	    if(pGC->capStyle == CapProjecting)
	    {
		if(dashes[0].which == ODD_DASH)
		{
		    pt = miExtendSegment(PointStash[0], PointStash[1],
					 width/2);
		    miGetPts(pt, PointStash[0],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miFillSppPoly(pDrawToBG, pGCTo, 4, PolyPoints, -xOrg,
				  -yOrg, 0.0, 0.0);
		}
		if(dashes[nseg].which == ODD_DASH)
		{
		    pt = miExtendSegment(PointStash[3], PointStash[2],
					 width/2);
		    miGetPts(pt, PointStash[3],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miFillSppPoly(pDrawToBG, pGCTo, 4, PolyPoints, -xOrg,
				  -yOrg, 0.0, 0.0);
		}
	    } else if (pGC->capStyle == CapRound) {
		if(dashes[0].which == ODD_DASH)
		{
		    miGetPts(PointStash[0], PointStash[1],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miRoundCap(pDrawToBG, pGCTo, PointStash[0], PointStash[1],
			       PolyPoints[3], PolyPoints[0], FirstEnd,
			       xOrg, yOrg, 0.0, 0.0);
		}
		if(dashes[nseg].which == ODD_DASH)
		{
		    if (dashes[nseg].which == ODD_DASH)
		    miGetPts(PointStash[3], PointStash[2],
			     &PolyPoints[0], &PolyPoints[1], &PolyPoints[2],
			     &PolyPoints[3], width);
		    miRoundCap(pDrawToBG, pGCTo, PointStash[3], PointStash[2],
			       PolyPoints[3], PolyPoints[0], SecondEnd,
			       xOrg, yOrg, 0.0, 0.0);
		}
	    }
	}
	if (!fTricky) {
	    /* gcinvert[1] is the original fg, and [2] is the orig. bg */
	    DoChangeGC(pGCTo, GCForeground | GCBackground, &gcinvert[1], 0);
	    ValidateGC(pDrawTo, pGCTo);
	}
    }
    if(fTricky)
    {
	unsigned long alu = GXandInverted;
	
	if (pGC->miTranslate && (pDraw->type == DRAWABLE_WINDOW) )
	{
	    xOrg += ((WindowPtr)pDraw)->absCorner.x;
	    yOrg += ((WindowPtr)pDraw)->absCorner.y;
	}

	/* erase the bg mask bits from the fg mask bits for doubledashes */
	if (IsDoubleDash) {
	    DoChangeGC(pGCTo, GCFunction, &alu, 0);
	    ValidateGC(pDrawTo, pGCTo);
	    (*pGCTo->PushPixels)(pGCTo, pDrawToBG, pDrawTo, dxi, dyi, 0, 0);
	}

	ValidateGC(pDraw, pGC);
	(*pGC->PushPixels)(pGC, pDrawTo, pDraw, dxi, dyi, xOrg, yOrg);

	if (IsDoubleDash) {
	    /* gcinv[0] is the original bg and [1] the orig. fg */
	    DoChangeGC(pGC, GCForeground | GCBackground, gcinvert, 0);
	    ValidateGC(pDraw, pGC);
	    (*pGC->PushPixels)(pGC, pDrawToBG, pDraw, dxi, dyi, xOrg, yOrg);
	    /* gcinv[1] is the original fg and [2] the orig. bg */
	    DoChangeGC(pGC, GCForeground | GCBackground, &gcinvert[1], 0);
	    ValidateGC(pDraw, pGC);
 	}
	(*pDraw->pScreen->DestroyPixmap)(pDrawTo);
	if (IsDoubleDash)
	    (*pDraw->pScreen->DestroyPixmap)(pDrawToBG);
        FreeScratchGC(pGCTo);
    }
    Xfree(dashes);
    Xfree(pPts);
}
