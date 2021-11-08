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
#include "mistruct.h"
#include "regionstr.h"

Bool apcCreateWindow(pWin)
WindowPtr pWin;
{
    pWin->ClearToBackground = miClearToBackground;
    pWin->PaintWindowBackground = apcPaintWindow;
    pWin->PaintWindowBorder = apcPaintWindow;
    pWin->CopyWindow = apcCopyWindow;

    /* backing store stuff 
       is this ever called with backing store turned on ???
    */
    if ((pWin->backingStore == WhenMapped) ||
	(pWin->backingStore == Always))
    {
    }
    else
    {
    }
    return TRUE;
}

Bool apcDestroyWindow(pWin)
WindowPtr pWin;
{
    if (pWin->backingStore != NotUseful)
    {
	miFreeBackingStore(pWin);
    }
}

Bool apcMapWindow(pWindow)
WindowPtr pWindow;
{
}

Bool apcPositionWindow(pWin, x, y)
WindowPtr pWin;
int x, y;
{
}

Bool apcUnmapWindow(pWindow, x, y)
WindowPtr pWindow;
int x, y;
{
}

void 
apcCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    RegionPtr 			prgnDst;
    BoxPtr 			pbox;
    BoxPtr 			pboxTmp, pboxNext, pboxBase;
    BoxPtr                      pboxNew = NULL;
    int 			dx, dy;
    int 			i, nbox, reset_rops;
    gpr_$bitmap_desc_t 		srcBitmap;
    gpr_$window_t 		srcrect;
    gpr_$position_t		dst;
    gpr_$mask_32_t		full_mask;
    gpr_$raster_op_array_t      ops;
    status_$t			status;

    prgnDst = (* pWin->drawable.pScreen->RegionCreate)(NULL, 
						       pWin->borderClip->numRects);

    dx = ptOldOrg.x - pWin->absCorner.x;
    dy = ptOldOrg.y - pWin->absCorner.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(prgnDst, pWin->borderClip, prgnSrc);

    pbox = prgnDst->rects;
    nbox = prgnDst->numRects;
                             
    if ( dy < 0 ) {
        /* walk source bottom to top */

	if (nbox > 1) {
	    /* keep ordering in each band, reverse order of bands */
	    pboxNew = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    if(!pboxNew) {
	        DEALLOCATE_LOCAL(pboxNew);
	        return;
	        }
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox) {
	        while ((pboxNext >= pbox) && 
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
	        pboxTmp = pboxNext+1;
	        while (pboxTmp <= pboxBase) {
		    *pboxNew++ = *pboxTmp++;
	            }
	        pboxBase = pboxNext;
	        }  
	    pboxNew -= nbox;
	    pbox = pboxNew;
            }
        }

    if ( dx < 0 ) {
	/* walk source right to left */

	if (nbox > 1) { 
	    /* reverse order of rects ineach band */
	    pboxNew = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    pboxBase = pboxNext = pbox;
	    if(!pboxNew) {
	        DEALLOCATE_LOCAL(pboxNew);
	        return;
	        }
	    while (pboxBase < pbox+nbox) {
	        while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
	        pboxTmp = pboxNext;
	        while (pboxTmp != pboxBase) {
		    *pboxNew++ = *--pboxTmp;
	            }
	        pboxBase = pboxNext;
	        }
	    pboxNew -= nbox;
	    pbox = pboxNew;
	    }
        }
                          
    srcBitmap = (gpr_$bitmap_desc_t)(apDisplayData[(pWin->drawable.pScreen->myNum)].display_bitmap);
    apc_$set_bitmap( srcBitmap);    
                                               
    reset_rops = 0;
    gpr_$inq_raster_ops( ops, status);
    if ( ops[1] != gpr_$rop_src ) {
	full_mask = 0xFFFFFFFF;
	reset_rops = 1;
	gpr_$set_raster_op_mask( full_mask, gpr_$rop_src, status);
	}
    gpr_$set_clipping_active( false, status );

    for (i=0; i<nbox; i++, pbox++) {
        dst.x_coord = pbox->x1;
        dst.y_coord = pbox->y1;
        srcrect.window_base.x_coord = pbox->x1 + dx;
        srcrect.window_base.y_coord = pbox->y1 + dy;
        srcrect.window_size.x_size = pbox->x2 - pbox->x1;
        srcrect.window_size.y_size = pbox->y2 - pbox->y1;

        gpr_$pixel_blt( srcBitmap, srcrect, dst, status);
        }
        
    /* reset state */
    if (reset_rops) 
        gpr_$set_raster_op_mask( full_mask, ops[1], status);
    gpr_$set_clipping_active( true, status );

    gpr_$enable_direct_access( status);
    (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
    if (pboxNew) {
	DEALLOCATE_LOCAL(pboxNew);
        }
}



/* swap in correct PaintWindow* routine.  If we can use a fast output
routine (i.e. the pixmap is paddable to 32 bits), also pre-rotate a copy
of it in devPrivate.
*/
Bool
apcChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    int mask;
{
    register int index;

    while(mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
	  case CWBackingStore:
	      if (pWin->backingStore != NotUseful)
	      {
		  miInitBackingStore(pWin, apcSaveAreas, apcRestoreAreas);
	      }
	      else
	      {
		  miFreeBackingStore(pWin);
	      }
	      /*
	       * XXX: The changing of the backing-store status of a window
	       * is serious enough to warrant a validation, since otherwise
	       * the backing-store stuff won't work.
	       */
	      pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	      break;

	}
    }
}

