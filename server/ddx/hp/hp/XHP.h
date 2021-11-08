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
 *  file: XHP.h
 *
 *
 *  ******************************************************************
 *  *  (c) Copyright Hewlett-Packard Company, 1984.  All rights are  *
 *  *  reserved.  Copying or other reproduction of this program      *
 *  *  except for archival purposes is prohibited without prior      *
 *  *  written consent of Hewlett-Packard Company.		     *
 *  ******************************************************************
 *
 *  Program purpose -- include file for all XHP files
 *
 *		Hewlett Packard -- Corvallis Workstation Operation
 *		Project -- port of X to HP9000S300
 *		Dan Garfinkel -- MTS
 *
 * $Log:	XHP.h,v $
 * Revision 1.2  88/08/17  10:49:03  jim
 * new version from hp
 * 
 * Revision 1.2  87/12/21  16:30:55  16:30:55  harry (Harry Phinney)
 * Merging with MIT sources
 * 
 * 
 * Revision 1.1  87/12/16  13:26:47  hp
 * Initial revision
 * 
 * Revision 1.1  87/12/14  14:41:22  hp
 * Initial revision
 * 
 * Revision 1.1  87/09/22 18:04:00 GMT  leichner
 * Initial revision
 * 
 * Revision 3.0  87/08/12  16:14:40  16:14:40  bennett
 * beta 3.0
 * 
 * Revision 1.1  87/07/02  10:08:26  bennett
 * Initial revision
 * 
 * Revision 1.1  87/06/01  10:16:08  10:16:08  palevich (John H. Palevich)
 * Initial revision
 * 
 * Revision 3.0  86/11/06  09:27:04  09:27:04  root ()
 * QA1 release (plus some fixes)
 * 
 * Revision 2.0  86/10/10  15:21:01  15:21:01  dan ()
 * Alpha Release
 * 
 * Revision 1.4  86/09/22  10:24:53  10:24:53  dan ()
 * Change the default font path to /usr/lib/Xfont.
 * 
 * Revision 1.1  86/04/07  10:50:52  10:50:52  dan ()
 * Initial revision
 * 
 */

#include <sys/types.h>
/**
#include <X/X.h>
#include <X/vsinput.h>
#include <X/Xdev.h>
***/
#include "topcat.h"

/*
 *  For V11 only
 */
#include   "x11_hildef.h"
/*
 */
#define DEFAULT_FONT_PATH	"/usr/lib/Xfont"
#define DEFAULT_FONT_SUFFIX	".onx"

#define INFB	0x10
#define SOLID	0x20

#ifndef INIT
extern u_char * XHP_base;
extern int XHP_width;
extern int XHP_height;
extern int XHP_bits;
extern int XHP_colors;
extern u_char XHP_NewRule [16][6];
extern vsCursor XHP_mpos;
extern TOPCAT * gp_hardware;
#endif

/*
 * Type definitions
 */

typedef struct {
    short state;		/* USED, FREE, UNUSED, or LAST */
    short x, y;			/* x, y position in the frame buffer */
    short h, w;			/* height and width of chunk */
    short p;			/* node this chunk belongs to */
} XHP_CHUNK;

typedef struct {
    u_short * index;		/* location of the glifs in the data */
    BITMAP  * bit;		/* bitmap containing the glifs */
    XHP_CHUNK * fb[3];		/* glifs chunks in frame buffer */
} XHP_FINFO;

/*
 * Macros 
 */


#define DispAddr(w,h)	(XHP_base + (((w) + XHP_width*(h)) << XHP_bits))
