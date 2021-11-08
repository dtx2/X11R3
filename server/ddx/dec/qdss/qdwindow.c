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
#include "qd.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "mi.h"

extern WindowRec WindowTable[];

Bool
qdDestroyWindow(pWin)
WindowPtr pWin;
{
    if (pWin->backingStore != NotUseful)
    {
	miFreeBackingStore(pWin);
    }
    return (TRUE);
}

Bool
qdChangeWindowAttributes(pWin, mask)
    register WindowPtr pWin;
    register int mask;
{
    int	index;
    extern void tlSaveAreas (), tlRestoreAreas ();

    while(mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
	  case CWBackingStore:
	      if (pWin->backingStore != NotUseful)
	      {
		  miInitBackingStore(pWin, tlSaveAreas, tlRestoreAreas, (void (*)()) 0);
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

Bool
qdMapWindow(pWindow)
WindowPtr pWindow;
{
}

Bool
qdUnmapWindow(pWindow, x, y)
WindowPtr pWindow;
int x, y;
{
}

Bool
qdPositionWindow(pWindow)
WindowPtr pWindow;
{
}

extern	int	Nplanes;

/*
 * DDX CopyWindow is required to translate prgnSrc by
 * pWin->absCorner - ptOldOrg .
 *
 * This change appears to have been made by MIT.  -dwm
 */
void
qdCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    GCPtr pcopyGC;
    GC cgc;		/* helps in changing pcopyGC */
    WindowPtr pwinRoot;
    ScreenPtr pScreen;
    int               dx, dy;
    BoxRec	box;    
    RegionPtr	clientClip;
    int		n;
    int		x, y, w, h;
    int		sx, sy;

    dx = pWin->absCorner.x - ptOldOrg.x;
    dy = pWin->absCorner.y - ptOldOrg.y;
    pScreen = pWin->drawable.pScreen;
    pwinRoot = &WindowTable[pScreen->myNum];

    (*pScreen->TranslateRegion) (prgnSrc, dx, dy);

    pcopyGC = GetScratchGC( Nplanes, pScreen);
    cgc.subWindowMode = IncludeInferiors;
    cgc.stateChanges = GCSubwindowMode;

    QDChangeGCHelper( pcopyGC, &cgc);

    clientClip = (*pScreen->RegionCreate) (NULL, 1);
    (*pScreen->Intersect) (clientClip, pWin->borderClip, prgnSrc);
    box = * (*pScreen->RegionExtents) (clientClip);
    (*pcopyGC->ChangeClip) ( pcopyGC, CT_REGION, (pointer) clientClip, 0);

    ValidateGC( pwinRoot, pcopyGC);

    x = box.x1;
    y = box.y1;
    w = ((int) box.x2) - x;
    h = ((int) box.y2) - y;
    sx = x - dx;
    sy = y - dy;

    qdCopyArea(pwinRoot, pwinRoot, pcopyGC, sx, sy, w, h, x, y);
    (*pwinRoot->drawable.pScreen->BlockHandler) ();
    (*pcopyGC->ChangeClip) (pcopyGC, CT_NONE, (pointer) NULL, 0);

    FreeScratchGC( pcopyGC);
}


Bool
qdCreateWindow(pWin)
WindowPtr pWin;
{
    extern void	tlSaveAreas (), tlRestoreAreas ();

    pWin->ClearToBackground = miClearToBackground;
    pWin->PaintWindowBackground = miPaintWindow;
    pWin->PaintWindowBorder = miPaintWindow;
    pWin->CopyWindow = qdCopyWindow;
    pWin->devBackingStore = (pointer) NULL;
    pWin->backStorage = (BackingStorePtr)NULL;

    return TRUE;
}
