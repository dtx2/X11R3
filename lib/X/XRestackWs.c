#include "copyright.h"

/* $XConsortium: XRestackWs.c,v 1.9 88/09/06 16:11:37 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XRestackWindows (dpy, windows, n)
    register Display *dpy;
    register Window *windows;
    int n;
    {
    int i = 0;
    unsigned long val = Below;		/* needed for macro below */

    LockDisplay(dpy);
    while (windows++, ++i < n) {
	register xConfigureWindowReq *req;

    	GetReqExtra (ConfigureWindow, 8, req);
	req->window = *windows;
	req->mask = CWSibling | CWStackMode;
#ifdef MUSTCOPY
	dpy->bufptr -= 8;
	Data32 (dpy, (long *)(windows-1), 4);
	Data32 (dpy, (long *)&val, 4);
#else
	{
	    register unsigned long *values = (unsigned long *)
	      NEXTPTR(req,xConfigureWindowReq);
	    *values++ = *(windows-1);
	    *values   = Below;
	}
#endif /* MUSTCOPY */
	}
    UnlockDisplay(dpy);
    SyncHandle();
    }

    

    
