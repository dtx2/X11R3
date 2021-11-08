#ifndef lint
static char rcsid[] = "$XConsortium: StrToCurs.c,v 1.3 88/09/22 17:41:06 swick Exp $";
#endif lint


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/cursorfont.h>
#include	<X11/IntrinsicP.h>	/* 'cause CoreP.h needs it */
#include	<X11/CoreP.h>		/* just to do XtConvert() */
#include	<sys/param.h>		/* just to get MAXPATHLEN */
#include	"Xmu.h"

extern void CvtStringToCursor();

/*
 * XmuConvertStringToCursor:
 *
 * allows String to specify a standard cursor name (from cursorfont.h), a
 * font name and glyph index of the form "FONT fontname index [[font] index]", 
 * or a bitmap file name (absolute, or relative to the global resource
 * bitmapFilePath, class BitmapFilePath).  If the resource is not
 * defined, the default value is the build symbol BITMAPDIR.
 *
 * shares lots of code with XmuCvtStringToPixmap, but unfortunately
 * can't use it as the hotspot info is lost.
 *
 * To use, include the following in your ClassInitialize procedure:

static XtConvertArgRec screenConvertArg[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof(Screen *)}
};

    XtAddConverter("String", "Cursor", XmuCvtStringToCursor,      
		   screenConvertArg, XtNumber(screenConvertArg));
 *
 */

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps"
#endif

#define FONTSPECIFIER		"FONT "

/*ARGSUSED*/
void XmuCvtStringToCursor(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Cursor cursor;
    char *name = (char *)fromVal->addr;
    Screen *screen;
    register int i;
    static char* bitmap_file_path = NULL;
    char filename[MAXPATHLEN], maskname[MAXPATHLEN];
    Pixmap source, mask;
    static XColor bgColor = {0, ~0, ~0, ~0};
    static XColor fgColor = {0, 0, 0, 0};
    int width, height, xhot, yhot;

    static struct _CursorName {
	char		*name;
	unsigned int	shape;
	Cursor		cursor;
    } cursor_names[] = {
			{"X_cursor",		XC_X_cursor,		NULL},
			{"arrow",		XC_arrow,		NULL},
			{"based_arrow_down",	XC_based_arrow_down,    NULL},
			{"based_arrow_up",	XC_based_arrow_up,      NULL},
			{"boat",		XC_boat,		NULL},
			{"bogosity",		XC_bogosity,		NULL},
			{"bottom_left_corner",	XC_bottom_left_corner,  NULL},
			{"bottom_right_corner",	XC_bottom_right_corner, NULL},
			{"bottom_side",		XC_bottom_side,		NULL},
			{"bottom_tee",		XC_bottom_tee,		NULL},
			{"box_spiral",		XC_box_spiral,		NULL},
			{"center_ptr",		XC_center_ptr,		NULL},
			{"circle",		XC_circle,		NULL},
			{"clock",		XC_clock,		NULL},
			{"coffee_mug",		XC_coffee_mug,		NULL},
			{"cross",		XC_cross,		NULL},
			{"cross_reverse",	XC_cross_reverse,       NULL},
			{"crosshair",		XC_crosshair,		NULL},
			{"diamond_cross",	XC_diamond_cross,       NULL},
			{"dot",			XC_dot,			NULL},
			{"dotbox",		XC_dotbox,		NULL},
			{"double_arrow",	XC_double_arrow,	NULL},
			{"draft_large",		XC_draft_large,		NULL},
			{"draft_small",		XC_draft_small,		NULL},
			{"draped_box",		XC_draped_box,		NULL},
			{"exchange",		XC_exchange,		NULL},
			{"fleur",		XC_fleur,		NULL},
			{"gobbler",		XC_gobbler,		NULL},
			{"gumby",		XC_gumby,		NULL},
			{"hand1",		XC_hand1,		NULL},
			{"hand2",		XC_hand2,		NULL},
			{"heart",		XC_heart,		NULL},
			{"icon",		XC_icon,		NULL},
			{"iron_cross",		XC_iron_cross,		NULL},
			{"left_ptr",		XC_left_ptr,		NULL},
			{"left_side",		XC_left_side,		NULL},
			{"left_tee",		XC_left_tee,		NULL},
			{"leftbutton",		XC_leftbutton,		NULL},
			{"ll_angle",		XC_ll_angle,		NULL},
			{"lr_angle",		XC_lr_angle,		NULL},
			{"man",			XC_man,			NULL},
			{"middlebutton",	XC_middlebutton,	NULL},
			{"mouse",		XC_mouse,		NULL},
			{"pencil",		XC_pencil,		NULL},
			{"pirate",		XC_pirate,		NULL},
			{"plus",		XC_plus,		NULL},
			{"question_arrow",	XC_question_arrow,	NULL},
			{"right_ptr",		XC_right_ptr,		NULL},
			{"right_side",		XC_right_side,		NULL},
			{"right_tee",		XC_right_tee,		NULL},
			{"rightbutton",		XC_rightbutton,		NULL},
			{"rtl_logo",		XC_rtl_logo,		NULL},
			{"sailboat",		XC_sailboat,		NULL},
			{"sb_down_arrow",	XC_sb_down_arrow,       NULL},
			{"sb_h_double_arrow",	XC_sb_h_double_arrow,   NULL},
			{"sb_left_arrow",	XC_sb_left_arrow,       NULL},
			{"sb_right_arrow",	XC_sb_right_arrow,      NULL},
			{"sb_up_arrow",		XC_sb_up_arrow,		NULL},
			{"sb_v_double_arrow",	XC_sb_v_double_arrow,   NULL},
			{"shuttle",		XC_shuttle,		NULL},
			{"sizing",		XC_sizing,		NULL},
			{"spider",		XC_spider,		NULL},
			{"spraycan",		XC_spraycan,		NULL},
			{"star",		XC_star,		NULL},
			{"target",		XC_target,		NULL},
			{"tcross",		XC_tcross,		NULL},
			{"top_left_arrow",	XC_top_left_arrow,      NULL},
			{"top_left_corner",	XC_top_left_corner,	NULL},
			{"top_right_corner",	XC_top_right_corner,    NULL},
			{"top_side",		XC_top_side,		NULL},
			{"top_tee",		XC_top_tee,		NULL},
			{"trek",		XC_trek,		NULL},
			{"ul_angle",		XC_ul_angle,		NULL},
			{"umbrella",		XC_umbrella,		NULL},
			{"ur_angle",		XC_ur_angle,		NULL},
			{"watch",		XC_watch,		NULL},
			{"xterm",		XC_xterm,		NULL},
    };
    struct _CursorName *cache;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToCursor","XtToolkitError",
             "String to cursor conversion needs screen argument",
              (String *)NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);

    if (0 == strncmp(FONTSPECIFIER, name, strlen(FONTSPECIFIER))) {
	char source_name[MAXPATHLEN], mask_name[MAXPATHLEN];
	int source_char, mask_char, fields;
	WidgetRec widgetRec;
	Font source_font, mask_font;
	XrmValue fromString, toFont;

	fields = sscanf(name, "FONT %s %d %s %d",
			source_name, &source_char,
			mask_name, &mask_char);
	if (fields < 2) {
	    XtStringConversionWarning( name, "Cursor" );
	    return;
	}

	/* widgetRec is stupid; we should just use XtDirectConvert,
	 * but the names in Xt/Converters aren't public. */
	widgetRec.core.screen = screen;
	fromString.addr = source_name;
	fromString.size = strlen(source_name);
	XtConvert(&widgetRec, XtRString, &fromString, XtRFont, &toFont);
	if (toFont.addr == NULL) {
	    XtStringConversionWarning( name, "Cursor" );
	    return;
	}
	source_font = *(Font*)toFont.addr;

	switch (fields) {
	  case 2:		/* defaulted mask font & char */
	    mask_font = source_font;
	    mask_char = source_char;
	    break;

	  case 3:		/* defaulted mask font */
	    mask_font = source_font;
	    mask_char = atoi(mask_name);
	    break;

	  case 4:		/* specified mask font & char */
	    fromString.addr = mask_name;
	    fromString.size = strlen(mask_name);
	    XtConvert(&widgetRec, XtRString, &fromString, XtRFont, &toFont);
	    if (toFont.addr == NULL) {
		XtStringConversionWarning( name, "Cursor" );
		return;
	    }
	    mask_font = *(Font*)toFont.addr;
	}

	cursor = XCreateGlyphCursor( DisplayOfScreen(screen), source_font,
				     mask_font, source_char, mask_char,
				     &fgColor, &bgColor );
	done(&cursor, Cursor);
	return;
    }

    for (i=0, cache=cursor_names; i < XtNumber(cursor_names); i++, cache++ ) {
	if (strcmp(name, cache->name) == 0) {
	    if (!cache->cursor)
		cache->cursor =
		    XCreateFontCursor(DisplayOfScreen(screen), cache->shape );
	    done(&(cache->cursor), Cursor);
	    return;
	}
    }

    /* isn't a standard cursor in cursorfont; try to open a bitmap file */
    if (bitmap_file_path == NULL) {
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;
	xrm_name[0] = XrmStringToName( "bitmapFilePath" );
	xrm_name[1] = NULL;
	xrm_class[0] = XrmStringToClass( "BitmapFilePath" );
	xrm_class[1] = NULL;
	if (XrmQGetResource( XtDatabase(DisplayOfScreen(screen)),
			     xrm_name, xrm_class, &rep_type, &value )
	    && rep_type == XrmStringToQuark(XtRString))
	    bitmap_file_path = value.addr;
	else
	    bitmap_file_path = BITMAPDIR;
    }

    if ( name[0] == '/' || name[0] == '.' )
 	strcpy( filename, name );
    else
	sprintf( filename, "%s/%s", bitmap_file_path, name );

    if (XReadBitmapFile( DisplayOfScreen(screen), RootWindowOfScreen(screen),
			 filename, &width, &height, &source, &xhot, &yhot )
	!= BitmapSuccess) {
	XtStringConversionWarning( name, "Cursor" );
	return;
    }
    (void) strcpy( maskname, filename );
    (void) strcat( maskname, "Mask" );
    if (XReadBitmapFile( DisplayOfScreen(screen), RootWindowOfScreen(screen),
			 maskname, &width, &height, &mask, &width, &height )
	!= BitmapSuccess) {
	(void) strcpy( maskname, filename );
	(void) strcat( maskname, "msk" );
	if (XReadBitmapFile(DisplayOfScreen(screen),RootWindowOfScreen(screen),
			    maskname, &width, &height, &mask, &width, &height )
	    != BitmapSuccess) {
	    mask = None;
	}
    }
    cursor = XCreatePixmapCursor( DisplayOfScreen(screen), source, mask,
				  &fgColor, &bgColor, xhot, yhot );
    XFreePixmap( DisplayOfScreen(screen), source );
    if (mask != None) XFreePixmap( DisplayOfScreen(screen), mask );

    done(&cursor, Cursor);
}
