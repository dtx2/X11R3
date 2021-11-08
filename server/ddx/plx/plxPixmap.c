/*
 *   Copyright (c) 1987, 88 by
 *   PARALLAX GRAPHICS, INCORPORATED, Santa Clara, California.
 *   All rights reserved
 *
 *   This software is furnished on an as-is basis, and may be used and copied
 *   only with the inclusion of the above copyright notice.
 *
 *   The information in this software is subject to change without notice.
 *   No committment is made as to the usability or reliability of this
 *   software.
 *
 *   Parallax Graphics, Inc.
 *   2500 Condensa Street
 *   Santa Clara, California  95051
 */

#ifndef lint
static char *sid_ = "@(#)plxPixmap.c	1.31 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

PixmapPtr
plxCreatePixmap(pScreen, width, height, depth)
ScreenPtr pScreen;
int width;
int height;
int depth;
{
	register PixmapPtr pPixmap;
	register MapPriv *mp;

	ifdebug(1) printf("plxCreatePixmap(), w,h,depth=%d,%d,%d\n", width, height, depth);

	switch (depth) {
	case 1:
	case 8:
		break;
	default:
		ifdebug(1) printf("plxCreatePixmap() NULL, bad depth\n");
		return (NullPixmap);
	}

	mp = (MapPriv *) Xalloc(sizeof(MapPriv));
	mp->plxcache = (plxCache *)NULL;
	mp->plxmemorycopy = NULL;
	mp->plxtile.expand = FALSE;
	mp->plxtile.x = mp->plxtile.y = 0;
#ifdef notdef
	mp->plxtile.rotate = FALSE;
	mp->plxtile.original = NullPixmap;
#endif
	mp->video = FALSE;

	pPixmap = (PixmapPtr)Xalloc(sizeof(PixmapRec));
	pPixmap->drawable.type = DRAWABLE_PIXMAP;
	pPixmap->drawable.pScreen = pScreen;
	pPixmap->drawable.depth = depth;
	pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pPixmap->width = width;
	pPixmap->height = height;
	pPixmap->refcnt = 1;
	pPixmap->devPrivate = (pointer)mp;
	switch (depth) {
	case 1:
		pPixmap->devKind = PixmapBytePad(width, depth);
		break;
	case 8:
		pPixmap->devKind = (((width + 3) >> 2) << 2);
		break;
	}

	if (!plxcachepixmap(pPixmap)) {
		plxDestroyPixmap (pPixmap);
		return (PixmapPtr)NULL;
	}

	ifdebug(1) printf("plxCreatePixmap(), 0x%08x\n", pPixmap);

	return (pPixmap);
}

Bool
plxDestroyPixmap(pPixmap)
register PixmapPtr pPixmap;
{
	register MapPriv *mp;

	ifdebug(1) printf("plxDestroyPixmap() 0x%08x\n", pPixmap);

        if (!IS_VALID_PIXMAP(pPixmap))
		return (TRUE);

	if (--pPixmap->refcnt) {
		return (TRUE);
	}

	mp = (MapPriv *)pPixmap->devPrivate;
	if (mp->plxcache)
		pl_cache_free(mp->plxcache, 0);
	if (mp->plxmemorycopy)
		Xfree(mp->plxmemorycopy);
	Xfree(mp);
	Xfree(pPixmap);
	return (TRUE);
}

void
plxreadbackpixmap(ce, pPixmap)
register plxCache *ce;
register PixmapPtr pPixmap;
{
	register MapPriv *mp = (MapPriv *)pPixmap->devPrivate;
	register int size;

	ifdebug(1) printf("plxreadbackpixmap() 0x%08x\n", pPixmap);

	if (mp->plxcache != ce) {
		printf("plxreadbackpixmap(), cache mismatch\n");
		return;
	}
	if (mp->plxmemorycopy) {
		printf("plxreadbackpixmap(), already have copy in host\n");
		return;
	}

	/* we could unload based on pPixmap->depth, but... */
	size = ce->grp->sizex * ce->grp->sizey;
	mp->plxmemorycopy = (char *)Xalloc(size);

	/*
	 * the routine is only called from another pl_*()
	 * call, hence we are working in a rmap/opaq/mask clean
	 * enviroment
	 */
	p_uimg(ce->x, ce->y, ce->x + ce->grp->sizex - 1, ce->y - ce->grp->sizey + 1, mp->plxmemorycopy);
}

Bool
plxcachepixmap(pPixmap)
register PixmapPtr pPixmap;
{
	register MapPriv *mp = (MapPriv *)pPixmap->devPrivate;
	register plxCache *ce = mp->plxcache;
	Bool just_allocated = FALSE;

	ifdebug(1) printf("plxcachepixmap() 0x%08x\n", pPixmap);

	p_mask(0xffff);
	p_rmap(0);
	p_opaq(0);
	p_damvg();
	plxclipinvalidate ();

	if (!ce) {
		ce = pl_cache_find(pPixmap->width, pPixmap->height);
		if (!ce) {
			ifdebug(1) printf ("plxcachepixmap() failed, size %dx%d\n",
					pPixmap->width, pPixmap->height);
			return(FALSE);
		}
		mp->plxcache = ce;
		/*
		 * set readback function for cache, this is our
		 * hook into the cache reclaim code.
		 */
		pl_cache_readback(ce, plxreadbackpixmap, pPixmap);
		just_allocated = TRUE;
	}
	
	if (mp->video)
		p_damvv();
	else
		p_damvg();

	if (mp->plxmemorycopy) {
		/*
		 * gosh, we got thrown out of the cache.
		 */
		p_limg(ce->x, ce->y, ce->x + ce->grp->sizex - 1, ce->y - ce->grp->sizey + 1, mp->plxmemorycopy);
		Xfree(mp->plxmemorycopy);
		mp->plxmemorycopy = (char *)0;
	} else if (just_allocated) {
		/* clear pixmap to 0's */
		p_box(0, ce->x, ce->y, ce->x + pPixmap->width - 1, ce->y - pPixmap->height + 1);
	}
	p_damvx();
	return(TRUE);
}

/*
 * plxpixmapuse -- retrieve off screen coordinates and mark use of cached data
 */

Bool
plxpixmapuse(rwflag, pPixmap, x, y)
int rwflag;			/* Do we want to read or write */
register PixmapPtr pPixmap;
short *x,*y;			/* coords in cache. *x = -1 if not cached */
{
	register MapPriv *mp = (MapPriv *)pPixmap->devPrivate;
	register plxCache *ce;

	ifdebug(1) printf("plxpixmapuse() 0x%08x, rw=%d\n", pPixmap, rwflag);

	/*
	 * this check is more for sanity than for safety, it seems
	 * silly for someone to read back a pixmap that has not been
	 * written too.
	 */
	ce = mp->plxcache;
	if ((rwflag == PIXMAP_READ) && (!ce)) {
		ErrorF("plxpixmapuse() EMPTY PIXMAP\n");
	}

	if (!plxcachepixmap(pPixmap)) {
		*x = *y = -1;
		return (FALSE);
	}

	ce = mp->plxcache;
	*x = ce->x;
	*y = ce->y;

	if (rwflag == PIXMAP_WRITE) {
		/* reset the expanded tile/stipple */
		mp->plxtile.expand = FALSE;
	}
	pl_cache_update(ce);
	ifdebug(1) printf("\t x,y=%d,%d\n", ce->x, ce->y);

	return (TRUE);
}

plxrotatepixmap(pPixmap, x, y)
register PixmapPtr pPixmap;
{
	register MapPriv *mp;
	register plxCache *ce;
	int deltax, deltay;

	ifdebug(1) printf("plxrotatepixmap() 0x%08x, x,y=%d, %d\n", pPixmap, x, y);
	mp = (MapPriv *)pPixmap->devPrivate;

	ce = mp->plxcache;
	if (!ce) {
		ErrorF("plxrotatepixmap() EMPTY PATTERN\n");
		return;
	}

	deltax = (mp->plxtile.x - x) % pPixmap->width;
	deltay = (mp->plxtile.y - y) % pPixmap->height;
	ifdebug(1) printf("\tdelta x,y=%d,%d\n", deltax, deltay);

	if (deltax != 0) {
		/* copy to AREA1 */
		/* copy back to cache box */
	}
	if (deltay != 0) {
		/* copy to AREA1 */
		/* copy back to cache box */
	}

	mp->plxtile.x = x;
	mp->plxtile.y = y;
}

/*
 * We have a pixmap we want to use for a parallax stipple operation.
 */
Bool
plxpreparepattern(pPixmap, xret, yret)
register PixmapPtr pPixmap;		/* pattern we won't to stipple with */
register short *xret, *yret;	/* what coordinates to use for stipple */
{
	register MapPriv *mp;
	register plxCache *ce;
	register short tileWidth, tileHeight;

	ifdebug(1) printf("plxpreparepattern(), 0x%08x\n", pPixmap);

        if (!IS_VALID_PIXMAP(pPixmap))
		return (FALSE);

	tileWidth = pPixmap->width;
	tileHeight = pPixmap->height;

	mp = (MapPriv *)pPixmap->devPrivate;
	ce = mp->plxcache;
	if (!ce) {
		ErrorF("plxpreparepattern() EMPTY PATTERN\n");
	}

	if (!plxpixmapuse(PIXMAP_READ, pPixmap, xret, yret)) {
		return (FALSE);
	}
	ce = mp->plxcache;

	ifdebug(1) printf("\tx,y=%d,%d\n", *xret, *yret);

	/*
	 * duplicate stipple/tile thru whole of cache box
	 * the idea is to get at least a 16by16 area to tile/stipple
	 * with.
	 */
	if ((tileWidth >= 16) || (tileHeight >= 16)) {
		/* we end up doing it by hand */
		mp->plxtile.expand = TRUE;
	}

	if (!mp->plxtile.expand) {
		register int x, y;

		p_rmap(0);
		p_opaq(0);
		p_mask(0xffff);
		plxclipinvalidate ();

		if (mp->video)
			p_damvv();
		else
			p_damvg();

		/* build a 16x16 stipple pattern */
		p_clip(*xret, *yret, *xret + (ce->grp->sizex - 1), *yret - (ce->grp->sizey - 1));
		for (x=0;x<16;x += tileWidth) {
			for (y=0;y<16;y += tileHeight) {
				if ((x == 0) && (y == 0))
					continue;
				p_boxc(*xret, *yret, *xret + x, *yret - y, *xret + x + (tileWidth - 1), *yret - y - (tileHeight - 1));
			}
		}
		p_damvx();
		mp->plxtile.expand = TRUE;
		p_clipd();
	}

	*yret -= tileHeight - 1;
	return (TRUE);
}
