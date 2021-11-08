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
#include "apc.h"
#include "misc.h"

void
apcZeroLine(dst, pgc, mode, nptInit, pptInit)
DrawablePtr dst;
GCPtr pgc;
int mode;
int nptInit;		/* number of points in polyline */
DDXPointRec *pptInit;	/* points in the polyline */
{
    int xorg, yorg;
    DDXPointRec *ppt;
    int npt;
    int i;		/* traditional name for loop counter */

    BoxPtr          pclip;
    short           clipx, temp, batchpts;
    apcPrivGCPtr   dpp = (apcPrivGCPtr) pgc->devPriv;
    gpr_$coordinate_t x[500], y[500];
    status_$t       status;

    ppt = pptInit;
    npt = nptInit;
    if (pgc->miTranslate) {
        if (dst->type == DRAWABLE_WINDOW) {
	    xorg = ((WindowPtr)dst)->absCorner.x;
	    yorg = ((WindowPtr)dst)->absCorner.y;
        }
        else {
	    xorg = 0;
	    yorg = 0;
        }

        if (mode == CoordModeOrigin) {
            for (i = 0; i<npt; i++) {    
	            ppt->x += xorg;
	            ppt++->y += yorg;
            }
        }
        else {
	    ppt->x += xorg;
	    ppt++->y += yorg;
	    for (i = 1; i<npt; i++) {
	        ppt->x += (ppt-1)->x;
	        ppt->y += (ppt-1)->y;
	        ppt++;
            }
        }
    }
    else {
	if (mode == CoordModePrevious) {
	    ppt++;
	    for (i = 1; i<npt; i++) {
	        ppt->x += (ppt-1)->x;
	        ppt->y += (ppt-1)->y;
	        ppt++;
            }
        }
    }
                  
    if (dst->type == DRAWABLE_WINDOW) {
	apc_$set_bitmap(apDisplayData[dst->pScreen->myNum].display_bitmap);
    }
    else {
	apc_$set_bitmap(((apcPrivPMPtr)(((PixmapPtr)dst)->devPrivate))->bitmap_desc);
    }

    pclip = dpp->pCompositeClip->rects;
    for ( clipx = 0; clipx < dpp->pCompositeClip->numRects; clipx++ ) {
        if (dpp->pCompositeClip->numRects != 1) {
            gpr_$window_t gwin;
            gwin.window_base.x_coord = pclip->x1;
            gwin.window_base.y_coord = pclip->y1;
            gwin.window_size.x_size = pclip->x2 - gwin.window_base.x_coord;
            gwin.window_size.y_size = pclip->y2 - gwin.window_base.y_coord;
            gpr_$set_clip_window( gwin, status );
            pclip++;
        }

        batchpts = npt;
        ppt = pptInit;
        gpr_$move((short)(ppt->x), (short)(ppt->y), status);
        ppt++;
        while (batchpts != 0) {
            temp=500;
            if (batchpts>500) batchpts-=500;
            else {
                temp=batchpts;
                batchpts=0;
	    }
            for (i=0; i<(temp-1); i++) {
                x[i] = ppt->x;
                y[i] = ppt->y;
                ppt++;
            }
            gpr_$polyline(x, y, (short)(temp-1), status);
        }
    }
} 

void
apcPolySegment(pDraw, pGC, nseg, pSegs)
    DrawablePtr pDraw;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    int         xorg, yorg;
    int         i;
    xSegment	*pSegtemp;
    BoxPtr          pclip;
    short           clipx, temp, batchsegs;
    apcPrivGCPtr    dpp = (apcPrivGCPtr) pGC->devPriv;
    gpr_$coordinate_t x[500], y[500];
    status_$t       status;

    if (pGC->miTranslate) {
        if (pDraw->type == DRAWABLE_WINDOW) {
	    xorg = ((WindowPtr)pDraw)->absCorner.x;
	    yorg = ((WindowPtr)pDraw)->absCorner.y;
        }
        else {
	    xorg = 0;
	    yorg = 0;
        }
    }

    if (pDraw->type == DRAWABLE_WINDOW) {
	apc_$set_bitmap(apDisplayData[pDraw->pScreen->myNum].display_bitmap);
    }
    else {
	apc_$set_bitmap(((apcPrivPMPtr)(((PixmapPtr)pDraw)->devPrivate))->bitmap_desc);
    }

    pclip = dpp->pCompositeClip->rects;
    for ( clipx = 0; clipx < dpp->pCompositeClip->numRects; clipx++ ) {
        if (dpp->pCompositeClip->numRects != 1) {
            gpr_$window_t gwin;
            gwin.window_base.x_coord = pclip->x1;
            gwin.window_base.y_coord = pclip->y1;
            gwin.window_size.x_size = pclip->x2 - gwin.window_base.x_coord;
            gwin.window_size.y_size = pclip->y2 - gwin.window_base.y_coord;
            gpr_$set_clip_window( gwin, status );
            pclip++;
        }

        batchsegs = nseg;
	pSegtemp = pSegs;
        while (batchsegs != 0) {
            temp=250;
            if (batchsegs>250) batchsegs-=250;
            else {
                temp=batchsegs;
                batchsegs=0;
	    }
            for (i=0; i<temp; i++) {
                x[i<<1] = pSegtemp->x1 + xorg;
                y[i<<1] = pSegtemp->y1 + yorg;
                x[(i<<1)+1] = pSegtemp->x2 + xorg;
                y[(i<<1)+1] = pSegtemp->y2 + yorg;
                pSegtemp++;
            }
            gpr_$multiline(x, y, (short)(temp<<1), status);
        }
    }
}
