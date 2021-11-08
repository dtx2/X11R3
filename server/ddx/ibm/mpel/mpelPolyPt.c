/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelPolyPt.c,v 6.5 88/10/25 01:17:34 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelPolyPt.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelPolyPt.c,v 6.5 88/10/25 01:17:34 kbg Exp $";
#endif

#include "X.h"
#include "misc.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "colormapst.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "region.h"

#include "OScompiler.h"

#include "ppc.h"
#include "ibmTrace.h"

#include "mpelHdwr.h"
#include "mpelFifo.h"

/* Cursor Stuff */
extern int mpelcursorSemaphore ;
extern int mpelCheckCursor() ;
extern void mpelReplaceCursor() ;

void
mpelPolyPoint( pDrawable, pGC, mode, npt, pptInit )
DrawablePtr	pDrawable ;
GCPtr		pGC ;
int		mode ;				/* Origin or Previous */
int		npt ;
xPoint		*pptInit ;
{
	register xPoint *ppt ;
	ppcPrivGC *devPriv = (ppcPrivGC *) ( pGC->devPriv ) ;
	register RegionPtr pRegion = devPriv->pCompositeClip ;
	int	cursor_saved ;

	TRACE( ("mpelPolyPoint(0x%x,0x%x,%d,%d,0x%x)\n",
		pDrawable, pGC, mode, npt, pptInit ) ) ;

	if ( pGC->alu == GXnoop || pRegion->numRects == 0 )
		return ;

	if ( pDrawable->type == DRAWABLE_PIXMAP ) {
		ppcPolyPoint( pDrawable, pGC, mode, npt, pptInit ) ;
		return ;
	}

	/* make pointlist origin relative */
	if ( mode == CoordModePrevious ) {
		register int nptTmp = npt ;
		for ( ppt = pptInit ; --nptTmp ; ) {
			ppt++ ;
			ppt->x += (ppt-1)->x ;
			ppt->y += (ppt-1)->y ;
		}
	}

	{ /* Validate & Translate the point list */
		register int (* PointInRegion)() = 
			pDrawable->pScreen->PointInRegion ;
        	register xPoint *mpt; /* xPoint is the same as mpelPoint */
		register const int xorg =
				( (WindowPtr) pDrawable )->absCorner.x ;
		register const int yorg =
				( (WindowPtr) pDrawable )->absCorner.y ;
		BoxRec box ; /* Scratch Space */

		/* NOTE: pGC->miTranslate is always TRUE in mpel */
		for ( ppt = pptInit ;
		      npt-- && (* PointInRegion)( pRegion,
						  ppt->x, ppt->y, &box ) ;
		      ppt++ ) {
			ppt->x += xorg ;
			ppt->y = ( MPEL_HEIGHT - 1 ) - ppt->y - yorg ;
		}
		for ( mpt = ppt ; npt-- > 0 ; ppt++ )
			if ( (* PointInRegion)( pRegion,
					ppt->x, ppt->y, &box ) ){
				mpt->x = ppt->x + xorg ;
				mpt->y = ( MPEL_HEIGHT - 1 ) - ppt->y - yorg ;
				mpt++ ;
			}

		if ( !( npt = mpt - pptInit) )
			return ;

	}
	/* If Cursor Is In The Way Remove It */
	cursor_saved = !mpelcursorSemaphore
		&& mpelCheckCursor(
			pRegion->extents.x1,
			pRegion->extents.y1,
			pRegion->extents.x2 - pRegion->extents.x1,
			pRegion->extents.y2 - pRegion->extents.y1
		) ;

	/* do the actual drawing */
	/* Check if the optimized rRop is valid */
	if ( devPriv->colorRrop.fillStyle == FillStippled
	  || devPriv->colorRrop.fillStyle == FillSolid ) {
		mpelSetALU( devPriv->colorRrop.alu ) ;
		mpelSetPlaneMask( devPriv->colorRrop.planemask ) ;
		MPELSetPolymarkerColor( devPriv->colorRrop.fgPixel ) ;
	}
	else {
		mpelSetALU( pGC->alu ) ;
		mpelSetPlaneMask( pGC->planemask ) ;
		MPELSetPolymarkerColor( pGC->fgPixel ) ;
	}
	MPELSetMarkerType( 1 ) ; /* Magic number for solid dots */
	MPELPolymarker( npt, pptInit ) ;

	if ( cursor_saved )
		mpelReplaceCursor() ;

	return ;
}
