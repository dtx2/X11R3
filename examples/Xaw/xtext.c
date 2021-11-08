/* $XConsortium: xtext.c,v 1.8 88/10/05 13:14:34 swick Exp $ */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/AsciiText.h>
#include <stdio.h>

static XrmOptionDescRec table[] = {
    {"-file",	"*file",		XrmoptionSepArg, NULL},
    {"-v",	"value",		XrmoptionSepArg, NULL},
    {"-w",	"wordWrap",		XrmoptionNoArg,  "on"},
    {"-nw",	"wordWrap",		XrmoptionNoArg,  "off"},
};

typedef struct {
    Boolean word_wrap;
    char *value;
    char *filename;
} app_resourceRec, *app_res;

app_resourceRec app_resources;

static char default_value[] =
	"This is a\ntest.  If this\nhad been an actual\nemergency...";

static XtResource resources[] = {
    {"value",	 "Value",    XtRString, sizeof(String),
	XtOffset(app_res,value), XtRString, (caddr_t)default_value},
    {"wordWrap", "WordWrap", XtRBoolean, sizeof(Boolean),
	XtOffset(app_res,word_wrap), XtRImmediate, (caddr_t)False},
    {"file",	 "File",     XtRString, sizeof(String),
	XtOffset(app_res,filename), XtRImmediate, NULL},
};

main(argc, argv)
unsigned int argc;
char **argv;
{
    static Arg arglist[] = {
	{XtNstring, (XtArgVal) NULL},
	{XtNeditType, (XtArgVal) XttextEdit},
	{XtNtextOptions, 0},
    };
    Widget toplevel;

    char *index();

    toplevel = XtInitialize("textTest", "Demo", table, XtNumber(table),
			    &argc, argv);

    XtGetApplicationResources(toplevel, &app_resources, resources,
			      XtNumber(resources), NULL, 0);

    arglist[0].value = (XtArgVal)app_resources.value;
    if (app_resources.word_wrap) arglist[2].value = wordBreak;

    XtCreateManagedWidget( argv[0],
			   (app_resources.filename == NULL)
			       ? asciiStringWidgetClass
			       : asciiDiskWidgetClass,
			    toplevel, arglist, XtNumber(arglist) );

    XtRealizeWidget(toplevel);
    XtMainLoop();
}
