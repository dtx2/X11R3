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
/*******************************************************************************
*
* File:         init.c
* RCS:          $Revision: 1.6 $
* Description:  Multiple screen initialization
* Author:       John Howard Palevich
* Created:      April 22, 1987
* Modified:     April 29, 1987  17:41:59 (John Howard Palevich)
* Language:     C
* Package:      USER
* Status:       Experimental (Do Not Distribute)
*
* (c) Copyright 1987, Hewlett-Packard, Inc., all rights reserved.
*
*******************************************************************************/

#include "X.h"
#include "Xproto.h"
#include <servermd.h>
#include "screenint.h"
#include "input.h"
#include "cursor.h"
#include "misc.h"
#include "scrnintstr.h"

#include "screentab.h"
#include "gcstruct.h"
#include "sun.h"
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include "cfb/cfb.h" /* XXX should this really be here? */
/* #include "hpversion.h"  /* defines the hpversion macro for "what" string */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "foo.h"	/* never at a loss for descriptive names */
#include <fcntl.h>
/* hpversion		/* produces the appropriate "what" string */

#define MAXSTRING 120
#define MAXARG 10

extern  char    *display;		/* display number as a string */
extern int	ErrorfOn;
static char xscreens[80];
static int	ConfigFile=FALSE;
static char	DefaultScreen[]="/dev/crt";
/*
 * NewRule is an array of replacement rules.  Given a replacement rule,
 *
 *   colunm	foreground pixel	background pixel
 *	0		0			0
 *	1		0			1
 *	2		1			0
 *	3		1			1
 *	4		1		      clear
 *	5		0		      clear
 */

u_char XHP_NewRule [16][6] = 
{
GXclear, GXclear,	GXclear,       GXclear, GXandInverted,GXandInverted,
GXclear, GXandInverted, GXand,	       GXnoop,  GXnoop,	      GXandInverted,
GXclear, GXnor,	        GXandReverse,  GXinvert,GXxor,	      GXandInverted,
GXclear, GXcopyInverted,GXcopy,	       GXset,   GXor,	      GXandInverted,
GXnoop,  GXand,		GXandInverted, GXclear, GXandInverted,GXnoop,
GXnoop,  GXnoop,	GXnoop,	       GXnoop,  GXnoop,	      GXnoop,
GXnoop,  GXequiv,	GXxor,	       GXinvert,GXxor,	      GXnoop,
GXnoop,  GXorInverted,	GXor,	       GXset,   GXor,	      GXnoop,
GXinvert,GXandReverse,  GXnor,	       GXclear, GXandInverted,GXxor,
GXinvert,GXxor,		GXequiv,       GXnoop,  GXnoop,	      GXxor,
GXinvert,GXinvert,	GXinvert,      GXinvert,GXxor,	      GXxor,
GXinvert,GXnand,	GXorReverse,   GXset,   GXor,	      GXxor,
GXset, 	 GXcopy,	GXcopyInverted,GXclear, GXandInverted,GXor,
GXset, 	 GXor,		GXorInverted,  GXnoop,  GXnoop,	      GXor,
GXset, 	 GXorReverse,	GXnand,	       GXinvert,GXxor,	      GXor,
GXset, 	 GXset,		GXset,	       GXset,	GXor,	      GXor
};

int XHP_QUADALIGN;

#define IOMAP_BASE 0xb00000
#define STUPID_MOBERLY -1
static unsigned char *iomapBase;
int Rtprio = 0;
int TopcatBrainDamage = 0;
#ifdef MULTI_X_HACK
int XMulti = 0;
#endif 


MultiScreenRec multiScreenTable[MAXSCREENS];
fbFd 	  sunFbs[MAXSCREENS];

static PixmapFormatRec	formats[] = {
    1, 1, BITMAP_SCANLINE_PAD,	/* 1-bit deep for all */
    4, 4, BITMAP_SCANLINE_PAD,  /* 4-bit deep for Burgundy */
    8, 8, BITMAP_SCANLINE_PAD,	/* 8-bit deep for most color displays */
   16,16, BITMAP_SCANLINE_PAD,	/*16-bit deep for most color displays */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])


/* Pmap represents all 256 combinations of 8 bits of information. */
/* For example, if my byte of 8 pixels of information is 01010101 */
/* then the bytes that get written in memory are 00ff00ff,00ff00ff*/
/* The bytes are written using the current write enable and       */
/* drawmode value.                                                */

   int XHP_pmap[256][2]={0x00000000,0x00000000,
                         0x00000000,0x000000ff,
                         0x00000000,0x0000ff00,
                         0x00000000,0x0000ffff,
                         0x00000000,0x00ff0000,
                         0x00000000,0x00ff00ff,
                         0x00000000,0x00ffff00,
                         0x00000000,0x00ffffff,
                         0x00000000,0xff000000,
                         0x00000000,0xff0000ff,
                         0x00000000,0xff00ff00,
                         0x00000000,0xff00ffff,
                         0x00000000,0xffff0000,
                         0x00000000,0xffff00ff,
                         0x00000000,0xffffff00,
                         0x00000000,0xffffffff,
                         0x000000ff,0x00000000,
                         0x000000ff,0x000000ff,
                         0x000000ff,0x0000ff00,
                         0x000000ff,0x0000ffff,
                         0x000000ff,0x00ff0000,
                         0x000000ff,0x00ff00ff,
                         0x000000ff,0x00ffff00,
                         0x000000ff,0x00ffffff,
                         0x000000ff,0xff000000,
                         0x000000ff,0xff0000ff,
                         0x000000ff,0xff00ff00,
                         0x000000ff,0xff00ffff,
                         0x000000ff,0xffff0000,
                         0x000000ff,0xffff00ff,
                         0x000000ff,0xffffff00,
                         0x000000ff,0xffffffff,
                         0x0000ff00,0x00000000,
                         0x0000ff00,0x000000ff,
                         0x0000ff00,0x0000ff00,
                         0x0000ff00,0x0000ffff,
                         0x0000ff00,0x00ff0000,
                         0x0000ff00,0x00ff00ff,
                         0x0000ff00,0x00ffff00,
                         0x0000ff00,0x00ffffff,
                         0x0000ff00,0xff000000,
                         0x0000ff00,0xff0000ff,
                         0x0000ff00,0xff00ff00,
                         0x0000ff00,0xff00ffff,
                         0x0000ff00,0xffff0000,
                         0x0000ff00,0xffff00ff,
                         0x0000ff00,0xffffff00,
                         0x0000ff00,0xffffffff,
                         0x0000ffff,0x00000000,
                         0x0000ffff,0x000000ff,
                         0x0000ffff,0x0000ff00,
                         0x0000ffff,0x0000ffff,
                         0x0000ffff,0x00ff0000,
                         0x0000ffff,0x00ff00ff,
                         0x0000ffff,0x00ffff00,
                         0x0000ffff,0x00ffffff,
                         0x0000ffff,0xff000000,
                         0x0000ffff,0xff0000ff,
                         0x0000ffff,0xff00ff00,
                         0x0000ffff,0xff00ffff,
                         0x0000ffff,0xffff0000,
                         0x0000ffff,0xffff00ff,
                         0x0000ffff,0xffffff00,
                         0x0000ffff,0xffffffff,
                         0x00ff0000,0x00000000,
                         0x00ff0000,0x000000ff,
                         0x00ff0000,0x0000ff00,
                         0x00ff0000,0x0000ffff,
                         0x00ff0000,0x00ff0000,
                         0x00ff0000,0x00ff00ff,
                         0x00ff0000,0x00ffff00,
                         0x00ff0000,0x00ffffff,
                         0x00ff0000,0xff000000,
                         0x00ff0000,0xff0000ff,
                         0x00ff0000,0xff00ff00,
                         0x00ff0000,0xff00ffff,
                         0x00ff0000,0xffff0000,
                         0x00ff0000,0xffff00ff,
                         0x00ff0000,0xffffff00,
                         0x00ff0000,0xffffffff,
                         0x00ff00ff,0x00000000,
                         0x00ff00ff,0x000000ff,
                         0x00ff00ff,0x0000ff00,
                         0x00ff00ff,0x0000ffff,
                         0x00ff00ff,0x00ff0000,
                         0x00ff00ff,0x00ff00ff,
                         0x00ff00ff,0x00ffff00,
                         0x00ff00ff,0x00ffffff,
                         0x00ff00ff,0xff000000,
                         0x00ff00ff,0xff0000ff,
                         0x00ff00ff,0xff00ff00,
                         0x00ff00ff,0xff00ffff,
                         0x00ff00ff,0xffff0000,
                         0x00ff00ff,0xffff00ff,
                         0x00ff00ff,0xffffff00,
                         0x00ff00ff,0xffffffff,
                         0x00ffff00,0x00000000,
                         0x00ffff00,0x000000ff,
                         0x00ffff00,0x0000ff00,
                         0x00ffff00,0x0000ffff,
                         0x00ffff00,0x00ff0000,
                         0x00ffff00,0x00ff00ff,
                         0x00ffff00,0x00ffff00,
                         0x00ffff00,0x00ffffff,
                         0x00ffff00,0xff000000,
                         0x00ffff00,0xff0000ff,
                         0x00ffff00,0xff00ff00,
                         0x00ffff00,0xff00ffff,
                         0x00ffff00,0xffff0000,
                         0x00ffff00,0xffff00ff,
                         0x00ffff00,0xffffff00,
                         0x00ffff00,0xffffffff,
                         0x00ffffff,0x00000000,
                         0x00ffffff,0x000000ff,
                         0x00ffffff,0x0000ff00,
                         0x00ffffff,0x0000ffff,
                         0x00ffffff,0x00ff0000,
                         0x00ffffff,0x00ff00ff,
                         0x00ffffff,0x00ffff00,
                         0x00ffffff,0x00ffffff,
                         0x00ffffff,0xff000000,
                         0x00ffffff,0xff0000ff,
                         0x00ffffff,0xff00ff00,
                         0x00ffffff,0xff00ffff,
                         0x00ffffff,0xffff0000,
                         0x00ffffff,0xffff00ff,
                         0x00ffffff,0xffffff00,
                         0x00ffffff,0xffffffff,
                         0xff000000,0x00000000,
                         0xff000000,0x000000ff,
                         0xff000000,0x0000ff00,
                         0xff000000,0x0000ffff,
                         0xff000000,0x00ff0000,
                         0xff000000,0x00ff00ff,
                         0xff000000,0x00ffff00,
                         0xff000000,0x00ffffff,
                         0xff000000,0xff000000,
                         0xff000000,0xff0000ff,
                         0xff000000,0xff00ff00,
                         0xff000000,0xff00ffff,
                         0xff000000,0xffff0000,
                         0xff000000,0xffff00ff,
                         0xff000000,0xffffff00,
                         0xff000000,0xffffffff,
                         0xff0000ff,0x00000000,
                         0xff0000ff,0x000000ff,
                         0xff0000ff,0x0000ff00,
                         0xff0000ff,0x0000ffff,
                         0xff0000ff,0x00ff0000,
                         0xff0000ff,0x00ff00ff,
                         0xff0000ff,0x00ffff00,
                         0xff0000ff,0x00ffffff,
                         0xff0000ff,0xff000000,
                         0xff0000ff,0xff0000ff,
                         0xff0000ff,0xff00ff00,
                         0xff0000ff,0xff00ffff,
                         0xff0000ff,0xffff0000,
                         0xff0000ff,0xffff00ff,
                         0xff0000ff,0xffffff00,
                         0xff0000ff,0xffffffff,
                         0xff00ff00,0x00000000,
                         0xff00ff00,0x000000ff,
                         0xff00ff00,0x0000ff00,
                         0xff00ff00,0x0000ffff,
                         0xff00ff00,0x00ff0000,
                         0xff00ff00,0x00ff00ff,
                         0xff00ff00,0x00ffff00,
                         0xff00ff00,0x00ffffff,
                         0xff00ff00,0xff000000,
                         0xff00ff00,0xff0000ff,
                         0xff00ff00,0xff00ff00,
                         0xff00ff00,0xff00ffff,
                         0xff00ff00,0xffff0000,
                         0xff00ff00,0xffff00ff,
                         0xff00ff00,0xffffff00,
                         0xff00ff00,0xffffffff,
                         0xff00ffff,0x00000000,
                         0xff00ffff,0x000000ff,
                         0xff00ffff,0x0000ff00,
                         0xff00ffff,0x0000ffff,
                         0xff00ffff,0x00ff0000,
                         0xff00ffff,0x00ff00ff,
                         0xff00ffff,0x00ffff00,
                         0xff00ffff,0x00ffffff,
                         0xff00ffff,0xff000000,
                         0xff00ffff,0xff0000ff,
                         0xff00ffff,0xff00ff00,
                         0xff00ffff,0xff00ffff,
                         0xff00ffff,0xffff0000,
                         0xff00ffff,0xffff00ff,
                         0xff00ffff,0xffffff00,
                         0xff00ffff,0xffffffff,
                         0xffff0000,0x00000000,
                         0xffff0000,0x000000ff,
                         0xffff0000,0x0000ff00,
                         0xffff0000,0x0000ffff,
                         0xffff0000,0x00ff0000,
                         0xffff0000,0x00ff00ff,
                         0xffff0000,0x00ffff00,
                         0xffff0000,0x00ffffff,
                         0xffff0000,0xff000000,
                         0xffff0000,0xff0000ff,
                         0xffff0000,0xff00ff00,
                         0xffff0000,0xff00ffff,
                         0xffff0000,0xffff0000,
                         0xffff0000,0xffff00ff,
                         0xffff0000,0xffffff00,
                         0xffff0000,0xffffffff,
                         0xffff00ff,0x00000000,
                         0xffff00ff,0x000000ff,
                         0xffff00ff,0x0000ff00,
                         0xffff00ff,0x0000ffff,
                         0xffff00ff,0x00ff0000,
                         0xffff00ff,0x00ff00ff,
                         0xffff00ff,0x00ffff00,
                         0xffff00ff,0x00ffffff,
                         0xffff00ff,0xff000000,
                         0xffff00ff,0xff0000ff,
                         0xffff00ff,0xff00ff00,
                         0xffff00ff,0xff00ffff,
                         0xffff00ff,0xffff0000,
                         0xffff00ff,0xffff00ff,
                         0xffff00ff,0xffffff00,
                         0xffff00ff,0xffffffff,
                         0xffffff00,0x00000000,
                         0xffffff00,0x000000ff,
                         0xffffff00,0x0000ff00,
                         0xffffff00,0x0000ffff,
                         0xffffff00,0x00ff0000,
                         0xffffff00,0x00ff00ff,
                         0xffffff00,0x00ffff00,
                         0xffffff00,0x00ffffff,
                         0xffffff00,0xff000000,
                         0xffffff00,0xff0000ff,
                         0xffffff00,0xff00ff00,
                         0xffffff00,0xff00ffff,
                         0xffffff00,0xffff0000,
                         0xffffff00,0xffff00ff,
                         0xffffff00,0xffffff00,
                         0xffffff00,0xffffffff,
                         0xffffffff,0x00000000,
                         0xffffffff,0x000000ff,
                         0xffffffff,0x0000ff00,
                         0xffffffff,0x0000ffff,
                         0xffffffff,0x00ff0000,
                         0xffffffff,0x00ff00ff,
                         0xffffffff,0x00ffff00,
                         0xffffffff,0x00ffffff,
                         0xffffffff,0xff000000,
                         0xffffffff,0xff0000ff,
                         0xffffffff,0xff00ff00,
                         0xffffffff,0xff00ffff,
                         0xffffffff,0xffff0000,
                         0xffffffff,0xffff00ff,
                         0xffffffff,0xffffff00,
                         0xffffffff,0xffffffff}; /*XHP_pmap end*/

/* Wordmap represents all 256 combinations of 8 bits of information. */
/* For example, if my byte of 8 pixels of information is 01010101 */
/* then the bytes that get written in memory are 00ff00ff,00ff00ff*/
/* The bytes are written using the current write enable and       */
/* drawmode value.  Note that this is the same as pmap except that */
/* wordmap is for the MRC topcat which writes a word per pixel */

   int XHP_wordmap[256][4]={
		 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		 0x00000000, 0x00000000, 0x00000000, 0x0000ffff,
		 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
		 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
		 0x00000000, 0x00000000, 0x0000ffff, 0x00000000,
		 0x00000000, 0x00000000, 0x0000ffff, 0x0000ffff,
		 0x00000000, 0x00000000, 0x0000ffff, 0xffff0000,
		 0x00000000, 0x00000000, 0x0000ffff, 0xffffffff,
		 0x00000000, 0x00000000, 0xffff0000, 0x00000000,
		 0x00000000, 0x00000000, 0xffff0000, 0x0000ffff,
		 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
		 0x00000000, 0x00000000, 0xffff0000, 0xffffffff,
		 0x00000000, 0x00000000, 0xffffffff, 0x00000000,
		 0x00000000, 0x00000000, 0xffffffff, 0x0000ffff,
		 0x00000000, 0x00000000, 0xffffffff, 0xffff0000,
		 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
		 0x00000000, 0x0000ffff, 0x00000000, 0x00000000,
		 0x00000000, 0x0000ffff, 0x00000000, 0x0000ffff,
		 0x00000000, 0x0000ffff, 0x00000000, 0xffff0000,
		 0x00000000, 0x0000ffff, 0x00000000, 0xffffffff,
		 0x00000000, 0x0000ffff, 0x0000ffff, 0x00000000,
		 0x00000000, 0x0000ffff, 0x0000ffff, 0x0000ffff,
		 0x00000000, 0x0000ffff, 0x0000ffff, 0xffff0000,
		 0x00000000, 0x0000ffff, 0x0000ffff, 0xffffffff,
		 0x00000000, 0x0000ffff, 0xffff0000, 0x00000000,
		 0x00000000, 0x0000ffff, 0xffff0000, 0x0000ffff,
		 0x00000000, 0x0000ffff, 0xffff0000, 0xffff0000,
		 0x00000000, 0x0000ffff, 0xffff0000, 0xffffffff,
		 0x00000000, 0x0000ffff, 0xffffffff, 0x00000000,
		 0x00000000, 0x0000ffff, 0xffffffff, 0x0000ffff,
		 0x00000000, 0x0000ffff, 0xffffffff, 0xffff0000,
		 0x00000000, 0x0000ffff, 0xffffffff, 0xffffffff,
		 0x00000000, 0xffff0000, 0x00000000, 0x00000000,
		 0x00000000, 0xffff0000, 0x00000000, 0x0000ffff,
		 0x00000000, 0xffff0000, 0x00000000, 0xffff0000,
		 0x00000000, 0xffff0000, 0x00000000, 0xffffffff,
		 0x00000000, 0xffff0000, 0x0000ffff, 0x00000000,
		 0x00000000, 0xffff0000, 0x0000ffff, 0x0000ffff,
		 0x00000000, 0xffff0000, 0x0000ffff, 0xffff0000,
		 0x00000000, 0xffff0000, 0x0000ffff, 0xffffffff,
		 0x00000000, 0xffff0000, 0xffff0000, 0x00000000,
		 0x00000000, 0xffff0000, 0xffff0000, 0x0000ffff,
		 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000,
		 0x00000000, 0xffff0000, 0xffff0000, 0xffffffff,
		 0x00000000, 0xffff0000, 0xffffffff, 0x00000000,
		 0x00000000, 0xffff0000, 0xffffffff, 0x0000ffff,
		 0x00000000, 0xffff0000, 0xffffffff, 0xffff0000,
		 0x00000000, 0xffff0000, 0xffffffff, 0xffffffff,
		 0x00000000, 0xffffffff, 0x00000000, 0x00000000,
		 0x00000000, 0xffffffff, 0x00000000, 0x0000ffff,
		 0x00000000, 0xffffffff, 0x00000000, 0xffff0000,
		 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
		 0x00000000, 0xffffffff, 0x0000ffff, 0x00000000,
		 0x00000000, 0xffffffff, 0x0000ffff, 0x0000ffff,
		 0x00000000, 0xffffffff, 0x0000ffff, 0xffff0000,
		 0x00000000, 0xffffffff, 0x0000ffff, 0xffffffff,
		 0x00000000, 0xffffffff, 0xffff0000, 0x00000000,
		 0x00000000, 0xffffffff, 0xffff0000, 0x0000ffff,
		 0x00000000, 0xffffffff, 0xffff0000, 0xffff0000,
		 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
		 0x00000000, 0xffffffff, 0xffffffff, 0x00000000,
		 0x00000000, 0xffffffff, 0xffffffff, 0x0000ffff,
		 0x00000000, 0xffffffff, 0xffffffff, 0xffff0000,
		 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff,
		 0x0000ffff, 0x00000000, 0x00000000, 0x00000000,
		 0x0000ffff, 0x00000000, 0x00000000, 0x0000ffff,
		 0x0000ffff, 0x00000000, 0x00000000, 0xffff0000,
		 0x0000ffff, 0x00000000, 0x00000000, 0xffffffff,
		 0x0000ffff, 0x00000000, 0x0000ffff, 0x00000000,
		 0x0000ffff, 0x00000000, 0x0000ffff, 0x0000ffff,
		 0x0000ffff, 0x00000000, 0x0000ffff, 0xffff0000,
		 0x0000ffff, 0x00000000, 0x0000ffff, 0xffffffff,
		 0x0000ffff, 0x00000000, 0xffff0000, 0x00000000,
		 0x0000ffff, 0x00000000, 0xffff0000, 0x0000ffff,
		 0x0000ffff, 0x00000000, 0xffff0000, 0xffff0000,
		 0x0000ffff, 0x00000000, 0xffff0000, 0xffffffff,
		 0x0000ffff, 0x00000000, 0xffffffff, 0x00000000,
		 0x0000ffff, 0x00000000, 0xffffffff, 0x0000ffff,
		 0x0000ffff, 0x00000000, 0xffffffff, 0xffff0000,
		 0x0000ffff, 0x00000000, 0xffffffff, 0xffffffff,
		 0x0000ffff, 0x0000ffff, 0x00000000, 0x00000000,
		 0x0000ffff, 0x0000ffff, 0x00000000, 0x0000ffff,
		 0x0000ffff, 0x0000ffff, 0x00000000, 0xffff0000,
		 0x0000ffff, 0x0000ffff, 0x00000000, 0xffffffff,
		 0x0000ffff, 0x0000ffff, 0x0000ffff, 0x00000000,
		 0x0000ffff, 0x0000ffff, 0x0000ffff, 0x0000ffff,
		 0x0000ffff, 0x0000ffff, 0x0000ffff, 0xffff0000,
		 0x0000ffff, 0x0000ffff, 0x0000ffff, 0xffffffff,
		 0x0000ffff, 0x0000ffff, 0xffff0000, 0x00000000,
		 0x0000ffff, 0x0000ffff, 0xffff0000, 0x0000ffff,
		 0x0000ffff, 0x0000ffff, 0xffff0000, 0xffff0000,
		 0x0000ffff, 0x0000ffff, 0xffff0000, 0xffffffff,
		 0x0000ffff, 0x0000ffff, 0xffffffff, 0x00000000,
		 0x0000ffff, 0x0000ffff, 0xffffffff, 0x0000ffff,
		 0x0000ffff, 0x0000ffff, 0xffffffff, 0xffff0000,
		 0x0000ffff, 0x0000ffff, 0xffffffff, 0xffffffff,
		 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000,
		 0x0000ffff, 0xffff0000, 0x00000000, 0x0000ffff,
		 0x0000ffff, 0xffff0000, 0x00000000, 0xffff0000,
		 0x0000ffff, 0xffff0000, 0x00000000, 0xffffffff,
		 0x0000ffff, 0xffff0000, 0x0000ffff, 0x00000000,
		 0x0000ffff, 0xffff0000, 0x0000ffff, 0x0000ffff,
		 0x0000ffff, 0xffff0000, 0x0000ffff, 0xffff0000,
		 0x0000ffff, 0xffff0000, 0x0000ffff, 0xffffffff,
		 0x0000ffff, 0xffff0000, 0xffff0000, 0x00000000,
		 0x0000ffff, 0xffff0000, 0xffff0000, 0x0000ffff,
		 0x0000ffff, 0xffff0000, 0xffff0000, 0xffff0000,
		 0x0000ffff, 0xffff0000, 0xffff0000, 0xffffffff,
		 0x0000ffff, 0xffff0000, 0xffffffff, 0x00000000,
		 0x0000ffff, 0xffff0000, 0xffffffff, 0x0000ffff,
		 0x0000ffff, 0xffff0000, 0xffffffff, 0xffff0000,
		 0x0000ffff, 0xffff0000, 0xffffffff, 0xffffffff,
		 0x0000ffff, 0xffffffff, 0x00000000, 0x00000000,
		 0x0000ffff, 0xffffffff, 0x00000000, 0x0000ffff,
		 0x0000ffff, 0xffffffff, 0x00000000, 0xffff0000,
		 0x0000ffff, 0xffffffff, 0x00000000, 0xffffffff,
		 0x0000ffff, 0xffffffff, 0x0000ffff, 0x00000000,
		 0x0000ffff, 0xffffffff, 0x0000ffff, 0x0000ffff,
		 0x0000ffff, 0xffffffff, 0x0000ffff, 0xffff0000,
		 0x0000ffff, 0xffffffff, 0x0000ffff, 0xffffffff,
		 0x0000ffff, 0xffffffff, 0xffff0000, 0x00000000,
		 0x0000ffff, 0xffffffff, 0xffff0000, 0x0000ffff,
		 0x0000ffff, 0xffffffff, 0xffff0000, 0xffff0000,
		 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
		 0x0000ffff, 0xffffffff, 0xffffffff, 0x00000000,
		 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
		 0x0000ffff, 0xffffffff, 0xffffffff, 0xffff0000,
		 0x0000ffff, 0xffffffff, 0xffffffff, 0xffffffff,
		 0xffff0000, 0x00000000, 0x00000000, 0x00000000,
		 0xffff0000, 0x00000000, 0x00000000, 0x0000ffff,
		 0xffff0000, 0x00000000, 0x00000000, 0xffff0000,
		 0xffff0000, 0x00000000, 0x00000000, 0xffffffff,
		 0xffff0000, 0x00000000, 0x0000ffff, 0x00000000,
		 0xffff0000, 0x00000000, 0x0000ffff, 0x0000ffff,
		 0xffff0000, 0x00000000, 0x0000ffff, 0xffff0000,
		 0xffff0000, 0x00000000, 0x0000ffff, 0xffffffff,
		 0xffff0000, 0x00000000, 0xffff0000, 0x00000000,
		 0xffff0000, 0x00000000, 0xffff0000, 0x0000ffff,
		 0xffff0000, 0x00000000, 0xffff0000, 0xffff0000,
		 0xffff0000, 0x00000000, 0xffff0000, 0xffffffff,
		 0xffff0000, 0x00000000, 0xffffffff, 0x00000000,
		 0xffff0000, 0x00000000, 0xffffffff, 0x0000ffff,
		 0xffff0000, 0x00000000, 0xffffffff, 0xffff0000,
		 0xffff0000, 0x00000000, 0xffffffff, 0xffffffff,
		 0xffff0000, 0x0000ffff, 0x00000000, 0x00000000,
		 0xffff0000, 0x0000ffff, 0x00000000, 0x0000ffff,
		 0xffff0000, 0x0000ffff, 0x00000000, 0xffff0000,
		 0xffff0000, 0x0000ffff, 0x00000000, 0xffffffff,
		 0xffff0000, 0x0000ffff, 0x0000ffff, 0x00000000,
		 0xffff0000, 0x0000ffff, 0x0000ffff, 0x0000ffff,
		 0xffff0000, 0x0000ffff, 0x0000ffff, 0xffff0000,
		 0xffff0000, 0x0000ffff, 0x0000ffff, 0xffffffff,
		 0xffff0000, 0x0000ffff, 0xffff0000, 0x00000000,
		 0xffff0000, 0x0000ffff, 0xffff0000, 0x0000ffff,
		 0xffff0000, 0x0000ffff, 0xffff0000, 0xffff0000,
		 0xffff0000, 0x0000ffff, 0xffff0000, 0xffffffff,
		 0xffff0000, 0x0000ffff, 0xffffffff, 0x00000000,
		 0xffff0000, 0x0000ffff, 0xffffffff, 0x0000ffff,
		 0xffff0000, 0x0000ffff, 0xffffffff, 0xffff0000,
		 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
		 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
		 0xffff0000, 0xffff0000, 0x00000000, 0x0000ffff,
		 0xffff0000, 0xffff0000, 0x00000000, 0xffff0000,
		 0xffff0000, 0xffff0000, 0x00000000, 0xffffffff,
		 0xffff0000, 0xffff0000, 0x0000ffff, 0x00000000,
		 0xffff0000, 0xffff0000, 0x0000ffff, 0x0000ffff,
		 0xffff0000, 0xffff0000, 0x0000ffff, 0xffff0000,
		 0xffff0000, 0xffff0000, 0x0000ffff, 0xffffffff,
		 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000,
		 0xffff0000, 0xffff0000, 0xffff0000, 0x0000ffff,
		 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
		 0xffff0000, 0xffff0000, 0xffff0000, 0xffffffff,
		 0xffff0000, 0xffff0000, 0xffffffff, 0x00000000,
		 0xffff0000, 0xffff0000, 0xffffffff, 0x0000ffff,
		 0xffff0000, 0xffff0000, 0xffffffff, 0xffff0000,
		 0xffff0000, 0xffff0000, 0xffffffff, 0xffffffff,
		 0xffff0000, 0xffffffff, 0x00000000, 0x00000000,
		 0xffff0000, 0xffffffff, 0x00000000, 0x0000ffff,
		 0xffff0000, 0xffffffff, 0x00000000, 0xffff0000,
		 0xffff0000, 0xffffffff, 0x00000000, 0xffffffff,
		 0xffff0000, 0xffffffff, 0x0000ffff, 0x00000000,
		 0xffff0000, 0xffffffff, 0x0000ffff, 0x0000ffff,
		 0xffff0000, 0xffffffff, 0x0000ffff, 0xffff0000,
		 0xffff0000, 0xffffffff, 0x0000ffff, 0xffffffff,
		 0xffff0000, 0xffffffff, 0xffff0000, 0x00000000,
		 0xffff0000, 0xffffffff, 0xffff0000, 0x0000ffff,
		 0xffff0000, 0xffffffff, 0xffff0000, 0xffff0000,
		 0xffff0000, 0xffffffff, 0xffff0000, 0xffffffff,
		 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
		 0xffff0000, 0xffffffff, 0xffffffff, 0x0000ffff,
		 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
		 0xffff0000, 0xffffffff, 0xffffffff, 0xffffffff,
		 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
		 0xffffffff, 0x00000000, 0x00000000, 0x0000ffff,
		 0xffffffff, 0x00000000, 0x00000000, 0xffff0000,
		 0xffffffff, 0x00000000, 0x00000000, 0xffffffff,
		 0xffffffff, 0x00000000, 0x0000ffff, 0x00000000,
		 0xffffffff, 0x00000000, 0x0000ffff, 0x0000ffff,
		 0xffffffff, 0x00000000, 0x0000ffff, 0xffff0000,
		 0xffffffff, 0x00000000, 0x0000ffff, 0xffffffff,
		 0xffffffff, 0x00000000, 0xffff0000, 0x00000000,
		 0xffffffff, 0x00000000, 0xffff0000, 0x0000ffff,
		 0xffffffff, 0x00000000, 0xffff0000, 0xffff0000,
		 0xffffffff, 0x00000000, 0xffff0000, 0xffffffff,
		 0xffffffff, 0x00000000, 0xffffffff, 0x00000000,
		 0xffffffff, 0x00000000, 0xffffffff, 0x0000ffff,
		 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
		 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff,
		 0xffffffff, 0x0000ffff, 0x00000000, 0x00000000,
		 0xffffffff, 0x0000ffff, 0x00000000, 0x0000ffff,
		 0xffffffff, 0x0000ffff, 0x00000000, 0xffff0000,
		 0xffffffff, 0x0000ffff, 0x00000000, 0xffffffff,
		 0xffffffff, 0x0000ffff, 0x0000ffff, 0x00000000,
		 0xffffffff, 0x0000ffff, 0x0000ffff, 0x0000ffff,
		 0xffffffff, 0x0000ffff, 0x0000ffff, 0xffff0000,
		 0xffffffff, 0x0000ffff, 0x0000ffff, 0xffffffff,
		 0xffffffff, 0x0000ffff, 0xffff0000, 0x00000000,
		 0xffffffff, 0x0000ffff, 0xffff0000, 0x0000ffff,
		 0xffffffff, 0x0000ffff, 0xffff0000, 0xffff0000,
		 0xffffffff, 0x0000ffff, 0xffff0000, 0xffffffff,
		 0xffffffff, 0x0000ffff, 0xffffffff, 0x00000000,
		 0xffffffff, 0x0000ffff, 0xffffffff, 0x0000ffff,
		 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
		 0xffffffff, 0x0000ffff, 0xffffffff, 0xffffffff,
		 0xffffffff, 0xffff0000, 0x00000000, 0x00000000,
		 0xffffffff, 0xffff0000, 0x00000000, 0x0000ffff,
		 0xffffffff, 0xffff0000, 0x00000000, 0xffff0000,
		 0xffffffff, 0xffff0000, 0x00000000, 0xffffffff,
		 0xffffffff, 0xffff0000, 0x0000ffff, 0x00000000,
		 0xffffffff, 0xffff0000, 0x0000ffff, 0x0000ffff,
		 0xffffffff, 0xffff0000, 0x0000ffff, 0xffff0000,
		 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
		 0xffffffff, 0xffff0000, 0xffff0000, 0x00000000,
		 0xffffffff, 0xffff0000, 0xffff0000, 0x0000ffff,
		 0xffffffff, 0xffff0000, 0xffff0000, 0xffff0000,
		 0xffffffff, 0xffff0000, 0xffff0000, 0xffffffff,
		 0xffffffff, 0xffff0000, 0xffffffff, 0x00000000,
		 0xffffffff, 0xffff0000, 0xffffffff, 0x0000ffff,
		 0xffffffff, 0xffff0000, 0xffffffff, 0xffff0000,
		 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
		 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,
		 0xffffffff, 0xffffffff, 0x00000000, 0x0000ffff,
		 0xffffffff, 0xffffffff, 0x00000000, 0xffff0000,
		 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
		 0xffffffff, 0xffffffff, 0x0000ffff, 0x00000000,
		 0xffffffff, 0xffffffff, 0x0000ffff, 0x0000ffff,
		 0xffffffff, 0xffffffff, 0x0000ffff, 0xffff0000,
		 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
		 0xffffffff, 0xffffffff, 0xffff0000, 0x00000000,
		 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
		 0xffffffff, 0xffffffff, 0xffff0000, 0xffff0000,
		 0xffffffff, 0xffffffff, 0xffff0000, 0xffffffff,
		 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
		 0xffffffff, 0xffffffff, 0xffffffff, 0x0000ffff,
		 0xffffffff, 0xffffffff, 0xffffffff, 0xffff0000,
		 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
   };  /*XHP_wordmap*/ 

static jmp_buf env;
/*
 * routine to handle the bus error we might get in testing for the alignment
 * restrictions of this cpu.
 */
 static int sigbusHandler()
{
  XHP_QUADALIGN = 1;
  longjmp(env);
}

/*-
 *-----------------------------------------------------------------------
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *	The
 *
 * Results:
 *	screenInfo init proc field set
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */

InitOutput(pScreenInfo, argc, argv)
    ScreenInfo 	  *pScreenInfo;
    int     	  argc;
    char    	  **argv;
{
    int i;
    FILE *in;
    int numScreens;
    int position[MAXSCREENS];
    static firstTime = 1;
    static int oldNumScreens;
    static ScreenTableRec *oldFoundScreens[MAXSCREENS];
    char *dispaddr, *getenv();
    ScreenTableRec *s, *FindScreen();
    struct stat statbuf;
    static char minornumber[80];

    if (firstTime) firstTime = 0;
    else {
	/* Re-register the screens we've already found */
	for ( i = 0; i < oldNumScreens; i++) {
	    s = oldFoundScreens[i];
	    AddScreen(s->InitScreen, argc, argv);
	    pScreenInfo->screen[i].CloseScreen = s->CloseScreen;
	}
	goto skip_search;
    }

    /*
     * test for data alignment restriction of this cpu.  If this cpu
     * doesn't allow long-word writes on an odd address, then we assume
     * that it requires quad-word alignment.  This reduces our headaches
     * to only two cases - 68020s (no restrictions) and others (e.g.
     * 68010 and Spectrum).
     */
    {
      int *test;
      char foo[8]; 
      static struct sigvec timeout_vec = { sigbusHandler, 0, 0 };
      struct sigvec old_vec;


      test = (int *) ((int)foo | 3) + 1; /* generate an odd address */
      XHP_QUADALIGN = 0;

#ifdef hp9000s300     /* check for 310 */
      sigvector(SIGBUS, &timeout_vec, &old_vec);
      if(!setjmp(env))
	*test = 1;  /* generate a bus error on 68010s or Spectrums */
      sigvector(SIGBUS, &old_vec, 0);
#else  /* need word align on 800 */
      XHP_QUADALIGN = 1;
#endif hp9000s300
    
    }
    ErrorfOn++;

    if (pScreenInfo->numScreens == pScreenInfo->arraySize)
    {
        int oldSize = pScreenInfo->arraySize;
	pScreenInfo->arraySize += 5;
	pScreenInfo->screen = (ScreenPtr)Xrealloc(
	    pScreenInfo->screen, 
	    pScreenInfo->arraySize * sizeof(ScreenRec));
    }

    iomapBase = (unsigned char *) IOMAP_BASE;
    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    if (dispaddr = getenv("SB_DISPLAY_ADDR"))
	iomapBase = (unsigned char *) strtol(dispaddr, (char **)NULL, 0);

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
    {
	pScreenInfo->formats[i] = formats[i];
    }

    sprintf(xscreens,"/usr/lib/X11/X%sscreens",display);

    if (NULL == (in = fopen(xscreens, "r")))
	{
	perror(xscreens);
#if 0
        FatalError("Can't open screen configuration file.\n");
#else
   	ErrorF("Can't open screen configuration file, defaulting to %s.\n",
	       DefaultScreen);
	ConfigFile=FALSE;
#endif
        }
    else {
	ConfigFile=TRUE;
	}

    for ( i = 0; i < MAXSCREENS; i++ )
	position[i] = -1;

    numScreens = 0;
    for ( i = 0; i < MAXSCREENS &&
		 ((ConfigFile == FALSE) ? TRUE : ! feof(in) ); i++ )
    {
	char *argv[MAXARG];
	int argc;
	int screenIndex;
	
	if(ConfigFile) {
	    while (!(argc = ReadLine(in, argv)))	/** handle blank lines **/
		;

	    if (argc == -1)				/* eof */
		break;

	    if ( argc < 0 )
	    {
		ErrorF("InitOutput: %s: line %d: Too many fields.\n",
		    xscreens, i+1);
		goto fatal_error;
	    }

	    /* for compatibility with R2 screens tables */
	    if (argc == 3) {
		if (strcmp (argv[0], "topcat") == 0) {
		    argv[0] = argv[2];
		    argc = 1;
		} else {
		    ErrorF ("InitOutput:  %s, line %d:  unsupported display type \"%s\".\n",
			    xscreens, i+1, argv[0]);
		    goto fatal_error;
		}
	    }

	    if ( argc != 1 )
	    {
		ErrorF("InitOutput: %s: line %d: Wrong number of fields.\n", xscreens, i+1);
		goto fatal_error;
	    }

	    screenIndex = i;		/* we will do it by lines  (cpa) */

	    if ( screenIndex < 0 || screenIndex >= MAXSCREENS )
	    {
		ErrorF("InitOutput: %s: line %d: More than %d screens specified.\n",
		    xscreens, i+1, MAXSCREENS);
		goto fatal_error;
	    }

	}
	else {
     	    screenIndex = i;
	    argv[0] = DefaultScreen;
	}
    
        position[screenIndex] = i;

	if(ConfigFile == TRUE || (ConfigFile == FALSE && i == 0)) {
	    if ( (s = FindScreen(argv[0])) == NULL )
	    {
		ErrorF("InitOutput: %s: line %d: Unknown screen %s.\n",
		    xscreens, i+1, argv[0]);
		goto fatal_error;
	    } 

	/* munge new structure to match old argv structures */
	/* BOGOSITY ALERT argv and argc are munged for subsequent calls */

	argv[2] = argv[0];
	argv[0] = s->productNickname;
 
        if (stat(argv[2],&statbuf) < 0)
		{
		ErrorF("%s: could not stat %s.\n",argv[0],argv[2]);
		return(FALSE);
		}
	
	sprintf(minornumber, "%x", minor(statbuf.st_rdev));
        argv[3] = minornumber;
        argc = 4;
     
	if ( FALSE == (s->InfoScreen)(screenIndex, argv, argc) )
	{
	    ErrorF("InitOutput: %s: line %d: Couldn't find this screen.\n",
		xscreens, i+1);
	    goto fatal_error;
	}

	pScreenInfo->screen[screenIndex].CloseScreen = s->CloseScreen;

	/* Keep track of this screen for re-registration later */
	oldFoundScreens[screenIndex] = s;
	numScreens++;
	}
    }

    if ( i > MAXSCREENS && ((ConfigFile == FALSE) ? FALSE :! feof(in)) )
    {
	ErrorF("InitOutput: %s: line %d: Too many screens.  MAXSCREENS = %d\n",
	    xscreens, i+1, MAXSCREENS);
	goto fatal_error;
    }
    if (ConfigFile)
	    fclose(in);
    
    /* Check if all the screens from 0..numScreens - 1 have been defined */

    for ( i = 0; i < numScreens; i++ )
	if ( position[i] == -1 ) {
	    ErrorF("InitOutput: %s: Screen %d not defined.\n",
		   xscreens, i);
	    goto fatal_error;
	}

    /** now that we're sure we've got valid data, do an AddScreen on each **/

    for (i=0; i<numScreens; i++) {
	s = oldFoundScreens[i];
	AddScreen(s->InitScreen, argc, argv);
    }

    /* Setup multiScreenTable */
    for ( i = 0; i < numScreens; i++ )
    {
	int left, here, right;
	here = position[i];
	multiScreenTable[here].position = i;
	multiScreenTable[here].left = ( i == 0 ? numScreens : i) - 1;
	multiScreenTable[here].right = 	( i == numScreens - 1 ) ? 0 : i + 1;
    }

    oldNumScreens = numScreens; /* Save for re-initialization time */
    
    ErrorfOn--;

  skip_search:

    sunInitCursor();

    if (Rtprio)
	if (rtprio(0,Rtprio) == -1)
	    ErrorF("rtprio failed.\n");

    return; /* What should we return? */

    fatal_error:
    fclose(in);
    ErrorF("InitOutput: Couldn't initialize screens.\n");
    exit(1);
}

int
ReadLine(in, argv)
    FILE *in;
    char **argv;
{
    int i, argc;
    static char line[MAXSTRING];
    char *s;
    int state;

    if (NULL == fgets(line, MAXSTRING, in)) return(-1);

    for ( state = argc = 0, s = line; argc < MAXARG; s++)
    {
	switch ( *s )
	{
	    case '#':
	    case '\n':
		*s = '\0';
	    case '\0':
		return (argc);
	    case ' ':
	    case '\t':
		state = 0;
		*s = '\0';
		break;
	    default:
		if ( state == 0 )
		{
		    state = 1;
		    argv[argc++] = s;
		}
		break;
	}
    }
    return(-2);
}

ScreenTableRec *FindScreen(devname)
    char *devname;
{
    ScreenTableRec *s;
    char *ch;
    int fd, gcon, gcid, s_gcid;
    char name[40];
    catseyePrivPtr catseye;
    static struct stat buf;
    CATSEYE *ce;
    unsigned char *FrameBufferBase();

#if 0
    /** downshift the names **/
    for (ch=name; *ch; ch++)
	if (isupper(*ch))
	    *ch = tolower(*ch);
#endif

    if ((fd = open(devname, O_RDWR)) <  0)
    {
        perror(devname);
        ErrorF("FindScreen couldn't open %s \n", devname);
        goto fatal_error;
    }
    if (ioctl(fd, GCON, &gcon) < 0 || ioctl(fd, GCID, &gcid) < 0)
    {
#if 0
        ErrorF("FindScreen: couldn't GCON and GCID %s \n", devname);
        goto fatal_error;
#else	
	gcid = STUPID_MOBERLY;
#endif
    }

    fstat(fd, &buf);

    switch (gcid) {
	case 8:
		strcpy(name, "gatorbox");
		break;
	case 9:
	        catseye = (catseyePrivPtr) Xalloc(sizeof(catseyePriv));
	        catseye->catseyeDev = (CATSEYE *) iomapBase;

		if (ioctl(fd, GCMAP, &catseye->catseyeDev) < 0)
		{
		    perror("GCMAP:");
		    ErrorF("FindScreen: Error getting address of %s\n", devname);
		    close(fd);
		    return FALSE;
		}

		ce = catseye->catseyeDev;
	        s_gcid = ce->id_second;
		if(s_gcid >= LCC)
		    strcpy(name, "catseye");
		else if (ce->bits)
		    strcpy(name, "mrtopcat");
		else
		    strcpy(name, "topcat");

		if (ioctl(fd, GCUNMAP, &catseye->catseyeDev) < 0) {
		    perror("GCUNMAP:");
		    ErrorF("FindScreen: Error freeing temp storage %s\n", devname);
		    close(fd);
		    return FALSE;
		}	

		Xfree(catseye);

                break;
        case 10:
		if ((minor(buf.st_rdev)) & 0x000003)
			strcpy(name, "orenaissance");
		else
	         	strcpy(name, "renaissance");
		break;
	case 11:
		strcpy(name, "catseye");
		break;
	case 14:
		if(( minor(buf.st_rdev)) & 0x000003)
			strcpy(name, "odavinci");
		else
	         	strcpy(name, "davinci");
		break;
	case STUPID_MOBERLY:
		strcpy(name, "moberly");
		break;
	default:
                ErrorF("FindScreen: unknown screen type  %s \n", devname);
        	goto fatal_error;
		break;
	}

    for ( s = screenTable; s->productNumber != NULL; s++)
    {
	if ( strcmp(s->productNumber, name) == 0 ||
	    strcmp(s->productNickname, name) == 0)
	{
	    return (s);
	}
    }
    return (NULL);

    fatal_error:
    ErrorF("InitOutput: Couldn't initialize screens.\n");
    exit(1);

}

unsigned char *FrameBufferBase(size)
    long int size;
{
    unsigned char *base = iomapBase;

    /* Round size to a 4K page */
    size = (size + 4095) & 0xfffff000;
    iomapBase += size;
    return ( base );
}


/*
 * The following routine was taken from server/ddx/sun/sunInit.c
 */

/*-
 *-----------------------------------------------------------------------
 * sunScreenInit --
 *	Things which must be done for all types of frame buffers...
 *	Should be called last of all.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *      The screen's video is forced on.
 *	The graphics context for the screen is created...
 *
 *-----------------------------------------------------------------------
 */
void
sunScreenInit (pScreen)
    ScreenPtr	  pScreen;
{
    fbFd  	*fb;
    DrawablePtr	pDrawable;

    fb = &sunFbs[pScreen->myNum];

    /*
     * Prepare the GC for cursor functions on this screen.
     * Do this before setting interceptions to avoid looping when
     * putting down the cursor...
     */
    pDrawable = (DrawablePtr)
                    (((cfbPrivScreenPtr)(pScreen->devPrivate))->pDrawable);

    fb->pGC = (GC *)CreateScratchGC (pDrawable->pScreen, pDrawable->depth);

    /*
     * By setting graphicsExposures false, we prevent any expose events
     * from being generated in the CopyArea requests used by the cursor
     * routines.
     */
    fb->pGC->graphicsExposures = FALSE;

    /*
     * Preserve the "regular" functions
     */
    fb->CreateGC =	    	    	pScreen->CreateGC;
    fb->CreateWindow = 	    	    	pScreen->CreateWindow;
    fb->ChangeWindowAttributes =    	pScreen->ChangeWindowAttributes;
    fb->GetImage =	    	    	pScreen->GetImage;
    fb->GetSpans =			pScreen->GetSpans;

    /*
     * Interceptions
     */
    pScreen->CreateGC =	    	    	sunCreateGC;
    pScreen->CreateWindow = 	    	sunCreateWindow;
    pScreen->ChangeWindowAttributes = 	sunChangeWindowAttributes;
    pScreen->GetImage =	    	    	sunGetImage;
    pScreen->GetSpans =			sunGetSpans;

    /*
     * Cursor functions
     */
    pScreen->RealizeCursor = 	    	sunRealizeCursor;
    pScreen->UnrealizeCursor =	    	sunUnrealizeCursor;
    pScreen->DisplayCursor = 	    	sunDisplayCursor;
    pScreen->SetCursorPosition =    	sunSetCursorPosition;
    pScreen->CursorLimits = 	    	sunCursorLimits;
    pScreen->PointerNonInterestBox = 	sunPointerNonInterestBox;
    pScreen->ConstrainCursor = 	    	sunConstrainCursor;
    pScreen->RecolorCursor = 	    	sunRecolorCursor;

}

/*
 * Ensure video is disabled when server exits....
 */
Bool
hpCloseScreen(i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
  
    if (pScreen->SaveScreen)
    {
	return ((*pScreen->SaveScreen)(pScreen, SCREEN_SAVER_OFF));
    }
    else
	return TRUE;
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX()
{
#ifdef MULTI_X_HACK
    if (XMulti)
	ipc_exit();
#endif
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
#ifdef MULTI_X_HACK
    if (XMulti)
	ipc_exit();
#endif
}

int
ddxProcessArgument (argc, argv, i)
    int	argc;
    char *argv[];
    int	i;
{
    void UseMsg();

    if ( strcmp( argv[i], "-rtprio") == 0)
    {
	if (++i >= argc)
	    UseMsg();
	Rtprio = atoi(argv[i]);
	return 2;
    }
    else if ( strcmp( argv[i], "-tcbd") == 0)
    {
	TopcatBrainDamage++;
	return 1;
    }
#ifdef MULTI_X_HACK
    else if ( strcmp( argv[i], "-multi") == 0)
    {
	XMulti++;
	return 1;
    }
#endif
    return 0;
}

void
ddxUseMsg()
{
#ifdef MULTI_X_HACK
    ErrorF("-multi                 run server in multi-X mode\n");
#endif
    ErrorF("-rtprio number         set real time priority\n");
    ErrorF("-tcbd                  run in topcat braindamage mode\n");
}
