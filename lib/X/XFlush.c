#include "copyright.h"

/* $XConsortium: XFlush.c,v 11.6 88/09/06 16:07:16 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

/* Flush all buffered output requests. */
/* NOTE: NOT necessary when calling any of the Xlib routines. */

XFlush (dpy)
    register Display *dpy;
    {
    _XFlush (dpy);
    }
