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
/*
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

*/
/* $XConsortium: mfbfont.c,v 1.2 88/09/06 15:20:21 jim Exp $ */
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "scrnintstr.h"

#include "mfb.h"

/*
 * Take advantage of the per-screen private information field in the font to
 * encode the results of fairly complex tests of the font's metric fields.
 * ValidateFont need merely examine the code to select the output routines to
 * be pointed to in the GC.
 */
Bool
mfbRealizeFont( pscr, pFont)
    ScreenPtr	pscr;
    FontPtr	pFont;
{
    /*
     * pGC->font is now known to be valid
     */
    int			index = pscr->myNum;
    FontInfoPtr		pfi = pFont->pFI;
    CharInfoPtr		maxb = &pfi->maxbounds;
    CharInfoPtr		minb = &pfi->minbounds;

#ifdef notdef
    /*
     * ifdef'd out by Harry on 12/6/87
     * using a devPriv[] pointer slot for a scalar is stupid.
     */
    /*
     * pick the fastest output routines that can do the job.
     */
    if (   maxb->metrics.rightSideBearing -
   		minb->metrics.leftSideBearing > 32	/* big glyphs */
	  || pfi->drawDirection != FontLeftToRight
	  || pfi->noOverlap == 0)
	pFont->devPriv[ index] = (pointer)FT_VARPITCH;
    else  /* an optimizable case */
    {
	if (     maxb->metrics.leftSideBearing ==
		    minb->metrics.leftSideBearing /* fixed pitch */
	      && maxb->metrics.leftSideBearing == 0	  /* fixed pitch */
	      && maxb->metrics.rightSideBearing ==
	            minb->metrics.rightSideBearing /* fixed pitch */
	      && maxb->metrics.characterWidth ==
	            minb->metrics.characterWidth  /* fixed pitch */
	      && maxb->metrics.ascent ==
	            minb->metrics.ascent	  /* fixed height */
	      && maxb->metrics.descent ==
	            minb->metrics.descent)  /* fixed height */
	    pFont->devPriv[ index] = (pointer)FT_FIXPITCH;
	else
	    pFont->devPriv[ index] = (pointer)FT_SMALLPITCH;
    }
#endif /* notdef */
    return (TRUE);
}

/*
 * no storage allocated in mfbRealizeFont, so there is nothing to do
 */
Bool
mfbUnrealizeFont( pscr, pFont)
    ScreenPtr	pscr;
    FontPtr	pFont;
{
    return (TRUE);
}
