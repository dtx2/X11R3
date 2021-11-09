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

/* $XConsortium: colormap.c,v 1.75 88/09/06 15:40:10 jim Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "dix.h"
#include "colormapst.h"
#include "os.h"
#include "scrnintstr.h"
#include "resource.h"
#include "windowstr.h"

extern XID clientErrorValue;

static Pixel FindBestPixel();
static void  CopyFree(), FreeCell(), AllocShared();
static int   AllComp(), RedComp(), GreenComp(), BlueComp(), FreeClientPixels();
static int   AllocDirect(), AllocPseudo(), AllocCP(), FreeCo();

/* GetNextBitsOrBreak(bits, mask, base)  -- 
 * (Suggestion: First read the macro, then read this explanation.
 *
 * Either generate the next value to OR in to a pixel or break out of this
 * while loop 
 *
 * This macro is used when we're trying to generate all 2^n combinations of
 * bits in mask.  What we're doing here is counting in binary, except that
 * the bits we use to count may not be contiguous.  This macro will be
 * called 2^n times, returning a different value in bits each time. Then
 * it will cause us to break out of a surrounding loop. (It will always be
 * called from within a while loop.)
 * On call: mask is the value we want to find all the combinations for
 * base has 1 bit set where the least significant bit of mask is set
 *
 * For example,if mask is 01010, base should be 0010 and we count like this:
 * 00010 (see this isn't so hard), 
 *     then we add base to bits and get 0100. (bits & ~mask) is (0100 & 0100) so
 *      we add that to bits getting (0100 + 0100) =
 * 01000 for our next value.
 *      then we add 0010 to get 
 * 01010 and we're done (easy as 1, 2, 3)
 */
#define GetNextBitsOrBreak(bits, mask, base)	\
	    if((bits) == (mask)) 		\
		break;		 		\
	    (bits) += (base);		 	\
	    while((bits) & ~(mask))		\
		(bits) += ((bits) & ~(mask));	
/* ID of server as client */
#define SERVER_ID	0

typedef struct 
{
	Colormap	mid;
	int		client;
	} colorResource;

/* Invariants:
 * refcnt == 0 means entry is empty
 * refcnt > 0 means entry is useable by many clients, so it can't be changed
 * refcnt == AllocPrivate means entry owned by one client only
 * fShared should only be set if refcnt == AllocPrivate, and only in red map
 */


/* Create and initialize the color map */
int 
CreateColormap (mid, pScreen, pVisual, ppcmap, alloc, client)
    Colormap	mid;		/* resource to use for this colormap */
    ScreenPtr	pScreen;
    VisualPtr	pVisual;
    ColormapPtr	*ppcmap;	
    int		alloc;		/* 1 iff all entries are allocated writeable */
    int		client;
{
    int		class, size;
    unsigned long sizebytes;
    ColormapPtr	pmap;
    register	EntryPtr	pent;
    int		i;
    register	Pixel	*ppix, **pptr;


    class = pVisual->class;
    if(!(class & DynamicClass) && (alloc != AllocNone) && (client != SERVER_ID))
	return (BadMatch);

    pmap = (ColormapPtr) xalloc(sizeof(ColormapRec));
    AddResource(mid, RT_COLORMAP, (pointer)pmap, FreeColormap, RC_CORE);
    pmap->mid = mid;
    pmap->flags = 0; 	/* start out with all flags clear */
    if(mid == pScreen->defColormap)
	pmap->flags |= IsDefault;
    pmap->pScreen = pScreen;
    pmap->pVisual = pVisual;
    pmap->class = class;

    /* allocate first (red) map */
    size = pVisual->ColormapEntries;
    pmap->freeRed = size;
    sizebytes = size * sizeof (Entry);
    pmap->red = (EntryPtr) xalloc(sizebytes);
    bzero ((char *) pmap->red, sizebytes);

    pmap->clientPixelsRed = (Pixel **) xalloc(MAXCLIENTS * sizeof(Pixel *));
    pmap->numPixelsRed = (int *) xalloc(MAXCLIENTS * sizeof(int));
    bzero((char *) pmap->numPixelsRed, MAXCLIENTS * sizeof(int));
    for (pptr = &pmap->clientPixelsRed[MAXCLIENTS]; --pptr >= pmap->clientPixelsRed; )
	*pptr = (Pixel *)NULL;
    if (alloc == AllocAll)
    {
	pmap->flags |= AllAllocated;
	for (pent = &pmap->red[size - 1]; pent >= pmap->red; pent--)
	    pent->refcnt = AllocPrivate;
	pmap->freeRed = 0;
	ppix = (Pixel *)xalloc(size * sizeof(Pixel));
	(pmap->clientPixelsRed)[client] = ppix;
	for(i = 0; i < size; i++)
	    ppix[i] = i;
	(pmap->numPixelsRed)[client] = size;
    }

    if ((class | DynamicClass) == DirectColor)
    {
	pmap->freeGreen = size;
	pmap->freeBlue = size;
	

	pmap->green = (EntryPtr) xalloc(sizebytes);
	pmap->blue = (EntryPtr) xalloc(sizebytes);

	bzero ((char *) pmap->green, sizebytes);
	bzero ((char *) pmap->blue, sizebytes);

	pmap->clientPixelsGreen = (Pixel **)xalloc(MAXCLIENTS * sizeof(Pixel *));
	pmap->clientPixelsBlue = (Pixel **)xalloc(MAXCLIENTS * sizeof(Pixel *));
	pmap->numPixelsGreen = (int *) xalloc(MAXCLIENTS * sizeof(int));
	pmap->numPixelsBlue = (int *) xalloc(MAXCLIENTS * sizeof(int));

	bcopy((char *) pmap->clientPixelsRed,
	      (char *) pmap->clientPixelsGreen,
	      MAXCLIENTS * sizeof(Pixel *));
	bcopy((char *) pmap->clientPixelsRed,
	      (char *) pmap->clientPixelsBlue,
	      MAXCLIENTS * sizeof(Pixel *));
	bzero((char *) pmap->numPixelsGreen, MAXCLIENTS * sizeof(int));
	bzero((char *) pmap->numPixelsBlue, MAXCLIENTS * sizeof(int));

	/* If every cell is allocated, mark its refcnt */
	if (alloc == AllocAll)
	{
	    for(pent = &pmap->green[size-1]; pent >= pmap->green; pent--)
		pent->refcnt = AllocPrivate;
	    for(pent = &pmap->blue[size-1]; pent >= pmap->blue; pent--)
		pent->refcnt = AllocPrivate;
	    pmap->freeGreen = 0;
	    pmap->freeBlue = 0;

	    ppix = (Pixel *) xalloc(size * sizeof(Pixel));
	    (pmap->clientPixelsGreen)[client] = ppix;
	    for(i = 0; i < size; i++)
		ppix[i] = i;
	    (pmap->numPixelsGreen)[client] = size;

	    ppix = (Pixel *) xalloc(size * sizeof(Pixel));
	    (pmap->clientPixelsBlue)[client] = ppix;

	    for(i = 0; i < size; i++)
		ppix[i] = i;
	    (pmap->numPixelsBlue)[client] = size;
	}
    }
    /* If the device wants a chance to initialize the colormap in any way,
     * this is it.  In specific, if this is a Static colormap, this is the
     * time to fill in the colormap's values */
    pmap->flags |= BeingCreated;
    (*pScreen->CreateColormap)(pmap);
    pmap->flags &= ~BeingCreated;
    *ppcmap = pmap;
    return (Success);
}

int
FreeColormap (pmap, mid)
    ColormapPtr	pmap;
    Colormap	mid;
{
    int		i;
    register EntryPtr pent;

    if(CLIENT_ID(mid) != SERVER_ID)
    {
        (*(pmap->pScreen->UninstallColormap)) (pmap);
        WalkTree(pmap->pScreen, TellNoMap, (pointer) &mid);
    }

    /* This is the device's chance to undo anything it needs to, especially
     * to free any storage it allocated */
    (*pmap->pScreen->DestroyColormap)(pmap);

    if(pmap->clientPixelsRed)
    {
	for(i = 0; i < MAXCLIENTS; i++)
	    xfree((pmap->clientPixelsRed)[i]);
	xfree(pmap->clientPixelsRed);
	xfree(pmap->numPixelsRed);
    }

    if ((pmap->class == PseudoColor) || (pmap->class == GrayScale))
    {
	for(pent = &pmap->red[pmap->pVisual->ColormapEntries - 1];
	    pent >= pmap->red;
	    pent--)
	{
	    if(pent->fShared)
	    {
		if (--pent->co.shco.red->refcnt == 0)
		    xfree(pent->co.shco.red);
		if (--pent->co.shco.green->refcnt == 0)
		    xfree(pent->co.shco.green);
		if (--pent->co.shco.blue->refcnt == 0)
		    xfree(pent->co.shco.blue);
	    }
	}
    }
    xfree(pmap->red);
    if((pmap->class | DynamicClass) == DirectColor)
    {
        for(i = 0; i < MAXCLIENTS; i++)
	{
            xfree((pmap->clientPixelsGreen)[i]);
            xfree((pmap->clientPixelsBlue)[i]);
        }
	xfree(pmap->clientPixelsGreen);
	xfree(pmap->clientPixelsBlue);
	xfree(pmap->numPixelsGreen);
	xfree(pmap->numPixelsBlue);
	xfree(pmap->green);
	xfree(pmap->blue);
    }
    xfree(pmap);
    return(Success);
}

/* Tell window that pmid has disappeared */
static int
TellNoMap (pwin, pmid)
    WindowPtr	pwin;
    Colormap 	*pmid;
{
    xEvent 	xE;
    if (pwin->colormap == *pmid)
    {
	/* This should be call to DeliverEvent */
	xE.u.u.type = ColormapNotify;
	xE.u.colormap.window = pwin->wid;
	xE.u.colormap.colormap = *pmid;
	xE.u.colormap.new = TRUE;
	xE.u.colormap.state = ColormapUninstalled;
	DeliverEvents(pwin, &xE, 1, (WindowPtr)NULL);
        pwin->colormap = None;
    }

    return (WT_WALKCHILDREN);
}

/* Tell window that pmid got uninstalled */
int
TellLostMap (pwin, pmid)
    WindowPtr	pwin;
    Colormap 	*pmid;
{
    xEvent 	xE;
    if (pwin->colormap == *pmid)
    {
	/* This should be call to DeliverEvent */
	xE.u.u.type = ColormapNotify;
	xE.u.colormap.window = pwin->wid;
	xE.u.colormap.colormap = *pmid;
	xE.u.colormap.new = FALSE;
	xE.u.colormap.state = ColormapUninstalled;
	DeliverEvents(pwin, &xE, 1, (WindowPtr)NULL);
    }

    return (WT_WALKCHILDREN);
}

/* Tell window that pmid got installed */
int
TellGainedMap (pwin, pmid)
    WindowPtr	pwin;
    Colormap 	*pmid;
{
    xEvent 	xE;
    if (pwin->colormap == *pmid)
    {
	/* This should be call to DeliverEvent */
	xE.u.u.type = ColormapNotify;
	xE.u.colormap.window = pwin->wid;
	xE.u.colormap.colormap = *pmid;
	xE.u.colormap.new = FALSE;
	xE.u.colormap.state = ColormapInstalled;
	DeliverEvents(pwin, &xE, 1, (WindowPtr)NULL);
    }

    return (WT_WALKCHILDREN);
}

  
int
CopyColormapAndFree (mid, pSrc, client)
    Colormap	mid;
    ColormapPtr	pSrc;
    int		client;
{
    ColormapPtr	pmap = (ColormapPtr) NULL;
    int		result, alloc, size;
    Colormap	midSrc;
    ScreenPtr	pScreen;
    VisualPtr	pVisual;

    pScreen = pSrc->pScreen;
    pVisual = pSrc->pVisual;
    midSrc = pSrc->mid;
    alloc = ((pSrc->flags & AllAllocated) && CLIENT_ID(midSrc) == client) ?
            AllocAll : AllocNone;
    size = pVisual->ColormapEntries;

    /* If the create returns non-0, it failed */
    result = CreateColormap (mid, pScreen, pVisual, &pmap, alloc, client);
    if(result != Success)
        return(result);
    if(alloc == AllocAll)
    {
	bcopy((char *)pSrc->red, (char *)pmap->red, size * sizeof(Entry));
	if((pmap->class | DynamicClass) == DirectColor)
	{
	    bcopy((char *)pSrc->green, (char *)pmap->green, size * sizeof(Entry));
	    bcopy((char *)pSrc->blue, (char *)pmap->blue, size * sizeof(Entry));
	}
	/* We're going to "change" the original map back to AllocNone. The
	 * easiest way to do this is to delete the map and create a new one
	 * with the same id */
	FreeResource(midSrc, RC_NONE);
	/* XXX if this fails we're in big trouble */
	CreateColormap(midSrc, pScreen, pVisual, &pmap, AllocNone, client);
	return(Success);
    }

    if (pmap->class & DynamicClass)
	CopyFree(REDMAP, client, pSrc, pmap);

    if (pmap->class == DirectColor)
    {
        CopyFree(GREENMAP, client, pSrc, pmap);
        CopyFree(BLUEMAP, client, pSrc, pmap);
    }
    /* XXX should worry about removing any RT_CMAPENTRY resource */
    return(Success);
}

/* Helper routine for freeing large numbers of cells from a map */
static void
CopyFree (channel, client, pmapSrc, pmapDst)
    int		channel, client;
    ColormapPtr	pmapSrc, pmapDst;
{
    int		z, npix, oldFree;
    EntryPtr	pentSrcFirst, pentDstFirst;
    EntryPtr	pentSrc, pentDst;
    Pixel	*ppix;

    switch(channel)
    {
      case PSEUDOMAP:
      case REDMAP:
	ppix = (pmapSrc->clientPixelsRed)[client];
	npix = (pmapSrc->numPixelsRed)[client];
	pentSrcFirst = pmapSrc->red;
	pentDstFirst = pmapDst->red;
	oldFree = pmapSrc->freeRed;
	break;
      case GREENMAP:
	ppix = (pmapSrc->clientPixelsGreen)[client];
	npix = (pmapSrc->numPixelsGreen)[client];
	pentSrcFirst = pmapSrc->green;
	pentDstFirst = pmapDst->green;
	oldFree = pmapSrc->freeGreen;
	break;
      case BLUEMAP:
	ppix = (pmapSrc->clientPixelsBlue)[client];
	npix = (pmapSrc->numPixelsBlue)[client];
	pentSrcFirst = pmapSrc->blue;
	pentDstFirst = pmapDst->blue;
	oldFree = pmapSrc->freeBlue;
	break;
    }
    for(z = npix; --z >= 0; ppix++)
    {
        /* Copy entries */
        pentSrc = pentSrcFirst + *ppix;
        pentDst = pentDstFirst + *ppix;
	if (pentDst->refcnt > 0)
	{
	    pentDst->refcnt++;
	}
	else
	{
	    *pentDst = *pentSrc;
	    if (pentSrc->refcnt > 0)
		pentDst->refcnt = 1;
	    else
		pentSrc->fShared = FALSE;
	}
    	FreeCell(pmapSrc, *ppix, channel);
    }

    /* Note that FreeCell has already fixed pmapSrc->free{Color} */
    switch(channel)
    {
      case PSEUDOMAP:
      case REDMAP:
        pmapDst->freeRed -= (pmapSrc->freeRed - oldFree);
        (pmapDst->clientPixelsRed)[client] =
	    (pmapSrc->clientPixelsRed)[client];
        (pmapSrc->clientPixelsRed)[client] = (Pixel *) NULL;
        (pmapDst->numPixelsRed)[client] = (pmapSrc->numPixelsRed)[client];
        (pmapSrc->numPixelsRed)[client] = 0;
	break;
      case GREENMAP:
        pmapDst->freeGreen -= (pmapSrc->freeGreen - oldFree);
        (pmapDst->clientPixelsGreen)[client] =
	    (pmapSrc->clientPixelsGreen)[client];
        (pmapSrc->clientPixelsGreen)[client] = (Pixel *) NULL;
        (pmapDst->numPixelsGreen)[client] = (pmapSrc->numPixelsGreen)[client];
        (pmapSrc->numPixelsGreen)[client] = 0;
	break;
      case BLUEMAP:
        pmapDst->freeBlue -= (pmapSrc->freeBlue - oldFree);
        pmapDst->clientPixelsBlue[client] = pmapSrc->clientPixelsBlue[client];
        pmapSrc->clientPixelsBlue[client] = (Pixel *) NULL;
        pmapDst->numPixelsBlue[client] = pmapSrc->numPixelsBlue[client];
        pmapSrc->numPixelsBlue[client] = 0;
	break;
    }
}

/* Free the ith entry in a color map.  Must handle freeing of
 * colors allocated through AllocColorPlanes */
static void
FreeCell (pmap, i, channel)
    ColormapPtr pmap;
    Pixel i;
    int	channel;
{
    EntryPtr pent;
    int	*pCount;


    switch (channel)
    {
      case PSEUDOMAP:
      case REDMAP:
          pent = (EntryPtr) &pmap->red[i];
	  pCount = &pmap->freeRed;
	  break;
      case GREENMAP:
          pent = (EntryPtr) &pmap->green[i];
	  pCount = &pmap->freeGreen;
	  break;
      case BLUEMAP:
          pent = (EntryPtr) &pmap->blue[i];
	  pCount = &pmap->freeBlue;
	  break;
    }
    /* If it's not privately allocated and it's not time to free it, just
     * decrement the count */
    if (pent->refcnt > 1)
	pent->refcnt--;
    else
    {
        /* If the color type is shared, find the sharedcolor. If decremented
         * refcnt is 0, free the shared cell. */
        if (pent->fShared)
	{
	    if(--pent->co.shco.red->refcnt == 0)
		xfree(pent->co.shco.red);
	    if(--pent->co.shco.green->refcnt == 0)
		xfree(pent->co.shco.green);
	    if(--pent->co.shco.blue->refcnt == 0)
		xfree(pent->co.shco.blue);
	    pent->fShared = FALSE;
	}
	pent->refcnt = 0;
	*pCount += 1;
    }
}


/* Get a read-only color from a ColorMap (probably slow for large maps)
 * Returns by changing the value in pred, pgreen, pblue and pPix
 * On Error sets Alloc
 */
int
AllocColor (pmap, pred, pgreen, pblue, pPix, client)
    ColormapPtr		pmap;
    unsigned short 	*pred, *pgreen, *pblue;
    Pixel		*pPix;
    int			client;
{
    Pixel	pixR, pixG, pixB;
    int		entries;
    xrgb	rgb;
    int		class;
    VisualPtr	pVisual;


    pVisual = pmap->pVisual;
    (*pmap->pScreen->ResolveColor) (pred, pgreen, pblue, pVisual);
    rgb.red = *pred;
    rgb.green = *pgreen;
    rgb.blue = *pblue;
    class = pmap->class;
    entries = pVisual->ColormapEntries;

    /* If the colormap is being created, then we want to be able to change
     * the colormap, even if it's a static type. Otherwise, we'd never be
     * able to initialize static colormaps
     */
    if(pmap->flags & BeingCreated)
	class |= DynamicClass;

    /* If this is one of the static storage classes, and we're not initializing
     * it, the best we can do is to find the closest color entry to the
     * requested one and return that.
     */
    switch (class) {
    /* If this is StaticColor or StaticGray, look up all three components
     * in the same pmap */
    case StaticColor:
    case StaticGray:
	*pPix = pixR = FindBestPixel(pmap->red, entries, &rgb, PSEUDOMAP);
	*pred = pmap->red[pixR].co.local.red;
	*pgreen = pmap->red[pixR].co.local.green;
	*pblue = pmap->red[pixR].co.local.blue;
	return(Success);

    case TrueColor:
	/* Look up each component in its own map, then OR them together */
	pixR = FindBestPixel(pmap->red, entries, &rgb, REDMAP);
	pixG = FindBestPixel(pmap->green, entries, &rgb, GREENMAP);
	pixB = FindBestPixel(pmap->blue, entries, &rgb, BLUEMAP);
	*pPix = (pixR << pVisual->offsetRed) |
		(pixG << pVisual->offsetGreen) |
		(pixB << pVisual->offsetBlue);
	*pred = pmap->red[pixR].co.local.red;
	*pgreen = pmap->green[pixG].co.local.green;
	*pblue = pmap->blue[pixB].co.local.blue;
	return(Success);

    case GrayScale:
    case PseudoColor:
	if (FindColor(pmap, pmap->red, entries, &rgb, pPix, PSEUDOMAP,
		      client, AllComp) != Success)
	{
	    return (BadAlloc);
	}
        break;

    case DirectColor:
	pixR = (*pPix & pVisual->redMask) >> pVisual->offsetRed; 
	if (FindColor(pmap, pmap->red, entries, &rgb, &pixR, REDMAP,
		      client, RedComp) != Success)
	{
	    return (BadAlloc);
	}
	pixG = (*pPix & pVisual->greenMask) >> pVisual->offsetGreen; 
	if (FindColor(pmap, pmap->green, entries, &rgb, &pixG, GREENMAP,
		      client, GreenComp) != Success)
	{
	    (void)FreeCo(pmap, client, REDMAP, 1, &pixR, (Pixel)0);
	    return (BadAlloc);
	}
	pixB = (*pPix & pVisual->blueMask) >> pVisual->offsetBlue; 
	if (FindColor(pmap, pmap->blue, entries, &rgb, &pixB, BLUEMAP,
		      client, BlueComp) != Success)
	{
	    (void)FreeCo(pmap, client, GREENMAP, 1, &pixG, (Pixel)0);
	    (void)FreeCo(pmap, client, REDMAP, 1, &pixR, (Pixel)0);
	    return (BadAlloc);
	}
	*pPix = pixR | pixG | pixB;
	break;
    }

    /* if this is the client's first pixel in this colormap, tell the
     * resource manager that the client has pixels in this colormap which
     * should be freed when the client dies */
    if (((pmap->numPixelsRed)[client] == 1) &&
	(CLIENT_ID(pmap->mid) != client) &&
	!(pmap->flags & BeingCreated))
    {
	colorResource	*pcr;

	pcr = (colorResource *) xalloc(sizeof(colorResource));
	pcr->mid = pmap->mid;
	pcr->client = client;
	AddResource(FakeClientID(client), RT_CMAPENTRY, (pointer)pcr,
		    FreeClientPixels, RC_CORE);
    }
    return (Success);
}


static Pixel
FindBestPixel(pentFirst, size, prgb, channel)
    EntryPtr	pentFirst;
    int		size;
    xrgb	*prgb;
    int		channel;
{
    EntryPtr	pent;
    Pixel	pixel, final;
    long	dr, dg, db;
    unsigned long minval, diff, sum;

    final = 0;
    minval = ~((Pixel)0);
    /* look for the minimal difference */
    for (pent = pentFirst, pixel = 0; pixel < size; pent++, pixel++)
    {
	dr = dg = db = 0;
	switch(channel)
	{
	  case PSEUDOMAP:
	      dg = pent->co.local.green - prgb->green;
	      db = pent->co.local.blue - prgb->blue;
	  case REDMAP:
	      dr = pent->co.local.red - prgb->red;
	      break;
	  case GREENMAP:
	      dg = pent->co.local.green - prgb->green;
	      break;
	  case BLUEMAP:
	      db = pent->co.local.blue - prgb->blue;
	      break;
	}
	diff = dr * dr;
	sum = diff + dg * dg;
	if (sum < diff)
	    continue;
	diff = sum + db * db;
	if ((diff >= sum) && (diff < minval))
	{
	    final = pixel;
	    minval = diff;
	}
    }
    return(final);
}

/* Tries to find a color in pmap that exactly matches the one requested in prgb 
 * if it can't it allocates one.
 * Starts looking at pentFirst + *pPixel, so if you want a specific pixel,
 * load *pPixel with that value, otherwise set it to 0
 * Returns -1 on error, assuming that no one has a map THAT big */
Pixel
FindColor (pmap, pentFirst, size, prgb, pPixel, channel, client, comp)
    ColormapPtr	pmap;
    EntryPtr	pentFirst;
    int		size;
    xrgb	*prgb;
    Pixel	*pPixel;
    int		channel;
    int		client;
    int		(*comp) ();
{
    EntryPtr	pent;
    Bool	foundFree;
    Pixel	pixel, Free;
    int		npix, count, *nump;
    Pixel	**pixp, *ppix;
    xColorItem	def;

    foundFree = FALSE;

    if((pixel = *pPixel) >= size)
	pixel = 0;
    /* see if there is a match, and also look for a free entry */
    for (pent = pentFirst + pixel, count = size; --count >= 0; )
    {
        if (pent->refcnt > 0)
	{
    	    if ((*comp) (pent, prgb))
	    {
    	        pent->refcnt++;
		*pPixel = pixel;
		switch(channel)
		{
		  case REDMAP:
		    *pPixel <<= pmap->pVisual->offsetRed;
		  case PSEUDOMAP:
		    break;
		  case GREENMAP:
		    *pPixel <<= pmap->pVisual->offsetGreen;
		    break;
		  case BLUEMAP:
		    *pPixel <<= pmap->pVisual->offsetBlue;
		    break;
		}
		goto gotit;
    	    }
        }
	else if (!foundFree && pent->refcnt == 0)
	{
	    Free = pixel;
	    foundFree = TRUE;
	    /* If we're initializing the colormap, then we are looking for
	     * the first free cell we can find, not to minimize the number
	     * of entries we use.  So don't look any further. */
	    if(pmap->flags & BeingCreated)
		break;
	}
	pixel++;
	if(pixel >= size)
	{
	    pent = pentFirst;
	    pixel = 0;
	}
	else
	    pent++;
    }

    /* If we got here, we didn't find a match.  If we also didn't find
     * a free entry, we're out of luck.  Otherwise, we'll usurp a free
     * entry and fill it in */
    if (!foundFree)
    {
	return (-1);
    }
    pent = pentFirst + Free;
    pent->fShared = FALSE;
    pent->refcnt = 1;

    def.flags = 0;
    switch (channel)
    {
      case PSEUDOMAP:
        pent->co.local.green = prgb->green;
        pent->co.local.blue = prgb->blue;
	def.green = prgb->green;
	def.blue = prgb->blue;
	def.flags |= DoGreen;
	def.flags |= DoBlue;
	/* For PseudoColor we load all three values for the pixel,
	 * but only put it in 1 map, the red one */

	/* So Fall through */
      case REDMAP:
        pent->co.local.red = prgb->red;
        def.red = prgb->red;
	def.flags |= DoRed;
	pmap->freeRed--;
	def.pixel = (channel == PSEUDOMAP) ? Free
					   : Free << pmap->pVisual->offsetRed;
	break;

      case GREENMAP:
	pent->co.local.green = prgb->green;
        def.green = prgb->green;
	def.flags |= DoGreen;
	pmap->freeGreen--;
	def.pixel = Free << pmap->pVisual->offsetGreen;
	break;

      case BLUEMAP:
	pent->co.local.blue = prgb->blue;
	def.blue = prgb->blue;
	def.flags |= DoBlue;
	pmap->freeBlue--;
	def.pixel = Free << pmap->pVisual->offsetBlue;
	break;
    }
    (*pmap->pScreen->StoreColors) (pmap, 1, &def);
    pixel = Free;	
    *pPixel = def.pixel;

gotit:
    if (pmap->flags & BeingCreated)
	return(Success);
    /* Now remember the pixel, for freeing later */
    switch (channel)
    {
      case PSEUDOMAP:
      case REDMAP:
	nump = pmap->numPixelsRed;
	pixp = pmap->clientPixelsRed;
	break;

      case GREENMAP:
	nump = pmap->numPixelsGreen;
	pixp = pmap->clientPixelsGreen;
	break;

      case BLUEMAP:
	nump = pmap->numPixelsBlue;
	pixp = pmap->clientPixelsBlue;
	break;
    }
    npix = nump[client];
    ppix = (Pixel *) xrealloc (pixp[client], (npix + 1) * sizeof(Pixel));
    ppix[npix] = pixel;
    pixp[client] = ppix;
    nump[client]++;

    return(Success);
}

/* Comparison functions -- passed to FindColor to determine if an
 * entry is already the color we're looking for or not */
static int
AllComp (pent, prgb)
    EntryPtr	pent;
    xrgb	*prgb;
{
    if((pent->co.local.red == prgb->red) &&
       (pent->co.local.green == prgb->green) &&
       (pent->co.local.blue == prgb->blue) )
       return (1);
    return (0);
}

static int
RedComp (pent, prgb)
    EntryPtr	pent;
    xrgb	*prgb;
{
    if (pent->co.local.red == prgb->red) 
	return (1);
    return (0);
}

static int
GreenComp (pent, prgb)
    EntryPtr	pent;
    xrgb	*prgb;
{
    if (pent->co.local.green == prgb->green) 
	return (1);
    return (0);
}

static int
BlueComp (pent, prgb)
    EntryPtr	pent;
    xrgb	*prgb;
{
    if (pent->co.local.blue == prgb->blue) 
	return (1);
    return (0);
}


/* Read the color value of a cell */

int
QueryColors (pmap, count, ppixIn, prgbList)
    ColormapPtr	pmap;
    int		count;
    Pixel	*ppixIn;
    xrgb	*prgbList;
{
    Pixel	*ppix, pixel;
    xrgb	*prgb;
    VisualPtr	pVisual;
    EntryPtr	pent;
    Pixel	i;
    int		errVal = Success;

    pVisual = pmap->pVisual;
    if ((pmap->class | DynamicClass) == DirectColor)
    {

	for( ppix = ppixIn, prgb = prgbList; --count >= 0; ppix++, prgb++)
	{
	    pixel = *ppix;
	    i  = (pixel & pVisual->redMask) >> pVisual->offsetRed;
	    if (i >= pVisual->ColormapEntries)
	    {
		clientErrorValue = pixel;
		errVal =  BadValue;
	    }
	    else
	    {
		prgb->red = pmap->red[i].co.local.red;


		i  = (pixel & pVisual->greenMask) >> pVisual->offsetGreen;
		if (i >= pVisual->ColormapEntries)
		{
		    clientErrorValue = pixel;
		    errVal =  BadValue;
		}
		else
		{
		    prgb->green = pmap->green[i].co.local.green;

		    i  = (pixel & pVisual->blueMask) >> pVisual->offsetBlue;
		    if (i >= pVisual->ColormapEntries)
		    {
			clientErrorValue = pixel;
			errVal =  BadValue;
		    }
		    else
			prgb->blue = pmap->blue[i].co.local.blue;
		}
	    }
	}
    }
    else
    {
	for( ppix = ppixIn, prgb = prgbList; --count >= 0; ppix++, prgb++)
	{
	    pixel = *ppix;
	    if (pixel >= pVisual->ColormapEntries)
	    {
		clientErrorValue = pixel;
		errVal = BadValue;
	    }
	    else
	    {
		pent = (EntryPtr)&pmap->red[pixel];
		if (pent->fShared)
		{
		    prgb->red = pent->co.shco.red->color;
		    prgb->green = pent->co.shco.green->color;
		    prgb->blue = pent->co.shco.blue->color;
		}
		else
		{
		    prgb->red = pent->co.local.red;
		    prgb->green = pent->co.local.green;
		    prgb->blue = pent->co.local.blue;
		}
	    }
	}
    }
    return (errVal);
}

/* Free all of a client's colors and cells */
/*ARGSUSED*/
static int
FreeClientPixels (pcr, fakeid)
    colorResource *pcr;
    XID	fakeid;
{
    register Pixel		*ppix, *ppixStart;
    register int 		n;
    register ColormapPtr	pmap;
    register int 		client;
    int				class;

    /* if mid is no longer a resource, the colormap has already been freed
     * and we can all go home.
     */
    if((pmap = (ColormapPtr) LookupID(pcr->mid, RT_COLORMAP, RC_CORE)) ==
        (ColormapPtr) NULL)
    {
        xfree(pcr);
	return(Success);
    }
    client = pcr->client;
    xfree(pcr);

    class = pmap->class;
    ppix = (pmap->clientPixelsRed)[client];
    ppixStart = ppix;
    for (n = (pmap->numPixelsRed)[client]; --n >= 0; )
	FreeCell(pmap, *ppix++, REDMAP);
    xfree(ppixStart);
    (pmap->clientPixelsRed)[client] = (Pixel *) NULL;
    (pmap->numPixelsRed)[client] = 0;
 
    if ((class | DynamicClass) == DirectColor) 
    {
        ppix = (pmap->clientPixelsGreen)[client];
	ppixStart = ppix;
	for (n = (pmap->numPixelsGreen)[client]; --n >= 0; )
	    FreeCell(pmap, *ppix++, GREENMAP);
	xfree(ppixStart);
	(pmap->clientPixelsGreen)[client] = (Pixel *) NULL;
	(pmap->numPixelsGreen)[client] = 0;

        ppix = (pmap->clientPixelsBlue)[client];
	ppixStart = ppix;
	for (n = (pmap->numPixelsBlue)[client]; --n >= 0; )
	    FreeCell(pmap, *ppix++, BLUEMAP);
	xfree(ppixStart);
	(pmap->clientPixelsBlue)[client] = (Pixel *) NULL;
	(pmap->numPixelsBlue)[client] = 0;
    }
    return(Success);
}

int
AllocColorCells (client, pmap, colors, planes, contig, ppix, masks)
    int		client;
    ColormapPtr	pmap;
    int		colors, planes;
    Bool	contig;
    Pixel	*ppix;
    Pixel	*masks;
{
    Pixel	rmask, gmask, bmask, *ppixFirst, r, g, b;
    int		n, class;
    int		ok;
    int		oldcount = pmap->numPixelsRed[client];

    class = pmap->class;
    if (!(class & DynamicClass))
    {
	return (BadAlloc); /* Shouldn't try on this type */
    }
    if (pmap->class == DirectColor)
    {
	oldcount += pmap->numPixelsGreen[client] + pmap->numPixelsBlue[client];
        ok = AllocDirect (client, pmap, colors, planes, planes, planes,
			  contig, ppix, &rmask, &gmask, &bmask);
	if(ok == Success)
	{
	    for (r = g = b = 1, n = planes; --n >= 0; r += r, g += g, b += b)
	    {
		while(!(rmask & r))
		    r += r;
		while(!(gmask & g))
		    g += g;
		while(!(bmask & b))
		    b += b;
		*masks++ = (r << pmap->pVisual->offsetRed) |
			   (g << pmap->pVisual->offsetGreen) |
			   (b << pmap->pVisual->offsetBlue);
	    }
	}
    }
    else
    {
        ok = AllocPseudo (client, pmap, colors, planes, contig, ppix, &rmask,
			  &ppixFirst);
	if(ok == Success)
	{
	    for (r = 1, n = planes; --n >= 0; r += r)
	    {
		while(!(rmask & r))
		    r += r;
		*masks++ = r;
	    }
	}
    }

    /* if this is the client's first pixels in this colormap, tell the
     * resource manager that the client has pixels in this colormap which
     * should be freed when the client dies */
    if (!oldcount && colors && (ok == Success) &&
	(CLIENT_ID(pmap->mid) != client))
    {
	colorResource	*pcr;

	pcr = (colorResource *) xalloc(sizeof(colorResource));
	pcr->mid = pmap->mid;
	pcr->client = client;
	AddResource(FakeClientID(client), RT_CMAPENTRY, (pointer)pcr,
		    FreeClientPixels, RC_CORE);
    }

    return (ok);

}


int
AllocColorPlanes (client, pmap, colors, r, g, b, contig, pixels,
		  prmask, pgmask, pbmask)
    int		client;
    ColormapPtr	pmap;
    int		colors, r, g, b;
    Bool	contig;
    Pixel	*pixels;
    Pixel	*prmask, *pgmask, *pbmask;
{
    Bool	ok;
    Pixel	mask, *ppixFirst;
    register Pixel shift;
    register int i;
    int		class;
    int		oldcount = pmap->numPixelsRed[client];

    class = pmap->class;
    if (!(class & DynamicClass))
    {
	return (BadAlloc); /* Shouldn't try on this type */
    }
    if (class == DirectColor)
    {
	oldcount += pmap->numPixelsGreen[client] + pmap->numPixelsBlue[client];
        ok = AllocDirect (client, pmap, colors, r, g, b, contig, pixels,
			  prmask, pgmask, pbmask);
    }
    else
    {
	/* Allocate the proper pixels */
	/* XXX This is sort of bad, because of contig is set, we force all
	 * r + g + b bits to be contiguous.  Should only force contiguity
	 * per mask 
	 */
        ok = AllocPseudo (client, pmap, colors, r + g + b, contig, pixels,
			  &mask, &ppixFirst);

	if(ok == Success)
	{
	    /* now split that mask into three */
	    *prmask = *pgmask = *pbmask = 0;
	    shift = 1;
	    for (i = r; --i >= 0; shift += shift)
	    {
		while (!(mask & shift))
		    shift += shift;
		*prmask |= shift;
	    }
	    for (i = g; --i >= 0; shift += shift)
	    {
		while (!(mask & shift))
		    shift += shift;
		*pgmask |= shift;
	    }
	    for (i = b; --i >= 0; shift += shift)
	    {
		while (!(mask & shift))
		    shift += shift;
		*pbmask |= shift;
	    }

	    /* set up the shared color cells */
	    AllocShared(pmap, pixels, colors, r, g, b,
	                *prmask, *pgmask, *pbmask, ppixFirst);
	}
    }

    /* if this is the client's first pixels in this colormap, tell the
     * resource manager that the client has pixels in this colormap which
     * should be freed when the client dies */
    if (!oldcount && colors && (ok == Success) &&
	(CLIENT_ID(pmap->mid) != client))
    {
	colorResource	*pcr;

	pcr = (colorResource *) xalloc(sizeof(colorResource));
	pcr->mid = pmap->mid;
	pcr->client = client;
	AddResource(FakeClientID(client), RT_CMAPENTRY, (pointer)pcr,
		    FreeClientPixels, RC_CORE);
    }

    return (ok);
	

}

static int
AllocDirect (client, pmap, c, r, g, b, contig, pixels, prmask, pgmask, pbmask)
    int		client;
    ColormapPtr	pmap;
    int		c, r, g, b;
    Bool	contig;
    Pixel	*pixels;
    Pixel	*prmask, *pgmask, *pbmask;
{
    Pixel	*ppixRed, *ppixGreen, *ppixBlue;
    Pixel	*ppix, *pDst, *p;
    int		npix;
    Bool	okR, okG, okB;

    /* start out with empty pixels */
    for(p = pixels; p < pixels + c; p++)
	*p = 0;

    ppixRed = (Pixel *)ALLOCATE_LOCAL((c << r) * sizeof(Pixel));
    ppixGreen = (Pixel *)ALLOCATE_LOCAL((c << g) * sizeof(Pixel));
    ppixBlue = (Pixel *)ALLOCATE_LOCAL((c << b) * sizeof(Pixel));
    if (!ppixRed || !ppixGreen || !ppixBlue)
    {
	if (ppixBlue) DEALLOCATE_LOCAL(ppixBlue);
	if (ppixGreen) DEALLOCATE_LOCAL(ppixGreen);
	if (ppixRed) DEALLOCATE_LOCAL(ppixRed);
	return(BadAlloc);
    }

    okR = AllocCP(pmap, pmap->red, c, pmap->freeRed, r, contig,
		  ppixRed, prmask);
    okG = AllocCP(pmap, pmap->green, c, pmap->freeGreen, g, contig,
		  ppixGreen, pgmask);
    okB = AllocCP(pmap, pmap->blue, c, pmap->freeBlue, b, contig,
		  ppixBlue, pbmask);

    if (!okR || !okG || !okB)
    {
	if (okR)
	    for(ppix = ppixRed, npix = (c << r); --npix >= 0; ppix++)
		pmap->red[*ppix].refcnt = 0;
	if (okG)
	    for(ppix = ppixGreen, npix = (c << g); --npix >= 0; ppix++)
		pmap->green[*ppix].refcnt = 0;
	if (okB)
	    for(ppix = ppixBlue, npix = (c << b); --npix >= 0; ppix++)
		pmap->blue[*ppix].refcnt = 0;
	DEALLOCATE_LOCAL(ppixBlue);
	DEALLOCATE_LOCAL(ppixGreen);
	DEALLOCATE_LOCAL(ppixRed);
	return(BadAlloc);
    }

    *prmask <<= pmap->pVisual->offsetRed;
    *pgmask <<= pmap->pVisual->offsetGreen;
    *pbmask <<= pmap->pVisual->offsetBlue;

    npix = c << r;
    ppix = (Pixel *) xrealloc((pmap->clientPixelsRed)[client],
			((pmap->numPixelsRed)[client] + npix) * sizeof(Pixel));
    (pmap->clientPixelsRed)[client] = ppix;
    ppix += (pmap->numPixelsRed)[client];
    for (pDst = pixels, p = ppixRed; p < ppixRed + npix; p++)
    {
	*ppix++ = *p;
	if(p < ppixRed + c)
	    *pDst++ |= *p << pmap->pVisual->offsetRed;
    }
    (pmap->numPixelsRed)[client] += npix;
    pmap->freeRed -= npix;

    npix = c << g;
    ppix = (Pixel *) xrealloc((pmap->clientPixelsGreen)[client],
			((pmap->numPixelsGreen)[client] + npix) * sizeof(Pixel));
    (pmap->clientPixelsGreen)[client] = ppix;
    ppix += (pmap->numPixelsGreen)[client];
    for (pDst = pixels, p = ppixGreen; p < ppixGreen + npix; p++)
    {
	*ppix++ = *p;
	if(p < ppixGreen + c)
	    *pDst++ |= *p << pmap->pVisual->offsetGreen;
    }
    (pmap->numPixelsGreen)[client] += npix;
    pmap->freeGreen -= npix;

    npix = c << b;
    ppix = (Pixel *) xrealloc((pmap->clientPixelsBlue)[client],
			((pmap->numPixelsBlue)[client] + npix) * sizeof(Pixel));
    (pmap->clientPixelsBlue)[client] = ppix;
    ppix += (pmap->numPixelsBlue)[client];
    for (pDst = pixels, p = ppixBlue; p < ppixBlue + npix; p++)
    {
	*ppix++ = *p;
	if(p < ppixBlue + c)
	    *pDst++ |= *p << pmap->pVisual->offsetBlue;
    }
    (pmap->numPixelsBlue)[client] += npix;
    pmap->freeBlue -= npix;

    DEALLOCATE_LOCAL(ppixBlue);
    DEALLOCATE_LOCAL(ppixGreen);
    DEALLOCATE_LOCAL(ppixRed);

    return (Success);
}

static int
AllocPseudo (client, pmap, c, r, contig, pixels, pmask, pppixFirst)
    int		client;
    ColormapPtr	pmap;
    int		c, r;
    Bool	contig;
    Pixel	*pixels;
    Pixel	*pmask;
    Pixel	**pppixFirst;
{
    Pixel	*ppix, *p, *pDst, *ppixTemp;
    int		npix;
    int	result;

    npix = c << r;
    if(!(ppixTemp = (Pixel *)ALLOCATE_LOCAL(npix * sizeof(Pixel))))
	return(BadAlloc);
    result = AllocCP(pmap, pmap->red, c, pmap->freeRed, r, contig,
		     ppixTemp, pmask);

    if (result)
    {

	/* all the allocated pixels are added to the client pixel list,
	 * but only the unique ones are returned to the client */
	ppix = (Pixel *)xrealloc((pmap->clientPixelsRed)[client],
			 ((pmap->numPixelsRed)[client] + npix) * sizeof(Pixel));
	(pmap->clientPixelsRed)[client] = ppix;
	ppix += (pmap->numPixelsRed)[client];
	*pppixFirst = ppix;
	pDst = pixels;
	for (p = ppixTemp; p < ppixTemp + npix; p++)
	{
	    *ppix++ = *p;
	    if(p < ppixTemp + c)
	        *pDst++ = *p;
	}
	(pmap->numPixelsRed)[client] += npix;
	pmap->freeRed -= npix;
    }
    DEALLOCATE_LOCAL(ppixTemp);
    return (result ? Success : BadAlloc);
}

/* Allocates count << planes pixels from colormap pmap for client. If
 * contig, then the plane mask is made of consecutive bits.  Returns
 * all count << pixels in the array pixels. The first count of those
 * pixels are the unique pixels.  *pMask has the mask to Or with the
 * unique pixels to get the rest of them.
 *
 * Returns True iff all pixels could be allocated 
 * All cells allocated will have refcnt set to AllocPrivate and shared to FALSE
 * (see AllocShared for why we care)
 */
static int
AllocCP (pmap, pentFirst, count, Free, planes, contig, pixels, pMask)
    ColormapPtr	pmap;
    EntryPtr	pentFirst;
    int		count, Free, planes;
    Bool	contig;
    Pixel	*pixels, *pMask;
    
{
    EntryPtr	ent;
    Pixel	pixel, base, entries, maxp, save;
    int		dplanes, found;
    Pixel	*ppix;
    Pixel	mask;
    Pixel	finalmask;

    dplanes = pmap->pVisual->nplanes;

    /* Easy case.  Allocate pixels only */
    if (planes == 0)
    {
        if (count == 0 || count > Free)
    	    return (FALSE);

        /* allocate writable entries */
	ppix = pixels;
        ent = pentFirst;
        pixel = 0;
        while (--count >= 0)
	{
            /* Just find count unallocated cells */
    	    while (ent->refcnt)
	    {
    	        ent++;
    	        pixel++;
    	    }
    	    ent->refcnt = AllocPrivate;
    	    *ppix++ = pixel;
	    ent->fShared = FALSE;
        }
        *pMask = 0;
        return (TRUE);
    }
    else if ( count <= 0  || planes > dplanes ||
      (count << planes) > Free)
    {
	return (FALSE);
    }

    /* General case count pixels * 2 ^ planes cells to be allocated */

    /* make room for new pixels */
    ent = pentFirst;

    /* first try for contiguous planes, since it's fastest */
    for (mask = (1 << planes) - 1, base = 1, dplanes -= (planes - 1);
         --dplanes >= 0;
         mask += mask, base += base)
    {
        ppix = pixels;
        found = 0;
        pixel = 0;
        entries = pmap->pVisual->ColormapEntries - mask;
        while (pixel < entries)
	{
    	    save = pixel;
    	    maxp = pixel + mask + base;
    	    /* check if all are free */
    	    while (pixel != maxp && ent[pixel].refcnt == 0)
    	        pixel += base;
	    if (pixel == maxp)
		{
		    /* this one works */
		    *ppix++ = save;
		    found++;
		    if (found == count)
		    {
			/* found enough, allocate them all */
			while (--count >= 0)
			{
			    pixel = pixels[count];
			    maxp = pixel + mask;
			    while (1)
			    {
				ent[pixel].refcnt = AllocPrivate;
				ent[pixel].fShared = FALSE;
				if (pixel == maxp)
				    break;
				pixel += base;
				*ppix++ = pixel;
			    }
			}
			*pMask = mask;
			return (TRUE);
		    }
		}
    	    pixel = save + 1;
    	    if (pixel & mask)
    	        pixel += mask;
        }
    }

    dplanes = pmap->pVisual->nplanes;
    if (contig || planes == 1 || dplanes < 3)
	return (FALSE);

    /* this will be very slow for large maps, need a better algorithm */

    /*
       we can generate the smallest and largest numbers that fits in dplanes
       bits and contain exactly planes bits set as follows. First, we need to
       check that it is possible to generate such a mask at all.
       (Non-contiguous masks need one more bit than contiguous masks). Then
       the smallest such mask consists of the rightmost planes-1 bits set, then
       a zero, then a one in position planes + 1. The formula is
         (3 << (planes-1)) -1
       The largest such masks consists of the leftmost planes-1 bits set, then
       a zero, then a one bit in position dplanes-planes-1. If dplanes is
       smaller than 32 (the number of bits in a word) then the formula is:
         (1<<dplanes) - (1<<(dplanes-planes+1) + (1<<dplanes-planes-1)
       If dplanes = 32, then we can't calculate (1<<dplanes) and we have
       to use:
         ( (1<<(planes-1)) - 1) << (dplanes-planes+1) + (1<<(dplanes-planes-1))
	  
	  << Thank you, Loretta>>>

    */

    finalmask =
        (((1<<(planes-1)) - 1) << (dplanes-planes+1)) + (1<<(dplanes-planes-1));
    for (mask = (3 << (planes -1)) - 1; mask <= finalmask; mask++)
    {
        /* next 3 magic statements count number of ones (HAKMEM #169) */
        pixel = (mask >> 1) & 033333333333;
        pixel = mask - pixel - ((pixel >> 1) & 033333333333);
        if ((((pixel + (pixel >> 3)) & 030707070707) % 077) != planes)
    	    continue;
        ppix = pixels;
        found = 0;
        entries = pmap->pVisual->ColormapEntries - mask;
        base = lowbit (mask);
        for (pixel = 0; pixel < entries; pixel++)
	{
	    if (pixel & mask)
	        continue;
	    maxp = 0;
	    /* check if all are free */
	    while (ent[pixel + maxp].refcnt == 0)
	    {
		GetNextBitsOrBreak(maxp, mask, base);
	    }
	    if ((maxp < mask) || (ent[pixel + mask].refcnt != 0))
		continue;
	    /* this one works */
	    *ppix++ = pixel;
	    found++;
	    if (found < count)
		continue;
	    /* found enough, allocate them all */
	    while (--count >= 0)
	    {
		pixel = (pixels)[count];
		maxp = 0;
		while (1)
		{
		    ent[pixel + maxp].refcnt = AllocPrivate;
		    ent[pixel + maxp].fShared = FALSE;
		    GetNextBitsOrBreak(maxp, mask, base);
		    *ppix++ = pixel + maxp;
		}
	    }

	    *pMask = mask;
	    return (TRUE);
	}
    }
    return (FALSE);
}

static void
AllocShared (pmap, ppix, c, r, g, b, rmask, gmask, bmask, ppixFirst)
    ColormapPtr	pmap;
    Pixel	*ppix;
    int		c, r, g, b;
    Pixel	rmask, gmask, bmask;
    Pixel	*ppixFirst;	/* First of the client's new pixels */
{
    Pixel	*pptr, *cptr;
    Pixel	basemask;	/* bits not used in any mask */
    int		npix, z, npixClientNew;
    Pixel	base, bits;
    SHAREDCOLOR *pshared;

    basemask = ~(rmask | gmask | bmask);
    npixClientNew = c << (r + g + b);

    for(pptr = ppix, npix = c; --npix >= 0; pptr++)
    {
	bits = 0;
	base = lowbit (rmask);
	while(1)
	{
	    pshared = (SHAREDCOLOR *) xalloc (sizeof(SHAREDCOLOR));
	    pshared->refcnt = 1 << (g + b);
	    for (cptr = ppixFirst, z = npixClientNew; --z >= 0; cptr++)
	    {
		if (((*cptr & basemask) == ((*pptr | bits) & basemask)) &&
		    ((*cptr & rmask) == ((*pptr | bits) & rmask)))
		{
		    pmap->red[*cptr].fShared = TRUE;
		    pmap->red[*cptr].co.shco.red = pshared;
		}
	    }
	    GetNextBitsOrBreak(bits, rmask, base);
	}

	bits = 0;
	base = lowbit (gmask);
	while(1)
	{
	    pshared = (SHAREDCOLOR *) xalloc (sizeof(SHAREDCOLOR));
	    pshared->refcnt = 1 << (r + b);
	    for (cptr = ppixFirst, z = npixClientNew; --z >= 0; cptr++)
	    {
		if (((*cptr & basemask) == ((*pptr | bits) & basemask)) &&
		    ((*cptr & gmask) == ((*pptr | bits) & gmask)))
		{
		    pmap->red[*cptr].co.shco.green = pshared;
		}
	    }
	    GetNextBitsOrBreak(bits, gmask, base);

	}

	bits = 0;
	base = lowbit (bmask);
	while(1)
	{
	    pshared = (SHAREDCOLOR *) xalloc (sizeof(SHAREDCOLOR));
	    pshared->refcnt = 1 << (r + g);
	    for (cptr = ppixFirst, z = npixClientNew; --z >= 0; cptr++)
	    {
		if (((*cptr & basemask) == ((*pptr | bits) & basemask)) &&
		    ((*cptr & bmask) == ((*pptr | bits) & bmask)))
		{
		    pmap->red[*cptr].co.shco.blue = pshared;
		}
	    }
	    GetNextBitsOrBreak(bits, bmask, base);
	}

    }
}


/* Free colors and/or cells (probably slow for large numbers) */

FreeColors (pmap, client, count, pixels, mask)
    ColormapPtr	pmap;
    int		client, count;
    Pixel	*pixels;
    Pixel	mask;
{
    int		rval, result, class;


    class = pmap->class;
    if((pmap->flags & AllAllocated) || !(class & DynamicClass))
    {
	return(BadAccess);
    }
    if (class == DirectColor)
    {
        result = FreeCo(pmap, client, REDMAP, count, pixels, mask);
	/* If any of the three calls fails, we must report that, if more
	 * than one fails, it's ok that we report the last one */
        rval = FreeCo(pmap, client, GREENMAP, count, pixels, mask);
	if(rval != Success)
	    result = rval;
	rval = FreeCo(pmap, client, BLUEMAP, count, pixels, mask);
	if(rval != Success)
	    result = rval;
    }
    else
        result = FreeCo(pmap, client, PSEUDOMAP, count, pixels, mask);

    /* XXX should worry about removing any RT_CMAPENTRY resource */
    return (result);
}

/* Helper for FreeColors -- frees all combinations of *newpixels and mask bits
 * which the client has allocated in channel colormap cells of pmap.
 * doesn't change newpixels if it doesn't need to */
static int
FreeCo (pmap, client, color, npixIn, ppixIn, mask)
    ColormapPtr	pmap;		/* which colormap head */
    int		client;		
    int		color;		/* which sub-map, eg RED, BLUE, PSEUDO */
    int		npixIn;		/* number of pixels passed in */
    Pixel	*ppixIn;	/* list of base pixels */
    Pixel	mask;		/* mask client gave us */ 
{

    Pixel	*ppixClient, pixTest;
    int		npixClient, npixNew, npix;
    Pixel	bits, base, cmask;
    Pixel	*pptr, *cptr;
    int 	n, zapped;
    int		errVal = Success;
    int		offset;

    if (npixIn == 0)
        return (errVal);
    bits = 0;
    zapped = 0;
    base = lowbit (mask);

    switch(color)
    {
      case REDMAP:
	cmask = pmap->pVisual->redMask;
	offset = pmap->pVisual->offsetRed;
	ppixClient = (pmap->clientPixelsRed)[client];
	npixClient = (pmap->numPixelsRed)[client];
	break;
      case GREENMAP:
	cmask = pmap->pVisual->greenMask;
	offset = pmap->pVisual->offsetGreen;
	ppixClient = (pmap->clientPixelsGreen)[client];
	npixClient = (pmap->numPixelsGreen)[client];
	break;
      case BLUEMAP:
	cmask = pmap->pVisual->blueMask;
	offset = pmap->pVisual->offsetBlue;
	ppixClient = (pmap->clientPixelsBlue)[client];
	npixClient = (pmap->numPixelsBlue)[client];
	break;
      case PSEUDOMAP:
	cmask = ~((Pixel)0);
	offset = 0;
	ppixClient = (pmap->clientPixelsRed)[client];
	npixClient = (pmap->numPixelsRed)[client];
	break;
    }

    /* zap all pixels which match */
    while (1)
    {
        /* go through pixel list */
        for (pptr = ppixIn, n = npixIn; --n >= 0; pptr++)
	{
	    pixTest = ((*pptr | bits) & cmask) >> offset;
	    if (pixTest >= pmap->pVisual->ColormapEntries)
	    {
		clientErrorValue = *pptr;
		errVal = BadValue;
		continue;
	    }

	    /* find match in client list */
	    for (cptr = ppixClient, npix = npixClient;
	         --npix >= 0 && *cptr != pixTest;
		 cptr++) ;

	    if (npix >= 0)
	    {
		FreeCell(pmap, pixTest, color);
		*cptr = ~((Pixel)0);
		zapped++;
	    }
	    else
		errVal = BadAccess;
	}
        /* generate next bits value */
	GetNextBitsOrBreak(bits, mask, base);
    }


    /* delete freed pixels from client pixel list */
    if (zapped)
    {
        npixNew = npixClient - zapped;
        if (npixNew)
	{
	    /* Since the list can only get smaller, we can do a copy in
	     * place and then realloc to a smaller size */
    	    pptr = cptr = ppixClient;

	    /* If we have all the new pixels, we don't have to examine the
	     * rest of the old ones */
	    for(npix = 0; npix < npixNew; cptr++)
	    {
    	        if (*cptr != ~((Pixel)0))
		{
    		    *pptr++ = *cptr;
		    npix++;
    	        }
    	    }
	    ppixClient = (Pixel *)xrealloc(ppixClient,
					   npixNew * sizeof(Pixel));
	    npixClient = npixNew;
        }
	else
	{
	    npixClient = 0;
	    xfree(ppixClient);
    	    ppixClient = (Pixel *)NULL;
	}
	switch(color)
	{
	  case PSEUDOMAP:
	  case REDMAP:
	    (pmap->clientPixelsRed)[client] = ppixClient;
	    (pmap->numPixelsRed)[client] = npixClient;
	    break;
	  case GREENMAP:
	    (pmap->clientPixelsGreen)[client] = ppixClient;
	    (pmap->numPixelsGreen)[client] = npixClient;
	    break;
	  case BLUEMAP:
	    (pmap->clientPixelsBlue)[client] = ppixClient;
	    (pmap->numPixelsBlue)[client] = npixClient;
	    break;
	}
    }
    return (errVal);
}



/* Redefine color values */
int
StoreColors (pmap, count, defs)
    ColormapPtr	pmap;
    int		count;
    xColorItem	*defs;
{
    register Pixel 	pix;
    register xColorItem *pdef;
    register EntryPtr 	pent, pentT, pentLast;
    register VisualPtr	pVisual;
    SHAREDCOLOR		*pred, *pgreen, *pblue;
    int			n, ChgRed, ChgGreen, ChgBlue, idef;
    int			class, errVal = Success;
    int			ok;


    class = pmap->class;
    if(!(class & DynamicClass) && !(pmap->flags & BeingCreated))
    {
	return(BadAccess);
    }
    pVisual = pmap->pVisual;

    idef = 0;
    if((class | DynamicClass) == DirectColor)
    {
        for (pdef = defs, n = 0; n < count; pdef++, n++)
	{
	    ok = TRUE;
            (*pmap->pScreen->ResolveColor)
	        (&pdef->red, &pdef->green, &pdef->blue, pmap->pVisual);

	    pix = (pdef->pixel & pVisual->redMask) >> pVisual->offsetRed;
	    if (pix >= pVisual->ColormapEntries )
	    {
		clientErrorValue = pdef->pixel;
		errVal = BadValue;
		ok = FALSE;
	    }
	    else if (pmap->red[pix].refcnt != AllocPrivate)
	    {
		errVal = BadAccess;
		ok = FALSE;
	    }
	    else if (pdef->flags & DoRed)
	    {
		pmap->red[pix].co.local.red = pdef->red;
	    }

	    pix = (pdef->pixel & pVisual->greenMask) >> pVisual->offsetGreen;
	    if (pix >= pVisual->ColormapEntries )
	    {
		clientErrorValue = pdef->pixel;
		errVal = BadValue;
		ok = FALSE;
	    }
	    else if (pmap->green[pix].refcnt != AllocPrivate)
	    {
		errVal = BadAccess;
		ok = FALSE;
	    }
	    else if (pdef->flags & DoGreen)
	    {
		pmap->green[pix].co.local.green = pdef->green;
	    }

	    pix = (pdef->pixel & pVisual->blueMask) >> pVisual->offsetBlue;
	    if (pix >= pVisual->ColormapEntries )
	    {
		clientErrorValue = pdef->pixel;
		errVal = BadValue;
		ok = FALSE;
	    }
	    else if (pmap->blue[pix].refcnt != AllocPrivate)
	    {
		errVal = BadAccess;
		ok = FALSE;
	    }
	    else if (pdef->flags & DoBlue)
	    {
		pmap->blue[pix].co.local.blue = pdef->blue;
	    }
	    /* If this is an o.k. entry, then it gets added to the list
	     * to be sent to the hardware.  If not, skip it.  Once we've
	     * skipped one, we have to copy all the others.
	     */
	    if(ok)
	    {
		if(idef != n)
		    defs[idef] = defs[n];
		idef++;
	    }
	}
    }
    else
    {
        for (pdef = defs, n = 0; n < count; pdef++, n++)
	{

	    ok = TRUE;
	    if (pdef->pixel >= pVisual->ColormapEntries)
	    {
		clientErrorValue = pdef->pixel;
	        errVal = BadValue;
		ok = FALSE;
	    }
	    else if (pmap->red[pdef->pixel].refcnt != AllocPrivate)
	    {
		errVal = BadAccess;
		ok = FALSE;
	    }

	    /* If this is an o.k. entry, then it gets added to the list
	     * to be sent to the hardware.  If not, skip it.  Once we've
	     * skipped one, we have to copy all the others.
	     */
	    if(ok)
	    {
		if(idef != n)
		    defs[idef] = defs[n];
		idef++;
	    }
	    else
		continue;

            (*pmap->pScreen->ResolveColor)
	        (&pdef->red, &pdef->green, &pdef->blue, pmap->pVisual);

	    pent = &pmap->red[pdef->pixel];

	    if(pdef->flags & DoRed)
	    {
		if(pent->fShared)
		{
		    pent->co.shco.red->color = pdef->red;
		    if (pent->co.shco.red->refcnt > 1)
			ok = FALSE;
		}
		else
		    pent->co.local.red = pdef->red;
	    }
	    if(pdef->flags & DoGreen)
	    {
		if(pent->fShared)
		{
		    pent->co.shco.green->color = pdef->green;
		    if (pent->co.shco.green->refcnt > 1)
			ok = FALSE;
		}
		else
		    pent->co.local.green = pdef->green;
	    }
	    if(pdef->flags & DoBlue)
	    {
		if(pent->fShared)
		{
		    pent->co.shco.blue->color = pdef->blue;
		    if (pent->co.shco.blue->refcnt > 1)
			ok = FALSE;
		}
		else
		    pent->co.local.blue = pdef->blue;
	    }

	    if(!ok)
	    {
                /* have to run through the colormap and change anybody who
		 * shares this value */
	        pred = pent->co.shco.red;
	        pgreen = pent->co.shco.green;
	        pblue = pent->co.shco.blue;
	        ChgRed = pdef->flags & DoRed;
	        ChgGreen = pdef->flags & DoGreen;
	        ChgBlue = pdef->flags & DoBlue;
	        pentLast = pmap->red + pVisual->ColormapEntries;

	        for(pentT = pmap->red; pentT < pentLast; pentT++)
		{
		    if(pentT->fShared && (pentT != pent))
		    {
			xColorItem	defChg;

			/* There are, alas, devices in this world too dumb
			 * to read their own hardware colormaps.  Sick, but
			 * true.  So we're going to be really nice and load
			 * the xColorItem with the proper value for all the
			 * fields.  We will only set the flags for those
			 * fields that actually change.  Smart devices can
			 * arrange to change only those fields.  Dumb devices
			 * can rest assured that we have provided for them,
			 * and can change all three fields */

			defChg.flags = 0;
			if(ChgRed && pentT->co.shco.red == pred)
			{
			    defChg.flags |= DoRed;
			}
			if(ChgGreen && pentT->co.shco.green == pgreen)
			{
			    defChg.flags |= DoGreen;
			}
			if(ChgBlue && pentT->co.shco.blue == pblue)
			{
			    defChg.flags |= DoBlue;
			}
			if(defChg.flags != 0)
			{
			    defChg.pixel = pentT - pmap->red;
			    defChg.red = pentT->co.shco.red->color;
			    defChg.green = pentT->co.shco.green->color;
			    defChg.blue = pentT->co.shco.blue->color;
			    (*(pmap->pScreen->StoreColors)) (pmap, 1, &defChg);
			}
		    }
		}

	    }
	}
    }
    /* Note that we use idef, the count of acceptable entries, and not
     * count, the count of proposed entries */
    if (idef != 0)
	( *(pmap->pScreen->StoreColors)) (pmap, idef, defs);
    return (errVal);
}

int
IsMapInstalled(map, pWin)
    Colormap	map;
    WindowPtr	pWin;
{
    Colormap	*pmaps;
    int		imap, nummaps, found;

    pmaps = (Colormap *) ALLOCATE_LOCAL( 
             pWin->drawable.pScreen->maxInstalledCmaps * sizeof(Colormap));
    if(!pmaps)
	return(FALSE);
    nummaps = (*pWin->drawable.pScreen->ListInstalledColormaps)
        (pWin->drawable.pScreen, pmaps);
    found = FALSE;
    for(imap = 0; imap < nummaps; imap++)
    {
	if(pmaps[imap] == map)
	{
	    found = TRUE;
	    break;
	}
    }
    DEALLOCATE_LOCAL(pmaps);
    return (found);
}
