/* $XConsortium: x_hil.c,v 1.12 88/09/06 15:25:59 jim Exp $ */

/***********************************************************************
 * file: x-hil.c
 *
 *
 *  ******************************************************************
 *  *  (c) Copyright Hewlett-Packard Company, 1984.  All rights are  *
 *  *  reserved.  Copying or other reproduction of this program      *
 *  *  except for archival purposes is prohibited without prior      *
 *  *  written consent of Hewlett-Packard Company.		     *
 *  ******************************************************************
 *
 *  Program purpose -- input driver for HIL devices for X-server.
 *
 *		Hewlett Packard -- Corvallis Workstation Operation
 *		Project -- port of X (Version 11) to HP9000S300
 *		Sankar L. Chakrabarti -- MTS.
 *		Jack Palevich -- MTS.
 *		Don Bennett - MTS.  
 *
 * Routines accessed from outside of this file:
 *      ProcessInputEvents
 *      hilEventTime
 *      move_mouse
 *      put_keyevent
 *      store_inputs
 *	XTestGenerateEvent
 *	XTestGetPointerPos
 *	XTestJumpPointer
 */

#define	 NEED_EVENTS
#include <sys/types.h>
#include "X.h"
#include "Xproto.h"
#include "hildef.h"
#include "sun.h"
#include "../cfb/cfb.h"
#include "x_hil.h"
#include "XHPproto.h"
#include "hpext.h"
#include "hpkeys.h"
#include "windowstr.h"

#define REPEAT_ARROW		0x2
#define BUF_SIZ		       2048	/* size of static buffer to use  */

#ifdef XTESTEXT1
#define  XTestSERVER_SIDE
#include "xtestext1.h"	
/*
 * defined in xtestext1di.c
 */
extern int	on_steal_input;
extern Bool	XTestStealKeyData();
#endif /* XTESTEXT1 */

/******************************************************************
 *
 * Externs and variables referenced from other files.
 *
 */

#ifdef	HPINPUT
extern	int		DeviceMotionNotify;
char			*dheadmotionBuf[MAX_LOGICAL_DEVS];
char			*dpmotionBuf[MAX_LOGICAL_DEVS];
#endif	/* HPINPUT */
extern	int 		screenIsSaved;
extern	int		xEventExtension;
extern	int		acceleration;
extern  int		open_devs;	
extern	int		threshold;
extern	int		lastEventTime;
extern	int 		keyboard_click;
extern	struct		inputs_selected sel_inputs;
extern	struct		inputs_selected valid_inputs;
extern	PtrPrivRec	*other_p[MAX_LOGICAL_DEVS];
extern	DevicePtr	hpOther[MAX_LOGICAL_DEVS]; 
extern	HPInputDevice	l_devs[MAX_LOGICAL_DEVS];
extern	WindowRec WindowTable[];

Bool		screenSaved = FALSE;
Bool		reset_enabled = TRUE;
int 		hpActiveScreen = 0; 		/* active screen ndx (Zaphod) */
int		queue_events_free = MAX_EVENTS;
int		max_events;			/* for experiments	*/
int		data_cnt = 0;
struct		x11EventQueue *events_queue;	/* pointer to events queue.  */
xEvent		events_array[MAX_EVENTS];	/* input event buffer	*/
xTimecoord	*headmotionBuf;
xTimecoord	*pmotionBuf;

/******************************************************************
 *
 * Variables global to this file.
 *
 */

static	u_char	last_direction;
static	u_char	last_key;
static	u_char	last_arrow = REPEAT_ARROW;/*keycode of arrow key pressed last */
static	int	pending_index;
static	int	pending_bytes;
static	int	k_down_flag[4];
static	int	k_down_incx[4];
static	int	k_down_incy[4];

static  xEvent  xE;

static  u_char	buf[BUF_SIZ];
static  u_char	*pkt_ptr = buf;
#ifdef	HPINPUT
static	xHPExtensionEvent	xExt;
#endif	/* HPINPUT */
unsigned int	hilEventTime();

static	struct	dev_info	hil_info;/* holds hil_data during processing */

/****************************************************************************
 *
 * store_inputs ( ready_inputs)
 *	Assumption: when this routine is called, the events_queue must be
 *	 empty since the processing loop in the Receive () routine in
 *	 main.c breaks only when the head pointer and tail pointers are
 *	 equal.
 *	1. Flush any pending data;
 *	2. process data saved from previous store_inputs() call;
 *	3. Process data from the input devices till either no more
 *		data is obtained from devices or there is no more room in
 *		the events queue.
 */

store_inputs(ready_inputs)
    long	ready_inputs;
    {
    int     mask,
	    checkmask,
	    checkfd;

    if (screenIsSaved == SCREEN_SAVER_ON)
	SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);

    sel_inputs.input_mask = ready_inputs  & valid_inputs.input_mask;
    mask = sel_inputs.input_mask;

    queue_events_free	= WR_EVENTS;

    checkfd = sel_inputs.bit_num;
    checkmask = 1 << checkfd;
    while (mask && (queue_events_free > MAXHILEVENTS))
	{
	if (mask & checkmask) 
	    {
	    mask &= ~checkmask;
	    process_inputs(checkfd);
	    }
	checkfd--;
	checkmask = checkmask >> 1;
        }
    }

		
/****************************************************************************
 *
 * process_inputs ( file_ds)
 *	Take care of the fact that HIL driver may send both motion and
 *	and button data within same data packet; for such a packet we
 *	have to generate two distinct events. This version contains this
 *	improvement.
 */

#define TIME_POLL_BYTES		5	/* bytes indicating time and poll*/
#define READ_SIZ	     2000	/* leave room for partial packets*/

static int process_inputs (file_ds)
    int	file_ds;			/* file_ds that broke select 	 */
    {
    int			i;
    u_char		id;
    u_char		packet_size;
    u_char		*hil_ptr;
    HPInputDevice 	*indevice = NULL; 
    int	bytes_read;		/* debugging	*/ 
    
    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	{
	if (file_ds == l_devs[i].file_ds) 
	    {
	    indevice = &(l_devs[i]);
	    break;
	    }
	}
	
    if (indevice != NULL)
	hil_info.hil_dev = indevice;		/* input device struct ptr  */
    else
	{
	printf ("X server couldn't find current input device - Aborting.\n");
	GiveUp(); 
	}

    if (data_cnt == 0)
	{
	pkt_ptr = buf;
	data_cnt = read (file_ds, buf, READ_SIZ);
	}
    while (data_cnt > 0) 			/* data yet to be processed */
	{
	hil_ptr = (unsigned char *) &hil_info;/* place to copy packet to*/
	packet_size = get_hil_event (file_ds, hil_ptr);

	pending_index = 0;
	pending_bytes = packet_size - 1  - TIME_POLL_BYTES;
	process_hil_data ( &(hil_info));
	}

    if ( xE.u.u.type == MotionNotify )
        (* hpOther[XPOINTER]->processInputProc) (&xE, hpOther[XPOINTER]);
#ifdef	HPINPUT
    else if ( xE.u.u.type == DeviceMotionNotify )
	{
	id = xE.u.keyButtonPointer.pad1;
        (* hpOther[id]->processInputProc) (&xE, hpOther[id], xTrue);
        (* hpOther[id]->processInputProc) (&xExt, hpOther[id], xFalse);
	}
#endif /* HPINPUT */
    xE.u.u.type = 0;
    }


/****************************************************************************
 *
 * get_hil_event (fd, dest)
 *	get an hil event and copy it into dest.
 */

get_hil_event (fd, dest)
    int		fd;
    char	*dest;
    {
    int	packet_size;
    int	i;

    packet_size = *pkt_ptr++;		/* 1st byte is size	    */
    if(data_cnt < packet_size)		/* We got a partial packet  */
	{
	data_cnt += read(fd,		/* get rest of packet */ 
			pkt_ptr + data_cnt, 
			packet_size - data_cnt);

	if(data_cnt != packet_size)
	    {
	    printf ("Mismatch in HILdevice reading. \n");
            GiveUp(); 
	    }
	}
    for (i=1; i<packet_size; i++)	/* copy the current packet  */
        *dest++ = *pkt_ptr++;

    data_cnt -= packet_size;		/* fix unprocessed data cnt */
    return (packet_size);
    }

/****************************************************************************
 *
 * process_hil_data ( hil_info)
 *	process the data in the hilinfo structure and generate events
 *	as needed.
 */

static process_hil_data ( hil_info)
    struct	dev_info	*hil_info;
    {
    u_char	dev_type = hil_info->hil_dev->dev_type;
    u_char	*hil_code;

    lastEventTime = GetTimeInMillis();        /* Used for ScreenSaver */

    switch (dev_type) 
	{
	case	MOUSE :
	case	ONE_KNOB:
	case	NINE_KNOB :
	case	QUADRATURE:
	case	TOUCHPAD:
	case	TRACKBALL:
	    parse_mouse(hil_info);
	    break;
	case	BARCODE :
	     while (pending_index < pending_bytes ) 
		{  
	        hil_code = ascii_to_code[(hil_info->dev_data)[pending_index]];
	        while (*hil_code != 0)
	            parse_keycode ( hil_info, *(hil_code++), KEYBOARD);
	        pending_index++;
		}
	    break;
	case	KEYBOARD:
	case	BUTTONBOX :
	case	KEYPAD :
	     while (pending_index < pending_bytes ) 
		{  
	        parse_keycode ( hil_info, (hil_info->dev_data)[pending_index], KEYBOARD);
	        pending_index++;
		}
	    break;
	case	TABLET :
	case	TOUCHSCREEN :
	    parse_tablet_data ( hil_info);
	    break;
	default	: 
	    printf ( "INPUT ERROR: UNSUPPORTED INPUT DEVICE.\n");
	    printf ( "file type is %x id is %d.\n", dev_type, (hil_info->hil_dev)->dev_id);
	    break;
	}
   }

/****************************************************************************
 *
 * parse_mouse ( hil_info )
 *
 *	We are capturing the motion event straight in the
 *	in the DEVICE structure, also we are calling to move the mouse
 *	right from here. The X-server will only use the motion event as
 *	an hint.
 *	Altered to accomodate genuine three button mouse.
 */

#define MOUSE_AXES		2	/* bytes indicating x,y of mouse */

static parse_mouse (hil_info)
    struct dev_info	*hil_info;
    {
    int		event;
    int		id;
    char	pollhdr;
    u_char  	buttoncode;
    short	x, y, z;

    id = hil_info->hil_dev->dev_id;
    event = hil_info->hil_dev->motion_event;/* event this device sends  */
    if (event == MotionNotify)		/* it's a standard motion event	    */
	id = XPOINTER;			/* fake that it came from pointer   */
    pollhdr = hil_info->poll_hdr;

    if (pollhdr & (SET1_KEY_MASK|MOTION_MASK)) 
        {
        if ( (pollhdr & SET1_KEY_MASK) && (pollhdr & MOTION_MASK)) 
	    {
	    /* data packet has both motion and keycode information.	*/
	    hil_info->poll_hdr &= ~(MOTION_MASK);
	    get_move (hil_info, &x, &y, &z);
	    process_motion (hil_info, other_p[id], x, y, z);
	    pending_index = pending_index + MOUSE_AXES;
	    process_mouse_button (hil_info);
	    }
        else if (pollhdr & MOTION_MASK)
	    {
	    get_move (hil_info, &x, &y, &z);
            process_motion (hil_info, other_p[id], x, y, z);
            }
    	else if (pollhdr & SET1_KEY_MASK)
	    process_mouse_button (hil_info);
    	}
    else
        printf ( "HIL: Unknown Mouse operation : pollhdr %x \n", pollhdr);
    }

/****************************************************************************
 *
 * parse_keycode ( hil_info,  byte, dev_type)
 *   : Parse the keycode information.
 */

#define UP_LEFT_ARROW		0xf8	/* HIL key codes for arrow keys. */
#define DOWN_LEFT_ARROW		0xf9
#define UP_DOWN_ARROW		0xfa
#define DOWN_DOWN_ARROW		0xfb
#define UP_UP_ARROW		0xfc
#define DOWN_UP_ARROW		0xfd
#define UP_RIGHT_ARROW		0xfe
#define DOWN_RIGHT_ARROW	0xff

static int parse_keycode (hil_info, byte, dev_type)
    struct	dev_info     *hil_info;
    u_char      byte;
    int		dev_type;
    {
#ifdef	HPINPUT
    extern	int		DeviceKeyPress;
    extern	int		DeviceKeyRelease;
    extern	int		DeviceButtonPress;
    extern	int		DeviceButtonRelease;
#endif	/* HPINPUT */
    extern	CARD16 		keyModifiersList[];
    PtrPrivRec	*dev;
    int		id 		= hil_info->hil_dev->dev_id;
    int		i;
    xEvent	*event;
    char	*time_stamp 	= hil_info->timestamp;
    char	key_direction;
    u_char	*kptr;
    u_char	mod_mask;

     /* Check if the key pressed should move the sprite. */

    if (dev_type == KEYBOARD && id == XPOINTER)
        if (move_sprite (hil_info, byte, id))
	    return;
    
     /* Check if cursor keys are repeating. */
    
    switch ( byte)
	{
        case	DOWN_DOWN_ARROW		:
        case	UP_DOWN_ARROW		:
        case    UP_LEFT_ARROW		:
        case	DOWN_LEFT_ARROW		:
        case	UP_RIGHT_ARROW		:
        case	DOWN_RIGHT_ARROW	:
        case	UP_UP_ARROW		:
        case	DOWN_UP_ARROW		:
	    last_arrow = byte;
	    break;
        case	REPEAT_ARROW		:
	    if (last_arrow != REPEAT_ARROW)
	        byte = last_arrow;
	    break;
        default:

	/* allow reset from any key device, but only
	   if reset is enabled.				*/

	    if (reset_enabled && byte == reset_down) 
		{
		if (hil_info->hil_dev->down_event == KeyPress)
		    kptr = ((DeviceIntPtr)hpOther[XKEYBOARD])->down;
		else
		    kptr = ((DeviceIntPtr)hpOther[id])->down;
		mod_mask = (reset_mod1 | reset_mod2 | reset_mod3);
		if ((mod_mask & kptr[1]) == mod_mask)
	            GiveUp();
		}

	    break;
	}

    if (byte & UP_MASK) 
	key_direction = hil_info->hil_dev->up_event;
    else
	key_direction = hil_info->hil_dev->down_event;

 /*
  *
  * Divide the keycode by two so that up and down transitions for a key
  * or button produce the same X keycode.
  *
  */

    if (dev_type == KEYBOARD) 
        {
        byte >>= 1;
        byte += MIN_KEYCODE; 		/* avoid mouse codes.  See hpKeyMap.c */
	dev = other_p[XPOINTER];
        if (keyModifiersList[byte] & LockMask) 
	    {
	    if (key_direction == KeyRelease)
	        return;
	    if (((DeviceIntPtr)hpOther[id])->down[byte>>3] & (1<<(byte & 7)))
	        key_direction = KeyRelease;
    	    }
	}
    else if (dev_type == MOUSE) 
        {
        byte -= BUTTON_1_OFFSET;
        byte >>= 1;
	if (hil_info->hil_dev->motion_event == MotionNotify)
	    dev = other_p[XPOINTER];
	else
	    dev = other_p[id];
        }

#ifdef XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(byte, key_direction, dev_type, dev->x, dev->y))
#endif /* XTESTEXT1 */
	put_keyevent(byte,
		     key_direction,
		     hilEventTime(time_stamp),
		     dev_type,
		     dev,
		     hil_info);
    }

/************************************************************************
 *
 * This routine checks to see if the key should be interpreted as a 
 * sprite movement or a button.
 *
 */

move_sprite (hil_info, key, id)
    struct	dev_info     *hil_info;
    u_char      key;
    int		id;
    {
    u_char      *kptr;
    u_char      ptr_mods = 0;
    u_char      mv_mods = 0;
    int		inc = pointer_move;
    int		ret = 0;

    if (hil_info->hil_dev->down_event == KeyPress)
        kptr = ((DeviceIntPtr)hpOther[XKEYBOARD])->down;
    else
        kptr = ((DeviceIntPtr)hpOther[id])->down;

    ptr_mods = (pointer_key_mod1 | pointer_key_mod2 | pointer_key_mod3);
    mv_mods = kptr[1] & ~ptr_mods;
    if ((ptr_mods & kptr[1]) == ptr_mods)
	ptr_mods = 1;
    else
	ptr_mods = 0;

    if (mv_mods & pointer_amt_mod1)
        inc = pointer_mod1_amt;
    else if (mv_mods & pointer_amt_mod2)
        inc = pointer_mod2_amt;
    else if (mv_mods & pointer_amt_mod3)
        inc = pointer_mod3_amt;

    k_down_incy[DOWN] = inc * -1;
    k_down_incx[RIGHT] = inc;
    k_down_incy[UP] = inc;
    k_down_incx[LEFT] = inc * -1;

    if (key == REPEAT_ARROW)
        key = last_arrow;
    else
	last_arrow = key;

    if ((key == down_cursor_down) && ptr_mods)
        return (send_motion (hil_info, key, 0, inc * -1, DOWN));
    else if (key == up_cursor_down)
        {
        k_down_flag[DOWN] = 0;
        return (1);
        }
    else if ((key == down_cursor_left) && ptr_mods)
        return (send_motion (hil_info, key, inc * -1, 0, LEFT));
    else if (key == up_cursor_left)
        {
        k_down_flag[LEFT] = 0;
        return (1);
        }
    else if ((key == down_cursor_right) &&  ptr_mods)
        return (send_motion (hil_info, key, inc, 0, RIGHT));
    else if (key == up_cursor_right)
        {
        k_down_flag[RIGHT] = 0;
        return (1);
        }
    else if ((key == down_cursor_up) &&  ptr_mods)
        return (send_motion (hil_info, key, 0, inc, UP));
    else if (key == up_cursor_up)
        {
        k_down_flag[UP] = 0;
        return (1);
        }
    else if (key == button_1_down)
        return (send_button (hil_info, ButtonPress, 1));
    else if (key == button_2_down)
        return (send_button (hil_info, ButtonPress, 2));
    else if (key == button_3_down)
        return (send_button (hil_info, ButtonPress, 3));
    else if (key == button_4_down)
        return (send_button (hil_info, ButtonPress, 4));
    else if (key == button_5_down)
        return (send_button (hil_info, ButtonPress, 5));
    else if (key == button_6_down)
        return (send_button (hil_info, ButtonPress, 6));
    else if (key == button_7_down)
        return (send_button (hil_info, ButtonPress, 7));
    else if (key == button_8_down)
        return (send_button (hil_info, ButtonPress, 8));
    else if (key == button_1_up )
        return (send_button (hil_info, ButtonRelease, 1));
    else if (key == button_2_up )
        return (send_button (hil_info, ButtonRelease, 2));
    else if (key == button_3_up )
        return (send_button (hil_info, ButtonRelease, 3));
    else if (key == button_4_up )
        return (send_button (hil_info, ButtonRelease, 4));
    else if (key == button_5_up )
        return (send_button (hil_info, ButtonRelease, 5));
    else if (key == button_6_up )
        return (send_button (hil_info, ButtonRelease, 6));
    else if (key == button_7_up )
        return (send_button (hil_info, ButtonRelease, 7));
    else if (key == button_8_up )
        return (send_button (hil_info, ButtonRelease, 8));
    return (0);
    }

/****************************************************************************
 *
 * Send motion information from the keyboard, when it is the pointer device.
 *
 */

send_motion (hil_info, key, x, y, which)
    struct	dev_info     *hil_info;
    u_char	key;
    short x, y, which;
    {
    int	i;

    for (i=0; i<4; i++)
        if (i != which && k_down_flag[i] != 0)
            {
            x += k_down_incx[i];
            y += k_down_incy[i];
            }
    k_down_flag[which] = 1;
    process_motion (hil_info, other_p[XPOINTER], x, y, 0);
    return (1);
    }

/****************************************************************************
 *
 * Send button information from the keyboard, when it is the pointer device.
 *
 */

send_button (hil_info, direction, code)
    struct	dev_info     *hil_info;
    u_char direction, code;
    {
    char	*time_stamp = hil_info->timestamp;

    if (code == last_key && direction == last_direction)
	return (1);
    last_key = code;
    last_direction = direction;

    put_keyevent (code, direction, hilEventTime(time_stamp), MOUSE,
	other_p[XPOINTER], hil_info);
    return (1);
    }

/****************************************************************************
 *
 * find out how far the mouse moved.
 *
 */

#define X3D_MOTION_MASK 	3

get_move (hil_info, x, y, z)
    struct	dev_info  *hil_info;
    short	*x, *y, *z;

    {
    *x = hil_info->dev_data[0];
    *y = hil_info->dev_data[1];
    if ((hil_info->poll_hdr & X3D_MOTION_MASK) == X3D_MOTION_MASK)
        *z = hil_info->dev_data[2];
    else
        *z = 0;
    }


/****************************************************************************
 *
 * process_motion (hil_info)
 *
 */

#define DEF_ACCELERATION	1			

process_motion ( hil_info, dev_p, x, y, z)
    struct	dev_info  *hil_info;
    PtrPrivRec	*dev_p;
    short	x, y, z;
    {
    xEvent	*event;
    int		ZaphodMode = 1;
    ScreenPtr  	pScreen = dev_p->pScreen;
    register 	cfbPrivScreenPtr cfb =  (cfbPrivScreenPtr) pScreen->devPrivate;
   
    /* compute x,y taking care of desired threshold and acceleration
     */

    if ( acceleration > DEF_ACCELERATION) 
	{
	if ( (x - threshold) > 0)
	    x = threshold + (x  - threshold)*acceleration;
	else if ( (x +threshold) < 0)
	    x = (x + threshold)*acceleration - threshold;

        if ( (y - threshold) > 0)
	    y = threshold + (y  - threshold)*acceleration;
        else if ( (y +threshold) < 0)
	    y = (y + threshold)*acceleration - threshold;

        if ( (z - threshold) > 0)
	    z = threshold + (z  - threshold)*acceleration;
        else if ( (z +threshold) < 0)
	    z = (z + threshold)*acceleration - threshold;
        }

    if (hil_info->hil_dev->dev_type != NINE_KNOB)
	y = -y;

    if (hil_info->hil_dev->mode == ABSOLUTE)
	{
	dev_p->x = dev_p->x + x;
	dev_p->y = dev_p->y + y;
	dev_p->z = dev_p->z + z;
	}
    else
	{
	dev_p->x = x;
	dev_p->y = y;
	dev_p->z = z;
	}

    /*
     * Active Zaphod implementation:
     *    increment or decrement the current screen
     *    if the x is to the right or the left of
     *    the current screen.
     */

    if (ZaphodMode && screenInfo.numScreens > 1 &&
	(dev_p->x >= dev_p->pScreen->width ||
	 dev_p->x < 0)) 
	{
        (*cfb->CursorOff) (dev_p->pScreen);
	if (dev_p->x < 0) 
	    { 
	    if (dev_p->pScreen->myNum != 0)
		(dev_p->pScreen)--;
	    else
		dev_p->pScreen = &screenInfo.screen[screenInfo.numScreens -1];
    
	    dev_p->x += dev_p->pScreen->width;
	    }
	else 
	    {
	    dev_p->x -= dev_p->pScreen->width;
    
	    if (dev_p->pScreen->myNum != screenInfo.numScreens -1)
		(dev_p->pScreen)++;
	    else
		dev_p->pScreen = &screenInfo.screen[0];
	    }

	cfb->ChangeScreen(dev_p->pScreen);
	NewCurrentScreen(dev_p->pScreen, dev_p->x, dev_p->y);
	hpPlaceCursorInScreen(dev_p->pScreen,dev_p->x,dev_p->y);
	hpActiveScreen = dev_p->pScreen->myNum;
      }

    /*
     * Clip the cursor to stay within the bound of screen.
     */
    if (hil_info->hil_dev->mode == ABSOLUTE &&
       (!hpConstrainXY (&dev_p->x, &dev_p->y)))
	   return;

    move_mouse (hil_info, dev_p);
    }

/****************************************************************************
 *
 *  move_mouse ()
 *  move the sprite, if the device is the pointer.
 *  Also move it if some other device is sending standard events.
 *  In any case, send a motion event to dix.
 *
 */



move_mouse (hil_info, dev_p)
    struct	dev_info  *hil_info;
    PtrPrivRec	  *dev_p;
    {
    int			id 	= hil_info->hil_dev->dev_id;
    int			axes 	= hil_info->hil_dev->hil_header.ax_num;
    int			size	= sizeof(Time) + (axes * sizeof (short));
    unsigned		int 	timestamp = hilEventTime (hil_info->timestamp);
    ScreenPtr  		pScreen = dev_p->pScreen;
    register 		cfbPrivScreenPtr cfb =  (cfbPrivScreenPtr) 
			   pScreen->devPrivate;
   
    xE.u.u.type = hil_info->hil_dev->motion_event;
    xE.u.keyButtonPointer.time = timestamp;
    xE.u.keyButtonPointer.rootX = other_p[XPOINTER]->x;
    xE.u.keyButtonPointer.rootY = other_p[XPOINTER]->y;
    xE.u.keyButtonPointer.pad1 = id;

#ifdef HPINPUT
    if (xE.u.u.type == DeviceMotionNotify)
	{
	xExt.type = xEventExtension;
	xExt.ext_type = DeviceMotionNotify;
	xExt.deviceid = id;
	xExt.axes_count = 2;
	xExt.data[0].ax_num = 0;
	xExt.data[0].ax_val = dev_p->x;
	xExt.data[1].ax_num = 1;
	xExt.data[1].ax_val = dev_p->y;
	if (axes == 3)
	    {
	    xExt.axes_count = 3;
	    xExt.data[2].ax_num = 2;
	    xExt.data[2].ax_val = dev_p->z;
	    }

        *((Time *)  dpmotionBuf[id]) = timestamp;
	dpmotionBuf[id] += sizeof (Time);
        *((short *) dpmotionBuf[id]) = dev_p->x;
	dpmotionBuf[id] += sizeof (short);
        *((short *) dpmotionBuf[id]) = dev_p->y;
	dpmotionBuf[id] += sizeof (short);
	if (axes == 3)
	    {
            *((short *) dpmotionBuf[id]) = dev_p->z;
	    dpmotionBuf[id] += sizeof (short);
	    }
    
        if((dheadmotionBuf[id] + 100*size) == dpmotionBuf[id])
	    dpmotionBuf[id] = dheadmotionBuf[id];
	}
    else
#endif /* HPINPUT */
	{
        (*cfb->MoveMouse) (dev_p->pScreen, dev_p->x,dev_p->y,1);
        pmotionBuf->time = timestamp;
        pmotionBuf->x = dev_p->x;
        pmotionBuf->y = dev_p->y;
    
        if(!((headmotionBuf + 99) == pmotionBuf))
	    pmotionBuf++;
        else
	    pmotionBuf = headmotionBuf;
	}

    }

/****************************************************************************
 *
 * process_mouse_button ()
 *  process the button event of a two button mouse as well as other
 *	kinds of mouse (1, 3, or more button ).
 */

static process_mouse_button (hil_info)
    struct	dev_info *hil_info;
    {
    int			done = FALSE;
    u_char		button;
    int			button_bytes;
    u_char		button_num;
    HPInputDevice	*current_indevice;

    current_indevice = hil_info->hil_dev;
    button_num = (current_indevice->hil_header).p_button_count;
    if ( pending_index >= pending_bytes) 
	done = TRUE;

    while (queue_events_free && !(done)) 
	{
	button = hil_info->dev_data[pending_index];
	pending_index++;
	if ( button_num == 2 )
	    parse_button_three (hil_info, button);
	else
	    if ( button_num > 0)
		parse_keycode ( hil_info, button,
		       current_indevice->x_type);

	if (pending_index >= pending_bytes) 
	    done = TRUE;
        }

    }

/****************************************************************************
 *
 * put_keyevent ( )
 *	put the key code in the event queue.
 */

put_keyevent (keycode, keystate, time_stamp, dev_type, p, hil_info)
    u_char	keycode;
    char	keystate;
    unsigned  int time_stamp;
    int	dev_type;
    PtrPrivRec	*p;
    struct	dev_info	*hil_info;
    {
    int		id = hil_info->hil_dev->dev_id;
    int		dn_event;
    xEvent	*event;
    xEvent	*allocate_event();
#ifdef HPINPUT
    xHPExtensionEvent	*xkExt;
#endif /* HPINPUT */

    dn_event = hil_info->hil_dev->down_event;/* event this device sends  */
    if (dn_event == KeyPress)		/* it's a standard key    event	    */
	id = XKEYBOARD;			/* fake that it came from keyboard  */
    else if (dn_event == ButtonPress)	/* it's a standard button event	    */
	id = XPOINTER;			/* fake that it came from pointer   */

    event = allocate_event ();
    
    event->u.u.type = keystate;
    event->u.u.detail = keycode;
    event->u.keyButtonPointer.time = time_stamp;
    event->u.keyButtonPointer.rootX = other_p[XPOINTER]->x;
    event->u.keyButtonPointer.rootY = other_p[XPOINTER]->y;
    event->u.keyButtonPointer.pad1 = id;

#ifdef HPINPUT
    if (id != XPOINTER && id != XKEYBOARD)
	{
	xkExt = (xHPExtensionEvent *) allocate_event ();
	xkExt->type = xEventExtension;
	xkExt->ext_type = keystate;
	xkExt->deviceid = id;
	xkExt->axes_count = 0;
	}
#endif /* HPINPUT */

    if ( xE.u.u.type == MotionNotify )
        (* hpOther[XPOINTER]->processInputProc) (&xE, hpOther[XPOINTER]);
#ifdef	HPINPUT
    else if ( xE.u.u.type == DeviceMotionNotify )
	{
	id = xE.u.keyButtonPointer.pad1;
        (* hpOther[id]->processInputProc) (&xE, hpOther[id], xTrue);
        (* hpOther[id]->processInputProc) (&xExt, hpOther[id], xFalse);
	}
#endif /* HPINPUT */
    xE.u.u.type = 0;
    }

#ifdef HPINPUT
/****************************************************************************
 *
 * put_proximity_event( )
 *	put the proximity code in the event queue.
 */

put_proximity_event (evtype, time_stamp, p, hil_info)
    char		evtype;
    unsigned  int 	time_stamp;
    PtrPrivRec		*p;
    struct		dev_info	*hil_info;
    {
    int			id = hil_info->hil_dev->dev_id;
    xEvent		*event;
    xHPExtensionEvent	*xkExt;
    xEvent		*allocate_event();

    event = allocate_event ();
    event->u.u.type = evtype;
    event->u.keyButtonPointer.time = time_stamp;
    event->u.keyButtonPointer.rootX = other_p[XPOINTER]->x;
    event->u.keyButtonPointer.rootY = other_p[XPOINTER]->y;
    event->u.keyButtonPointer.pad1 = id;

    xkExt = (xHPExtensionEvent *) allocate_event ();
    xkExt->type = xEventExtension;
    xkExt->ext_type = evtype;
    xkExt->deviceid = id;
    xkExt->axes_count = 0;

    if ( xE.u.u.type == MotionNotify )
        (* hpOther[XPOINTER]->processInputProc) (&xE, hpOther[XPOINTER]);
    else if ( xE.u.u.type == DeviceMotionNotify )
	{
	id = xE.u.keyButtonPointer.pad1;
        (* hpOther[id]->processInputProc) (&xE, hpOther[id], xTrue);
        (* hpOther[id]->processInputProc) (&xExt, hpOther[id], xFalse);
	}
    xE.u.u.type = 0;
    }

#endif /* HPINPUT */
/********************************************************************
 *
 * ProcessInputEvents()
 * This routine is invoked from the dispatcher to route events.  
 * It invokes the dix routines to do this.
 *
 */

#define CLICK_VOICE 		2

ProcessInputEvents()
    {
    int	head, tail, id;
    xEvent	*event;
#ifdef HPINPUT
    xHPExtensionEvent	*x;
#endif /* HPINPUT */
    ScreenPtr  	pScreen = other_p[XPOINTER]->pScreen;
    register 	cfbPrivScreenPtr cfb =  (cfbPrivScreenPtr) pScreen->devPrivate;

    while ( events_queue->head != events_queue->tail) 
	{
        event =  &((events_queue->events)[(events_queue->head)]);

	switch (event->u.u.type) 
	    {
	    case KeyPress:
	        if (keyboard_click)
		    beep(CLICK_VOICE,800,keyboard_click,1);
	    case KeyRelease:
	        ProcessKeyboardEvent(event, hpOther[XKEYBOARD]);
	        break;
	    case ButtonPress:
	    case ButtonRelease:
	    case MotionNotify:
	        ProcessPointerEvent(event, hpOther[XPOINTER]);
	        break;
	    default:
#ifdef HPINPUT
	        if (event->u.u.type == xEventExtension)
		    {
	            x = (xHPExtensionEvent *) event;
		    id = x->deviceid;
	            HpProcessOtherEvent(event, hpOther[id], xFalse);
		    }
	        else
		    {
		    id = event->u.keyButtonPointer.pad1;
	            HpProcessOtherEvent(event, hpOther[id], xTrue);
		    }
#endif /* HPINPUT */
	    break;
	    }

	if (events_queue->head == max_events)
	    events_queue->head = 0;
	else
	    events_queue->head++;
        }
    (*cfb->MoveMouse)(pScreen,other_p[XPOINTER]->x,other_p[XPOINTER]->y,0);
    }

#ifdef XTESTEXT1
void
XTestGenerateEvent(dev_type, keycode, keystate, mousex, mousey)
	int	dev_type;
	int	keycode;
	int	keystate;
	int	mousex;
	int	mousey;
{
	struct dev_info		hil_info;
	PtrPrivRec		*tmp_ptr;

	ProcessInputEvents();
	/*
	 * the server expects to have the x and y position of the locator
	 * when the action happened placed in other_p[XPOINTER]
	 */
	if (dev_type == MOUSE)
	{
		hil_info.hil_dev = &l_devs[XPOINTER];
		other_p[XPOINTER]->x = mousex;
		other_p[XPOINTER]->y = mousey;
		tmp_ptr = other_p[XPOINTER];
	}
	else
	{
		hil_info.hil_dev = &l_devs[XKEYBOARD];
		other_p[XKEYBOARD]->x = mousex;
		other_p[XKEYBOARD]->y = mousey;
		tmp_ptr = other_p[XKEYBOARD];
	}
	/*
	 * convert the keystate back into server-dependent state values
	 */
	if (keycode < 8 )
	{
		/*
		 * if keycode < 8, this is really a button. 
		 */
		if (keystate == XTestKEY_UP)
		{
			keystate = ButtonRelease;
		}
		else
		{
			keystate = ButtonPress;
		}
	}
	else
	{
		if (keystate == XTestKEY_UP)
		{
			keystate = KeyRelease;
		}
		else
		{
			keystate = KeyPress;
		}
	}

	/*
	 * set the last event time so that the screen saver code will
	 * think that a key has been pressed
	 */
	lastEventTime = GetTimeInMillis();

	put_keyevent(keycode,
		     keystate,
		     0,
		     dev_type,
		     tmp_ptr,
		     &hil_info);

	ProcessInputEvents();
}


void
XTestGetPointerPos(fmousex, fmousey)
	short *fmousex, *fmousey;
{
	*fmousex = other_p[XPOINTER]->x;
	*fmousey = other_p[XPOINTER]->y;
}

/******************************************************************************
 *
 *	Tell the server to move the mouse.
 *
 */
void
XTestJumpPointer(jx, jy, dev_type)
/*
 * the x and y position to move the mouse to
 */
int	jx;
int	jy;
/*
 * which device is supposed to move (ignored)
 */
int	dev_type;
{
	struct dev_info		hil_info;

	/*
	 * set the last event time so that the screen saver code will
	 * think that the mouse has been moved
	 */
	lastEventTime = GetTimeInMillis();
	/*
	 * tell the server where the mouse is being moved to
	 */
	other_p[XPOINTER]->x = jx;
	other_p[XPOINTER]->y = jy;
	hil_info.hil_dev = &l_devs[XPOINTER];
	/*
	 * move the mouse
	 */
	move_mouse(&hil_info, other_p[XPOINTER]);
	/*
	 * tell the server to process the motion event that we just
	 * specified in the move_mouse routine
	 */
	if ( xE.u.u.type == MotionNotify )
	{
		(* hpOther[XPOINTER]->processInputProc) (&xE, hpOther[XPOINTER]);
		xE.u.u.type = 0;
	}
}
#endif /* XTESTEXT1 */
