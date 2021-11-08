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

/* tlDstate.c - keep track of and setup dragon state */

#include	"miscstruct.h"
#include	<sys/types.h>
#include	"Ultrix2.0inc.h"
#include	"qdreg.h"

#include	"tldstate.h"	/* load the constants and structures */

/* setup the default shadow state */
dStateRec	dState;

tldinit()
{
	register int	i;

	dState.template = ILL_TEMPLATE;
	dState.planemask = ILL_PLANEMASK;

	for (i = i_BVIPERS; i < i_EVIPERS; i++)
		dState.planes[PLANEINDEX(i)] = ILL_PLANEMASK;

	dState.common.alu =	dState.remain.alu =	ILL_ALU;
	dState.common.mask =	dState.remain.mask =	ILL_MASK;
	dState.common.source =	dState.remain.source =	ILL_SOURCE;
	dState.common.bgpixel =	dState.remain.bgpixel =	ILL_BGPIXEL;
	dState.common.fgpixel =	dState.remain.fgpixel =	ILL_FGPIXEL;

	dState.adder.clip.x1 =	dState.adder.clip.x2 =
	dState.adder.clip.y1 =	dState.adder.clip.y2 =	ILL_CLIP;
	dState.adder.translate.x =
	dState.adder.translate.y =	ILL_TRANSLATE;
	dState.adder.rastermode =	ILL_RASTERMODE;

	for (i = 0; i < 6; i++)
		dState.common.ocr[i] =
		dState.remain.ocr[i] =	ILL_OCR;
}

Invalid_Shadow()
{
	dState.planemask = ILL_PLANEMASK;
}

Hose_Shadow()
{
	register int	i;

	for (i = i_BVIPERS; i < i_EVIPERS; i++)
		dState.planes[PLANEINDEX(i)] = ILL_PLANEMASK;
	dState.template = ILL_TEMPLATE;
	dState.planemask = ILL_PLANEMASK;
	dState.adder.clip.x1 =	dState.adder.clip.x2 =
	dState.adder.clip.y1 =	dState.adder.clip.y2 =	ILL_CLIP;
	dState.adder.translate.x =
	dState.adder.translate.y =	ILL_TRANSLATE;
	dState.adder.rastermode =	ILL_RASTERMODE;
}

