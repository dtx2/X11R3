#include "copyright.h"

/* $XConsortium: XIntAtom.c,v 11.12 88/09/06 16:08:47 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

Atom XInternAtom (dpy, name, onlyIfExists)
    register Display *dpy;
    char *name;
    Bool onlyIfExists;
{
    register long nbytes;
    xInternAtomReply rep;
    register xInternAtomReq *req;
    LockDisplay(dpy);
    GetReq(InternAtom, req);
    nbytes = req->nbytes = name ? strlen(name) : 0;
    req->onlyIfExists = onlyIfExists;
    req->length += (nbytes+3)>>2;
    _XSend (dpy, name, nbytes);
    	/* use _XSend instead of Data, since the following _XReply
           will always flush the buffer anyway */
    if(_XReply (dpy, (xReply *)&rep, 0, xTrue) == 0) rep.atom = None;
    UnlockDisplay(dpy);
    SyncHandle();
    return (rep.atom);
}

