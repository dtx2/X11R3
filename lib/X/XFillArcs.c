#include "copyright.h"

/* $XConsortium: XFillArcs.c,v 11.12 88/09/06 16:07:08 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XFillArcs(dpy, d, gc, arcs, n_arcs)
register Display *dpy;
Drawable d;
GC gc;
XArc *arcs;
int n_arcs;
{
    register xPolyFillArcReq *req;
    register long nbytes;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq(PolyFillArc, req);
    req->drawable = d;
    req->gc = gc->gid;

    /* SIZEOF(xArc) will be a multiple of 4 */
    req->length += n_arcs * (SIZEOF(xArc) / 4);
    
    nbytes = n_arcs * SIZEOF(xArc);

    Data16 (dpy, (short *) arcs, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}
