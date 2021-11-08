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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16OSD.c,v 9.1 88/10/17 14:45:21 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16OSD.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16OSD.c,v 9.1 88/10/17 14:45:21 erik Exp $";
static char sccsid[] = "@(#)apa16_osd.c	3.4 88/09/26 06:45:36";
#endif /* lint */

#include <sys/file.h>
#include <sys/ioctl.h>

#include "OSio.h"
#include "ibmTrace.h"

extern void ibmInfoMsg() ;

/***==================================================================***/

#ifdef BSDrt
/* Only Works for TCS BSD4.3 */

int
apa16Probe()
{
register int fd;
struct screen_control scr ;

    TRACE(("apa16Probe()\n")) ;

    if ( ( fd = open( "/dev/apa16", O_RDWR | O_NDELAY | O_EXCL ) ) < 0 ) 
	return -1 ;
    scr.device = CONS_APA16;
    if ( ioctl( fd, SCRIOCGETF, &scr ) < 0 ) {
	ErrorF( "WSGO! ioctl SCRIOCGETF failed in apa16Probe()\n" ) ;
	(void) close( fd ) ;
	return -1 ;
    }
    if ( scr.switches & CONSDEV_PRESENT ) {
	ibmInfoMsg( "Found an an apa16\n" ) ;
	return fd ;
    }
    else {
	(void) close( fd ) ;
	return -1 ;
    }
/*NOTREACHED*/
}
#else /* AIX rt */

#include <sys/hft.h>
#include "hftUtils.h"

extern int hftQFD;

#include "trace.h"

extern void ibmInfoMsg() ;

/***==================================================================***/

int
apa16Probe()
{

    TRACE(("apa16Probe()\n")) ;
    AIXDefaultDisplay= HFT_APA16_ID;
    if (hftHasAnAttached(HFT_APA16_ID)) {
	ibmInfoMsg("Found a 6155 Extended Monochrome Graphics Adapter\n");
	if (hftQFD>=0)	return hftQFD;
	else		return 0;
    }
    else {
	ibmInfoMsg("Didn't find a 6155 Extended Monochrome Graphics Adapter\n");
    	return -1;
    }
}
#endif /*RtAIX*/
