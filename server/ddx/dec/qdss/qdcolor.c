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

#include <sys/types.h>

#include "X.h"		/* required for DoRed ... */
#include "Xproto.h"	/* required for xColorItem */

#include "misc.h"	/* required for colormapst.h */
#include "colormapst.h"
#include "resource.h"
#include "scrnintstr.h"

/* There are used by QDSS */
#include "qdioctl.h"
#include "qduser.h"
/* end - QDSS */

#include "qd.h"

#define NOMAPYET	(ColormapPtr) 1
ColormapPtr	pInstalledMap;

extern int fd_qdss;	/* possibly should be in a header 	XX */
extern int Vaxstar;
extern int Nplanes;
extern int	TellLostMap(), TellGainedMap();

/*
 * qdScreenInit calls DIX CreateColormap to create the default colormap.
 */
void
qdCreateColormap( pcmap)
    ColormapPtr     pcmap;
{
    /*
     * if class is read-only, initialize the color values
     */
#if NPLANES==24
    if ( pcmap->class == TrueColor)
    {
	Entry *	red = pcmap->red;
	Entry *	green = pcmap->green;
	Entry *	blue = pcmap->blue;
	int	i;

	for ( i=0; i<pcmap->pVisual->ColormapEntries; i++)
	{
	    red[i].co.local.red =
	    green[i].co.local.green =
	    blue[i].co.local.blue = i<<8;
	    red[i].fShared = green[i].fShared = blue[i].fShared = FALSE;
	}
    }
#else /* NPLANES==8 or 4 */
    /*
     * 3/3/2 bit fields of R/G/B within a pixel value
     */
    if ( pcmap->class == StaticColor)
    {
        Entry *	red = pcmap->red;
	int	i;

        for ( i=0; i<pcmap->pVisual->ColormapEntries; i++)
	{
	    red[i].co.local.red = (i&0xe0)<<(8+0);
	    red[i].co.local.green = (i&0x1c)<<(8+3);
	    red[i].co.local.blue = (i&0x3)<<(8+6);
	    red[i].fShared = FALSE;
	}
    }
#endif
}

void
qdDestroyColormap( pcmap)
    ColormapPtr     pcmap;
{
}

int
qdListInstalledColormaps( pscr, pcmaps)
    ScreenPtr	pscr;
    Colormap *	pcmaps;
{
    *pcmaps = pInstalledMap->mid;
    return 1;
}

/*
 * impersonate the dispatcher, calling qdStoreColors for each color in the
 * colormap to be installed.
 *
 * qdScreenInit calls qdInstallColormap.
 */
void
qdInstallColormap( pcmap)
    ColormapPtr	pcmap;
{
    int		entries = pcmap->pVisual->ColormapEntries;
    Pixel *	ppix = (Pixel *)alloca( entries * sizeof(Pixel));
    xrgb *	prgb = (xrgb *)alloca( entries * sizeof(xrgb));
    xColorItem *defs = (xColorItem *)alloca( entries * sizeof(xColorItem));
    int		i;

    void qdStoreColors();

    if ( pcmap == pInstalledMap)
	return;

    if ( pInstalledMap != NOMAPYET)
	WalkTree( pcmap->pScreen, TellLostMap, &pInstalledMap->mid);
    pInstalledMap = pcmap;
#if NPLANES==24
    for ( i=0; i<entries; i++)
	ppix[i] = i | i<<8 | i<<16;
#else
    for ( i=0; i<entries; i++)
	ppix[i] = i;
#endif
    QueryColors( pcmap, entries, ppix, prgb); 
    for ( i=0; i<entries; i++) /* convert xrgbs to xColorItems */
    {
	defs[i].pixel = ppix[i];
	defs[i].red = prgb[i].red;
	defs[i].green = prgb[i].green;
	defs[i].blue = prgb[i].blue;
	defs[i].flags =  DoRed|DoGreen|DoBlue;
    }
    qdStoreColors( pcmap, entries, defs);
    WalkTree(pcmap->pScreen, TellGainedMap, &pcmap->mid);

}

void
qdUninstallColormap( pcmap)
    ColormapPtr	pcmap;
{
    if ( pcmap != pInstalledMap)
        return;

    WalkTree(pcmap->pScreen, TellLostMap, &pcmap->mid);
    qdInstallColormap ((ColormapPtr) LookupID( pcmap->pScreen->defColormap,
			RT_COLORMAP, RC_CORE));
}

#define EXTRACTCOLOR( c )	(((Nplanes == 4) && Vaxstar)\
				? (((c)>>12) & 0xff) : (((c)>>8) & 0xff))
#define INSERTCOLOR( c )		((c)<<8)

/*
 * limit to the values that are reasonable to present to the DACs
 * only the high-order byte is useful
 */
void
qdResolveColor( pred, pgreen, pblue)
    unsigned short	*pred;
    unsigned short	*pgreen;
    unsigned short	*pblue;
{
    *pred = *pred & 0xff00;
    *pgreen = *pgreen & 0xff00;
    *pblue = *pblue & 0xff00;
}


/*
 * write to the dragon's color map(s)
 *
 * Storing each channel separately ensures we will never overrun
 * the driver's color_buf.
 */
void
qdStoreColors( pcmap, idef, defs)
    ColormapPtr	pcmap;
    int		idef;
    xColorItem *defs;
{
    struct color_buf *  color_struct;

    if ( ioctl( fd_qdss, QD_MAPCOLOR, &color_struct) < 0) {
	FatalError( "Couldn't get ptr to color map.\n");
    }

    if (   pcmap != pInstalledMap
	&& pcmap != NULL)	/* hack for changing cursor colors */
	return;

#if NPLANES==24
    storeChannel( color_struct, idef, defs, DoRed);
    storeChannel( color_struct, idef, defs, DoGreen);
    storeChannel( color_struct, idef, defs, DoBlue);
#else
    storeChannel( color_struct, idef, defs, DoRed|DoGreen|DoBlue);
#endif

    ioctl (fd_qdss, QD_UNMAPCOLOR);
}

/*
 * The hardware color map must be shadowed in software because the
 * QZSS device driver insists on updating all three of red, green and blue
 * of a given hardware color map index in parallel; whereas X11 specifies
 * pixel values with different red, green and blue indices.
 *
 * pInstalledMap could be used as the shadow, although a format conversion
 * would be required.
 */
static
storeChannel( color_struct, idef, defs, chan)
    struct color_buf *	color_struct;
    int		idef;
    xColorItem *defs;
    int         chan;
{
    int         ic;
    register int	i;
    register int	imap;
    register char *	status;
    static struct rgb 	shadowmap[256];	/* "offset" field not used */
    int	done = 0;

    for ( ic=0; ic<idef; ic++, defs++)
    {
	/*
	 * 24-plane: we are assuming that chan is set to only one
	 *   of Do{Red,Green,Blue}
	 */
#if NPLANES==24
	if (chan & DoRed)
	    imap = RED(defs->pixel);
	else if (chan & DoGreen)
	    imap = GREEN(defs->pixel);
	else if (chan & DoBlue)
	    imap = BLUE(defs->pixel);
#else
	imap = defs->pixel;
#endif
	(color_struct->rgb)[ ic].offset = imap;
	(color_struct->rgb)[ ic].red = shadowmap[ imap].red =
	    (defs->flags & DoRed && chan & DoRed)
	    ? EXTRACTCOLOR( defs->red)
	    : shadowmap[ imap].red;
	(color_struct->rgb)[ ic].green = shadowmap[ imap].green =
	    (defs->flags & DoGreen && chan & DoGreen)
	    ? EXTRACTCOLOR( defs->green)
	    : shadowmap[ imap].green;
	(color_struct->rgb)[ ic].blue = shadowmap[ imap].blue =
	    (defs->flags & DoBlue && chan & DoBlue)
	    ? EXTRACTCOLOR( defs->blue)
	    : shadowmap[ imap].blue;
    }

    color_struct->count = idef;
    color_struct->status = LOAD_COLOR_MAP;

    status = &(color_struct->status);
    while (!done)
    {
        i = 100000;
        while (((*status & LOAD_COLOR_MAP) == LOAD_COLOR_MAP) && (i > 0))
	        i--;
        if (!(i))
	    ErrorF("Timed out in LoadColorMap. status = %d\n",
		(int) (*status));
        else
	    done = 1;
    }
}
