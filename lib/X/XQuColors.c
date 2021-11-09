#include "copyright.h"

/* $XConsortium: XQuColors.c,v 11.17 88/09/06 16:09:54 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XQueryColors(dpy, cmap, defs, ncolors)
    register Display *dpy;
    Colormap cmap;
    XColor defs[]; 		/* RETURN */
    int ncolors;
{
    register int i;
    xrgb *color;
    xQueryColorsReply rep;
    long nbytes;
    register xQueryColorsReq *req;

    LockDisplay(dpy);
    GetReq(QueryColors, req);

    req->cmap = cmap;

    req->length += ncolors; /* each pixel is a CARD32 */

    for (i = 0; i < ncolors; i++)
      Data32 (dpy, (long *)&defs[i].pixel, 4L);
       /* XXX this isn't very efficient */

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) != 0) {

	    color = (xrgb *) Xmalloc(
		(unsigned)(nbytes = (long)ncolors * SIZEOF(xrgb)));
	    _XRead(dpy, (char *) color, nbytes);

	    for (i = 0; i < ncolors; i++) {
		register XColor *def = &defs[i];
		register xrgb *rgb = &color[i];
		def->red = rgb->red;
		def->green = rgb->green;
		def->blue = rgb->blue;
		def->flags = DoRed | DoGreen | DoBlue;
	    }

	    Xfree((char *)color);
	}
    UnlockDisplay(dpy);

    SyncHandle();
}

