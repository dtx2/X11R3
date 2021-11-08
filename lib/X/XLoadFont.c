#include "copyright.h"

/* $XConsortium: XLoadFont.c,v 11.8 88/09/06 16:09:01 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

Font XLoadFont (dpy, name)
    register Display *dpy;
    char *name;
{
    register long nbytes;
    Font fid;
    register xOpenFontReq *req;
    LockDisplay(dpy);
    GetReq(OpenFont, req);
    nbytes = req->nbytes = name ? strlen(name) : 0;
    req->fid = fid = XAllocID(dpy);
    req->length += (nbytes+3)>>2;
    Data (dpy, name, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
    return (fid); 
       /* can't return (req->fid) since request may have already been sent */
}

