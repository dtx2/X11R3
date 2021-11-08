/***********************************************************
		Copyright IBM Corporation 1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaSet.c,v 9.0 88/10/18 12:52:45 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaSet.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaSet.c,v 9.0 88/10/18 12:52:45 erik Exp $";
static char sccsid[] = "@(#)setega.c	3.1 88/09/22 09:33:27";
#endif

#include "egaReg.h"
#include "ibmIOArch.h"

void
egaSetColor( color, r, g, b )
register int color ;
register short r, b, g ;
{
register unsigned char tmp ;

if ( color > 0xF ) /* Sanity Check */
	return ;

tmp = inb( 0x3DA ) ;	/* Reset The Flip-Flop */
outb( 0x3C0, color ) ;	/* Point Palette Register To Color Entry */

tmp  = ( r >> 13 ) & 0x04 ;
tmp |= ( r >>  9 ) & 0x20 ;
tmp |= ( g >> 14 ) & 0x02 ;
tmp |= ( g >> 10 ) & 0x10 ;
tmp |= ( b >> 15 ) & 0x01 ;
tmp |= ( b >> 11 ) & 0x08 ;
outb( 0x3C0, tmp ) ;		/* Set The Palette Register */
outb( 0x3C0, color | 0x20 ) ;	/* Re-Enable Video Access To The Palette */

return ;
}

/*
 * Initialize the ega to 640 x 350, 16 of 64 colors @ b8000
 */

void
set_ega_graphics()
{
/* Setup I/O Base Address */
/* Color 640 by 350 -- 16 Color */
#define Color_Base_Reg ( 0x3D0 )

/* Setup Graphic-Chip Decoders */
outb( 0x3CC, 0x0 ) ; /* VS.Graphic1Pos */
outb( 0x3CA, 0x1 ) ; /* VS.Graphic2Pos */

/* Sequencer Registers  03C4 & 03C5 */
outb( 0x3C4, 0x00 ) ;
outb( 0x3C5, /* VS.Seq_Reset */ 0x1 ) ; /* -- Hardware Syncronous RESET */
outb( 0x3C4, 0x01 ) ;
outb( 0x3C5, /* VS.Clock_Mode */ 0x01 ) ;
outb( 0x3C4, 0x02 ) ;
outb( 0x3C5, /* VS.Mask_Map */ 0x0F ) ; /* All Planes Writable */
outb( 0x3C4, 0x03 ) ;
outb( 0x3C5, /* VS.Char_Map_Select */ 0x00 ) ;
outb( 0x3C4, 0x04 ) ;
outb( 0x3C5, /* VS.Memory_Mode */ 0x06 ) ;

/* Write General Registers */
/* VS.Input_Status_0 & VS.Input_Status_1 are READ-ONLY */
outb( 0x3C2, 0xA7 ) ; /* VS.Misc_Output_Reg */
outb( Color_Base_Reg + 0xA, /* VS_Start.Feature_Control */ 0x0 ) ;

/* Re-Enable Hardware i.e. Reset Register */
outb( 0x3C4, 0x00 ) ;
outb( 0x3C5, 0x03 ) ; /* VS.Seq_Reset */ /* Enable Hardware Reset Register */

/* Attribute Registers */
INTS_OFF();
/* Initialize Flip-Flop */
{ register tmp = inb( Color_Base_Reg + 0xA ) ; }

/* Set Palette Register Value Equal To Palette Register Index Number */
/* i.e. Palette is 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
{
	register IO_Address Target_Reg ;
	for ( Target_Reg = 0 ; Target_Reg <= 0xF ; Target_Reg++ ) {
		outb( 0x3C0, Target_Reg ) ;
		outb( 0x3C0, Target_Reg ) ;
	}
}
/* Rest Of The Attribute Registers */
/* Note: 0x20 is added to the index */
outb( 0x3c0, 0x30 ) ;
outb( 0x3C0, 0x03 ) ; /* VS.Attr_Mode - ega different fron vga */
outb( 0x3c0, 0x31 ) ;
outb( 0x3C0, 0x00 ) ; /* VS.Overscan_Color */
outb( 0x3c0, 0x32 ) ;
outb( 0x3C0, 0x0F ) ; /* VS.Color_Plane_En */
outb( 0x3c0, 0x33 ) ;
outb( 0x3C0, 0x00 ) ; /* VS.Horiz_PEL_Pan */
/* Re-Enable Video Access To The Color Palette */
INTS_ON();

/* Set CRT Controller Registers  03?4 & 03?5 */
outb( 0x3D4, 0x00 ) ;
outb( 0x3D5, 0x5B ) ; /* VS.Horiz_Total */
outb( 0x3D4, 0x01 ) ;
outb( 0x3D5, 0x4F ) ; /* VS.Horiz_End */
outb( 0x3D4, 0x02 ) ;
outb( 0x3D5, 0x53 ) ; /* VS.H_Blank_Start */
outb( 0x3D4, 0x03 ) ;
outb( 0x3D5, 0x37 ) ; /* VS.H_Blank_End */
outb( 0x3D4, 0x04 ) ;
outb( 0x3D5, 0x52 ) ; /* VS.H_Retrace_Start */
outb( 0x3D4, 0x05 ) ;
outb( 0x3D5, 0x00 ) ; /* VS.H_Retrace_End */
outb( 0x3D4, 0x06 ) ;
outb( 0x3D5, 0x6C ) ; /* VS.Vert_Total */
outb( 0x3D4, 0x07 ) ;
outb( 0x3D5, 0x1F ) ; /* VS.Overflow */
outb( 0x3D4, 0x08 ) ;
outb( 0x3D5, 0x00 ) ; /* VS.Preset_Row_Scan */
outb( 0x3D4, 0x09 ) ;
outb( 0x3D5, 0x00 ) ; /* VS.Max_Scan_Line */
outb( 0x3D4, 0x0A ) ;
outb( 0x3D5, 0x00 ) ; /* VS.Cursor_Start */
outb( 0x3D4, 0x0B ) ;
outb( 0x3D5, 0x00 ) ; /* VS.Cursor_End */
outb( 0x3D4, 0x0C ) ;
outb( 0x3D5, /* VS.Start_Addr_Hi */ 0 ) ;
outb( 0x3D4, 0x0D ) ;
outb( 0x3D5, /* VS.Start_Addr_Lo */ 0 ) ;
outb( 0x3D4, 0x0E ) ;
outb( 0x3D5, /* VS.Cursor_Loc_Hi */ 0 ) ;
outb( 0x3D4, 0x0F ) ;
outb( 0x3D5, /* VS.Cursor_Loc_Lo */ 0 ) ;
outb( 0x3D4, 0x10 ) ;
outb( 0x3D5, /* VS.V_Retrace_Start */ 0x5E ) ;
outb( 0x3D4, 0x11 ) ;
outb( 0x3D5, 0x2B ) ; /* VS.V_Retrace_End */ /* Clear any pending interrupt */
outb( 0x3D4, 0x12 ) ;
outb( 0x3D5, /* VS.V_Display_End */ 0x5D ) ;
outb( 0x3D4, 0x13 ) ;
outb( 0x3D5, /* VS.Offset */ 0x28 ) ;
outb( 0x3D4, 0x14 ) ;
outb( 0x3D5, /* VS.Underline_Loc */ 0x0F ) ;
outb( 0x3D4, 0x15 ) ;
outb( 0x3D5, /* VS.V_Blank_Start */ 0x5F ) ;
outb( 0x3D4, 0x16 ) ;
outb( 0x3D5, /* VS.V_Blank_End */ 0x0A ) ;
outb( 0x3D4, 0x17 ) ;
outb( 0x3D5, /* VS.CRTC_Mode */ 0xE3 ) ;
outb( 0x3D4, 0x18 ) ;
outb( 0x3D5, 0xFF ) ; /* VS.Line_Compare */

/* Set Graphics Registers  03CE & 03CF */
outb( 0x3CE, 0x00 ) ;
outb( 0x3CF, /* VS.Set_Reset */ 0x00 ) ;
outb( 0x3CE, 0x01 ) ;
outb( 0x3CF, /* VS.Enb_Set_Reset */ 0x00 ) ;
outb( 0x3CE, 0x02 ) ;
outb( 0x3CF, /* VS.Color_Compare */ 0x00 ) ;
outb( 0x3CE, 0x03 ) ;
outb( 0x3CF, /* VS.Data_Rotate */ 0x00 ) ;
outb( 0x3CE, 0x04 ) ;
outb( 0x3CF, /* VS.Read_Map_Select */ 0x00 ) ;
outb( 0x3CE, 0x05 ) ;
outb( 0x3CF, /* VS.Graphics_Mode */ 0x02 ) ; /* Set For Writing Individual Pixels */
outb( 0x3CE, 0x06 ) ;
outb( 0x3CF, /* VS.Miscellaneous */ 0x0D ) ; /* @ B8000 for 32K */
outb( 0x3CE, 0x07 ) ;
outb( 0x3CF, /* VS.Color_Dont_Care */ 0x0F ) ;
outb( 0x3CE, 0x08 ) ;
outb( 0x3CF, /* VS.Bit_Mask */ 0xFF ) ; /* All Bits Writable */

return ;
}
