/* $XConsortium: extension.h,v 1.4 88/09/06 15:48:25 jim Exp $ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

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
SOFTWARE.

******************************************************************/
#ifndef EXTENSION_H
#define EXTENSION_H 

/****************************************************************
 *  extension.h: static bindings for extending X protocol.  
 *
 *  Each protocol extension is encapsulated in the following record:
 *  
 *      typedef struct _extension_entry {
 *          char *name;                  
 *          int base;                   
 *          void (* InitProc)();          
 *          int (* MainProc)();          
 *      } EXTENSION_ENTRY;
 *  
 *  When Dispatch() gets a request type > 128, it calls
 *  XDispatchToExtension. This routine looks in the extension table for the
 *  the (MainProc) to call.  It gets one parameter, char *request (the
 *  string returned from ReadRequestFromClient).  MainProc() should return 
 *  a status flag to the disptacher.  In mai n(), the init procedure
 *  for each loaded extension is called.  
 *  
 *  Extension procedures to call when a GC is created and destroy are
 *  defined in gc.h (in the include directory)
 *  
 ****************************************************************/

#define GetGCAndDrawableAndValidate(gcID, pGC, drawID, pDraw, client)\
    if ((client->lastDrawableID != drawID) || (client->lastGCID != gcID))\
    {\
        if (client->lastDrawableID != drawID)\
    	    pDraw = (DrawablePtr)LookupID(drawID, RT_DRAWABLE, RC_CORE);\
        else\
	    pDraw = client->lastDrawable;\
        if (client->lastGCID != gcID)\
	    pGC = (GC *)LookupID(gcID, RT_GC, RC_CORE);\
        else\
            pGC = client->lastGC;\
	if (pDraw && pGC)\
	{\
	    if ((pDraw->type == UNDRAWABLE_WINDOW) ||\
		(pGC->depth != pDraw->depth) ||\
		(pGC->pScreen != pDraw->pScreen))\
		return (BadMatch);\
	    client->lastDrawable = pDraw;\
	    client->lastDrawableID = drawID;\
            client->lastGC = pGC;\
            client->lastGCID = gcID;\
	}\
    }\
    else\
    {\
        pGC = client->lastGC;\
        pDraw = client->lastDrawable;\
    }\
    if (!pDraw)\
    {\
        client->errorValue = drawID; \
	return (BadDrawable);\
    }\
    if (!pGC)\
    {\
        client->errorValue = gcID;\
        return (BadGC);\
    }\
    if (pGC->serialNumber != pDraw->serialNumber)\
	ValidateGC(pDraw, pGC);
#endif /* EXTENSION_H */
