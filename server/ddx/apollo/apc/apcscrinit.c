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
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "apc.h"
#include "Xmd.h"
#include <servermd.h>
#include "pixmapstr.h"
#include "resource.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "ap_text.h"

Bool
apcScreenInit(index, pScreen, pbits, xsize, ysize, dpi)
    int 		index;
    ScreenPtr 		pScreen;
    gpr_$bitmap_desc_t 	pbits;		/* screen bitmap descriptor*/
    int 		xsize, ysize;	/* in pixels */
    int 		dpi;		/* dots per inch */
{
    PixmapPtr 		pPixmap;
    int			i;
    apcPrivPMPtr 	pPrivPM;

    pScreen->myNum = index;
    pScreen->width = xsize;
    pScreen->height = ysize;
    pScreen->mmWidth = (xsize * 254) / (dpi * 10);
    pScreen->mmHeight = (ysize * 254) / (dpi * 10);

    pScreen->backingStoreSupport = NotUseful;
    pScreen->saveUnderSupport = NotUseful;

    /* anything that apc doesn't know about is assumed to be done
       elsewhere.  (we put in no-op only for things that we KNOW
       are really no-op.
    */
    pScreen->QueryBestSize = apcQueryBestSize;
    pScreen->CreateWindow = apcCreateWindow;
    pScreen->DestroyWindow = apcDestroyWindow;
    pScreen->PositionWindow = apcPositionWindow;
    pScreen->ChangeWindowAttributes = apcChangeWindowAttributes;
    pScreen->RealizeWindow = apcMapWindow;
    pScreen->UnrealizeWindow = apcUnmapWindow;
    pScreen->WindowExposures = miWindowExposures;

    pScreen->RealizeFont = gprRealizeFont;
    pScreen->UnrealizeFont = gprUnrealizeFont;
    pScreen->GetImage = miGetImageWithBS;
    pScreen->GetSpans = apcGetSpans;
    pScreen->CreateGC = apcCreateGC;
    pScreen->CreatePixmap = apcCreatePixmap;
    pScreen->DestroyPixmap = apcDestroyPixmap;
    pScreen->ValidateTree = miValidateTree;

    pScreen->RegionCreate = miRegionCreate;
    pScreen->RegionCopy = miRegionCopy;
    pScreen->RegionDestroy = miRegionDestroy;
    pScreen->Intersect = miIntersect;
    pScreen->Inverse = miInverse;
    pScreen->Union = miUnion;
    pScreen->Subtract = miSubtract;
    pScreen->RegionReset = miRegionReset;
    pScreen->TranslateRegion = miTranslateRegion;
    pScreen->RectIn = miRectIn;
    pScreen->PointInRegion = miPointInRegion;
    pScreen->RegionNotEmpty = miRegionNotEmpty;
    pScreen->RegionEmpty = miRegionEmpty;
    pScreen->RegionExtents = miRegionExtents;
    pScreen->SendGraphicsExpose = miSendGraphicsExpose;

    pScreen->blockData = (pointer)0;
    pScreen->wakeupData = (pointer)0;
    pScreen->devPrivate = NULL;

    return( TRUE );
}
