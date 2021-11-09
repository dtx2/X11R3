/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 2021 Edward Halferty

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.*/
#ifndef DISPATCH_H
#define DISPATCH_H


#define LEGAL_CLIENT(id, c) (\
    (c == CLIENT_ID(id)) && (clientUsed[CLIENT_ID(id)].used)) 

#define REQUEST_AT_LEAST_SIZE(req) \
    if ((sizeof(req) >> 2) > stuff->length )\
         return(BadLength)

#define REQUEST_SIZE_MATCH(req)\
    if ((sizeof(req) >> 2) != stuff->length)\
         return(BadLength)

extern int ProcBell();
extern int ProcChangeActivePointerGrab();
extern int ProcChangeKeyboardControl();
extern int ProcChangePointerControl();
extern int ProcGetDeviceMapping();
extern int ProcGetInputFocus();
extern int ProcGetKeyboardControl();
extern int ProcGetMotionEvents();
extern int ProcGetPointerControl();
extern int ProcGrabKey();
extern int ProcGrabKeyboard();
extern int ProcGrabPointer();
extern int ProcQueryKeymap();
extern int ProcQueryPointer();
extern int ProcSetDeviceMapping();
extern int ProcSetInputFocus();
extern int ProcSendEvent();
extern int ProcUngrabKey();
extern int ProcUngrabKeyboard();
extern int ProcUngrabPointer();
extern int ProcWarpPointer();

#endif /* DISPATCH_H */

extern int curclient;
extern int ErrorResID;
extern int *RequestSequenceNumber;
