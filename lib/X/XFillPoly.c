#include "copyright.h"

/* $XConsortium: XFillPoly.c,v 11.10 88/09/06 16:07:10 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XFillPolygon(dpy, d, gc, points, n_points, shape, mode)
register Display *dpy;
Drawable d;
GC gc;
XPoint *points;
int n_points;
int shape;
int mode;
{
    register xFillPolyReq *req;
    register long nbytes;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq(FillPoly, req);

    req->drawable = d;
    req->gc = gc->gid;
    req->shape = shape;
    req->coordMode = mode;

    /* on the VAX, each point is 2 16-bit ints = 4 bytes */
    req->length += n_points;

    /* shift (mult. by 4) before passing to the (possible) macro */

    nbytes = n_points << 2;
    
    Data16 (dpy, (short *) points, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}
