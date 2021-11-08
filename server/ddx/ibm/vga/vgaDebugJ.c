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
/* $Header: debugj.c,v 6.0 88/08/18 08:56:43 erik Exp $ */
/* $Source: /vice/X11/r3/server/ddx/ibm/vga/RCS/debugj.c,v $ */

/* Include Helpful Procedures For Debugging */

#ifdef DEBUG
#ifndef NDEBUG

#ifndef lint
static char *rcsid = "$Header: debugj.c,v 6.0 88/08/18 08:56:43 erik Exp $";
#endif

#include "ibmIOArch.h"

int inbyte( a )
register a ;
{
return inb( a ) ;
}

int outbyte( a, b )
register a, b ;
{
return outb( a, b ) ;
}

static int (*randomjunk[])() = { inbyte, outbyte } ;

#endif
#endif
