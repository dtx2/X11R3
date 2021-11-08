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
/*
 *
 *  Hardware interface routines for IBM VGA adapter for
 *  X.11 server( s ) on IBM equipment.
 *
 *  P. Shupak  
 *  Dec. 1987 - Largely Plagurized From ibm8514
 *
 */
/* $Header: vgafont.c,v 6.0 88/08/18 08:57:45 erik Exp $ */
/* $Source: /vice/X11/r3/server/ddx/ibm/vga/RCS/vgafont.c,v $ */

#ifndef lint
static char *rcsid = "$Header: vgafont.c,v 6.0 88/08/18 08:57:45 erik Exp $" ;
#endif

/* Cached Font Manager
 * 
 * Whenever a shifted copy of a glyph is needed.
 *	Check if any chars have been shifted that amount.
 * Create an "in-core" shifted copy of the char for future use.
 *
 * Only cache fonts with chars of up to 1024 pixels.
 * Only cache up to seven fonts fonts.
 * Always draw characters aligned to make life easy on the hardware
 * If no room, leave for Image Glyph Blit
 * When releasing, free up the space
 *
 * P. Shupak 12/87
 *
 */
#include "vga_video.h"

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "dixfont.h"
#include "font.h"
#include "scrnintstr.h"

#include "mfb.h"

#include "vgaFont.h"

vgaFontCacheRec vgaFontCacheEntry ;

extern int vgaNumberOfPlanes ;

void vgaInitFontCache()
{
vgaFontCacheEntry.numInstalled 	= 0 ;
vgaFontCacheEntry.FontList	= NULL ;

return ;
}

Bool vgaRealizeFont( pscr, pFont )
ScreenPtr pscr ;
FontPtr	pFont ;
{
register int		i ;
FontInfoPtr		pfi = pFont->pFI ;
CharInfoPtr		maxb = &pfi->maxbounds ;
int			numchars, firstchar, lastchar ;
vgaCachedFontPtr	ptr, pIFont ;
CharInfoPtr		pCinfo ;

TRACE( ( "vgaRealizeFont: realizing %s\n", pFont->pathname ) ) ;
if ( pfi->lastRow == 0 ) {
	numchars = n1dChars( pfi ) ;
	firstchar = pfi->chFirst ;
	lastchar = pfi->chLast ;
}
else {
/* XXX  This code needs to be looked at more  XXX */
	numchars = n2dChars( pfi ) ;
	pCinfo = pFont->pCI ;
	for ( firstchar = -1, i = 0 ;
	      ( ( i < 0xffff ) && ( firstchar == -1 ) ) ;
	      i++ )
		if ( pCinfo[i].exists )
			firstchar = i ;
	if ( ( i + numchars ) > ( 0xffff ) ) {
		ErrorF( "vgaRealizeFont: bad 16bit realize" ) ;
		return mfbRealizeFont( pscr,pFont ) ;
	}
	lastchar = firstchar + numchars - 1 ;
}

if ( ( GLYPHHEIGHTPIXELS( maxb ) > VGA_MAX_CACHED_GLYPH_HEIGHT )
  || ( GLYPHWIDTHPIXELS( maxb ) > VGA_MAX_CACHED_GLYPH_WIDTH ) ) {
	ErrorF( "Xibm: vga: Font To Large To Cache ..." ) ;
	ErrorF( "Font was %s\n", pFont->pathname ) ;
	return mfbRealizeFont( pscr, pFont ) ;
}

if ( !( pIFont = (vgaCachedFontPtr *) Xalloc( sizeof( vgaCachedFontPtr ) ) ) ) {
	ErrorF( "vgaRealizeFont: Cannot Xalloc for %s\n", pFont->pathname ) ;
	return mfbRealizeFont( pscr, pFont ) ;
}

/* Initialize The Font Descriptor */
pIFont->firstchar = firstchar ;
pIFont->lastchar = lastchar ;
pIFont->numchars = numchars ;
for ( i = VGA_NUM_SHIFTED_ENTRIES ; i-- ; )
	pIFont->ShiftedArray[ i ] = (vgaShiftedGlyphPtr) 0 ;

/* tack the Installed Font into the beginning of the fontlist */
ptr = vgaFontCacheEntry.FontList ;
vgaFontCacheEntry.FontList = pIFont ;
pIFont->NextFont = ptr ;
vgaFontCacheEntry.numInstalled++ ;

/* ok, so tell the world that it's been off-screened */
pFont->devPriv[pscr->myNum] = (pointer) pIFont ; 

return ;
}

Bool vgaUnrealizeFont( pscr, pFont )
ScreenPtr pscr ;
FontPtr	pFont ;
{
register int			i, j ;
register vgaShiftedGlyphPtr	*shiftCharPtr ;
register vgaShiftedGlyphPtr	tPtr ;
vgaCachedFontPtr		currentfont, lastfont, pIFont ;

/* this code depends on knowing what is going on in the mfb.  I am assuming
that the mfbrealizefont routine will only store a 0,1,2 in the devPriv
field, whereas I store a pointer.  In case the mfb grows, I will check
the field to see if it is a mfb number else assuming a pointer
*/
switch ( (long) ( pIFont = (vgaCachedFontPtr) pFont->devPriv[pscr->myNum] ) ) {
	case FT_VARPITCH:
	case FT_FIXPITCH:
	case FT_SMALLPITCH:
		ErrorF( "vgaUnrealizeFont: using mfb\n" ) ;
		return mfbUnrealizeFont( pscr, pFont ) ;
	default :
		break ;
}

/* Must Be Cached "in-core" -- Free All Memory Used */
for ( i = VGA_NUM_SHIFTED_ENTRIES ; i-- ; )
	/* For each Possible Shift */
	if ( shiftCharPtr = pIFont->ShiftedArray[ i ] ) {
		/* For each Possible Character */
		for ( j = pIFont->numchars ; j-- ; shiftCharPtr++ )
			if ( tPtr = *shiftCharPtr )
				Xfree( tPtr ) ;
		Xfree( pIFont->ShiftedArray[ i ] ) ;
	}

/* Patch The Linked List Of Fonts */
if ( ( currentfont = vgaFontCacheEntry.FontList ) == pIFont )
	vgaFontCacheEntry.FontList = pIFont->NextFont ;
else {
	for ( ;
	      currentfont != pIFont ;
	      currentfont = currentfont->NextFont )
		lastfont = currentfont ;
	lastfont->NextFont = pIFont->NextFont ;
}

vgaFontCacheEntry.numInstalled-- ;
Xfree( pIFont ) ;

return TRUE ;
}
