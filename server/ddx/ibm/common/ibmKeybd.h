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
#ifndef IBM_KEYBOARD
#define IBM_KEYBOARD 1

/* $Header: /site/forMIT/server/ddx/ibm/common/RCS/ibmKeybd.h,v 9.0 88/10/17 14:55:23 erik Exp Locker: erik $ */
/* $Source: /site/forMIT/server/ddx/ibm/common/RCS/ibmKeybd.h,v $ */
/* "@(#)ibmkeybd.h	3.1 88/09/22 09:31:58" */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char *rcsidrtkeyboard = "$Header: /site/forMIT/server/ddx/ibm/common/RCS/ibmKeybd.h,v 9.0 88/10/17 14:55:23 erik Exp Locker: erik $";
#endif

#define	IBM_LED_NUMLOCK		1
#define IBM_LED_CAPSLOCK	2
#define IBM_LED_SCROLLOCK	4

extern	DevicePtr	ibmKeybd;
extern	int 		ibmUsePCKeys;
extern	int		ibmBellPitch;
extern	int		ibmBellDuration;
extern	int		ibmLEDState;
extern	int		ibmKeyClick;
extern	int		ibmKeyRepeat;
extern	int		ibmLockState;
extern	int		ibmCurLockKey;
extern	int		ibmLockEnabled;

/* defined in OS specific directories */
extern	int		osKeybdProc();
extern	KeySym		ibmmap[];

#endif /* IBM_KEYBOARD */
