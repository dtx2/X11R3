/*-
 * sunCG3C.c --
 *	Functions to support the sun CG3 board as a memory frame buffer.
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

#include    "colormap.h"
#include    "colormapst.h"
#include    "resource.h"
#include    <struct.h>

/* XXX - next line means only one CG3 - fix this */
static ColormapPtr ndCfbInstalledMap;

extern int TellLostMap(), TellGainedMap();

/*-
 *-----------------------------------------------------------------------
 * ndCfbInit --	install cfb screen
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static Bool
ndCfbInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    if (sunFbs[index].info.fb_width == CG3A_WIDTH) {
        if (!cfbScreenInit (index, pScreen, CG3ACfb->cpixel,	
		    sunFbs[index].info.fb_width,
		    sunFbs[index].info.fb_height, 90))
	    return (FALSE);
    }
    else {
        if (!cfbScreenInit (index, pScreen, CG3BCfb->cpixel,	
		    sunFbs[index].info.fb_width,
		    sunFbs[index].info.fb_height, 90))
	    return (FALSE);
    }

    pScreen->SaveScreen =   	    	ndCfbSaveScreen;

    {
	ColormapPtr cmap = (ColormapPtr)LookupID(pScreen->defColormap, RT_COLORMAP, RC_CORE);

	if (!cmap)
	    FatalError("Can't find default colormap\n");
	if (AllocColor(cmap, &ones, &ones, &ones, &(pScreen->whitePixel), 0)
	    || AllocColor(cmap, &zero, &zero, &zero, &(pScreen->blackPixel), 0))
		FatalError("Can't alloc black & white pixels in cfbScreeninit\n");
	ndCfbInstallColormap(cmap);
    }


    ndCfbSaveScreen( pScreen, SCREEN_SAVER_FORCER );
    sunScreenInit (pScreen);
    return (TRUE);

#else	/* non 386 */
    return (FALSE);
#endif
}


/*-
 *-----------------------------------------------------------------------
 * ndCfbProbe --
 *	Attempt to find and initialize a cg3 framebuffer
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped.
 *
 *-----------------------------------------------------------------------
 */
Bool ndCfbProbe(ScreenInfo *pScreenInfo, int index, int fbNum) {
    int i, oldNumScreens, fd;
    if (sunFbData[fbNum].probeStatus == probedAndFailed) { return FALSE; }
    if (sunFbData[fbNum].probeStatus == neverProbed) {
        struct fbtype fbType;
        if ((fd = sunOpenFrameBuffer(FBTYPE_SUN3COLOR, &fbType, index, fbNum)) < 0) {
            sunFbData[fbNum].probeStatus = probedAndFailed;
            return FALSE;
        }
        if (fbType.fb_width == CG3A_WIDTH) {
            CG3ACfb = (CG3ACPtr) valloc(CG3AC_MONOLEN +
            CG3AC_ENBLEN + CG3AC_IMAGELEN);
            if (CG3ACfb == (CG3ACPtr) NULL) {
                sunFbData[fbNum].probeStatus = probedAndFailed;
                return FALSE;
            }
            if (mmap((caddr_t) CG3ACfb, CG3AC_MONOLEN + CG3AC_ENBLEN + CG3AC_IMAGELEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) < 0) {
                sunFbData[fbNum].probeStatus = probedAndFailed;
                (void) close(fd);
                return FALSE;
            }
        } else if (fbType.fb_width == CG3B_WIDTH) {
            CG3BCfb = (CG3BCPtr) valloc(CG3BC_MONOLEN +
            CG3BC_ENBLEN + CG3BC_IMAGELEN);
            if (CG3BCfb == (CG3BCPtr) NULL) {
                sunFbData[fbNum].probeStatus = probedAndFailed;
                return FALSE;
            }
            if (mmap((caddr_t) CG3BCfb, CG3BC_MONOLEN + CG3BC_ENBLEN + CG3BC_IMAGELEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) < 0) {
                sunFbData[fbNum].probeStatus = probedAndFailed;
                (void) close(fd);
                return FALSE;
            }
        } else {
            sunFbData[fbNum].probeStatus = probedAndFailed;
            (void) close(fd);
            return FALSE;
        }
        sunFbs[index].fd = fd;
        sunFbs[index].info = fbType;
        sunFbData[fbNum].probeStatus = probedAndSucceeded;
        if (fbType.fb_width == CG3A_WIDTH) { sunFbs[index].fb = (pointer) CG3ACfb; }
        else { sunFbs[index].fb = (pointer) CG3BCfb; }
    }
    // If we've ever successfully probed this device, do the following.
    oldNumScreens = pScreenInfo->numScreens;
    return (i > oldNumScreens);
}
