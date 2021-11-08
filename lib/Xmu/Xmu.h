/*
 * $XConsortium: Xmu.h,v 1.11 88/10/21 17:44:07 jim Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_H_
#define _XMU_H_

#include <X11/Intrinsic.h>

typedef struct _AtomRec* AtomPtr;

#ifdef  _Atoms_c_
#define globalref
#else
#define globalref extern
#endif /*_Atoms_c_*/

globalref AtomPtr
    _XA_TEXT, _XA_TIMESTAMP, _XA_LIST_LENGTH, _XA_LENGTH,
    _XA_TARGETS, _XA_CHARACTER_POSITION, _XA_DELETE, _XA_HOSTNAME,
    _XA_IP_ADDRESS, _XA_DECNET_ADDRESS, _XA_USER, _XA_CLASS,
    _XA_NAME, _XA_CLIENT_WINDOW, _XA_ATOM_PAIR, _XA_SPAN,
    _XA_NET_ADDRESS, _XA_NULL, _XA_FILENAME, _XA_OWNER_OS, _XA_CLIPBOARD;

/*
 * types and constants
 */
typedef enum {
    XtJustifyLeft,       /* justify text to left side of button   */
    XtJustifyCenter,     /* justify text in center of button      */
    XtJustifyRight       /* justify text to right side of button  */
} XtJustify;

typedef enum {XtorientHorizontal, XtorientVertical} XtOrientation;

#define XtNbackingStore		"backingStore"

#define XtCBackingStore		"BackingStore"

#define XtRBackingStore		"BackingStore"

/* BackingStore constants */
#define XtEnotUseful		"notUseful"
#define XtEwhenMapped		"whenMapped"
#define XtEalways		"always"
#define XtEdefault		"default"

/* Justify constants */
#define XtEleft			"left"
#define XtEcenter		"center"
#define XtEright		"right"

/*
 * public entry points
 */
Boolean XmuConvertStandardSelection( /* Widget, Time, Atom*, ... */ );
void XmuCopyISOLatin1Lowered();
void XmuDrawRoundedRectangle();
Pixmap XmuCreatePixmapFromBitmap();
void XmuCvtFunctionToCallback();
void XmuCvtStringToBackingStore();
void XmuCvtStringToCursor();
void XmuCvtStringToJustify();
void XmuCvtStringToOrientation();
void XmuCvtStringToPixmap();
void XmuCvtStringToWidget();
AtomPtr XmuMakeAtom( /* char* */ );
Atom XmuInternAtom( /* Display*, AtomPtr */ );
void XmuInternStrings( /* Display*, String*, Cardinal, Atom* */);
char *XmuGetAtomName( /* Display*, Atom */ );
char *XmuNameOfAtom( /* AtomPtr */ );
int XmuReadBitmapDataFromFile();
int XmuPrintDefaultErrorMessage();

#ifndef _Atoms_c_
#define XA_ATOM_PAIR(d)		XmuInternAtom(d, _XA_ATOM_PAIR)
#define XA_CHARACTER_POSITION(d) XmuInternAtom(d, _XA_CHARACTER_POSITION)
#define XA_CLASS(d)		XmuInternAtom(d, _XA_CLASS)
#define XA_CLIENT_WINDOW(d)	XmuInternAtom(d, _XA_CLIENT_WINDOW)
#define XA_CLIPBOARD(d)		XmuInternAtom(d, _XA_CLIPBOARD)
#define XA_DECNET_ADDRESS(d)	XmuInternAtom(d, _XA_DECNET_ADDRESS)
#define XA_DELETE(d)		XmuInternAtom(d, _XA_DELETE)
#define XA_FILENAME(d)		XmuInternAtom(d, _XA_FILENAME)
#define XA_HOSTNAME(d)		XmuInternAtom(d, _XA_HOSTNAME)
#define XA_IP_ADDRESS(d)	XmuInternAtom(d, _XA_IP_ADDRESS)
#define XA_LENGTH(d)		XmuInternAtom(d, _XA_LENGTH)
#define XA_LIST_LENGTH(d)	XmuInternAtom(d, _XA_LIST_LENGTH)
#define XA_NAME(d)		XmuInternAtom(d, _XA_NAME)
#define XA_NET_ADDRESS(d)	XmuInternAtom(d, _XA_NET_ADDRESS)
#define XA_NULL(d)		XmuInternAtom(d, _XA_NULL)
#define XA_OWNER_OS(d)		XmuInternAtom(d, _XA_OWNER_OS)
#define XA_SPAN(d)		XmuInternAtom(d, _XA_SPAN)
#define XA_TARGETS(d)		XmuInternAtom(d, _XA_TARGETS)
#define XA_TEXT(d)		XmuInternAtom(d, _XA_TEXT)
#define XA_TIMESTAMP(d)		XmuInternAtom(d, _XA_TIMESTAMP)
#define XA_USER(d)		XmuInternAtom(d, _XA_USER)
#endif /*_Atoms_c_*/

#endif /* _XMU_H_ */

