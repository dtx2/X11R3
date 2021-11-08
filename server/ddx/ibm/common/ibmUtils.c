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
/* $Header: /site/forMIT/server/ddx/ibm/common/RCS/ibmUtils.c,v 9.0 88/10/17 14:55:36 erik Exp Locker: erik $ */
/* $Source: /site/forMIT/server/ddx/ibm/common/RCS/ibmUtils.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /site/forMIT/server/ddx/ibm/common/RCS/ibmUtils.c,v 9.0 88/10/17 14:55:36 erik Exp Locker: erik $";
static char sccsid[] = "@(#)ibmutils.c	3.1 88/09/22 09:32:17";
#endif

#include "X.h"
#include "cursorstr.h"
#include "miscstruct.h"
#include "scrnintstr.h"

#include "ibmScreen.h"

#include "OSio.h"

#include "ibmTrace.h"

int	ibmTrace;

#ifdef NOT_X
#define ErrorF printf
#endif

/***==================================================================***/

int ibmQuietFlag = 0 ;

/*VARARGS1*/
/*ARGSUSED*/
void
ibmInfoMsg( str, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 )
char *str ;
{
if ( !ibmQuietFlag )
	(void) ErrorF( str, arg1, arg2, arg3, arg4, arg5,
		       arg6, arg7, arg8, arg9 ) ;
return ;
}

/***==================================================================***/

void
ddxGiveUp()
{
    TRACE(("ddxGiveUp()"));
    OS_CapsLockFeedback(0);
    return;
}

/***==================================================================***/

void
AbortDDX()
{
    TRACE(("AbortDDX()"));
    OS_CapsLockFeedback(0);
    return;
}

/***==================================================================***/

ddxUseMsg()
{
int	i;

    TRACE(("ddxUseMsg()\n"));
    ErrorF("Recognized screens are:\n");
    ErrorF("    -all		opens all attached, supported screens\n");
    for (i=0;(ibmPossibleScreens[i]!=NULL);i++) {
	ErrorF("    %s\n",ibmPossibleScreens[i]->ibm_ScreenFlag);
    }
    ErrorF("Other device dependent options are:\n");
    ErrorF("    -bs		turn on backing store for windows that request it\n");
#if defined(AIXrt) && defined(IBM_GSL)
    ErrorF("	-dd <dev>	specify a default display for GSL\n");
#endif
#ifdef IBM_SPECIAL_MALLOC
    ErrorF("    -lock 		enable caps lock key (default)\n");
    ErrorF("    -malloc #	set malloc check level (0-5)\n");
#endif
    ErrorF("    -nobs		always deny backing store\n");
#ifndef RT_MUST_USE_HDWR
    ErrorF("    -nohdwr		use generic functions where applicable\n");
#endif
    ErrorF("    -nolock 	disable caps lock key (default)\n");
    ErrorF("    -pckeys		swap CAPS LOCK and CTRL (for touch typists)\n");
#ifdef IBM_SPECIAL_MALLOC
    ErrorF("    -plumber string	dump malloc arena to named file\n");
#endif
    ErrorF("    -quiet		do not print informational messages\n");
    ErrorF("    -rtkeys		use CAPS LOCK and CTRL as labelled\n");
#ifdef TRACE_X
    ErrorF("    -trace		trace execution of IBM specific functions\n");
#endif
    ErrorF("    -wrap		wrap mouse in both dimensions\n");
    ErrorF("    -wrapx		wrap mouse in X only\n");
    ErrorF("    -wrapy		wrap mouse in Y only\n");
    ErrorF("See Xibm(1) for a more complete description\n");
}

/***==================================================================***/

#ifdef IBM_SPECIAL_MALLOC
#include <stdio.h>
#include <signal.h>

int	ibmShouldDumpArena= 0;
static	char	*ibmArenaFile= 0;

static
ibmMarkDumpArena()
{
    ibmShouldDumpArena= 1;
    return 0;
}

ibmDumpArena()
{
FILE  *mfil;
static	char	fileName[100];
static	int	dumpNum= 0;

   (void) sprintf(fileName,ibmArenaFile,dumpNum++);
   mfil= fopen(fileName,"a");
   if (!mfil) {
	ErrorF("Couldn't open %s to dump arena, ignored\n",fileName);
	return 0 ;
   }
   else {
	ErrorF("Dumping malloc arena to %s\n",fileName);
	plumber(mfil);
	(void) fflush(mfil);
	(void) fclose(mfil);
   }
   ibmShouldDumpArena= 0;
   return 1 ;
}

ibmNoteHit()
{
static int old= 4;

   ErrorF("received SIGTERM\n");
   old= SetMallocCheckLevel(old);
   return 1 ;
}

int
ibmSetupPlumber( name )
register char *name ;
{
extern exit() ;

    ibmInfoMsg( "Setting up plumber to dump to %s\n", name ) ;
    (void) unlink( ibmArenaFile = name ) ;
    (void) signal( SIGUSR1, ibmMarkDumpArena ) ;
#ifdef AIXrt
#else
    (void) signal( SIGEMT, ibmDumpArena ) ;
#endif
    (void) signal( SIGTERM, ibmNoteHit ) ;
    (void) signal( SIGUSR2, exit ) ;
    return 1 ;
}

#endif /* IBM_SPECIAL_MALLOC */

/***==================================================================***/
/* Debugging Aids */
/* print_pattern() & print_event() are just for debugging */
#if defined(DEBUG) && !defined(NDEBUG)

/* these don't work with AIX compiler */
#if !defined(AIXrt)
#define PUT_BIT(d,b)   (((d)&(((unsigned)1)<<(b)))?ErrorF("*"):ErrorF("."))

void
print_pattern(width,height,data)
int	width,height;
char	*data;
{
char	*tmp=data;
int	row,bit;
unsigned data_byte;
int	bits_left;

    TRACE(("print_pattern( width= %d, height= %d, data= 0x%x )\n",
		width,height,data));

    for (row=0;row<height;row++) {
	ErrorF("0x");
	for (bit=0;bit<(width+7)/8;bit++) {
	   ErrorF("%02x",*tmp++);
	}
	ErrorF("\n");
    }
    for (row=0;row<height;row++) {
	for (bits_left=width;bits_left>0;bits_left-=8) {
	    data_byte= *data++;
	    for (bit=7;bit>=(bits_left>8?0:8-bits_left);bit--) {
		PUT_BIT(data_byte,bit);
	    }
	}
	ErrorF("\n");
    }
    return;
}

/***==================================================================***/

#include "OSio.h"

void
print_event(xE)
XEvent	*xE;
{

    TRACE(("print_event( xE= 0x%x )\n",xE));

    ErrorF("Event(%d,%d): ",xE->xe_x,xE->xe_y);
    switch (xE->xe_device) {
	case XE_MOUSE:		ErrorF("mouse "); break;
	case XE_DKB:		ErrorF("keyboard "); break;
	case XE_TABLET:		ErrorF("tablet "); break;
	case XE_AUX:		ErrorF("aux "); break;
	case XE_CONSOLE:	ErrorF("console "); break;
	default:		ErrorF("unknown(%d) ",xE->xe_device); break;
    }
    if (xE->xe_type==XE_BUTTON) {
	ErrorF("button ");
	if	(xE->xe_direction==XE_KBTUP)	ErrorF("up ");
	else if	(xE->xe_direction==XE_KBTDOWN)	ErrorF("down ");
	else if	(xE->xe_direction==XE_KBTRAW)	ErrorF("raw ");
	else 			ErrorF("unknown(%d) ",xE->xe_direction);
	ErrorF("(key= %d)",xE->xe_key);
    }
    else if (xE->xe_type==XE_MMOTION)	ErrorF("MMOTION");
    else if (xE->xe_type==XE_TMOTION)	ErrorF("TMOTION");
    ErrorF("\n");
    return;
}

#endif
#endif /* Debugging Aids */
