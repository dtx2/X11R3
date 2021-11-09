/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 2021 Edward Halferty

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
/* $XConsortium: ndx_io.c,v 1.6 88/10/06 14:01:07 rws Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <errno.h>

#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "pixmap.h"
#include "pixmapstr.h"
#include "input.h"
#include "windowstr.h"
#include "regionstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "mi.h"
#include "servermd.h"		/* For PixmapBytePad */

#define	MFB_SCREEN	0
#define	CFB_SCREEN	1

extern void	cfbInstallColormap();
extern void	cfbUninstallColormap();
extern int	cfbListInstalledColormaps();
extern void	cfbResolveStaticColor();
extern void	mfbQueryBestSize();
extern void	cfbInitialize332Colormap();

int  ndxGetMotionEvents();
void ndxChangePointerControl(), ndxChangeKeyboardControl(), ndxBell();

extern int errno;
static int qLimit;
static int lastEventTime;
static DevicePtr ndxKeyboard;
static DevicePtr ndxPointer;

int	bitsPerPixel[] = { 1, 8 };	/* depends on {MFB,CFB}_SCREEN */
int bitsPerPad[] = { 32, 32 };
int log2ofBitsPerPad[] = { 5, 5 };
int depth[] = { 1, 8 };

static Bool TrueDDA() {
    return(TRUE);
}
static Bool ndxSaveScreen(ScreenPtr pScreen, int on) {
    if (on == SCREEN_SAVER_FORCER) {
        lastEventTime = GetTimeInMillis();
	    return TRUE; // pretend it's saved
    } else { return FALSE; } // ok, so it's not saved
}
static void ndCfbQueryBestSize(int class, short *pwidth, short *pheight) {
    unsigned width, test;
    switch(class) { // LOL what the hell is the point of this switch?
      case CursorShape:
      case TileShape:
      case StippleShape:
          width = *pwidth;
          if (width != 0) {
              // Return the closest power of two not less than width
              test = 0x80000000;
              // Find the highest 1 bit in the width given
              while(!(test & width)) { test >>= 1; }
              //If their number is greater than that, bump up to the next power of two
              if((test - 1) & width) { test <<= 1; }
              *pwidth = test;
          }
          // We don't care what height they use
          break;
    }
}
void ndMfbResolveColor(CARD16 *pRed, CARD16 *pGreen, CARD16 *pBlue, VisualPtr pVisual) {
    *pRed = *pGreen = *pBlue =
	(((39L * *pRed + 50L * *pGreen +
	11L * *pBlue) >> 8) >= (((1<<8)-1)*50)) ? ~0 : 0;
}
// ndx cursor top-left corner cannot go to negative coordinates. return value in pPhysBox
static void ndxCursorLimits(ScreenPtr pScr, CursorPtr pCurs, BoxPtr pHotBox, BoxPtr pPhysBox) {
    pPhysBox->x1 = max( pHotBox->x1, pCurs->xhot);
    pPhysBox->y1 = max( pHotBox->y1, pCurs->yhot);
    pPhysBox->x2 = min( pHotBox->x2, 1024);
    pPhysBox->y2 = min( pHotBox->y2, 864);
}
void ndMfbCreateColormap(ColormapPtr pmap) {
    int	red, green, blue, pix;
    // this is a monochrome colormap, it only has two entries, just fill them in by hand.
    // If it were a more complex static map, it would be worth writing a for loop or three to initialize it
    pix = 0;
    red = green = blue = 0;
    AllocColor(pmap, &red, &green, &blue, &pix, 0);
    pix = 0;
    red = green = blue = ~0;
    AllocColor(pmap, &red, &green, &blue, &pix, 0);
}
void ndxDestroyColormap(ColormapPtr	pmap) {}
static Bool ndxScreenClose(int index, ScreenPtr pScreen) {
    if (index == MFB_SCREEN)
	mfbScreenClose(pScreen);
    // TODO: sloppy:  no close for cfb
    return (TRUE);
}
// pad a number of bits to N bits and return the number of padding units required
#define PADBITS(nbits,bitsInPad,log2pad) \
  (((nbits) + (bitsInPad) - 1)>>(log2pad))
Bool ndxScreenInit(int index, ScreenPtr pScreen) {
    int retval, i;
    char *blackValue, *whiteValue, *bitmap;
    ColormapPtr	pColormap;
    bitmap = (char *)xalloc( PixmapBytePad(1024, depth[index]) * 864 );
    if (index == MFB_SCREEN) { retval = mfbScreenInit(index, pScreen, bitmap, 1024, 864, 80, 80); }
    else { retval = cfbScreenInit(index, pScreen, bitmap, 1024, 864, 80, 80); }	/* index assumed to be CFB_SCREEN */
    pScreen->CloseScreen =		ndxScreenClose;
    if (index == MFB_SCREEN)
	pScreen->QueryBestSize =	mfbQueryBestSize;
    else	/* CFB_SCREEN */
	pScreen->QueryBestSize =	ndCfbQueryBestSize;
    pScreen->SaveScreen =		ndxSaveScreen;
    /* GetImage */
    /* GetSpans */
    pScreen->PointerNonInterestBox =	NoopDDA;
    /* Window Routines... */
    /* Pixmap Routines... */
    /* Font Routines... */
    pScreen->ConstrainCursor =		NoopDDA;
    pScreen->CursorLimits =		ndxCursorLimits;
    pScreen->DisplayCursor =		TrueDDA;
    pScreen->RealizeCursor =		TrueDDA;
    pScreen->UnrealizeCursor =		TrueDDA;
    pScreen->RecolorCursor =		NoopDDA;
    pScreen->SetCursorPosition =	TrueDDA;
    /* CreateGC */
    if (index == MFB_SCREEN) {
      pScreen->CreateColormap =		ndMfbCreateColormap;
      pScreen->DestroyColormap =	NoopDDA;
      /* InstallColormap */
      /* UninstallColormap */
      /* ListInstalledColormaps */
      /* StoreColors */
      pScreen->ResolveColor =		ndMfbResolveColor;
    } else {	/* CFB_SCREEN */
      pScreen->CreateColormap =		cfbInitialize332Colormap;
      /* DestroyColormap */
      pScreen->InstallColormap =	cfbInstallColormap;
      pScreen->UninstallColormap =	cfbUninstallColormap;
      pScreen->ListInstalledColormaps =	cfbListInstalledColormaps;
      pScreen->StoreColors =		NoopDDA;
      pScreen->ResolveColor =		cfbResolveStaticColor;
    }
    // Region Routines...
    // Block/Wakeup Routines and Data...
    CreateColormap(pScreen->defColormap, pScreen, LookupID(pScreen->rootVisual, RT_VISUALID, RC_CORE), &pColormap, AllocNone, 0);
    if (index == MFB_SCREEN) {
    	pScreen->blackPixel = 0;
    	pScreen->whitePixel = 1;
    	mfbInstallColormap(pColormap);
    } else { // CFB_SCREEN
        pScreen->blackPixel = 0;
        pScreen->whitePixel = ~0;
        cfbInstallColormap(pColormap);
    }
    return(retval);
}
int ndxMouseProc(DevicePtr pDev, int onoff) {
    BYTE map[4];
    switch (onoff) {
        case DEVICE_INIT:
            break;
        case DEVICE_ON:
            pDev->on = TRUE;
            break;
        case DEVICE_OFF:
            pDev->on = FALSE;
            break;
        case DEVICE_CLOSE:
            break;
    }
    return Success;
}
int ndxKeybdProc(DevicePtr pDev, int onoff) {
    KeySymsRec keySyms;
    CARD8 modMap[MAP_LENGTH];
    int	i;
    switch (onoff) {
        case DEVICE_INIT:
            ndxKeyboard = pDev;
            keySyms.minKeyCode = 0;
            keySyms.maxKeyCode = 0;
            keySyms.mapWidth = 0;
            for (i=0; i<MAP_LENGTH; i++)
            modMap[i] = NoSymbol;
            InitKeyboardDeviceStruct(
                ndxKeyboard, &keySyms, modMap, ndxBell,
                ndxChangeKeyboardControl);
            break;
        case DEVICE_ON:
            pDev->on = TRUE;
            break;
        case DEVICE_OFF:
            pDev->on = FALSE;
            break;
        case DEVICE_CLOSE:
            break;
    }
    return Success;
}

void
ProcessInputEvents()
{
    xEvent	x;
    int     nowInCentiSecs, nowInMilliSecs, adjustCentiSecs;
    struct timeval  tp;
    int     needTime = 1;
}
TimeSinceLastInputEvent() {
    if (lastEventTime == 0)
	lastEventTime = GetTimeInMillis();
    return GetTimeInMillis() - lastEventTime;
}
// DDX - specific abort routine.  Called by AbortServer().
void AbortDDX() {}
// Called by GiveUp().
void ddxGiveUp() {}
void ddxUseMsg() {}
static void ndxBell(int loud, DevicePtr pDevice) {} // sorry, "null-devices" don't ding either.
static void ndxChangeKeyboardControl(DevicePtr pDevice, KeybdCtrl *ctrl) {}
static void ndxChangePointerControl(DevicePtr pDevice, PtrCtrl *ctrl) {}
static int ndxGetMotionEvents(xTimecoord *buff, CARD32 start, CARD32 stop) { return 0; }
Bool LegalModifier(unsigned char key) { return( FALSE ); }
static ColormapPtr InstalledMaps[MAXSCREENS];
int cfbListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps) {
    // By the time we are processing requests, we can guarantee that there is always a colormap installed
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return (1);
}
void cfbInstallColormap(ColormapPtr pmap) {
    int index = pmap->pScreen->myNum;
    ColormapPtr oldpmap = InstalledMaps[index];
    if(pmap != oldpmap) {
        // Uninstall pInstalledMap. No hardware changes required, just notify all interested parties.
        if(oldpmap != (ColormapPtr)None) { WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid); }
        // Install pmap
        InstalledMaps[index] = pmap;
        WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);
    }
}
void cfbUninstallColormap(ColormapPtr pmap) {
    int index = pmap->pScreen->myNum;
    ColormapPtr curpmap = InstalledMaps[index];
    if(pmap == curpmap) {
        // Uninstall pmap
        WalkTree(pmap->pScreen, TellLostMap, (char *)&pmap->mid);
        curpmap = (ColormapPtr) LookupID(pmap->pScreen->defColormap, RT_COLORMAP, RC_CORE);
        // Install default map
        InstalledMaps[index] = curpmap;
        WalkTree(pmap->pScreen, TellGainedMap, (char *)&curpmap->mid);
    }
}
void cfbResolveStaticColor(unsigned short *pred, unsigned short *pgreen, unsigned short *pblue, VisualPtr pVisual) {
    *pred &= 0xe000;
    *pgreen &= 0xe000;
    *pblue &= 0xc000;
}
void cfbInitialize332Colormap(ColormapPtr pmap) {
    int	i;
    for (i = 0; i < pmap->pVisual->ColormapEntries; i++) {
        // assume 256 for now
        pmap->red[i].co.local.red = (i & 0x7) << 13;
        pmap->red[i].co.local.green = (i & 0x38) << 10;
        pmap->red[i].co.local.blue = (i & 0xc0) << 8;
    }
}
