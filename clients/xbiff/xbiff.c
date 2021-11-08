/*
 * $XConsortium: xbiff.c,v 1.7 88/09/26 18:49:32 jim Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "Mailbox.h"

extern void exit();

char *ProgramName;

static XrmOptionDescRec options[] = {
{ "-update", "*mailbox.update", XrmoptionSepArg, (caddr_t) NULL },
{ "-file",   "*mailbox.file", XrmoptionSepArg, (caddr_t) NULL },
{ "-volume", "*mailbox.volume", XrmoptionSepArg, (caddr_t) NULL },
};

static void Usage ()
{
    static char *help_message[] = {
"where options include:",
"    -display host:dpy              X server to contact",
"    -geometry geom                 size of mailbox",
"    -file file                     file to watch",
"    -update seconds                how often to check for mail",
"    -volume percentage             how load to ring the bell",
"    -bg color                      background color",
"    -fg color                      foreground color",
"    -rv                            reverse video",
NULL};
    char **cpp;

    fprintf (stderr, "usage:  %s [-options ...]\n", ProgramName);
    for (cpp = help_message; *cpp; cpp++) {
	fprintf (stderr, "%s\n", *cpp);
    }
    fprintf (stderr, "\n");
    exit (1);
}


void main (argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel, w;

    ProgramName = argv[0];

    toplevel = XtInitialize ("main", "XBiff", options, XtNumber (options),
			     &argc, argv);
    if (argc != 1) Usage ();

    w = XtCreateManagedWidget ("mailbox", mailboxWidgetClass, toplevel,
			       NULL, 0);
    XtRealizeWidget (toplevel);
    XtMainLoop ();
}

