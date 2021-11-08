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
/* $XConsortium: miwindow.c,v 1.17 88/09/06 14:49:38 jim Exp $ */
#include "X.h"
#include "miscstruct.h"
#include "region.h"
#include "mi.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"

/* 
 * miwindow.c : machine independent window routines 
 *  miClearToBackground
 *  miPaintWindow
 *
 *  author: drewry
 *          Dec 1986
 */


void 
miClearToBackground(pWin, x, y, w, h, generateExposures)
    WindowPtr pWin;
    short x,y;
    unsigned short w,h;
    Bool generateExposures;
{
    BoxRec box;
    RegionPtr pReg;

    box.x1 = pWin->absCorner.x + x;
    box.y1 = pWin->absCorner.y + y;
    if (w)
        box.x2 = box.x1 + w;
    else
        box.x2 = box.x1 + pWin->clientWinSize.width - x;
    if (h)
        box.y2 = box.y1 + h;	
    else
        box.y2 = box.y1 + pWin->clientWinSize.height - y;

    pReg = (* pWin->drawable.pScreen->RegionCreate)(&box, 1);
    if ((pWin->backingStore != NotUseful) &&
	(pWin->backStorage != (BackingStorePtr)NULL))
    {
	/*
	 * If the window has backing-store on, call through the
	 * ClearToBackground vector to handle the special semantics
	 * (i.e. things backing store is to be cleared out and
	 * an Expose event is to be generated for those areas in backing
	 * store if generateExposures is TRUE).
	 */
	(* pWin->backStorage->ClearToBackground)(pWin, x, y, w, h,
						 generateExposures);
    }

    if (generateExposures)
    {
        (* pWin->drawable.pScreen->Intersect)(pWin->exposed, pReg, pWin->clipList);
        HandleExposures(pWin);
    }
    else if (pWin->backgroundTile != (PixmapPtr)None)
    {
        (* pWin->drawable.pScreen->Intersect)(pReg, pReg, pWin->clipList);
        (*pWin->PaintWindowBackground)(pWin, pReg, PW_BACKGROUND);
    }
    (* pWin->drawable.pScreen->RegionDestroy)(pReg);
}


