#ifndef HPEXT_H
#define HPEXT_H
/* $XConsortium: hpext.h,v 1.2 88/09/06 15:24:45 jim Exp $ */

/* Definitions for HP extensions used by the server and Xlib */

/*********************************************************
 *
 * number of hp events, errors, and extension name.
 *
 */

#define HPEVENTS		11
#define HPERRORS		3
#define CLIENT_REQ		1
#define HPNAME 			"HPExtension"
#define MIN_EVENT_REQUEST	1
#define MAX_EVENT_REQUEST	10

/*********************************************************
 *
 * Protocol request constants
 *
 */

#define X_GetHpKeyboardId		1	/* DO NOT CHANGE THIS LINE! */
#define X_HPListInputDevices		2
#define X_HPSetInputDevice		3
#define X_HPGetExtEventMask		4
#define X_HPGetDeviceFocus		5
#define X_HPGetClipList			6
#define X_HPGrabDevice 			7
#define X_HPSetDeviceFocus		8
#define X_HPUnGrabDevice  		9
#define X_HPSelectExtensionEvent  	10
#define X_HPGetCurrentDeviceMask  	11
#define X_HPEnableReset 		12
#define X_HPDisableReset 		13
#define X_HPGetDeviceMotionEvents 	14

/*********************************************************
 *
 * Protocol request and reply structures.
 *
 */

typedef struct {
    CARD8 reqType;          /* always HpReqCode */
    CARD8 hpReqType;        /* always X_HPListInputDevices */
    CARD16 length;
} xHPListInputDevicesReq;

typedef struct {
    CARD8 repType;  		/* X_Reply */
    CARD8 hpRepType;        	/* always X_HPListInputDevices  */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 ndevices B32;
    CARD32 t_axes B32;
    CARD32 data02 B32;
    CARD32 data03 B32;
    CARD32 data04 B32;
    CARD32 data05 B32;
    } xHPListInputDevicesReply;


typedef struct {
    CARD8 reqType;          /* always HpReqCode */
    CARD8 hpReqType;        /* always X_HPSetInputDevice */
    CARD16 length;
    XID    deviceid;
    CARD32 mode;
} xHPSetInputDeviceReq;

typedef struct {
    CARD8 repType;  		/* X_Reply */
    CARD8 hpRepType;        	/* always X_HPSetInputDevice  */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 status B32;
    CARD32 data01 B32;
    CARD32 data02 B32;
    CARD32 data03 B32;
    CARD32 data04 B32;
    CARD32 data05 B32;
    } xHPSetInputDeviceReply;


typedef struct {
    CARD8 reqType;          /* always HpReqCode */
    CARD8 hpReqType;        /* always X_HPGetExtEventMask */
    CARD16 length;
    CARD32 evconst;
} xHPGetExtEventMaskReq;

typedef struct {
    CARD8 repType;  		/* X_Reply */
    CARD8 hpRepType;        	/* always X_HPGetExtEventMask  */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 mask   B32;
    CARD32 evtype B32;
    CARD32 data01 B32;
    CARD32 data02 B32;
    CARD32 data03 B32;
    CARD32 data04 B32;
    } xHPGetExtEventMaskReply;

typedef struct {
    CARD8 reqType;          /* always HpeqCode */
    CARD8 hpReqType;        /* always X_HPGetCurrentDeviceMask */
    CARD16 length;
    Window window B32;
    XID    deviceid;
} xHPGetCurrentDeviceMaskReq;

typedef struct {
    CARD8 repType;  		/* X_Reply */
    CARD8 hpRepType;        	/* always X_HPGetCurrentDeviceMask  */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 mask   B32;
    CARD32 data01 B32;
    CARD32 data02 B32;
    CARD32 data03 B32;
    CARD32 data04 B32;
    CARD32 data05 B32;
    } xHPGetCurrentDeviceMaskReply;


typedef struct {
    CARD8 reqType;          /* always HpReqCode */
    CARD8 hpReqType;        /* always X_HPGetDeviceFocus */
    CARD16 length;
    XID  deviceid;
    CARD32 mode;
} xHPGetDeviceFocusReq;

typedef struct {
    CARD8 repType;  		/* X_Reply */
    CARD8 hpRepType;        	/* always X_HPGetDeviceFocus  */
    CARD16 sequenceNumber B16;
    CARD32 length 	B32;
    CARD32 status 	B32;
    CARD32 focus  	B32;
    Window revertTo 	B32;
    CARD32 data01 	B32;
    CARD32 data02 	B32;
    CARD32 data03 	B32;
    } xHPGetDeviceFocusReply;

typedef struct {
    CARD8 reqType;          /* always HpReqCode */
    CARD8 hpReqType;        /* always X_HPSetDeviceFocus */
    CARD16 length;
    Window focus  	B32;
    XID    deviceid;
    Time   time 	B32;
    CARD8  revertTo;
    CARD8  pad00;
    CARD16 pad01;
} xHPSetDeviceFocusReq;

typedef struct {
    CARD8	reqType;
    CARD8 	hpReqType;        /* always X_HPGrabDevice */
    CARD16 	length B16;
    Window 	grabWindow B32;
    Time 	time B32;
    XID   	deviceid;
    CARD32 	eventMask B32;
    BOOL 	ownerEvents;
    CARD8	pad00;
    CARD16 	pad01 B16;
} xHPGrabDeviceReq;

typedef struct {
    CARD8 repType;  		/* X_Reply */
    CARD8 hpRepType;        	/* always X_HPGrabDevice  */
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    CARD32 status;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xHPGrabDeviceReply;


typedef struct {
    CARD8	reqType;
    CARD8 	hpReqType;        /* always X_HPUnGrabDevice */
    CARD16 	length B16;
    Time 	time B32;
    XID   	deviceid;
} xHPUnGrabDeviceReq;

typedef struct {
    CARD8	reqType;
    CARD8 	hpReqType;        /* always X_HPSelectExtensionEvent */
    CARD16 	length B16;
    Window 	window B32;
    CARD32	extensionMask B32;
    XID		deviceid;
} xHPSelectExtensionEventReq;

typedef struct {
    CARD8	reqType;
    CARD8 	hpReqType;        /* always X_HPEnableReset          */
    CARD16 	length B16;
} xHPEnableResetReq;

typedef struct {
    CARD8	reqType;
    CARD8 	hpReqType;        /* always X_HPDisableReset          */
    CARD16 	length B16;
} xHPDisableResetReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	hpReqType;        /* always X_HPGetDeviceMotionEvents*/
    CARD16 	length B16;
    Window 	window B32;
    Time 	start B32;
    Time	stop B32;
    XID		deviceid;
} xHPGetDeviceMotionEventsReq;

typedef struct {
    CARD8 repType;  		/* X_Reply */
    CARD8 hpRepType;        	/* always X_HPGetDeviceMotionEvents  */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 nEvents B32;
    INT16  axes B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
} xHPGetDeviceMotionEventsReply;

typedef struct {
    CARD8 reqType;          /* always HpReqCode */
    CARD8 hpReqType;        /* always X_HPGetClipList */
    CARD16 length;
    CARD32 id;		    /* window id */
} xHPGetClipListReq;

typedef struct {
    BYTE   type;  	/* X_Reply */
    BYTE   pad0;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    INT16  x	  B16;	/* x origin of window */
    INT16  y	  B16;	/* y origin of window */
    CARD16 width  B16;	/* width of window */
    CARD16 height B16;	/* height of window */
    CARD32 count  B32;	/* number of clipping rectanges */
    CARD32 data03 B32;
    CARD32 data04 B32;
    CARD32 data05 B32;
    } xHPGetClipListReply;

/**********************************************************
 *
 * extension events.
 *
 */

typedef struct 
    {
    INT16	ax_num;
    INT16	ax_val;
    } XHPaxis_data;

typedef struct
    {
    BYTE 	type;
    BYTE 	ext_type;
    CARD16 	sequencenumber B16;
    XID    	deviceid;
    INT16  	axes_count B16;
    CARD16 	pad00 B16;
    XHPaxis_data data[4];
    CARD32 	pad01 B32;
    }  xHPExtensionEvent;

typedef	struct 
    {
    BYTE type;
    BYTE pad00;
    CARD16 sequencenumber B16;
    INT16 detail B16;
    BYTE mode; 			/* really XMode */
    BYTE pad1;
    XID   deviceid;
    Window window B32;
    CARD32 pad01 B32;
    CARD32 pad02 B32;
    CARD32 pad03 B32;
    CARD32 pad04 B32;
    } xHPdevicefocus;

typedef	struct 
    {
    BYTE type;
    BYTE deviceid;
    BYTE map[30];
    } xHPDeviceKeymapEvent;

#endif
