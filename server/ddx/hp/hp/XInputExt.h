#ifndef XINPUTEXT_H
#define XINPUTEXT_H
#include "X.h"
#include "XHPproto.h"
#include "misc.h"
#include "dixstruct.h"
/**********************************************************
 *
 * extension structures.
 *
 */

typedef struct _ExtOtherClients *ExtOtherClientsPtr;
typedef struct _DeviceClients *DeviceClientsPtr;
typedef struct _DeviceMasks *DeviceMasksPtr;

typedef struct _ExtOtherClients {
    ExtOtherClientsPtr	next;
    ClientPtr		client;	  /* which client is selecting on this window */
    XID			resource; /* id for putting into resource manager */
    Mask		mask[MAX_LOGICAL_DEVS];
} ExtOtherClients;

typedef struct _DeviceClients {
    DeviceClientsPtr	next;
    ClientPtr		client;	  /* which client wants this device       */
    XID			resource; /* id for putting into resource manager */
    int			mode;
} DeviceClients;

typedef struct _DeviceMasks {
    ClientPtr 		client;
    Mask		dontPropagateMask;
    Mask		allEventMasks;
    Mask		deliverableEvents;
    Mask		eventMask[MAX_LOGICAL_DEVS];
    ExtOtherClientsPtr	otherClients;
} DeviceMasks;
#endif
