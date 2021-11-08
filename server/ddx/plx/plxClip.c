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
static char *sid_ = "@(#)plxClip.c	1.13 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

#include "gcstruct.h"

void
plxDestroyClip(pGC)
GCPtr pGC;
{
	if(pGC->clientClipType == CT_NONE)
		return;
	(*pGC->pScreen->RegionDestroy)(pGC->clientClip);
	pGC->clientClip = NULL;
	pGC->clientClipType = CT_NONE;
	pGC->stateChanges |= (GCClipXOrigin | GCClipYOrigin | GCClipMask);
}

void
plxChangeClip(pGC, type, pvalue, nrects)
GCPtr pGC;
int type;
pointer pvalue;
int nrects;
{
	plxDestroyClip(pGC);
	if(type == CT_PIXMAP) {
		pGC->clientClip = (pointer)mfbPixmapToRegion(pvalue);
		(*pGC->pScreen->DestroyPixmap)(pvalue);
	} else if (type == CT_REGION) {
		pGC->clientClip = (pointer)(*pGC->pScreen->RegionCreate)( NULL, 0 );
		(*pGC->pScreen->RegionCopy)( pGC->clientClip, pvalue );
	} else if (type != CT_NONE) {
		pGC->clientClip = (pointer)miRectsToRegion(pGC, nrects, pvalue, type);
		Xfree(pvalue);
	}
	pGC->clientClipType = (pGC->clientClip) ? CT_REGION : CT_NONE;
	pGC->stateChanges |= (GCClipXOrigin | GCClipYOrigin | GCClipMask);
}

void
plxCopyClip(pgcDst, pgcSrc)
GCPtr pgcDst, pgcSrc;
{
	RegionPtr prgnNew;

	switch(pgcSrc->clientClipType) {
	case CT_NONE:
	case CT_PIXMAP:
		plxChangeClip(pgcDst, pgcSrc->clientClipType, pgcSrc->clientClip, 0);
		break;
	case CT_REGION:
		prgnNew = (*pgcSrc->pScreen->RegionCreate)(NULL, 1);
		(*pgcSrc->pScreen->RegionCopy)(prgnNew,
		(RegionPtr)(pgcSrc->clientClip));
		plxChangeClip(pgcDst, CT_REGION, prgnNew, 0);
		break;
	}
}

/*
 * this infomation should be kept with the screen private area
 */
short *plxcliplist = (short *)0;
int plxcliplistsize = 0;			/* # of clip regions */

RegionPtr plxclipcurrentregion;
int plxclipcurrentx, plxclipcurrenty;

plxclipinit(pScreen)
ScreenPtr pScreen;
{
	extern void	plxVideoReshape ();

	ifdebug(17) printf("plxclipinit() 0x%08x\n", pScreen);

	plxclipcurrentregion = (RegionPtr)0;
	plxclipcurrentx = 0;
	plxclipcurrenty = 0;
	miClipNotify (plxVideoReshape);
}

/*
 * plxclipdownload()
 *	returns 0 if no clip area is valid, ie: dont draw anything.
 *	returns non 0 if clip is setup and ready to draw.
 */
plxclipdownload(pGC, xorg, yorg)
GCPtr pGC;
short xorg, yorg;
{
	return plxclipdownloadregion(((plxPrivGC *)pGC->devPriv)->pCompositeClip, xorg, yorg);
}

plxclipdownloadregion(pRegion, xorg, yorg)
register RegionPtr pRegion;
register short xorg, yorg;
{
	register BoxPtr pbox, pboxLast;
	register short *p;

	ifdebug(7) printf("plxclipdownloadregion() 0x%08x n=%d, xorg,yorg=%d,%d\n", pRegion, pRegion->numRects, xorg, yorg);

	if (pRegion->numRects == 0)
		return (0);

	if ((pRegion == plxclipcurrentregion) && (xorg == plxclipcurrentx) && (yorg == plxclipcurrenty)) {
		ifdebug(7) printf("\tclip ok\n");
		p_clipe();
		return (1);
	}

	plxclipcurrentregion = pRegion;
	plxclipcurrentx = xorg;
	plxclipcurrenty = yorg;
	ifdebug(7) printf("\tclip reload\n");

	/*p_tranc(xorg, yorg);*/
	if (pRegion->numRects == 1) {
		pbox = pRegion->rects;
		ifdebug(7) printf("\t%d,%d,%d,%d\n", pbox->x1, pbox->y1, pbox->x2 - 1, pbox->y2 - 1);
		ifdebug(7) p_boxo(0x30,xorg+pbox->x1, PTY(yorg+pbox->y1), xorg+pbox->x2-1, PTY(yorg+pbox->y2-1));
		p_clip(xorg+pbox->x1, PTY(yorg+pbox->y1), xorg+pbox->x2-1, PTY(yorg+pbox->y2-1));
	} else {
		if (plxcliplistsize < pRegion->numRects) {
			plxcliplistsize = pRegion->numRects;
			plxcliplist = (short *)Xrealloc(plxcliplist, plxcliplistsize*sizeof(short)*4);
		}
		pbox = pRegion->rects;
		pboxLast = pbox + pRegion->numRects;
		p = plxcliplist;
		for (;pbox<pboxLast;pbox++) {
			ifdebug(7) printf("\t%d,%d,%d,%d\n", pbox->x1, pbox->y1, pbox->x2-1, pbox->y2-1);
			*p++ = xorg+pbox->x1;
			*p++ = PTY(yorg+pbox->y1);
			*p++ = xorg+pbox->x2-1;
			*p++ = PTY(yorg+pbox->y2-1);
			ifdebug(7) p_boxo(0x30,xorg+pbox->x1, PTY(yorg+pbox->y1), xorg+pbox->x2-1, PTY(yorg+pbox->y2-1));
		}
		p_clipm(pRegion->numRects, plxcliplist);
	}
	return (1);
}

plxclipinvalidate()
{
	ifdebug(7) printf("plxclipinvalidate()\n");

	plxclipcurrentregion = (RegionPtr)0;
	p_clipd();
}
