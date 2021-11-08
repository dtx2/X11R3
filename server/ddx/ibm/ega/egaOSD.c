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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaOSD.c,v 9.0 88/10/18 12:52:19 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaOSD.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaOSD.c,v 9.0 88/10/18 12:52:19 erik Exp $";
static char sccsid[] = "@(#)ega_osd.c	3.1 88/09/22 09:32:58";
#endif

#include <fcntl.h>
#include <sys/ioctl.h>

#if defined(ibm032)
#if defined(BSDrt) /* BSD 4.3 */
/*
#include <sys/file.h>
*/
#include <machinecons/buf_emul.h>
#include <machinecons/xio.h>
#else
	******** ERROR -- AIX/rt
#endif /* defined(ibm032) */
#else
#if defined(i386) /* AIX386 */
/*
#include <sys/fcntl.h>
*/
#include <machdep.h>
#else
	******** ERROR -- DOS ??
#endif
#endif

#include "X.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "OScompiler.h"

#include "ibmScreen.h"
#include "ibmTrace.h"

#include "egaVideo.h"
#include "egaReg.h"

extern void ErrorF() ;

#ifdef i386
EgaMemoryPtr egaScreenBase ;
#endif

int egaCheckDisplay( fd )
register int fd ; /* Device File Decriptor */
{
#if defined(ibm032)
#if defined(BSDrt)
static unsigned long int infoword ;

return ( ioctl( fd, BUFDISPINFO, &infoword ) < 0 ) ? -1 : ( infoword & 3 ) ;
#else
	******** ERROR -- AIX/rt
#endif
#else
#if defined(i386)
return COLOR_TUBE ; /* THIS IS CERTAINLY WRONG !! */
#else
	******** ERROR -- DOS ??
#endif
#endif
}

int
egaProbe()
{
register int fd ;

extern int egaCheckDisplay( ) ;

    TRACE( ( "egaProbe()\n" ) ) ;
    if ( ( fd = open( "/dev/ega", O_RDWR | O_NDELAY | O_EXCL ) ) < 0 ) {
	return fd ;
    }
    ibmInfoMsg( "Found an EGA display.\n" ) ;

    return fd ;
}

#if !defined(DOS)
static int
unix_ega_init( index )
register int index ;
{
register int fd ;
#if defined(BSDrt)
static int out_emulator = E_XOUTPUT ;
#if defined(ATRIO)
static const int mode = (int) MODE_124 ;
static unsigned page = SCREEN_ADDR ;
#endif
#endif

extern void exit() ;
extern int ioctl() ;

/* Open Device File */
if ( ( fd = ibmScreenFD( index ) ) < 0 ) {
	ErrorF( "unix_ega_init: ega file descriptor invalid, exiting..." ) ;
	exit( 1 ) ;
}

#if defined(ibm032)
#if defined(BSDrt)
#if defined(RTIO)

/* Open Device File */
if ( ( fd = ibmScreenFD( index ) ) < 0 ) {
	ErrorF( "unix_ega_init: ega file desciptor invalid, exiting..." ) ;
	exit( 1 ) ;
}
#else
#if defined(ATRIO)

/* Set 128k window to point to ega display buffer */
if ( ioctl( fd, BUFSETWIND, &page ) < 0 )
	ErrorF( "unix_ega_init: egaioctl, set window" ) ;
/* Set The Display Mode To Avoid Confusing The BIOS Later */
if ( ioctl( fd, BUFINITVGA, &mode ) < 0 )
	ErrorF( "unix_ega_init: egaioctl, ega init" ) ;

#else /* !defined(ATRIO) || !defined(RTIO) */
	****** ERROR -- PCIO on an ibm032 ??
#endif /* ATRIO */
#endif /* RTIO */

if ( ioctl( fd, EOSETD, &out_emulator ) < 0 )
	ErrorF( "unix_ega_init: error setting output emulator" ) ;

#else /* !defined(BSDrt) */
	******** ERROR -- AIX/rt
#endif /* RTIO */
#endif /* BSDrt */
#else /* !defined(ibm032) */
#if defined(i386)

/* Set the display to BIOS mode 0x12 -- AIX386 mode 0x1C */
if ( ioctl( fd, EGAMODE, 0x1C ) < 0 )
	ErrorF( "unix_ega_init: egaioctl, set ega mode" ) ;
/* Map the display buffer into user space */
if ( ( (int) ( egaScreenBase = (EgaMemoryPtr) ioctl( fd, MAPCONS, 0 ) ) ) <= 0 )
	ErrorF( "unix_ega_init: egaioctl, ega map video memory" ) ;
#else
	******** ERROR -- DOS ??
#endif /* defined(i386) */
#endif /* !defined(ibm032) */

return fd ;
}

/*ARGSUSED*/
int
egaScreenInitHW( index )
register int	index ;
{
#if !defined(DOS)
static char been_here = 0 ;

if ( !been_here ) {
	unix_ega_init( index ) ;
	been_here = 1 ;
}
#endif
set_ega_graphics() ;

return 4 ; /* Number Of Bit Planes */
}

/*ARGSUSED*/
void
egaCloseHW( index )
register int	index ;
{
/* Do nothin' for nobody */
return ;
}
