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

#include "X.h"

#include "misc.h"
#include "windowstr.h"
#include "gcstruct.h"

/*
 * make this FillSpans instead?		XX
 */
void
qdPolyFillRectOddSize( pDrawable, pGC, nrect, prect)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrect;           /* number of rectangles to fill */
    xRectangle * prect;          /* Pointer to first rectangle to fill */
{
    WindowPtr	pwin = (WindowPtr)pDrawable;
    BoxPtr	pdestboxes;
    BoxPtr	pdb;
    int		nr, ok;
    void	tlPlaneStipple(), tlPlaneCopy(), tlbitblt();

    /*
     * This could be done by qdValidateGC	XX
     */
    if ( pDrawable->type != DRAWABLE_WINDOW)
    {
	miPolyFillRect( pDrawable, pGC, nrect, prect);
	return;
    }

    if ( nrect == 0)
	return;

    pdb = pdestboxes = (BoxPtr) alloca( nrect*sizeof(BoxRec));
    /*
    * turn the rectangles into Boxes
    */
    for ( nr=0; nr<nrect; nr++, pdb++, prect++)
    {
	pdb->x1 = prect->x;
	pdb->x2 = prect->x + (int)prect->width;
	pdb->y1 = prect->y;
	pdb->y2 = prect->y + (int)prect->height;
    }
    switch ( pGC->fillStyle) {
      case FillStippled:
	ok = tlOddSize( pwin, pGC,
			tlPlaneStipple, pGC->stipple, nrect, pdestboxes);
	if (!ok) {
	  /* XXX */
	}
	break;
      case FillOpaqueStippled:
	ok = tlOddSize( pwin, pGC,
			tlPlaneCopy, pGC->stipple, nrect, pdestboxes);
	if (!ok) {
	  /* XXX */
	}
	break;
      case FillTiled:
	ok = tlOddSize( pwin, pGC,
			tlbitblt, pGC->tile, nrect, pdestboxes);
	if (!ok)
	    for (nr = 0, prect -= nrect; nr < nrect; nr++, prect++) {
		if (prect->x >= pGC->patOrg.x &&
		    prect->x - pGC->patOrg.x + prect->width <= pGC->tile->width &&
		    prect->y >= pGC->patOrg.y &&
		    prect->y - pGC->patOrg.y + prect->height <= pGC->tile->height)
		    tlspaca(pGC->tile, pwin, pGC,
			    prect->x - pGC->patOrg.x, prect->y - pGC->patOrg.y,
			    prect->width, prect->height, prect->x, prect->y);
		else {
		    /* XXX */
		}
	    }
	break;
      case FillSolid:
	FatalError( "Should have called tldrawshapes code!\n");
	break;
    }
}

void
qdFillPolygon( pDrawable, pGC, shape, mode, npt, pptInit)
    DrawablePtr         pDrawable;
    register GCPtr      pGC;
    int                 shape, mode;
    register int        npt;
    DDXPointPtr         pptInit;
{
    DDXPointPtr         abspts;
    DDXPointPtr         closepts;

    if ( pDrawable->type != DRAWABLE_WINDOW || shape != Convex ||
		pGC->fillStyle != FillSolid)
    {
	miFillPolygon( pDrawable, pGC, shape, mode, npt, pptInit);
	return;
    }

    if ( mode == CoordModeOrigin)
	abspts = pptInit;
    else	/* CoordModePrevious */
    {
	register int	ip;

	abspts = (DDXPointPtr) alloca( npt * sizeof( DDXPointRec));
	if ( npt == 0)		/* make sure abspts[0] is valid */
	    return;
	abspts[ 0].x = pptInit[ 0].x;
	abspts[ 0].y = pptInit[ 0].y;
	for ( ip=1; ip<npt; ip++)
	{
	    abspts[ ip].x = abspts[ ip-1].x + pptInit[ ip].x;
	    abspts[ ip].y = abspts[ ip-1].y + pptInit[ ip].y;
	}
    }
    /* close the polygon if necessary */
    if (abspts[npt-1].x != abspts[0].x
	    || abspts[npt-1].y != abspts[0].y)	/* not closed */
    {
	register int	ip;

	closepts = (DDXPointPtr) alloca( (npt+1) * sizeof( DDXPointRec));
	if ( npt == 0)		/* make sure abspts[0] is valid */
	    return;
	for ( ip=0; ip<npt; ip++)
	{
	    closepts[ ip].x = abspts[ ip].x;
	    closepts[ ip].y = abspts[ ip].y;
	}
	closepts[ip].x = abspts[0].x;
	closepts[ip].y = abspts[0].y;
	npt++;	/* add duplicate point to end */
    }
    else
	closepts = abspts;
    tlconpoly( (WindowPtr)pDrawable, pGC, npt, closepts);
}
