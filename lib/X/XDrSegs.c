#include "copyright.h"

/* $XConsortium: XDrSegs.c,v 11.11 88/09/06 16:06:57 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDrawSegments (dpy, d, gc, segments, nsegments)
    register Display *dpy;
    Drawable d;
    GC gc;
    XSegment *segments;
    int nsegments;
{
    register xPolySegmentReq *req;
    long nbytes;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq (PolySegment, req);
    req->drawable = d;
    req->gc = gc->gid;
    req->length += nsegments<<1;
       /* each segment is 4 16-bit integers, i.e. 2*32 bits */
    nbytes = nsegments << 3;		/* watch out for macros... */
    Data16 (dpy, (short *) segments, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}

