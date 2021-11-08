#ifndef MPEL_H_SEEN
#define	MPEL_H_SEEN	1
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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpel.h,v 1.2 88/10/25 01:33:46 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpel.h,v $ */
/* "@(#)mpel.h	3.1 88/09/22 09:33:35" */

#ifndef lint
static char *rcsidmpel = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpel.h,v 1.2 88/10/25 01:33:46 kbg Exp $";
#endif

/* this whole file should go away , another day */

#define	MPEL_WIDTH	1024
#define	MPEL_HEIGHT	1024
/* 8 bit planes */
#define	MPEL_ALLPLANES 0xFF

#ifdef __HIGHC__
#define MIN(a,b) _min((a),(b))
#define MAX(a,b) _max((a),(b))
#define ABS(a)	_abs(a)
#define MOVE _move
#else
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define	ABS(a)	((a)>=0?(a):-(a))
#define MOVE _bcopy
#endif

#define NULL 0

#endif /* ndef MPEL_H_SEEN */
