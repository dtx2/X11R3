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
 *  Hardware interface routines for IBM VGA adapter for
 *  X.11 server(s) on IBM equipment.
 *
 *  P. Shupak  
 *  Dec. 1987
 *
 */
/* $Header: vgafont.h,v 6.0 88/08/18 08:57:48 erik Exp $ */
/* $Source: /vice/X11/r3/server/ddx/ibm/vga/RCS/vgafont.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidvgafonts = "$Header: vgafont.h,v 6.0 88/08/18 08:57:48 erik Exp $" ;
#endif

#define VGA_NUM_SHIFTED_ENTRIES		   7
#define VGA_MAX_CACHED_GLYPH_HEIGHT	  64
#define VGA_MAX_CACHED_GLYPH_WIDTH	  32
#define VGA_MAX_CACHED_GLYPH_PIXEL_SIZE	1024

typedef struct vgaCachedShiftedGlyph {
	int		w ;
	int		h ;
	char		frontEdgeMask ;
	char		backEdgeMask ;
	CharInfoPtr	pInfo ;
} vgaGlyphDescriptor, *vgaShiftedGlyphPtr ;

typedef struct vgaCachedFont {
	int			firstchar ;
	int			lastchar ;
	int			numchars ;
	vgaShiftedGlyphPtr	ShiftedArray[ VGA_NUM_SHIFTED_ENTRIES ] ;
	struct vgaCachedFont	*NextFont ;
} vgaCachedFontDescriptor, *vgaCachedFontPtr ;

typedef struct _vgaFontCacheEntry {
	int			numInstalled ;
	int			FirstAvailableLine ;
	int			NumLinesAvailable ;
	vgaCachedFontPtr	FontList ;
} vgaFontCacheRec, *vgaFontCachePtr ;
