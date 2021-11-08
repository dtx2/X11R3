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
		Copyright IBM Corporation 1988

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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaScrInit.c,v 9.0 88/10/18 12:52:40 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaScrInit.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaScrInit.c,v 9.0 88/10/18 12:52:40 erik Exp $";
static char sccsid[] = "@(#)egascrinit.c	3.1 88/09/22 09:33:20";
#endif

#include "X.h"
#include "Xproto.h"
#include "resource.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "screen.h"
#include "windowstr.h"
#include "window.h"
#include "font.h"
#include "pixmapstr.h"
#include "colormap.h"
#include "mistruct.h"

#include "mfb.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "egaProcs.h"
#include "egaVideo.h"

#include "ibmTrace.h"

extern 		mfbFreeVisual() ;
extern void 	Xfree() ;
extern void 	NoopDDA() ;
extern int	ibmAllowBackingStore;

Colormap    egaDefaultColormap ;

extern int egaNumberOfPlanes ;
extern int egaDisplayTubeType ;

static DepthRec depths[] = {
/* depth	numVid	vids */
    1,		0,	NULL,
    4,		1,	NULL,
} ;

static void
egaFreeVisual( p, id )
register pointer const p ;
register int id ;
{
    /*$$$XXX are visuals the same (no allocs inside a priv or whatever?*/
    Xfree( p ) ;
    return ;
}

/* Declared in "egacurs.c" */
extern int egaCursorChecking ;
extern int egaCheckCursor() ;
extern void egaReplaceCursor() ;

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
egaInitScreen( index, pScreen, xsize, ysize, dpix, dpiy )
    register int index ;
    register ScreenPtr const pScreen ;
    int xsize, ysize ;		/* in pixels */
    int dpix, dpiy ;		/* dots per inch */
{
    register PixmapPtr pPixmap ;
    register ppcScrnPrivPtr devPriv ;
    VisualPtr pVisual ;
    unsigned long *pVids ;

    TRACE( ( "egaInitScreen(index= %d, pScreen= 0x%x, xsize= %d, \
ysize= %d, dpix= %d, dpiy = %d)\n", index, pScreen, xsize, ysize, dpix, dpiy ) ) ;

    pScreen->myNum = index ;
    pScreen->width = xsize ;
    pScreen->height = ysize ;
    pScreen->mmWidth = ( xsize * 254 ) / ( dpix * 10 ) ;
    pScreen->mmHeight = ( ysize * 254 ) / ( dpiy * 10 ) ;
    pScreen->numDepths = 2 ;	/* 1 or 4 == 2 depths ? 3 w/ depth == 8 ? */
    pScreen->allowedDepths = depths ;

    pScreen->rootDepth = egaNumberOfPlanes ;
    pScreen->rootVisual = FakeClientID( 0 ) ;

    pScreen->defColormap = (Colormap) FakeClientID( 0 ) ;
    pScreen->minInstalledCmaps = 1 ;
    pScreen->maxInstalledCmaps = 1 ;
    if (ibmAllowBackingStore)	pScreen->backingStoreSupport = Always ;
    else			pScreen->backingStoreSupport = NotUseful ;
    pScreen->saveUnderSupport = NotUseful ;

    /* cursmin and cursmax are device specific */

    pScreen->numVisuals = 1 ;
    pScreen->visuals = pVisual = (VisualPtr) Xalloc( sizeof( VisualRec ) ) ;

    pScreen->devPrivate =
	(pointer) ( devPriv = 
		(ppcScrnPrivPtr) Xalloc( sizeof( ppcScrnPriv ) ) ) ;

/* PPC Screen Primatives */
    devPriv->blit = egaBitBlt ;
    devPriv->solidFill = egaFillSolid ;
#ifdef EGA_SPECIFIC_ROUTINES
    devPriv->tileFill = egaTileRect ;
#else
    devPriv->tileFill = ppcTileRect ;
#endif
    devPriv->stipFill = egaFillStipple ;
    devPriv->opstipFill = ppcOpaqueStipple ;
    devPriv->imageFill = egaDrawColorImage ;
    devPriv->imageRead = egaReadColorImage ;
    devPriv->lineHorz = egaHorzLine ;
    devPriv->lineVert = egaVertLine ;
    devPriv->lineBres = egaBresLine ;
    devPriv->setColor = egaSetColor ;
    devPriv->RecolorCursor = NoopDDA;
    devPriv->monoFill = NoopDDA;
    devPriv->glyphFill = egaDrawGlyph ;
    devPriv->CursorSemaphore = &egaCursorChecking ;
    devPriv->CheckCursor = egaCheckCursor ;
    devPriv->ReplaceCursor = egaReplaceCursor ;
/* END -*- PPC Screen Primatives */

    pPixmap = (PixmapPtr) pScreen->devPrivate ;

    pPixmap->drawable.pScreen = pScreen ;
    pPixmap->drawable.serialNumber = 0 ;
    pPixmap->drawable.type = DRAWABLE_PIXMAP ;
    pPixmap->height = ysize ;
    pPixmap->width = xsize ;
    pPixmap->drawable.depth = egaNumberOfPlanes ;
    pPixmap->devKind = PixmapBytePad( xsize, egaNumberOfPlanes ) ;
    pPixmap->refcnt = 1 ;
    pPixmap->devPrivate = NULL ;

/* WINDOW STUFF */
    pScreen->CreateWindow = ppcCreateWindowForXYhardware ;
    pScreen->DestroyWindow = ppcDestroyWindow ;
    pScreen->PositionWindow = ppcPositionWindow ;
    pScreen->ChangeWindowAttributes = ppcChangeWindowAttributes ;
    pScreen->RealizeWindow = mfbMapWindow ;
    pScreen->UnrealizeWindow = mfbUnmapWindow ;

/* FONT STUFF */
    pScreen->RealizeFont = mfbRealizeFont ;
    pScreen->UnrealizeFont = mfbUnrealizeFont ;

/* PIX-BLIT STUFF */
    pScreen->GetImage = miGetImage ;
    pScreen->GetSpans = (unsigned int *(*)()) ppcGetSpans ;

/* REGION STUFF */
    pScreen->RegionCreate = miRegionCreate ;
    pScreen->RegionCopy = miRegionCopy ;
    pScreen->RegionDestroy = miRegionDestroy ;
    pScreen->Intersect = miIntersect ;
    pScreen->Inverse = miInverse ;
    pScreen->Union = miUnion ;
    pScreen->Subtract = miSubtract ;
    pScreen->RegionReset = miRegionReset ;
    pScreen->TranslateRegion = miTranslateRegion ;
    pScreen->RectIn = miRectIn ;
    pScreen->PointInRegion = miPointInRegion ;
    pScreen->WindowExposures = miWindowExposures ;
    pScreen->RegionNotEmpty = miRegionNotEmpty ;
    pScreen->RegionEmpty = miRegionEmpty ;
    pScreen->RegionExtents = miRegionExtents ;
    pScreen->SendGraphicsExpose = miSendGraphicsExpose;

/* EXCEPTION STUFF */
    pScreen->BlockHandler = NoopDDA ;
    pScreen->WakeupHandler = NoopDDA ;
    pScreen->blockData = (pointer) 0 ;
    pScreen->wakeupData = (pointer) 0 ;

/* MISC. STUFF */
    pScreen->CreateGC = egaCreateGC ;
    pScreen->CreatePixmap = ppcCreatePixmap ;
    pScreen->DestroyPixmap = mfbDestroyPixmap ;
    pScreen->ValidateTree = miValidateTree ;

/* COLORMAP STUFF */
    pScreen->CreateColormap = NoopDDA ;
    pScreen->DestroyColormap = NoopDDA ;
    pScreen->InstallColormap = ppcInstallColormap ;
    pScreen->UninstallColormap = ppcUninstallColormap ;
    pScreen->ListInstalledColormaps = ppcListInstalledColormaps ;
    pScreen->StoreColors = ppcStoreColors ;
    pScreen->ResolveColor = ppcResolveColor ;

    pVisual->class = PseudoColor ;

    pVisual->redMask = 0 ;
    pVisual->greenMask = 0 ;
    pVisual->blueMask = 0 ;
    pVisual->bitsPerRGBValue = 2 ;
    pVisual->ColormapEntries = 1 << ( pVisual->nplanes = egaNumberOfPlanes ) ;
    pVisual->vid = pScreen->rootVisual ;
    pVisual->screen = index ;

    depths[1].vids =
    pVids = (unsigned long *) Xalloc( sizeof (unsigned long) ) ;
    pVids[0] = pScreen->rootVisual ;	/* our one and only visual */
    AddResource( pScreen->rootVisual, RT_VISUALID,
		 pVisual, egaFreeVisual, RC_CORE ) ;

    /* we MIGHT return 0 if we had been keeping track of potential
       allocation failures.  one day someone will clean this up.
    */
    return 1 ;
}
