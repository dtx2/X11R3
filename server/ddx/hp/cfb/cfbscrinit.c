/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
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

#include "X.h"
#include "Xmd.h"
#include <servermd.h>
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"

extern void miGetImageWithBS();	/* XXX should not be needed */
extern ColormapPtr CreateStaticColormap();	/* XXX is this needed? */
extern void hpChangeScreens();

/** Each screen should have its own copy of the visuals tailored to
 ** its characteristics, and shouldn't share a common static set;
 **/

#ifdef UNDEFINED
static VisualRec visuals[] = {
/* vid screen class rMask gMask bMask oRed oGreen oBlue bpRGB cmpE nplan */
#ifdef	notdef
    /*  Eventually,  we would like to offer this visual too */
    0,	0, StaticGray, 0,   0,    0,   0,    0,	  0,    1,   2,    1,
#endif
/* cmpE and nplan will be modified for limited number of planes displays */
#ifdef	STATIC_COLOR
    0,  0, StaticColor,0,   0,    0,   0,    0,   0,    8,  256,   8,
#else
    0,  0, PseudoColor,0,   0,    0,   0,    0,   0,    8,  256,   8,
#endif
};

#define	NUMVISUALS	((sizeof visuals)/(sizeof visuals[0]))
#define	ROOTVISUAL	(NUMVISUALS-1)

static DepthRec depths[] = {
/* depth	numVid		vids */
    1,		0,		NULL,
    8,		1,		NULL,
};

static ColormapPtr cfbColorMaps[NUMVISUALS];	/* assume one per visual */

#endif UNDEFINED

static VisualRec PseudoColorVisual = {
    0,  0, PseudoColor,0,   0,    0,   0,    0,   0,    8,  256,   8,
};

static VisualRec StaticColorVisual = {
    0,  0, StaticColor,0,   0,    0,   0,    0,   0,    8,  256,   8,
};

static VisualRec StaticGrayVisual = {
    0,  0, StaticGray ,0,   0,    0,   0,    0,   0,    1,    2,   1,
};

#define NUMVISUALS  2
#define ROOTVISUAL  0
#define MAX_VISUALS 2

static DepthRec depth1 = {
/* depth	numVid		vids */
    1,		0,		NULL
};

static DepthRec depth8 = {
/* depth	numVid		vids */
    8,		1,		NULL
};

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
cfbScreenInit(index, pScreen, pbits, xsize, ysize, dpi, numPlanes)
    int index;
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpi;			/* dots per inch */
    int numPlanes;              /* number of planes in frame buffer */
{
    VisualID	*pVids;
    register PixmapPtr pPixmap;
    register cfbPrivPixmapPtr pPrivPixmap;
    register cfbPrivScreenPtr pPrivScreen;
    int	i;
    void cfbInitializeColormap();
    int numVisuals;
    int numDepths;
    VisualRec *visuals;
    DepthRec *depths;
    ColormapPtr cfbColorMaps[MAX_VISUALS];

    if (numPlanes == 1) {
	numVisuals = 1;
        visuals = (VisualRec *) xalloc(numVisuals*sizeof(VisualRec));
	bcopy(&StaticGrayVisual, &(visuals[0]), sizeof(VisualRec));

	numDepths = 2;
        depths =  (DepthRec *) xalloc(numDepths*sizeof(DepthRec));
	bcopy(&depth8, &(depths[0]), sizeof(DepthRec));
	bcopy(&depth1, &(depths[1]), sizeof(DepthRec));
    }
    else {
	numVisuals = 1;
        visuals = (VisualRec *) xalloc(numVisuals*sizeof(VisualRec));
	bcopy(&PseudoColorVisual, &(visuals[0]), sizeof(VisualRec));

	numDepths = 2;
        depths =  (DepthRec *) xalloc(numDepths*sizeof(DepthRec));
	bcopy(&depth8, &(depths[0]), sizeof(DepthRec));
	bcopy(&depth1, &(depths[1]), sizeof(DepthRec));
#ifdef not_now
	/** adjust the depth of the first supported depth **/
	depths[0].depth = numPlanes;
#endif not_now
    }

    pScreen->myNum = index;
    pScreen->width = xsize;
    pScreen->height = ysize;
    pScreen->mmWidth = (xsize * 254) / (dpi * 10);
    pScreen->mmHeight = (ysize * 254) / (dpi * 10);
    pScreen->numDepths = numDepths;
    pScreen->allowedDepths = depths;

    pScreen->rootDepth = depths[0].depth;
    pScreen->minInstalledCmaps = 1;
    pScreen->maxInstalledCmaps = 1;
    pScreen->backingStoreSupport = Always;
    pScreen->saveUnderSupport = NotUseful;

    /* cursmin and cursmax are device specific */

    if (numPlanes != 8) {
      /* need to adjust visual for limited number of planes. We assume
	 that the caller has given us a legitimate value between 2 and 8. */
      int i;
      for (i=0; i<numVisuals; i++)
	if (visuals[i].nplanes == 8) {
	  visuals[i].nplanes = numPlanes;
	  visuals[i].ColormapEntries = 1 << numPlanes;
	}
    }
    pScreen->numVisuals = numVisuals;
    pScreen->visuals = visuals;

    /* The following information is used by cfbCreatePixmap */
    pPrivScreen = (cfbPrivScreenPtr)(pScreen->devPrivate);
    if (pPrivScreen == (cfbPrivScreenPtr)NULL) /* allocate space only if needed */
      pPrivScreen = (cfbPrivScreenPtr) xalloc(sizeof(cfbPrivScreen));
    pPrivScreen->bits = pbits;
    pPrivScreen->stride = pPrivScreen->memWidth;
	pPrivScreen->ChangeScreen = hpChangeScreens;
	pPrivScreen->isBlank = FALSE;
    pScreen->devPrivate = (pointer) pPrivScreen;

    hpBufAllocInit(pScreen, pPrivScreen->stride, pPrivScreen->memHeight,
		   xsize, ysize); /* set up for off-screen memory use */
    /****** allocate the screen pixmap for the full visible area. ******/
    /******   THIS MUST BE THE FIRST PIXMAP CREATED!              ******/
    pPixmap = cfbCreatePixmap(pScreen, xsize, ysize,
			      pScreen->rootDepth);
    pPrivScreen->pDrawable = (DrawablePtr) pPixmap;
    ((cfbPrivScreenPtr)(pScreen->devPrivate))->pTmpPixmap = 
	(pointer) cfbCreatePixmap(pScreen, PRIV_PIX_WIDTH, PRIV_PIX_HEIGHT,
				  pScreen->rootDepth);

    /* anything that cfb doesn't know about is assumed to be done
       elsewhere.  (we put in no-op only for things that we KNOW
       are really no-op.
    */
    pScreen->QueryBestSize = cfbQueryBestSize;
    pScreen->CreateWindow = cfbCreateWindow;
    pScreen->DestroyWindow = cfbDestroyWindow;
    pScreen->PositionWindow = cfbPositionWindow;
    pScreen->ChangeWindowAttributes = cfbChangeWindowAttributes;
    pScreen->RealizeWindow = cfbMapWindow;
    pScreen->UnrealizeWindow = cfbUnmapWindow;

    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->GetImage = miGetImageWithBS;
    pScreen->GetSpans = cfbGetSpans;	/* XXX */
    pScreen->CreateGC = cfbCreateGC;
    pScreen->CreatePixmap = cfbCreatePixmap;
    pScreen->DestroyPixmap = cfbDestroyPixmap;
    pScreen->ValidateTree = miValidateTree;

#ifdef	STATIC_COLOR
    pScreen->InstallColormap = cfbInstallColormap;
    pScreen->UninstallColormap = cfbUninstallColormap;
    pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
    pScreen->StoreColors = NoopDDA;
    pScreen->ResolveColor = cfbResolveStaticColor;
#endif

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
    pScreen->WindowExposures = miWindowExposures;
    pScreen->RegionNotEmpty = miRegionNotEmpty;
    pScreen->RegionEmpty = miRegionEmpty;
    pScreen->RegionExtents = miRegionExtents;
    pScreen->SendGraphicsExpose = miSendGraphicsExpose;

    pScreen->BlockHandler = NoopDDA;
    pScreen->WakeupHandler = NoopDDA;
    pScreen->blockData = (pointer)0;
    pScreen->wakeupData = (pointer)0;

    pScreen->CreateColormap = cfbInitializeColormap;
    pScreen->DestroyColormap = NoopDDA;

    /*  Set up the remaining fields in the visuals[] array & make a RT_VISUALID */
    for (i = 0; i < numVisuals; i++) {
	visuals[i].vid = FakeClientID(0);
	visuals[i].screen = index;
	AddResource(visuals[i].vid, RT_VISUALID, &visuals[i], NoopDDA, RC_CORE);
	switch (visuals[i].class) {
	case StaticGray:
	case StaticColor:
	    CreateColormap(FakeClientID(0), pScreen, &visuals[i], 
		&cfbColorMaps[i], AllocAll, 0);
	    break;
	case PseudoColor:
	case GrayScale:
	    CreateColormap(FakeClientID(0), pScreen, &visuals[i], 
		&cfbColorMaps[i], AllocNone, 0);
	    break;
	case TrueColor:
	case DirectColor:
	    FatalError("Bad visual in cfbScreenInit\n");
	}
	if (!cfbColorMaps[i])
	    FatalError("Can't create colormap in cfbScreenInit\n");
    }
    pScreen->defColormap = cfbColorMaps[ROOTVISUAL]->mid;
    pScreen->rootVisual = visuals[ROOTVISUAL].vid;

    /*  Set up the remaining fields in the depths[] array */
    for (i = 0; i < numDepths; i++) {
	if (depths[i].numVids > 0) {
	    depths[i].vids = pVids = (VisualID *) xalloc(sizeof (VisualID) *
						  depths[i].numVids);
	    /* XXX - here we offer only the 8-bit visual */
	    pVids[0] = visuals[ROOTVISUAL].vid;
	}
    }

    return( TRUE );
}

cfbScreenClose(pScreen)
ScreenPtr pScreen;
{
   xfree(pScreen->visuals);
   xfree(pScreen->allowedDepths);
}
