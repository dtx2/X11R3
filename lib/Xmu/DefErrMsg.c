/*
 * $XConsortium: DefErrMsg.c,v 1.2 88/10/10 14:34:48 jim Exp $
 *
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

#include <X11/Xlib.h>
#include <stdio.h>

/*
 * XmuPrintDefaultErrorMessage - print a nice error that looks like the usual 
 * message.  Returns 1 if the caller should consider exitting else 0.
 */

int XmuPrintDefaultErrorMessage (dpy, event, fp)
    Display *dpy;
    XErrorEvent *event;
    FILE *fp;
{
    char buffer[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    char *mtype = "XlibMessage";
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    (void) fprintf(fp, "%s:  %s\n  ", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", 
	mesg, BUFSIZ);
    (void) fprintf(fp, mesg, event->request_code);
    sprintf(number, "%d", event->request_code);
    XGetErrorDatabaseText(dpy, "XRequest", number, "", 	buffer, BUFSIZ);
    (void) fprintf(fp, " (%s)", buffer);
    fputs("\n  ", fp);
    XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code", 
	mesg, BUFSIZ);
    (void) fprintf(fp, mesg, event->minor_code);
    fputs("\n  ", fp);
    XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
	mesg, BUFSIZ);
    (void) fprintf(fp, mesg, event->resourceid);
    fputs("\n  ", fp);
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", 
	mesg, BUFSIZ);
    (void) fprintf(fp, mesg, event->serial);
    fputs("\n  ", fp);
    XGetErrorDatabaseText(dpy, mtype, "CurrentSerial", "Current Serial #%d",
	mesg, BUFSIZ);
    (void) fprintf(fp, mesg, NextRequest(dpy)-1);
    fputs("\n", fp);
    if (event->error_code == BadImplementation) return 0;
    return 1;
}

