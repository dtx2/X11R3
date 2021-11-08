/***********************************************************
		Copyright IBM Corporation 1987,1988

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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaSuspScr.c,v 1.1 88/10/24 22:22:19 paul Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaSuspScr.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaSuspScr.c,v 1.1 88/10/24 22:22:19 paul Exp $";
#endif

#include "ibmIOArch.h"
#include "vgaVideo.h"
#include "vgaReg.h"
#include "vgaSave.h"

static void
vgaSaveScreenMemory( OnScreenPixels )
unsigned char (*OnScreenPixels)[1024 * 64 * 4] ;
{
register unsigned char *pixelPtr = (unsigned char *) OnScreenPixels ;
register volatile unsigned char *screen_ptr ;
register unsigned int j ;
register unsigned int i ;

/* Setup VGA Registers */
SetVideoGraphicsIndex( Graphics_ModeIndex ) ;
SetVideoGraphicsData( inb( 0x3CF ) & ~0x8 ) ; /* Clear the bit */
SetVideoGraphicsIndex( Read_Map_SelectIndex ) ;

for ( i = 4 ; i-- ; ) {
	screen_ptr = (volatile unsigned char *) VIDEO_MEMORY_BASE ;
	SetVideoGraphicsData( i ) ;
	for ( j = 64 * 1024 ; j-- ; )
		*pixelPtr++ = *( (VgaMemoryPtr) ( screen_ptr++ ) ) ;
}

return ;
}

static void
vgaRestoreScreenMemory( OnScreenPixels )
unsigned char (*OnScreenPixels)[1024 * 64 * 4] ;
{
register unsigned char *pixelPtr = (unsigned char *) OnScreenPixels ;
register volatile unsigned char *screen_ptr ;
register unsigned int j ;
register unsigned int planeMask ;

/* Setup VGA Registers */
SetVideoGraphics( Graphics_ModeIndex, 0 ) ; /* Write Mode 0 */
SetVideoGraphics( Data_RotateIndex, 0 ) ;
SetVideoSequencerIndex( Mask_MapIndex ) ;

for ( planeMask = ~( ~0 << 4 ) ; planeMask ; planeMask >>= 1 ) {
	screen_ptr = (volatile unsigned char *) VIDEO_MEMORY_BASE ;
	outb( SequencerDataRegister, planeMask ) ;
	for ( j = 64 * 1024 ; j-- ; )
		*( (VgaMemoryPtr) ( screen_ptr++ ) ) = *pixelPtr++ ;
}

return ;
}

/* ************************************************************************** */

#define STATE_SAVED	1
#define PALETTE_SAVED	2
#define OFFSCREEN_SAVED	4
#define ONSCREEN_SAVED	8

static struct vga_video_hardware_state SavedRegisterState ;
static DAC_TABLE SavedDACstate ;
static unsigned char (*OnScreenPixels)[1024 * 64 * 4] = 0 ;
static int ScreenIsSaved = 0 ;

void
vgaSuspendScreenAndSave()
{
if ( ScreenIsSaved ) {
	ErrorF( "vga State Is Already Saved!" ) ;
	return ;
}
save_vga_state( &SavedRegisterState ) ;
ScreenIsSaved = STATE_SAVED ;
save_dac( &SavedDACstate ) ;
ScreenIsSaved |= PALETTE_SAVED ;
if ( !OnScreenPixels
  && !( OnScreenPixels = (unsigned char (*)[1024 * 64 * 4])
		 malloc( sizeof *OnScreenPixels ) ) )
	ErrorF( "malloc() for vga OnScreenPixels Failed\n" ) ;
else {
	vgaSaveScreenMemory( OnScreenPixels ) ;
	ScreenIsSaved |= ONSCREEN_SAVED ;
}

return ;
}

void
vgaRestoreScreenAndActivate()
{
if ( !ScreenIsSaved ) {
	ErrorF( "vga State Is Not Already Saved!" ) ;
	return ;
}

set_graphics_mode( &SavedRegisterState ) ;
if ( ScreenIsSaved & PALETTE_SAVED ) {
	restore_dac( &SavedDACstate ) ;
	ScreenIsSaved &= ~PALETTE_SAVED ;
}
if ( ScreenIsSaved & ONSCREEN_SAVED ) {
	vgaRestoreScreenMemory( OnScreenPixels ) ;
	free( OnScreenPixels ) ;
	OnScreenPixels = 0 ;
	ScreenIsSaved &= ~ONSCREEN_SAVED ;
}
if ( ScreenIsSaved & STATE_SAVED ) {
	restore_vga_state( &SavedRegisterState ) ;
	ScreenIsSaved &= ~STATE_SAVED ;
}

return ;
}
