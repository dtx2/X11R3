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
		Copyright IBM Corporation 1987,1988

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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Win.c,v 9.1 88/10/17 14:45:29 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Win.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Win.c,v 9.1 88/10/17 14:45:29 erik Exp $";
static char sccsid[] = "@(#)apa16win.c	3.1 88/09/22 09:31:21";
#endif


#include "X.h"
#include "Xmd.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mfb.h"
#include "mistruct.h"
#include "regionstr.h"

#include "OScompiler.h"
#include "ibmTrace.h"

#include "apa16Decls.h"
#include "apa16Hdwr.h"

extern WindowRec WindowTable[];

Bool apa16CreateWindow(pWin)
WindowPtr pWin;
{
    mfbPrivWin *pPrivWin;

    TRACE(("apa16CreateWindow( pWin= 0x%x )\n",pWin));

    pWin->ClearToBackground = miClearToBackground;
    pWin->PaintWindowBackground = mfbPaintWindowNone;
    pWin->PaintWindowBorder = mfbPaintWindowPR;
    pWin->CopyWindow = apa16CopyWindow;
    pPrivWin = (mfbPrivWin *)Xalloc(sizeof(mfbPrivWin));
    pWin->devPrivate = (pointer)pPrivWin;
    pPrivWin->pRotatedBorder = NullPixmap;
    pPrivWin->pRotatedBackground = NullPixmap;
    pPrivWin->fastBackground = 0;
    pPrivWin->fastBorder = 0;
    return TRUE;
}

/* UNCLEAN!
   this code calls the bitblt helper code directly.

   mfbCopyWindow copies only the parts of the destination that are
visible in the source.
*/


void 
apa16CopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    RegionPtr prgnDst;
    register BoxPtr pbox;
    register int dx, dy;
    register int i, nbox;
    WindowPtr pwinRoot;

    TRACE(("apa16CopyWindow(pWin= 0x%x, ptOldOrg= 0x%x, prgnSrc= 0x%x)\n",
							pWin,ptOldOrg,prgnSrc));

    pwinRoot = &WindowTable[pWin->drawable.pScreen->myNum];

    prgnDst = (* pWin->drawable.pScreen->RegionCreate)(NULL, 
						       pWin->borderClip->numRects);

    dx = ptOldOrg.x - pWin->absCorner.x;
    dy = ptOldOrg.y - pWin->absCorner.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(prgnDst, pWin->borderClip, prgnSrc);

    pbox = prgnDst->rects;
    nbox = prgnDst->numRects;
    if(!(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL( prgnDst->numRects *
      sizeof(DDXPointRec))))
	return;
    ppt = pptSrc;

    for (i=0; i<nbox; i++, ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }

    apa16DoBitblt(pwinRoot, pwinRoot, GXcopy, prgnDst, pptSrc);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
}


/***====================================================================***/

void
apa16PaintWindow32(pWin, pRegion, what)
WindowPtr pWin;
RegionPtr pRegion;
int what;
{
int		nbox;
BoxPtr		pBox;
PixmapPtr	pPix;

    TRACE(("apa16PaintWindow32(0x%x,0x%x,0x%x)\n",pWin,pRegion,what));
    if (what==PW_BACKGROUND)
	pPix= (((mfbPrivWin *)(pWin->devPrivate))->pRotatedBackground);
    else
	pPix= (((mfbPrivWin *)(pWin->devPrivate))->pRotatedBorder);
    nbox= pRegion->numRects;
    pBox= pRegion->rects;
    while (nbox--) {
	apa16TileArea32(pWin, 1, pBox, GXcopy, pPix);
	pBox++;
    }
    return;
}

/***====================================================================***/

/* 
 *swap in correct PaintWindow* routine.  
 */

Bool
apa16ChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    int mask;
{
mfbPrivWin	*pPrivWin;
extern int	ibmAllowBackingStore;

    TRACE(("apa16ChangeWindowAttributes( pWin= 0x%x, mask= 0x%x )\n",
								pWin,mask));

    pPrivWin = (mfbPrivWin *)(pWin->devPrivate);

    if (mask&CWBackPixel) {
	pWin->PaintWindowBackground = apa16PaintWindowSolid;
	pPrivWin->fastBackground = 0;
	mask&= (~CWBackPixel);
    }
    if (mask&CWBorderPixel) {
	pWin->PaintWindowBorder = apa16PaintWindowSolid;
	pPrivWin->fastBorder = 0;
	mask&= (~CWBorderPixel);
    }

    if (!ibmAllowBackingStore)
	mask&=(~CWBackingStore);

    if ((mask)&&(mfbChangeWindowAttributes(pWin,mask))) {
	if (pWin->PaintWindowBorder==(void (*)())mfbPaintWindow32)
	    pWin->PaintWindowBorder= apa16PaintWindow32;
	if (pWin->PaintWindowBackground==(void (*)())mfbPaintWindow32)
	    pWin->PaintWindowBackground= apa16PaintWindow32;
	return(TRUE);
    }
    return(FALSE);
}

