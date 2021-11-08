/*
 * $XConsortium: XGetDflt.c,v 1.14 88/09/19 13:55:55 jim Exp $
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

#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <X11/Xos.h>
#include "Xlibint.h"
#include <Xresource.h>

static char *GetHomeDir (dest)
	char *dest;
{
	int uid;
	extern char *getenv();
	extern int getuid();
	extern struct passwd *getpwuid(), *getpwnam();
	struct passwd *pw;
	register char *ptr;

	if((ptr = getenv("HOME")) != NULL) {
		(void) strcpy(dest, ptr);

	} else {
		if((ptr = getenv("USER")) != NULL) {
			pw = getpwnam(ptr);
		} else {
			uid = getuid();
			pw = getpwuid(uid);
		}
		if (pw) {
			(void) strcpy(dest, pw->pw_dir);
		} else {
		        *dest = '\0';
		}
	}
	return dest;
}


static XrmDatabase InitDefaults (dpy)
    Display *dpy;			/* display for defaults.... */
{
    XrmDatabase userdb = NULL;
    XrmDatabase xdb = NULL;
    char fname[BUFSIZ];                 /* longer than any conceivable size */
    char *getenv();
    char *xenv;

    XrmInitialize();

    /*
     * See lib/Xtk/Initialize.c
     *
     * First, get the defaults from the server; if none, then load from
     * ~/.Xdefaults.  Next, if there is an XENVIRONMENT environment variable,
     * then load that file.
     */

    if (dpy->xdefaults == NULL) {
	fname[0] = '\0';
	(void) GetHomeDir (fname);
	(void) strcat (fname, "/.Xdefaults");
	xdb = XrmGetFileDatabase (fname);
    } else {
	xdb = XrmGetStringDatabase(dpy->xdefaults);
    }

    if ((xenv = getenv ("XENVIRONMENT")) == NULL) {
	int len;
	fname[0] = '\0';
	(void) GetHomeDir (fname);
	(void) strcat (fname, "/.Xdefaults-");
	len = strlen (fname);
	gethostname (fname+len, BUFSIZ-len);
	xenv = fname;
    }
    userdb = XrmGetFileDatabase (xenv);
    XrmMergeDatabases (userdb, &xdb);
    return (xdb);

#ifdef old
    if (fname[0] != '\0') userdb =  XrmGetFileDatabase(fname);
    xdb = XrmGetStringDatabase(dpy->xdefaults);
    XrmMergeDatabases(userdb, &xdb);
    return xdb;
#endif
}

char *XGetDefault(dpy, prog, name)
	Display *dpy;			/* display for defaults.... */
	register char *name;		/* name of option program wants */
	char *prog;			/* name of program for option	*/

{					/* to get, for example, "font"  */
	char temp[BUFSIZ];
	XrmString type;
	XrmValue result;
	char *progname;

	/*
	 * strip path off of program name
	 */
	progname = rindex (prog, '/');
	if (progname)
	    progname++;
	else
	    progname = prog;

	/*
	 * see if database has ever been initialized.  Lookups can be done
	 * without locks held.
	 */
	LockDisplay(dpy);
	if (dpy->db == NULL) {
		dpy->db = InitDefaults(dpy);
		}
	UnlockDisplay(dpy);

	sprintf(temp, "%s.%s", progname, name);
	XrmGetResource(dpy->db, temp, "Program.Name", &type, &result);

	return (result.addr);
}

