#ifndef lint
static char rcsid[] = "$XConsortium: xcutsel.c,v 1.4 88/10/17 14:02:57 swick Exp $";
#endif	lint

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Command.h>
#include <X11/Box.h>
#include <X11/Cardinals.h>
#include <X11/Xmu.h>
#include <X11/Xatom.h>


static XrmOptionDescRec options[] = {
    {"-selection", "selection",	XrmoptionSepArg, NULL},
    {"-select",    "selection",	XrmoptionSepArg, NULL},
    {"-sel",	   "selection",	XrmoptionSepArg, NULL},
    {"-s",	   "selection",	XrmoptionSepArg, NULL},
    {"-cutbuffer", "cutBuffer",	XrmoptionSepArg, NULL},
};


struct _app_resources {
    String  selection_name;
    int	    buffer;
    Atom    selection;
    char*   value;
    int     length;
} app_resources;

static XtResource resources[] = {
    {"selection", "Selection", XtRString, sizeof(String),
       XtOffset(struct _app_resources*, selection_name), XtRString, "PRIMARY"},
    {"cutBuffer", "CutBuffer", XtRInt, sizeof(int),
       XtOffset(struct _app_resources*, buffer), XtRImmediate, (caddr_t)0},
};

typedef struct {
    Widget button;
    Boolean is_on;
} ButtonState;

static ButtonState state;

Syntax(call)
	char *call;
{
    fprintf( stderr, "Usage: %s [-selection <name>] [-buffer <number>]\n", call );
}


static void StoreBuffer(w, client_data, selection, type, value, length, format)
    Widget w;
    caddr_t client_data;
    Atom *selection, *type;
    caddr_t value;
    unsigned long *length;
    int *format;
{

    if (*type == 0 /*XT_CONVERT_FAIL*/ || *length == 0) {
	XBell( XtDisplay(w), 0 );
	return;
    }

    XStoreBuffer( XtDisplay(w), (char*)value, (int)(*length),
		  app_resources.buffer );
   
    XtFree(value);
}


static Boolean ConvertSelection(w, selection, target,
				type, value, length, format)
    Widget w;
    Atom *selection, *target, *type;
    caddr_t *value;
    unsigned long *length;
    int *format;
{
    Display* d = XtDisplay(w);

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection(w, CurrentTime, selection, target, type,
				   (caddr_t*)&std_targets, &std_length, format);
	*value = XtMalloc(sizeof(Atom)*(std_length + 4));
	targetP = *(Atom**)value;
	*length = std_length + 4;
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*targetP++ = XA_LENGTH(d);
	*targetP++ = XA_LIST_LENGTH(d);
/*
	*targetP++ = XA_CHARACTER_POSITION(d);
*/
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }
    if (*target == XA_STRING || *target == XA_TEXT(d)) {
	*type = XA_STRING;
	*value = app_resources.value;
	*length = app_resources.length;
	*format = 8;
	return True;
    }
    if (*target == XA_LIST_LENGTH(d)) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = 1;
	else {
	    long temp = 1;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
    if (*target == XA_LENGTH(d)) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = app_resources.length;
	else {
	    long temp = app_resources.length;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
#ifdef notdef
    if (*target == XA_CHARACTER_POSITION(d)) {
	*value = XtMalloc(8);
	(*(long**)value)[0] = ctx->text.s.left + 1;
	(*(long**)value)[1] = ctx->text.s.right;
	*type = XA_SPAN(d);
	*length = 2;
	*format = 32;
	return True;
    }
#endif /*notdef*/

    if (XmuConvertStandardSelection(w, CurrentTime, selection, target, type,
				    value, length, format))
	return True;

    /* else */
    return False;
}


static void SetButton(state, on)
    ButtonState *state;
    Boolean on;
{
    if (state->is_on != on) {
	Arg args[2];
	Pixel fg, bg;
	XtSetArg( args[0], XtNforeground, &fg );
	XtSetArg( args[1], XtNbackground, &bg );
	XtGetValues( state->button, args, TWO );
	args[0].value = (XtArgVal)bg;
	args[1].value = (XtArgVal)fg;
	XtSetValues( state->button, args, TWO );
	state->is_on = on;
    }
}


static void LoseSelection(w, selection)
    Widget w;
    Atom *selection;
{
    XtFree( app_resources.value );
    app_resources.value = NULL;
    SetButton(&state, False);
}


/* ARGSUSED */
static void Quit(w, closure, callData)
    Widget w;
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    XtCloseDisplay( XtDisplay(w) );
    exit(0);
}


/* ARGSUSED */
static void GetSelection(w, closure, callData)
    Widget w;
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    XtGetSelectionValue(w, app_resources.selection, XA_STRING,
			StoreBuffer, NULL, CurrentTime);
}


/* ARGSUSED */
static void GetBuffer(w, closure, callData)
    Widget w;
    caddr_t closure;
    caddr_t callData;		/* unused */
{
    XtFree( app_resources.value );
    app_resources.value =
	XFetchBuffer(XtDisplay(w), &app_resources.length, app_resources.buffer);
    if (app_resources.value != NULL) {
	if (XtOwnSelection(w, app_resources.selection, CurrentTime,
			   ConvertSelection, LoseSelection, NULL))
	    SetButton((ButtonState*)closure, True);
    }
}


void main(argc, argv)
    unsigned int argc;
    char **argv;
{
    char label[100];
    Widget box, button;
    Widget shell =
	XtInitialize( "xcutsel", "XCutsel", options, XtNumber(options),
		      &argc, argv );
    XrmDatabase rdb = XtDatabase(XtDisplay(shell));

    if (argc != 1) Syntax(argv[0]);

    XtGetApplicationResources( shell, (caddr_t)&app_resources,
			       resources, XtNumber(resources),
			       NULL, ZERO );

    app_resources.value = NULL;
    XmuInternStrings( XtDisplay(shell), &app_resources.selection_name, ONE,
		      &app_resources.selection );

    box = XtCreateManagedWidget("box", boxWidgetClass, shell, NULL, ZERO);

    button =
	XtCreateManagedWidget("quit", commandWidgetClass, box, NULL, ZERO);
	XtAddCallback( button, XtNcallback, Quit, NULL );

    /* %%% hack alert... */
    sprintf(label, "*label:copy %s to %d",
	    app_resources.selection_name,
	    app_resources.buffer);
    XrmPutLineResource( &rdb, label );

    button =
	XtCreateManagedWidget("sel-cut", commandWidgetClass, box, NULL, ZERO);
	XtAddCallback( button, XtNcallback, GetSelection, NULL );

    sprintf(label, "*label:copy %d to %s",
	    app_resources.buffer,
	    app_resources.selection_name);
    XrmPutLineResource( &rdb, label );

    button =
	XtCreateManagedWidget("cut-sel", commandWidgetClass, box, NULL, ZERO);
	XtAddCallback( button, XtNcallback, GetBuffer, (caddr_t)&state );
 	state.button = button;
	state.is_on = False;
   
    XtRealizeWidget(shell);
    XtMainLoop();
}
