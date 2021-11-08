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

#include <sys/types.h>

#include "X.h"
#include "windowstr.h"
#include "gcstruct.h"

/*
 * driver headers
 */
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"

#include "qdprocs.h"

#include "qd.h"
#include "tltemplabels.h"
#include "tl.h"

/*
 * Tile or Stipple everything inside the boxes with the GC's pixmap.
 * For now, write the pixmap into off-screen memory and send
 * multiple bitblt or pixelblt packets to tile the area.
 *
 * Do some terrible things to the argument GC in order to get the
 * hardware to do the necessary clipping
 *
 * (*BlitFunc) is required to clip to the GC composite clip region.
 */
int
tlOddSize( pWin, pGC, BlitFunc, pTile, nboxes, pboxes)
    WindowPtr	pWin;		/* destination window */
    GCPtr	pGC;		/* needed for planemask */
    void 	(*BlitFunc)();  /* tlbitblt, tlPlaneStipple, or tlPlaneCopy */
    PixmapPtr	pTile;		/* typically points to pGC's tile or stipple */
    int		nboxes;
    BoxPtr	pboxes;		/* window coordinates */
{
    RegionPtr	pSaveGCclip = ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip;
    RegionPtr	pargregion;		/* built from pboxes argument */
    RegionPtr	pcompregion;		/*intersection of arg and window clip*/
    BoxPtr	pcclip;			/* step through pcompregion */
    int		tiley;			/* off-screen tile address */
    int		ib;			/* index boxes */
    int		planemask;

    /*
     * store the pixmap off-screen
     */
    if ( ! (planemask = tlConfirmPixmap( pTile->devPrivate, pTile, &tiley)))
	return 0;

    pcompregion = qdRegionInit( NULL, 0);
    pargregion = qdRegionInit( pboxes, nboxes);
    miTranslateRegion( pargregion, pWin->absCorner.x, pWin->absCorner.y);
			/* make *pargregion absolute */
    miIntersect( pcompregion, pargregion,
				((QDPrivGCPtr)pGC->devPriv)->pCompositeClip);
    ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip = pcompregion;

    {
    int rx = pcompregion->extents.x1 - pWin->absCorner.x;
    int ry = pcompregion->extents.y1 - pWin->absCorner.y; /* origin in
						window coordinates */
    int width = pcompregion->extents.x2 - pcompregion->extents.x1;
    int height = pcompregion->extents.y2 - pcompregion->extents.y1;
    int ix, iy;	/* steps through the box; in window coordinate */

    /*
     * for each row of tiles, bump iy
     * subtract enough off of initial iy to align tile
     */
    for (   iy = ry - UMOD( ry-pGC->patOrg.y, pTile->height);
	    iy < ry + height;
	    iy += pTile->height)
    {
	/*
	 * for each column of tiles, bump ix
	 * subtract enough off of initial ix to align tile
	 */
	for (   ix = rx - UMOD( rx-pGC->patOrg.x, pTile->width);
		ix < rx + width;
		ix += pTile->width)
	{
	    /* these functions take absolute dst{x,y}. */
	    (* BlitFunc)( pGC,
			    pWin->absCorner.x + ix, pWin->absCorner.y + iy,
			    pTile->width, pTile->height,
			    0, tiley, planemask);
	}
    }
    }
    miRegionDestroy( pcompregion);
    miRegionDestroy( pargregion);
    ((QDPrivGCPtr)pGC->devPriv)->pCompositeClip = pSaveGCclip;
    return 1;
}
