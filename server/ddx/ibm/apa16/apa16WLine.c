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
/***********************************************************
		Copyright IBM Corporation 1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16WLine.c,v 9.1 88/10/17 14:45:17 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16WLine.c,v $ */

#ifndef lint
static char sccsid[] = "@(#)apa16wline.c	3.1 88/09/22 09:31:23";
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16WLine.c,v 9.1 88/10/17 14:45:17 erik Exp $";
#endif

#include "X.h"
#include "misc.h"
#include "windowstr.h"
#include "Xprotostr.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "miscstruct.h"
#include "pixmap.h"
#include "pixmapstr.h"
#include "mifpoly.h"
#include "mi.h"
#include "OScompiler.h"

#define IS_VERT(pp) (pp[0].x == pp[1].x)
#define IS_HORZ(pp) (pp[0].y == pp[1].y)

/* PPCWIDELINE - Public entry for PolyLine call
 * handles 1 segment wide lines specially.  Calls miWideLine
 * for all other cases.  Code taken from miOneSegment.
 */
void
apa16WideLine(pDrawable, pGC, mode, npt, pPts)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointRec pPts[];
{
    int		width = (pGC->lineWidth ? pGC->lineWidth : 1);
    DDXPointRec	tmpPts[4];
    DDXPointRec	otmpPts[2];
    SppPointRec SppPts[2];
    SppPointRec PolyPoints[4];
    xRectangle	tmpRect;

    if(pDrawable->type != DRAWABLE_WINDOW || npt > 2){
	miWideLine(pDrawable, pGC, mode, npt, pPts);
	return;
    }

    if(npt == 1)
	tmpPts[0] = tmpPts[1] = pPts[0];
    if(npt == 2){
	tmpPts[0] = pPts[0];
	tmpPts[1] = pPts[1];
    }

    /* make everything absolute */
    if (mode == CoordModePrevious)
    {
	tmpPts[1].x += tmpPts[0].x;
    	tmpPts[1].y += tmpPts[0].y;
    }
    otmpPts[0] = tmpPts[0];
    otmpPts[1] = tmpPts[1];

    if(tmpPts[0].x > tmpPts[1].x){
	DDXPointRec tp;

	tp = tmpPts[0];
	tmpPts[0] = tmpPts[1];
	tmpPts[1] = tp;
    }

    if(pGC->capStyle == CapProjecting)
    {
	if(PtEqual(pPts[0], pPts[1]))
	{
	    tmpPts[0].y -= width/2.0;
	    tmpPts[1].y += width/2.0;
	}
	else
	{
	    if(IS_VERT(tmpPts)){
		tmpPts[0].y -= width/2;
		tmpPts[1].y += width/2;
	    } else {
	    	if(IS_HORZ(tmpPts)){
			tmpPts[0].x -= width/2;
			tmpPts[1].x += width/2;
	    	} else {
			SppPts[0].x = (double)tmpPts[0].x;
			SppPts[0].y = (double)tmpPts[0].y;
			SppPts[1].x = (double)tmpPts[1].x;
			SppPts[1].y = (double)tmpPts[1].y;
	    		SppPts[0] = 
				miExtendSegment(SppPts[0], SppPts[1], width/2);
	    		SppPts[1] =
				miExtendSegment(SppPts[1], SppPts[0], width/2);
		}
	    }
	}
    }

    /* get points for rect */

    if(IS_VERT(tmpPts)){
	tmpRect.x = tmpPts[0].x - width/2;
	tmpRect.y = MIN(tmpPts[0].y, tmpPts[1].y);
	tmpRect.width = width;
	tmpRect.height = ABS(tmpPts[1].y - tmpPts[0].y);
    } else {
    	if(IS_HORZ(tmpPts)){
		tmpRect.y = tmpPts[0].y - width/2;
		tmpRect.x = tmpPts[0].x;
		tmpRect.height = width;
		tmpRect.width = tmpPts[1].x - tmpPts[0].x;
	} else {
		if(pGC->capStyle != CapProjecting){
			SppPts[0].x = (double)tmpPts[0].x;
			SppPts[0].y = (double)tmpPts[0].y;
			SppPts[1].x = (double)tmpPts[1].x;
			SppPts[1].y = (double)tmpPts[1].y;
		}
    		miGetPts(SppPts[0], SppPts[1], &PolyPoints[0], 
	  		&PolyPoints[1], &PolyPoints[2], &PolyPoints[3], width);

    		if(pGC->capStyle == CapRound) {
    			miFillSppPoly(pDrawable, pGC, 4, PolyPoints, 0, 0);
			miRoundCap(pDrawable, pGC,
				SppPts[0], SppPts[1], PolyPoints[0],
	  			PolyPoints[3], FirstEnd, 0, 0);
			miRoundCap(pDrawable, pGC,
				SppPts[1], SppPts[0], PolyPoints[2],
	  			PolyPoints[1], SecondEnd, 0, 0);
    		} else {
			tmpPts[0].x = ROUNDTOINT(PolyPoints[0].x);
			tmpPts[0].y = ROUNDTOINT(PolyPoints[0].y);
			tmpPts[1].x = ROUNDTOINT(PolyPoints[1].x);
			tmpPts[1].y = ROUNDTOINT(PolyPoints[1].y);
			tmpPts[2].x = ROUNDTOINT(PolyPoints[2].x);
			tmpPts[2].y = ROUNDTOINT(PolyPoints[2].y);
			tmpPts[3].x = ROUNDTOINT(PolyPoints[3].x);
			tmpPts[3].y = ROUNDTOINT(PolyPoints[3].y);
			(*pGC->FillPolygon)(pDrawable, pGC,
				Convex, CoordModeOrigin, 4, tmpPts);
		}
		return;
	}
    }

    (*pGC->PolyFillRect)(pDrawable, pGC, 1, &tmpRect);

    if(pGC->capStyle == CapRound)
    {
	register int w2 = width/2;
	SppPointRec pCenter, pEnd;
	SppPointRec pCorner, pOtherCorner;

	pCenter.x = (double)otmpPts[0].x;
	pCenter.y = (double)otmpPts[0].y;
	pEnd.x = (double)otmpPts[1].x;
	pEnd.y = (double)otmpPts[1].y;

	if(IS_VERT(otmpPts)){
		pCorner.x = pCenter.x + w2;
		pCorner.y = pCenter.y;
		pOtherCorner.x = pCenter.x - w2;
		pOtherCorner.y = pCenter.y;
	} else {
		pCorner.x = pCenter.x;
		pCorner.y = pCenter.y + w2;
		pOtherCorner.x = pCenter.x;
		pOtherCorner.y = pCenter.y - w2;
	}
	miRoundCap(pDrawable, pGC, pCenter, pEnd, pCorner,
	  pOtherCorner, FirstEnd, 0, 0);

	pCenter.x = (double)otmpPts[1].x;
	pCenter.y = (double)otmpPts[1].y;
	pEnd.x = (double)otmpPts[0].x;
	pEnd.y = (double)otmpPts[0].y;

	if(IS_VERT(otmpPts)){
		pCorner.x = pCenter.x + w2;
		pCorner.y = pCenter.y;
		pOtherCorner.x = pCenter.x - w2;
		pOtherCorner.y = pCenter.y;
	} else {
		pCorner.x = pCenter.x;
		pCorner.y = pCenter.y + w2;
		pOtherCorner.x = pCenter.x;
		pOtherCorner.y = pCenter.y - w2;
	}
	miRoundCap(pDrawable, pGC, pCenter, pEnd, pCorner,
	  pOtherCorner, SecondEnd, 0, 0);
    }
    return;
}
