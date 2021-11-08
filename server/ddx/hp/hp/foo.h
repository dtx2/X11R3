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

#if	RCSID && IN_MAIN
static char *trcsid="$XConsortium: foo.h,v 1.2 88/09/06 15:24:36 jim Exp $";
#endif	RCSID

/*************************************************************
 *  foo.h	- generic hp9000/s300 display card
 *
 *  This module is part of the low-level graphics primitives 
 *  for the Hewlett-Packard 9000 Series 300/800.
 *
 *  (c) Copyright 1986 Hewlett-Packard Company.  All rights are
 *  reserved.  Copying or other reproduction of the program except
 *  for archival purposes is prohibited without prior written
 *  consent of Hewlett-Packard Company.
 * 
 */
 
#include <sys/types.h> 
#include <sys/graphics.h>  

#define u_char unsigned char
#define u_short unsigned short
#define u_int unsigned int

#ifndef LCC	
#define LCC 5
#endif

#ifndef HRC
#define HRC 6
#endif

#ifndef HRM
#define HRM 7
#endif

#ifndef CC	/* this is for the Castrato Catseye (640x480) for 319x */
#define CC  9
#endif

typedef struct {
    u_short id_reset;		/* id and reset register 	*//* 0x001 */
    u_char filler2;
    u_char interrupts;		/* interrupts			*//* 0x03 */
    u_char filler2a;
    u_char t_memwide;		/* top half frame buf width	*//* 0x05 */
    u_char filler2b;
    u_char b_memwide;		/* bottom half frame buf width	*//* 0x07 */
    u_char filler2c;
    u_char t_memhigh;		/* top half frame buf height 	*//* 0x09 */
    u_char filler2d;
    u_char b_memhigh;		/* bot half frame buf height	*//* 0x0b */
    u_char filler2e;
    u_char t_dispwide;		/* top half display width	*//* 0x0d */
    u_char filler2f;
    u_char b_dispwide;		/* bot half display width	*//* 0x0f */
    u_char filler2g;
    u_char t_disphigh;		/* top half display height	*//* 0x11 */
    u_char filler2h;
    u_char b_disphigh;		/* bot half display height	*//* 0x13 */
    u_char filler2i;
    u_char id_second;		/* secondary id 5=LCC 6=HRC 	*//* 0x15 */
    u_char filler2j;            /* 7=HRM, 9=319X		*/
    u_char bits;		/* 0 square pixels, 1 double hi *//* 0x17 */
    u_char filler2k;
    u_char byte_pixel;		/* byte/pixel at powerup        *//* 0x19 */
    u_char filler2l;
    u_char id_crtc;		/* CRTC ID IRIS			*//* 0x1B */
    u_char filler2m;
    u_char rom_rev;		/* rom revision 		*//* 0x1D */
    u_char filler3[57];
    u_char t_cmapaddr;		/* color map address (MSB)	*//* 0x57 */
    u_char filler2n;
    u_char b_cmapaddr;		/* color map address (LSB)	*//* 0x59 */
    u_char filler2o;
    u_char num_planes;		/* number of color planes	*//* 0x5b */
    u_char id_font[16356];	/* display id, font, ...	*/

} CATSEYE;


typedef struct {
    CATSEYE  *catseyeDev;        /* pointer to device hardware */
    ColormapPtr InstalledMap;  	      /* pointer to installed colormap */
} catseyePriv;
typedef catseyePriv *catseyePrivPtr;



#define getGpHardware(pScreen)						\
    (((catseyePrivPtr)((cfbPrivScreenPtr)((pScreen)->devPrivate))	\
       ->pHardwareScreen)->catseyeDev)

#define getPlanesMask(pScreen)						\
    (((cfbPrivScreenPtr)((pScreen)->devPrivate))->planesMask)


#ifndef	GXclear

/* defines for replacement rules -- same as for X window system */

#define	GXclear		0x0		/* 0 			*/
#define GXand		0x1		/* src AND dst 		*/
#define GXandReverse	0x2		/* src AND NOT dst	*/
#define GXcopy		0x3		/* src 			*/
#define GXandInverted	0x4		/* NOT src AND dst	*/
#define GXnoop		0x5		/* dst			*/
#define GXxor		0x6		/* src XOR dst		*/
#define GXor		0x7		/* src OR dst		*/
#define GXnor		0x8		/* NOT src AND NOT dst	*/
#define GXequiv		0x9		/* NOT src XOR dst	*/
#define GXinvert	0xa		/* NOT dst		*/
#define GXorReverse	0xb		/* src OR NOT dst	*/
#define GXcopyInverted	0xc		/* NOT src		*/
#define GXorInverted	0xd		/* NOT src OR dst	*/
#define GXnand		0xe		/* NOT src OR NOT dst	*/
#define GXset		0xf		/* 1			*/

#endif	GXclear

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define SET_FRM_SPACE	;
#define SET_CTL_SPACE 	;

