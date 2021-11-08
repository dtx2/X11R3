/* $XConsortium: dispatch.c,v 1.68 88/10/11 20:49:46 rws Exp $ */
/************************************************************
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

********************************************************/

#include "X.h"
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xproto.h"
#include "windowstr.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "gcstruct.h"
#include "osstruct.h"
#include "selection.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "opaque.h"
#include "input.h"
#include "servermd.h"

extern WindowRec WindowTable[];
extern xConnSetupPrefix connSetupPrefix;
extern char *ConnectionInfo;
extern void ProcessInputEvents();
extern void ValidateGC();
extern Atom MakeAtom();
extern char *NameForAtom();
extern void SaveScreens();
extern void ReleaseActiveGrabs();
extern void QueryFont();
extern void NotImplemented();
extern WindowPtr RealChildHead();

Selection *CurrentSelections;
int NumCurrentSelections;

extern long ScreenSaverTime;
extern long ScreenSaverInterval;
extern int  ScreenSaverBlanking;
extern int  ScreenSaverAllowExposures;
extern long defaultScreenSaverTime;
extern long defaultScreenSaverInterval;
extern int  defaultScreenSaverBlanking;
extern int  defaultScreenSaverAllowExposures;
static ClientPtr onlyClient;
static Bool grabbingClient = FALSE;
static long *checkForInput[2];
extern Bool clientsDoomed;
extern int connBlockScreenStart;

extern int (* ProcVector[256]) ();
extern int (* SwappedProcVector[256]) ();
extern void (* EventSwapVector[128]) ();
extern void (* ReplySwapVector[256]) ();
extern void Swap32Write(), SLHostsExtend(), SQColorsExtend(), WriteSConnectionInfo();
extern void WriteSConnSetupPrefix();
static void KillAllClients();
static void DeleteClientFromAnySelections();

/* buffers for clients. legal values below */
static int nextFreeClientID=1;	   /* 0 is for the server */

static int	nClients = 0;	/* number active clients */


/* Various of the DIX function interfaces were not designed to allow
 * the client->errorValue to be set on BadValue and other errors.
 * Rather than changing interfaces and breaking untold code we introduce
 * a new global that dispatch can use.
 */
XID clientErrorValue;   /* XXX this is a kludge */

#define SAME_SCREENS(a, b) (\
    (a.pScreen == b.pScreen))

#define LEGAL_NEW_RESOURCE(id,client)\
    if ((LookupID(id, RT_ANY, RC_CORE) != 0) || (id & SERVER_BIT) \
	|| (client->clientAsMask != CLIENT_BITS(id)))\
    {\
	client->errorValue = id;\
        return(BadIDChoice);\
    }

#define LOOKUP_DRAWABLE(did, client)\
    ((client->lastDrawableID == did) ? \
     client->lastDrawable : (DrawablePtr)LookupDrawable(did, client))

#define VERIFY_GC(pGC, rid, client)\
    if (client->lastGCID == rid)\
        pGC = client->lastGC;\
    else\
	pGC = (GC *)LookupID(rid, RT_GC, RC_CORE);\
    if (!pGC)\
    {\
	client->errorValue = rid;\
	return (BadGC);\
    }

#define VALIDATE_DRAWABLE_AND_GC(drawID, pDraw, pGC, client)\
    if ((client->lastDrawableID != drawID) || (client->lastGCID != stuff->gc))\
    {\
        if (client->lastDrawableID != drawID)\
    	    pDraw = (DrawablePtr)LookupID(drawID, RT_DRAWABLE, RC_CORE);\
        else\
	    pDraw = client->lastDrawable;\
        if (client->lastGCID != stuff->gc)\
	    pGC = (GC *)LookupID(stuff->gc, RT_GC, RC_CORE);\
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
            client->lastGCID = stuff->gc;\
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
        client->errorValue = stuff->gc;\
        return (BadGC);\
    }\
    if (pGC->serialNumber != pDraw->serialNumber)\
	ValidateGC(pDraw, pGC);

void
SetInputCheck(c0, c1)
    long *c0, *c1;
{
    checkForInput[0] = c0;
    checkForInput[1] = c1;
}

void
InitSelections()
{
    if (CurrentSelections)
	xfree(CurrentSelections);
    CurrentSelections = (Selection *)NULL;
    NumCurrentSelections = 0;
}

void 
FlushClientCaches(id)
    XID id;
{
    int i;
    register ClientPtr client;

    client = clients[CLIENT_ID(id)];
    if (client == NullClient)
        return ;
    for (i=0; i<currentMaxClients; i++)
    {
	client = clients[i];
        if (client != NullClient)
	{
            if (client->lastDrawableID == id)
	    {
                client->lastDrawableID = INVALID;
		client->lastDrawable = (DrawablePtr)NULL;
	    }
            else if (client->lastGCID == id)
	    {
                client->lastGCID = INVALID;
		client->lastGC = (GCPtr)NULL;
	    }
	}
    }
}

Dispatch()
{
    ClientPtr	        *clientReady;     /* mask of request ready clients */
    ClientPtr	        *newClients;      /* mask of new clients */ 
    int			result;
    xReq		*request;
    int			ErrorStatus;
    ClientPtr		client;
    int			nready, nnew;

    nextFreeClientID = 1;
    InitSelections();
    nClients = 0;
    clientsDoomed = FALSE;

    clientReady = (ClientPtr *) ALLOCATE_LOCAL(sizeof(ClientPtr) * MaxClients);
    newClients = (ClientPtr *)ALLOCATE_LOCAL(sizeof(ClientPtr) * MaxClients);
    request = (xReq *)NULL;

    while (1) 
    {
        if (*checkForInput[0] != *checkForInput[1]) {
	    ProcessInputEvents();
	    FlushIfCriticalOutputPending();
	}

	WaitForSomething(clientReady, &nready, newClients, &nnew);

	/*****************
	 *  Establish any new connections
	 *****************/

	while (nnew--)
        {
	    client = newClients[nnew];
	    client->requestLogIndex = 0;
	    InitClientResources(client);
	    SendConnectionSetupInfo(client);
	    nClients++;
	}

       /***************** 
	*  Handle events in round robin fashion, doing input between 
	*  each round 
	*****************/

	while ((nready--) > 0)
	{
	    client = clientReady[nready];
	    if (! client)
	    {
		/* KillClient can cause this to happen */
		continue;
	    }
	    /* GrabServer activation can cause this to be true */
	    if (grabbingClient && (client != onlyClient))
		break;
	    isItTimeToYield = FALSE;
 
            requestingClient = client;
	    while (! isItTimeToYield)
	    {
	        if (*checkForInput[0] != *checkForInput[1]) {
		    ProcessInputEvents();
		    FlushIfCriticalOutputPending();
		}
	   
		/* now, finally, deal with client requests */

	        request = (xReq *)ReadRequestFromClient(
				      client, &result, (char *)request);
	        if (result < 0) 
	        {
		    CloseDownClient(client);
		    break;
	        }
	        else if (result == 0)
	        {
#ifdef notdef
		    ErrorF(  "Blocked read in dispatcher\n");
		    ErrorF(  "reqType %d %d\n", 
			     (request ? request->reqType : -1),
			       nready);
#endif
		    continue;
		}

		client->sequence++;
		client->requestBuffer = (pointer)request;
		if (client->requestLogIndex == MAX_REQUEST_LOG)
		    client->requestLogIndex = 0;
		client->requestLog[client->requestLogIndex] = request->reqType;
		client->requestLogIndex++;
		if ((client->swapped ? lswaps(request->length) :
				       request->length) > MAX_REQUEST_SIZE)
		    ErrorStatus = BadLength;
		else
		    ErrorStatus = (* (client->swapped ? SwappedProcVector :
							ProcVector)
				               [request->reqType])(client);
	    
		if (ErrorStatus != Success) 
		{
		    if (client->noClientException != Success)
                        CloseDownClient(client);
                    else
		        Oops(client, request->reqType, 0, ErrorStatus);
		    break;
	        }
	    }
	    FlushAllOutput();
	}
	/* Not an error, we just need to know to restart */
	if((nClients == -1) || clientsDoomed)
	    break;         /* so that DEALLOCATE_LOCALs happen */
    }
    if (clientsDoomed)
        KillAllClients();
    DEALLOCATE_LOCAL(newClients);
    DEALLOCATE_LOCAL(clientReady);
}

/*ARGSUSED*/
int
ProcBadRequest(client)
    ClientPtr client;
{
    return (BadRequest);
}

extern int Ones();

int
ProcCreateWindow(client)
    register ClientPtr client;
{
    register WindowPtr pParent, pWin;
    REQUEST(xCreateWindowReq);
    int result;
    int len;

    REQUEST_AT_LEAST_SIZE(xCreateWindowReq);
    
    LEGAL_NEW_RESOURCE(stuff->wid, client);
    if (!(pParent = (WindowPtr)LookupWindow(stuff->parent, client)))
        return BadWindow;
    len = stuff->length -  (sizeof(xCreateWindowReq) >> 2);
    if (Ones(stuff->mask) != len)
        return BadLength;
    if (!stuff->width || !stuff->height)
    {
	client->errorValue = 0;
        return BadValue;
    }
    pWin = CreateWindow(stuff->wid, pParent, stuff->x,
			      stuff->y, stuff->width, stuff->height, 
			      stuff->borderWidth, stuff->class,
			      stuff->mask, (XID *) &stuff[1], 
			      (int)stuff->depth, 
			      client, stuff->visual, &result);
    if (pWin)
        AddResource(stuff->wid, RT_WINDOW, (pointer)pWin, DeleteWindow, RC_CORE);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcChangeWindowAttributes(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xChangeWindowAttributesReq);
    register int result;
    int len;

    REQUEST_AT_LEAST_SIZE(xChangeWindowAttributesReq);
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    len = stuff->length - (sizeof(xChangeWindowAttributesReq) >> 2);
    if (len != Ones(stuff->valueMask))
        return BadLength;
    result =  ChangeWindowAttributes(pWin, 
				  stuff->valueMask, 
				  (XID *) &stuff[1], 
				  client);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcGetWindowAttributes(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow(stuff->id, client);
    if (!pWin)
        return(BadWindow);
    GetWindowAttributes(pWin, client);
    return(client->noClientException);
}

int
ProcDestroyWindow(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow(stuff->id, client);
    if (!pWin)
        return(BadWindow);
    if (pWin->parent)
	FreeResource(stuff->id, RC_NONE);
    return(client->noClientException);
}

int
ProcDestroySubwindows(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow(stuff->id, client);
    if (!pWin)
        return(BadWindow);
    DestroySubwindows(pWin, client);
    return(client->noClientException);
}

int
ProcChangeSaveSet(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xChangeSaveSetReq);
    register result;
		  
    REQUEST_SIZE_MATCH(xChangeSaveSetReq);
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    if (client->clientAsMask == (CLIENT_BITS(pWin->wid)))
        return BadMatch;
    if ((stuff->mode == SetModeInsert) || (stuff->mode == SetModeDelete))
    {
        result = AlterSaveSetForClient(client, pWin, stuff->mode);
	if (client->noClientException != Success)
	    return(client->noClientException);
	else
            return(result);
    }
    else
    {
	client->errorValue = stuff->mode;
	return( BadValue );
    }
}

int
ProcReparentWindow(client)
    register ClientPtr client;
{
    register WindowPtr pWin, pParent;
    REQUEST(xReparentWindowReq);
    register int result;

    REQUEST_SIZE_MATCH(xReparentWindowReq);
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    pParent = (WindowPtr)LookupWindow(stuff->parent, client);
    if (!pParent)
        return(BadWindow);
    if (SAME_SCREENS(pWin->drawable, pParent->drawable))
    {
        if ((pWin->backgroundTile == (PixmapPtr)ParentRelative) &&
            (pParent->drawable.depth != pWin->drawable.depth))
            return BadMatch;
        result =  ReparentWindow(pWin, pParent, 
			 (short)stuff->x, (short)stuff->y, client);
	if (client->noClientException != Success)
            return(client->noClientException);
	else
            return(result);
    }
    else 
        return (BadMatch);
}

int
ProcMapWindow(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow(stuff->id, client);
    if (!pWin)
        return(BadWindow);
    MapWindow(pWin, HANDLE_EXPOSURES, BITS_DISCARDED,
		  SEND_NOTIFICATION, client);
           /* update cache to say it is mapped */
    return(client->noClientException);
}

int
ProcMapSubwindows(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow( stuff->id, client);
    if (!pWin)
        return(BadWindow);
    MapSubwindows(pWin, HANDLE_EXPOSURES, client);
           /* update cache to say it is mapped */
    return(client->noClientException);
}

int
ProcUnmapWindow(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow( stuff->id, client);
    if (!pWin)
        return(BadWindow);
    UnmapWindow(pWin, HANDLE_EXPOSURES, SEND_NOTIFICATION, FALSE);
           /* update cache to say it is mapped */
    return(client->noClientException);
}

int
ProcUnmapSubwindows(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow( stuff->id, client);
    if (!pWin)
        return(BadWindow);
    UnmapSubwindows(pWin, HANDLE_EXPOSURES);
    return(client->noClientException);
}

int
ProcConfigureWindow(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xConfigureWindowReq);
    register int result;
    int len;

    REQUEST_AT_LEAST_SIZE(xConfigureWindowReq);
    pWin = (WindowPtr)LookupWindow( stuff->window, client);
    if (!pWin)
        return(BadWindow);
    len = stuff->length - (sizeof(xConfigureWindowReq) >> 2);
    if (Ones((Mask)stuff->mask) != len)
        return BadLength;
    result =  ConfigureWindow(pWin, (Mask)stuff->mask, (XID *) &stuff[1], 
			      client);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcCirculateWindow(client)
    register ClientPtr client;
{
    register WindowPtr pWin;
    REQUEST(xCirculateWindowReq);

    REQUEST_SIZE_MATCH(xCirculateWindowReq);
    if ((stuff->direction != RaiseLowest) &&
	(stuff->direction != LowerHighest))
    {
	client->errorValue = stuff->direction;
        return BadValue;
    }
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    CirculateWindow(pWin, (int)stuff->direction, client);
    return(client->noClientException);
}

int
ProcGetGeometry(client)
    register ClientPtr client;
{
    xGetGeometryReply rep;
    register DrawablePtr pDraw;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    if (!(pDraw = LOOKUP_DRAWABLE(stuff->id, client)))
    {                /* can be inputonly */
        if (!(pDraw = (DrawablePtr)LookupWindow(stuff->id, client))) 
            return (BadDrawable);
    }
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.root = WindowTable[pDraw->pScreen->myNum].wid;
    rep.depth = pDraw->depth;

    if (pDraw->type == DRAWABLE_PIXMAP)
    {
	PixmapPtr pPixmap = (PixmapPtr)pDraw;

	rep.x = rep.y = rep.borderWidth = 0;
	rep.width = pPixmap->width;
	rep.height = pPixmap->height;
    }
    else
    {
        register WindowPtr pWin = (WindowPtr)pDraw;
	rep.x = pWin->clientWinSize.x - pWin->borderWidth;
	rep.y = pWin->clientWinSize.y - pWin->borderWidth;
	rep.borderWidth = pWin->borderWidth;
	rep.width = pWin->clientWinSize.width;
	rep.height = pWin->clientWinSize.height;
    }
    WriteReplyToClient(client, sizeof(xGetGeometryReply), &rep);
    return(client->noClientException);
}

int
ProcQueryTree(client)
    register ClientPtr client;
{

    xQueryTreeReply reply;
    int numChildren = 0;
    register WindowPtr pChild, pWin, pHead;
    Window  *childIDs = (Window *)NULL;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow(stuff->id, client);
    if (!pWin)
        return(BadWindow);
    reply.type = X_Reply;
    reply.root = WindowTable[pWin->drawable.pScreen->myNum].wid;
    reply.sequenceNumber = client->sequence;
    if (pWin->parent)
	reply.parent = pWin->parent->wid;
    else
        reply.parent = (Window)None;

    pHead = RealChildHead(pWin);
    for (pChild = pWin->lastChild; pChild != pHead; pChild = pChild->prevSib)
	numChildren++;
    if (numChildren)
    {
	int curChild = 0;

	childIDs = (Window *) xalloc(numChildren * sizeof(Window));
	for (pChild = pWin->lastChild; pChild != pHead; pChild = pChild->prevSib)
	    childIDs[curChild++] = pChild->wid;
    }
    
    reply.nChildren = numChildren;
    reply.length = (numChildren * sizeof(Window)) >> 2;
    
    WriteReplyToClient(client, sizeof(xQueryTreeReply), &reply);
    if (numChildren)
    {
    	client->pSwapReplyFunc = Swap32Write;
	WriteSwappedDataToClient(client, numChildren * sizeof(Window), childIDs);
	xfree(childIDs);
    }

    return(client->noClientException);
}

int
ProcInternAtom(client)
    register ClientPtr client;
{
    Atom atom;
    char *tchar;
    REQUEST(xInternAtomReq);

    REQUEST_AT_LEAST_SIZE(xInternAtomReq);
    if ((stuff->onlyIfExists != xTrue) && (stuff->onlyIfExists != xFalse))
    {
	client->errorValue = stuff->onlyIfExists;
        return(BadValue);
    }
    tchar = (char *) &stuff[1];
    atom = MakeAtom(tchar, stuff->nbytes, !stuff->onlyIfExists);
    if (atom || stuff->onlyIfExists)
    {
	xInternAtomReply reply;
	reply.type = X_Reply;
	reply.length = 0;
	reply.sequenceNumber = client->sequence;
	reply.atom = (atom ? atom : None);
	WriteReplyToClient(client, sizeof(xInternAtomReply), &reply);
	return(client->noClientException);
    }
    else
	return (BadAlloc);
}

int
ProcGetAtomName(client)
    register ClientPtr client;
{
    char *str;
    xGetAtomNameReply reply;
    int len;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    if (str = NameForAtom(stuff->id)) 
    {
	len = strlen(str);
	reply.type = X_Reply;
	reply.length = (len + 3) >> 2;
	reply.sequenceNumber = client->sequence;
	reply.nameLength = len;
	WriteReplyToClient(client, sizeof(xGetAtomNameReply), &reply);
	(void)WriteToClient(client, len, str);
	return(client->noClientException);
    }
    else 
    { 
	client->errorValue = stuff->id;
	return (BadAtom);
    }
}

int 
ProcDeleteProperty(client)
    register ClientPtr client;
{
    WindowPtr pWin;
    REQUEST(xDeletePropertyReq);
    int result;
              
    REQUEST_SIZE_MATCH(xDeletePropertyReq);
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    if (ValidAtom(stuff->property))
    {
	result = DeleteProperty(pWin, stuff->property);
        if (client->noClientException != Success)
            return(client->noClientException);
	else
	    return(result);
    }
    else 
    {
	client->errorValue = stuff->property;
	return (BadAtom);
    }
}


int
ProcSetSelectionOwner(client)
    register ClientPtr client;
{
    WindowPtr pWin;
    TimeStamp time;
    REQUEST(xSetSelectionOwnerReq);

    REQUEST_SIZE_MATCH(xSetSelectionOwnerReq);
    time = ClientTimeToServerTime(stuff->time);

    /* If the client's time stamp is in the future relative to the server's
	time stamp, do not set the selection, just return success. */
    if (CompareTimeStamps(time, currentTime) == LATER)
    	return Success;
    if (stuff->window != None)
    {
        pWin = (WindowPtr)LookupWindow(stuff->window, client);
        if (!pWin)
            return(BadWindow);
    }
    else
        pWin = (WindowPtr)None;
    if (ValidAtom(stuff->selection))
    {
	int i = 0;

	/*
	 * First, see if the selection is already set... 
	 */
	while ((i < NumCurrentSelections) && 
	       CurrentSelections[i].selection != stuff->selection) 
            i++;
        if (i < NumCurrentSelections)
        {        
	    xEvent event;

	    /* If the timestamp in client's request is in the past relative
		to the time stamp indicating the last time the owner of the
		selection was set, do not set the selection, just return 
		success. */
            if (CompareTimeStamps(time, CurrentSelections[i].lastTimeChanged)
		== EARLIER)
		return Success;
	    if ((CurrentSelections[i].window != None) &&
		(CurrentSelections[i].client != client))
	    {
		event.u.u.type = SelectionClear;
		event.u.selectionClear.time = time.milliseconds;
		event.u.selectionClear.window = CurrentSelections[i].window;
		event.u.selectionClear.atom = CurrentSelections[i].selection;
		(void) TryClientEvents (CurrentSelections[i].client, &event, 1,
				NoEventMask, NoEventMask /* CantBeFiltered */,
				NullGrab);
	    }
	}
	else
	{
	    /*
	     * It doesn't exist, so add it...
	     */
	    NumCurrentSelections++;
	    if (i == 0)
		CurrentSelections = (Selection *)xalloc(sizeof(Selection));
	    else
		CurrentSelections = (Selection *)xrealloc(CurrentSelections, 
			    NumCurrentSelections * sizeof(Selection));
	    CurrentSelections[i].selection = stuff->selection;
	}
        CurrentSelections[i].lastTimeChanged = time;
	CurrentSelections[i].window = stuff->window;
	CurrentSelections[i].pWin = pWin;
	CurrentSelections[i].client = client;
	return (client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->selection;
        return (BadAtom);
    }
}

int
ProcGetSelectionOwner(client)
    register ClientPtr client;
{
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    if (ValidAtom(stuff->id))
    {
	int i;
        xGetSelectionOwnerReply reply;

	i = 0;
        while ((i < NumCurrentSelections) && 
	       CurrentSelections[i].selection != stuff->id) i++;
        reply.type = X_Reply;
	reply.length = 0;
	reply.sequenceNumber = client->sequence;
        if (i < NumCurrentSelections)
            reply.owner = CurrentSelections[i].window;
        else
            reply.owner = None;
        WriteReplyToClient(client, sizeof(xGetSelectionOwnerReply), &reply);
        return(client->noClientException);
    }
    else            
    {
	client->errorValue = stuff->id;
        return (BadAtom); 
    }
}

int
ProcConvertSelection(client)
    register ClientPtr client;
{
    Bool paramsOkay;
    xEvent event;
    WindowPtr pWin;
    REQUEST(xConvertSelectionReq);

    REQUEST_SIZE_MATCH(xConvertSelectionReq);
    pWin = (WindowPtr)LookupWindow(stuff->requestor, client);
    if (!pWin)
        return(BadWindow);

    paramsOkay = (ValidAtom(stuff->selection) && ValidAtom(stuff->target));
    if (stuff->property != None)
	paramsOkay &= ValidAtom(stuff->property);
    if (paramsOkay)
    {
	int i;

	i = 0;
	while ((i < NumCurrentSelections) && 
	       CurrentSelections[i].selection != stuff->selection) i++;
	if ((i < NumCurrentSelections) && 
	    (CurrentSelections[i].window != None))
	{        
	    event.u.u.type = SelectionRequest;
	    event.u.selectionRequest.time = stuff->time;
	    event.u.selectionRequest.owner = 
			CurrentSelections[i].window;
	    event.u.selectionRequest.requestor = stuff->requestor;
	    event.u.selectionRequest.selection = stuff->selection;
	    event.u.selectionRequest.target = stuff->target;
	    event.u.selectionRequest.property = stuff->property;
	    if (TryClientEvents(
		CurrentSelections[i].client, &event, 1, NoEventMask,
		NoEventMask /* CantBeFiltered */, NullGrab))
		return (client->noClientException);
	}
	event.u.u.type = SelectionNotify;
	event.u.selectionNotify.time = stuff->time;
	event.u.selectionNotify.requestor = stuff->requestor;
	event.u.selectionNotify.selection = stuff->selection;
	event.u.selectionNotify.target = stuff->target;
	event.u.selectionNotify.property = None;
	(void) TryClientEvents(client, &event, 1, NoEventMask,
			       NoEventMask /* CantBeFiltered */, NullGrab);
	return (client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->property;
        return (BadAtom);
    }
}

int
ProcGrabServer(client)
    register ClientPtr client;
{
    OnlyListenToOneClient(client);
    grabbingClient = TRUE;
    onlyClient = client;
    return(client->noClientException);
}

int
ProcUngrabServer(client)
    register ClientPtr client;
{
    REQUEST(xReq);
    REQUEST_SIZE_MATCH(xReq);
    grabbingClient = FALSE;
    ListenToAllClients();
    return(client->noClientException);
}

int
ProcTranslateCoords(client)
    register ClientPtr client;
{
    REQUEST(xTranslateCoordsReq);

    register WindowPtr pWin, pDst;
    xTranslateCoordsReply rep;

    REQUEST_SIZE_MATCH(xTranslateCoordsReq);
    pWin = (WindowPtr)LookupWindow(stuff->srcWid, client);
    if (!pWin)
        return(BadWindow);
    pDst = (WindowPtr)LookupWindow(stuff->dstWid, client);
    if (!pDst)
        return(BadWindow);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (!SAME_SCREENS(pWin->drawable, pDst->drawable))
    {
	rep.sameScreen = xFalse;
        rep.child = None;
	rep.dstX = rep.dstY = 0;
    }
    else
    {
	INT16 x, y;
	rep.sameScreen = xTrue;
	rep.child = None;
	/* computing absolute coordinates -- adjust to destination later */
	x = pWin->absCorner.x + stuff->srcX;
	y = pWin->absCorner.y + stuff->srcY;
	pWin = pDst->firstChild;
	while (pWin)
	{
	    if ((pWin->mapped) &&
		(x >= pWin->absCorner.x - pWin->borderWidth) &&
		(x < pWin->absCorner.x + (int)pWin->clientWinSize.width +
		 pWin->borderWidth) &&
		(y >= pWin->absCorner.y - pWin->borderWidth) &&
		(y < pWin->absCorner.y + (int)pWin->clientWinSize.height
		 + pWin->borderWidth))
            {
		rep.child = pWin->wid;
		pWin = (WindowPtr) NULL;
	    }
	    else
		pWin = pWin->nextSib;
	}
	/* adjust to destination coordinates */
	rep.dstX = x - pDst->absCorner.x;
	rep.dstY = y - pDst->absCorner.y;
    }
    WriteReplyToClient(client, sizeof(xTranslateCoordsReply), &rep);
    return(client->noClientException);
}

int
ProcOpenFont(client)
    register ClientPtr client;
{
    FontPtr pFont;
    REQUEST(xOpenFontReq);

    REQUEST_AT_LEAST_SIZE(xOpenFontReq);
    client->errorValue = stuff->fid;
    LEGAL_NEW_RESOURCE(stuff->fid, client);
    if ( pFont = OpenFont( stuff->nbytes, (char *)&stuff[1]))
    {
	AddResource( stuff->fid, RT_FONT, (pointer)pFont, CloseFont,RC_CORE);
	return(client->noClientException);
    }
    else
	return (BadName);
}

int
ProcCloseFont(client)
    register ClientPtr client;
{
    FontPtr pFont;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pFont = (FontPtr)LookupID(stuff->id, RT_FONT, RC_CORE);
    if ( pFont != (FontPtr)NULL)	/* id was valid */
    {
        FreeResource( stuff->id, RC_NONE);
	return(client->noClientException);
    }
    else
    {
	client->errorValue = stuff->id;
        return (BadFont);
    }
}

int
ProcQueryFont(client)
    register ClientPtr client;
{
    xQueryFontReply	*reply;
    FontPtr pFont;
    register GC *pGC;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    client->errorValue = stuff->id;		/* EITHER font or gc */
    pFont = (FontPtr)LookupID(stuff->id, RT_FONT, RC_CORE);
    if (!pFont)
    {
	  /* can't use VERIFY_GC because it might return BadGC */
	pGC = (GC *) LookupID(stuff->id, RT_GC, RC_CORE);
        if (!pGC)
	{
	    client->errorValue = stuff->id;
            return(BadFont);     /* procotol spec says only error is BadFont */
	}
	pFont = pGC->font;
    }

    {
	CharInfoPtr	pmax = pFont->pInkMax;
	CharInfoPtr	pmin = pFont->pInkMin;
	int		nprotoxcistructs;
	int		rlength;

	nprotoxcistructs = (
	   pmax->metrics.rightSideBearing == pmin->metrics.rightSideBearing &&
	   pmax->metrics.leftSideBearing == pmin->metrics.leftSideBearing &&
	   pmax->metrics.descent == pmin->metrics.descent &&
	   pmax->metrics.ascent == pmin->metrics.ascent &&
	   pmax->metrics.characterWidth == pmin->metrics.characterWidth) ?
		0 : n2dChars(pFont->pFI);

	rlength = sizeof(xQueryFontReply) +
	             pFont->pFI->nProps * sizeof(xFontProp)  +
		     nprotoxcistructs * sizeof(xCharInfo);
	reply = (xQueryFontReply *)ALLOCATE_LOCAL(rlength);
	if(!reply)
	{
	    return(BadAlloc);
	}

	reply->type = X_Reply;
	reply->length = (rlength - sizeof(xGenericReply)) >> 2;
	reply->sequenceNumber = client->sequence;
	QueryFont( pFont, reply, nprotoxcistructs);

        WriteReplyToClient(client, rlength, reply);
	DEALLOCATE_LOCAL(reply);
	return(client->noClientException);
    }
}

int
ProcQueryTextExtents(client)
    register ClientPtr client;
{
    REQUEST(xQueryTextExtentsReq);
    xQueryTextExtentsReply reply;
    FontPtr pFont;
    GC *pGC;
    ExtentInfoRec info;
    unsigned long length;

    REQUEST_AT_LEAST_SIZE(xQueryTextExtentsReq);
        
    pFont = (FontPtr)LookupID( stuff->fid, RT_FONT, RC_CORE);
    if (!pFont)
    {
        pGC = (GC *)LookupID( stuff->fid, RT_GC, RC_CORE);
        if (!pGC)
	{
	    client->errorValue = stuff->fid;
            return(BadFont);
	}
	pFont = pGC->font;
    }
    length = stuff->length - (sizeof(xQueryTextExtentsReq) >> 2);
    length = length << 1;
    if (stuff->oddLength)
        length--;
    QueryTextExtents(pFont, length, (unsigned char *)&stuff[1], &info);   
    reply.type = X_Reply;
    reply.length = 0;
    reply.sequenceNumber = client->sequence;
    reply.drawDirection = info.drawDirection;
    reply.fontAscent = info.fontAscent;
    reply.fontDescent = info.fontDescent;
    reply.overallAscent = info.overallAscent;
    reply.overallDescent = info.overallDescent;
    reply.overallWidth = info.overallWidth;
    reply.overallLeft = info.overallLeft;
    reply.overallRight = info.overallRight;
    WriteReplyToClient(client, sizeof(xQueryTextExtentsReply), &reply);
    return(client->noClientException);
}

int
ProcListFonts(client)
    register ClientPtr client;
{
    xListFontsReply reply; 
    FontPathPtr fpr;
    int stringLens, i;
    char *bufptr, *bufferStart;
    REQUEST(xListFontsReq);

    REQUEST_AT_LEAST_SIZE(xListFontsReq);

    fpr = ExpandFontNamePattern( stuff->nbytes, 
				 (char *) &stuff[1], stuff->maxNames);
    stringLens = 0;
    for (i=0; i<fpr->npaths; i++)
        stringLens += fpr->length[i];

    reply.type = X_Reply;
    reply.length = (stringLens + fpr->npaths + 3) >> 2;
    reply.nFonts = fpr->npaths;
    reply.sequenceNumber = client->sequence;

    bufptr = bufferStart = (char *)ALLOCATE_LOCAL(reply.length << 2);
    if(!bufptr)
        return(BadAlloc);

            /* since WriteToClient long word aligns things, 
	       copy to temp buffer and write all at once */
    for (i=0; i<fpr->npaths; i++)
    {
        *bufptr++ = fpr->length[i];
        bcopy(fpr->paths[i], bufptr,  fpr->length[i]);
        bufptr += fpr->length[i];
    }
    WriteReplyToClient(client, sizeof(xListFontsReply), &reply);
    (void)WriteToClient(client, stringLens + fpr->npaths, bufferStart);
    FreeFontRecord(fpr);
    DEALLOCATE_LOCAL(bufferStart);
    
    return(client->noClientException);
}

int
ProcListFontsWithInfo(client)
    register ClientPtr client;
{
    register xListFontsWithInfoReply *reply;
    xListFontsWithInfoReply last_reply;
    FontRec font;
    FontInfoRec finfo;
    register FontPathPtr fpaths;
    register char **path;
    register int n, *length;
    int rlength;
    REQUEST(xListFontsWithInfoReq);

    REQUEST_AT_LEAST_SIZE(xListFontsWithInfoReq);

    fpaths = ExpandFontNamePattern( stuff->nbytes,
				    (char *) &stuff[1], stuff->maxNames);
    font.pFI = &finfo;
    font.pInkMin = &finfo.minbounds;
    font.pInkMax = &finfo.maxbounds;
    for (n = fpaths->npaths, path = fpaths->paths, length = fpaths->length;
	 --n >= 0;
	 path++, length++)
    {
	if (!(DescribeFont(*path, *length, &finfo, &font.pFP)))
	   continue;
	rlength = sizeof(xListFontsWithInfoReply)
		    + finfo.nProps * sizeof(xFontProp);
	if (reply = (xListFontsWithInfoReply *)ALLOCATE_LOCAL(rlength))
	{
		reply->type = X_Reply;
		reply->sequenceNumber = client->sequence;
		reply->length = (rlength - sizeof(xGenericReply)
				 + *length + 3) >> 2;
		QueryFont(&font, (xQueryFontReply *) reply, 0);
		reply->nameLength = *length;
		reply->nReplies = n;
		WriteReplyToClient(client, rlength, reply);
		(void)WriteToClient(client, *length, *path);
		DEALLOCATE_LOCAL(reply);
	}
	xfree(font.pFP);
    }
    FreeFontRecord(fpaths);
    bzero((char *)&last_reply, sizeof(xListFontsWithInfoReply));
    last_reply.type = X_Reply;
    last_reply.sequenceNumber = client->sequence;
    last_reply.length = (sizeof(xListFontsWithInfoReply)
			  - sizeof(xGenericReply)) >> 2;
    WriteReplyToClient(client, sizeof(xListFontsWithInfoReply), &last_reply);
    return(client->noClientException);
}

/*ARGSUSED*/
static
dixDestroyPixmap(pPixmap, pid)
    PixmapPtr pPixmap;
    Pixmap pid;
{
    (*pPixmap->drawable.pScreen->DestroyPixmap)(pPixmap);
}

int
ProcCreatePixmap(client)
    register ClientPtr client;
{
    PixmapPtr pMap;
    register DrawablePtr pDraw;
    REQUEST(xCreatePixmapReq);
    DepthPtr pDepth;
    register int i;

    REQUEST_AT_LEAST_SIZE(xCreatePixmapReq);
    client->errorValue = stuff->pid;
    LEGAL_NEW_RESOURCE(stuff->pid, client);
    if (!(pDraw = LOOKUP_DRAWABLE(stuff->drawable, client)))
    {        /* can be inputonly */
        if (!(pDraw = (DrawablePtr)LookupWindow(stuff->drawable, client))) 
            return (BadDrawable);
    }

    if (!stuff->width || !stuff->height)
    {
	client->errorValue = 0;
        return BadValue;
    }
    if (stuff->depth != 1)
    {
        pDepth = pDraw->pScreen->allowedDepths;
        for (i=0; i<pDraw->pScreen->numDepths; i++, pDepth++)
	   if (pDepth->depth == stuff->depth)
               goto CreatePmap;
	client->errorValue = stuff->depth;
        return BadValue;
    }
CreatePmap:
    pMap = (PixmapPtr)(*pDraw->pScreen->CreatePixmap)
		(pDraw->pScreen, stuff->width,
		 stuff->height, stuff->depth);
    if (pMap)
    {
	pMap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	AddResource(
	    stuff->pid, RT_PIXMAP, (pointer)pMap, dixDestroyPixmap,
	    RC_CORE);
	return(client->noClientException);
    }
    else
	return (BadAlloc);
}

int
ProcFreePixmap(client)
    register ClientPtr client;
{
    PixmapPtr pMap;

    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pMap = (PixmapPtr)LookupID(stuff->id, RT_PIXMAP, RC_CORE);
    if (pMap) 
    {
	FreeResource(stuff->id, RC_NONE);
	return(client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->id;
	return (BadPixmap);
    }
}

int
ProcCreateGC(client)
    register ClientPtr client;
{
    int error;
    GC *pGC;
    register DrawablePtr pDraw;
    unsigned len;
    REQUEST(xCreateGCReq);

    REQUEST_AT_LEAST_SIZE(xCreateGCReq);
    client->errorValue = stuff->gc;
    LEGAL_NEW_RESOURCE(stuff->gc, client);
    if (!(pDraw = LOOKUP_DRAWABLE( stuff->drawable, client) ))
    {
	client->errorValue = stuff->drawable;
        return (BadDrawable);
    }
    len = stuff->length -  (sizeof(xCreateGCReq) >> 2);
    if (len != Ones(stuff->mask))
        return BadLength;
    pGC = (GC *)CreateGC(pDraw, stuff->mask, 
			 (XID *) &stuff[1], &error);
    if (error != Success)
        return error;
    if (pGC)
    {
	AddResource(stuff->gc, RT_GC, (pointer)pGC, FreeGC, RC_CORE);
	return(client->noClientException);
    }
    else 
	return (BadAlloc);
}

int
ProcChangeGC(client)
    register ClientPtr client;
{
    GC *pGC;
    REQUEST(xChangeGCReq);
    int result;
    unsigned len;
		
    REQUEST_AT_LEAST_SIZE(xChangeGCReq);
    VERIFY_GC(pGC, stuff->gc, client);
    len = stuff->length -  (sizeof(xChangeGCReq) >> 2);
    if (len != Ones(stuff->mask))
        return BadLength;
    result = DoChangeGC(pGC, stuff->mask, (XID *) &stuff[1], 0);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
    {
	client->errorValue = clientErrorValue;
        return(result);
    }
}

int
ProcCopyGC(client)
    register ClientPtr client;
{
    register GC *dstGC;
    register GC *pGC;
    int result;
    REQUEST(xCopyGCReq);

    REQUEST_SIZE_MATCH(xCopyGCReq);
    VERIFY_GC( pGC, stuff->srcGC, client);
    VERIFY_GC( dstGC, stuff->dstGC, client);
    if ((dstGC->pScreen != pGC->pScreen) || (dstGC->depth != pGC->depth))
        return (BadMatch);    
    result = CopyGC(pGC, dstGC, stuff->mask);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
    {
	client->errorValue = clientErrorValue;
        return(result);
    }
}

int
ProcSetDashes(client)
    register ClientPtr client;
{
    register GC *pGC;
    int result;
    REQUEST(xSetDashesReq);

    REQUEST_AT_LEAST_SIZE(xSetDashesReq);
    if ((sizeof(xSetDashesReq) >> 2) == stuff->length)
    {
	 client->errorValue = 0;
         return BadValue;
    }

    VERIFY_GC(pGC,stuff->gc, client);

    result = SetDashes(pGC, stuff->dashOffset, stuff->nDashes,
		       (unsigned char *)&stuff[1]);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
    {
	client->errorValue = clientErrorValue;
        return(result);
    }
}

int
ProcSetClipRectangles(client)
    register ClientPtr client;
{
    int	nr;
    int result;
    register GC *pGC;
    REQUEST(xSetClipRectanglesReq);

    REQUEST_AT_LEAST_SIZE(xSetClipRectanglesReq);
    if ((stuff->ordering != Unsorted) && (stuff->ordering != YSorted) &&
	(stuff->ordering != YXSorted) && (stuff->ordering != YXBanded))
    {
	client->errorValue = stuff->ordering;
        return BadValue;
    }
    VERIFY_GC(pGC,stuff->gc, client);
		 
    nr = ((stuff->length  << 2) - sizeof(xSetClipRectanglesReq)) >> 3;
    result = SetClipRects(pGC, stuff->xOrigin, stuff->yOrigin,
			  nr, (xRectangle *)&stuff[1], (int)stuff->ordering);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcFreeGC(client)
    register ClientPtr client;
{
    register GC *pGC;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    VERIFY_GC(pGC,stuff->id,client);
    FreeResource(stuff->id, RC_NONE);
    return(client->noClientException);
}

int
ProcClearToBackground(client)
    register ClientPtr client;
{
    REQUEST(xClearAreaReq);
    register WindowPtr pWin;

    REQUEST_SIZE_MATCH(xClearAreaReq);
    pWin = (WindowPtr)LookupWindow( stuff->window, client);
    if (!pWin)
        return(BadWindow);
    if (pWin->class == InputOnly)
    {
	client->errorValue = stuff->window;
	return (BadWindow);
    }		    
    if ((stuff->exposures != xTrue) && (stuff->exposures != xFalse))
    {
	client->errorValue = stuff->exposures;
        return(BadValue);
    }
    (*pWin->ClearToBackground)(pWin, stuff->x, stuff->y,
			       stuff->width, stuff->height,
			       (Bool)stuff->exposures);
    return(client->noClientException);
}

int
ProcCopyArea(client)
    register ClientPtr client;
{
    register DrawablePtr pDst;
    register DrawablePtr pSrc;
    register GC *pGC;
    REQUEST(xCopyAreaReq);
    RegionPtr pRgn;

    REQUEST_SIZE_MATCH(xCopyAreaReq);

    VALIDATE_DRAWABLE_AND_GC(stuff->dstDrawable, pDst, pGC, client); 
    if (stuff->dstDrawable != stuff->srcDrawable)
    {
        if (!(pSrc = LOOKUP_DRAWABLE(stuff->srcDrawable, client)))
	{
	    client->errorValue = stuff->srcDrawable;
            return(BadDrawable);
	}
	if ((pDst->pScreen != pSrc->pScreen) || (pDst->depth != pSrc->depth))
	{
	    client->errorValue = stuff->dstDrawable;
	    return (BadMatch);
	}
    }
    else
        pSrc = pDst;
    pRgn = (*pGC->CopyArea)(pSrc, pDst, pGC, stuff->srcX, stuff->srcY,
				 stuff->width, stuff->height, 
				 stuff->dstX, stuff->dstY);
    if (pGC->graphicsExposures)
    {
	(*pDst->pScreen->SendGraphicsExpose)
 		(client, pRgn, stuff->dstDrawable, X_CopyArea, 0);
	if (pRgn)
	    (*pDst->pScreen->RegionDestroy) (pRgn);
    }

    return(client->noClientException);
}

int
ProcCopyPlane(client)
    register ClientPtr client;
{
    register DrawablePtr psrcDraw, pdstDraw;
    register GC *pGC;
    REQUEST(xCopyPlaneReq);
    RegionPtr pRgn;

    REQUEST_SIZE_MATCH(xCopyPlaneReq);

    VALIDATE_DRAWABLE_AND_GC(stuff->dstDrawable, pdstDraw, pGC, client);
    if (stuff->dstDrawable != stuff->srcDrawable)
    {
        if (!(psrcDraw = LOOKUP_DRAWABLE(stuff->srcDrawable, client)))
	{
	    client->errorValue = stuff->srcDrawable;
            return(BadDrawable);
	}
	if (pdstDraw->pScreen != psrcDraw->pScreen)
	{
	    client->errorValue = stuff->dstDrawable;
	    return (BadMatch);
	}
    }
    else
        psrcDraw = pdstDraw;

    /* Check to see if stuff->bitPlane has exactly ONE good bit set */
    if(stuff->bitPlane == 0 || (stuff->bitPlane & (stuff->bitPlane - 1)) ||
       (stuff->bitPlane > (1L << (psrcDraw->depth - 1))))
    {
       client->errorValue = stuff->bitPlane;
       return(BadValue);
    }

    pRgn = (*pGC->CopyPlane)(psrcDraw, pdstDraw, pGC, stuff->srcX, stuff->srcY,
				 stuff->width, stuff->height, 
				 stuff->dstX, stuff->dstY, stuff->bitPlane);
    if (pGC->graphicsExposures)
    {
	(*pdstDraw->pScreen->SendGraphicsExpose)
 		(client, pRgn, stuff->dstDrawable, X_CopyPlane, 0);
	if (pRgn)
	    (*pdstDraw->pScreen->RegionDestroy) (pRgn);
    }
    return(client->noClientException);
}

int
ProcPolyPoint(client)
    register ClientPtr client;
{
    int npoint;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xPolyPointReq);

    REQUEST_AT_LEAST_SIZE(xPolyPointReq);
    if ((stuff->coordMode != CoordModeOrigin) && 
	(stuff->coordMode != CoordModePrevious))
    {
	client->errorValue = stuff->coordMode;
        return BadValue;
    }
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client); 
    npoint = ((stuff->length << 2) - sizeof(xPolyPointReq)) >> 2;
    if (npoint)
        (*pGC->PolyPoint)(pDraw, pGC, stuff->coordMode, npoint,
			  (xPoint *) &stuff[1]);
    return (client->noClientException);
}

int
ProcPolyLine(client)
    register ClientPtr client;
{
    int npoint;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xPolyLineReq);

    REQUEST_AT_LEAST_SIZE(xPolyLineReq);
    if ((stuff->coordMode != CoordModeOrigin) && 
	(stuff->coordMode != CoordModePrevious))
    {
	client->errorValue = stuff->coordMode;
        return BadValue;
    }
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    npoint = ((stuff->length << 2) - sizeof(xPolyLineReq));
    if(npoint % sizeof(xPoint) != 0)
	return(BadLength);
    npoint >>= 2;
    if (npoint < 1)
	return(BadLength);

    (*pGC->Polylines)(pDraw, pGC, stuff->coordMode, npoint, 
			  (xPoint *) &stuff[1]);
    return(client->noClientException);
}

int
ProcPolySegment(client)
    register ClientPtr client;
{
    int nsegs;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xPolySegmentReq);

    REQUEST_AT_LEAST_SIZE(xPolySegmentReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    nsegs = (stuff->length << 2) - sizeof(xPolySegmentReq);
    if(nsegs % sizeof(xSegment) != 0)
	return(BadLength);
    nsegs >>= 3;
    if (nsegs)
        (*pGC->PolySegment)(pDraw, pGC, nsegs, (xSegment *) &stuff[1]);
    return (client->noClientException);
}

int
ProcPolyRectangle (client)
    register ClientPtr client;
{
    int nrects;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xPolyRectangleReq);

    REQUEST_AT_LEAST_SIZE(xPolyRectangleReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    nrects = ((stuff->length << 2) - sizeof(xPolyRectangleReq)) >> 3;
    if (nrects)
        (*pGC->PolyRectangle)(pDraw, pGC, 
		    nrects, (xRectangle *) &stuff[1]);
    return(client->noClientException);
}

int
ProcPolyArc(client)
    register ClientPtr client;
{
    int		narcs;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xPolyArcReq);

    REQUEST_AT_LEAST_SIZE(xPolyArcReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    narcs = ((stuff->length << 2) - sizeof(xPolyArcReq)) / 
	    sizeof(xArc);
    if (narcs)
        (*pGC->PolyArc)(pDraw, pGC, narcs, (xArc *) &stuff[1]);
    return (client->noClientException);
}

int
ProcFillPoly(client)
    register ClientPtr client;
{
    int          things;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xFillPolyReq);

    REQUEST_AT_LEAST_SIZE(xFillPolyReq);
    if ((stuff->shape != Complex) && (stuff->shape != Nonconvex) &&  
	(stuff->shape != Convex))
    {
	client->errorValue = stuff->shape;
        return BadValue;
    }
    if ((stuff->coordMode != CoordModeOrigin) && 
	(stuff->coordMode != CoordModePrevious))
    {
	client->errorValue = stuff->coordMode;
        return BadValue;
    }

    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    things = ((stuff->length << 2) - sizeof(xFillPolyReq)) >> 2;
    if (things)
        (*pGC->FillPolygon) (pDraw, pGC, stuff->shape,
			 stuff->coordMode, things,
			 (DDXPointPtr) &stuff[1]);
    return(client->noClientException);
}

int
ProcPolyFillRectangle(client)
    register ClientPtr client;
{
    int             things;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xPolyFillRectangleReq);

    REQUEST_AT_LEAST_SIZE(xPolyFillRectangleReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    things = ((stuff->length << 2) - 
	      sizeof(xPolyFillRectangleReq)) >> 3;
    if (things)
        (*pGC->PolyFillRect) (pDraw, pGC, things,
		      (xRectangle *) &stuff[1]);
    return (client->noClientException);
}

int
ProcPolyFillArc               (client)
    register ClientPtr client;
{
    int		narcs;
    register GC *pGC;
    register DrawablePtr pDraw;
    REQUEST(xPolyFillArcReq);

    REQUEST_AT_LEAST_SIZE(xPolyFillArcReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    narcs = ((stuff->length << 2) - 
	     sizeof(xPolyFillArcReq)) / sizeof(xArc);
    if (narcs)
        (*pGC->PolyFillArc) (pDraw, pGC, narcs, (xArc *) &stuff[1]);
    return (client->noClientException);
}

int
ProcPutImage(client)
    register ClientPtr client;
{
    register GC *pGC;
    register DrawablePtr pDraw;
    long length;
    REQUEST(xPutImageReq);

    REQUEST_AT_LEAST_SIZE(xPutImageReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    if (stuff->format == XYBitmap)
    {
        if ((stuff->depth != 1) ||
	    (stuff->leftPad >= screenInfo.bitmapScanlinePad))
            return BadMatch;
        length = PixmapBytePad(stuff->width + stuff->leftPad, 1);
    }
    else if (stuff->format == XYPixmap)
    {
        if ((pDraw->depth != stuff->depth) || 
	    (stuff->leftPad >= screenInfo.bitmapScanlinePad))
            return BadMatch;
        length = PixmapBytePad(stuff->width + stuff->leftPad, 1);
	length *= stuff->depth;
    }
    else if (stuff->format == ZPixmap)
    {
        if ((pDraw->depth != stuff->depth) || (stuff->leftPad != 0))
            return BadMatch;
        length = PixmapBytePad(stuff->width, stuff->depth);
    }
    else
    {
	client->errorValue = stuff->format;
        return BadValue;
    }
    length *= stuff->height;
    if ((((length + 3) >> 2) + (sizeof(xPutImageReq) >> 2)) != stuff->length)
	return BadLength;
    (*pGC->PutImage) (pDraw, pGC, stuff->depth, stuff->dstX, stuff->dstY,
		  stuff->width, stuff->height, 
		  stuff->leftPad, stuff->format, 
		  (char *) &stuff[1]);
     return (client->noClientException);
}

int
ProcGetImage(client)
    register ClientPtr	client;
{
    register DrawablePtr pDraw;
    int			nlines, linesPerBuf;
    register int	height, linesDone;
    long		widthBytesLine, length;
    Mask		plane;
    char		*pBuf;
    xGetImageReply	xgi;

    REQUEST(xGetImageReq);

    height = stuff->height;
    REQUEST_SIZE_MATCH(xGetImageReq);
    if ((stuff->format != XYPixmap) && (stuff->format != ZPixmap))
    {
	client->errorValue = stuff->format;
        return(BadValue);
    }
    if(!(pDraw = LOOKUP_DRAWABLE(stuff->drawable, client) ))
    {
	client->errorValue = stuff->drawable;
	return (BadDrawable);
    }
    if(pDraw->type == DRAWABLE_WINDOW)
    {
      if( /* check for being viewable */
	 !((WindowPtr) pDraw)->realized ||
	  /* check for being on screen */
         ((WindowPtr) pDraw)->absCorner.x + stuff->x < 0 ||
         ((WindowPtr) pDraw)->absCorner.x + stuff->x + (int)stuff->width >
             pDraw->pScreen->width ||
         ((WindowPtr) pDraw)->absCorner.y + stuff->y < 0 ||
         ((WindowPtr) pDraw)->absCorner.y + stuff->y + height >
             pDraw->pScreen->height ||
          /* check for being inside of border */
         stuff->x < -((WindowPtr)pDraw)->borderWidth ||
         stuff->x + (int)stuff->width >
              ((WindowPtr)pDraw)->borderWidth +
              (int)((WindowPtr)pDraw)->clientWinSize.width ||
         stuff->y < -((WindowPtr)pDraw)->borderWidth ||
         stuff->y + height >
              ((WindowPtr)pDraw)->borderWidth +
              (int)((WindowPtr)pDraw)->clientWinSize.height
        )
	    return(BadMatch);
	xgi.visual = ((WindowPtr) pDraw)->visual;
    }
    else
    {
      if((stuff->x < 0) ||
         (stuff->x+(int)stuff->width > ((PixmapPtr) pDraw)->width) ||
         (stuff->y < 0) ||
         (stuff->y+height > ((PixmapPtr) pDraw)->height)
        )
	    return(BadMatch);
	xgi.visual = None;
    }
    xgi.type = X_Reply;
    xgi.sequenceNumber = client->sequence;
    xgi.depth = pDraw->depth;
    if(stuff->format == ZPixmap)
    {
	widthBytesLine = PixmapBytePad(stuff->width, pDraw->depth);
	length = widthBytesLine * height;
    }
    else 
    {
	widthBytesLine = PixmapBytePad(stuff->width, 1);
	plane = ((Mask)1) << (pDraw->depth - 1);
	/* only planes asked for */
	length = widthBytesLine * height *
		 Ones(stuff->planeMask & (plane | (plane - 1)));
    }
    xgi.length = (length + 3) >> 2;
    if (widthBytesLine == 0 || height == 0)
	linesPerBuf = 0;
    else if (widthBytesLine >= IMAGE_BUFSIZE)
	linesPerBuf = 1;
    else
    {
	linesPerBuf = IMAGE_BUFSIZE / widthBytesLine;
	if (linesPerBuf > height)
	    linesPerBuf = height;
    }
    length = linesPerBuf * widthBytesLine;
    if (linesPerBuf < height)
    {
	/* we have to make sure intermediate buffers don't need padding */
	while ((linesPerBuf > 1) && (length & 3))
	{
	    linesPerBuf--;
	    length -= widthBytesLine;
	}
	while (length & 3)
	{
	    linesPerBuf++;
	    length += widthBytesLine;
	}
    }
    if(!(pBuf = (char *) ALLOCATE_LOCAL(length)))
        return (BadAlloc);

    WriteReplyToClient(client, sizeof (xGetImageReply), &xgi);

    if (linesPerBuf == 0)
    {
	/* nothing to do */
    }
    else if (stuff->format == ZPixmap)
    {
        linesDone = 0;
        while (height - linesDone > 0)
        {
	    nlines = min(linesPerBuf, height - linesDone);
	    (*pDraw->pScreen->GetImage) (pDraw,
	                                 stuff->x,
				         stuff->y + linesDone,
				         stuff->width, 
				         nlines,
				         stuff->format,
				         stuff->planeMask,
				         pBuf);
	    /* Note that this is NOT a call to WriteSwappedDataToClient,
               as we do NOT byte swap */
	    (void)WriteToClient(client, nlines * widthBytesLine, pBuf);
	    linesDone += nlines;
        }
    }
    else
    {
        for (; plane; plane >>= 1)
	{
	    if (stuff->planeMask & plane)
	    {
	        linesDone = 0;
	        while (height - linesDone > 0)
	        {
		    nlines = min(linesPerBuf, height - linesDone);
	            (*pDraw->pScreen->GetImage) (pDraw,
	                                         stuff->x,
				                 stuff->y + linesDone,
				                 stuff->width, 
				                 nlines,
				                 stuff->format,
				                 plane,
				                 pBuf);
		    /* Note: NOT a call to WriteSwappedDataToClient,
		       as we do NOT byte swap */
		    (void)WriteToClient(client, nlines * widthBytesLine, pBuf);
		    linesDone += nlines;
		}
            }
	}
    }
    DEALLOCATE_LOCAL(pBuf);
    return (client->noClientException);
}


int
ProcPolyText(client)
    register ClientPtr client;
{
    int		xorg;
    REQUEST(xPolyTextReq);
    register DrawablePtr pDraw;
    register GC *pGC;
    register FontPtr pFont;

    int (* polyText)();
    register unsigned char *pElt;
    unsigned char *pNextElt;
    unsigned char *endReq;
    int		itemSize;
    
#define TextEltHeader 2
#define FontShiftSize 5

    REQUEST_AT_LEAST_SIZE(xPolyTextReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

    pElt = (unsigned char *)&stuff[1];
    endReq = ((unsigned char *) stuff) + (stuff->length <<2);
    xorg = stuff->x;
    if (stuff->reqType == X_PolyText8)
    {
	polyText = pGC->PolyText8;
	itemSize = 1;
    }
    else
    {
	polyText =  pGC->PolyText16;
	itemSize = 2;
    }

    while (endReq - pElt > TextEltHeader)
    {
	if (*pElt == FontChange)
        {
	    Font	fid;

	    if (endReq - pElt < FontShiftSize)
		 return (BadLength);
	    fid =  ((Font)*(pElt+4))		/* big-endian */
		 | ((Font)*(pElt+3)) << 8
		 | ((Font)*(pElt+2)) << 16
		 | ((Font)*(pElt+1)) << 24;
	    pFont = (FontPtr)LookupID(fid, RT_FONT, RC_CORE);
	    if (!pFont)
	    {
		client->errorValue = fid;
		return (BadFont);
	    }
	    if (pFont != pGC->font)
	    {
		DoChangeGC( pGC, GCFont, &fid, 0);
		ValidateGC(pDraw, pGC);
	    }
	    pElt += FontShiftSize;
	}
	else	/* print a string */
	{
	    pNextElt = pElt + TextEltHeader + (*pElt)*itemSize;
	    if ( pNextElt > endReq)
		return( BadLength);
	    xorg += *((char *)(pElt + 1));	/* must be signed */
	    xorg = (* polyText)(pDraw, pGC, xorg, stuff->y, *pElt,
		pElt + TextEltHeader);
	    pElt = pNextElt;
	}
    }
    return (client->noClientException);
#undef TextEltHeader
#undef FontShiftSize
}

int
ProcImageText(client)
    register ClientPtr client;
{
    register DrawablePtr pDraw;
    register GC *pGC;

    REQUEST(xImageTextReq);

    REQUEST_AT_LEAST_SIZE(xImageTextReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

    (*((stuff->reqType == X_ImageText8) ? pGC->ImageText8 : pGC->ImageText16))
	(pDraw, pGC, stuff->x, stuff->y, stuff->nChars, &stuff[1]);
    return (client->noClientException);
}


int
ProcCreateColormap(client)
    register ClientPtr client;
{
    VisualPtr	pVisual;
    ColormapPtr	pmap;
    Colormap	mid;
    register WindowPtr   pWin;
    REQUEST(xCreateColormapReq);
    int result;

    REQUEST_SIZE_MATCH(xCreateColormapReq);

    if ((stuff->alloc != AllocNone) && (stuff->alloc != AllocAll))
    {
	client->errorValue = stuff->alloc;
        return(BadValue);
    }
    mid = stuff->mid;
    LEGAL_NEW_RESOURCE(mid, client);
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);

    pVisual = (VisualPtr)LookupID(stuff->visual, RT_VISUALID, RC_CORE);
    if ((!pVisual) || pVisual->screen != pWin->drawable.pScreen->myNum)
    {
	client->errorValue = stuff->visual;
	return(BadValue);
    }
    result =  CreateColormap(mid, pWin->drawable.pScreen,
        pVisual, &pmap, (int)stuff->alloc, client->index);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcFreeColormap(client)
    register ClientPtr client;
{
    ColormapPtr pmap;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pmap = (ColormapPtr )LookupID(stuff->id, RT_COLORMAP, RC_CORE);
    if (pmap) 
    {
	/* Freeing a default colormap is a no-op */
	if (!(pmap->flags & IsDefault))
	    FreeResource(stuff->id, RC_NONE);
	return (client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->id;
	return (BadColor);
    }
}


int
ProcCopyColormapAndFree(client)
    register ClientPtr client;
{
    Colormap	mid;
    ColormapPtr	pSrcMap;
    REQUEST(xCopyColormapAndFreeReq);
    int result;

    REQUEST_SIZE_MATCH(xCopyColormapAndFreeReq);
    mid = stuff->mid;
    LEGAL_NEW_RESOURCE(mid, client);
    if(pSrcMap = (ColormapPtr )LookupID(stuff->srcCmap, RT_COLORMAP, RC_CORE))
    {
	result = CopyColormapAndFree(mid, pSrcMap, client->index);
	if (client->noClientException != Success)
            return(client->noClientException);
	else
            return(result);
    }
    else
    {
	client->errorValue = stuff->srcCmap;
	return(BadColor);
    }
}

int
ProcInstallColormap(client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pcmp = (ColormapPtr  )LookupID(stuff->id, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
        (*(pcmp->pScreen->InstallColormap)) (pcmp);
        return (client->noClientException);        
    }
    else
    {
        client->errorValue = stuff->id;
        return (BadColor);
    }
}

int
ProcUninstallColormap(client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pcmp = (ColormapPtr )LookupID(stuff->id, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	if(pcmp->mid != pcmp->pScreen->defColormap)
            (*(pcmp->pScreen->UninstallColormap)) (pcmp);
        return (client->noClientException);        
    }
    else
    {
        client->errorValue = stuff->id;
        return (BadColor);
    }
}

int
ProcListInstalledColormaps(client)
    register ClientPtr client;
{
    xListInstalledColormapsReply *preply; 
    int nummaps;
    WindowPtr pWin;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = (WindowPtr)LookupWindow(stuff->id, client);

    if (!pWin)
        return(BadWindow);

    preply = (xListInstalledColormapsReply *) 
		ALLOCATE_LOCAL(sizeof(xListInstalledColormapsReply) +
		     pWin->drawable.pScreen->maxInstalledCmaps *
		     sizeof(Colormap));
    if(!preply)
        return(BadAlloc);

    preply->type = X_Reply;
    preply->sequenceNumber = client->sequence;
    nummaps = (*pWin->drawable.pScreen->ListInstalledColormaps)
        (pWin->drawable.pScreen, (Colormap *)&preply[1]);
    preply->nColormaps = nummaps;
    preply->length = nummaps;
    WriteReplyToClient(client, sizeof (xListInstalledColormapsReply), preply);
    client->pSwapReplyFunc = Swap32Write;
    WriteSwappedDataToClient(client, nummaps * sizeof(Colormap), &preply[1]);
    DEALLOCATE_LOCAL(preply);
    return(client->noClientException);
}

int
ProcAllocColor                (client)
    register ClientPtr client;
{
    ColormapPtr pmap;
    int	retval;
    xAllocColorReply acr;
    REQUEST(xAllocColorReq);

    REQUEST_SIZE_MATCH(xAllocColorReq);
    pmap = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pmap)
    {
	acr.type = X_Reply;
	acr.length = 0;
	acr.sequenceNumber = client->sequence;
	acr.red = stuff->red;
	acr.green = stuff->green;
	acr.blue = stuff->blue;
	acr.pixel = 0;
	if(retval = AllocColor(pmap, &acr.red, &acr.green, &acr.blue,
	                       &acr.pixel, client->index))
	{
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
	        return (retval);
	}
        WriteReplyToClient(client, sizeof(xAllocColorReply), &acr);
	return (client->noClientException);

    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcAllocNamedColor           (client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xAllocNamedColorReq);

    REQUEST_AT_LEAST_SIZE(xAllocNamedColorReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	int		retval;

	xAllocNamedColorReply ancr;

	ancr.type = X_Reply;
	ancr.length = 0;
	ancr.sequenceNumber = client->sequence;

	if(OsLookupColor(pcmp->pScreen->myNum, (char *)&stuff[1], stuff->nbytes,
	                 &ancr.exactRed, &ancr.exactGreen, &ancr.exactBlue))
	{
	    ancr.screenRed = ancr.exactRed;
	    ancr.screenGreen = ancr.exactGreen;
	    ancr.screenBlue = ancr.exactBlue;
	    ancr.pixel = 0;
	    if(retval = AllocColor(pcmp,
	                 &ancr.screenRed, &ancr.screenGreen, &ancr.screenBlue,
			 &ancr.pixel, client->index))
	    {
                if (client->noClientException != Success)
                    return(client->noClientException);
                else
    	            return(retval);
	    }
            WriteReplyToClient(client, sizeof (xAllocNamedColorReply), &ancr);
	    return (client->noClientException);
	}
	else
	    return(BadName);
	
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcAllocColorCells           (client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xAllocColorCellsReq);

    REQUEST_SIZE_MATCH(xAllocColorCellsReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	xAllocColorCellsReply	accr;
	int			npixels, nmasks, retval;
	long			length;
	unsigned long		*ppixels, *pmasks;

	npixels = stuff->colors;
	nmasks = stuff->planes;
	length = ((long)npixels + (long)nmasks) * sizeof(Pixel);
	ppixels = (Pixel *)ALLOCATE_LOCAL(length);
	if(!ppixels)
            return(BadAlloc);
	pmasks = ppixels + npixels;

	if(retval = AllocColorCells(client->index, pcmp, npixels, nmasks, 
				    (Bool)stuff->contiguous, ppixels, pmasks))
	{
	    DEALLOCATE_LOCAL(ppixels);
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
	        return(retval);
	}
	accr.type = X_Reply;
	accr.length = length >> 2;
	accr.sequenceNumber = client->sequence;
	accr.nPixels = npixels;
	accr.nMasks = nmasks;
        WriteReplyToClient(client, sizeof (xAllocColorCellsReply), &accr);
	client->pSwapReplyFunc = Swap32Write;
        WriteSwappedDataToClient(client, length, ppixels);
	DEALLOCATE_LOCAL(ppixels);
        return (client->noClientException);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcAllocColorPlanes(client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xAllocColorPlanesReq);

    REQUEST_SIZE_MATCH(xAllocColorPlanesReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	xAllocColorPlanesReply	acpr;
	int			npixels, retval;
	long			length;
	unsigned long		*ppixels;

	npixels = stuff->colors;
	acpr.type = X_Reply;
	acpr.sequenceNumber = client->sequence;
	acpr.nPixels = npixels;
	length = (long)npixels * sizeof(Pixel);
	ppixels = (Pixel *)ALLOCATE_LOCAL(length);
	if(!ppixels)
            return(BadAlloc);
	if(retval = AllocColorPlanes(client->index, pcmp, npixels,
	    (int)stuff->red, (int)stuff->green, (int)stuff->blue,
	    (int)stuff->contiguous, ppixels,
	    &acpr.redMask, &acpr.greenMask, &acpr.blueMask))
	{
            DEALLOCATE_LOCAL(ppixels);
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
	        return(retval);
	}
	acpr.length = length >> 2;
	WriteReplyToClient(client, sizeof(xAllocColorPlanesReply), &acpr);
	client->pSwapReplyFunc = Swap32Write;
	WriteSwappedDataToClient(client, length, ppixels);
	DEALLOCATE_LOCAL(ppixels);
        return (client->noClientException);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcFreeColors          (client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xFreeColorsReq);

    REQUEST_AT_LEAST_SIZE(xFreeColorsReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	int	count;
        int     retval;

	if(pcmp->flags & AllAllocated)
	    return(BadAccess);
	count = ((stuff->length << 2)- sizeof(xFreeColorsReq)) >> 2;
	retval =  FreeColors(pcmp, client->index, count,
	    (unsigned long *)&stuff[1], stuff->planeMask);
        if (client->noClientException != Success)
            return(client->noClientException);
        else
	{
	    client->errorValue = clientErrorValue;
            return(retval);
	}

    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcStoreColors               (client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xStoreColorsReq);

    REQUEST_AT_LEAST_SIZE(xStoreColorsReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	int	count;
        int     retval;

        if(pcmp->flags & AllAllocated)
	    if(CLIENT_ID(stuff->cmap) != client->index)
	        return(BadAccess);
        count =
	  ((stuff->length << 2) - sizeof(xStoreColorsReq)) / sizeof(xColorItem);
	retval = StoreColors(pcmp, count, (xColorItem *)&stuff[1]);
        if (client->noClientException != Success)
            return(client->noClientException);
        else
	{
	    client->errorValue = clientErrorValue;
            return(retval);
	}
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcStoreNamedColor           (client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xStoreNamedColorReq);

    REQUEST_AT_LEAST_SIZE(xStoreNamedColorReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	xColorItem	def;
        int             retval;

        if(pcmp->flags & AllAllocated)
	    if(CLIENT_ID(stuff->cmap) != client->index)
	        return(BadAccess);

	if(OsLookupColor(pcmp->pScreen->myNum, (char *)&stuff[1],
	                 stuff->nbytes, &def.red, &def.green, &def.blue))
	{
	    def.flags = stuff->flags;
	    def.pixel = stuff->pixel;
	    retval = StoreColors(pcmp, 1, &def);
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
		return(retval);
	}
        return (BadName);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcQueryColors(client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xQueryColorsReq);

    REQUEST_AT_LEAST_SIZE(xQueryColorsReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	int			count, retval;
	xrgb 			*prgbs;
	xQueryColorsReply	qcr;

	count = ((stuff->length << 2) - sizeof(xQueryColorsReq)) >> 2;
	if(!(prgbs = (xrgb *)ALLOCATE_LOCAL(count * sizeof(xrgb))))
            return(BadAlloc);
	if(retval = QueryColors(pcmp, count, (unsigned long *)&stuff[1], prgbs))
	{
   	    DEALLOCATE_LOCAL(prgbs);
	    if (client->noClientException != Success)
                return(client->noClientException);
	    else
	    {
		client->errorValue = clientErrorValue;
	        return (retval);
	    }
	}
	qcr.type = X_Reply;
	qcr.length = (count * sizeof(xrgb)) >> 2;
	qcr.sequenceNumber = client->sequence;
	qcr.nColors = count;
	WriteReplyToClient(client, sizeof(xQueryColorsReply), &qcr);
	client->pSwapReplyFunc = SQColorsExtend;
	WriteSwappedDataToClient(client, count * sizeof(xrgb), prgbs);
	DEALLOCATE_LOCAL(prgbs);
	return(client->noClientException);
	
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
} 

int
ProcLookupColor(client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    REQUEST(xLookupColorReq);

    REQUEST_AT_LEAST_SIZE(xLookupColorReq);
    pcmp = (ColormapPtr )LookupID(stuff->cmap, RT_COLORMAP, RC_CORE);
    if (pcmp)
    {
	xLookupColorReply lcr;

	if(OsLookupColor(pcmp->pScreen->myNum, (char *)&stuff[1], stuff->nbytes,
	                 &lcr.exactRed, &lcr.exactGreen, &lcr.exactBlue))
	{
	    lcr.type = X_Reply;
	    lcr.length = 0;
	    lcr.sequenceNumber = client->sequence;
	    lcr.screenRed = lcr.exactRed;
	    lcr.screenGreen = lcr.exactGreen;
	    lcr.screenBlue = lcr.exactBlue;
	    (*pcmp->pScreen->ResolveColor)(&lcr.screenRed,
	                                   &lcr.screenGreen,
					   &lcr.screenBlue,
					   pcmp->pVisual);
	    WriteReplyToClient(client, sizeof(xLookupColorReply), &lcr);
	    return(client->noClientException);
	}
        return (BadName);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }
}

int
ProcCreateCursor( client)
    register ClientPtr client;
{
    CursorPtr	pCursor;

    register PixmapPtr 	src;
    register PixmapPtr 	msk;
    unsigned char *	srcbits;
    unsigned char *	mskbits;
    long		width, height;
    long		n;
    CursorMetricRec cm;


    REQUEST(xCreateCursorReq);

    REQUEST_SIZE_MATCH(xCreateCursorReq);
    LEGAL_NEW_RESOURCE(stuff->cid, client);

    src = (PixmapPtr)LookupID( stuff->source, RT_PIXMAP, RC_CORE);
    msk = (PixmapPtr)LookupID( stuff->mask, RT_PIXMAP, RC_CORE);
    if (   src == (PixmapPtr)NULL)
    {
	client->errorValue = stuff->source;
	return (BadPixmap);
    }
    if ( msk == (PixmapPtr)NULL)
    {
	if (stuff->mask != None)
	{
	    client->errorValue = stuff->mask;
	    return (BadPixmap);
	}
    }
    else if (  src->width != msk->width
	    || src->height != msk->height
	    || src->drawable.depth != 1
	    || msk->drawable.depth != 1)
	return (BadMatch);

    width = src->width;
    height = src->height;

    if ( stuff->x > width 
      || stuff->y > height )
	return (BadMatch);

    n = PixmapBytePad(width, 1)*height;
    srcbits = (unsigned char *)xalloc(n);
    mskbits = (unsigned char *)xalloc(n);

    (* src->drawable.pScreen->GetImage)( src, 0, 0, width, height,
					 XYPixmap, 1, srcbits);
    if ( msk == (PixmapPtr)NULL)
    {
	register unsigned char *bits = mskbits;
	while (--n >= 0)
	    *bits++ = ~0;
    }
    else
	(* msk->drawable.pScreen->GetImage)( msk, 0, 0, width, height,
					     XYPixmap, 1, mskbits);
    cm.width = width;
    cm.height = height;
    cm.xhot = stuff->x;
    cm.yhot = stuff->y;
    pCursor = AllocCursor( srcbits, mskbits, &cm,
	    stuff->foreRed, stuff->foreGreen, stuff->foreBlue,
	    stuff->backRed, stuff->backGreen, stuff->backBlue);

    AddResource( stuff->cid, RT_CURSOR, (pointer)pCursor, FreeCursor, RC_CORE);
    return (client->noClientException);
}

/*
 * protocol requires positioning of glyphs so hot-spots are coincident	XXX
 */
int
ProcCreateGlyphCursor( client)
    register ClientPtr client;
{
    FontPtr  sourcefont;
    FontPtr  maskfont;
    unsigned char   *srcbits;
    unsigned char   *mskbits;
    CursorPtr pCursor;
    CursorMetricRec cm;
    int res;

    REQUEST(xCreateGlyphCursorReq);

    REQUEST_SIZE_MATCH(xCreateGlyphCursorReq);
    LEGAL_NEW_RESOURCE(stuff->cid, client);

    sourcefont = (FontPtr) LookupID(stuff->source, RT_FONT, RC_CORE);
    maskfont = (FontPtr) LookupID(stuff->mask, RT_FONT, RC_CORE);

    if (sourcefont == (FontPtr) NULL)
    {
	client->errorValue = stuff->source;
	return(BadFont);
    }
    if (!CursorMetricsFromGlyph(sourcefont, stuff->sourceChar, &cm))
    {
	client->errorValue = stuff->sourceChar;
	return BadValue;
    }
    if (maskfont == (FontPtr) NULL)
    {
	register long n;
	register unsigned char *bits;

	if (stuff->mask != None)
	{
	    client->errorValue = stuff->mask;
	    return(BadFont);
	}
	n = PixmapBytePad(cm.width, 1)*cm.height;
	bits = mskbits = (unsigned char *)xalloc(n);
	while (--n >= 0)
	    *bits++ = ~0;
    }
    else
    {
	if (!CursorMetricsFromGlyph(maskfont, stuff->maskChar, &cm))
	{
	    client->errorValue = stuff->maskChar;
	    return BadValue;
	}
	if (res = ServerBitsFromGlyph(stuff->mask, 
				      maskfont, stuff->maskChar,
				      &cm, &mskbits))
	    return res;
    }
    if (res = ServerBitsFromGlyph(stuff->source, 
				  sourcefont, stuff->sourceChar,
				  &cm, &srcbits))
    {
	xfree(mskbits);
	return res;
    }

    pCursor = AllocCursor(srcbits, mskbits, &cm,
	    stuff->foreRed, stuff->foreGreen, stuff->foreBlue,
	    stuff->backRed, stuff->backGreen, stuff->backBlue);

    AddResource(stuff->cid, RT_CURSOR, (pointer)pCursor, FreeCursor, RC_CORE);
    return client->noClientException;
}


int
ProcFreeCursor(client)
    register ClientPtr client;
{
    CursorPtr pCursor;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pCursor = (CursorPtr)LookupID(stuff->id, RT_CURSOR, RC_CORE);
    if (pCursor) 
    {
	FreeResource( stuff->id, RC_NONE);
	return (client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->id;
	return (BadCursor);
    }
}

int
ProcQueryBestSize   (client)
    register ClientPtr client;
{
    xQueryBestSizeReply	reply;
    register DrawablePtr pDraw;
    ScreenPtr pScreen;
    REQUEST(xQueryBestSizeReq);

    REQUEST_SIZE_MATCH(xQueryBestSizeReq);
    if ((stuff->class != CursorShape) && 
	(stuff->class != TileShape) && 
	(stuff->class != StippleShape))
    {
	client->errorValue = stuff->class;
        return(BadValue);
    }
    if (!(pDraw = LOOKUP_DRAWABLE(stuff->drawable, client)))
    {
	client->errorValue = stuff->drawable;
	return (BadDrawable);
    }
    pScreen = pDraw->pScreen;
    (* pScreen->QueryBestSize)(stuff->class, &stuff->width,
			       &stuff->height);
    reply.type = X_Reply;
    reply.length = 0;
    reply.sequenceNumber = client->sequence;
    reply.width = stuff->width;
    reply.height = stuff->height;
    WriteReplyToClient(client, sizeof(xQueryBestSizeReply), &reply);
    return (client->noClientException);
}


int
ProcSetScreenSaver            (client)
    register ClientPtr client;
{
    int blankingOption, exposureOption;
    REQUEST(xSetScreenSaverReq);

    REQUEST_SIZE_MATCH(xSetScreenSaverReq);
    blankingOption = stuff->preferBlank;
    if ((blankingOption != DontPreferBlanking) &&
        (blankingOption != PreferBlanking) &&
        (blankingOption != DefaultBlanking))
        return BadMatch;

    exposureOption = stuff->allowExpose;
    if ((exposureOption != DontAllowExposures) &&
        (exposureOption != AllowExposures) &&
        (exposureOption != DefaultExposures))
        return BadMatch;

    if ((stuff->timeout < -1) || (stuff->interval < -1))
        return BadMatch;

    if (blankingOption == DefaultBlanking)
	ScreenSaverBlanking = defaultScreenSaverBlanking;
    else
	ScreenSaverBlanking = blankingOption; 
    if (exposureOption == DefaultExposures)
	ScreenSaverAllowExposures = defaultScreenSaverAllowExposures;
    else
	ScreenSaverAllowExposures = exposureOption;

    if (stuff->timeout >= 0)
	ScreenSaverTime = stuff->timeout * MILLI_PER_SECOND;
    else 
	ScreenSaverTime = defaultScreenSaverTime;
    if (stuff->interval >= 0)
	ScreenSaverInterval = stuff->interval * MILLI_PER_SECOND;
    else
	ScreenSaverInterval = defaultScreenSaverInterval;
    return (client->noClientException);
}

int
ProcGetScreenSaver(client)
    register ClientPtr client;
{
    xGetScreenSaverReply rep;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.timeout = ScreenSaverTime / MILLI_PER_SECOND;
    rep.interval = ScreenSaverInterval / MILLI_PER_SECOND;
    rep.preferBlanking = ScreenSaverBlanking;
    rep.allowExposures = ScreenSaverAllowExposures;
    WriteReplyToClient(client, sizeof(xGetScreenSaverReply), &rep);
    return (client->noClientException);
}

int
ProcChangeHosts(client)
    register ClientPtr client;
{
    REQUEST(xChangeHostsReq);
    int result;

    REQUEST_AT_LEAST_SIZE(xChangeHostsReq);

    if(stuff->mode == HostInsert)
	result = AddHost(client, (int)stuff->hostFamily,
			 stuff->hostLength, (pointer)&stuff[1]);
    else if (stuff->mode == HostDelete)
	result = RemoveHost(client, (int)stuff->hostFamily, 
			    stuff->hostLength, (pointer)&stuff[1]);  
    else
    {
	client->errorValue = stuff->mode;
        return BadValue;
    }
    if (!result)
	result = client->noClientException;
    return (result);
}

int
ProcListHosts(client)
    register ClientPtr client;
{
extern int GetHosts();
    xListHostsReply reply;
    int	len, nHosts;
    pointer	pdata;
    REQUEST(xListHostsReq);

    REQUEST_SIZE_MATCH(xListHostsReq);
    if((len = GetHosts(&pdata, &nHosts, &reply.enabled)) < 0)
	return(BadImplementation);
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.nHosts = nHosts;
    reply.length = len >> 2;
    WriteReplyToClient(client, sizeof(xListHostsReply), &reply);
    client->pSwapReplyFunc = SLHostsExtend;
    WriteSwappedDataToClient(client, len, pdata);
    xfree(pdata);
    return (client->noClientException);
}

int
ProcChangeAccessControl(client)
    register ClientPtr client;
{
    int result;
    REQUEST(xSetAccessControlReq);

    REQUEST_SIZE_MATCH(xSetAccessControlReq);
    if ((stuff->mode != EnableAccess) && (stuff->mode != DisableAccess))
    {
	client->errorValue = stuff->mode;
        return BadValue;
    }
    result = ChangeAccessControl(client, stuff->mode == EnableAccess);
    if (!result)
	result = client->noClientException;
    return (result);
}

int
ProcKillClient(client)
    register ClientPtr client;
{
    REQUEST(xResourceReq);

    pointer *pResource;
    int clientIndex, myIndex;

    REQUEST_SIZE_MATCH(xResourceReq);
    if (stuff->id == AllTemporary)
    {
	CloseDownRetainedResources();
        return (client->noClientException);
    }
    pResource = (pointer *)LookupID(stuff->id, RT_ANY, RC_CORE);
  
    clientIndex = CLIENT_ID(stuff->id);

    if (clientIndex && pResource)
    {
	myIndex = client->index;
    	if (clients[clientIndex])
 	{
	    CloseDownClient(clients[clientIndex]);
	}
	if (myIndex == clientIndex)
	{
	    /* force yield and return Success, so that Dispatch()
	     * doesn't try to touch client
	     */
	    isItTimeToYield = TRUE;
	    return (Success);
	}
	return (client->noClientException);
    }
    else   /* can't kill client 0, which is server */
    {
	client->errorValue = stuff->id;
	return (BadValue);
    }
}

int
ProcSetFontPath(client)
    register ClientPtr client;
{
    REQUEST(xSetFontPathReq);
    
    REQUEST_AT_LEAST_SIZE(xSetFontPathReq);
    
    SetFontPath(stuff->nFonts, stuff->length, (char *)&stuff[1]);
    return (client->noClientException);
}

int
ProcGetFontPath(client)
    register ClientPtr client;
{
    FontPathPtr pFP;
    xGetFontPathReply reply;
    int stringLens, i;
    char *bufferStart;
    register char  *bufptr;
    REQUEST (xReq);

    REQUEST_SIZE_MATCH(xReq);
    pFP = GetFontPath();
    stringLens = 0;
    for (i=0; i<pFP->npaths; i++)
        stringLens += pFP->length[i];

    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = (stringLens + pFP->npaths + 3) >> 2;
    reply.nPaths = pFP->npaths;

    bufptr = bufferStart = (char *)ALLOCATE_LOCAL(reply.length << 2);
    if(!bufptr)
        return(BadAlloc);
            /* since WriteToClient long word aligns things, 
	       copy to temp buffer and write all at once */
    for (i=0; i<pFP->npaths; i++)
    {
        *bufptr++ = pFP->length[i];
        bcopy(pFP->paths[i], bufptr,  pFP->length[i]);
        bufptr += pFP->length[i];
    }
    WriteReplyToClient(client, sizeof(xGetFontPathReply), &reply);
    (void)WriteToClient(client, stringLens + pFP->npaths, bufferStart);
    DEALLOCATE_LOCAL(bufferStart);
    return(client->noClientException);
}

int
ProcChangeCloseDownMode(client)
    register ClientPtr client;
{
    REQUEST(xSetCloseDownModeReq);

    REQUEST_SIZE_MATCH(xSetCloseDownModeReq);
    if ((stuff->mode == AllTemporary) ||
	(stuff->mode == RetainPermanent) ||
	(stuff->mode == RetainTemporary))
    {
	client->closeDownMode = stuff->mode;
	return (client->noClientException);
    }
    else   
    {
	client->errorValue = stuff->mode;
	return (BadValue);
    }
}

int ProcForceScreenSaver(client)
    register ClientPtr client;
{    
    REQUEST(xForceScreenSaverReq);

    REQUEST_SIZE_MATCH(xForceScreenSaverReq);
    
    if ((stuff->mode != ScreenSaverReset) && 
	(stuff->mode != ScreenSaverActive))
    {
	client->errorValue = stuff->mode;
        return BadValue;
    }
    SaveScreens(SCREEN_SAVER_FORCER, (int)stuff->mode);
    return client->noClientException;
}

int ProcNoOperation(client)
    register ClientPtr client;
{
    REQUEST(xReq);

    REQUEST_AT_LEAST_SIZE(xReq);
    
    /* noop -- don't do anything */
    return(client->noClientException);
}

void
InitProcVectors()
{
    int i;
    for (i = 0; i<256; i++)
    {
	if(!ProcVector[i])
	{
            ProcVector[i] = SwappedProcVector[i] = ProcBadRequest;
	    ReplySwapVector[i] = NotImplemented;
	}
    }
    for(i = LASTEvent; i < 128; i++)
    {
	EventSwapVector[i] = NotImplemented;
    }
    
}

/**********************
 * CloseDownClient
 *
 *  Client can either mark his resources destroy or retain.  If retained and
 *  then killed again, the client is really destroyed.
 *********************/

void
CloseDownClient(client)
    register ClientPtr client;
{
    register int i;
      /* ungrab server if grabbing client dies */
    if (grabbingClient &&  (onlyClient == client))
    {
	grabbingClient = FALSE;
	ListenToAllClients();
    }
    DeleteClientFromAnySelections(client);
    ReleaseActiveGrabs(client);
    
    if (client->closeDownMode == DestroyAll)
    {
        client->clientGone = TRUE;  /* so events aren't sent to client */
        CloseDownConnection(client);
        FreeClientResources(client);
	nextFreeClientID = client->index;
	clients[client->index] = NullClient;
        xfree(client);
	if(--nClients == 0)
	    nClients = -1;
    }
            /* really kill resources this time */
    else if (client->clientGone)
    {
        FreeClientResources(client);
	nextFreeClientID = client->index;
	clients[client->index] = NullClient;
        xfree(client);
    }
    else
    {
        client->clientGone = TRUE;
        CloseDownConnection(client);
	--nClients;
    }
}

static void
KillAllClients()
{
    int i;
    for (i=1; i<currentMaxClients; i++)
        if (clients[i])
            CloseDownClient(clients[i]);     
}

void
KillServerResources()
{
    int i;

    KillAllClients();
    CloseDownExtensions();
    /* Good thing we stashed these two in globals so we could get at them
     * here. */
    CloseDownDevices(argcGlobal, argvGlobal);
    for (i = 0; i < screenInfo.numScreens; i++)
	(*screenInfo.screen[i].CloseScreen)(i, &screenInfo.screen[i]);
}


/*********************
 * CloseDownRetainedResources
 *
 *    Find all clients that are gone and have terminated in RetainTemporary 
 *    and  destroy their resources.
 *********************/

CloseDownRetainedResources()
{
    register int i;
    register ClientPtr client;

    for (i=1; i<currentMaxClients; i++)
    {
        client = clients[i];
        if (client && (client->closeDownMode == RetainTemporary)
	    && (client->clientGone))
	{
            FreeClientResources(client);
            nextFreeClientID = i;
	    xfree(client);
	    clients[i] = NullClient;
	}
    }
}

/************************
 * int NextAvailableClientID()
 *
 * OS depedent portion can't assign client id's because of CloseDownModes.
 * Returns NULL if the there are no free clients.
 *************************/

ClientPtr
NextAvailableClient()
{
    register int i;
    register ClientPtr client;

    i = nextFreeClientID;
    while (clients[i])
    {
	i++;
	if (i >= currentMaxClients)
	    i = 1;
	if (i != nextFreeClientID)
	    continue;
	if (currentMaxClients == MAXCLIENTS)
	    return (ClientPtr) NULL;
	i = currentMaxClients;
	currentMaxClients++;
	clients = (ClientPtr *)xrealloc(clients,
					currentMaxClients * sizeof(ClientPtr));
	break;
    }
    nextFreeClientID = i + 1;
    if (nextFreeClientID == currentMaxClients)
	nextFreeClientID = 1;

    clients[i] = client =  (ClientPtr)xalloc(sizeof(ClientRec));
    client->index = i;
    client->sequence = 0; 
    client->clientAsMask = ((Mask)i) << CLIENTOFFSET;
    client->closeDownMode = DestroyAll;
    client->clientGone = FALSE;
    client->lastDrawable = (DrawablePtr) NULL;
    client->lastDrawableID = INVALID;
    client->lastGC = (GCPtr) NULL;
    client->lastGCID = INVALID;
    client->numSaved = 0;
    client->saveSet = (pointer *)NULL;
    client->noClientException = Success;

    return(client);
}

SendConnectionSetupInfo(client)
    ClientPtr client;
{
    register xWindowRoot *root;
    register int i;

    ((xConnSetup *)ConnectionInfo)->ridBase = client->clientAsMask;
    ((xConnSetup *)ConnectionInfo)->ridMask = 0xfffff;
        /* fill in the "currentInputMask" */
    root = (xWindowRoot *)(ConnectionInfo + connBlockScreenStart);
    for (i=0; i<screenInfo.numScreens; i++) 
    {
	register int j;
	register xDepth *pDepth;

        root->currentInputMask = WindowTable[i].allEventMasks;
	pDepth = (xDepth *)(root + 1);
	for (j = 0; j < root->nDepths; j++)
	{
	    pDepth = (xDepth *)(((char *)(pDepth + 1)) +
				pDepth->nVisuals * sizeof(xVisualType));
	}
	root = (xWindowRoot *)pDepth;
    }

    if (client->swapped) {
	WriteSConnSetupPrefix(client, &connSetupPrefix);
        WriteSConnectionInfo(client, connSetupPrefix.length << 2, ConnectionInfo);
	}
    else {
        (void)WriteToClient(client, sizeof(xConnSetupPrefix),
				(char *) &connSetupPrefix);
        (void)WriteToClient(client, connSetupPrefix.length << 2,
				ConnectionInfo);
	}
}

/*****************
 * Oops
 *    Send an Error back to the client. 
 *****************/

Oops (client, reqCode, minorCode, status)
    ClientPtr client;
    unsigned reqCode, minorCode;
    int status;
{
    xError rep;

    rep.type = X_Error;
    rep.sequenceNumber = client->sequence;
    rep.errorCode = status;
    rep.majorCode = reqCode;
    rep.minorCode = minorCode;
    rep.resourceID = client->errorValue;

#ifdef notdef
    ErrorF(  "OOPS! => client: %x, seq: %d, err: %d, maj:%d, min: %d resID: %x\n",
    	client->index, rep.sequenceNumber, rep.errorCode,
	rep.majorCode, rep.minorCode, rep.resourceID);
#endif

    WriteEventsToClient (client, 1, (xEvent *) &rep); 
}


void
DeleteWindowFromAnySelections(pWin)
    WindowPtr pWin;
{
    register int i;

    for (i = 0; i< NumCurrentSelections; i++)
        if (CurrentSelections[i].pWin == pWin)
        {
            CurrentSelections[i].pWin = (WindowPtr)NULL;
            CurrentSelections[i].window = None;
	    CurrentSelections[i].client = NullClient;
	}
}

static void
DeleteClientFromAnySelections(client)
    ClientPtr client;
{
    register int i;

    for (i = 0; i< NumCurrentSelections; i++)
        if (CurrentSelections[i].client == client)
        {
            CurrentSelections[i].pWin = (WindowPtr)NULL;
            CurrentSelections[i].window = None;
	    CurrentSelections[i].client = NullClient;
	}
}

void
MarkClientException(client)
    ClientPtr client;
{
    client->noClientException = -1;
}
