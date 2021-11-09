#include "copyright.h"

/* $XConsortium: XGrPointer.c,v 11.16 88/09/06 16:08:35 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

int XGrabPointer(dpy, grab_window, owner_events, event_mask, pointer_mode,
	     keyboard_mode, confine_to, curs, time)
register Display *dpy;
Window grab_window;
Bool owner_events;
unsigned int event_mask; /* CARD16 */
int pointer_mode, keyboard_mode;
Window confine_to;
Cursor curs;
Time time;
{
    xGrabPointerReply rep;
    register xGrabPointerReq *req;
    register int status;
    LockDisplay(dpy);
    GetReq(GrabPointer, req);
    req->grabWindow = grab_window;
    req->ownerEvents = owner_events;
    req->eventMask = event_mask;
    req->pointerMode = pointer_mode;
    req->keyboardMode = keyboard_mode;
    req->confineTo = confine_to;
    req->cursor = curs;
    req->time = time;
    
    /* if we ever return, suppress the error */
    if (_XReply (dpy, (xReply *) &rep, 0, xTrue) == 0)
	rep.status = GrabSuccess;
    status = rep.status;
    UnlockDisplay(dpy);
    SyncHandle();
    return (status);
}
