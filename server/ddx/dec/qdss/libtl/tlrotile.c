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

#include "X.h"
#include "windowstr.h"
#include "gcstruct.h"

/*
 * driver headers
 */
#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"

#include	"qdprocs.h"

#include "qd.h"
#include "tltemplabels.h"
#include "tl.h"


/* we don't care about the translate point.  rotile template code doesn't
 *	use it.  Confirm sets it itself.
 */
VOID
tlrotile( ppixmap, patOrg, ptileOrg, pmask, pmagic )
PixmapPtr	ppixmap;
DDXPointRec	patOrg, *ptileOrg;
int		*pmask, *pmagic;
{
    register unsigned short *p;
    int		tilexoff;	/* from tile origin to current point */
    int		tileyoff;	/* from tile origin to current point */
    QDPixPtr	pPpriv = (QDPixPtr)(ppixmap->devPrivate);
    int		nby = 0;	/* short count for rotile packs */
    int		xextra;	/* number of extra tiles on xaxis of rotated copy */
    int		yextra;	/* number of extra tiles on yaxis of rotated copy */

    patOrg.x = UMOD( patOrg.x, ppixmap->width);
    patOrg.y = UMOD( patOrg.y, ppixmap->height);
    ptileOrg->x = (ppixmap->width) >> 4;
    ptileOrg->x = ((ptileOrg->x) << 4) & 0x3fff;
    /* round to dragon word boundary */
    if (ptileOrg->x != ppixmap->width)
	ptileOrg->x += 16;
    if (ppixmap->width < QD_MINXTILE)
    {
	xextra = QD_MINXTILE / ppixmap->width - 1;
	*pmagic = power2bit(QD_MINXTILE) - 2;
    } else
    {
	xextra = 0;
	*pmagic = power2bit(ppixmap->width) - 2;
    }
    if (ppixmap->height < QD_MINYTILE)
    {
	yextra = QD_MINYTILE / ppixmap->height - 1;
	*pmagic |= (power2bit(QD_MINYTILE) - 2) << 4;
    } else
    {
	yextra = 0;
	*pmagic |= (power2bit(ppixmap->height) - 2) << 4;
    }
    *pmagic &= 0x3fff;
    /* is it offscreen already? */
    if (*pmask = tlConfirmPixmap( ppixmap->devPrivate, NULL, &(ptileOrg->y)))
    {
	/* same rotation? */
	if (pPpriv->lastOrg.x == patOrg.x && pPpriv->lastOrg.y == patOrg.y)
	    return;	/* all is groovy... */
    } else
    {	/* nope--reload it offscreen */
	if (!(*pmask = tlConfirmPixmap( ppixmap->devPrivate, ppixmap,
		&(ptileOrg->y))))
	    FatalError( "tlrotile: could not store tile off-screen\n");
    }
    pPpriv->lastOrg.x = patOrg.x;
    pPpriv->lastOrg.y = patOrg.y;
    /* planes+setvip+rotile_base + rotile_packs */
    for (tileyoff = patOrg.y + yextra * ppixmap->height;
    	tileyoff > -ppixmap->height; tileyoff -= ppixmap->height)
    {
        for (tilexoff = patOrg.x + xextra * ppixmap->width;
    	    tilexoff > -ppixmap->width; tilexoff -= ppixmap->width)
        {
    	    nby += 2;		/* rotile_packs */
	}
    }

    INVALID_SHADOW;	/* XXX */
    Need_dma(15 + nby);	/* planes+vipers+rotile_base */
    *p++ = JMPT_SETRGBPLANEMASK;
    if (*pmask >= 0xff)
    {
    	*p++ = 0xff;	/* full-depth */
    	*p++ = 0xff;
    	*p++ = 0xff;
    } else
    {
	*p++ = 0x0;	/* single-depth, in green zblock */
	*p++ = (*pmask) & 0x3fff;
	*p++ = 0x0;
    }
    *p++ = JMPT_SETVIPER24;	/* for rotile */
    *p++ = umtable[GXcopy] | FULL_SRC_RESOLUTION;
	/*
	 * RoTile
	 *	this rotates the offscreen tile.
	 *	the result is placed at tilex, tiley.
	 * DMA packet (RoTile)
	 *       srcx,srcy,dx,dy
	 *       clipx1,clipx2,clipy1.clipy2  (single clip rect)
	 *        {dstx,dsty}*     (new dest; same size, clip)
	 */
    *p++ = JMPT_INITROTILE;
    *p++ = 0 & 0x3fff;	/* src x */
    *p++ = (ptileOrg->y) & 0x3fff;	/* src y */
    *p++ = ppixmap->width & 0x3fff;	/* width */
    *p++ = ppixmap->height & 0x3fff;	/* height */
    *p++ = (ptileOrg->x) & 0x3fff;		/* bitblt clipping */
    *p++ = ((ptileOrg->x)+(xextra+1)*ppixmap->width) & 0x3fff;
    *p++ = (ptileOrg->y) & 0x3fff;
    *p++ = ((ptileOrg->y)+(yextra+1)*ppixmap->height) & 0x3fff;
	/*
	 * four bitblt's (a la drewry)
	 */
    for (tileyoff = patOrg.y + yextra * ppixmap->height;
    	tileyoff > -ppixmap->height; tileyoff -= ppixmap->height)
    {
        for (tilexoff =  patOrg.x + xextra * ppixmap->width;
    	    tilexoff > -ppixmap->width; tilexoff -= ppixmap->width)
        {
	    *p++ = ((ptileOrg->x)+tilexoff) & 0x3fff;	/* dst x */
	    *p++ = ((ptileOrg->y)+tileyoff) & 0x3fff;	/* dst y */
        }
    }

    /*----> Confirm that end of buffer is not exceeded*/
    Confirm_dma();

}
