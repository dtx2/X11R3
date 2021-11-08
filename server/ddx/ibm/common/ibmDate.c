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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmDate.c,v 9.0 88/10/17 14:55:06 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmDate.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmDate.c,v 9.0 88/10/17 14:55:06 erik Exp $";
static char sccsid[] = "@(#)ibmdate.c	3.1 88/09/22 09:31:42";
#endif


#if !defined(DATE) || !defined(HOST) || !defined(DEVELOPER)
	********* ERROR ********
#endif

extern void ibmInfoMsg() ;

void
ibmPrintBuildDate()
{
#ifdef DEBUG
static char buildDate[] = DATE ;
static char buildDeveloper[] = DEVELOPER ;
static char buildHost[] = HOST ;

ibmInfoMsg( "Server linked %s by %s on %s \n",
	    buildDate, buildDeveloper, buildHost );
#endif
return ;
}
