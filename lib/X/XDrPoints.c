#include "copyright.h"

/* $XConsortium: XDrPoints.c,v 1.11 88/09/06 16:06:53 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDrawPoints(dpy, d, gc, points, n_points, mode)
    register Display *dpy;
    Drawable d;
    GC gc;
    XPoint *points;
    int n_points;
    int mode; /* CoordMode */
{
    register xPolyPointReq *req;
    register long nbytes;
    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq(PolyPoint, req);
    req->drawable = d;
    req->gc = gc->gid;
    req->coordMode = mode;

    /* on the VAX, each point is 2 16-bit integers */
    req->length += n_points;

    nbytes = (long)n_points << 2;	/* watch out for macros... */
    Data16 (dpy, (short *) points, nbytes);

    UnlockDisplay(dpy);
    SyncHandle();
}
