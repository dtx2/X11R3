#include "copyright.h"

/* $XConsortium: XDrArcs.c,v 11.12 88/09/06 16:06:36 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDrawArcs(dpy, d, gc, arcs, n_arcs)
register Display *dpy;
Drawable d;
GC gc;
XArc *arcs;
int n_arcs;
{
    register xPolyArcReq *req;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq(PolyArc,req);
    req->drawable = d;
    req->gc = gc->gid;

    /* SIZEOF(xArc) will be a multiple of 4 */
    req->length += n_arcs * (SIZEOF(xArc) / 4);
    
    n_arcs *= SIZEOF(xArc);

    Data16 (dpy, (short *) arcs, n_arcs);
    UnlockDisplay(dpy);
    SyncHandle();
}
