#ifndef HILDEF_H
#define HILDEF_H
/* $XConsortium: hildef.h,v 1.3 88/09/06 15:25:40 jim Exp $ */
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
/*
** 	File: hildefs.h
**
**	 defines for hil devices to the X environment.
**
*/

#include "XInputExt.h"

# define MAX_X_NAMELEN		64
# define MAX_PHYSICAL_DEVS	7
# define ILLEGAL  		-1
# define UP_MASK   		1 << 0
# define MOTION_MASK 		1 << 1
# define SET1_KEY_MASK 		1 << 6
# define PROXIMITY_IN   	0x8e
# define PROXIMITY_OUT		0x8f
# define MOUSE_BASE_CODE 	0x80
# define BUTTON_1_OFFSET 	0x7e


# define TRUE   1
# define FALSE  0


# define MOUSE_BASE_CODE 0x80	/* Mouse codes are 0x80 up to 0x8d */


/*
**	Button codes for the X applications.
*/

# define XMOUSE_LEFT	 1
# define XMOUSE_MIDDLE	 2
# define XMOUSE_RIGHT	 3
# define XMOUSE_BUTTON_4 4

# define  MOTION_ONLY	0
# define  BUTTON_ONLY	1
# define  MOTION_AND_BUTTON 2

/*
 * The Keyboard ids for hp keyboards
 */

#define COMPRESSED	0x00
#define EXTENDED	0x20
#define STANDARD	0x40

#define SWISSFR		0x03
#define CANADIANENG	0x07
#define ITALIAN		0x0b
#define DUTCH		0x0d
#define SWEDISH		0x0e
#define GERMAN		0x0f
#define SPANISH		0x13
#define FLEMISH		0x15
#define FINNISH		0x16
#define UK		0x17
#define FRENCHCAN	0x18
#define SWISSGER	0x19
#define NORWEGIAN	0x1a
#define FRENCH		0x1b
#define DANISH		0x1c
#define KATAKANA	0x1d
#define LATINAMERICAN	0x1e
#define US		0x1f


#define	HILDRH_TWO_AXES	0x80	/* Device contains two independent sets axes */
#define	HILDRH_ABS_POS	0x40	/* Device has absolute positioning data */
#define HILDRH_16BIT_POS 0x20	/* Device has position data 16 bit accuracy */
#define HILDRH_IODB	0x10	/* Device has I/O description byte */
#define HILDRH_EDES	0x08	/* Device supports extended describe */
#define HILDRH_RSC	0x04	/* Device supports report security code */
#define HILDRH_AXES	0x03	/* Number of axes supported */

#define HILIOB_PAA	0x80	/* Device supports prompt and acknowledge */
#define HILIOB_NPA	0x70	/* Number of prompts & acknowledges supported */
#define HILIOB_PIO	0x08	/* Device supports Proximity In/Out */
#define HILIOB_BUTTONS	0x07	/* Number of buttons on device */

struct	hil_desc_record {
    int	resolution;		/* counts / meter		*/
    u_short size_x, size_y, size_z;
    short	num_cords; 	/* 0, 1, 2 			*/
    short	position ;  	/* 1 = absolute, 0 = relative 	*/
    short	bits_axes;	/* 8 or 16 			*/	
    short	ax_num;		/* number of axes		*/
    				/*  0 = kboard, button;		*/
				/*  1 = X, 2 = X,Y; 3 = X,Y,Z   */
    short  	min_kcode;	/* minimum keycode           	*/
    short  	max_kcode;	/* maximum keycode           	*/
    u_char	id;		/* device id			*/
    u_char	flags;		/* device record header 	*/
    u_char	iob;		/* I/O descriptor Byte 		*/
    char	io_type	 ; 	/* 0 = keyboard; 1 :  other 	*/
    char	ext_command;	/* 0 = No extended command. 	*/
    char	security;	/* 0 = No security; 1 yes	*/
    u_char p_button_count; 	 /* count of physical buttons 	*/
    u_char v_button_count; 	 /* count of virtual buttons  	*/
    u_char 	num_keys; 	 /* number of keys            	*/
} ;

typedef  struct	 _indevices {
    struct  	hil_desc_record hil_header;  /* HIL hdr */
    float	scaleX; 	 /* Tablet scaling 	*/
    float	scaleY; 	 /* Tablet scaling 	*/
    int		file_ds;
    DeviceClientsPtr clients;	/* clients using device */
    u_short 	screenSizeX; 	/* Size of screen 	*/
    u_short 	screenSizeY;    /* Size of screen 	*/
    u_short	axis_bytes;
    u_char	dev_type;	/* HIL device type	*/
    char	x_type;		/* MOUSE or KEYBOARD	*/ 
    u_char	dev_id;	  	/* device id	 	*/ 
    u_char	mode;     	/* abs or rel movement  */
    u_char	up_event;
    u_char	down_event;
    u_char	motion_event;
    u_char	open_cnt;
    char	dev_name[MAX_X_NAMELEN];
    char	x_name[MAX_X_NAMELEN];
}  HPInputDevice;

struct	dev_info {
    char		timestamp[4];
    char		poll_hdr;
    char		dev_data[12];
    HPInputDevice	*hil_dev;
}; 

struct	inputs_selected
	{ 
	int	num_readable;
	int	input_mask;
	int	bit_num;
 	};
#endif
