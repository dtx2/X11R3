#ifndef lint
static char Xrcsid[] = "$XConsortium: Converters.c,v 1.36 88/10/18 11:22:55 swick Exp $";
/* $oHeader: Converters.c,v 1.6 88/09/01 09:26:23 asente Exp $ */
#endif lint
/*LINTLIBRARY*/

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

/* Conversion.c - implementations of resource type conversion procs */

#include	"StringDefs.h"
#include	<stdio.h>
#include        <X11/cursorfont.h>
#include	"IntrinsicI.h"
#include	"Quarks.h"

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

void XtStringConversionWarning(from, toType)
    String from, toType;
{
#ifdef notdef
    static enum {Check, Report, Ignore} report_it = Check;

    /* %%% aarrgghh.  We really want an app context handle! */
    if (report_it == Check && _XtDefaultAppContext()->list[0] != NULL) {
	XrmDatabase rdb = XtDatabase(_XtDefaultAppContext()->list[0]);
	static void CvtStringToBoolean();
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;
	xrm_name[0] = StringToName( "stringConversionWarnings" );
	xrm_name[1] = NULL;
	xrm_class[0] = StringToClass( "StringConversionWarnings" );
	xrm_class[1] = NULL;
	if (XrmQGetResource( rdb, xrm_name, xrm_class,
			     &rep_type, &value ))
	{
	    if (rep_type == StringToQuark(XtRBoolean) && value.addr)
		report_it = Report;
	    else if (rep_type == StringToQuark(XtRString)) {
		XrmValue toVal;
		XtDirectConvert(CvtStringToBoolean, NULL, 0, &value, &toVal);
		if (toVal.addr && *(Boolean*)toVal.addr)
		    report_it = Report;
	    }
	}
    }

    if (report_it == Report) {
#endif /*notdef*/
	String params[2];
	Cardinal num_params = 2;
	params[0] = from;
	params[1] = toType;
	XtWarningMsg("conversionError","string","XtToolkitError",
		   "Cannot convert string \"%s\" to type %s",
		    params,&num_params);
#ifdef notdef
    }
#endif /*notdef*/
}

static void CvtXColorToPixel();
static void CvtIntToBoolean();
static void CvtIntToBool();
static void CvtIntToPixmap();
static void CvtIntToFont();
static void CvtIntOrPixelToXColor();
static void CvtIntToPixel();

static void CvtStringToBoolean();
static void CvtStringToBool();
static void CvtStringToCursor();
static void CvtStringToDisplay();
static void CvtStringToFile();
static void CvtStringToFont();
static void CvtStringToFontStruct();
static void CvtStringToGeometry();
static void CvtStringToInt();
static void CvtStringToShort();
static void CvtStringToUnsignedChar();
static void CvtStringToPixel();

/*ARGSUSED*/
static void CvtIntToBoolean(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Boolean	b;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtIntToBoolean","XtToolkitError",
                  "Integer to Boolean conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    b = (*(int *)fromVal->addr != 0);
    done(&b, Boolean);
};


/*ARGSUSED*/
static void CvtIntToShort(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static short    s;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtIntToShort","XtToolkitError",
                  "Integer to Short conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    s = (*(int *)fromVal->addr);
    done(&s, short);
};


/*ARGSUSED*/
static void CvtStringToBoolean(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Boolean b;
    XrmQuark	q;
    char	lowerName[1000];

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtStringToBoolean","XtToolkitError",
                  "String to Boolean conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);

    LowerCase((char *) fromVal->addr, lowerName);
    q = XrmStringToQuark(lowerName);

    if (q == XtQEtrue || q == XtQEon || q == XtQEyes) {
	b = TRUE;
	done(&b, Boolean);
	return;
    }
    if (q == XtQEfalse || q ==XtQEoff || q == XtQEno) {
	b = FALSE;
	done(&b, Boolean);
	return;
    }

    XtStringConversionWarning((char *) fromVal->addr, "Boolean");
};


/*ARGSUSED*/
static void CvtIntToBool(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Bool	b;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtIntToBool","XtToolkitError",
                  "Integer to Bool conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);
    b = (*(int *)fromVal->addr != 0);
    done(&b, Bool);
};


/*ARGSUSED*/
static void CvtStringToBool(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Bool	b;
    XrmQuark	q;
    char	lowerName[1000];

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtStringToBool",
		"XtToolkitError",
                 "String to Bool conversion needs no extra arguments",
                  (String *)NULL, (Cardinal *)NULL);

    LowerCase((char *) fromVal->addr, lowerName);
    q = XrmStringToQuark(lowerName);

    if (q == XtQEtrue || q == XtQEon || q == XtQEyes) {
	b = TRUE;
	done(&b, Bool);
	return;
    }
    if (q == XtQEfalse || q ==XtQEoff || q == XtQEno) {
	b = FALSE;
	done(&b, Bool);
	return;
    }

    XtStringConversionWarning((char *) fromVal->addr, "Bool");
};

XtConvertArgRec colorConvertArgs[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen),  sizeof(Screen *)},
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.colormap),sizeof(Colormap)}
};


static void CvtIntOrPixelToXColor(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{    
    static XColor   c;
    Screen	    *screen;
    Colormap	    colormap;

    if (*num_args != 2)
      XtErrorMsg("wrongParameters","cvtIntOrPixelToXColor","XtToolkitError",
         "Pixel to color conversion needs screen and colormap arguments",
          (String *)NULL, (Cardinal *)NULL);
    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);
    c.pixel = *(int *)fromVal->addr;

    XQueryColor(DisplayOfScreen(screen), colormap, &c);
    done(&c, XColor);
};


/*ARGSUSED*/
static void CvtStringToPixel(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XColor   screenColor;
    XColor	    exactColor;
    Screen	    *screen;
    XtPerDisplay    perDpy;
    XtAppContext    app;
    Colormap	    colormap;
    Status	    status;
    char	    message[1000];
    XrmQuark	    q;
    String          params[1];
    Cardinal       num_params=1;

    if (*num_args != 2)
     XtErrorMsg("wrongParameters","cvtStringToPixel","XtToolkitError",
       "String to pixel conversion needs screen and colormap arguments",
        (String *)NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);
    perDpy = _XtGetPerDisplay(DisplayOfScreen(screen));
    app = perDpy->appContext;
    colormap = *((Colormap *) args[1].addr);

    LowerCase((char *) fromVal->addr, message);
    q = XrmStringToQuark(message);

    if (q == XtQExtdefaultbackground) {
	if (app->rv) { done(&screen->black_pixel, Pixel); return; }
	else { done(&screen->white_pixel, Pixel); return; }
    }
    if (q == XtQExtdefaultforeground) {
	if (app->rv) { done(&screen->white_pixel, Pixel); return; }
        else { done(&screen->black_pixel, Pixel); return; }
    }

    if ((char) fromVal->addr[0] == '#') {  /* some color rgb definition */

        status = XParseColor(DisplayOfScreen(screen), colormap,
                 (String) fromVal->addr, &screenColor);

        if (status != 0)
           status = XAllocColor(DisplayOfScreen(screen), colormap,
                                &screenColor);
    } else  /* some color name */

        status = XAllocNamedColor(DisplayOfScreen(screen), colormap,
                                  (String) fromVal->addr, &screenColor,
				  &exactColor);
    if (status == 0) {
       params[0]=(String)fromVal->addr;
       XtWarningMsg("noColormap","cvtStringToPixel","XtToolkitError",
                 "Cannot allocate colormap entry for \"%s\"",
                  params,&num_params);
    } else {
        done(&(screenColor.pixel), Pixel)
    }

};


XtConvertArgRec screenConvertArg[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof(Screen *)}
};

/*ARGSUSED*/
static void CvtStringToCursor(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;

{
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
#ifdef XC_dotbox
			{"dotbox",		XC_dotbox,		NULL},
#endif
			{"double_arrow",	XC_double_arrow,	NULL},
			{"draft_large",		XC_draft_large,		NULL},
			{"draft_small",		XC_draft_small,		NULL},
			{"draped_box",		XC_draped_box,		NULL},
			{"exchange",		XC_exchange,		NULL},
			{"fleur",		XC_fleur,		NULL},
			{"gobbler",		XC_gobbler,		NULL},
			{"gumby",		XC_gumby,		NULL},
#ifdef XC_hand1
			{"hand1",		XC_hand1,		NULL},
#endif
#ifdef XC_hand2
			{"hand2",		XC_hand2,		NULL},
#endif
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
    char *name = (char *)fromVal->addr;
    register int i;
    Screen	    *screen;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToCursor","XtToolkitError",
             "String to cursor conversion needs screen argument",
              (String *)NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);
    for (i=0, cache=cursor_names; i < XtNumber(cursor_names); i++, cache++ ) {
	if (strcmp(name, cache->name) == 0) {
	    if (!cache->cursor)
		cache->cursor =
		    XCreateFontCursor(DisplayOfScreen(screen), cache->shape );
	    done(&(cache->cursor), Cursor);
	    return;
	}
    }
    XtStringConversionWarning(name, "Cursor");
};


/*ARGSUSED*/
static void CvtStringToDisplay(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Display	*d;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtStringToDisplay","XtToolkitError",
                  "String to Display conversion needs no extra arguments",
                   (String *)NULL, (Cardinal *)NULL);

    d = XOpenDisplay((char *)fromVal->addr);
    if (d != NULL) {
	done(&d, Display);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Display");
    }
};


/*ARGSUSED*/
static void CvtStringToFile(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static FILE	*f;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtStringToFile","XtToolkitError",
                 "String to File conversion needs no extra arguments",
                 (String *) NULL, (Cardinal *)NULL);

    f = fopen((char *)fromVal->addr, "r");
    if (f != NULL) {
	done(&f, FILE);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "File");
    }
};


/*ARGSUSED*/
static void CvtStringToFont(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Font	f;
    Screen	    *screen;
    Display         *display;
    char	    lcfont[1000];
    XrmQuark	    q;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToFont","XtToolkitError",
             "String to font conversion needs screen argument",
              (String *) NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);
    LowerCase((char *) fromVal->addr, lcfont);
    q = XrmStringToQuark(lcfont);

    if (q != XtQExtdefaultfont) {
	f = XLoadFont(DisplayOfScreen(screen), (char *)fromVal->addr);
	if (f != 0) {
	    done(&f, Font);
	    return;
	}
	XtStringConversionWarning((char *) fromVal->addr, "Font");
    }
    /* try and get the default font */

    display   = DisplayOfScreen(screen);

    f = XLoadFont(display,"fixed");

/* this crashes the server for some reason.  I think that it is */
/* supposed to work.  so for now we will use the above line     */
/*  done(&f, DefaultGCOfScreen(screen)->values.font);        */

    if (f != 0) done(&f, Font);
}


/*ARGSUSED*/
static void CvtIntToFont(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtIntToFont","XtToolkitError",
           "Integer to Font conversion needs no extra arguments",
            (String *) NULL, (Cardinal *)NULL);
    done(fromVal->addr, int);
};


/*ARGSUSED*/
static void CvtStringToFontStruct(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XFontStruct	*f;
    Screen	    *screen;
    char	    lcfont[1000];
    XrmQuark	    q;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToFontStruct","XtToolkitError",
             "String to cursor conversion needs screen argument",
              (String *) NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);
    LowerCase((char *) fromVal->addr, lcfont);
    q = XrmStringToQuark(lcfont);

    if (q != XtQExtdefaultfont) {
	f = XLoadQueryFont(DisplayOfScreen(screen), (char *)fromVal->addr);
	if (f != NULL) {
	    done(&f, XFontStruct *);
	    return;
	}
	XtStringConversionWarning((char *) fromVal->addr, "XFontStruct");
    }

    /* try and get the default font */

    /* This still crashes the server... */
/*  
    f = XQueryFont(DisplayOfScreen(screen),
	           DefaultGCOfScreen(screen)->values.font);
*/
    /* ...so we do this instead */

    f = XLoadQueryFont(DisplayOfScreen(screen), "fixed");

    if (f != 0) done(&f, XFontStruct *);
}

/*ARGSUSED*/
static void CvtStringToInt(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static int	i;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtStringToInt","XtToolkitError",
                  "String to Integer conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);
    if (sscanf((char *)fromVal->addr, "%d", &i) == 1) {
	done(&i, int);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Integer");
    }
}

/*ARGSUSED*/
static void CvtStringToShort(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static short i;

    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToShort","XtToolkitError",
          "String to Integer conversion needs no extra arguments",
           (String *) NULL, (Cardinal *)NULL);
    if (sscanf((char *)fromVal->addr, "%hd", &i) == 1) {
        done(&i, short);
    } else {
        XtStringConversionWarning((char *) fromVal->addr, "Short");
    }
}
/*ARGSUSED*/
static void CvtStringToUnsignedChar(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static int i;
    static unsigned char uc;

    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToUnsignedChar","XtToolkitError",
                  "String to Integer conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    if (sscanf((char *)fromVal->addr, "%d", &i) == 1) {

        if ( i < 0 || i > 255 )
            XtStringConversionWarning((char *) fromVal->addr, "Unsigned Char");
        uc = (unsigned char)i;
        done(&uc, unsigned char);
    } else {
        XtStringConversionWarning((char *) fromVal->addr, "Unsigned Char");
    }
}


/*ARGSUSED*/
static void CvtXColorToPixel(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtXColorToPixel","XtToolkitError",
                  "Color to Pixel conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    done(&((XColor *)fromVal->addr)->pixel, int);
};

/*ARGSUSED*/
static void CvtIntToPixel(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtIntToPixel","XtToolkitError",
                  "Integer to Pixel conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    done(fromVal->addr, int);
};

/*ARGSUSED*/
static void CvtIntToPixmap(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtIntToPixmap","XtToolkitError",
                  "Integer to Pixmap conversion needs no extra arguments",
                   (String *) NULL, (Cardinal *)NULL);
    done(fromVal->addr, int);
};


void LowerCase(source, dest)
    register char  *source, *dest;
{
    register char ch;

    for (; (ch = *source) != 0; source++, dest++) {
    	if ('A' <= ch && ch <= 'Z')
	    *dest = ch - 'A' + 'a';
	else
	    *dest = ch;
    }
    *dest = 0;
}

XrmQuark  XtQBoolean;
XrmQuark  XtQBool;
XrmQuark  XtQColor;
XrmQuark  XtQCursor;
XrmQuark  XtQDisplay;
XrmQuark  XtQDimension;
XrmQuark  XtQFile;
XrmQuark  XtQFont;
XrmQuark  XtQFontStruct;
XrmQuark  XtQInt;
XrmQuark  XtQPixel;
XrmQuark  XtQPixmap;
XrmQuark  XtQPointer;
XrmQuark  XtQPosition;
XrmQuark  XtQShort;
XrmQuark  XtQString;
XrmQuark  XtQUnsignedChar;
XrmQuark  XtQWindow;

XrmQuark  XtQEoff;
XrmQuark  XtQEfalse;
XrmQuark  XtQEno;
XrmQuark  XtQEon;
XrmQuark  XtQEtrue;
XrmQuark  XtQEyes;
XrmQuark  XtQEnotUseful;
XrmQuark  XtQEwhenMapped;
XrmQuark  XtQEalways;
XrmQuark  XtQEdefault;

XrmQuark  XtQExtdefaultbackground;
XrmQuark  XtQExtdefaultforeground;
XrmQuark  XtQExtdefaultfont;

static Boolean initialized = FALSE;

void _XtConvertInitialize()
{
    if (initialized) return;
    initialized = TRUE;

/* Representation types */

    XtQBoolean		= XrmStringToQuark(XtRBoolean);
    XtQColor		= XrmStringToQuark(XtRColor);
    XtQCursor		= XrmStringToQuark(XtRCursor);
    XtQDimension	= XrmStringToQuark(XtRDimension);
    XtQDisplay		= XrmStringToQuark(XtRDisplay);
    XtQFile		= XrmStringToQuark(XtRFile);
    XtQFont		= XrmStringToQuark(XtRFont);
    XtQFontStruct	= XrmStringToQuark(XtRFontStruct);
    XtQInt		= XrmStringToQuark(XtRInt);
    XtQBool		= XrmStringToQuark(XtRBool);
    XtQPixel		= XrmStringToQuark(XtRPixel);
    XtQPixmap		= XrmStringToQuark(XtRPixmap);
    XtQPointer		= XrmStringToQuark(XtRPointer);
    XtQPosition		= XrmStringToQuark(XtRPosition);
    XtQShort            = XrmStringToQuark(XtRShort);
    XtQString		= XrmStringToQuark(XtRString);
    XtQUnsignedChar     = XrmStringToQuark(XtRUnsignedChar);
    XtQWindow		= XrmStringToQuark(XtRWindow);

/* Boolean enumeration constants */

    XtQEfalse		= XrmStringToQuark(XtEfalse);
    XtQEno		= XrmStringToQuark(XtEno);
    XtQEoff		= XrmStringToQuark(XtEoff);
    XtQEon		= XrmStringToQuark(XtEon);
    XtQEtrue		= XrmStringToQuark(XtEtrue);
    XtQEyes		= XrmStringToQuark(XtEyes);

/* Default color and font  enumeration constants */

    XtQExtdefaultbackground = XrmStringToQuark(XtExtdefaultbackground);
    XtQExtdefaultforeground = XrmStringToQuark(XtExtdefaultforeground);
    XtQExtdefaultfont	    = XrmStringToQuark(XtExtdefaultfont);
}

_XtAddDefaultConverters(table)
    ConverterTable table;
{
#define Add(from, to, proc, convert_args, num_args) \
    _XtTableAddConverter(table, from, to, (XtConverter) proc, \
	    (XtConvertArgList) convert_args, num_args)

    Add(XtQColor,   XtQPixel,       CvtXColorToPixel,	    NULL, 0);
    Add(XtQInt,     XtQBoolean,     CvtIntToBoolean,	    NULL, 0);
    Add(XtQInt,     XtQBool,        CvtIntToBool,	    NULL, 0);
    Add(XtQInt,	    XtQDimension,   CvtIntToShort,	    NULL, 0);
    Add(XtQInt,     XtQPixel,       CvtIntToPixel,          NULL, 0);
    Add(XtQInt,     XtQPosition,    CvtIntToShort,          NULL, 0);
    Add(XtQInt,     XtQPixmap,      CvtIntToPixmap,	    NULL, 0);
    Add(XtQInt,     XtQFont,        CvtIntToFont,	    NULL, 0);
    Add(XtQInt,     XtQColor,       CvtIntOrPixelToXColor,  
	colorConvertArgs, XtNumber(colorConvertArgs));

    Add(XtQString,  XtQBoolean,     CvtStringToBoolean,     NULL, 0);
    Add(XtQString,  XtQBool,        CvtStringToBool,	    NULL, 0);
    Add(XtQString,  XtQCursor,      CvtStringToCursor,      
	screenConvertArg, XtNumber(screenConvertArg));
    Add(XtQString,  XtQDimension,   CvtStringToShort,       NULL, 0);
    Add(XtQString,  XtQDisplay,     CvtStringToDisplay,     NULL, 0);
    Add(XtQString,  XtQFile,        CvtStringToFile,	    NULL, 0);
    Add(XtQString,  XtQFont,        CvtStringToFont,	    
	screenConvertArg, XtNumber(screenConvertArg));
    Add(XtQString,  XtQFontStruct,  CvtStringToFontStruct,  
	screenConvertArg, XtNumber(screenConvertArg));
    Add(XtQString,  XtQInt,         CvtStringToInt,	    NULL, 0);
    Add(XtQString,  XtQPosition,    CvtStringToShort,       NULL, 0);
    Add(XtQString,  XtQPixel,       CvtStringToPixel,       
	colorConvertArgs, XtNumber(colorConvertArgs));
    Add(XtQString,  XtQShort,       CvtStringToShort,       NULL, 0);
    Add(XtQString,  XtQUnsignedChar,CvtStringToUnsignedChar,NULL, 0);

    Add(XtQPixel,   XtQColor,       CvtIntOrPixelToXColor,  
	colorConvertArgs, XtNumber(colorConvertArgs));

   _XtAddTMConverters(table);
}
