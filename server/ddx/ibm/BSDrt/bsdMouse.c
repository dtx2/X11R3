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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdMouse.c,v 9.1 88/10/17 14:35:46 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdMouse.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/BSDrt/RCS/bsdMouse.c,v 9.1 88/10/17 14:35:46 erik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/tbioctl.h>
#include <machineio/mouseio.h>

#include "X.h"
#include "Xproto.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "input.h"
#include "cursorstr.h"

#include "bsdIO.h"
#include "ibmMouse.h"
#include "ibmScreen.h"

#include "ibmTrace.h"

extern	char	*getenv();

DevicePtr	ibmPtr;

/***================================================================***/

/*
 * Initialize mouse 
 */

static
BSDMouseInit(MouseName)
char	*MouseName;
{
	char *MouseType = getenv("MOUSETYPE");
	struct sgttyb MouseSG;
	int ldisc, ioarg;
	int mdev = -1;

	TRACE(( "BSDMouseInit(%s)\n",MouseName));

	/*
	 * Open mouse 
	 */

	if ((mdev = open (MouseName, O_RDWR|O_NDELAY)) < 0) {
		fprintf  (stderr, "Error in open of (%s)\n", MouseName);
		fflush(stderr);
		exit (2);
	}

	/*
	 * Switch to mouse line discipline
	 */

	ldisc = TABLDISC;
	ioctl(mdev, TIOCSETD, (caddr_t) &ldisc);

	/*
	 * Check for Mouse Systems Mouse
	 */

	if (MouseType && (strcmp (MouseType, "MSCMOUSE") == 0)) {
		MouseSG.sg_ispeed = 9;
		MouseSG.sg_ospeed = 9;
		MouseSG.sg_erase = -1;
		MouseSG.sg_kill = -1;
		MouseSG.sg_flags = RAW | ANYP;
		ioctl(mdev, TIOCSETP, (caddr_t) &MouseSG);

		/*
		 * Set to MSCmouse emulation
		 */

		ldisc = PCMS_DISC;
		if (ioctl (mdev, TBIOSETD, (caddr_t) &ldisc) < 0)
		    ErrorF("Error in setting PCMS_DISC Line Discipline");

	} else {	/* Assume the Planar Mouse */

		/*
		 * Use 3 button emulation
		 */

		ldisc = PLANMS_DISC3;
		if (ioctl (mdev, TBIOSETD, (caddr_t) &ldisc) < 0)
		   ErrorF("Error in setting PLANMS_DISC3 Line Discipline");

		/*
		 * Set default mouse sample rate
		 */

		ioarg = MS_RATE_40;
		if (ioctl (mdev, MSIC_SAMP, (caddr_t) &ioarg) < 0)
			ErrorF("Error in setting mouse sample rate");

		/*
		 * Set default mouse resolution
		 */

		ioarg = MS_RES_200;
		if (ioctl (mdev, MSIC_RESL, (caddr_t) &ioarg) < 0)
			ErrorF("Error in setting mouse resolution");
	}
	 
}

/***================================================================***/

static void
BSDChangePointerControl(pDevice,pCtrl)
DevicePtr	pDevice;
PtrCtrl		*pCtrl;
{
int	scale;

    TRACE(("BSDChangePointerControl( pDevice= 0x%x, pCtrl= 0x%x )\n",
						pDevice,pCtrl));
    scale= pCtrl->num/pCtrl->den;
    if (scale<1) scale= 1;
    BSDXaddr->mscale= scale;
    BSDXaddr->mthreshold= pCtrl->threshold;
    return;
}

static int
BSDGetMotionEvents(buff, start, stop)
    CARD32 start, stop;
    xTimecoord *buff;
{
    TRACE(("BSDGetMotionEvents( buff= 0x%x, start= %d, stop= %d )\n",
							buff,start,stop));
    return 0;
}

/***================================================================***/

int
BSDMouseProc(pDev, onoff)
    DevicePtr	pDev;
    int onoff;
{
    static int been_here= FALSE;
    BYTE map[4];

    TRACE(("BSDMouseProc( pDev= 0x%x, onoff= 0x%x )\n",pDev, onoff ));

    switch (onoff)
    {
    case DEVICE_INIT: 
	    ibmPtr = pDev;
	    pDev->devicePrivate = (pointer) &ibmQueue;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    if (!been_here) {
	       BSDMouseInit(ibmScreenPointer(0));
	       been_here= TRUE;
	    }
	    InitPointerDeviceStruct(
		ibmPtr, map, 3, BSDGetMotionEvents, BSDChangePointerControl );
	    SetInputCheck( &ibmQueue->head, &ibmQueue->tail);
	    break;
    case DEVICE_ON:
	    pDev->on = TRUE;
	    AddEnabledDevice(BSDEmulatorFD);
	    break;
    case DEVICE_OFF:
	pDev->on = FALSE;
	break;
    case DEVICE_CLOSE:
	break;
    }
    return Success;
}
