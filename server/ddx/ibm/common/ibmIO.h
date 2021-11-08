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
#ifndef IBM_IO
#define IBM_IO 1

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmIO.h,v 9.0 88/10/17 14:55:28 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmIO.h,v $ */
/* "%W% %E% %U%" */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char *rcsidibmio = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmIO.h,v 9.0 88/10/17 14:55:28 erik Exp $";
#endif

extern	void	ProcessInputEvents();
extern	int	TimeSinceLastInputEvent();
extern	void	ibmQueryBestSize();
extern	int	lastEventTime;
#ifdef IBM_OS_HAS_X_QUEUE
extern	XEventQueue	*ibmQueue;
#endif

#endif /* IBM_IO */
