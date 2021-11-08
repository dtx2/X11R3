/* $XConsortium: xclipboard.c,v 1.2 88/10/18 14:28:15 swick Exp $ */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Form.h>
#include <X11/Command.h>
#include <X11/AsciiText.h>
#include <X11/Xatom.h>
#include <X11/Xmu.h>
#include <X11/Cardinals.h>
#include <sys/param.h>
#include <stdio.h>

static XrmOptionDescRec table[] = {
    {"-w",	"wordWrap",		XrmoptionNoArg,  "on"},
    {"-nw",	"wordWrap",		XrmoptionNoArg,  "off"},
};

typedef struct {
    Boolean word_wrap;
} app_resourceRec, *app_res;

app_resourceRec app_resources;

static XtResource resources[] = {
    {"wordWrap", "WordWrap", XtRBoolean, sizeof(Boolean),
	XtOffset(app_res,word_wrap), XtRImmediate, (caddr_t)False},
};


static void InsertClipboard(w, client_data, selection, type,
			    value, length, format)
    Widget w;
    caddr_t client_data;
    Atom *selection, *type;
    caddr_t value;
    unsigned long *length;
    int *format;
{
    XtTextBlock text;
    Arg args[1];
    XtTextPosition last;

    if (*type == 0 /*XT_CONVERT_FAIL*/ || *length == 0) {
	XBell( XtDisplay(w), 0 );
	return;
    }

#ifdef notdef
    XtSetArg( args[0], XtNlength, &end );
    XtGetValues( w, args, ONE );
#else
    XtTextSetInsertionPoint(w, 9999999);
    last = XtTextGetInsertionPoint(w);
#endif /*notdef*/

    text.ptr = (char*)value;
    text.firstPos = 0;
    text.length = *length;
    text.format = FMT8BIT;

    if (XtTextReplace(w, last, last, &text))
	XBell( XtDisplay(w), 0);
    else {
	XtTextPosition newend;
	XtTextSetInsertionPoint(w, last + text.length);
	newend = XtTextGetInsertionPoint(w);
	if (text.ptr[text.length-1] != '\n') {
	    text.ptr = "\n";
	    text.length = 1;
	    XtTextReplace(w, newend, newend, &text);
	    XtTextSetInsertionPoint(w, newend += 1);
	}
    }
    
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
    TextWidget ctx = (TextWidget)w;

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection(w, CurrentTime, selection, target, type,
				   (caddr_t*)&std_targets, &std_length, format);
	*value = XtMalloc(sizeof(Atom)*(std_length /* + 5 */));
	targetP = *(Atom**)value;
	*length = std_length /* + 5 */;
/*
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*targetP++ = XA_LENGTH(d);
	*targetP++ = XA_LIST_LENGTH(d);
	*targetP++ = XA_CHARACTER_POSITION(d);
*/
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }

#ifdef notdef
    if (*target == XA_STRING || *target == XA_TEXT(d)) {
	*type = XA_STRING;
	*value = _XtTextGetText(ctx, ctx->text.s.left, ctx->text.s.right);
	*length = strlen(*value);
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
	    *(long*)*value = ctx->text.s.right - ctx->text.s.left;
	else {
	    long temp = ctx->text.s.right - ctx->text.s.left;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
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


static void LoseSelection(w, selection)
    Widget w;
    Atom *selection;
{
    XtGetSelectionValue(w, *selection, XA_STRING, InsertClipboard,
			NULL, CurrentTime);

    XtOwnSelection(w, XA_CLIPBOARD(XtDisplay(w)), CurrentTime,
		   ConvertSelection, LoseSelection, NULL);
}


static void Quit(w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
    XtCloseDisplay( XtDisplay(w) );
    exit( 0 );
}


main(argc, argv)
unsigned int argc;
char **argv;
{
    static Arg textArgs[] = {
	{XtNfile, 0},
	{XtNtextOptions, (XtArgVal)scrollVertical },
	{XtNeditType, (XtArgVal)XttextAppend},
	{XtNwidth, 500},
	{XtNheight, 100},
    };
    Widget top, p, w, text;
    char file[MAXPATHLEN];
    FILE *f;

    top = XtInitialize( "xclipboard", "XClipboard", table, XtNumber(table),
			  &argc, argv);

    XtGetApplicationResources(top, &app_resources, resources,
			      XtNumber(resources), NULL, ZERO);

    p = XtCreateManagedWidget("shell", formWidgetClass, top, NULL, ZERO);
    w = XtCreateManagedWidget("quit",  commandWidgetClass, p, NULL, ZERO);
    XtAddCallback(w, XtNcallback, Quit, NULL);
    w = XtCreateManagedWidget("erase", commandWidgetClass, p, NULL, ZERO);
    /*XtAddCallback(w, XtNcallback, Erase, NULL);*/
    XtSetSensitive(w, False);

    (void)tmpnam(file);
    if ((f = fopen(file, "w")) == NULL) {
	perror( argv[0] );
	exit(1);
    }
    fclose(f);

    textArgs[0].value = (XtArgVal)file;
    if (app_resources.word_wrap) textArgs[1].value |= wordBreak;

    text = XtCreateManagedWidget( "text", asciiDiskWidgetClass,
				  p, textArgs, XtNumber(textArgs) );

    XtRealizeWidget(top);
    unlink(file);

    (void)XmuInternAtom( XtDisplay(text), XmuMakeAtom("NULL") ); /* %%% */
    XtOwnSelection(text, XA_CLIPBOARD(XtDisplay(text)), CurrentTime,
		   ConvertSelection, LoseSelection, NULL);

    XtMainLoop();
}
