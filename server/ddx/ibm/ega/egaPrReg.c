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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaPrReg.c,v 9.0 88/10/18 12:52:23 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaPrReg.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaPrReg.c,v 9.0 88/10/18 12:52:23 erik Exp $";
static char sccsid[] = "@(#)pregareg.c	3.1 88/09/22 09:33:24";
#endif

/*
 * REGISTER USAGE
 *	EVERYTHING
 */

#include "egaVideo.h"

void save_ega_state( VS )
register struct video_hardware_state *VS ;
{
register IO_Address Target_Reg ; /* if mono == 0x3B0, if color == 3D0 */
register IO_Address Base_Reg ; /* if mono == 0x3B0, if color == 3D0 */

/* Read General Registers */
VS->Feature_Control = 0x0 ; /* Write-Only */
VS->Video_Enable = inb( 0x3C3 ) ;
Base_Reg = 0x3D0 ;
VS->Misc_Output_Reg = inb( 0x3CC ) ;
VS->Input_Status_0 = inb( 0x3C2 ) ;
/* Initialize Flip-Flop */
VS->Input_Status_1 = inb( Base_Reg + 0xA ) ;

/* Save Attribute Registers  03C0 & 03C1 */
INTS_OFF() ;
VS->Attr_Addr_Reg = inb( 0x3C0 ) ; /* Save Existing Index First */
/* Target_Reg Used As Scrap */
for ( Target_Reg = 0 ; Target_Reg <= 0xF ; Target_Reg++ ) {
	outb( 0x3C0, Target_Reg ) ;
	/* Read It, Save It, Then Write It Back */
	outb( 0x3c0, ( VS->Palette[Target_Reg] = inb( 0x3C1 ) ) ) ;
}
outb( 0x3c0, 0x30 ) ;
outb( 0x3c0, ( VS->Attr_Mode = inb( 0x3C1 ) ) ) ;
outb( 0x3c0, 0x31 ) ;
outb( 0x3c0, ( VS->Overscan_Color = inb( 0x3C1 ) ) ) ;
outb( 0x3c0, 0x32 ) ;
outb( 0x3c0, ( VS->Color_Plane_En = inb( 0x3C1 ) ) ) ;
outb( 0x3c0, 0x33 ) ;
outb( 0x3c0, ( VS->Horiz_PEL_Pan = inb( 0x3C1 ) ) ) ;
outb( 0x3c0, 0x34 ) ;
outb( 0x3c0, ( VS->Color_Select = inb( 0x3C1 ) ) ) ; /* Attr_Addr_Reg == 14 */
/* Re-Enable Video Access To The Color Palette */
INTS_ON() ;

/* Save Crt Controller Registers  03?4 & 03?5 */
VS->Index_Reg = inb( Target_Reg = ( Base_Reg += 0x4 ) ) ;
outb( Base_Reg, 0x00 ) ;
VS->Horiz_Total = inb( ++Target_Reg ) ;
outb( Base_Reg, 0x01 ) ;
VS->Horiz_End = inb( Target_Reg ) ;
outb( Base_Reg, 0x02 ) ;
VS->H_Blank_Start = inb( Target_Reg ) ;
outb( Base_Reg, 0x03 ) ;
VS->H_Blank_End = inb( Target_Reg ) ;
outb( Base_Reg, 0x04 ) ;
VS->H_Retrace_Start = inb( Target_Reg ) ;
outb( Base_Reg, 0x05 ) ;
VS->H_Retrace_End = inb( Target_Reg ) ;
outb( Base_Reg, 0x06 ) ;
VS->Vert_Total = inb( Target_Reg ) ;
outb( Base_Reg, 0x07 ) ;
VS->Overflow = inb( Target_Reg ) ;
outb( Base_Reg, 0x08 ) ;
VS->Preset_Row_Scan = inb( Target_Reg ) ;
outb( Base_Reg, 0x09 ) ;
VS->Max_Scan_Line = inb( Target_Reg ) ;
outb( Base_Reg, 0x0A ) ;
VS->Cursor_Start = inb( Target_Reg ) ;
outb( Base_Reg, 0x0B ) ;
VS->Cursor_End = inb( Target_Reg ) ;
outb( Base_Reg, 0x0C ) ;
VS->Start_Addr_Hi = inb( Target_Reg ) ;
outb( Base_Reg, 0x0D ) ;
VS->Start_Addr_Lo = inb( Target_Reg ) ;
outb( Base_Reg, 0x0E ) ;
VS->Cursor_Loc_Hi = inb( Target_Reg ) ;
outb( Base_Reg, 0x0F ) ;
VS->Cursor_Loc_Lo = inb( Target_Reg ) ;
outb( Base_Reg, 0x10 ) ;
VS->V_Retrace_Start = inb( Target_Reg ) ;
outb( Base_Reg, 0x11 ) ;
VS->V_Retrace_End = inb( Target_Reg ) ;
outb( Base_Reg, 0x12 ) ;
VS->V_Display_End = inb( Target_Reg ) ;
outb( Base_Reg, 0x13 ) ;
VS->Underline_Loc = inb( Target_Reg ) ;
outb( Base_Reg, 0x14 ) ;
VS->Offset = inb( Target_Reg ) ;
outb( Base_Reg, 0x15 ) ;
VS->V_Blank_Start = inb( Target_Reg ) ;
outb( Base_Reg, 0x16 ) ;
VS->V_Blank_End = inb( Target_Reg ) ;
outb( Base_Reg, 0x17 ) ;
VS->CRTC_Mode = inb( Target_Reg ) ;
outb( Base_Reg, 0x18 ) ;
VS->Line_Compare = inb( Target_Reg ) ;
/* Readjust Base Register */
Base_Reg -= 0x4 ;

/* Sequencer Registers  03C4 & 03C5 */
/* VS->Seq_Addr_Reg = inb( 0x3C4 ) ; */ /*	03C4	--	SAME */
outb( 0x3C4, 0x00 ) ;
VS->Seq_Reset = inb( 0x3C5 ) ;
outb( 0x3C4, 0x01 ) ;
VS->Clock_Mode = inb( 0x3C5 ) ;
outb( 0x3C4, 0x02 ) ;
VS->Mask_Map = inb( 0x3C5 ) ;
outb( 0x3C4, 0x03 ) ;
VS->Char_Map_Select = inb( 0x3C5 ) ;
outb( 0x3C4, 0x04 ) ;
VS->Memory_Mode = inb( 0x3C5 ) ;

/* Graphics Registers  03CE & 03CF */
/* VS->Graphics_Addr = inb( 0x3CE ) ; */ /*	03CE	--	SAME */
/* ??????? */		/*	03CF	--	SAME */
outb( 0x3CE, 0x00 ) ;
VS->Set_Reset = inb( 0x3CF ) ;
outb( 0x3CE, 0x01 ) ;
VS->Enb_Set_Reset = inb( 0x3CF ) ;
outb( 0x3CE, 0x02 ) ;
VS->Color_Compare = inb( 0x3CF ) ;
outb( 0x3CE, 0x03 ) ;
VS->Data_Rotate = inb( 0x3CF ) ;
outb( 0x3CE, 0x04 ) ;
VS->Read_Map_Select = inb( 0x3CF ) ;
outb( 0x3CE, 0x05 ) ;
VS->Graphics_Mode = inb( 0x3CF ) ;
outb( 0x3CE, 0x06 ) ;
VS->Miscellaneous = inb( 0x3CF ) ;
outb( 0x3CE, 0x07 ) ;
VS->Color_Dont_Care = inb( 0x3CF ) ;
outb( 0x3CE, 0x08 ) ;
VS->Bit_Mask = inb( 0x3CF ) ;		/*	Graphics_Addr == 08   */

return ;
} ;

void restore_ega_state( VS )
register struct video_hardware_state * const VS ;
{
register IO_Address Base_Reg ; /* if mono == 0x3B0, if color == 3D0 */
register IO_Address Target_Reg ;

/* Setup I/O Base Address */
Base_Reg = ( VS->Misc_Output_Reg & 1 ) ? 0x3D0 : 0x3B0 ;

/* Sequencer Registers  03C4 & 03C5 */
outb( 0x3C4, 0x00 ) ;
outb( 0x3C5, VS->Seq_Reset & 0xFD ) ; /* Do Hardware Syncronous RESET */
outb( 0x3C4, 0x01 ) ;
outb( 0x3C5, VS->Clock_Mode ) ;
outb( 0x3C4, 0x02 ) ;
outb( 0x3C5, VS->Mask_Map ) ;
outb( 0x3C4, 0x03 ) ;
outb( 0x3C5, VS->Char_Map_Select ) ;
outb( 0x3C4, 0x04 ) ;
outb( 0x3C5, VS->Memory_Mode ) ;

/* Write General Registers */
outb( 0x3C2, VS->Misc_Output_Reg ) ;
/* VS->Input_Status_0 & VS->Input_Status_1 are READ-ONLY */
outb( Base_Reg + 0xA, VS->Feature_Control ) ;
outb( 0x3C3, VS->Video_Enable ) ;

/* Attribute Registers */
INTS_OFF() ;
/* Initialize Flip-Flop */
{ register tmp = inb( Base_Reg + 0xA ) ; }

/* Target_Reg Used As Scrap */
for ( Target_Reg = 0 ; Target_Reg <= 0xF ; Target_Reg++ ) {
	outb( 0x3C0, Target_Reg ) ;
	outb( 0x3C0, VS->Palette[Target_Reg] ) ;
}
outb( 0x3c0, 0x30 ) ;
outb( 0x3C0, VS->Attr_Mode ) ;
outb( 0x3c0, 0x31 ) ;
outb( 0x3C0, VS->Overscan_Color ) ;
outb( 0x3c0, 0x32 ) ;
outb( 0x3C0, VS->Color_Plane_En ) ;
outb( 0x3c0, 0x33 ) ;
outb( 0x3C0, VS->Horiz_PEL_Pan ) ;
outb( 0x3c0, 0x34 ) ;
outb( 0x3C0, VS->Color_Select ) ;		/*	Attr_Addr_Reg == 14 */
/* Re-Enable Video Access To The Color Palette */
INTS_ON() ;

/* Enable CRT Controller Registers 0-7 */
outb( Target_Reg = ( Base_Reg += 0x4 ), 0x11 ) ;
outb( ++Target_Reg, 0x0C ) ;
/* Restore Crt Controller Registers  03?4 & 03?5 */
outb( Base_Reg, 0x00 ) ;
outb( Target_Reg, VS->Horiz_Total ) ;
outb( Base_Reg, 0x01 ) ;
outb( Target_Reg, VS->Horiz_End ) ;
outb( Base_Reg, 0x02 ) ;
outb( Target_Reg, VS->H_Blank_Start ) ;
outb( Base_Reg, 0x03 ) ;
outb( Target_Reg, VS->H_Blank_End ) ;
outb( Base_Reg, 0x04 ) ;
outb( Target_Reg, VS->H_Retrace_Start ) ;
outb( Base_Reg, 0x05 ) ;
outb( Target_Reg, VS->H_Retrace_End ) ;
outb( Base_Reg, 0x06 ) ;
outb( Target_Reg, VS->Vert_Total ) ;
outb( Base_Reg, 0x07 ) ;
outb( Target_Reg, VS->Overflow ) ;
outb( Base_Reg, 0x08 ) ;
outb( Target_Reg, VS->Preset_Row_Scan ) ;
outb( Base_Reg, 0x09 ) ;
outb( Target_Reg, VS->Max_Scan_Line ) ;
outb( Base_Reg, 0x0A ) ;
outb( Target_Reg, VS->Cursor_Start ) ;
outb( Base_Reg, 0x0B ) ;
outb( Target_Reg, VS->Cursor_End ) ;
outb( Base_Reg, 0x0C ) ;
outb( Target_Reg, VS->Start_Addr_Hi ) ;
outb( Base_Reg, 0x0D ) ;
outb( Target_Reg, VS->Start_Addr_Lo ) ;
outb( Base_Reg, 0x0E ) ;
outb( Target_Reg, VS->Cursor_Loc_Hi ) ;
outb( Base_Reg, 0x0F ) ;
outb( Target_Reg, VS->Cursor_Loc_Lo ) ;
outb( Base_Reg, 0x10 ) ;
outb( Target_Reg, VS->V_Retrace_Start ) ;
outb( Base_Reg, 0x11 ) ;
outb( Target_Reg, VS->V_Retrace_End ) ;
outb( Base_Reg, 0x12 ) ;
outb( Target_Reg, VS->V_Display_End ) ;
outb( Base_Reg, 0x13 ) ;
outb( Target_Reg, VS->Underline_Loc ) ;
outb( Base_Reg, 0x14 ) ;
outb( Target_Reg, VS->Offset ) ;
outb( Base_Reg, 0x15 ) ;
outb( Target_Reg, VS->V_Blank_Start ) ;
outb( Base_Reg, 0x16 ) ;
outb( Target_Reg, VS->V_Blank_End ) ;
outb( Base_Reg, 0x17 ) ;
outb( Target_Reg, VS->CRTC_Mode ) ;
outb( Base_Reg, 0x18 ) ;
outb( Target_Reg, VS->Line_Compare ) ;

/* Restore Graphics Registers  03CE & 03CF */
outb( 0x3CE, 0x00 ) ;
outb( 0x3CF, VS->Set_Reset ) ;
outb( 0x3CE, 0x01 ) ;
outb( 0x3CF, VS->Enb_Set_Reset ) ;
outb( 0x3CE, 0x02 ) ;
outb( 0x3CF, VS->Color_Compare ) ;
outb( 0x3CE, 0x03 ) ;
outb( 0x3CF, VS->Data_Rotate ) ;
outb( 0x3CE, 0x04 ) ;
outb( 0x3CF, VS->Read_Map_Select ) ;
outb( 0x3CE, 0x05 ) ;
outb( 0x3CF, VS->Graphics_Mode ) ;
outb( 0x3CE, 0x06 ) ;
outb( 0x3CF, VS->Miscellaneous ) ;
outb( 0x3CE, 0x07 ) ;
outb( 0x3CF, VS->Color_Dont_Care ) ;
outb( 0x3CE, 0x08 ) ;
outb( 0x3CF, VS->Bit_Mask ) ;		/*	Graphics_Addr == 08   */

/* Re-Enable Hardware i.e. Reset Register */
outb( 0x3C4, 0x00 ) ;
outb( 0x3C5, VS->Seq_Reset | 0x03 ) ; /* Do Enable Hardware Reset Register */

return ;
}
