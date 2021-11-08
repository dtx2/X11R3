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
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelInfo.c,v 1.2 88/10/25 01:51:21 kbg Exp $";
#endif

#include "X.h"
#include "misc.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "ibmScreen.h"

extern	Bool	mpelScreenInit();
extern	Bool	mpelProbe();
extern	void	mpelRemoveCursor();
extern	PixmapFormatRec	mpelFormats[];
extern void  mpelSaveState();
extern void  mpelRestoreState(); 


ibmPerScreenInfo mpelScreenInfoStruct = {
	{ 0, 0, 1023, 1023 },
	1,
	mpelFormats,
	mpelScreenInit,
	mpelProbe,
	mpelRemoveCursor,
	"-mpel",
	"/dev/mpel",
	"/dev/msmpel",
	mpelSaveState,
	mpelRestoreState
} ;
