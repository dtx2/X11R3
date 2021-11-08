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

/*
 * qXss off-screen memory manager.  Treats off-screen memory as
 * a cache of pixmaps.  Imposes ruthless cache invalidation policies.
 *
 * syntax:
 * planemask =	tlConfirmPixmap( id, pPixmap, ry);
 * 		tlCancelPixmap( id);
 * planemask =	tlTempPixmap( id, pPixmap, ry);
 *
 * tlConfirmPixmap verifies that a pixmap has been loaded.
 * It requires a unique 32-bit key for the pixmap, typically the
 * address of some storage associated with the pixmap.
 *
 * If this storage is deallocated be sure to call tlCancelPixmap.
 * (the name "FreePixmap" is taken by the ddx layer)
 * It is harmless (except possibly to performance) to call tlCancelPixmap
 * redundantly or with a nonsense id.
 *
 * TempPixmap is like tlConfirmPixmap except that no record is
 * preserved of the pixmap having been loaded; the next call to
 * tlConfirmPixmap or tlTempPixmap will overwrite it.
 */

#include <sys/types.h>

#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"

#include "qd.h"

#include "tl.h"
#include "tltemplabels.h"

extern	unsigned	int	Allplanes;

#define QZHIGHY 864
#define QZLOWY  2032
#define QZMAXALLOCY     (QZLOWY - QZHIGHY)

#define MAXIM		0x100
#define INDEXBITS	0xff	/* MAXIM-1 */
static int BADBITS =	8;

typedef struct {
    int		tag1;
    int		maskplane;
    short	y;
    short	padtolongword;
} PixmapCache; 

extern int Nplanes;
extern int Nentries;

static PixmapCache	QDPixmapCache[ MAXIM];

/*
 * full-depth entries grow down from the top
 * single-plane entries grow up from the bottom
 */
static int	Lowclouds = QZHIGHY;	/* nothing up there yet */
static int	Highwater[] =		/* nothing down here yet */
	{QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,
	 QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,
	 QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY,QZLOWY};

#define QDHASH(lw)	( ((((lw)) >> BADBITS) ^ (lw)) & INDEXBITS)

/*
 * Load a pixel map which may (actually, will) be overwritten
 * the next time someone allocates off-screen space.
 *
 * returns the plane mask
 */
int
tlTempPixmap( id, pPixmap, ry)
    int		id;
    PixmapPtr	pPixmap;
    int *	ry;
{
    int		pm;

    pm = tlConfirmPixmap( id, pPixmap, ry);
    /*
     * Invalidate the cache entry which tlConfirmPixmap just validated
     */
    tlCancelPixmap( id);

    return pm;
}


/*
 * Confirm that an off-screen pixel map is loaded.
 *
 * A NULL pPixmap argument is permissible, and causes a check that
 * a pixel map was previously loaded with the argument id.
 *
 * The hash function is tuned to dealing well with unique "id"s which are
 * addresses of objects in virtual memory.
 *
 * writes the y location into *ry.
 * returns maskplane, or 0 if Pixmap could not be loaded
 */
int
tlConfirmPixmap( id, pPixmap, ry)
    register int		id;
    register PixmapPtr	pPixmap;
    register int *	ry;
{
    int		tag1 = id;
    register PixmapCache *pCEnt = &QDPixmapCache[ QDHASH( id)];

    /*
     *  if tag not in cache, attempt to load the pixel map
     */
    if (   pCEnt->tag1 != tag1)
    {
	/*
 	 * return failure if there is no pixel map to load
	 * allocate space, but don't do the transfer, if the pixmap has 
	 *  a NULL bits pointer.
	 */
        if ( !pPixmap)
	    return 0;
	/* return failure if pixmap is too large to load */
	if (pPixmap->width > 1024 || pPixmap->height > QZMAXALLOCY)
	    return 0;
	if ( pPixmap->drawable.depth == 1)
	{
	    if (   makeFBspace1( pCEnt, id, pPixmap->height)
		&& pPixmap->devPrivate)
	    {
		pCEnt->tag1 = tag1;
		QDptbxy( 0, pCEnt->y, pCEnt->maskplane,
			pPixmap->width, pPixmap->height,
			(short *)pPixmap->devPrivate);
	    }
	}
	else
        {
	    if ( makeFBspaceN( pCEnt, id, pPixmap->height)
		&& ((QDPixPtr)pPixmap->devPrivate)->data)
	    {
		pCEnt->tag1 = tag1;
		tlspadump( 0, pCEnt->y, pPixmap->width, pPixmap->height,
			((QDPixPtr)pPixmap->devPrivate)->data);
	    }
	}
    }
    *ry = pCEnt->y;
    if (pPixmap != NULL)
	((QDPixPtr) pPixmap->devPrivate)->offscreen = *ry;
    return pCEnt->maskplane;
}

int
tlCancelPixmap( id)
    int		id;
{
    register PixmapCache *pCEnt = &QDPixmapCache[ QDHASH( id)];

    pCEnt->tag1 = 0;
}

int
tlPurgePixmaps( tag1)
    int	tag1;
{
    register PixmapCache *pCEnt;
    int	count;

    for (
	    count = MAXIM, pCEnt = QDPixmapCache;
	    count;
	    --count, pCEnt++)
	if ( pCEnt->tag1 == tag1)
	    pCEnt->tag1 = 0;
}

/*
 * End of interface routines
 */

/*
 * Start of internal routines.
 */


/*
 * returns the number of entries thrown out
 */
static int
ThrowOutEntries( mask)
    int		mask;
{
    register PixmapCache *pPC;
    int			count = 0;

    for ( pPC=QDPixmapCache; pPC < &QDPixmapCache[ MAXIM]; pPC++)
	if ( pPC->maskplane == mask)
	{
	    pPC->maskplane = 0;
	    pPC->tag1 = 0;
	    count++;
	}
    if ( mask == Allplanes)
    {
#ifdef DEBUG
	ErrorF("threw out %d full-pixel-entries\n", count);
#endif
	Lowclouds = QZHIGHY;
    }
    else
    {
#ifdef DEBUG
	ErrorF("threw out %d single-plane-entries\n", count);
#endif
	Highwater[ ffs( mask)-1] = QZLOWY;
    }
    return count;
}

/*
 * given a pointer to a cache entry, and a height requirement,
 * finds space in the frame buffer and updates the cache entry.
 */
static int
makeFBspace1( pCEnt, id, h)
    register PixmapCache *pCEnt;	/* pointer to already allocated entry */
    caddr_t	id;
    register int	h;		/* FB height required */
{
    int			bestplane;
    int			maskbestplane;
    register int	lowhighwater = Lowclouds;
    register int	ip;

    /*
     * Find the plane which has the most space to spare.
     */
    for ( ip=0; ip<Nentries; ip++)
	if ( Highwater[ip] >= lowhighwater)	/* > means lower */
	{
	    bestplane = ip;
	    lowhighwater = Highwater[ip];
	}
    /*
     * If there is room between the Highwater and Lowclouds, take it.
     */
    if ( lowhighwater-Lowclouds >= h)
    {
	pCEnt->maskplane = 1<<bestplane;
	pCEnt->y = lowhighwater-h;

	Highwater[bestplane] -= h;
	return TRUE;
    }
    /*
     * Strategy time.
     * If no room in bestplane, pick another at random and throw all
     * of its entries out.
     */
#ifdef DEBUG
    ErrorF("makeFBspace1: freeing up space\n");
#endif
    bestplane = (int)pCEnt->tag1 % Nentries;
    maskbestplane = 1 << bestplane;
    /*
     * Try freeing up all the single-plane entries of one plane.
     * If that is not sufficient, blow out the full-pixel entries.
     */
    if ( ! ThrowOutEntries( maskbestplane))
	ThrowOutEntries( Allplanes);

    return makeFBspace1( pCEnt, id, h);	/* recursion! (only one call deep) */
}
 

/*
 * Given a pointer to a cache entry, and a height requirement,
 * finds space in the frame buffer and updates the cache entry.
 *
 * Contains the strategy decision to throw out as many single plane entries
 * as possible first.
 */
static int
makeFBspaceN( pCEnt, id, h)		/* N means full-depth: 8 or 24 planes */
    register PixmapCache *pCEnt;	/* pointer to already allocated entry */
    caddr_t	id;
    register int	h;		/* FB height required */
{
    int			worstplane;
    int			highhighwater;
    register int	i;
    register PixmapCache *pPC;

    do
    {
	/*
	 * Find the plane which has the least space to spare.
	 */
	highhighwater = QZLOWY;
	for ( i=0; i<Nentries; i++)
	    if ( Highwater[i] <= highhighwater)	/* < means higher */
	    {
		worstplane = i;
		highhighwater = Highwater[i];
	    }
	/*
	 * If no room because of worstplane, throw everyone in it out
	 */
	if ( highhighwater-Lowclouds < h)
	{
#ifdef DEBUG
	    ErrorF("makeFBspaceN: freeing up space\n");
#endif
	    ThrowOutEntries( 1 << worstplane);
	}
    } while ( ( highhighwater-Lowclouds < h)	     /* not enough room yet */
	&& highhighwater != Highwater[worstplane]);  /* more planes to go */

    /*
     * If there is room between the Highwater and Lowclouds, take it.
     */
    if ( highhighwater-Lowclouds >= h)
    {
	pCEnt->maskplane = Allplanes;
	pCEnt->y = Lowclouds;
	Lowclouds += h;
	return TRUE;
    }
    /*
     * Strategy time.
     * Throwing out all the single plane entries was not sufficient,
     * so throw out all the full-pixel-depth entries.
     */
    ThrowOutEntries( Allplanes);
    if ( ! makeFBspaceN( pCEnt, id, h))
    {
/* this never is executed, instead we get infinite recursion XXX */
#ifdef DEBUG
	ErrorF( "makeFBspaceN: could not make space \n");
#endif
	return FALSE;
    }
    return TRUE;
}
 

/*
 * single planes on left, full-depth entries on right
 */
DumpOffscreenState()
{
    int	pl;
#   define NCOLS	80

    for ( pl=0; pl<Nentries; pl++)
    {
	int ichar;

	for (	ichar=0;
		ichar < NCOLS*(QZLOWY-Highwater[pl])/QZMAXALLOCY;
		ichar++)
	    ErrorF( "%d", pl);
	for (	;
		ichar < NCOLS*(Lowclouds-QZHIGHY)/QZMAXALLOCY;
		ichar++)
	    ErrorF( " ");
	for (	;
		ichar < NCOLS;
		ichar++)
	    ErrorF( "X");
	ErrorF( "\n");
    }
    ErrorF( "\n");

}


#define WORDSWORDPAD( x, w)	((((x) % 16) + (w)+15)/16)

/*
 * Copy a bitmap from main memory into one plane of the frame buffer.
 *
 * This routine requires the leftmost bit of the pixel map to be aligned
 * with bit (x % 16) in the first short word of each scanline,
 * and the right end of each scanline to be padded to a
 * short word boundary.
 *
 *  the dma buffer has
 *  JMPT_SETVIPER24
 *  rop | FULL_SRC_RESOLUTION
 *  JMPT_RESETCLIP
 *  JMPT_PTOBXY
 *  maskplane
 *  x, y, width, height
 *  PTB | count
 *  pixvals
 *  JMPT_PTOBXYCLEAN
 */
static int
QDptbxy( x0, y0, maskplane, width, height, pshort)
    int                 x0, y0, maskplane;
    register int        width;
    register int        height;
    register short      *pshort;
{
    int	nshortsscanline;
    int	nscansperblock;
    int	nscansdone;

    extern int     req_buf_size;

    SETTRANSLATEPOINT( 0, 0);
    /* 
     * pad width to short word bounds on both ends
     */
    nshortsscanline = WORDSWORDPAD( x0, width);
    /*
     * save 15 words for setup display list
     */
    nscansperblock = ( min( req_buf_size, MAXDMAPACKET/sizeof(short)) - 15 )
							    / nshortsscanline;
    for (   nscansdone=0;
	    height-nscansdone > nscansperblock;
	    nscansdone+=nscansperblock, y0+=nscansperblock,
				    pshort+= nscansperblock*nshortsscanline)
	doptbxy( x0, y0, maskplane, width, nscansperblock, pshort);
    doptbxy( x0, y0, maskplane, width, height-nscansdone, pshort);
}

static int
doptbxy( x0, y0, maskplane, width, height, pshort)
    int			x0, y0, maskplane;
    register int	width;
    register int	height;
    register short *	pshort;
{
    register unsigned short *	p;
    register int		nshort = WORDSWORDPAD( x0, width) * height;

    INVALID_SHADOW;	/* XXX */
    Need_dma( 14 + nshort);
    *p++ = JMPT_SETRGBPLANEMASK;
    *p++ = 0xff;
    *p++ = 0xff;
    *p++ = 0xff;
    *p++ = JMPT_SETVIPER24;
    *p++ = LF_SOURCE | FULL_SRC_RESOLUTION;
    *p++ = JMPT_RESETCLIP;
    *p++ = JMPT_PTOBXY;			/* Only uses first 8 planes */
    *p++ = maskplane & Allplanes;	/* Only uses first 8 planes */
    *p++ = x0 & 0x3fff;
    *p++ = y0 & 0x3fff;
    *p++ = width & 0x3fff;
    *p++ = height & 0x3fff;
    /*
     * DGA magic bit pattern for PTB, see VCB02 manual
     */
    *p++ = 0x6000 |(0x1fff & -nshort);
    while (nshort--)
        *p++ = *pshort++;
    Confirm_dma();
}
