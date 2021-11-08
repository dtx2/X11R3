/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/


#include "X.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "resource.h"

#ifdef	STATIC_COLOR

static ColormapPtr InstalledMaps[MAXSCREENS];

int
cfbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    /* By the time we are processing requests, we can guarantee that there
     * is always a colormap installed */
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return (1);
}
void
cfbInstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr oldpmap = InstalledMaps[index];

    if(pmap != oldpmap)
    {
	/* Uninstall pInstalledMap. No hardware changes required, just
	 * notify all interested parties. */
	if(oldpmap != (ColormapPtr)None)
	    WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
	/* Install pmap */
	InstalledMaps[index] = pmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

    }
}

void
cfbUninstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr curpmap = InstalledMaps[index];

    if(pmap == curpmap)
    {
        /* Uninstall pmap */
	WalkTree(pmap->pScreen, TellLostMap, (char *)&pmap->mid);
	curpmap = (ColormapPtr) LookupID(pmap->pScreen->defColormap,
					 RT_COLORMAP, RC_CORE);
	/* Install default map */
	InstalledMaps[index] = curpmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&curpmap->mid);
    }
}

void
cfbResolveStaticColor(pred, pgreen, pblue, pVisual)
    unsigned short	*pred, *pgreen, *pblue;
    VisualPtr		pVisual;
{
    *pred &= 0xe000;
    *pgreen &= 0xe000;
    *pblue &= 0xc000;
}

#endif

void
cfbInitializeColormap(pmap)
    ColormapPtr	pmap;
{
    int	i;

    if (pmap->pVisual->ColormapEntries == 256) /* 332 rgb map */
      for(i = 0; i < 256; i++)
	{
	  pmap->red[i].co.local.red = (i & 0x7) << 13;
	  pmap->red[i].co.local.green = (i & 0x38) << 10;
	  pmap->red[i].co.local.blue = (i & 0xc0) << 8;
	}
    else
      if (pmap->pVisual->ColormapEntries == 64) /* 222 rgb map */
	for(i = 0; i < 64; i++)
	  {
	    pmap->red[i].co.local.red = (i & 0x3) << 14;
	    pmap->red[i].co.local.green = (i & 0xc) << 12;
	    pmap->red[i].co.local.blue = (i & 0x30) << 10;
	  }
    else
      if (pmap->pVisual->ColormapEntries == 16) /* 121 rgb map */
	for(i = 0; i < 16; i++)
	  {
	    pmap->red[i].co.local.red = (i & 0x1) << 15;
	    pmap->red[i].co.local.green = (i & 0x6) << 13;
	    pmap->red[i].co.local.blue = (i & 0x8) << 12;
	  }
    else
      if (pmap->pVisual->ColormapEntries == 8) /* 111 rgb map */
	for(i = 0; i < 8; i++)
	  {
	    pmap->red[i].co.local.red = (i & 0x1) << 15;
	    pmap->red[i].co.local.green = (i & 0x2) << 14;
	    pmap->red[i].co.local.blue = (i & 0x4) << 13;
	  }
    else
      if (pmap->pVisual->ColormapEntries == 2) /* b&w */
	for (i=0; i<=1; i++)
	  {
#ifdef hpux
	    /*
	     * this is to set the red, green, blue fields to 0xff00
	     * instead of 0xff
	     */
	    pmap->red[i].co.local.red = i * 0xff00;
	    pmap->red[i].co.local.green = i * 0xff00;
	    pmap->red[i].co.local.blue = i * 0xff00;
#else
	    pmap->red[i].co.local.red = i * 255;
	    pmap->red[i].co.local.green = i * 255;
	    pmap->red[i].co.local.blue = i * 255;
#endif
          }
    else
      {
        ErrorF("cfbInitializeColormap: Unsupported colormap depth %d\n",
	       pmap->pVisual->ColormapEntries);
      }
    
}
