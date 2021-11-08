#ifndef lint
static char Xrcsid[] = "$XConsortium: AsciiText.c,v 1.18 88/10/18 12:28:26 swick Exp $";
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

#include <X11/copyright.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/AsciiTextP.h>

/* from Text.c */

extern void ForceBuildLineTable(); /* in Text.c */

static XtResource string_resources[] = {
  {XtNstring, XtCString, XtRString, sizeof(String),
     XtOffset(AsciiStringWidget, ascii_string.string), XtRString, NULL}
};

static XtResource disk_resources[] = {
  {XtNfile, XtCFile, XtRString, sizeof(String),
     XtOffset(AsciiDiskWidget, ascii_disk.file_name), XtRString, NULL}
};

static void StringClassInitialize(), StringInitialize(),
    StringCreateSourceSink(), StringDestroy();
static Boolean StringSetValues();

static void DiskClassInitialize(), DiskInitialize(),
    DiskCreateSourceSink(), DiskDestroy();
static Boolean DiskSetValues();

AsciiStringClassRec asciiStringClassRec = {
  { /* core fields */
    /* superclass       */      (WidgetClass) &textClassRec,
    /* class_name       */      "Text",
    /* widget_size      */      sizeof(AsciiStringRec),
    /* class_initialize */      StringClassInitialize,
    /* class_part_init  */	NULL,
    /* class_inited     */      FALSE,
    /* initialize       */      StringInitialize,
    /* initialize_hook  */	StringCreateSourceSink,
    /* realize          */      XtInheritRealize,
    /* actions          */      textActionsTable,
    /* num_actions      */      0,
    /* resources        */      string_resources,
    /* num_ resource    */      XtNumber(string_resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      FALSE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      StringDestroy,
    /* resize           */      XtInheritResize,
    /* expose           */      XtInheritExpose,
    /* set_values       */      StringSetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost*/	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */      XtInheritAcceptFocus,
    /* version          */	XtVersion,
    /* callback_private */      NULL,
    /* tm_table         */      XtInheritTranslations,
    /* query_geometry	*/	XtInheritQueryGeometry
  },
  { /* text fields */
    /* empty            */      0
  },
  { /* ascii_string fields */
    /* empty            */      0
  }
};

AsciiDiskClassRec asciiDiskClassRec = {
  { /* core fields */
    /* superclass       */      (WidgetClass) &textClassRec,
    /* class_name       */      "Text",
    /* widget_size      */      sizeof(AsciiDiskRec),
    /* class_initialize */      DiskClassInitialize,
    /* class_part_init  */	NULL,
    /* class_inited     */      FALSE,
    /* initialize       */      DiskInitialize,
    /* initialize_hook  */	DiskCreateSourceSink,
    /* realize          */      XtInheritRealize,
    /* actions          */      textActionsTable,
    /* num_actions      */      0,
    /* resources        */      disk_resources,
    /* num_ resource    */      XtNumber(disk_resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      FALSE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      DiskDestroy,
    /* resize           */      XtInheritResize,
    /* expose           */      XtInheritExpose,
    /* set_values       */      DiskSetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost*/	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */      XtInheritAcceptFocus,
    /* version          */	XtVersion,
    /* callback_private */      NULL,
    /* tm_table         */      XtInheritTranslations,
    /* query_geometry	*/	XtInheritQueryGeometry
  },
  { /* text fields */
    /* empty            */      0
  },
  { /* ascii_disk fields */
    /* empty            */      0
  }
};


WidgetClass asciiStringWidgetClass = (WidgetClass)&asciiStringClassRec;
WidgetClass asciiDiskWidgetClass = (WidgetClass)&asciiDiskClassRec;


static void StringClassInitialize()
{
    asciiStringClassRec.core_class.num_actions = textActionsTableCount;
}


/* ARGSUSED */
static void StringInitialize(request, new)
    Widget request, new;
{
    /* superclass Initialize can't set the following,
     * as it didn't know the source or sink when it was called */
    if (request->core.height == DEFAULT_TEXT_HEIGHT)
	new->core.height = DEFAULT_TEXT_HEIGHT;
}

static void StringCreateSourceSink(widget, args, num_args)
    Widget widget;
    ArgList args;
    Cardinal *num_args;
{
    AsciiStringWidget w = (AsciiStringWidget)widget;
    void (*NullProc)() = NULL;	/* some compilers require this */

    w->text.source = XtStringSourceCreate( widget, args, *num_args );
    w->text.sink = XtAsciiSinkCreate( widget, args, *num_args );

    if (w->core.height == DEFAULT_TEXT_HEIGHT)
        w->core.height = (2*yMargin) + 2
			  + (*w->text.sink->MaxHeight)(widget, 1);

    w->text.lastPos = /* GETLASTPOS */
      (*w->text.source->Scan) ( w->text.source, 0, XtstAll,
			        XtsdRight, 1, TRUE );

    if (w->text.sink->SetTabs != NullProc) {
#define TAB_COUNT 32
	int i;
	Position tabs[TAB_COUNT], tab;

	for (i=0, tab=0; i<TAB_COUNT;i++) {
	    tabs[i] = (tab += 8);
	}
	(w->text.sink->SetTabs) (widget, w->text.leftmargin, TAB_COUNT, tabs);
#undef TAB_COUNT
    }

    ForceBuildLineTable( (TextWidget)w );
}


/* ARGSUSED */
static Boolean StringSetValues(current, request, new)
    Widget current, request, new;
{
    AsciiStringWidget old = (AsciiStringWidget)current;
    AsciiStringWidget w = (AsciiStringWidget)new;

    if (w->ascii_string.string != old->ascii_string.string)
        XtError( "SetValues on AsciiStringWidget string not supported." );

    return False;
}


static void StringDestroy(w)
    Widget w;
{
    XtStringSourceDestroy( ((AsciiStringWidget)w)->text.source );
    XtAsciiSinkDestroy( ((AsciiStringWidget)w)->text.sink );
}


static void DiskClassInitialize()
{
    asciiDiskClassRec.core_class.num_actions = textActionsTableCount;
}


/* ARGSUSED */
static void DiskInitialize(request, new)
    Widget request, new;
{
    /* superclass Initialize can't set the following,
     * as it didn't know the source or sink when it was called */
    if (request->core.height == DEFAULT_TEXT_HEIGHT)
	new->core.height = DEFAULT_TEXT_HEIGHT;
}

static void DiskCreateSourceSink(widget, args, num_args)
    Widget widget;
    ArgList args;
    Cardinal *num_args;
{
    AsciiDiskWidget w = (AsciiDiskWidget)widget;
    void (*NullProc)() = NULL;	/* some compilers require this */

    w->text.source = XtDiskSourceCreate( widget, args, *num_args );
    w->text.sink = XtAsciiSinkCreate( widget, args, *num_args );

    w->text.lastPos = /* GETLASTPOS */
      (*w->text.source->Scan) ( w->text.source, 0, XtstAll,
			        XtsdRight, 1, TRUE );

    if (w->core.height == DEFAULT_TEXT_HEIGHT)
        w->core.height = (2*yMargin) + 2
			  + (*w->text.sink->MaxHeight)(widget, 1);

    if (w->text.sink->SetTabs != NullProc) {
#define TAB_COUNT 32
	int i;
	Position tabs[TAB_COUNT], tab;

	for (i=0, tab=0; i<TAB_COUNT;i++) {
	    tabs[i] = (tab += 8);
	}
	(w->text.sink->SetTabs) (widget, w->text.leftmargin, TAB_COUNT, tabs);
#undef TAB_COUNT
    }

    ForceBuildLineTable( (TextWidget)w );
}


/* ARGSUSED */
static Boolean DiskSetValues(current, request, new)
    Widget current, request, new;
{
    AsciiDiskWidget old = (AsciiDiskWidget)current;
    AsciiDiskWidget w = (AsciiDiskWidget)new;

    if (w->ascii_disk.file_name != old->ascii_disk.file_name)
        XtError( "SetValues on AsciiDiskWidget file not supported." );

    return False;
}


static void DiskDestroy(w)
    Widget w;
{
    XtDiskSourceDestroy( ((AsciiDiskWidget)w)->text.source );
    XtAsciiSinkDestroy( ((AsciiDiskWidget)w)->text.sink );
}
