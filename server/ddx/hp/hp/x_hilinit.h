#ifndef X_HILINIT_H
#define X_HILINIT_H
/* $XConsortium: x_hilinit.h,v 1.2 88/09/06 15:25:55 jim Exp $ */
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
/**************************************************************************
 *
 * file: x_hilinit.h
 *
 * contains key definitions and other static information used by x_hilinit.c
 *
 */
 
#ifdef hp9000s300
#define	MAX_POSITIONS		7
#else
#define	MAX_POSITIONS		28
#endif							/* hp9000s300	*/
#define	STARTUP			0
#define	MAX_DEV_TYPES		(sizeof (devices) / sizeof (struct dev_table))
#define	MAX_POINTER_FUNCS	(sizeof (pointerfunc) / sizeof (struct pointerfunc))
#define	QUAD_INDEX		7	/* index of quad entry in dev_table */
#define	NINE_KNOB_ID		0x61
#define	KEY			0
#define	NUMBER			1

struct	opendevs
    {
    int	type;
    int	pos;
    };

struct	dev_table
    {
    int			lowid;
    int			highid;
    int			dev_type;
    int			x_type;
    char		*name;
    unsigned char	num_keys;
    unsigned char	min_kcode;
    unsigned char	max_kcode;
    };

struct	dev_table  devices[] =
	{{0x00,0x1f,KEYPAD,KEYBOARD,"KEYPAD",0,0,0},
	 {0x30,0x33,BUTTONBOX,KEYBOARD,"BUTTONBOX",32,8,39},
	 {0x34,0x34,ID_MODULE,XOTHER,"ID_MODULE",0,0,0},
	 {0x35,0x3f,BUTTONBOX,KEYBOARD,"BUTTONBOX",32,8,39},
	 {0x5c,0x5f,BARCODE,KEYBOARD,"BARCODE",109,8,135},
	 {0x60,0x60,ONE_KNOB,MOUSE,"ONE_KNOB",0,0,0},
	 {0x61,0x61,NINE_KNOB,MOUSE,"NINE_KNOB",0,0,0},
	 {0x62,0x67,QUADRATURE,MOUSE,"QUADRATURE",0,0,0},
	 {0x68,0x6b,MOUSE,MOUSE,"MOUSE",0,0,0},
	 {0x6c,0x6f,TRACKBALL,MOUSE,"TRACKBALL",0,0,0},
	 {0x88,0x8b,TOUCHPAD,MOUSE,"TOUCHPAD",0,0,0},
	 {0x8c,0x8f,TOUCHSCREEN,MOUSE,"TOUCHSCREEN",0,0,0},
	 {0x90,0x97,TABLET,MOUSE,"TABLET",0,0,0},
	 {0xA0,0xBF,KEYBOARD,KEYBOARD,"KEYBOARD",93,8,135},
	 {0xC0,0xDF,KEYBOARD,KEYBOARD,"KEYBOARD",109,8,135},
	 {0xE0,0xFF,KEYBOARD,KEYBOARD,"KEYBOARD",87,8,135},
	 {0x00,0x00,NULL_DEVICE,NULL_DEVICE,"NULL",0,0,0}};

char	*position[] = 
    {
    "FIRST",
    "SECOND",
    "THIRD",
    "FOURTH",
    "FIFTH",
    "SIXTH",
    "SEVENTH"
    };

extern	u_char	down_cursor_down;
extern	u_char	up_cursor_down;
extern	u_char	down_cursor_left;
extern	u_char	up_cursor_left;
extern	u_char	down_cursor_right;
extern	u_char	up_cursor_right;
extern	u_char	down_cursor_up;
extern	u_char	up_cursor_up;
extern	u_char	button_1_down;
extern	u_char	button_1_up;
extern	u_char	button_2_down;
extern	u_char	button_2_up;
extern	u_char	button_3_down;
extern	u_char	button_3_up;
extern	u_char	button_4_down;
extern	u_char	button_4_up;
extern	u_char	button_5_down;
extern	u_char	button_5_up;
extern	u_char	button_6_down;
extern	u_char	button_6_up;
extern	u_char	button_7_down;
extern	u_char	button_7_up;
extern	u_char	button_8_down;
extern	u_char	button_8_up;
extern	u_char	pointer_key_mod1;
extern	u_char	pointer_key_mod2;
extern	u_char	pointer_key_mod3;
extern	u_char	pointer_amt_mod1;
extern	u_char	pointer_amt_mod2;
extern	u_char	pointer_amt_mod3;
extern	u_char	pointer_move;
extern	u_char	pointer_mod1_amt;
extern	u_char	pointer_mod2_amt;
extern	u_char	pointer_mod3_amt;
extern	u_char	reset_down;
extern	u_char	reset_up;
extern	u_char	reset_mod1;
extern	u_char	reset_mod2;
extern	u_char	reset_mod3;
    
struct	pointerfunc
    {
    char	*name;
    u_char	*down;
    u_char	*up;
    int		type;
    };

struct	pointerfunc pointerfunc [] =
	{{"pointer_left_key", &down_cursor_left, &up_cursor_left,KEY},
	 {"pointer_right_key", &down_cursor_right, &up_cursor_right,KEY},
	 {"pointer_up_key", &down_cursor_up, &up_cursor_up,KEY},
	 {"pointer_down_key", &down_cursor_down, &up_cursor_down,KEY},
	 {"pointer_key_mod1", &pointer_key_mod1, NULL,KEY},
	 {"pointer_key_mod2", &pointer_key_mod2, NULL,KEY},
	 {"pointer_key_mod3", &pointer_key_mod3, NULL,KEY},
	 {"pointer_button1_key", &button_1_down, &button_1_up,KEY},
	 {"pointer_button2_key", &button_2_down, &button_2_up,KEY},
	 {"pointer_button3_key", &button_3_down, &button_3_up,KEY},
	 {"pointer_button4_key", &button_4_down, &button_4_up,KEY},
	 {"pointer_button5_key", &button_5_down, &button_5_up,KEY},
	 {"pointer_button6_key", &button_6_down, &button_6_up,KEY},
	 {"pointer_button7_key", &button_7_down, &button_7_up,KEY},
	 {"pointer_button8_key", &button_8_down, &button_8_up,KEY},
	 {"pointer_move", &pointer_move, &pointer_move,NUMBER},
	 {"pointer_mod1_amt", &pointer_mod1_amt, &pointer_mod1_amt,NUMBER},
	 {"pointer_mod2_amt", &pointer_mod2_amt, &pointer_mod2_amt,NUMBER},
	 {"pointer_mod3_amt", &pointer_mod3_amt, &pointer_mod3_amt,NUMBER},
	 {"reset", &reset_down, &reset_up,KEY},
	 {"reset_mod1", &reset_mod1, NULL,KEY},
	 {"reset_mod2", &reset_mod2, NULL,KEY},
	 {"reset_mod3", &reset_mod3, NULL,KEY},
	 {"pointer_amt_mod1", &pointer_amt_mod1, NULL,KEY},
	 {"pointer_amt_mod2", &pointer_amt_mod2, NULL,KEY},
	 {"pointer_amt_mod3", &pointer_amt_mod3, NULL,KEY}};

char *keyset1[] = {
    "5",
    "",
    "right_extend",
    "left_extend",
    "right_shift",
    "left_shift",
    "control",
    "break",
    "keypad_4",
    "keypad_8",
    "keypad_5",
    "keypad_9",
    "keypad_6",
    "keypad_7",
    "keypad_comma",
    "keypad_enter",
    "keypad_1",
    "keypad_/",
    "keypad_2",
    "keypad_+",
    "keypad_3",
    "keypad_*",
    "keypad_0",
    "keypad_-",
    "B",
    "V",
    "C",
    "X",
    "Z",
    "",
    "",
    "escape",
    "6",
    "blank_f10",
    "3",
    "blank_f11",
    "keypad_period",
    "blank_f9",
    "keypad_tab",
    "blank_f12",
    "H",
    "G",
    "F",
    "D",
    "S",
    "A",
    "",
    "caps_lock",
    "U",
    "Y",
    "T",
    "R",
    "E",
    "W",
    "Q",
    "tab",
    "7",
    "6",
    "5",
    "4",
    "3",
    "2",
    "1",
    "`",
    "button_1",
    "button_2",
    "button_3",
    "button_4",
    "button_5",
    "button_6",
    "button_7",
    "proximity_in_out",
    "menu",
    "f4",
    "f3",
    "f2",
    "f1",
    "8",
    "stop",
    "enter",
    "system",
    "f5",
    "f6",
    "f7",
    "f8",
    "9",
    "clear_line",
    "clear_display",
    "8",
    "9",
    "0",
    "-",
    "=",
    "backspace",
    "insert_line",
    "delete_line",
    "I",
    "O",
    "P",
    "[",
    "]",
    "\\",
    "insert_char",
    "delete_char",
    "J",
    "K",
    "L",
    ";",
    "'",
    "return",
    "home_cursor",
    "prev",
    "M",
    ",",
    ".",
    "/",
    "",
    "select",
    "",
    "next",
    "N",
    "space_bar",
    ".",
    "",
    "cursor_left",
    "cursor_down",
    "cursor_up",
    "cursor_right"};

#endif
