/* 
 * wshadow.c : the window shadow
 *  a few munged routines from sunCursor.c
 */
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
/*-
 * sunCursor.c --
 *	Functions for maintaining the Sun software cursor...
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
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

#define NEED_EVENTS
#include    "sun.h"
#include    <servermd.h>
#include    <windowstr.h>
#include    <regionstr.h>
#include    <dix.h>
#include    <dixstruct.h>
#include    <opaque.h>
#include "cfb/cfb.h" /* XXX should this really be here? */

/* 
    pWin->ClearToBackground = miClearToBackground;
    pWin->PaintWindowBackground = cfbPaintAreaNone;
    pWin->PaintWindowBorder = cfbPaintAreaPR;
*/

/* ******************************************************************** */
/* *********** the routines used by the shadow ************************ */
/* ******************************************************************** */

#define RemoveCursor(pScreen) \
  (*((cfbPrivScreenPtr)pScreen->devPrivate)->CursorOff)(pScreen)

/*-
 * sunPaintWindowBackground & sunPaintWindowBorder --
 *	Paint the window's background while preserving the cursor
 */
void
hpPaintWindowBackground(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    BoxRec	cursorBox;
    ScreenPtr	pScreen;

    pScreen = pWin->drawable.pScreen;

    if (hpCursorLoc(pScreen, &cursorBox))
    {
	/*
	 * If the cursor is on the same screen as the window, check the
	 * region to paint for the cursor and remove it as necessary
	 */
	if ((*pScreen->RectIn)(pRegion, &cursorBox) != rgnOUT)
	    RemoveCursor(pScreen);
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunCopyWindow --
 *	Protect the cursor from window copies..
 *-----------------------------------------------------------------------
 */
static void
hpCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr	  pWin;
    DDXPointRec	  ptOldOrg;
    RegionPtr	  prgnSrc;
{
  BoxRec	cursorBox;
  ScreenPtr	pScreen;

  pScreen = pWin->drawable.pScreen;

  if (hpCursorLoc(pScreen, &cursorBox))
  {
	/*
	 * If the cursor is on the same screen, compare the box for the
	 * cursor against the original window clip region (prgnSrc) and
	 * the current window clip region (pWin->borderClip) and if it
	 * overlaps either one, remove the cursor. (Should it really be
	 * borderClip?)
	 */
    switch ((*pScreen->RectIn)(prgnSrc,&cursorBox))
    {
      case rgnOUT:
	if ((*pScreen->RectIn)(pWin->borderClip,&cursorBox)==rgnOUT) break;
      case rgnIN:
      case rgnPART: RemoveCursor(pScreen);
    }
  }
}

/* ******************************************************************** */
/* *********** the shadow ********************************************* */
/* ******************************************************************** */

void cfbpaintareanone(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  cfbPaintAreaNone(pWin, pRegion, what);
}

void cfbpaintarea32(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  cfbPaintArea32(pWin, pRegion, what);
}

void mipaintwindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  miPaintWindow(pWin, pRegion, what);
}

void cfbpaintareapr(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  cfbPaintAreaPR(pWin, pRegion, what);
}

void cfbpaintareaother(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  cfbPaintAreaOther(pWin, pRegion, what);
}

void cfbcopywindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr	  pWin;
    DDXPointRec	  ptOldOrg;
    RegionPtr	  prgnSrc;
{
  hpCopyWindow(pWin, ptOldOrg, prgnSrc);
  cfbCopyWindow(pWin, ptOldOrg, prgnSrc);
}
/* ******************************************************************** */
/* *********** the shadows for MRM/MRC ******************************** */
/* ******************************************************************** */

void mrcfbpaintareanone(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  mrcfbPaintAreaNone(pWin, pRegion, what);
}

void mrcfbpaintarea32(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  mrcfbPaintArea32(pWin, pRegion, what);
}

void mrcfbpaintareapr(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  mrcfbPaintAreaPR(pWin, pRegion, what);
}

void mrcfbpaintareaother(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  mrcfbPaintAreaOther(pWin, pRegion, what);
}

void mrcfbcopywindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr	  pWin;
    DDXPointRec	  ptOldOrg;
    RegionPtr	  prgnSrc;
{
  hpCopyWindow(pWin, ptOldOrg, prgnSrc);
  mrcfbCopyWindow(pWin, ptOldOrg, prgnSrc);
}
