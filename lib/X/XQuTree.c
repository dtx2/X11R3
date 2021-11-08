#include "copyright.h"

/* $XConsortium: XQuTree.c,v 11.17 88/09/06 16:04:02 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

Status XQueryTree (dpy, w, root, parent, children, nchildren)
    register Display *dpy;
    Window w;
    Window *root;
    Window *parent;  /* RETURN */
    Window **children; /* RETURN */
    unsigned int *nchildren;  /* RETURN */
{
    long nbytes;
    xQueryTreeReply rep;
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(QueryTree, w, req);
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return (0);
	}
    *parent = rep.parent;
    *root = rep.root;
    *nchildren = rep.nChildren;
    *children = (Window *) NULL;
    if (rep.nChildren != 0) {
	nbytes = rep.nChildren * sizeof(Window);
	*children = (Window *) Xmalloc (nbytes);
	nbytes = rep.nChildren * 4;
	_XRead32 (dpy, (char *) *children, nbytes);
    }
       /* Note: won't work if sizeof(Window) is not 32 bits! */
    UnlockDisplay(dpy);
    SyncHandle();
    return (1);
}

