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
/* $Header: vga_osd.c,v 6.1 88/08/18 10:23:25 mar Exp $ */
/* $Source: /vice/X11/r3/server/ddx/ibm/vga/RCS/vga_osd.c,v $ */

#ifndef lint
static char *rcsid = "$Header: vga_osd.c,v 6.1 88/08/18 10:23:25 mar Exp $";
#endif

#include <sys/ioctl.h>

#ifdef BSDrt
/* BSD 4.3 */
#include <sys/file.h>
#include <machinecons/buf_emul.h>

#else
/* AIX386 */
#include <sys/fcntl.h>
#endif

#include "X.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "OScompiler.h"

#include "ibmScreen.h"

#include "vgaVideo.h"
#include "vgaSave.h"

/* Global Variables */
int vgaNumberOfPlanes  = 0 ;
int vgaDisplayTubeType = 0 ;

int vgaCheckDisplay( fd )
register int fd ; /* Device File Decriptor */
{
#ifndef i386
static unsigned long int infoword ;

return ( ioctl( fd, BUFDISPINFO, &infoword ) < 0 ) ? -1 : ( infoword & 3 ) ;
#else
return COLOR_TUBE ; /* THIS IS CERTAINLY WRONG !! */
#endif
}

static int vga_file_desc = -1 ;

int
vgaProbe()
{
register int fd ;

    if ( ( fd = open( "/dev/vga", O_RDWR | O_NDELAY ) ) < 0 ) {
	return fd ;
    }
    if ( ( vgaDisplayTubeType = vgaCheckDisplay( fd ) ) <= 0 ) {
	(void) close( fd ) ;
	return -1 ;
    }
#if defined(i386)
/* Set the display to BIOS mode 0x12 -- AIX386 mode 0x1C */
    if ( ioctl( fd, EGAMODE, 0x1C ) < 0 ) {
	perror( "vgaProbe: vgaioctl, set ega mode" ) ;
	return -1 ;
    }
    /* Map the display buffer into user space */
    if ( SCREEN_ADDR != ioctl( fd, MAPCONS, 0 ) ) {
	perror( "vgaProbe: vgaioctl, vga map video memory address wrong" ) ;
	return -1 ;
    }
#endif

    ibmInfoMsg( ( vgaDisplayTubeType == COLOR_TUBE )
      ? "Found a vga with color display.\n" : 
	"Found a vga with gray scale display.\n" ) ;

    return vga_file_desc = fd ;
}

#if !defined(DOS)
static void
unix_vga_init( index )
int index ;
{
register int fd ;
#if defined(BSDrt) && defined(ATRIO)
static const int mode = (int) MODE_124 ;
static unsigned page = SCREEN_ADDR ;

extern void perror() ;
extern void exit() ;
#endif

extern int ioctl() ;

/* Open Device File */
	
if ( ( fd = vga_file_desc ) < 0 ) {
	perror( "unix_vga_init: vga file descriptor invalid, exiting..." ) ;
	exit( 1 ) ;
}

#if defined(BSDrt) && defined(ATRIO)

/* Set 128k window to point to vga display buffer */
if ( ioctl( fd, BUFSETWIND, &page ) < 0 )
	perror( "unix_vga_init: vgaioctl, set window" ) ;
/* Set The Display Mode To Avoid Confusing The BIOS Later */
if ( ioctl( fd, BUFINITVGA, &mode ) < 0 )
	perror( "unix_vga_init: vgaioctl, vga init" ) ;
#endif

return ;
}

#endif

extern void save_vga_state( ) ;
extern void restore_vga_state( ) ;
extern void save_dac( ) ;
extern void restore_dac( ) ;
extern void set_graphics_mode() ;

/* Video State On Program Entry */
static struct vga_video_hardware_state VS_Start ;
static DAC_TABLE init_dac_table ;

int
vgaScreenInitHW( index )
register int	index ;
{
static char been_here = 0 ;

if ( !been_here ) {
#if !defined(DOS)
	unix_vga_init( index ) ;
#endif
	/* Save Extant Video State & Setup For Graphics */
	save_vga_state( &VS_Start ) ;
	save_dac( init_dac_table ) ;
	been_here = 1 ;
}
/* If The Display Is Turned Off Or Changed It Should Take Effect Now */
if ( ( vgaDisplayTubeType = vgaCheckDisplay( vga_file_desc ) ) > 0 ) {
	set_graphics_mode( &VS_Start ) ;
	return 4 ; /* Number Of Bit Planes */
}
else {
	ErrorF( "Headless vga!\n Check monitor cables!\n" ) ;
	return 0 ; /* Error Condition */
}
/*NOTREACHED*/
}

/*ARGSUSED*/
void
vgaCloseHW( index )
register int index ;
{

#if defined(i386) && !defined(DOS)
/* Set the display to BIOS mode 0x7 -- AIX386 mode 0x3 */
if ( ioctl( vga_file_desc, EGAMODE, 0x3 ) < 0 )
	perror( "vgaCloseHW: vgaioctl, set ega mode" ) ;
#endif
restore_dac( init_dac_table ) ;
restore_vga_state( &VS_Start ) ;

return ;
}
