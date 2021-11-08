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

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Info.c,v 9.1 88/10/17 14:44:35 erik Exp $";
static char sccsid[] = "@(#)apa16Info.c	3.1 88/09/22 09:30:15";
#endif

#include "X.h"
#include "misc.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "ibmScreen.h"

extern	Bool	apa16ScreenInit();
extern	Bool	apa16Probe();
extern	void	apa16HideCursor();
extern	void	apa16SaveState();
extern	void	apa16RestoreState();

ibmPerScreenInfo apa16ScreenInfoStruct = {
	{ 0, 0, 1023, 767 },
	0,
	NULL,
	apa16ScreenInit,
	apa16Probe,
	apa16HideCursor,
	"-apa16",
	"/dev/apa16",
	"/dev/msapa16",
	apa16SaveState,
	apa16RestoreState
} ;
