/***********************************************************
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

******************************************************************/
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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelDash.c,v 1.3 88/10/25 00:05:07 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelDash.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelDash.c,v 1.3 88/10/25 00:05:07 kbg Exp $";
#endif

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "font.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mfb.h"

#include "mistruct.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "ibmTrace.h"

#include "mpelProcs.h"
#include "mpelFifo.h"
#include "mpelHdwr.h"

#define barrel_shift16(v, s) \
	(v = (v >> s) | (v << (16 - s)))

void
mpel_do_dashline_gc(pGC)
register GCPtr pGC;
{
	register unsigned int len = 0;
	register unsigned int n;
       	mpelPrivGCPtr mpelPriv =
		(mpelPrivGCPtr) ( (ppcPrivGCPtr) pGC->devPriv )->devPriv;

	/* check for dash pattern useable to mpel */
	for ( n = 0 ; n < pGC->numInDashList ; n++ )
		len += pGC->dash[n];

	if (	len == 2 ||
		len == 4 ||
		len == 8 ||
		(len == 16 && ((pGC->numInDashList & 1) == 0)))
	{
		register int bit;
		register int offset = 0;
		register unsigned char *dp = &pGC->dash[0];
		mpelUserLineStyle ULineStyle;

		ULineStyle.factor = 0;
		ULineStyle.mask = 0;
		/* encode dash pattern into bitmask */
		while (n-- > 0 && *dp != 0){
			ULineStyle.mask |= ( ( ( 1 << *dp ) - 1 ) << offset ) ;
			offset += *dp++;
			if (n-- > 0)
				offset += *dp++;
		}
		/* replicate pattern over full 16 bits */
		for (bit = offset; bit < 16; bit++){
			ULineStyle.mask |=
				(ULineStyle.mask & (1 << bit - offset))
					<< offset;
		}
		/* shift pattern to implement offset - mod 16 */
		/* is it legal for offset > length ?? */
		if (pGC->dashOffset != 0)
			barrel_shift16(ULineStyle.mask, pGC->dashOffset % 16);
		mpelPriv->LineType = MPEL_USERLINESTYLE_CONT;
		mpelPriv->LineStyle = ULineStyle;
    		pGC->PolySegment = mpelPolySegment;
		pGC->Polylines = mpelZeroLine;
		pGC->PolyArc = mpelPolyArc;
	} else {
    		pGC->PolySegment = miPolySegment;
		pGC->Polylines =  ppcScrnZeroDash;
		pGC->PolyArc = miPolyArc;
		mpelPriv->LineType = MPEL_SOLIDLINE;
	}
	return ;
}
