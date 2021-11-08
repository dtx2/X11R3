#include "copyright.h"

/* $XConsortium: XLiProps.c,v 11.18 88/09/06 16:08:58 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

Atom *XListProperties(dpy, window, n_props)
register Display *dpy;
Window window;
int *n_props;  /* RETURN */
{
    long nbytes;
    xListPropertiesReply rep;
    Atom *properties;
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(ListProperties, window, req);
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	*n_props = 0;
	UnlockDisplay(dpy);
        SyncHandle();
	return (NULL);
	}

    *n_props = rep.nProperties;
    nbytes = rep.nProperties * sizeof(Atom);
    properties = (Atom *) Xmalloc (nbytes);
    nbytes = rep.nProperties * 4;
    _XRead32 (dpy, (char *) properties, nbytes);

    UnlockDisplay(dpy);
    SyncHandle();
    return (properties);
}
