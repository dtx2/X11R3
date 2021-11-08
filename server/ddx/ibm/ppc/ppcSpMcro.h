/***********************************************************
		Copyright IBM Corporation 1988

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
	/*
	 * IBM Confidential Restricted
	 * Copyright (c) IBM Corporation 1987,1988
	 * All Rights Reserved.
	 *
	 * Owner: Tom Paquin, 8-465-4423
	 * Do not redistribute without permission 
	 */

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/ppc/RCS/ppcSpMcro.h,v 9.1 88/10/24 22:30:23 paul Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/ppc/RCS/ppcSpMcro.h,v $ */


/* This screwy macro is used in all the spans routines and you find
   it all over the place, so it is a macro just to tidy things up.
*/

#define SETSPANPTRS(IN,N,IPW,PW,IPPT,PPT,FPW,FPPT,FSORT)		\
	{								\
	N = IN * miFindMaxBand(((ppcPrivGC *)(pGC->devPriv))->pCompositeClip);\
	if(!(PW = (int *)ALLOCATE_LOCAL(N * sizeof(int))))		\
		return;							\
	if(!(PPT = (DDXPointRec *)ALLOCATE_LOCAL(N * sizeof(DDXPointRec)))) \
		{							\
		DEALLOCATE_LOCAL(PW);					\
		return;							\
    		}							\
	FPW = PW;							\
	FPPT = PPT;							\
	N = miClipSpans(((ppcPrivGC *)(pGC->devPriv))->pCompositeClip,	\
		IPPT, IPW, IN,						\
		PPT, PW, FSORT);					\
	}

