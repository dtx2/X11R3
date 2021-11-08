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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdInitEmul.c,v 9.1 88/10/17 14:35:52 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdInitEmul.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdInitEmul.c,v 9.1 88/10/17 14:35:52 erik Exp $";
#endif

#if !defined(ibm032) || !defined(BSDrt)
	******** ERROR ********
#endif

#include <sys/types.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <machinecons/screen_conf.h>
#include <machinecons/buf_emul.h>

#include <sys/time.h>		/* For resource-limit calls */
#include <sys/resource.h>	/* For resource-limit calls */

#include "X.h"
#include "misc.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "ibmScreen.h"
#include "bsdIO.h"

#include "ibmTrace.h"

extern void ibmInfoMsg() ;

XIoAddr		*BSDXaddr;
int		 BSDEmulatorFD=	-1;
int		 BSDdevbusFD=	-1;

extern XEventQueue	*ibmQueue;

#ifdef SIGGRANT	/* only if signal is defined */
#define MAX_PENDING_SCREEN_CHANGES 8
static int signalProcessingPending = 0 ;
#ifndef SIGRETRACT
#define SIGRETRACT SIGRELEASE /* Blame Bob Relyea ! */
#endif

void
BSDScreenStateChange( pE )
register XEvent *pE ;
{
register ibmPerScreenInfo *screenInfo = ibmScreens[ pE->xe_key ] ;

/* NOTE: This function uses the fact that sigblock() returns the current
 *	signal mask AND ignores attempts to block SIGKILL !!
 */

/* Up for lose screen -- Down for regain */
if ( pE->xe_direction == XE_KBTUP ) {
	(* screenInfo->ibm_SaveFunc)();
	if ( ioctl( screenInfo->ibm_ScreenFD, BUFSAVEDONE) < 0 )
		ErrorF( "BSDScreenStateChange: ioctl Failed\n" ) ;
	/* Should Unblock SIGRETRACT here !! */
	(void) sigsetmask( sigblock( SIGKILL ) & ~ sigmask( SIGRETRACT ) ) ;
}
else if ( pE->xe_direction == XE_KBTDOWN ) {
	(* screenInfo->ibm_RestoreFunc)();
	if ( ioctl( screenInfo->ibm_ScreenFD, BUFRESTOREDONE ) < 0 )
		ErrorF( "BSDScreenStateChange: ioctl Failed\n" ) ;
	/* Should Unblock SIGGRANT here !! */
	(void) sigsetmask( sigblock( SIGKILL ) & ~ sigmask( SIGGRANT ) ) ;
}
else
	ErrorF( "BSDScreenStateChange: Bad Screen Event\n" ) ;

return ;
}

static int
findSignaledScreen( desiredState )
register int desiredState ;
{
register int i ;
unsigned long int screenStatus ;

for ( i = ibmNumScreens ; i-- ; )
	if ( ioctl( ibmScreenFD(i), BUFGETSAVE, &screenStatus ) < 0 )
		ErrorF( "FindSignaledScreen: ioctl BUFGETSAVE FAILED\n" ) ;
	else if ( screenStatus == desiredState )
		return i ;

return -1 ;
}

/* BSDRetractHandler( sig ) -- Should Do :
 -	ioctl() to determine which screen
 -	then ioctl() to push a "Screen" event onto the shared memory queue
 -	All "real" stuff is done when the event is taken off the queue !
 -	NOTE: After receiving the signal, further signals MUST be "held"
 -		until the screen has actually been saved !
 */
void
BSDRetractHandler( sig, code, cp )
register int sig ;
int code ;
register struct sigcontext *cp ;
{
register int whichScreen ;
XEvent dummyEvent ;

if ( ( whichScreen = findSignaledScreen( BUF_NEED_SAVE ) ) < 0 )
	ErrorF( "BSDRetractHandler : no screen needs to be saved!\n" ) ;
else {
	dummyEvent.xe_key = whichScreen ;
	dummyEvent.xe_x = 0 ;
	dummyEvent.xe_y = 0 ;
	dummyEvent.xe_time = 0 ;
	dummyEvent.xe_type = 0 ;
	/* Up for lose screen -- Down for regain */
	dummyEvent.xe_direction = XE_KBTUP ;
	dummyEvent.xe_device = XE_CONSOLE ; /* console */
	if ( ioctl( ibmScreenFD(0), QIOCMUSTADD, &dummyEvent ) < 0 )
		ErrorF( "BSDRetractHandler : ioctl QIOCMUSTADD FAILED\n" ) ;
	cp->sc_mask |= sigmask( SIGRETRACT ) ;
}

#if defined(DEBUG)
(void) printf( "BSDRetractHandler(%d) Called!!\n", sig ) ;
(void) printf( "BSDRetractHandler: final Screen #%d\n", whichScreen ) ;
#endif

return ;
}

/* BSDGrantHandler()
 - Should Do :
 -	ioctl() to determine which screen
 -	then ioctl() to push a "Screen" event onto the shared memory queue
 -	All "real" stuff is done when the event is taken off the queue !
 */
void
BSDGrantHandler( sig, code, cp )
register int sig ;
int code ;
register struct sigcontext *cp ;
{
register int whichScreen ;
XEvent dummyEvent ;

if ( ( whichScreen = findSignaledScreen( BUF_NEED_RESTORE ) ) < 0 )
	ErrorF( "BSDGrantHandler : no screen needs to be restored!\n" ) ;
else {
	dummyEvent.xe_key = whichScreen ;
	dummyEvent.xe_x = 0 ;
	dummyEvent.xe_y = 0 ;
	dummyEvent.xe_time = 0 ;
	dummyEvent.xe_type = 0 ;
	/* Up for lose screen -- Down for regain */
	dummyEvent.xe_direction = XE_KBTDOWN ;
	dummyEvent.xe_device = XE_CONSOLE ; /* console */
	if ( ioctl( ibmScreenFD(0), QIOCMUSTADD, &dummyEvent ) < 0 )
		ErrorF( "BSDGrantHandler : ioctl QIOCMUSTADD FAILED\n" ) ;
	cp->sc_mask |= sigmask( SIGGRANT ) ;
}
#if defined(DEBUG)
(void) printf( "BSDGrantHandler( %d ) Called!!\n", sig ) ;
(void) printf( "BSDGrantHandler: final Screen #%d\n", whichScreen ) ;
#endif

return ;
}
#endif /* def SIGGRANT */

static void
ibm6152_os2_setup()
{
#ifdef SIGGRANT	/* only if signal is defined */
signal( SIGRETRACT, BSDRetractHandler ) ;
signal( SIGGRANT, BSDGrantHandler ) ;
#endif 
return ;
}

static void
rtVS6152()
{
unsigned long int infoword ;

if ( ioctl( ibmScreenFD(0), BUFDISPINFO, &infoword ) >= 0 ) {
	if ( BUF_IS_RTPC(infoword) ) {
		ibmInfoMsg( "Machine Type Is IBM RT-PC\n" ) ;
		if ( ( BSDdevbusFD = open( "/dev/bus", O_RDWR ) ) < 0 ) {
			ErrorF("open of /dev/bus failed!\nExiting.\n") ;
			exit(1);
		}
	}
	else {
		ibmInfoMsg( "Machine Type Is IBM 6152\n" ) ;
		ibm6152_os2_setup() ;
	}
}
else
	ErrorF( "ioctl of emulator FD failed!\n" ) ;

return ;
}

static void
setBSDlimits()
{
struct rlimit res ;

/* Make sure the process doesn't bang against any resource limits !! */
/* Set each of CPU, FSIZE, DATA, STACK, CORE, & RSS to the maximum allowed !! */
if ( getrlimit( RLIMIT_CPU, &res ) )
	ErrorF( "Can't getrlimit( RLIMIT_CPU, &res ) !\n" ) ;
else {
	res.rlim_cur = res.rlim_max ;
	if ( setrlimit( RLIMIT_CPU, &res ) )
		ErrorF( "Can't setrlimit( RLIMIT_CPU, &res ) !\n" ) ;
}
if ( getrlimit( RLIMIT_FSIZE, &res ) )
	ErrorF( "Can't getrlimit( RLIMIT_FSIZE, &res ) !\n" ) ;
else {
	res.rlim_cur = res.rlim_max ;
	if ( setrlimit( RLIMIT_FSIZE, &res ) )
		ErrorF( "Can't setrlimit( RLIMIT_FSIZE, &res ) !\n" ) ;
}
if ( getrlimit( RLIMIT_DATA, &res ) )
	ErrorF( "Can't getrlimit( RLIMIT_DATA, &res ) !\n" ) ;
else {
	res.rlim_cur = res.rlim_max ;
	if ( setrlimit( RLIMIT_DATA, &res ) )
		ErrorF( "Can't setrlimit( RLIMIT_DATA, &res ) !\n" ) ;
}
if ( getrlimit( RLIMIT_STACK, &res ) )
	ErrorF( "Can't getrlimit( RLIMIT_STACK, &res ) !\n" ) ;
else {
	res.rlim_cur = res.rlim_max ;
	if ( setrlimit( RLIMIT_STACK, &res ) )
		ErrorF( "Can't setrlimit( RLIMIT_STACK, &res ) !\n" ) ;
}
if ( getrlimit( RLIMIT_CORE, &res ) )
	ErrorF( "Can't getrlimit( RLIMIT_CORE, &res ) !\n" ) ;
else {
	res.rlim_cur = res.rlim_max ;
	if ( setrlimit( RLIMIT_CORE, &res ) )
		ErrorF( "Can't setrlimit( RLIMIT_CORE, &res ) !\n" ) ;
}
if ( getrlimit( RLIMIT_RSS, &res ) )
	ErrorF( "Can't getrlimit( RLIMIT_RSS, &res ) !\n" ) ;
else {
	res.rlim_cur = res.rlim_max ;
	if ( setrlimit( RLIMIT_RSS, &res ) )
		ErrorF( "Can't setrlimit( RLIMIT_RSS, &res ) !\n" ) ;
}

return ;
}

/*
 * -- This Entry Point is called from common code !
 */
void
BSDMachineDependentInit()
{
/* Do anything which depends upon whether we're on an rt or a 6152 */
rtVS6152() ;

/* Maximize BSD resource allocations */
setBSDlimits() ;

return ;
}

void
BSDInitEmulator()
{
static int been_here= 0;
int	emulator= E_XINPUT;
struct	x_screen_size bounds;
int	scr;
int	miny,maxy;

    TRACE(("BSDInitEmulator()"));

    if ( ibmNumScreens <= 0 ) {
	ErrorF(
	  "BSDInitEmulator: No open screens in BSDInitEmulator.\nExiting.\n" ) ;
	exit(1) ;
    }
    if (!been_here) {
	been_here= 1;
	/* Make the first screen the input source !! */
	BSDEmulatorFD = ibmScreenFD(0) ;
	if ( BSDEmulatorFD < 0 ) {
	    ErrorF( "BSDInitEmulator: ScreenFD 0 not opened!\nExiting.\n" ) ;
	    exit(1);
	}
	if ( ioctl( BSDEmulatorFD, EISETD, &emulator ) < 0 ) {
	    ErrorF( "BSDInitEmulator: Failed to set emulator!\nExiting.\n" ) ;
	    exit(1);
	}

#ifdef SCRSETNINP
	/* Shut Off kernel output emulators for all but the first screen !! */
	for ( scr = 1 ; scr < ibmNumScreens ; scr++ ) {
		/* This ioctl is "Screen-Set-No-Input" */
		if ( ioctl( ibmScreenFD(scr), SCRSETNINP ) < 0 )
			ErrorF(
"BSDInitEmulator: Failed to remove screen emulator! Possible old kernel.\n" ) ;
	}
#endif

	if ( ioctl( BSDEmulatorFD, QIOCHIDECUR ) < 0 ) {
	    ErrorF( "BSDInitEmulator: Failed to hide cursor!\n" ) ;
	}

	/* Ask OS for X style information */
	if ( ioctl( BSDEmulatorFD, QIOCADDR, &BSDXaddr ) < 0 ) {
	    ErrorF(
		"BSDInitEmulator: Failed to get X event queue!\nExiting.\n" ) ;
	    exit(1);
	}

	BSDXaddr->mbox.bottom =
	BSDXaddr->mbox.top =
	BSDXaddr->mbox.left =
	BSDXaddr->mbox.right = 10000 ;

	/* Initialize OS-INdependent X-Queue */
	ibmQueue = (XEventQueue *) &BSDXaddr->ibuff ;

	miny = ibmScreenMinY(0);
	maxy = ibmScreenMaxY(0);
	for ( scr = 1 ; scr < ibmNumScreens ; scr++ ) {
	    if ( ibmScreenMinY(scr) < miny )
		miny = ibmScreenMinY(scr) ;
	    if ( ibmScreenMaxY(scr) > maxy )
		maxy = ibmScreenMaxY(scr) ;
	}

	/*
	 * If WrapScreen is defined, let the cursor leave the bounds of all
	 * the screens, so we can wrap it.  50 is an arbitrary choice.
	 */
	bounds.x_x_min = ibmScreenMinX(0) - (ibmXWrapScreen?1:0) ;
	bounds.x_x_max = ibmScreenMaxX(ibmNumScreens-1) + (ibmXWrapScreen?1:0) ;
	bounds.x_y_min = miny - (ibmYWrapScreen?1:0) ;
	bounds.x_y_max = maxy + (ibmYWrapScreen?1:0) ;
	if ( ioctl( BSDEmulatorFD, QIOCSETSIZE, &bounds ) == -1 ) {
	    ErrorF( "BSDInitEmulator: couldn't set bounds:\nExiting ...\n" ) ;
	    exit(1) ;
	}
    }
    return ;
}
