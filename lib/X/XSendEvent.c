#include "copyright.h"
/* $XConsortium: XSendEvent.c,v 11.10 88/09/06 16:10:01 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/
#include "Xlibint.h"
extern Status _XEventToWire();
// In order to avoid all images requiring _XEventToWire, we install the event converter here if it
// has never been installed.
Status XSendEvent(Display *dpy, Window w, Bool propagate, long event_mask, XEvent *event) {
    register xSendEventReq *req;
    xEvent ev;
    register Status (**fp)();
    Status status;
    LockDisplay (dpy);
    // call through display to find proper conversion routine
    fp = &dpy->wire_vec[event->type & 0177];
    if (*fp == NULL) *fp = _XEventToWire;
    status = (**fp)(dpy, event, &ev);
    if (status) {
        GetReq(SendEvent, req);
        req->destination = w;
        req->propagate = propagate;
        req->eventMask = event_mask;
        req->event = ev;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return(status);
}
