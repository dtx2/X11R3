/* $XConsortium: x_hilinit.c,v 1.4 88/09/06 15:25:48 jim Exp $ */
/***********************************************************************
 * file: x-hilinit.c
 *
 *
 *  ******************************************************************
 *  *  (c) Copyright Hewlett-Packard Company, 1988.  All rights are  *
 *  *  reserved.  Copying or other reproduction of this program      *
 *  *  except for archival purposes is prohibited without prior      *
 *  *  written consent of Hewlett-Packard Company.		     *
 *  ******************************************************************
 *
 *  Program purpose -- initialize HIL devices for X-server.
 *
 *		Hewlett Packard -- Corvallis Workstation Operation
 *		Project -- port of X (Version 11) to HP9000S300
 *		George Sachs - MTS.  
 */

#define	    NEED_EVENTS
#include <stdio.h>
#include <ctype.h>
#include <sys/hilioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include "X.h"
#include "Xproto.h"
#include "hildef.h"
#include "sun.h"
#include <ndir.h>			/* must be after sun.h */
#include "x_hil.h"
#include "XHPproto.h"			/* extension constants	*/
#include "x_hilinit.h"
#include "input.h"			/* for PointerRootWin	*/
#include "hpext.h"

/******************************************************************
 *
 * Externs and global variables that may be referenced by other files.
 *
 */

#ifdef XTESTEXT1
extern KeyCode xtest_command_key;
#endif /* XTESTEXT1 */

#ifdef MULTI_X_HACK
extern int XMulti; 			/* extern flag for the multi-x stuff */
#endif /* MULTI_X_HACK */

#ifdef HPINPUT
extern	int	DeviceKeyPress;
extern	int	DeviceKeyRelease;
extern	int	DeviceButtonPress;
extern	int	DeviceButtonRelease;
extern	int	DeviceMotionNotify; 
extern	char	*dpmotionBuf[];
extern	char	*dheadmotionBuf[];
#endif  /* HPINPUT */
extern  char   	*display;		/* display number as a string */
extern  int	max_events;
extern  int	queue_events_free;
extern  xEvent	events_array[];
extern  struct	x11EventQueue *events_queue;
extern	xTimecoord	*pmotionBuf;
extern	xTimecoord	*headmotionBuf;

int	acceleration = 0;		/* to control mouse motion	*/
int	threshold    = 0;
int	lastEventTime;
int 	keyboard_click;
int	open_devs = 0;	
int	avail_devs = 0;	

unsigned char	xhp_kbdid;

struct	inputs_selected sel_inputs;
struct	inputs_selected valid_inputs;

PtrPrivRec	*other_p[MAX_LOGICAL_DEVS];
DevicePtr	hpOther[MAX_LOGICAL_DEVS]; 
HPInputDevice	l_devs[MAX_LOGICAL_DEVS];
PtrPrivRec 	sysOtherPriv[MAX_LOGICAL_DEVS];

/******************************************************************
 *
 * Variables that are global to this file only.
 *
 */

static	int 	bell_pitch;
static	int 	bell_duration;
static	int 	hilControlFD;
static	int 	xpointer_is_xkeyboard = FALSE;

DevicePtr 	hpAddInputDevice();

static	struct		x11EventQueue ev_queue;
static	int 		count [NUM_DEV_TYPES];

#ifdef hp9000s300
static char  *dev_names[MAX_LOGICAL_DEVS] =
    { 
    "/dev/hil1",
    "/dev/hil2",
    "/dev/hil3",
    "/dev/hil4",
    "/dev/hil5",
    "/dev/hil6",
    "/dev/hil7",
    "/dev/foo",
    "/dev/null"};
#else
static char  *dev_names[MAX_LOGICAL_DEVS] =
    { 
    "/dev/hil_0.1",
    "/dev/hil_0.2",
    "/dev/hil_0.3",
    "/dev/hil_0.4",
    "/dev/hil_0.5",
    "/dev/hil_0.6",
    "/dev/hil_0.7",
    "/dev/hil_1.1",
    "/dev/hil_1.2",
    "/dev/hil_1.3",
    "/dev/hil_1.4",
    "/dev/hil_1.5",
    "/dev/hil_1.6",
    "/dev/hil_1.7",
    "/dev/hil_2.1",
    "/dev/hil_2.2",
    "/dev/hil_2.3",
    "/dev/hil_2.4",
    "/dev/hil_2.5",
    "/dev/hil_2.6",
    "/dev/hil_2.7",
    "/dev/hil_3.1",
    "/dev/hil_3.2",
    "/dev/hil_3.3",
    "/dev/hil_3.4",
    "/dev/hil_3.5",
    "/dev/hil_3.6",
    "/dev/hil_3.7",
    "/dev/foo",
    "/dev/null"};
#endif	/* hp9000s300 */

/****************************************************************************
 *
 * Change acceleration & threshold.
 *
 */

static void hpChangePointerControl(pDevice, ctrl)
    DevicePtr pDevice;
    PtrCtrl *ctrl;
    {
    threshold = ctrl->threshold;
    acceleration = ctrl->num;
    if (acceleration <= 0)
	acceleration = 1;
    }

/****************************************************************************
 *
 * The members of the keybdCtrl structure have the following values:
 *
 * click:	0(off) - 100 (loud);	-1 => default;
 * bell:	0(off) - 100 (loud); 	-1 => default;
 * bell_pitch:  Pitch of the bell in Hz;-1 => default;
 * bell_duration: in miliseconds;	-1 => default;
 *
 */

static void hpChangeKeyboardControl(pDevice, ctrl)
    DevicePtr pDevice;
    KeybdCtrl *ctrl;
    {
    static int bell_volume;

    keyboard_click = (int)((double)(ctrl->click) * 15.0 / 100.0);
    bell_volume = (int)((double)(ctrl->bell) * 15.0 / 100.0);
    if (ctrl->bell_pitch)
        bell_pitch = (int)(1023.0 - 83333.0/((double)ctrl->bell_pitch));
    else
        bell_pitch = 0;
    bell_duration = ctrl->bell_duration/10;
    if (!bell_duration)
	bell_duration = 1;
    SetAutoRepeat(ctrl->autoRepeat);
    }

/****************************************************************************
 *
 *  range values on beep:
 *   VOICE    : from 1 to 3                                                   *
 *   PITCH    : from 0 to 1023 (incl)                                         *
 *              To get the pitch value from frequency,
 *		pitch = 1023 - 83333/f ;
 *   VOLUME   : from 0 to 15   (incl). Zero turns voice off.                  *
 */

#define BELL_VOICE 		1

static void hpBell(loud, pDevice)
    DevicePtr pDevice;
    int loud;
    {
    loud = (int)((double)loud * 15.0 / 100.0);
    beep(BELL_VOICE, bell_pitch, loud, bell_duration);
    }

/****************************************************************************
 *
 * hpGetMotionEvents.
 *
 */

static int hpGetMotionEvents (dev, buff, start, stop)
    DeviceIntPtr  dev;
    CARD32 start, stop;
    xTimecoord *buff;
    {
    int			count = 0;
    xTimecoord 		*first;
    xTimecoord 		*last;
    xTimecoord 		*curr;
    xTimecoord 		*pmBuf = pmotionBuf;
    xTimecoord 		*hmBuf = headmotionBuf;


    if (pmBuf == hmBuf)
        if (pmBuf->time == 0)		/* no events yet           */
	    return 0;
	else
	    last = hmBuf+99;
    else
	last = pmBuf-1;

    if (pmBuf->time == 0)		/* haven't wrapped yet	    */
	first = hmBuf;
    else
	first = pmBuf;

    if (start > last->time)		/* start time > last time    */
        return 0;
    else
	{
	curr = first;
	while (curr->time < start)
	    {
	    curr++;
	    if (curr == hmBuf+100)
		curr = hmBuf;
	    if (curr == first)
		return 0;
	    }
	while (curr->time <= stop && curr->time != 0)
	    {
	    *buff++ = *curr++;
	    count++;
	    if (curr == hmBuf+100)
		curr = hmBuf;
	    if (curr == first)
		break;
	    }
	}
    return (count);
    }

/****************************************************************************
 *
 * hpGetDeviceMotionEvents.
 *
 */

#ifdef HPINPUT
static int hpGetDeviceMotionEvents (dev, buff, start, stop)
    DeviceIntPtr  dev;
    CARD32 start, stop;
    char *buff;
    {
    PtrPrivPtr 		p = (PtrPrivPtr) dev->public.devicePrivate;
    HPInputDevice 	*pHPDev = (HPInputDevice *) p->devPrivate;
    int			i = 0;
    int			count = 0;
    int			axes = pHPDev->hil_header.ax_num;
    int			size = (sizeof(Time) + (axes * sizeof (short)));
    char 	*first;
    char 	*last;
    char 	*curr;
    char 	*pmBuf = dpmotionBuf[pHPDev->dev_id];
    char 	*hmBuf = dheadmotionBuf[pHPDev->dev_id];
    CARD32	firsttime;
    Time  	lasttime;


    if (pmBuf == hmBuf)
	{
        if (*((Time *) pmBuf) == 0)		/* no events yet           */
	    return 0;
	else
	    last = hmBuf+(99 * size);
	}
    else
	last = pmBuf-size;

    if (*((Time *) pmBuf) == 0)		/* haven't wrapped yet	    */
	first = hmBuf;
    else
	first = pmBuf;

    firsttime = *((Time *) first);
    lasttime = *((Time *) last);

    if (start > *((Time *) last))	/* start time > last time    */
        return 0;
    else
	{
	curr = first;
	while (*((Time *) curr) < start)
	    {
	    curr += size;
	    if (curr == hmBuf+(100*size))
		curr = hmBuf;
	    if (curr == first)
		return 0;
	    }
	while (*((Time *) curr) <= stop && *((Time *) curr) != 0)
	    {
	    for (i=0; i<size; i++)
	        *buff++ = *curr++;
	    count++;
	    if (curr == hmBuf+(100*size))
		curr = hmBuf;
	    if (curr == first)
		break;
	    }
	}
    return (count);
    }
#endif  /* HPINPUT */

/****************************************************************************
 *
 * Add an X mouse.
 * NOTE: The first parameter passed to this routine is really a DeviceIntPtr.
 *       The declaration used here works because the first element of the    
 *	 structure pointed to by the DeviceIntPtr is a DeviceRec.
 *
 */

static Bool hpMouseProc(pDev, onoff)
    DevicePtr pDev;
    int onoff;
    {
    PtrPrivPtr 		pPtrPriv = 	(PtrPrivPtr) pDev->devicePrivate;
    HPInputDevice 	*tDev = 	(HPInputDevice *) pDev->devicePrivate;
    HPInputDevice 	*pHPDev = 	(HPInputDevice *) pPtrPriv->devPrivate;
    int			ndx = 		tDev->dev_id;
    int			button_count =	tDev->hil_header.v_button_count;
    int			axes	     =  tDev->hil_header.ax_num;

    switch (onoff)
        {
	case DEVICE_INIT: 
	    hpOther[ndx] = pDev;
	    hpOther[ndx]->on = FALSE;

	    sysOtherPriv[ndx].pScreen = &screenInfo.screen[0];
	    if ((ndx == XPOINTER) &&(tDev->dev_type == NULL_DEVICE))
		{
	        sysOtherPriv[ndx].x = sysOtherPriv[ndx].pScreen->width;
	        sysOtherPriv[ndx].y = sysOtherPriv[ndx].pScreen->height;
		}
	    else
		{
	        sysOtherPriv[ndx].x = sysOtherPriv[ndx].pScreen->width / 2;
	        sysOtherPriv[ndx].y = sysOtherPriv[ndx].pScreen->height / 2;
		}
	    sysOtherPriv[ndx].z = 0;

	    sysOtherPriv[ndx].devPrivate = hpOther[ndx]->devicePrivate;
	    hpOther[ndx]->devicePrivate = (pointer) &sysOtherPriv[ndx];
	    if (ndx == XPOINTER)
		{
	        initpointerdevicestruct (hpOther[ndx], button_count,
		    hpGetMotionEvents);
	        pmotionBuf = (xTimecoord *) Xalloc 
		    (MOTION_BUFFER_SIZE * sizeof (xTimecoord));
		memset (pmotionBuf, 0, (MOTION_BUFFER_SIZE * 
		    sizeof (xTimecoord)));
	        headmotionBuf = pmotionBuf;
		}
#ifdef HPINPUT
	    else
		{
	        initpointerdevicestruct (hpOther[ndx], button_count,
		    hpGetDeviceMotionEvents);
	        dpmotionBuf[ndx] = (char *) Xalloc (MOTION_BUFFER_SIZE * 
		    (sizeof(Time) + (axes * sizeof(short))));
		memset (dpmotionBuf[ndx], 0, (MOTION_BUFFER_SIZE * 
		    (sizeof(Time) + (axes * sizeof(short)))));
	        dheadmotionBuf[ndx] = dpmotionBuf[ndx];
		}
#endif /* HPINPUT */
 	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    if ( pHPDev != NULL) 
		{
		AddEnabledDevice( pHPDev->file_ds );
		if ( pHPDev->hil_header.position ) 	 /* 1 = Absolute */
		    { 
		    pHPDev->screenSizeX = pPtrPriv->pScreen->width;
		    pHPDev->scaleX = ((float)pPtrPriv->pScreen->width) /
			((float)pHPDev->hil_header.size_x);
		    pHPDev->screenSizeY = pPtrPriv->pScreen->height;
		    pHPDev->scaleY = ((float)pPtrPriv->pScreen->height) /
			((float)pHPDev->hil_header.size_y);
		    }
	        }
	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
	    if ( pHPDev != NULL && pHPDev->file_ds > 0)
		RemoveEnabledDevice( pHPDev->file_ds);
	    break;
	case DEVICE_CLOSE: 
	    if (pHPDev->dev_id == XPOINTER)
		{
	        Xfree (headmotionBuf);
	        headmotionBuf = NULL;
	        pmotionBuf = NULL;
		}
#ifdef HPINPUT
	    else
		{
	        Xfree (dheadmotionBuf[pHPDev->dev_id]);
	        dheadmotionBuf[pHPDev->dev_id] = NULL;
	        dpmotionBuf[pHPDev->dev_id] = NULL;
		}
#endif /* HPINPUT */
	    break;
	}
    return(Success);
    }

/****************************************************************************
 *
 * Initialize the pointer device struct.
 * If other devices are treated as extensions to the pointer,
 * we may have to adjust the button count later.
 *
 */

initpointerdevicestruct (dev, button_count, proc)
    DevicePtr	dev;
    int	button_count;
    int	(*proc) ();
    {
    int	 i;
    BYTE map[MAP_LENGTH];

    for (i=0; i<=button_count; i++)
        map[i] = i;
    InitPointerDeviceStruct(dev, map, button_count, 
	proc, hpChangePointerControl);
    }

/****************************************************************************
 *
 * Add an X keyboard.
 * NOTE: The first parameter passed to this routine is really a DeviceIntPtr.
 *       The declaration used here works because the first element of the    
 *	 structure pointed to by the DeviceIntPtr is a DeviceRec.
 *
 */

Bool hpKeybdProc(pDev, onoff)
    DevicePtr pDev;
    int onoff;
    {
    HPInputDevice 	*pHPDev 	= (HPInputDevice *) pDev->devicePrivate;
    extern		CARD8    	*hpMapRec[];
    extern		KeySymsRec      hpKeySyms[];
    int			ndx 		= pHPDev->dev_id;

    switch (onoff)
	{
	case DEVICE_INIT: 
	    {
	    u_char keyId;
	    hpOther[ndx] = pDev;

	/* see the manual on using hp-hil devices with hpux for the keyboard
	 * nationality codes; they are the low order 5 bits of the device id;
	 * 0x1f is United States, so we'll subtract from 0x1f to give the
	 * U.S. a keyId of zero;
	 */

	    if (pHPDev != NULL)
	        keyId = 0x1f - (pHPDev->hil_header.id & 0x1f);
	    else
	        keyId = 0;

	    if (pHPDev->hil_header.id < 0xA0)	/* nonkbd device, (buttonbox) */
		keyId = 0;			/* Interpret as USASCII    */

	    InitKeyboardDeviceStruct(hpOther[ndx],
		 &(hpKeySyms[keyId]),
		 (hpMapRec[keyId]),
		 hpBell,
		 hpChangeKeyboardControl);
	    break;
	    }
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    if (pHPDev != NULL)
		AddEnabledDevice(pHPDev->file_ds);
	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
	    if ( pHPDev != NULL && pHPDev->file_ds > 0)
		RemoveEnabledDevice(pHPDev->file_ds);
	    break;
	case DEVICE_CLOSE: 
	    break;
	}
    return(Success);
    }

/****************************************************************************
 *
 * InitInput --
 *	Initialize pointer and keyboard devices.
 *
 */
 
static int hil_ready = 0;

InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
    {
    int	result = -1;
    int	i;
    HPInputDevice *pHPDev;
    static int zero = 0;

#ifdef XTESTEXT1
    xtest_command_key = 0x2d;
#endif /* XTESTEXT1 */

    if (!(hil_ready)) 
	{
	result = init_indevices ();
#ifdef MULTI_X_HACK
	if (XMulti)
	    hil_ready = 1;
	else 
	    {
#endif 					/* MULTI_X_HACK */
	    if ( result > 0) 
		hil_ready = 1;
	    /* ErrorF("%d hil input devices found.\n", result); */
#ifdef MULTI_X_HACK
	    }
#endif 					/* MULTI_X_HACK */
        }

    /*
     * Now initialize the devices as far as X is concerned.
     */

    for (i=0, pHPDev=l_devs; i<open_devs; pHPDev++) 
	{
	if (pHPDev->open_cnt > 0)
	    {
	    x_init_device (pHPDev, STARTUP);
	    i++;
	    }
	}
    SetInputCheck (&zero, &isItTimeToYield);
    }

/***********************************************************
 *
 * Perform X initialization for the device.
 *
 */

x_init_device (dev, mode)
    HPInputDevice *dev;
    int	mode;
    {
    DevicePtr pXDev;

    switch (dev->x_type) 
	{
	case MOUSE:
	    if (dev->dev_id == XPOINTER)
		{
	    	pXDev = hpAddInputDevice(hpMouseProc, TRUE, dev);
		RegisterPointerDevice(pXDev, MOTION_BUFFER_SIZE);
	    	}
#ifdef HPINPUT
	    else
	    	{
	    	pXDev = hpAddInputDevice(hpMouseProc, TRUE, dev);
		if (dev->hil_header.p_button_count > l_devs[XPOINTER].hil_header.v_button_count)
		    {
		    l_devs[XPOINTER].hil_header.v_button_count = dev->hil_header.p_button_count;
		    initpointerdevicestruct (pXDev, dev->hil_header.p_button_count, hpGetDeviceMotionEvents);
		    }
	    	RegisterOtherDevice(pXDev);
		if (mode != STARTUP)
		    {
	            hpMouseProc (pXDev, DEVICE_INIT);
	            hpMouseProc (pXDev, DEVICE_ON);
		    }
	    	}
#endif /* HPINPUT */
	    break;
	case XOTHER:
	case KEYBOARD:
	    if (dev->dev_id == XKEYBOARD)
		{
	        pXDev = hpAddInputDevice(hpKeybdProc, TRUE, dev);
		RegisterKeyboardDevice(pXDev);
                if (dev->dev_type == KEYBOARD)
		    xhp_kbdid = dev->hil_header.id - 0xA0;
		}
	    else if (dev->dev_id == XPOINTER)
		{
	    	pXDev = hpAddInputDevice(hpMouseProc, TRUE, dev);
		RegisterPointerDevice(pXDev, MOTION_BUFFER_SIZE);
	    	}
	    else
	    	{
	        pXDev = hpAddInputDevice(hpKeybdProc, TRUE, dev);
#ifdef HPINPUT
	    	RegisterOtherDevice(pXDev);
		if (mode != STARTUP)
		    {
	            hpKeybdProc (pXDev, DEVICE_INIT);
	            hpKeybdProc (pXDev, DEVICE_ON);
		    }
#endif /* HPINPUT */
	    	}
	    break;
	}
    }

/***********************************************************
 *
 * init_indevices ( )
 * Initialize the l_devs structure.
 * Opens HIL devices in the non-blocking mode.
 * Called from InitInput();
 *
 */

static int init_indevices ()
    { 
    int		fd = -1;
    int		max_fd = 0;
    int		mask = 0;
    int		dev_num = 0;
    int		j;

    dev_num = init_l_devs ();

#ifdef MULTI_X_HACK
    if (XMulti) 
	{
	ipc_set_display_num(display);
	ipc_init();

	for (j=0; j<dev_num; j++) 
	    {
	    fd = ipc_try_to_open_hil ( dev_names[j], O_RDONLY | O_NDELAY);
	    if (fd > 0)
		mask |= (1 << fd);
	    if (fd > max_fd)
		max_fd = fd;
	    }

	valid_inputs.input_mask = mask;
	valid_inputs.bit_num    = (max_fd + 1);
	}
#endif /* MULTI_X_HACK */

    get_pointerkeys();
    sel_inputs.bit_num = valid_inputs.bit_num;
    init_events_queue ( &ev_queue);
    for (j=0; j<MAX_LOGICAL_DEVS; j++)
	other_p[j] = &sysOtherPriv[j];
    return (open_devs) ;
    }

/*****************************************************************
 *
 * Initialize the l_devs array of structures.
 * There is one per logical input device.
 *
 */
 
init_l_devs()
    {
    int		i;
    int		j;
    int		dev_num = 0;
    FILE	*fp;
    char	fname[MAXNAMLEN];
    struct	opendevs opendevs [MAX_LOGICAL_DEVS];

#ifdef hp9000s800
    int	dispnum = atoi(display);

    if (dispnum >= 0 && dispnum < 4)
        for (i=0; i<MAX_LOGICAL_DEVS; i++)
	    {
	    if (i < 7)
		dev_names[i][9] = display[0];
	    else if (i<28)
		dev_names[i][9] = '_';
	    }
#endif

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	{
	opendevs[i].type = -1;
	opendevs[i].pos = -1;
	strcpy (l_devs[i].dev_name,dev_names[i]);
        l_devs[i].mode = ABSOLUTE;
        l_devs[i].motion_event = MotionNotify;
	l_devs[i].dev_id = i;
	l_devs[i].clients = NULL;
	l_devs[i].file_ds = -1;
	l_devs[i].x_name[0] = '\0';
	l_devs[i].open_cnt = 0;
	}

    sprintf(fname, "/usr/lib/X11/X%sdevices",display);
    fp = fopen ( fname, "r");
    if (fp) 
	{
        dev_num = device_files (fp, opendevs);
	fclose (fp);
	}

    dev_num = default_devs (opendevs, dev_num);

    if (l_devs[XPOINTER].x_type == KEYBOARD)
	l_devs[XPOINTER].hil_header.v_button_count = 8;

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	{
	l_devs[i].dev_id = i;
        if (l_devs[i].x_type == KEYBOARD)
	    {    
	    l_devs[i].down_event = KeyPress;
	    l_devs[i].up_event = KeyRelease;
	    }
        else
	    {
	    l_devs[i].down_event = ButtonPress;
	    l_devs[i].up_event = ButtonRelease;
	    }
	}
    return (dev_num);
    }

/********************************************************************
 *
 * Find the requested key and pointer devices.
 * If no key or pointer device was named, find a default one.
 *
 */

static int default_devs (opendevs, numdev)
    struct	opendevs opendevs [];
    int		numdev;
    {
    int	i;
    int	j;
    int	k;
    int	fd;
    int	spare = MAX_LOGICAL_DEVS - 2;
    int	tnumdev;
    int	last_mouse = -1;
    int	last_pointer = -1;
    int	last_keyboard = -1;
    int	last_key_device = -1;
    int ndx[MAX_LOGICAL_DEVS];
    HPInputDevice	tmp;
    HPInputDevice	*d;

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	ndx[i] = -1;

/*****************************************************************************
 *
 * Attempt to open all devices and find out what they are.
 * Find out which will be the default devices.
 * Count them so that we can assign names by position.
 * A device that can't be opened is considered not present.
 *
 */
    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	if ((fd = open_device (&i)) >= 0)
	    {
	    d = &l_devs[i];
	    for (j=0; j<numdev; j++)
	        if (d->dev_type==opendevs[j].type && 
		    count[d->dev_type]==opendevs[j].pos)
		    ndx[j] = i;
	    count[d->dev_type]++;
	    if (d->dev_type == MOUSE)
		last_mouse = i;
	    else if (d->dev_type == KEYBOARD)
		last_keyboard = i;
	    else if (d->x_type == KEYBOARD)
		last_key_device = i;
	    else if (d->x_type == MOUSE)
		last_pointer = i;
	    }

/*****************************************************************************
 *
 * If the user picked a keyboard and pointer, they're in the ndx[] array.
 * If he didn't, assign a default.
 * If present, defaults are the last keyboard and last mouse.
 *
 */

    if (ndx[XKEYBOARD] == -1)
	{
        if (last_keyboard != -1)
	    ndx[XKEYBOARD] = last_keyboard;
	else if (last_key_device != -1)
	    ndx[XKEYBOARD] = last_key_device;
	else
	    {
	    ErrorF ("Couldn't find a key device!\n");
	    GiveUp();
	    }
	numdev++;
	}

    if (ndx[XPOINTER] == -1)
	{
        if (last_mouse != -1)
	    ndx[XPOINTER] = last_mouse;
	else if (last_pointer != -1)
	    ndx[XPOINTER] = last_pointer;
	else if (ndx[XKEYBOARD] != -1)
	    ndx[XPOINTER] = ndx[XKEYBOARD];
	else
	    GiveUp();
	numdev++;
	}

    if (ndx[XPOINTER] == ndx[XKEYBOARD])
	{
	xpointer_is_xkeyboard = TRUE;
	open_devs++;
	}

/*****************************************************************************
 *
 * Now weed out any requested devices we couldn't open.
 *
 */

    for (i=0,tnumdev=numdev; i<tnumdev; i++)
	if (ndx[i] < 0)
	    {
	    for (j=i; j<numdev-1; j++)
		ndx[i] = ndx[i+1];
	    numdev--;
	    }

/***********************************************************************
 *
 * Reorder the l_devs file.
 * This starts by being in HIL position order.
 * We want to change this so that the pointer is first,
 * the keyboard is second, and any unopened devices are at the end.
 *
 */

    for (i=0; i<numdev; i++)
	{
	if (i == ndx[i])		/* it's already correct		*/
	    continue;
	tmp = l_devs[i];		/* save current entry	        */
	l_devs[i] = l_devs[ndx[i]];  	/* replace with correct entry   */
	if (ndx[i] < i)   		/* this one already moved       */
	    {
	    ndx[i] = spare;
	    spare++;
	    }
	l_devs[ndx[i]] = tmp;
	for (j=i+1; j<numdev; j++)
	    if (ndx[j] == i)		/* we moved this one       	*/
		ndx[j] = ndx[i];    	/* replace its index		*/
	    else if (ndx[j] == ndx[i])  /* the other one we moved       */
		ndx[j] = i;
	}


/***********************************************************************
 *
 * Now close all the devices that X was not instructed to use.
 *
 */

    if ((opendevs[XPOINTER].type == NULL_DEVICE ||
        opendevs[XKEYBOARD].type == NULL_DEVICE) &&
        l_devs[MAX_LOGICAL_DEVS -1].dev_type == NULL_DEVICE)
	{
        l_devs[MAX_LOGICAL_DEVS - 1].file_ds = -1;
	}

    for (i=MAX_LOGICAL_DEVS-1; i>=numdev; i--)
	if (l_devs[i].file_ds != -1 )
	    {
	    close_device (i,l_devs[i].file_ds);
	    }

    return (numdev);
    }

/****************************************************************************
 *
 * open_device opens one of the input devices.
 * The dev_name is filled in by device_files(), or is the default.
 * If the open fails, it may be because the keyboard and pointer
 * are the same device, and the device is already open.
 *
 */

open_device (ndx)
    int		*ndx;
    {
    int		fd;
    HPInputDevice tmp;

    fd = open (l_devs[*ndx].dev_name, O_RDONLY | O_NDELAY);
    if (fd < 0) 
        return (fd);

    l_devs[*ndx].open_cnt++;
    if (get_device_details (fd, &(l_devs[*ndx])) > 0)
        avail_devs++; 
    valid_inputs.input_mask |=  1<<fd;
    if (fd > valid_inputs.bit_num - 1)
    valid_inputs.bit_num = fd + 1;
    sel_inputs.bit_num = valid_inputs.bit_num;

    open_devs++;
    return (fd);
    }

/****************************************************************************
 *
 * Query the hil device for detailed information.
 *
 */

static int get_device_details(file_ds, input_dev)
    int file_ds;
    HPInputDevice *input_dev;
    {
    int 	dev_status;
    u_char	describe[11];
    u_char      hbyte;
    u_char      *io_record;
    u_char	id;
    struct	hil_desc_record		*hd;
    int		hi_resol =0;
    int 	lo_resol = 0;
    int		i;
    int 	support_it = TRUE;

    for (i=0; i<11; i++)
	describe[i] = 0;
    dev_status = ioctl (file_ds, HILID, &(describe[0]));
    input_dev->file_ds = file_ds;

    if (dev_status >= 0) 
	{
	hd = &(input_dev->hil_header);
	hd->id  = describe[0];

	hbyte = describe[1];
	hd->flags = describe[1];

	hd->num_cords   = FALSE;
	hd->position    = FALSE;
	hd->bits_axes   = FALSE;
	hd->io_type     = FALSE;
	hd->ext_command = FALSE;
	hd->security    = FALSE;

	if (hbyte &  HILDRH_TWO_AXES) hd->num_cords = TRUE;
	if (hbyte & HILDRH_ABS_POS)   hd->position  =  TRUE;
	if (hbyte & HILDRH_16BIT_POS) hd->bits_axes = TRUE;
	if (hbyte & HILDRH_IODB)      hd->io_type  = TRUE;
	if (hbyte & HILDRH_EDES)      hd->ext_command  = TRUE; 
	if (hbyte & HILDRH_RSC)       hd->security = TRUE ;	    

	hd->ax_num = (hbyte & HILDRH_AXES);

	/*
	 *
	 * if # of axes indicate it is a positional device
	 * then gather resolution.	
	 * if 16 bits of information are reported, resolution is
	 * in counts/ cm.  In this case, convert to counts/ meter.
	 *
	 */

	if ( hd->ax_num) 
	    {
	    lo_resol =  describe[2];
	    hi_resol =  describe[3];
	    hd->resolution = (hi_resol << 8) + lo_resol;
	    if (hd->bits_axes == TRUE)
		hd->resolution *= 100;
	    /* If it is an absolute device, gather size */
	    if ( hd->position ) 
		{
		switch ( hd->ax_num) 
		    {
		    case 3:
		        hd->size_z = (int)describe[8]|((int)describe[9] << 8);
		    case 2:
		        hd->size_y = (int)describe[6]|((int)describe[7] << 8);
		    case 1:
			 hd->size_x = (int)describe[4]|((int)describe[5] << 8);
		    default:
			 break;
		    }
		io_record = &(describe[4 + hd->ax_num * 2]);
		}
	    else
		io_record = &(describe[4]);
	    }
	else 
	    {
	    io_record = &(describe[2]);
	    hd->resolution = 0;
	    }
		   
	if ( hd->io_type) 
	   {
	   hd->iob = *io_record;
	   hd->p_button_count = *io_record & HILIOB_BUTTONS ;
	   hd->v_button_count = *io_record & HILIOB_BUTTONS ;

	   /*
	    * initialize structures for mapping 2-button mouse
	    *  three button mouse.
	    */

	   if (hd->p_button_count == 2)  
		{
	        hd->v_button_count = 3;  
		X_mouse_three (file_ds);
		}
	   }
        get_device_type (input_dev, hd->id);
	}
    else
	{
	input_dev->hil_header.id = 0;
        input_dev->dev_type = NULL_DEVICE;
        input_dev->x_type = XOTHER;
	strcpy (input_dev->x_name,"FIRST_NULL");
	support_it = FALSE;
	}
    return ( support_it);
    }

/****************************************************************************
 *
 * This routine determines the type of the input device.
 * dev_type is the actual type, x_type is what X considers it to be
 * (mouse or keyboard).
 * The 9-knob box and quadrature box have the same HIL id.
 * But if it doesn't have 3 axes, it's not a 9-knob box.
 *
 */

get_device_type (dev, id)
    HPInputDevice *dev;
    int	id;
    {
    int		i;
    int		dev_count;

    for (i=0; devices[i].dev_type != NULL_DEVICE; i++)
	if (id >= devices[i].lowid && id <= devices[i].highid)
	    {
    	    if (id == NINE_KNOB_ID && dev->hil_header.ax_num != 3)
		i = QUAD_INDEX;
	    dev->hil_header.min_kcode = devices[i].min_kcode;
	    dev->hil_header.max_kcode = devices[i].max_kcode;
	    dev->hil_header.num_keys = devices[i].num_keys;
	    dev->dev_type = devices[i].dev_type;
	    dev->x_type    = devices[i].x_type;
	    dev_count = count [dev->dev_type];
	    strcpy (dev->x_name, position[dev_count]);
	    strcat (dev->x_name, "_");
	    strcat (dev->x_name, devices[i].name);
	    break;
	    }
    }

/****************************************************************************
 *
 * This routine recalculates the device x_name.
 * The x_name is a string created by concatenating the device type and position.
 * The position may change if a device that was previously inaccessible
 * to X is made accessible.
 *
 */

recalculate_x_name ()
    {
    int	i;
    int	j;
    int	fd;

    for (i=0; i<NUM_DEV_TYPES; i++)
	count[i] = 0;
    avail_devs = open_devs;
    
    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	for (j=0; j<MAX_LOGICAL_DEVS; j++)
	    if (strcmp (l_devs[j].dev_name,"/dev/null") == 0)
		continue;
	    else if (strcmp (dev_names[i], l_devs[j].dev_name) == 0)
		{
		if (l_devs[j].open_cnt > 0)
		    {
		    if (j==XKEYBOARD && xpointer_is_xkeyboard)
			continue;
		    get_device_type (&l_devs[j], l_devs[j].hil_header.id);
		    count [l_devs[j].dev_type]++;
		    }
		else if ((fd = open_device (&j)) > 0)
		    {
		    count [l_devs[j].dev_type]++;
		    close_device(j,fd);
		    }
		else 
		    l_devs[j].x_name[0] = '\0';
		break;
		}
    }

/****************************************************************************
 *
 * SetAutoRepeat (onoff)
 *  Enable or disable the auto repeat feature of the hil devices.
 *  This will set the auto repeat mode for all hil devices. If not
 *	so desired it should be modified for selected devices.
 */

static int SetAutoRepeat (onoff)
    int onoff;
    {
    int	i,j;
    char ioctl_data[12];
    int state = HILDKR;
	
    if (onoff)
	state = HILER1;

    for (i=0,j=0; j<MAX_LOGICAL_DEVS && i<open_devs; j++)
	{
	if (l_devs[j].file_ds != -1)
	    {
	    ioctl (l_devs[j].file_ds, state, ioctl_data);
	    i++;
	    }
	}
    return (onoff);
    }

/*************************************************************
 *
 * Return a timestamp in miliseconds
 * Note that HIL returns time in 10-millisecond ticks 
 *
 */

unsigned int hilEventTime (timeStamp)
    u_char	*timeStamp;
    {
    unsigned int time, *pTime;


    pTime = (unsigned int *) timeStamp;  
    time = *pTime;
    return ( 10 * time );
    }

/********************************************************************
 *
 * If the file "/usr/lib/X11/X[display#]devices exists, this routine 
 * processes it.
 * It translates the strings in the file to a device type and relative
 * position on the HIL.
 *
 */

static int device_files (fd, opendevs)
    FILE	*fd;
    struct	opendevs opendevs [];
    {
    char *ret;
    char buf[MAXNAMLEN+1];
    char devuse[MAXNAMLEN+1];
    char name[MAXNAMLEN+1];
    char pos[MAXNAMLEN+1];
    char *fgets();
    int	len;
    int	i;
    int	other = XOTHER;

    while ((ret = fgets(buf,MAXNAMLEN+1,fd)) != NULL)
	{
	len = strlen (buf);
	for (i=0; i<len; i++)
	    buf[i] = toupper(buf[i]);
	sscanf (buf, "%s%s%s", pos, name, devuse);

	if (pos[0] == '#')	/* comment, skip it */
	    continue;

	for (i=0; i<MAX_POSITIONS; i++)
	    if (strcmp (position[i], pos) == 0)
		{
		if (strcmp (devuse, "POINTER") == 0)
		    opendevs[XPOINTER].pos= i;
		else if (strcmp (devuse, "KEYBOARD") == 0)
		    opendevs[XKEYBOARD].pos= i;
		else if (strcmp (devuse, "OTHER") == 0)
		    opendevs[other].pos= i;
		else
		    i = MAX_POSITIONS;
		break;
		}

	if (i== MAX_POSITIONS) /* failed, skip to next */
	    continue;

	for (i=0; i<MAX_DEV_TYPES; i++)
	    if (strcmp (devices[i].name,name) == 0)
		{
		if (strcmp (devuse, "POINTER") == 0)
		    opendevs[XPOINTER].type = devices[i].dev_type;
		else if (strcmp (devuse, "KEYBOARD") == 0)
		    opendevs[XKEYBOARD].type = devices[i].dev_type;
		else if (strcmp (devuse, "OTHER") == 0)
		    opendevs[other++].type = devices[i].dev_type;
		break;
		}
	}
    return (other);
    }

/********************************************************************
 *
 * get_pointerkeys().
 * This routine provides the ability to configure keyboard keys to 
 * move the pointer and act like buttons on the pointer device.
 * The file processed is the X*pointerkeys file, which consists
 * of pairs.  The form is:
 *
 * 	function	key, modifier, or value
 *
 * Look at the pointerfunc table in x_hilinit.h to understand this code.
 * There are 3 types of assignment done:
 * 	1). keys - have both a down and an up code to assign.
 *	2). modifiers - are a bit position in a mask.
 *	3). values - are a single integer number.
 * Possible errors:
 *	1). only 1 of the pair was specified.
 *	2). an invalid function was specified.
 *	3). an invalid key or modifier was specified.
 */

get_pointerkeys()
    {
    char	fname[MAXNAMLEN+1];
    FILE	*fp;
    int 	cret;
    int 	vret;
    int 	ret2;
    int 	index;
    char 	*ret;
    char 	buf[MAXNAMLEN+1];
    char 	function[MAXNAMLEN+1];
    char 	key[MAXNAMLEN+1];
    char 	*fgets();
    u_char 	down;
    u_char 	up;
    u_char 	*downvar;
    u_char 	*upvar;

    sprintf(fname, "/usr/lib/X11/X%spointerkeys",display);
    fp = fopen ( fname, "r");
    if (fp == NULL)
	return;

    while ((ret = fgets(buf,MAXNAMLEN+1,fp)) != NULL)
	{
	ret2 = sscanf (buf,"%s%s",function,key);

	/* comments begin with a '#'.  Skip them. */

	if (function[0] == '#')		/* comment, skip it 	*/
	    continue;

	if (ret2 == 2)			/* error if < 2 items 	*/
	    {
	    cret = get_codes (key, &down, &up);
	    vret = get_vars (function, &downvar, &upvar,&index);
	    if (vret < 0)		/* invalid function     */
		continue;		/* error - skip this one*/
	    if (cret < 0 &&		/* not a key or modifier*/
	        pointerfunc[index].type == KEY) /* but must be  */
		continue;		/* error - skip this one*/

	    *downvar = down;		/* code for key press	*/
	    if (upvar == NULL)		/* modifier - compute bit*/
	        *downvar = (1 << (down / 2));
	    else
	        *upvar = up;		/* code for key release */
	    }
	}
	
    fclose (fp);
    }

/********************************************************************
 *
 * get_codes()
 * Used to assign codes to keys used to move the pointer.
 * Also to assign numbers to the amount to move the pointer.
 * This routine uses the index into the file to determine the keycode.
 * The down keycode is (index * 2), the up keycode is that plus 1.
 * If we don't find a matching string, assume the key passed in is
 * really the ascii representation of a number.
 * This is used as the increment to move the pointer.
 *
 */

#define 	MAX_HIL_KEYS		128

get_codes (key, down, up)
    char	*key;
    u_char	*down;
    u_char	*up;
    {
    int		i;

    for (i=0; i<MAX_HIL_KEYS; i++)
        if (strcmp (key, keyset1[i]) == 0)
	    {
	    *down = i*2;
	    *up = *down+1;
	    return (0);
	    }
    *down = atoi (key);
    *up = atoi (key);
    return (-1);
    }

/********************************************************************
 *
 * get_vars()
 * get the address of variables to contain keycodes for pointer functions.
 *
 */

get_vars (func, downvar, upvar, index)
    char	*func;
    u_char	**downvar;
    u_char	**upvar;
    int		*index;
    {
    int		i;
    
    for (i=0; i<MAX_POINTER_FUNCS; i++)
        if (strcmp (func, pointerfunc[i].name) == 0)
	    {
	    *downvar = pointerfunc[i].down;
	    *upvar = pointerfunc[i].up;
	    *index = i;
	    return (0);
	    }
    return (-1);
    }

/****************************************************************************
 *
 * TimeSinceLastInputEvent()
 * - aparently returns time in miliseconds since last input event
 *
 */

TimeSinceLastInputEvent()
    {
    if (lastEventTime == 0)
	lastEventTime = GetTimeInMillis();
    return GetTimeInMillis() - lastEventTime;
    }

/****************************************************************************
 *
 * hpAddInputDevice(deviceProc, autoStart, pHPDev)
 * create an X input device, then assign pHPDev to it's devicePrivate field.
 *
 */

static DevicePtr hpAddInputDevice(deviceProc, autoStart, pHPDev)
    DeviceProc deviceProc;
    Bool autoStart;
    HPInputDevice *pHPDev;
    {
    DevicePtr pXDev;

    pXDev = AddInputDevice(deviceProc, autoStart);
    pXDev->devicePrivate = (pointer) pHPDev;
    return  pXDev;
    }

/****************************************************************************
 *
 * Can the keyboard hardware support this scan code as a legal modifier?
 * By emperical observation, our HIL keyboards support 6-key
 * roll-over on any key.  This means that essentially any key
 * can be used as a modifier.
 *
 */

LegalModifier(key)
    BYTE key;
    {
    return TRUE;
    }

/****************************************************************************
 *
 * add a device.  Used only by multi-x code.
 *
 */

#ifdef MULTI_X_HACK
fixup_device_details(fd)
    int fd;
    {
    int status;

    status = get_device_details(fd, &(l_devs[open_devs]));
    open_devs++;

    if (!status)
	close_hil(fd);
    else 
        {
        valid_inputs.input_mask |= 1 << fd;
        if (fd+1 > valid_inputs.bit_num) 
	    {
	    valid_inputs.bit_num = fd + 1;
	    sel_inputs.bit_num = fd + 1;
	    }
        }        
    return(status);
    }

/****************************************************************************
 *
 * Remove a device.  Used only by mult-x code.
 *
 */

remove_details_entry(fd)
    int fd;
    {
    int i;

    for (i=0; i<open_devs; i++)
	if (fd == l_devs[i].file_ds)
	     break;

    /** (i == open_devs)   => fd was not in table;
     ** (i == open_devs-1) => don't worry; removed device is at end;
     ** (i < open_devs-1)  => copy entry open_devs-1 to i;
     **/

    valid_inputs.input_mask &= ~(1 << fd);
    if (fd+1 == valid_inputs.bit_num) 
	{
	valid_inputs.bit_num--;
	sel_inputs.bit_num--;
	}

    open_devs--;
    if (i < open_devs)
        memcpy(&(l_devs[i]),&(l_devs[open_devs]), sizeof(HPInputDevice));
    }
#endif /* MULTI_X_HACK */

/****************************************************************************
 *
 * close_device closes one of the input devices.
 *
 */

close_device(ndx,fd)
    int fd;
    int ndx;
    {
    int 		ret;
    char		tmp[MAXNAMLEN+1];
    HPInputDevice	tmp_file;

    l_devs[ndx].open_cnt--;
    l_devs[ndx].file_ds = -1;
    if (close (fd) < 0)
	return (-1);

    valid_inputs.input_mask &= ~(1 << fd);
    if (fd+1 == valid_inputs.bit_num) 
	{
	valid_inputs.bit_num--;
	sel_inputs.bit_num--;
	}
    open_devs--;
    return (0);
    }

/****************************************************************************
 *
 * This routine finds the correct index into the device data structures
 * from the deviceid it is passed.  The deviceid and index are not
 * necessarily the same.
 *
 */

find_device (id)
    XID		id;
    {
    int		i;

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	if (id == l_devs[i].dev_id)
	    return(i);
    return (-1);
    }

/*****************************
 *
 * init_events_queue (queue)
 * 
 */

init_events_queue(queue)
    struct  x11EventQueue	*queue;		
    {
    queue->events = events_array;	
    queue->head = 0;
    queue->tail = 0;
    queue->size = MAX_EVENTS;
    events_queue = queue;
    max_events = WR_EVENTS;
    }

/*****************************************************************
 *
 * allocate_event ()
 *	allocates the next available event to the caller and increments
 *	the tail pointer of the events queue; sets queue_events_free as needed.
 *
 */

xEvent  *allocate_event ()
    {
    xEvent		*event;

    event = &( (events_queue->events)[events_queue->tail]);

    if ( events_queue->tail == WR_EVENTS)
	events_queue->tail = 0;
    else  (events_queue->tail)++;

    queue_events_free--;
    return (event);
    }
