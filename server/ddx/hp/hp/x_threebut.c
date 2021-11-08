/* $XConsortium: x_threebut.c,v 1.5 88/10/01 10:36:38 rws Exp $ */
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
 * file: x-threebut.c
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
 *			Converts a two button mouse to a soft-three
 *			button mouse. 
 *		Hewlett Packard -- Corvallis Workstation Operation
 *		Project -- port of X to HP9000S300
 *		Sankar L. Chakrabarti -- MTS.
 */

#include <sys/types.h>
#include "hildef.h"
#include "sun.h"
#include "XHPproto.h"
#include "x_threebut.h"

#ifdef	XTESTEXT1
/*
 * defined in xtestext1di.c
 */
extern int	on_steal_input;		/* steal input mode is on.	*/
extern int	exclusive_steal;
#endif  /* XTESTEXT1 */

extern PtrPrivRec *other_p[];
extern  int	data_cnt;

int	mouse_state = Lup_Rup_Mup;
int	s_mask = 0;
int	mouse_ds;		/* file descriptor of two button mouse. */
int	s_max;
int	XButtonPress;
int	XButtonRelease;

static	u_char	next_mouse_key();	
unsigned int	hilEventTime();

struct	timeval	  wait_time =
	{ 
	LOOKAHEAD_SEC,
	LOOKAHEAD_USEC
	};

struct	mouse_result
	{ 
	short	keycode;	/* 0,1,2, 3 */
	short	keystate;	/* UP or DOWN	*/
	short	transition_type; /* reportable to X server? */
	}  mouse_key,
	   look_ahead;

struct	mouse_two_button
	{  
	struct	mouse_result  button_state;
	short	locx,
		locy;
	int	timestamp;
	}	two_button;

short	Xmouse_map[ ] =		/* maps HP mouse code X code	*/
	{  XMOUSE_LEFT,
	   XMOUSE_LEFT,
	   XMOUSE_RIGHT,
	   XMOUSE_RIGHT,
	   XMOUSE_MIDDLE,
	   XMOUSE_MIDDLE,   
	   XMOUSE_BUTTON_4,
	   XMOUSE_BUTTON_4
	};

/****************************************************************
 *
 *
 *
 */

X_mouse_three ( file_ds)
    int	file_ds;
    {
    mouse_ds = file_ds;
    s_max = mouse_ds + 1;
    s_mask = 1 << mouse_ds;
    }

/****************************************************************
 *
 *	next_mouse_key ( hil_info)
 *
 */

static	u_char
next_mouse_key (hil_info, id)
    struct	dev_info	*hil_info;
    int		id;
    {	
    int		there_is_data =0;
    int		bytes_read;
    short	x, y, z;
    u_char	this_key = NO_OPKEY;
    char	packet_size = 0,
		pollhdr;

    if (data_cnt != 0)
	{
	there_is_data = TRUE;
	packet_size = get_hil_event ( mouse_ds, hil_info);
	}
    else
	{
	there_is_data = read (mouse_ds, &packet_size, 1);
        if ( there_is_data)
	    bytes_read = read (mouse_ds, hil_info, (packet_size - 1));
	}

    if ( there_is_data)
	{  
	pollhdr = hil_info->poll_hdr;
	if (( pollhdr & SET1_KEY_MASK) &&
	    ( pollhdr & MOTION_MASK) )
	    { 
	    hil_info->poll_hdr &= ~(MOTION_MASK);
	    get_move (hil_info, &x, &y, &z);
	    process_motion (hil_info, other_p[id], x, y, z);
	    this_key =  hil_info->dev_data[2];
	    hil_info->poll_hdr &= ~(SET1_KEY_MASK);
	    }
	else
	    { 
	    if (pollhdr & MOTION_MASK)
		{ 
	        get_move (hil_info, &x, &y, &z);
                process_motion (hil_info, other_p[id], x, y, z);
		}
	    else
	        { 
		if ( pollhdr & SET1_KEY_MASK)
		    this_key =  hil_info->dev_data[0];
		else
		    printf ("Next Key error. \n");
	        }
	    }
	}
    return ( this_key);
    }
			
/****************************************************************
 *
 * next_mouse_state ( state, stimulus)
 *	:: return next state;
 *	Aleters the globlas: mouse_state, keystate;
 */

int
next_mouse_state (state, stimulus, mouse_key)
    int	state;
    u_char	stimulus;
    struct	mouse_result	*mouse_key;
    {	
    int	keycode;
    struct	stat_button	*new_state;


    keycode =  ( (int) stimulus) - MOUSE_BASE_CODE;
    new_state = &(stat_table [state] [keycode]);
    if ( (new_state->button < 0) | (new_state->button > 7) )
	{   
	printf ("ILLEGAL New state Button %d. \n",
			new_state->button);
	printf ( "Input State %d ; Input Keycode %d, Stimulus %x\n",
			state, keycode, stimulus);
	printf ("Current State : State %d,  button %d \n",
			new_state->state, new_state->button);
	getchar();
	}
    mouse_key->keycode = Xmouse_map [new_state->button];
    if ((mouse_key->keycode < 1) | (mouse_key->keycode > 4))
	printf ("ILLEGAL MOUSE CODE %d BEING SENT \n", 
			mouse_key->keycode);

    if ( new_state->button == Mup || ((new_state->button) & UP_MASK))
	mouse_key->keystate = XButtonRelease;
    else if ( new_state->button == Mdn || !((new_state->button) & UP_MASK))
	mouse_key->keystate = XButtonPress;

    mouse_key->transition_type = new_state->transition_type;
    if (new_state->state < 0)
	{ 
	printf ("Error: Mouse entered illegal state. \n");
	printf (" Entering state =%d, keycode =%d mouse_key=%d stimulus=%d\n", 
	    state, keycode, mouse_key, stimulus);
	}
    return (new_state->state);
    }

/*************************************************************************
 * 
 * parse_button_three ()	
 *	 :: compute current state of the mouse, save it;
 *	Look ahead for the next state; if the two states are to be merged
 *	to generate a new state (like middle button) then do so and report
 *	the merged state.
 *	Else report both the current state and look_ahead state;
 */

int
parse_button_three ( hil_info, keycode)
    struct	dev_info   *hil_info;
    u_char		keycode;
    {	
    int	id = hil_info->hil_dev->dev_id;
    u_char	next_key = NO_OPKEY;
    int     num_read = 0;
    int	curstate;
    int	mask;

    if (hil_info->hil_dev->motion_event == MotionNotify)
	id = XPOINTER;
    XButtonPress = hil_info->hil_dev->down_event;
    XButtonRelease = hil_info->hil_dev->up_event;
    curstate = next_mouse_state ( mouse_state,keycode, &(mouse_key));
    /*
     * fill up the two_button mouse structure.
     */
    two_button.button_state = mouse_key;
    two_button.locx	=  other_p[id]->x;	
    two_button.locy =  other_p[id]->y; 
    two_button.timestamp = hilEventTime (hil_info->timestamp);
    if  ( ( (curstate == Lup_Rdn_Mup) || (curstate == Ldn_Rup_Mup))
	&& ( mouse_state == Lup_Rup_Mup))
	{	/* look ahead for the next key */
	mouse_state = curstate;
	mask = s_mask;

	if (data_cnt == 0)
  	    num_read = select (s_max, &(mask), (int *) NULL, 
		(int *) NULL, &(wait_time));
	else
	    num_read = 1;

        if (num_read > 0)
	    { 
	    next_key = next_mouse_key (hil_info, id);
	    if ( next_key > NO_OPKEY)
		{ 
		curstate = next_mouse_state ( curstate, next_key,
						&look_ahead);
		if (curstate == Ldn_Rdn_Mup)
		    { /* map it to nonexistent third button.	*/
		     mouse_state = Lup_Rup_Mdn;
		     mouse_key.keycode = XMOUSE_MIDDLE;
		     mouse_key.keystate= XButtonPress;
		     mouse_key.transition_type = REPORTABLE;
		     two_button.button_state = mouse_key;
		     two_button.timestamp = hilEventTime( hil_info->timestamp);
		     }
		else
		     { 
		     mouse_state = curstate;
		     /*
		      * report the saved event.
		      */
		     put_button_event(hil_info,other_p[id]);
		     mouse_key = look_ahead;
		     two_button.button_state = mouse_key;
		     two_button.timestamp = hilEventTime(hil_info->timestamp);
		     }
		}
	    }
	}
    else
	{
	mouse_state = curstate;
	}
    put_button_event (hil_info,other_p[id]);
    return (mouse_state);
    }

/***********************************************************************
 *
 * put_button_event (hil_info)
 *	put the mouse button event on the event queue.
 *	The caller must be sure that there is place on te queue;
 *	This routine does not check. It merely grabs an event and writes
 *	on it. It increments tail however.
 */

put_button_event(hil_info, p)
    struct	dev_info	*hil_info;
    PtrPrivRec	*p;
    {	
    u_char	keycode;
    char	keystate;
    int		dev_type;

    if ( two_button.button_state.transition_type == REPORTABLE)
	{ 
	keycode = two_button.button_state.keycode;
	keystate = two_button.button_state.keystate;
	dev_type = MOUSE;

#ifdef	XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(keycode, keystate, dev_type,
			      two_button.locx, two_button.locy))
#endif /* XTESTEXT1 */
	put_keyevent (two_button.button_state.keycode,
	    two_button.button_state.keystate,
	    (u_short) two_button.timestamp,
	    MOUSE, 
	    p, hil_info);
	}
    }
