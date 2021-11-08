#include "copyright.h"

/* $XConsortium: XLiICmaps.c,v 11.15 88/09/06 16:08:56 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

Colormap *XListInstalledColormaps(dpy, win, n)
register Display *dpy;
Window win;
int *n;  /* RETURN */
{
    long nbytes;
    Colormap *cmaps;
    xListInstalledColormapsReply rep;
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(ListInstalledColormaps, win, req);

    if(_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    *n = 0;
	    return((Colormap *)None);
	}
	

    *n = rep.nColormaps;
    nbytes = rep.nColormaps * sizeof(Colormap);
    cmaps = (Colormap *) Xmalloc(nbytes);
    nbytes = rep.nColormaps * 4;
    _XRead32 (dpy, (char *) cmaps, nbytes);

    UnlockDisplay(dpy);
    SyncHandle();
    return(cmaps);
}

