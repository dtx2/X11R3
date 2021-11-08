#ifndef OS_IOH
#define OS_IOH 1

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
/* $Header: /site/forMIT/server/ddx/ibm/BSDrt/RCS/OSio.h,v 9.2 88/10/17 14:35:25 erik Exp Locker: erik $ */
/* $Source: /site/forMIT/server/ddx/ibm/BSDrt/RCS/OSio.h,v $ */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char *rcsidOSio = "$Header: /site/forMIT/server/ddx/ibm/BSDrt/RCS/OSio.h,v 9.2 88/10/17 14:35:25 erik Exp Locker: erik $";
#endif

#include "bsdIO.h"
#define	CURRENT_X()	(BSDXaddr->mouse.x-ibmScreenMinX(ibmCurrentScreen))
#define	CURRENT_Y()	(BSDXaddr->mouse.y-ibmScreenMinY(ibmCurrentScreen))

extern	void	NoopDDA();

#define	OS_BlockHandler	NoopDDA
#define OS_WakeupHandler NoopDDA

#define	OS_MouseProc	BSDMouseProc
#define	OS_KeybdProc	BSDKeybdProc

#define	OS_CapsLockFeedback(dir)	BSDCapsLockFeedback(dir)

#define	OS_PreScreenInit()	BSDMachineDependentInit()
#define	OS_PostScreenInit()	BSDInitEmulator()
#define	OS_ScreenStateChange(e)	BSDScreenStateChange(e)

#define	OS_GetDefaultScreens()
#define	OS_InitInput()
#define	OS_AddAndRegisterOtherDevices()
#define	OS_SaveState()
#define	OS_RestoreState()
#endif /* ndef OS_IOH */
