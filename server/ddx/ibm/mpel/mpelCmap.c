/***********************************************************
		Copyright IBM Corporation 1987,1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelCmap.c,v 6.2 88/10/24 23:55:05 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelCmap.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelCmap.c,v 6.2 88/10/24 23:55:05 kbg Exp $";
#endif

#include "X.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "cursor.h"
#include "resource.h"

#include "OScompiler.h"
#include "ibmTrace.h"

#include "ppc.h"

#include "ibmScreen.h"

#include "mpelHdwr.h"

typedef struct _mpelColorTable
	{
	unsigned short 	start;
	unsigned short 	count;
	unsigned int 	rgb[256];
	} mpelColorTable, *mpelColorTablePtr;

static struct { unsigned short r,g,b;} installed[256];

#if defined(AIXrt)	/* no comment */
static unsigned long tableptr = 0x1008400;
#else
static unsigned long tableptr = mpelAddr(MPEL_COLOR_TABLE);
#endif

/***==================================================================***/

/*ARGSUSED*/
void
mpelSetColor(number,red,green,blue,pVisual)
    register unsigned	number, red, green, blue;
    VisualPtr	pVisual;
{
    register mpelColorTable *pmap;

    TRACE(("mpelSetColor(%d,%d,%d,%d,0x%x)\n",number,red,green,blue,pVisual));

    installed[number].r = red;
    installed[number].g = green;
    installed[number].b = blue;

    if ( ibmScreenState( pVisual->screen ) == SCREEN_ACTIVE ) {
	pmap = (mpelColorTable *) (MPEL_COLOR_TABLE);
	pmap->start = number;
	pmap->count = 1;	/* was 0 */
	pmap->rgb[0] =	((red & 0xff00) << 8)	|
			(green & 0xff00)	|
			((blue & 0xff00) >> 8);
	mpelicmd(MPELCMD_LOAD_CMAP, &tableptr, 2);
    }
    return ;
}

/***==================================================================***/

/* MegaPel Colormap Installation
   Does not use PPC because SetColor is so slow.
   For an entire map, should do it this way.  Duplicates PPC function,
   but better keep up to date with PPC if InstallColormap responsibilities
   change   - paquin 10/12/87
 */

void
mpelInstallColormap(pColormap)
    register ColormapPtr 	pColormap;
{
    register int		i;
    register mpelColorTablePtr	pmap;
    register int		red, green, blue;
    ScreenPtr 			pScreen = pColormap->pScreen;
    ppcScrnPrivPtr		devPriv;
    int				updateHdwr;

    TRACE(("mpelInstallColormap(pColormap=0x%x)\n",pColormap));

    devPriv = (ppcScrnPriv *) pScreen->devPrivate;
    updateHdwr= (ibmScreenState(pScreen->myNum)==SCREEN_ACTIVE);

    if (updateHdwr) {
	pmap = (mpelColorTable *) (MPEL_COLOR_TABLE);
	pmap->start = 0;
	pmap->count = pColormap->pVisual->ColormapEntries;
    }

    for(i=0; i<(pColormap->pVisual->ColormapEntries); i++) {
	installed[i].r = red   = pColormap->red[i].co.local.red;
	installed[i].g = green = pColormap->red[i].co.local.green;
	installed[i].b = blue  = pColormap->red[i].co.local.blue;

	if (updateHdwr) {
	    pmap->rgb[i] =
			(((red   & 0xff00) << 8) |
			((green & 0xff00)     ) |
			((blue  & 0xff00) >> 8) );
	}
    }
    if (updateHdwr)
	mpelicmd(MPELCMD_LOAD_CMAP, &tableptr, 2);

    devPriv->InstalledColormap = pColormap;

    WalkTree(pScreen,TellGainedMap,&(pColormap->mid));

    /* RecolorCursor---
     *
     * Added 9/28/87  T Paquin
     *
     * I know there is a RecolorCursor in the Screen structure, but
     * It requires a pCurs pointer, which we do not have here.  Therefore,
     * a ppcScreenPrivate function will be called with a pScreen parameter.
     * At worst, the private function will return.  At second worst,
     * it will have remembered the pCurs, and will call miRecolorCursor.
     * At best, it will have remembered the cursor's FG and BG rgb values
     * (which it has if it has saved the pCurs pointer)
     * and will cause the cursor's colors to match the best available in
     * the colormap we just finished installing.
     * miRecolorCursor is a dog and calls code you had already in
     * DisplayCursor (as well as Realize/Unrealize) anyway.
     */

   (*(devPriv->RecolorCursor))(pColormap);
    return ;
}

/***==================================================================***/

void
mpelUninstallColormap(pColormap)
    register ColormapPtr	pColormap;
{
    register ColormapPtr	pCmap;

    TRACE(("mpelUninstallColormap(pColormap=0x%x)\n",pColormap));

    pCmap = (ColormapPtr) LookupID(pColormap->pScreen->defColormap,
		RT_COLORMAP,RC_CORE);

    if (pCmap==pColormap)
	return; /* never uninstall the default map */

    WalkTree(pColormap->pScreen,TellLostMap,&(pColormap->mid));
    mpelInstallColormap(pCmap);
    /* Install will WalkTree and tell people about the new map */
    return ;
}


/***==================================================================***/

void
mpelFindColor(color,r,g,b)
    register unsigned long int *color;
    register unsigned short int r,g,b;
{
    register int	i ;
    unsigned long int bestdiff, best, diff;
    register int	dr, dg, db;

    TRACE(("mpelFindColor(0x%x,%d,%d,%d)\n",color,r,g,b));

    best = 0;
    bestdiff = 0xffffffff; /* MAXINT for unsigned */

    for ( i = 256; i--; ) {
	dr = installed[i].r - r;
	dg = installed[i].g - g;
	db = installed[i].b - b;
	diff =  ABS(dr) + ABS(dg) + ABS(db);
	if (diff < bestdiff) {
	    bestdiff = diff;
	    best = i;
	}
    }
    *color = best;
    return ;
}
