#include "copyright.h"

/* $XConsortium: XQuTextExt.c,v 11.12 88/09/06 16:10:03 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986, 1987	*/

#include "Xlibint.h"

XQueryTextExtents (dpy, fid, string, nchars, dir, font_ascent, font_descent,
                   overall)
    register Display *dpy;
    Font fid;
    register char *string;
    register int nchars;
    int *dir;
    int *font_ascent, *font_descent;
    register XCharStruct *overall;
{
    register int i;
    register char *ptr;
    char *buf;
    xQueryTextExtentsReply rep;
    long nbytes;
    register xQueryTextExtentsReq *req;

    LockDisplay(dpy);
    GetReq(QueryTextExtents, req);
    req->fid = fid;
    nbytes = nchars << 1;
    req->length += (nbytes + 3)>>2;
    req->oddLength = nchars & 1;
    buf = _XAllocScratch (dpy, (unsigned long)nbytes);
    for (ptr = buf, i = nchars; --i >= 0;) {
	*ptr++ = 0;
	*ptr++ = *string++;
    }
    Data (dpy, buf, nbytes);
    if (!_XReply (dpy, (xReply *)&rep, 0, xTrue))
	return (0);
    *dir = rep.drawDirection;
    *font_ascent = cvtINT16toInt (rep.fontAscent);
    *font_descent = cvtINT16toInt (rep.fontDescent);
    overall->ascent = (short) cvtINT16toInt (rep.overallAscent);
    overall->descent = (short) cvtINT16toInt (rep.overallDescent);
    /* XXX bogus - we're throwing away information!!! */
    overall->width  = (short) cvtINT32toInt (rep.overallWidth);
    overall->lbearing = (short) cvtINT32toInt (rep.overallLeft);
    overall->rbearing = (short) cvtINT32toInt (rep.overallRight);
    UnlockDisplay(dpy);
    SyncHandle();
    return (1);
}
