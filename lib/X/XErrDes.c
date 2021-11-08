/*
 * $XConsortium: XErrDes.c,v 11.31 88/09/30 16:19:36 jim Exp $
 */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <X11/Xos.h>
#include "Xlibint.h"
#include "Xresource.h"

#ifndef ERRORDB
#define ERRORDB "/usr/lib/X11/XErrorDB"
#endif

char *XErrorList[] = {
	/* No error	*/	"",
	/* BadRequest	*/	"BadRequest, invalid request code or no such operation",
	/* BadValue	*/	"BadValue, integer parameter out of range for operation",
	/* BadWindow	*/	"BadWindow, invalid Window parameter",
	/* BadPixmap	*/	"BadPixmap, invalid Pixmap parameter",
	/* BadAtom	*/	"BadAtom, invalid Atom parameter",
	/* BadCursor	*/	"BadCursor, invalid Cursor parameter",
	/* BadFont	*/	"BadFont, invalid Font parameter",
	/* BadMatch	*/	"BadMatch, invalid parameter attributes",
	/* BadDrawable	*/	"BadDrawable, invalid Pixmap or Window parameter",
	/* BadAccess	*/	"BadAccess, attempt to access private resource denied", 
	/* BadAlloc	*/	"BadAlloc, insufficient resources for operation",
    	/* BadColor   	*/  	"BadColor, invalid Colormap parameter",
    	/* BadGC   	*/  	"BadGC, invalid GC parameter",
	/* BadIDChoice  */	"BadIDChoice, invalid resource ID chosen for this connection",
	/* BadName	*/	"BadName, named color or font does not exist",
	/* BadLength	*/	"BadLength, poly request too large or internal Xlib length error",
	/* BadImplementation */	"BadImplementation, server does not implement operation",
};
int XErrorListSize = sizeof(XErrorList);


XGetErrorText(dpy, code, buffer, nbytes)
    register int code;
    register Display *dpy;
    char *buffer;
    int nbytes;
{

    char *defaultp = NULL;
    char buf[32];
    register _XExtension *ext;

    sprintf(buf, "%d", code);
    if (code <= (XErrorListSize/ sizeof (char *)) && code > 0) {
	defaultp =  XErrorList[code];
	XGetErrorDatabaseText(dpy, "XProtoError", buf, defaultp, buffer, nbytes);
	}
    ext = dpy->ext_procs;
    while (ext) {		/* call out to any extensions interested */
 	if (ext->error_string != NULL) 
 	    (*ext->error_string)(dpy, code, &ext->codes, buffer, nbytes);
 	ext = ext->next;
    }    
    return;
}

XGetErrorDatabaseText(dpy, name, type, defaultp, buffer, nbytes)
    register char *name, *type;
    char *defaultp;
    Display *dpy;
    char *buffer;
    int nbytes;
{

    static XrmDatabase db;
    XrmString type_str;
    XrmValue result;
    static int initialized = False;
    char temp[BUFSIZ];

    if (initialized == False) {
	_XInitErrorHandling (&db);
	initialized = True;
    }
    sprintf(temp, "%s.%s", name, type);
    XrmGetResource(db, temp, "ErrorType.ErrorNumber", &type_str, &result);
    if (result.addr) {
	(void) strncpy (buffer, result.addr, nbytes);
	if (result.size < nbytes) buffer[result.size] = 0;
    } else (void) strncpy(buffer, defaultp, nbytes);
}

_XInitErrorHandling (db)
    XrmDatabase *db;
    {
    XrmDatabase errordb;
    XrmInitialize();
    errordb = XrmGetFileDatabase(ERRORDB);
    XrmMergeDatabases(errordb, db);
     }

