/* $XConsortium: x_tablet.c,v 1.4 88/10/01 10:37:42 rws Exp $ */
/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
/***********************************************************************
 * file: x-tablet.c
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
 *		Puts graphics tablets/ digitizers in the X environment.
 *		Makes them behave like a mouse.
 *		Hewlett Packard -- Corvallis Workstation Operation
 *		Project -- port of X to HP9000S300
 *		Sankar L. Chakrabarti -- MTS.
 *	3/5/87:
 *	   Being Modified for incorporation in V11.
 *
 */

/********************************************************************
*	Problem: In version 10, there was only one display, so the
*	 mapping between the display and the tablet could be done
*	 only once at initialization. But in version 11, with multiple
*	 displays, the mapping has to be restablished whenever the
*	 pointer moves to a different display. This is a problem to be
*	 solved. It should be done as part of context shift whenever
*	 pointer moves to a different display. The mi-lib should detect
*	 it and call init routines in this file. To support this, we may
*	 need to restructure the routine. Shall do it when, X supports 
*	 multiple screens.
*	    -- Sankar   3/10/87
***********************************************************************/


#include <sys/types.h>
#include "hildef.h"
#include "XHPproto.h"
#include "sun.h"

#ifdef	XTESTEXT1
/*
 * defined in xtestext1di.c
 */
extern int	on_steal_input;
#endif	/* XTESTEXT1 */

int	tablet_active = FALSE,
	vmousex = -1,   	/* tablet-x mapped to mouse X on display*/
	vmousey = -1;   	/* tablet-y mapped to mouse Y on display*/
u_char	pending_tab_button; 	/* pending keycode of the tablet 	*/

extern PtrPrivRec 	*other_p[];
extern	HPInputDevice	l_devs[MAX_LOGICAL_DEVS];
extern  int		queue_events_free;
extern	short   	Xmouse_map[];	

/****************************************************************
 *
 *	parse_tablet_data ()
 *
 */

parse_tablet_data(hil_info)
    struct	dev_info   *hil_info;
    {
    int		id = hil_info->hil_dev->dev_id;
    int		bytes_coord = 2;
    int		axis_bytes;
    char	pollhdr;

    if (hil_info->hil_dev->motion_event == MotionNotify)
	id = XPOINTER;
    pollhdr = hil_info->poll_hdr;

    /*
     * # bytes of axis info = # axes * bytes/coord.
     */

    if (hil_info->hil_dev->hil_header.bits_axes == 0)
	bytes_coord = 1;

    axis_bytes = hil_info->hil_dev->hil_header.ax_num * bytes_coord;
	
    if ((pollhdr & MOTION_MASK) && (pollhdr & SET1_KEY_MASK)) {
	pending_tab_button = hil_info->dev_data[axis_bytes];
	tab_but_motion (hil_info, id);
    }
    else if ( pollhdr & MOTION_MASK) {
	tab_motion (hil_info, id);
	hil_info->poll_hdr &= ~(MOTION_MASK);
    }
    else if ( pollhdr & SET1_KEY_MASK) {
	pending_tab_button = hil_info->dev_data[0];
	put_tab_button (hil_info, id);
	hil_info->poll_hdr &= ~(SET1_KEY_MASK);
    }
    else
	printf ("Unknown Type of Tablet  data \n");
}

/*
 *
 * tab_but_motion ()
 *	process a hil_info packet of button and motion data.
 *	Change the values of other_p;
 *
 */
tab_but_motion(hil_info, id)
    struct 	dev_info	*hil_info;
    int		id;
    {
    int	mx, my;
    int	j;
    char	*cp;
    u_char	prx;

    map_tablet_display(hil_info, &mx, &my);

    other_p[id]->x = mx;
    other_p[id]->y = my;
    other_p[id]->z = 0;

#ifdef	XTESTEXT1
	if (on_steal_input)
	{
		/*
		 * defined in xtestext1dd.c
		 */
		XTestStealJumpData(mx, my, TABLET);
	}
#endif	/* XTESTEXT1 */

    move_mouse (hil_info, other_p[id]);

    hil_info->poll_hdr &= (~MOTION_MASK);

    put_tab_button (hil_info, id);
    }

/***************************************************************
 *
 *  map_tablet_display ()
 *  map the tablet coordinates to display pixels.
 *
 */


map_tablet_display(hil, mousex, mousey)
struct	dev_info *hil;
int	*mousex, *mousey;
    {
    unsigned  locx, locy;
    u_char	xlo, xhi, ylo, yhi;
    float tx, ty;
    HPInputDevice *dev;
    
    dev = hil->hil_dev;

    if (dev->hil_header.bits_axes == 0)
	{
        locx = hil->dev_data[0];
        locy = hil->dev_data[1];
	}
    else
	{
        xlo = hil->dev_data[0];
        xhi = hil->dev_data[1];
        ylo = hil->dev_data[2];
        yhi = hil->dev_data[3];
        locx = xhi*256 + xlo;
        locy = yhi*256 + ylo;
	}

    if (dev->motion_event == MotionNotify)
	{
        tx = locx * dev->scaleX;
        ty = locy * dev->scaleY;
        *mousex = tx;
        *mousey = dev->screenSizeY - ty; 	/* Y-coord  reversed.*/
	}
    else
	{
        tx = locx;
        ty = locy;
        *mousex = tx;
        *mousey = dev->hil_header.size_y - ty; 	/* Y-coord  reversed.*/
	}
    }

/*******************************************************************
 *
 * tab_motion ()
 *
 */
	
tab_motion (hil_info, id)
    struct	dev_info  *hil_info;
    int		id;
    {	
    int	mx, my;
    int	put_event = FALSE;

	map_tablet_display ( hil_info, &mx, &my);

	if ( mx != vmousex)
	   { 
	   vmousex = mx;
	   other_p[id]->x = mx;		
	   put_event = TRUE;
	   }

	if ( my != vmousey)
	   { 
	   vmousey = my;
	   other_p[id]->y = my;		
	   put_event = TRUE;
	   }
#ifdef	XTESTEXT1
	if (put_event && on_steal_input)
	{
		/*
		 * defined in xtestext1dd.c
		 */
		XTestStealJumpData(mx, my, TABLET);
	}
#endif	/* XTESTEXT1 */
	
	if (put_event) 
    	    move_mouse (hil_info, other_p[id]);

}
		
/*****************************************************************************
** put_tab_button ()
**	puts a tablet button to the event queue. The button is in the
**	variable pending_button;
**	It should be possible to merge this routine with processing of
**	pending button from the mouse_device.
**	But that for the later.
**	The caller must make sure that there is room in the events_queue.
*/

put_tab_button (hil_info, id)
    struct	dev_info	*hil_info;
    int		id;
    {	
    extern	int	ProximityIn;
    extern	int	ProximityOut;
    short	xmouse;
    u_char	keycode;
    char	keystate;
    unsigned int timestamp;

    timestamp = hilEventTime(hil_info->timestamp);

#ifdef HPINPUT
    if (pending_tab_button == PROXIMITY_IN)
	put_proximity_event (ProximityIn, timestamp, other_p[id], hil_info);
    else if (pending_tab_button == PROXIMITY_OUT)
	put_proximity_event (ProximityOut, timestamp, other_p[id], hil_info);
#else
    if (pending_tab_button == PROXIMITY_IN)
	return;
    else if (pending_tab_button == PROXIMITY_OUT)
	return;
#endif /* HPINPUT */
    else
	{
	xmouse =  Xmouse_map[ pending_tab_button - MOUSE_BASE_CODE];
	keycode = (u_char) xmouse;
	if (pending_tab_button & UP_MASK)
	    keystate = hil_info->hil_dev->up_event;
	else	
	    keystate = hil_info->hil_dev->down_event;

#ifdef	XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(keycode, keystate, TABLET,
			      other_p[id]->x, other_p[id]->y))
#endif	/* XTESTEXT1 */
	put_keyevent( keycode, keystate, timestamp, MOUSE, other_p[id], 
	    hil_info);
	}

    }
