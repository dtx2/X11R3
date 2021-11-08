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
#ifndef BSD_IO
#define BSD_IO 1

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdIO.h,v 9.1 88/10/17 14:35:40 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdIO.h,v $ */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char *rcsidrtio = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdIO.h,v 9.1 88/10/17 14:35:40 erik Exp $";
#endif

#include <machinecons/xio.h>
extern  XIoAddr	*BSDXaddr;	
extern	int	BSDEmulatorFD;

#ifndef IBM_OS_HAS_X_QUEUE
#define	IBM_OS_HAS_X_QUEUE 1
#endif

#include "ibmIO.h"

#endif /* BSD_IO */
