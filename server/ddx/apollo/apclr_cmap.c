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
                    
/*
 * Functions implementing color-specific parts of the driver
 * having to do with colormaps.
 */

#include "apollo.h"

#include "colormapst.h"

extern int  TellLostMap(), TellGainedMap();

/* Constant value for GPR colormap entry to be ignored */
#define DONT_UPDATE (0xFFFFFFFF)

/*
 * apClrCreateColormap -- DDX interface (screen)
 *      If the colormap being created is the default one, allocate whitePixel and
 *      blackPixel.
 */
void
apClrCreateColormap(pmap)
    ColormapPtr pmap;
{
    ScreenPtr           pScreen;
    unsigned short      dark = 0;
    unsigned short      bright = ~0;

    pScreen = pmap->pScreen;

    if ((pmap->flags) & IsDefault)
    {
        AllocColor(pmap, &bright, &bright, &bright, &(pScreen->whitePixel), 0);
        AllocColor(pmap, &dark, &dark, &dark, &(pScreen->blackPixel), 0);
    }
}

/*
 * apClrDestroyColormap -- DDX interface (screen)
 *      Nothing to do, since we didn't allocate any dynamic resources
 *      in apClrCreateColormap.
 */
void
apClrDestroyColormap(pmap)
    ColormapPtr pmap;
{
}

/*
 * apUpdateColormap -- Driver internal code
 *      Update the hardware color map of the specified Apollo display.
 *      Special hack:  if an entry has the value DONT_UPDATE,
 *      that means don't change this color slot.  This may reduce
 *      technicolor effects when installing a new color map.
 */
static void
apUpdateColormap(pDisp, start_index, n_entries, gpr_map)
    apDisplayDataPtr    pDisp;
    int                 start_index;
    int                 n_entries;
    gpr_$color_vector_t gpr_map;
{
    gpr_$bitmap_desc_t  cur_bitmap;
    status_$t           status;
    gpr_$pixel_value_t  i1, i2, l;
    short               n;

    gpr_$inq_bitmap (cur_bitmap, status);
    if ( (pDisp->display_bitmap) != cur_bitmap )
        gpr_$set_bitmap (pDisp->display_bitmap, status);

    i1 = 0;
    while (i1 < n_entries)
    {
        if (gpr_map[i1] == DONT_UPDATE)
            i1++;
        else
        {
            i2 = i1;
            do i2++; while ( (i2 < n_entries) && (gpr_map[i2] != DONT_UPDATE) );
            n = i2 - i1;
            gpr_$set_color_map (i1+start_index, n, gpr_map[i1], status);
            i1 = i2;
        }
    }

    if ( (pDisp->display_bitmap) != cur_bitmap )
        gpr_$set_bitmap (cur_bitmap, status);
}
/*
 * apScanAndUpdateColormap -- Driver internal code
 *      Given the DIX colormap, convert it to GPR form and call
 *      apUpdateColormap on it.
 */
void
apScanAndUpdateColormap(pDisp, pmap)
    apDisplayDataPtr    pDisp;
    ColormapPtr         pmap;
{
    int                 i;
    int                 nEnt;
    Entry              *pEnt;
    gpr_$color_vector_t gpr_map;
    unsigned long       red, green, blue;

    nEnt = pmap->pVisual->ColormapEntries;
    for (i = 0, pEnt = pmap->red; i < nEnt; i++, pEnt++)
        if (pEnt->refcnt)
        {
            if (pEnt->fShared)
            {
                red = pEnt->co.shco.red->color >> 8;
                green = pEnt->co.shco.green->color >> 8;
                blue = pEnt->co.shco.blue->color >> 8;
            }
            else
            {
                red = pEnt->co.local.red >> 8;
                green = pEnt->co.local.green >> 8;
                blue = pEnt->co.local.blue >> 8;
            }
            gpr_map[i] = (red << 16) | (green << 8) | blue;
        }
        else
            gpr_map[i] = DONT_UPDATE;

    apUpdateColormap (pDisp, 0, nEnt, gpr_map);
}

/*
 * apClrInstallColormap -- DDX interface (screen)
 *      Install the given colormap on its screen.  If it's already installed,
 *      do nothing.  If another is installed, uninstall it first (we only support
 *      one color map at a time).
 */
void
apClrInstallColormap(pmap)
    ColormapPtr pmap;
{
    apDisplayDataPtr    pDisp;
    ScreenPtr           pScreen;

    pScreen = pmap->pScreen;
    pDisp = &apDisplayData[pScreen->myNum];

    if (pmap != pDisp->installedCmap)
    {
        if (pDisp->installedCmap)
            WalkTree (pScreen, TellLostMap, (char *) &(pDisp->installedCmap->mid));
        pDisp->installedCmap = pmap;

        apScanAndUpdateColormap (pDisp, pmap);

        WalkTree (pmap->pScreen, TellGainedMap, (char *) &(pmap->mid));
    }
}

/*
 * apClrUninstallColormap -- DDX interface (screen)
 *      Uninstall the given colormap on its screen, assuming it's the installed
 *      one and not the default.  Install the default colormap instead.
 */
void
apClrUninstallColormap(pmap)
    ColormapPtr pmap;
{
    apDisplayDataPtr    pDisp;
    Colormap            defaultMapID;
    ColormapPtr         pDefaultMap;

    pDisp = &apDisplayData[pmap->pScreen->myNum];
    if (pmap == pDisp->installedCmap)
    {
        defaultMapID = pmap->pScreen->defColormap;
        if (pmap->mid != defaultMapID)
        {
            pDefaultMap = (ColormapPtr) LookupID (defaultMapID, RT_COLORMAP, RC_CORE);
            apClrInstallColormap (pDefaultMap);
        }
    }
}

/*
 * apClrListInstalledColormaps -- DDX interface (screen)
 *      Return the list of installed colormaps for a screen, which for us will
 *      always be of length 1.
 */
int
apClrListInstalledColormaps(pScreen, pCmapList)
    ScreenPtr   pScreen;
    Colormap   *pCmapList;
{
    apDisplayDataPtr    pDisp;

    pDisp = &apDisplayData[pScreen->myNum];
    *pCmapList = pDisp->installedCmap->mid;
    return (1);
}

/*
 * apClrStoreColors -- DDX interface (screen)
 *      If the given colormap is the installed one, modify the hardware
 *      colormap entries with the given list of new entries.
 */
void
apClrStoreColors(pmap, ndef, pdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem *pdefs;
{
    apDisplayDataPtr    pDisp;

    pDisp = &apDisplayData[pmap->pScreen->myNum];
    if (pmap == pDisp->installedCmap)
    {
        int                 i, npix;
        gpr_$pixel_value_t  pix, minpix, maxpix;
        unsigned long       red, green, blue;
        xColorItem         *pd;
        gpr_$color_vector_t gpr_map;

        for (i = ndef , pd = pdefs , maxpix = 0 , minpix = 0x000000FF ; i-- ; pd++)
        {
            pix = (pd->pixel) & 0x000000FF;
            if (maxpix < pix) maxpix = pix;
            if (minpix > pix) minpix = pix;
        }
        npix = maxpix - minpix + 1;
        for (i = 0 ; i < npix ; i++)
            gpr_map[i] = DONT_UPDATE;

        for (i = ndef , pd = pdefs ; i-- ; pd++)
        {
            pix = (pd->pixel) & 0x000000FF;
            red = (pd->red) >> 8;
            green = (pd->green) >> 8;
            blue = (pd->blue) >> 8;

            gpr_map[pix-minpix] = (red << 16) | (green << 8) | blue;
        }
        apUpdateColormap (pDisp, minpix, npix, gpr_map);
    }
}

/*
 * apClrResolveColor -- DDX interface (screen)
 *      Find the nearest displayable color to a given color.
 *      Adjust the input red, green and blue values accordingly.
 *      We do this by getting the number of significant color bits
 *      from the visual, and masking off all insignificant bits.
 */
void
apClrResolveColor(pred, pgreen, pblue, pVisual)
    unsigned short      *pred, *pgreen, *pblue;
    VisualPtr           pVisual;
{
    short           n_color_bits;
    unsigned short  mask;

    n_color_bits = pVisual->bitsPerRGBValue;
    mask = ((1 << n_color_bits) - 1) << (16 - n_color_bits);
    *pred &= mask;
    *pgreen &= mask;
    *pblue &= mask;
}
