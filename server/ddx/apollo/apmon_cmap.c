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
 * Functions implementing monochrome-specific parts of the driver
 * having to do with colormaps.
 *
 * These routines are nearly vacuous.
 */

#include "apollo.h"

/*
 * apMonoResolveColor -- DDX interface (screen)
 *      Find the nearest monochrome color (i.e. white or black) to a given
 *      color.  Adjust the input red, green and blue values to all-ones
 *      or all-zeroes accordingly.
 */
void
apMonoResolveColor(pred, pgreen, pblue, pVisual)
    unsigned short      *pred, *pgreen, *pblue;
    VisualPtr           pVisual;
{
    /* Gets intensity from RGB.  If intensity is >= half, pick white, else
     * pick black.  This may well be more trouble than it's worth. */
    *pred = *pgreen = *pblue = 
        (((39L * *pred +
           50L * *pgreen +
           11L * *pblue) >> 8) >= (((1<<8)-1)*50)) ? ~0 : 0;
}

/*
 * apMonoCreateColormap -- DDX interface (screen)
 *      Allocate black for pixel value 0 and white for pixel value 1.
 */
void
apMonoCreateColormap(pmap)
    ColormapPtr pmap;
{
    int red, green, blue, pix;

    /* this is a monochrome colormap, it only has two entries, just fill
     * them in by hand.  If it were a more complex static map, it would be
     * worth writing a for loop or three to initialize it */
    pix = 0;
    red = green = blue = 0;
    AllocColor(pmap, &red, &green, &blue, &pix, 0);
    red = green = blue = ~0;
    AllocColor(pmap, &red, &green, &blue, &pix, 0);

}

/*
 * apMonoDestroyColormap -- DDX interface (screen)
 *      Nothing to do, since we didn't allocate any dynamic resources
 *      in apMonoCreateColormap.
 */
void
apMonoDestroyColormap(pmap)
    ColormapPtr pmap;
{
}
