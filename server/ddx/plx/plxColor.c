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
static char *sid_ = "@(#)plxColor.c	1.10 7/31/88 Parallax Graphics Inc";
#endif

/*
 *	Parallax Color Routines.
 *
 *	plxCreateColormap()
 *	plxInstallColormap()
 *	plxUninstallColormap()
 *	plxListInstalledColormaps()
 *	plxStoreColors()
 *	plxResolveColor()
 */

#include	"Xplx.h"

#include	"Xproto.h"
#include	"colormapst.h"

#ifdef notdef
#include	"colortbl.inc"
#endif

extern int TellLostMap(), TellGainedMap();

/*
 * the colormap install/uninstall code uses this infomation to
 * get the correct colormap installed. this infomation should
 * be in the screen devprivate area.
 */
ColormapPtr plxcurrentmap[MAXSCREENS];

typedef struct _colorpriv {
	Bool	inuse[256];
} ColorPriv;

void
plxCreateColormap(pColormap)
ColormapPtr pColormap;
{
	register int i;
	register int entries = pColormap->pVisual->ColormapEntries;
	register ColorPriv *cp;

	ifdebug(6) printf("plxCreateColormap(), class=%d, entries=%d\n",
				pColormap->class, entries);

	switch (pColormap->class) {
	case GrayScale:
	case StaticGray:
		for (i=0;i<entries;i++) {
			pColormap->red[i].co.local.red =
			pColormap->red[i].co.local.green =
			pColormap->red[i].co.local.blue =
				((i + entries/2) << 8)/entries;
		}
		break;
#ifdef notdef
	case StaticColor:
		if (entries != 256)
			break;
		for (i=0;i<entries;i++) {
			pColormap->red[i].co.local.red = real_red[i] << 8;
			pColormap->red[i].co.local.green = real_green[i] << 8;
			pColormap->red[i].co.local.blue = real_blue[i] << 8;

			ifdebug(6) printf("\t%d r,g,b=0x%04x,0x%04x,0x%04x\n",
				i,
				pColormap->red[i].co.local.red,
				pColormap->red[i].co.local.green,
				pColormap->red[i].co.local.blue);
		}
		break;
#endif
	case PseudoColor:
		break;
	case TrueColor:
	case DirectColor:
	default:
		ErrorF("plxCreateColormap: unsupported class, class=%d\n", pColormap->class);
		break;
	}

	cp = (ColorPriv *)Xalloc(sizeof(ColorPriv));
	for (i=0;i<entries;i++) {
		cp->inuse[i] = FALSE;
	}
	pColormap->devPriv = (pointer)cp;
}

void
plxDestroyColormap(pColormap)
register ColormapPtr pColormap;
{
	register ColorPriv *cp;

	ifdebug(6) printf("plxDestroyColormap() 0x%08x, class=%d\n", pColormap, pColormap->class);

	cp = (ColorPriv *)pColormap->devPriv;
	Xfree(cp);
}

void
plxInstallColormap(pColormap)
register ColormapPtr pColormap;
{
	register int i;
	register int idx = pColormap->pScreen->myNum;
	register Entry *pEntry;
	register ColorPriv *cp = (ColorPriv *)pColormap->devPriv;

	ifdebug(6) printf("plxInstallColormap() 0x%08x, class=%d\n", pColormap, pColormap->class);

	if (pColormap == plxcurrentmap[idx]) {
		return;
	}

	/*
	 * Uninstall old color map.
	 */
	if (plxcurrentmap[idx]) {
		WalkTree(pColormap->pScreen, TellLostMap, (char *)&(plxcurrentmap[idx]->mid));
	}

	/*
	 * Update hardware
	 */
	switch (pColormap->class) {
	case GrayScale:
	case StaticGray:
	case StaticColor:
	case PseudoColor:
		for (i=0,pEntry=pColormap->red;i<pColormap->pVisual->ColormapEntries;i++,pEntry++) {
			if (cp->inuse[i] == FALSE)
				continue;
			if (pEntry->fShared) {
				p_clt8(i,
					(pEntry->co.shco.red->color >> 8),
					(pEntry->co.shco.green->color >> 8),
					(pEntry->co.shco.blue->color >> 8));
				ifdebug(6) printf("\tshared entry %d set r,g,b=%d,%d,%d\n",
					i,
					(pEntry->co.shco.red->color >> 8),
					(pEntry->co.shco.green->color >> 8),
					(pEntry->co.shco.blue->color >> 8));
			} else {
				p_clt8(i,
					(pEntry->co.local.red >> 8),
					(pEntry->co.local.green >> 8),
					(pEntry->co.local.blue >> 8));
				ifdebug(6) printf("\tentry %d set r,g,b=%d,%d,%d\n",
					i,
					(pEntry->co.local.red >> 8),
					(pEntry->co.local.green >> 8),
					(pEntry->co.local.blue >> 8));
			}
		}
		break;
	case TrueColor:
	case DirectColor:
	default:
		ErrorF("plxInstallColormap: unsupported class, class=%d\n", pColormap->class);
		break;
	}

	plxcurrentmap[idx] = pColormap;
	/*
	 * Install new color map.
	 */
	WalkTree(pColormap->pScreen, TellGainedMap, (char *)&(plxcurrentmap[idx]->mid));
}

void
plxUninstallColormap(pColormap)
ColormapPtr pColormap;
{
	int idx = pColormap->pScreen->myNum;
	ColormapPtr curpmap = plxcurrentmap[idx];

	ifdebug(6) printf("plxUninstallColormap() 0x%08x, class=%d\n", pColormap, pColormap->class);

	if (pColormap == plxcurrentmap[idx]) {
		/*
 		 * Uninstall colormap
		 */
		WalkTree(pColormap->pScreen, TellLostMap, (char *)&(pColormap->mid));
		/*
		 * Install default colormap
		 */
		plxcurrentmap[idx] = (ColormapPtr) LookupID(pColormap->pScreen->defColormap, RT_COLORMAP, RC_CORE);
		WalkTree(pColormap->pScreen, TellGainedMap, (char *)&(plxcurrentmap[idx]->mid));
	}
}

plxListInstalledColormaps(pScreen, pColormapList)
ScreenPtr pScreen;
Colormap *pColormapList;
{
	register int idx = pScreen->myNum;

	ifdebug(6) printf("plxListInstalledColormaps()\n");

	*pColormapList = plxcurrentmap[idx]->mid;
	return (1);
}

void
plxStoreColors(pColormap, ndefs, pdefs)
ColormapPtr pColormap;
int ndefs;
xColorItem *pdefs;
{
	register int idx = pColormap->pScreen->myNum;
	register ColorPriv *cp;

	ifdebug(6) printf("plxStoreColors() 0x%08x, ndefs=%d\n", pColormap, ndefs);

	cp = (ColorPriv *)pColormap->devPriv;

	switch (pColormap->class) {
	case PseudoColor:
		while (ndefs--) {
			if (pColormap == plxcurrentmap[idx]) {
				/*
				 * only need to update tables if on the screen
				 */
				p_clt8(pdefs->pixel,
					(pdefs->red >> 8),
					(pdefs->green >> 8),
					(pdefs->blue >> 8));
				ifdebug(6) printf("\tentry %d set r,g,b=%d,%d,%d\n",
					pdefs->pixel,
					pdefs->red>>8,
					pdefs->green>>8,
					pdefs->blue>>8);
			}
			cp->inuse[pdefs->pixel] = TRUE;
			pdefs++;
		}
		break;
	case StaticGray:
	case StaticColor:
	case GrayScale:
	case TrueColor:
	case DirectColor:
	default:
		ErrorF("plxStoreColor: unsupported class, class=%d\n", pColormap->class);
		break;
	}
}

void
plxResolveColor(pR, pG, pB, pVisual)
CARD16 *pR, *pG, *pB;
VisualPtr pVisual;
{
	ifdebug(6) printf("plxResolveColor(), IN, r,g,b=0x%04x,0x%04x,0x%04x\n", *pR, *pG, *pB);

	switch (pVisual->class) {
	case StaticGray:
		*pR = *pG = *pB = *pR;
		break;
#ifdef notdef
	case StaticColor:
		{
			register int e1, e2;

			e1 = red_inverse[(*pR>>8) & 0xff]
				+ green_inverse[(*pG>>8) & 0xff]
				+ blue_inverse[(*pB>>8) & 0xff];
			e2 = color_munge[e1];

			*pR = real_red[e2] << 8;
			*pG = real_green[e2] << 8;
			*pB = real_blue[e2] << 8;
		}
		break;
#endif
	case PseudoColor:
		switch (pVisual->ColormapEntries) {
		case 2:
			*pR &= 0x8000;
			*pG &= 0x8000;
			*pB &= 0x8000;
			break;
		case 192:
		case 256:
			*pR &= 0xff00;
			*pG &= 0xff00;
			*pB &= 0xff00;
			break;
		default:
			ErrorF("plxResolveColor: unsupported PseudoColor depth, entries=%d\n", pVisual->ColormapEntries);
			break;
		}
		break;
	case GrayScale:
	case TrueColor:
	case DirectColor:
	default:
		ErrorF("plxResolveColor: unsupported class, class=%d\n", pVisual->class);
		break;
	}
	ifdebug(6) printf("plxResolveColor(), OUT, r,g,b=0x%04x,0x%04x,0x%04x\n", *pR, *pG, *pB);
}
