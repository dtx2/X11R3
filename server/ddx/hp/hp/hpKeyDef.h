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
 * TTY Functions unique to HP. Augments definitions in
 * server/include/keysymdef.h.  Section 6 of the protocol
 * document states that bit 29 should be set for vendor-
 * specific keysyms.
 */

#define XK_Reset                0x1000FF6C  /* HP -- The shift of Break */
#define XK_System               0x1000FF6D  /* HP */
#define XK_User                 0x1000FF6E  /* HP */
#define XK_ClearLine            0x1000FF6F  /* HP */
#define XK_InsertLine           0x1000FF70  /* HP */
#define XK_DeleteLine           0x1000FF71  /* HP */
#define XK_InsertChar           0x1000FF72  /* HP */
#define XK_DeleteChar           0x1000FF73  /* HP */
#define XK_BackTab              0x1000FF74  /* HP */
#define XK_KP_BackTab           0x1000FF75  /* HP */
#define XK_Ext16bit_L		0x1000FF76  /* Kanji Left  */
#define XK_Ext16bit_R		0x1000FF77  /* Kanji Right */
#define XK_Muhenkan		0x1000FF22  /* HP */
#define XK_Henkan		0x1000FF23  /* HP */

/* 
 * HP Roman8 characters that do NOT have equivalents in 
 * the ISO Latin 1-4 character sets.  The values for the
 * least signigicant byte were derived from the values in
 * the Roman8 character set.
 */

#define XK_mute_acute		0x100000a8
#define XK_mute_grave		0x100000a9
#define XK_mute_asciicircum	0x100000aa
#define XK_mute_diaeresis	0x100000ab
#define XK_mute_asciitilde	0x100000ac

#define XK_lira 		0x100000af
#define XK_guilder             	0x100000be
#define XK_Ydiaeresis   	0x100000ee
#define XK_IO		   	0x100000ee
#define XK_longminus		0x100000f6
#define XK_block   		0x100000fc
