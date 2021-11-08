/*
 * xdm - display manager daemon
 *
 * $XConsortium: pseudoreset.c,v 1.2 88/11/17 17:04:58 keith Exp $
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
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * pseudoReset -- pretend to reset the server by killing all clients
 * with windows.  It will reset the server most of the time, unless
 * a client remains connected with no windows.
 */

# include	<X11/Xlib.h>

static int
ignoreErrors (dpy, event)
Display	*dpy;
XErrorEvent	*event;
{
	Debug ("ignoring error\n");
}

/*
 * this is mostly bogus -- but quite useful.  I wish the protocol
 * had some way of enumerating and identifying clients, that way
 * this code wouldn't have to be this kludgy.
 */

static
killWindows (dpy, window)
Display	*dpy;
Window	window;
{
	Window	root, parent, *children;
	int	child, nchildren = 0;
	
	while (XQueryTree (dpy, window, &root, &parent, &children, &nchildren)
	       && nchildren > 0)
	{
		for (child = 0; child < nchildren; child++) {
			Debug ("XKillClient 0x%x\n", children[child]);
			XKillClient (dpy, children[child]);
		}
		XFree (children);
	}
}

/*
 * this display connection better not have any windows...
 */
 
pseudoReset (dpy)
Display	*dpy;
{
	Window	root;
	int	screen;

	XSetErrorHandler (ignoreErrors);
	for (screen = 0; screen < ScreenCount (dpy); screen++) {
		Debug ("pseudoReset screen %d\n", screen);
		root = RootWindow (dpy, screen);
		killWindows (dpy, root);
	}
	Debug ("before XSync\n");
	XSync (dpy, False);
	XSetErrorHandler ((int (*)) 0);
	Debug ("pseudoReset done\n");
}
