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
#include "windowstr.h"
#include "gcstruct.h"
#include "mi.h"

#include "qd.h"

/* due to validation...							*
 *	if this is called, pDstDrawable is SURELY a DRAWABLE_WINDOW.	*
 */
RegionPtr
qdCopyArea( pSrcDrawable, pDstDrawable,
				    pGC, srcx, srcy, width, height, dstx, dsty)
    register DrawablePtr pSrcDrawable;
    register DrawablePtr pDstDrawable;
    GCPtr	pGC;	/* composite clip region here is that of pDstDrawable */
    int		srcx, srcy;
    int		width, height;
    int		dstx, dsty;
{
    WindowPtr	psrcwin;
    WindowPtr	pdstwin;
    RegionPtr	pcompclip;
    RegionPtr	psavecclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    int		abssrcx, abssrcy;	/* screen coordinates */
    int 	absdstx, absdsty;	/* screen coordinates */

    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	tlspaca( pSrcDrawable, pDstDrawable,
				pGC, srcx, srcy, width, height, dstx, dsty);
	return NULL;	/* note that there are no exposures (?) */
    } else if (pSrcDrawable->type == UNDRAWABLE_WINDOW)
    {
	return miCopyArea( pSrcDrawable, pDstDrawable,
				pGC, srcx, srcy, width, height, dstx, dsty);
    }

    /* src->type must be a DRAWABLE_WINDOW: */
    psrcwin = (WindowPtr)pSrcDrawable;
    pdstwin = (WindowPtr)pDstDrawable;
    abssrcx = psrcwin->absCorner.x + srcx;
    abssrcy = psrcwin->absCorner.y + srcy;
    absdstx = pdstwin->absCorner.x + dstx;
    absdsty = pdstwin->absCorner.y + dsty;
    pcompclip = miRegionCreate( NULL, 1);

    if ( pGC->subWindowMode == IncludeInferiors)     /* used by qdCopyWindow */
	miRegionCopy( pcompclip, psrcwin->winSize);
    else
	miRegionCopy( pcompclip, psrcwin->clipList);
    miTranslateRegion( pcompclip, absdstx-abssrcx, absdsty-abssrcy);
    miIntersect( 
		pcompclip,
		pcompclip,
		((QDPrivGCPtr)pGC->devPriv)->pCompositeClip);

    ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip = pcompclip;

    tlbitblt( pGC,
	    absdstx, absdsty, 
	    width, height,
	    abssrcx, abssrcy);

    ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip = psavecclip;
    miRegionDestroy( pcompclip);
    /*
     * miHandleExposures wants window-relative coordinates
     */
    return miHandleExposures( pSrcDrawable, pDstDrawable,
				pGC, srcx, srcy, width, height, dstx, dsty, 0);
}
