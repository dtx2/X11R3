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

#include	"X.h"
#include        "Xmd.h"
#define XK_KATAKANA
#include	"keysym.h"
#include        "hpKeyDef.h"
#include	"sun.h"
#include	"input.h"

/* This file was composed from the X10 hil_keymap.h by
 * Jack Palevich, HP-Labs
 */


static KeySym USASCIIMap[] = {

 /* code values in comments at line end are actual value reported on HIL.
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_ampersand,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_asciicircum,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_numbersign,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_at,			XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_quoteleft,		XK_asciitilde,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_asterisk,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenleft,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_parenright,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_minus,		XK_underscore,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_equal,		XK_plus,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_bracketleft,		XK_braceleft,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_bracketright,	XK_braceright,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_backslash,		XK_bar,			NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_semicolon,		XK_colon,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_quoteright,		XK_quotedbl,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_less,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_greater,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_slash,		XK_question,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};

static KeySym LatinMap[] = {

 /* code values in comments at line end are actual value reported on HIL.
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */
	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,		XK_ampersand,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,		XK_asciicircum,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,		XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,		XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,		XK_numbersign,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,		XK_at,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,		XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_quoteleft,	XK_ccedilla,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,		XK_asterisk,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,		XK_parenleft,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,		XK_parenright,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_minus,	XK_underscore,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_equal,	XK_plus,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_mute_acute,	XK_mute_diaeresis,	XK_degree,	NoSymbol,	/* 0x63 */
	XK_quoteright,	XK_quotedbl,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_questiondown,XK_exclamdown,		NoSymbol,	NoSymbol,	/* 0x65 */
	
		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_ntilde,	XK_Ntilde,		XK_lira, 	NoSymbol,	/* 0x6b */
	XK_semicolon,	XK_colon,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,		NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,	XK_less,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,	XK_greater,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_slash,	XK_question,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};

static KeySym KatakanaMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */		
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		XK_Henkan,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		XK_Muhenkan,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		XK_Kanji,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_kana_KO,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_kana_HI,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_kana_SO,	NoSymbol,	/* 0x1a */
	XK_X,			NoSymbol,		XK_kana_SA,	NoSymbol,	/* 0x1b */
	XK_Z,			NoSymbol,		XK_kana_TU,	XK_kana_tu,	/* 0x1c */
/* Was Kanji Left.... */		
	XK_Ext16bit_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_kana_KU,	NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_kana_KI,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_kana_HA,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_kana_SHI,	NoSymbol,	/* 0x2b */
	XK_S,			NoSymbol,		XK_kana_TO,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_kana_TI,	NoSymbol,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_kana_NA,	NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_kana_N,	NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_kana_KA,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_kana_SU,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_kana_I,	XK_kana_i,	/* 0x34 */
	XK_W,			NoSymbol,		XK_kana_TE,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		XK_kana_TA,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_ampersand,		XK_kana_YA,	XK_kana_ya,	/* 0x38 */
	XK_6,			XK_asciicircum,		XK_kana_O,	XK_kana_o,	/* 0x39 */
	XK_5,			XK_percent,		XK_kana_E,	XK_kana_e,	/* 0x3a */
	XK_4,			XK_dollar,		XK_kana_U,	XK_kana_u,	/* 0x3b */
	XK_3,			XK_numbersign,		XK_kana_A,	XK_kana_a,	/* 0x3c */
	XK_2,			XK_at,			XK_kana_HU,	NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_kana_NU,	NoSymbol,	/* 0x3e */
	XK_quoteleft,		XK_asciitilde,		XK_kana_RO,	XK_prolongedsound,	/* 0x3f */
/* Was Mouse-L */		
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */		
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */		
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */		
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */		
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */		
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */		
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_asterisk,		XK_kana_YU,	XK_kana_yu,	/* 0x58 */
	XK_9,			XK_parenleft,		XK_kana_YO,	XK_kana_yo,	/* 0x59 */
	XK_0,			XK_parenright,		XK_kana_WA,	XK_kana_WO,	/* 0x5a */
	XK_minus,		XK_underscore,		XK_kana_HO,	NoSymbol,	/* 0x5b */
	XK_equal,		XK_plus,		XK_kana_HE,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
					
	XK_I,			NoSymbol,		XK_kana_NI,	NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_kana_RA,	NoSymbol,	/* 0x61 */
	XK_P,			NoSymbol,		XK_kana_SE,	NoSymbol,	/* 0x62 */
	XK_bracketleft,		XK_braceleft,		XK_voicedsound,	NoSymbol,	/* 0x63 */
	XK_bracketright,	XK_braceright,		XK_semivoicedsound,XK_kana_openingbracket,/* 0x64 */
	XK_backslash,		XK_bar,			XK_kana_MU,	XK_kana_closingbracket,/* 0x65 */
		
	/* HP special  might also be Insert */				
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_kana_MA,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_kana_NO,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_kana_RI,	NoSymbol,	/* 0x6a */
	XK_semicolon,		XK_colon,		XK_kana_RE,	NoSymbol,	/* 0x6b */
	XK_quoteright,		XK_quotedbl,		XK_kana_KE,	NoSymbol,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */		
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
		
	XK_M,			NoSymbol,		XK_kana_MO,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_less,		XK_kana_NE,	XK_kana_comma,	/* 0x71 */
	XK_period,		XK_greater,		XK_kana_RU,	XK_kana_fullstop,	/* 0x72 */
	XK_slash,		XK_question,		XK_kana_ME,	XK_kana_middledot,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_kana_MI,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */		
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */		
	XK_Ext16bit_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
		
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	


static KeySym DenmarkMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,		XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,		XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,		XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,		XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,		XK_section,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,		XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,		XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_less,     	XK_greater,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,		XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,		XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,		XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_plus,	XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_mute_acute,	XK_mute_grave,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_aring,	XK_Aring,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_mute_diaeresis,XK_asciicircum,	XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_at,		XK_asterisk,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special, might also be Insert */		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_ae,		XK_AE,			XK_lira,	NoSymbol,	/* 0x6b */
	XK_oslash,	XK_Ooblique,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,		NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,	XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,	XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,	XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym FranceMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_W,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_Q,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_Z,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_A,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_egrave,	XK_7,			XK_backslash,	NoSymbol,	/* 0x38 */
	XK_section,	XK_6,			XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_parenleft,	XK_5,			XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_quoteright,	XK_4,			XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_quotedbl,	XK_3,			XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_eacute,	XK_2,			XK_at,		NoSymbol,	/* 0x3d */
	XK_ampersand,	XK_1,			XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_dollar,	XK_sterling,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_exclam,	XK_8,			XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_ccedilla,	XK_9,			XK_bracketright,XK_braceright,	/* 0x59 */
	XK_agrave,	XK_0,			XK_questiondown,NoSymbol,	/* 0x5a */
	XK_parenright,	XK_degree,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_minus,	XK_underscore,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_mute_asciicircum,XK_diaeresis,	XK_degree,	NoSymbol,	/* 0x63 */
	XK_quoteleft,	XK_asterisk,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_less,	XK_greater,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special, might also be Insert */		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_M,		NoSymbol,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_ugrave,	XK_percent,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_comma,	XK_question,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_semicolon,	XK_period,		XK_less,	NoSymbol,	/* 0x71 */
	XK_colon,	XK_slash,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_equal,	XK_plus,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
			
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym NorwayMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,		XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,		XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,		XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,		XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,		XK_numbersign,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,		XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,		XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_less,     	XK_greater,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,		XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,		XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,		XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_plus,	XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_mute_acute,	XK_mute_grave,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_aring,	XK_Aring,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_mute_diaeresis,XK_asciicircum,	XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_at,		XK_asterisk,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special, might also be Insert */		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_oslash,	XK_Ooblique,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_ae,		XK_AE,		 	XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,		NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,	XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,	XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,	XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	

static KeySym Swiss_GermanMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Y,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Z,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_ccedilla,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_asterisk,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_plus,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_section,		XK_degree,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_exclam,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_mute_asciicircum,	XK_mute_grave,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_udiaeresis,		XK_egrave,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_mute_diaeresis,	XK_mute_acute,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_dollar,		XK_sterling,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_odiaeresis,		XK_eacute,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_adiaeresis,		XK_agrave,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
	
static KeySym Canada_FrenchMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,		XK_ampersand,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,		XK_question,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,		XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,		XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,		XK_slash,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,		XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,		XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_bracketright,XK_bracketleft,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,		XK_asterisk,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,		XK_parenleft,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,		XK_parenright,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_minus,	XK_underscore,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_equal,	XK_plus,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_mute_asciicircum,XK_mute_diaeresis,	XK_degree,	NoSymbol,	/* 0x63 */
	XK_ccedilla,	XK_Ccedilla,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_at,		XK_numbersign,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special, might also be Insert */		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_semicolon,	XK_colon,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_mute_grave,	XK_quoteright,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,		NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,	XK_less,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,	XK_greater,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_eacute,	XK_Eacute,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym UKMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_asciicircum,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_sterling,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_at,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_quoteleft,		XK_asciitilde,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_plus,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_quoteright,		XK_slash,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_bracketleft,		XK_braceleft,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_bracketright,	XK_braceright,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_backslash,		XK_bar,			NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_asterisk,		XK_at,			XK_lira,	NoSymbol,	/* 0x6b */
	XK_backslash,		XK_bar,			XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym FinishMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_numbersign,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_less,		XK_greater,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_plus,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_eacute,		XK_Eacute,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_aring,		XK_Aring,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_udiaeresis,		XK_Udiaeresis,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_quoteleft,		XK_asterisk,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_odiaeresis,		XK_Odiaeresis,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_adiaeresis,		XK_Adiaeresis,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym BelgianMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_W,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_Q,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_Z,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_A,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_egrave,	XK_7,			XK_backslash,	NoSymbol,	/* 0x38 */
	XK_section,	XK_6,			XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_parenleft,	XK_5,			XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_quoteright,	XK_4,			XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_quotedbl,	XK_3,			XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_eacute,	XK_2,			XK_at,		NoSymbol,	/* 0x3d */
	XK_ampersand,	XK_1,			XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_dollar,	XK_sterling,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_exclam,	XK_8,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_ccedilla,	XK_9,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_agrave,	XK_0,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_parenright,	XK_degree,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_minus,	XK_underscore,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_mute_asciicircum,XK_diaeresis,	XK_degree,	NoSymbol,	/* 0x63 */
	XK_quoteleft,   XK_asterisk,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_less,	XK_greater,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special, might also be Insert */		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_M,		NoSymbol,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_ugrave,	XK_percent,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_comma,	XK_question,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_semicolon,	XK_period,		XK_less,	NoSymbol,	/* 0x71 */
	XK_colon,	XK_slash,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_equal,	XK_plus,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space the final frontier..." */	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	

static KeySym Swiss_German2Map[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Y,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Z,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_ccedilla,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_asterisk,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_plus,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_section,		XK_degree,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_mute_acute,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_mute_asciicircum,	XK_mute_grave,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_udiaeresis,		XK_egrave,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_mute_diaeresis,	XK_exclam,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_dollar,		XK_sterling,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_odiaeresis,		XK_eacute,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_adiaeresis,		XK_agrave,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	

static KeySym SpainMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_dollar, 		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_questiondown,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_less,		XK_greater,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_quoteright,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_quoteleft,		XK_exclamdown,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_at,			XK_numbersign,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_plus,		XK_asterisk,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_ccedilla,		XK_degree,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_ntilde,		XK_Ntilde,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_mute_acute,		XK_mute_diaeresis,	XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym Swiss_French2Map[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Y,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Z,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_ccedilla,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_asterisk,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_plus,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_section,		XK_degree,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_mute_acute,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_mute_asciicircum,	XK_mute_grave,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_egrave,		XK_udiaeresis,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_mute_diaeresis,	XK_exclam,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_dollar,		XK_sterling,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_eacute,		XK_odiaeresis,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_agrave,		XK_adiaeresis,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
	
static KeySym GermanMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Y,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Z,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_section,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_less,		XK_greater,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_ssharp,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_quoteleft,		XK_quoteright,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_udiaeresis,		XK_Udiaeresis,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_plus,		XK_asterisk,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_sterling,		XK_asciicircum,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_odiaeresis,		XK_Odiaeresis,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_adiaeresis,		XK_Adiaeresis,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym SwedenMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_numbersign,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_less,		XK_greater,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_plus,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_eacute,		XK_Eacute,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_aring,		XK_Aring,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_udiaeresis,		XK_Udiaeresis,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_quoteright,		XK_asterisk,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_odiaeresis,		XK_Odiaeresis,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_adiaeresis,		XK_Adiaeresis,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym HollandMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,		XK_underscore,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,		XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,		XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,		XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,		XK_numbersign,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,		XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,		XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_at,		XK_section,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,		XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,		XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,		XK_quoteright,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_slash,	XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_bar,		XK_backslash,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_mute_diaeresis,XK_mute_asciicircum,	XK_degree,	NoSymbol,	/* 0x63 */
	XK_less,	XK_greater,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_ccedilla,	XK_guilder,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special, might also be Insert */		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_colon,	XK_plus,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_mute_acute,	XK_mute_grave,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,		NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,	XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,	XK_asterisk,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,	XK_equal,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym ItalyMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_W,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_Z,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_egrave,		XK_7,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_underscore,		XK_6,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_parenleft,		XK_5,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_quoteright,		XK_4,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_quotedbl,		XK_3,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_eacute,		XK_2,		XK_at,		NoSymbol,	/* 0x3d */
	XK_sterling,		XK_1,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_less,		XK_greater,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_mute_asciicircum,	XK_8,			XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_ccedilla,		XK_9,			XK_bracketright,XK_braceright,	/* 0x59 */
	XK_agrave,		XK_0,			XK_questiondown,NoSymbol,	/* 0x5a */
	XK_parenright,		XK_degree,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_minus,		XK_plus,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_igrave,		XK_equal,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_dollar,		XK_ampersand,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_asterisk,		XK_section,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_M,			NoSymbol,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_ugrave,		XK_percent,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_quoteright,		XK_question,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_semicolon,		XK_period,		XK_less,	NoSymbol,	/* 0x71 */
	XK_colon,		XK_slash,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_ograve,		XK_exclam,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym Canada_EnglishMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,   NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,	XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,		NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,		NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,		NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,		NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,	XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,	XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,		NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,		NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,		NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,		NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,		NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,		NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,		NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,		NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,		NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,		NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,		NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,		NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,		XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,		XK_ampersand,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,		XK_question,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,		XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,		XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,		XK_slash,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,		XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,		XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_bracketright,XK_bracketleft,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,	XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,	XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,		XK_asterisk,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,		XK_parenleft,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,		XK_parenright,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_minus,	XK_underscore,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_equal,	XK_plus,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,		NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,		NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,		NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_mute_asciicircum,XK_mute_diaeresis,	XK_degree,	NoSymbol,	/* 0x63 */
	XK_ccedilla,	XK_Ccedilla,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_at,		XK_numbersign,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special, might also be Insert */		
	XK_InsertChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,		NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,		NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,		NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_semicolon,	XK_colon,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_mute_grave,	XK_quoteright,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,		NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,	XK_less,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,	XK_greater,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_eacute,	XK_Eacute,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,		NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,	NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym Swiss_FrenchMap[] = {	
	
 /* code values in comments at line end are actual value reported on HIL.	
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */	
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Y,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Z,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_slash,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_ampersand,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_ccedilla,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_asterisk,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_quotedbl,		XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_plus,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_section,		XK_degree,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_parenleft,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenright,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_equal,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_exclam,		XK_question,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_mute_asciicircum,	XK_mute_grave,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_egrave,		XK_udiaeresis,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_mute_diaeresis,	XK_mute_acute,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_dollar,		XK_sterling,		NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_eacute,		XK_odiaeresis,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_agrave,		XK_adiaeresis,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_semicolon,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_colon,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_minus,		XK_underscore,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Kanji,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};	
	
static KeySym AsianMap[] = {

 /* code values in comments at line end are actual value reported on HIL.
    REMEMBER, there is an offset of MIN_KEYCODE applied to this table! */
	/* Extend Char Right -- a.k.a. Kanji? */	
	XK_Meta_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x02 */
	XK_Meta_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x03 */
	XK_Shift_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4 */
	XK_Shift_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5 */
	XK_Control_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6 */
	XK_Break,		XK_Reset,		NoSymbol,	NoSymbol,	/* 0x7 */
	XK_KP_4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x8 */
	XK_KP_8,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x9 */
	XK_KP_5,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xa */
	XK_KP_9,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xb */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xc */
	XK_KP_7,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xd */
	XK_KP_Separator,	NoSymbol,		NoSymbol,	NoSymbol,	/* 0xe */
	XK_KP_Enter,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0xf */
	XK_KP_1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x10 */
	XK_KP_Divide,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x11 */
	XK_KP_2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x12 */
	XK_KP_Add,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x13 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x14 */
	XK_KP_Multiply,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x15 */
	XK_KP_0,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x16 */
	XK_KP_Subtract,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x17 */
	XK_B,			NoSymbol,		XK_block,	NoSymbol,	/* 0x18 */
	XK_V,			NoSymbol,		XK_section,	NoSymbol,	/* 0x19 */
	XK_C,			NoSymbol,		XK_ccedilla,	XK_Ccedilla,	/* 0x1a */
	XK_X,			NoSymbol,		XK_scaron,	XK_Scaron,	/* 0x1b */
	XK_Z,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1c */
/* Was Kanji Left.... */	
	XK_Ext16bit_L,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x1e */
	XK_Escape,		XK_Delete,		NoSymbol,	NoSymbol,	/* 0x1f */
	XK_KP_6,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x20 */
	XK_KP_F2,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x21 */
	XK_KP_3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x22 */
	XK_KP_F3,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x23 */
	XK_KP_Decimal,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x24 */
	XK_KP_F1,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x25 */
	XK_KP_Tab,		XK_KP_BackTab,		NoSymbol,	NoSymbol,	/* 0x26 */
	XK_KP_F4,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x27 */
	XK_H,			NoSymbol,		XK_yen,		NoSymbol,	/* 0x28 */
	XK_G,			NoSymbol,		XK_currency,	NoSymbol,	/* 0x29 */
	XK_F,			NoSymbol,		XK_guilder,	NoSymbol,	/* 0x2a */
	XK_D,			NoSymbol,		XK_eth,		XK_Eth,		/* 0x2b */
	XK_S,			NoSymbol,		XK_ssharp,	NoSymbol,	/* 0x2c */
	XK_A,			NoSymbol,		XK_aring,	XK_Aring,	/* 0x2d */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2e */
	XK_Caps_Lock,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x2f */
	XK_U,			NoSymbol,		XK_mute_diaeresis,NoSymbol,	/* 0x30 */
	XK_Y,			NoSymbol,		XK_mute_asciicircum,NoSymbol,	/* 0x31 */
	XK_T,			NoSymbol,		XK_mute_grave,	NoSymbol,	/* 0x32 */
	XK_R,			NoSymbol,		XK_mute_acute,	NoSymbol,	/* 0x33 */
	XK_E,			NoSymbol,		XK_ae,		XK_AE,		/* 0x34 */
	XK_W,			NoSymbol,		XK_asciitilde,	NoSymbol,	/* 0x35 */
	XK_Q,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x36 */
	XK_Tab,			XK_BackTab,		NoSymbol,	NoSymbol,	/* 0x37 */
	XK_7,			XK_ampersand,		XK_backslash,	NoSymbol,	/* 0x38 */
	XK_6,			XK_asciicircum,		XK_asciicircum,	NoSymbol,	/* 0x39 */
	XK_5,			XK_percent,		XK_onehalf,	NoSymbol,	/* 0x3a */
	XK_4,			XK_dollar,		XK_onequarter,	NoSymbol,	/* 0x3b */
	XK_3,			XK_numbersign,		XK_numbersign,	NoSymbol,	/* 0x3c */
	XK_2,			XK_at,			XK_at,		NoSymbol,	/* 0x3d */
	XK_1,			XK_exclam,		XK_exclamdown,	NoSymbol,	/* 0x3e */
	XK_quoteleft,		XK_asciitilde,		XK_guillemotleft,XK_guillemotright,/* 0x3f */
/* Was Mouse-L */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x40 */
/* Was Mouse-M */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x41 */
/* Was Mouse-R */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x42 */
/* Was 4 button puck */	
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x43 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x44 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x45 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x46 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x47 */
	XK_Menu,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x48 */
	XK_F4,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x49 */
	XK_F3,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4a */
	XK_F2,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4b */
	XK_F1,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4c */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4d */
/* Was 'Stop' */	
	XK_Cancel,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x4e */
/* Was 'Enter' */	
	XK_Execute,		XK_Print,		NoSymbol,	NoSymbol,	/* 0x4f */
	XK_System,		XK_User,		NoSymbol,	NoSymbol,	/* 0x50 */
	XK_F5,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x51 */
	XK_F6,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x52 */
	XK_F7,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x53 */
	XK_F8,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x54 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x55 */
	XK_ClearLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x56 */
/* Was 'Clear Display' */	
	XK_Clear,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x57 */
	XK_8,			XK_asterisk,		XK_bracketleft,	XK_braceleft,	/* 0x58 */
	XK_9,			XK_parenleft,		XK_bracketright,XK_braceright,	/* 0x59 */
	XK_0,			XK_parenright,		XK_questiondown,NoSymbol,	/* 0x5a */
	XK_minus,		XK_underscore,		XK_longminus,	XK_macron,	/* 0x5b */
	XK_equal,		XK_plus,		XK_plusminus,	NoSymbol,	/* 0x5c */
	XK_BackSpace,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5d */
	XK_InsertLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5e */
	XK_DeleteLine,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x5f */
			
	XK_I,			NoSymbol,		XK_mute_asciitilde,NoSymbol,	/* 0x60 */
	XK_O,			NoSymbol,		XK_oslash,	XK_Ooblique,	/* 0x61 */
	XK_P,			NoSymbol,		XK_thorn,	XK_Thorn,	/* 0x62 */
	XK_bracketleft,		XK_braceleft,		XK_degree,	NoSymbol,	/* 0x63 */
	XK_bracketright,	XK_braceright,		XK_brokenbar,	NoSymbol,	/* 0x64 */
	XK_backslash,		XK_bar,			NoSymbol,	NoSymbol,	/* 0x65 */
	
	/* HP special  might also be Insert */		
	XK_InsertChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x66 */
	XK_DeleteChar,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x67 */
	XK_J,			NoSymbol,		XK_dollar,	NoSymbol,	/* 0x68 */
	XK_K,			NoSymbol,		XK_cent,	NoSymbol,	/* 0x69 */
	XK_L,			NoSymbol,		XK_sterling,	NoSymbol,	/* 0x6a */
	XK_semicolon,		XK_colon,		XK_lira,	NoSymbol,	/* 0x6b */
	XK_quoteright,		XK_quotedbl,		XK_quoteleft,	XK_quoteright,	/* 0x6c */
	XK_Return,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6d */
	XK_Home,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6e */
	/* Prev */	
	XK_Prior,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x6f */
	
	XK_M,			NoSymbol,		XK_masculine,	NoSymbol,	/* 0x70 */
	XK_comma,		XK_less,		XK_less,	NoSymbol,	/* 0x71 */
	XK_period,		XK_greater,		XK_greater,	NoSymbol,	/* 0x72 */
	XK_slash,		XK_question,		XK_underscore,	NoSymbol,	/* 0x73 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x74 */
	XK_Select,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x75 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x76 */
	XK_Next,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x77 */
	XK_N,			NoSymbol,		XK_ordfeminine,	NoSymbol,	/* 0x78 */
	/* "Space  the final frontier..." */	
	XK_space,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x79 */
	NoSymbol,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7a */
	/* Kanji Right */	
	XK_Ext16bit_R,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7b */
	
	XK_Left,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7c */
	XK_Down,		NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7d */
	XK_Up,			NoSymbol,		NoSymbol,	NoSymbol,	/* 0x7e */
	XK_Right,		NoSymbol,		NoSymbol,	NoSymbol	/* 0x7f */
};
	

KeySymsRec hpKeySyms[] = {
    /*	map	           minKeyCode              maxKC       width */
    USASCIIMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    LatinMap,         (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    KatakanaMap,      (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    DenmarkMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    FranceMap,        (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    NorwayMap,        (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    Swiss_GermanMap,  (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    Canada_FrenchMap, (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    UKMap,            (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    FinishMap,        (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    BelgianMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    Swiss_German2Map, (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    SpainMap,         (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    Swiss_French2Map, (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*Chinese trad */AsianMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*Chinese simp */AsianMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    GermanMap,        (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    SwedenMap,        (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    HollandMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*Korean */ AsianMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    ItalyMap,         (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*undefined*/ USASCIIMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*undefined*/ USASCIIMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*undefined*/ USASCIIMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    Canada_EnglishMap,(MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*undefined*/ USASCIIMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*undefined*/ USASCIIMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*undefined*/ USASCIIMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
    Swiss_FrenchMap,  (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
/*Japenese*/ KatakanaMap,       (MIN_KEYCODE + 0x02), (MIN_KEYCODE + 0x80), 4,
};

#define	cT	(ControlMask)
#define	sH	(ShiftMask)
#define	lK	(LockMask)
#define	mT	(Mod1Mask)

static CARD8 type0modmap[MAP_LENGTH] = {
/* shift table values up by 8. This offset is necessary to reserve codes
   for mouse buttons. Note last 8 entries of table are commented out to
   preserve length of table.
   Note: '#define MIN_KEYCODE 8' in ddx/hp/sun.h */
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  mT, mT, sH, sH, cT, 0,  0,  0,  0,  0,  0,  0,  0,  0, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  lK,/* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 30-3f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 40-4f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 50-5f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 60-6f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 70-7f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 80-8f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 90-9f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* a0-af */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* b0-bf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* c0-cf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* d0-df */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* e0-ef */
    0,  0,  0,  0,  0,  0,  0,  0,/*0,  0,  0,  0,  0,  0,  0,  0, /* f0-ff */
};

CARD8 *hpMapRec[] = {
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
      type0modmap,
};
