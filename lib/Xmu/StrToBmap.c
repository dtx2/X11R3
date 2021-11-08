#ifndef lint
static char rcsid[] = "$XConsortium: StrToBmap.c,v 1.1 88/09/09 14:56:38 swick Exp $";
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
#include	<sys/param.h>		/* just to get MAXPATHLEN */
#include	"Xmu.h"

/*
 * XmuConvertStringToPixmap:
 *
 * creates a depth-1 Pixmap suitable for window manager icons.
 * "string" represents a bitmap(1) filename which may be absolute,
 * or relative to the global resource bitmapFilePath, class
 * BitmapFilePath.  If the resource is not defined, the default
 * value is the build symbol BITMAPDIR.
 *
 * shares lots of code with XmuConvertStringToCursor.  
 *
 * To use, include the following in your ClassInitialize procedure:

static XtConvertArgRec screenConvertArg[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof(Screen *)}
};

    XtAddConverter("String", "Pixmap", XmuCvtStringToPixmap,
		   screenConvertArg, XtNumber(screenConvertArg));
 *
 */

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps"
#endif

/*ARGSUSED*/
void XmuCvtStringToPixmap(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Pixmap pixmap;
    char *name = (char *)fromVal->addr;
    Screen *screen;
    static char* bitmap_file_path = NULL;
    char filename[MAXPATHLEN];
    static XColor bgColor = {0, 0, 0, 0};
    static XColor fgColor = {0, ~0, ~0, ~0};
    int width, height, xhot, yhot;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToPixmap","XtToolkitError",
             "String to pixmap conversion needs screen argument",
              (String *)NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);

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
			 filename, &width, &height, &pixmap, &xhot, &yhot )
	!= BitmapSuccess) {
	XtStringConversionWarning( name, "Pixmap" );
	return;
    }

    done(&pixmap, Pixmap);
}
