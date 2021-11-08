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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Save.c,v 9.1 88/10/17 14:45:40 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Save.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Save.c,v 9.1 88/10/17 14:45:40 erik Exp $";
static char sccsid[] = "@(#)apa16save.c	3.2 88/09/25 10:08:34";
#endif

#include "apa16Hdwr.h"

#include "ibmTrace.h"

void
apa16SaveState()
{
    TRACE(("apa16SaveState()\n"));
    apa16InvalidateFonts();
    apa16InvalidateCursor();
    QUEUE_WAIT();
    return;
}

void
apa16RestoreState()
{
    TRACE(("apa16RestoreState()\n"));
    QUEUE_WAIT();
    WHITE_ON_BLACK();
    apa16ReinitializeFonts();
    apa16ReinitializeCursor();
    return;
}
