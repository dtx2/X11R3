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

/* $XConsortium: resource.c,v 1.67 88/12/22 16:58:58 rws Exp $ */

/*	Routines to manage various kinds of resources:
 *
 *	CreateNewResourceType, CreateNewResourceClass, InitClientResources,
 *	FakeClientID, AddResource, FreeResource, FreeClientResources,
 *	FreeAllResources, LookupID
 */

/* 
 *      a resource is a 32 bit quantity.  the upper 12 bits are client id.  
 *      client provides a 19 bit resource id. this is "hashed" by me by
 *      taking the 10 lower bits and xor'ing with the mid 10 bits.
 *
 *      It is sometimes necessary for the server to create an ID that looks
 *      like it belongs to a client.  This ID, however,  must not be one
 *      the client actually can create, or we have the potential for conflict.
 *      The 20th bit of the ID is resevered for the server's use for this
 *      purpose.  By setting CLIENT_ID(id) to the client, the SERVER_BIT to
 *      1, and an otherwise unused ID in the low 19 bits, we can create a
 *      resource "owned" by the client.
 *      
 *      The following IDs are currently reserved for siccing on the client:
 *      1 - allocated color to be freed when the client dies
 */

#include "X.h"
#include "misc.h"
#include "os.h"
#include "resource.h"
#include "dixstruct.h" 
#include "opaque.h"

extern void HandleSaveSet();
extern void FlushClientCaches();

#define CACHEDTYPES (RT_WINDOW | RT_PIXMAP | RT_GC)
#define INITBUCKETS 64
#define INITHASHSIZE 6
#define MAXHASHSIZE 11

typedef struct _Resource {
    struct _Resource	*next;
    XID			id;
    int			(*DeleteFunc)();
    unsigned short	type;
    unsigned short	class;
    pointer		value;
} ResourceRec, *ResourcePtr;
#define NullResource ((ResourcePtr)NULL)

typedef struct _ClientResource {
    ResourcePtr *resources;
    int		elements;
    int		buckets;
    int		hashsize;	/* log(2)(buckets) */
    XID		fakeID;
} ClientResourceRec;

static unsigned short lastResourceType;
static unsigned short lastResourceClass;

unsigned short
CreateNewResourceType()
{
    if (lastResourceType == 0x8000)	/* this is compiler dependent  XXX */
	lastResourceType = 0;
    lastResourceType <<= 1;
    return lastResourceType;
}

short
CreateNewResourceClass()
{
    return ++lastResourceClass;
}

ClientResourceRec clientTable[MAXCLIENTS];

/*****************
 * InitClientResources
 *    When a new client is created, call this to allocate space
 *    in resource table
 *****************/

InitClientResources(client)
    ClientPtr client;
{
    register int i, j;
 
    if (client == serverClient)
    {
	lastResourceType = RT_LASTPREDEF;
	lastResourceClass = RC_LASTPREDEF;
    }
    clientTable[i = client->index].resources =
	(ResourcePtr *)xalloc(INITBUCKETS*sizeof(ResourcePtr));
    clientTable[i].buckets = INITBUCKETS;
    clientTable[i].elements = 0;
    clientTable[i].hashsize = INITHASHSIZE;
    clientTable[i].fakeID = 100;
    for (j=0; j<INITBUCKETS; j++) 
    {
        clientTable[i].resources[j] = NullResource;
    }
}

static int
Hash(client, id)
    int client;
    register XID id;
{
    id &= RESOURCE_ID_MASK;
    switch (clientTable[client].hashsize)
    {
	case 6:
	    return ((int)(0x03F & (id ^ (id>>6) ^ (id>>12))));
	case 7:
	    return ((int)(0x07F & (id ^ (id>>7) ^ (id>>13))));
	case 8:
	    return ((int)(0x0FF & (id ^ (id>>8) ^ (id>>16))));
	case 9:
	    return ((int)(0x1FF & (id ^ (id>>9))));
	case 10:
	    return ((int)(0x3FF & (id ^ (id>>10))));
	case 11:
	    return ((int)(0x7FF & (id ^ (id>>11))));
    }
    return -1;
}

XID
FakeClientID(client)
    int client;
{
	return (
	    (client<<CLIENTOFFSET) + (SERVER_BIT) +
	    ((clientTable[client].fakeID++) & RESOURCE_ID_MASK));
}

void
AddResource(id, type, value, func, class)
    XID id;
    unsigned short type, class;
    pointer value;
    int (* func)();
{
    int client;
    register int j;
    register ResourcePtr res, next, *head;
    	
    client = CLIENT_ID(id);
    if (!clientTable[client].buckets)
    {
	ErrorF("AddResource(%x, %d, %x, %d), client=%d \n",
		id, type, value, class, client);
        FatalError("client not in use\n");
    }
    if (!func)
    {
	ErrorF("AddResource(%x, %d, %x, %d), client=%d \n",
		id, type, value, class, client);
        FatalError("No delete function given to AddResource \n");
    }
    if ((clientTable[client].elements >= 4*clientTable[client].buckets) &&
	(clientTable[client].hashsize < MAXHASHSIZE))
    {
	register ResourcePtr *resources = (ResourcePtr *)
	    xalloc(2*clientTable[client].buckets*sizeof(ResourcePtr));
	for (j = 0; j < clientTable[client].buckets*2; j++)
	    resources[j] = NullResource;
	clientTable[client].hashsize++;
	for (j = 0; j < clientTable[client].buckets; j++)
	{
	    /*
	     * Must preserve insertion order so that FreeResource doesn't free
	     * "subclasses" before main resources are freed.  Sigh.
	     */
	    for (res = clientTable[client].resources[j]; res; res = next)
	    {
		next = res->next;
		head = &resources[Hash(client, res->id)];
		while (*head)
		    head = &(*head)->next;
		*head = res;
		res->next = NullResource;
	    }
	}
	clientTable[client].buckets *= 2;
	xfree(clientTable[client].resources);
	clientTable[client].resources = resources;
    }
    head = &clientTable[client].resources[Hash(client, id)];
    res = (ResourcePtr)xalloc(sizeof(ResourceRec));
    res->next = *head;
    res->id = id;
    res->DeleteFunc = func;
    res->type = type;
    res->class = class;
    res->value = value;
    *head = res;
    clientTable[client].elements++;
}

void
FreeResource(id, skipDeleteFuncClass)
XID id;
int skipDeleteFuncClass;

{
    int		cid;
    register    ResourcePtr res;
    register	ResourcePtr *prev, *head;
    register	int *eltptr;
    int		elements;
    Bool	gotOne = FALSE;

    if (((cid = CLIENT_ID(id)) < MaxClients) && clientTable[cid].buckets)
    {
	head = &clientTable[cid].resources[Hash(cid, id)];
	eltptr = &clientTable[cid].elements;

	prev = head;
	while (res = *prev)
	{
	    if (res->id == id)
	    {
		*prev = res->next;
		elements = --*eltptr;
		if (res->type & CACHEDTYPES)
		    FlushClientCaches(res->id);
		if (skipDeleteFuncClass != res->class)
		    (*res->DeleteFunc) (res->value, res->id);
		xfree(res);
		if (*eltptr != elements)
		    prev = head; /* prev may no longer be valid */
		gotOne = TRUE;
	    }
	    else
		prev = &res->next;
        }
	if(clients[cid] && (id == clients[cid]->lastDrawableID))
	{
	    clients[cid]->lastDrawable = (DrawablePtr) NULL;
	    clients[cid]->lastDrawableID = INVALID;
	}
    }
    if (!gotOne)
	FatalError("Freeing resource id=%X which isn't there", id);
}

void
FreeClientResources(client)
    ClientPtr client;
{
    register ResourcePtr *resources;
    register ResourcePtr this;
    int j;

    /* This routine shouldn't be called with a null client, but just in
	case ... */

    if (!client)
	return;

    HandleSaveSet(client);

    resources = clientTable[client->index].resources;
    for (j=0; j < clientTable[client->index].buckets; j++) 
    {
        /* It may seem silly to update the head of this resource list as
	we delete the members, since the entire list will be deleted any way, 
	but there are some resource deletion functions "FreeClientPixels" for 
	one which do a LookupID on another resource id (a Colormap id in this
	case), so the resource list must be kept valid up to the point that
	it is deleted, so every time we delete a resource, we must update the
	head, just like in FreeResource. I hope that this doesn't slow down
	mass deletion appreciably. PRH */

	ResourcePtr *head;

	head = &resources[j];

        for (this = *head; this; this = *head)
	{
	    *head = this->next;
	    if (this->type & CACHEDTYPES)
		FlushClientCaches(this->id);
	    (*this->DeleteFunc)(this->value, this->id);
	    xfree(this);	    
	}
    }
    xfree(clientTable[client->index].resources);
    clientTable[client->index].buckets = 0;
}

FreeAllResources()
{
    int	i;

    for (i=0; i<currentMaxClients; i++) 
    {
        if (clientTable[i].buckets) 
	    FreeClientResources(clients[i]);
    }
}

/*
 *  LookupID returns the value field in the resource or NULL
 *  if an illegal id was handed to it or given type doesn't match the
 *  type for this id and class.
 */ 
pointer
LookupID(id, rType, class)
    XID id;
    unsigned short rType, class;
{
    int    cid;
    register    ResourcePtr res;

    if (((cid = CLIENT_ID(id)) < MaxClients) && clientTable[cid].buckets)
    {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if ((res->id == id) && (res->class == class))
		if (res->type & rType)
		    return res->value;
		else
		    return(pointer) NULL;
    }
    return(pointer) NULL;
}
