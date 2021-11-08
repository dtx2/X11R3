#include "copyright.h"

/* $XConsortium: XGetAtomNm.c,v 11.14 88/09/06 16:07:41 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/
#define NEED_REPLIES
#include "Xlibint.h"

char *XGetAtomName(dpy, atom)
register Display *dpy;
Atom atom;
{
    xGetAtomNameReply rep;
    xResourceReq *req;
    char *storage;

    LockDisplay(dpy);
    GetResReq(GetAtomName, atom, req);

    if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) {
	UnlockDisplay(dpy);
	SyncHandle();
	return(NULL);
	}

    storage = (char *) Xmalloc(rep.nameLength+1);

    _XReadPad(dpy, storage, (long)rep.nameLength);
    storage[rep.nameLength] = '\0';

    UnlockDisplay(dpy);
    SyncHandle();
    return(storage);
}
