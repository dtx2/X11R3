#include <X11/copyright.h>

/* $XConsortium: AsciiTextP.h,v 1.10 88/10/18 12:25:24 swick Exp $ */


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

#ifndef _AsciiTextP_h
#define _AsciiTextP_h

#include <X11/AsciiText.h>
#include <X11/TextP.h>

extern XtActionsRec textActionsTable[];
extern Cardinal textActionsTableCount;

typedef struct {int empty;} AsciiStringClassPart;

typedef struct _AsciiStringClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
    AsciiStringClassPart ascii_string_class;
} AsciiStringClassRec;

extern AsciiStringClassRec asciiStringClassRec;

typedef struct {
    String	string;		/* string for which to create a source */
} AsciiStringPart;

typedef struct _AsciiStringRec {
    CorePart		core;
    SimplePart		simple;
    TextPart		text;
    AsciiStringPart	ascii_string;
} AsciiStringRec;

typedef struct {int empty;} AsciiDiskClassPart;

typedef struct _AsciiDiskClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
    AsciiDiskClassPart	ascii_disk_class;
} AsciiDiskClassRec;

extern AsciiDiskClassRec asciiDiskClassRec;

typedef struct {
    String	file_name;	/* file for which to create a source */
} AsciiDiskPart;

typedef struct _AsciiDiskRec {
    CorePart		core;
    SimplePart		simple;
    TextPart		text;
    AsciiDiskPart	ascii_disk;
} AsciiDiskRec;

#endif  _AsciiTextP_h
