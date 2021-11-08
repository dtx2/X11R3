static char rcsid[] =
	"$XConsortium: Lookup.c,v 1.3 88/10/13 08:44:36 rws Exp $";

/* 
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

/* XXX
 * These routines are probably a bit buggy.  The goal is that they
 * actually return a a standaard character set.  It is likely that in
 * some cases the least significant byte of the keysym does not match
 * the standard character set representation.  Also, there are probably
 * characters in Latin<N-1> that belong in Latin<N>.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* return 7-bit ASCII plus least significant byte from specified keysym set */
int XmuLookupString (event, buffer, nbytes, keysym, status, keysymSet)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
    unsigned long keysymSet;
{
    int count;
    KeySym symbol;

    count = XLookupString(event, buffer, nbytes, &symbol, status);
    if (keysym) *keysym = symbol;
    if ((count == 0) && (nbytes > 0) && (symbol != NoSymbol) &&
	((symbol >> 8) == keysymSet)) {
	buffer[0] = (symbol & 0xff);
	count = 1;
    } else if ((keysymSet != 0) && (count == 1) &&
	       (((unsigned char *)buffer)[0] == symbol) &&
	       (symbol & 0x80)) {
	count = 0;
    }
    return count;
}

/* the following return 7-bit ASCII plus specified character set */

int XmuLookupLatin1 (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XLookupString(event, buffer, nbytes, keysym, status);
}

int XmuLookupLatin2 (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 1);
}

int XmuLookupLatin3 (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 2);
}

int XmuLookupLatin4 (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 3);
}

int XmuLookupKana (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 4);
}

int XmuLookupArabic (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 5);
}

int XmuLookupCyrillic (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 6);
}

int XmuLookupGreek (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 7);
}

int XmuLookupAPL (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 11);
}

int XmuLookupHebrew (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;
    int nbytes;
    KeySym *keysym;
    XComposeStatus *status;
{
    return XmuLookupString(event, buffer, nbytes, keysym, status, 12);
}
