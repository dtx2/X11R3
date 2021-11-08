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

#include "X.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "cfb.h"
#include "mistruct.h"
#include "regionstr.h"

extern WindowRec WindowTable[];

Bool cfbCreateWindow(pWin)
WindowPtr pWin;
{
    cfbPrivWin *pPrivWin;

    pWin->ClearToBackground = miClearToBackground;
    pWin->PaintWindowBackground = cfbPaintAreaNone;
    pWin->PaintWindowBorder = cfbPaintAreaPR;
    pWin->CopyWindow = cfbCopyWindow;
    pPrivWin = (cfbPrivWin *)Xalloc(sizeof(cfbPrivWin));
    pWin->devPrivate = (pointer)pPrivWin;
    pPrivWin->pRotatedBorder = NullPixmap;
    pPrivWin->pRotatedBackground = NullPixmap;
    pPrivWin->fastBackground = 0;
    pPrivWin->fastBorder = 0;

    return TRUE;
}

Bool cfbDestroyWindow(pWin)
WindowPtr pWin;
{
    cfbPrivWin *pPrivWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivate);

    /* cfbDestroyPixmap() deals with any NULL pointers */
    cfbDestroyPixmap(pPrivWin->pRotatedBorder);
    cfbDestroyPixmap(pPrivWin->pRotatedBackground);
    Xfree(pWin->devPrivate);
    if (pWin->backingStore != NotUseful)
    {
	miFreeBackingStore(pWin);
    }
    return(TRUE);
}

Bool cfbMapWindow(pWindow)
WindowPtr pWindow;
{
    return(TRUE);
}

/* (x, y) is the upper left corner of the window on the screen 
   do we really need to pass this?  (is it a;ready in pWin->absCorner?)
   we only do the rotation for pixmaps that are 32 bits wide (padded
or otherwise.)
   cfbChangeWindowAttributes() has already put a copy of the pixmap
in pPrivWin->pRotated*
*/
Bool cfbPositionWindow(pWin, x, y)
WindowPtr pWin;
int x, y;
{
    cfbPrivWin *pPrivWin;
    int setxy = 0;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivate);
    if (IS_VALID_PIXMAP(pWin->backgroundTile) &&
	(pPrivWin->fastBackground != 0))
    {
	cfbXRotatePixmap(pPrivWin->pRotatedBackground,
		      pWin->absCorner.x - pPrivWin->oldRotate.x);
	cfbYRotatePixmap(pPrivWin->pRotatedBackground,
		      pWin->absCorner.y - pPrivWin->oldRotate.y);
	setxy = 1;
    }

    if (IS_VALID_PIXMAP(pWin->borderTile) &&
	(pPrivWin->fastBorder != 0))
    {
	cfbXRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->absCorner.x - pPrivWin->oldRotate.x);
	cfbYRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->absCorner.y - pPrivWin->oldRotate.y);
	setxy = 1;
    }
    if (setxy)
    {
	pPrivWin->oldRotate.x = pWin->absCorner.x;
	pPrivWin->oldRotate.y = pWin->absCorner.y;
    }
    return (TRUE);
}

Bool cfbUnmapWindow(pWindow)
WindowPtr pWindow;
{
    return (TRUE);
}

/* UNCLEAN!
   this code calls the bitblt helper code directly.

   cfbCopyWindow copies only the parts of the destination that are
visible in the source.
*/


void 
cfbCopyWindow(pWin, ptOldOrg, prgnSrc)
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

    cfbDoBitblt(pwinRoot, pwinRoot, GXcopy, prgnDst, pptSrc);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
}



/* swap in correct PaintWindow* routine.  If we can use a fast output
routine (i.e. the pixmap is paddable to 32 bits), also pre-rotate a copy
of it in devPrivate.
*/
Bool
cfbChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    int mask;
{
    register int index;
    register cfbPrivWin *pPrivWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivate);
    while(mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
	  case CWBackingStore:
	      if (pWin->backingStore != NotUseful)
	      {
		  miInitBackingStore(pWin, cfbSaveAreas, cfbRestoreAreas, (void (*)()) 0);
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

	  case CWBackPixmap:
	      if (pWin->backgroundTile == (PixmapPtr)None)
	      {
		  pWin->PaintWindowBackground = cfbPaintAreaNone;
		  pPrivWin->fastBackground = 0;
	      }
	      else if (pWin->backgroundTile == (PixmapPtr)ParentRelative)
	      {
		  pWin->PaintWindowBackground = cfbPaintAreaPR;
		  pPrivWin->fastBackground = 0;
	      }
	      else
	      {
		  if(cfbPadPixmap(pWin->backgroundTile))
		  {
		      pPrivWin->fastBackground = 1;
		      pPrivWin->oldRotate.x = pWin->absCorner.x;
		      pPrivWin->oldRotate.y = pWin->absCorner.y;
		      if (pPrivWin->pRotatedBackground)
			  cfbDestroyPixmap(pPrivWin->pRotatedBackground);
		      pPrivWin->pRotatedBackground =
			cfbCopyPixmap(pWin->backgroundTile);
		      cfbXRotatePixmap(pPrivWin->pRotatedBackground,
				    pWin->absCorner.x);
		      cfbYRotatePixmap(pPrivWin->pRotatedBackground,
				    pWin->absCorner.y);
		      pWin->PaintWindowBackground = cfbPaintArea32;
		  }
		  else
		  {
		      pPrivWin->fastBackground = 0;
		      pWin->PaintWindowBackground = miPaintWindow;
		  }
		  break;
	      }
	      break;

	  case CWBackPixel:
              pWin->PaintWindowBackground = cfbPaintAreaSolid;
	      pPrivWin->fastBackground = 0;
	      break;

	  case CWBorderPixmap:
	      switch((int)pWin->borderTile)
	      {
		case None:
		  pWin->PaintWindowBorder = cfbPaintAreaNone;
		  pPrivWin->fastBorder = 0;
		  break;
		case ParentRelative:
		  pWin->PaintWindowBorder = cfbPaintAreaPR;
		  pPrivWin->fastBorder = 0;
		  break;
		default:
		  if(cfbPadPixmap(pWin->borderTile))
		  {
		      pPrivWin->fastBorder = 1;
		      pPrivWin->oldRotate.x = pWin->absCorner.x;
		      pPrivWin->oldRotate.y = pWin->absCorner.y;
		      if (pPrivWin->pRotatedBorder)
			  cfbDestroyPixmap(pPrivWin->pRotatedBorder);
		      pPrivWin->pRotatedBorder =
			cfbCopyPixmap(pWin->borderTile);
		      cfbXRotatePixmap(pPrivWin->pRotatedBorder,
				    pWin->absCorner.x);
		      cfbYRotatePixmap(pPrivWin->pRotatedBorder,
				    pWin->absCorner.y);
		      pWin->PaintWindowBorder = cfbPaintArea32;
		  }
		  else
		  {
		      pPrivWin->fastBorder = 0;
		      pWin->PaintWindowBorder = cfbPaintAreaOther;
		  }
		  break;
	      }
	      break;
	    case CWBorderPixel:
	      pWin->PaintWindowBorder = cfbPaintAreaSolid;
	      pPrivWin->fastBorder = 0;
	      break;

	}
    }
    return (TRUE);
}

