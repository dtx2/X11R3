#ifndef lint
static char rcs_id[] = "$XConsortium: xedit.c,v 1.16 88/10/18 13:30:41 swick Exp $";
#endif

/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission.
 */

#include "xedit.h"
#include <X11/Shell.h>

#ifndef DEFWIDTH
#define DEFWIDTH 500
#endif

#ifndef DEFHEIGHT
#define DEFHEIGHT 700
#endif

int Editable;
int saved;
int backedup;
int lastChangeNumber;
char *displayname;
char *filename;
char *loadedfile;
char *savedfile;
char *searchstring;
char *replacestring;

Widget toplevel;
Widget outer;
Widget Row1;
Widget Row2;
Widget fileBox;
Widget replaceBox;
Widget searchBox;
Widget textwindow;
Widget messwidget;
Widget undoBox;
Widget labelwindow;
		
Widget quitbutton;
Widget loadbutton;
Widget savebutton;
Widget editbutton;
Widget filenamewindow;

Widget searchstringwindow;
Widget replacestringwindow;
Widget qbutton;
Widget ubutton;
Widget umbutton;
Widget searchbutton;
Widget lsearchbutton;
Widget replacebutton;
Widget replaceallbutton;
Widget jumpbutton;

Display *CurDpy;
XtTextSource source, asource, dsource, psource, messsource;
extern DoQ();


makeButtonsAndBoxes()
{
  int boxHeight;
  static Arg TextArgs[] = {
	{ XtNtextSource, NULL },
	{ XtNtextSink, NULL },
	{ XtNtextOptions , (XtArgVal)(scrollVertical | wordBreak) }, 
  };
  static Arg MessArgs[] = {
	{ XtNtextSource, NULL },
	{ XtNtextSink, NULL },
	{ XtNtextOptions, (XtArgVal)(scrollVertical | wordBreak) }, 
  };
  static Arg labelArgs[] = {
	{ XtNjustify,	 (XtArgVal)XtJustifyCenter },
	{ XtNlabel, 	(XtArgVal)"no file yet" },
  };
    outer = XtCreateManagedWidget( "vpaned", vPanedWidgetClass, toplevel,
                                  NULL, 0 );
    XtPanedSetRefigureMode(outer, FALSE);

/*    XtPanedAllowResize(outer, TRUE); */
    Row1 = XtCreateManagedWidget("row1", boxWidgetClass, outer, NULL,0);
        quitbutton = makeCommandButton(Row1, "Quit", DoQuit);
        savebutton = makeCommandButton(Row1, "Save", DoSave);
        editbutton = makeCommandButton(Row1, "Edit", DoEdit);
/*        qbutton = makeCommandButton(Row1, "q", DoQ);   */
        loadbutton = makeCommandButton(Row1, "Load", DoLoad);
        filenamewindow = makeStringBox(Row1, filename,110); 
        ubutton = makeCommandButton(Row1, "Undo", DoUndo);
        umbutton = makeCommandButton(Row1, "More", DoUndoMore);
        jumpbutton = makeCommandButton(Row1, "Jump", DoJump);
/*        linebutton = makeCommandButton(Row1, "Line?", DoLine);  */
    Row2 = XtCreateManagedWidget("row2", boxWidgetClass, outer, NULL,0);
        lsearchbutton = makeCommandButton(Row2, "<< ", DoSearchLeft);
        searchbutton = makeCommandButton(Row2,"Search >>",DoSearchRight); 
        searchstringwindow = makeStringBox(Row2, searchstring, 120); 
        replacebutton= makeCommandButton(Row2, "Replace", DoReplaceOne);
        replaceallbutton = makeCommandButton(Row2, "All", DoReplaceAll);
        replacestringwindow = makeStringBox(Row2, replacestring, 120); 
    {
    int _Min, _Max;
    Dimension boxHeight;
    Arg getargs[1];
    XtSetArg(getargs[0], XtNheight, &boxHeight);
    XtGetValues(quitbutton, getargs, XtNumber(getargs));
    boxHeight += 8;
    XtPanedSetMinMax((Widget) Row1, (int)boxHeight, (int)boxHeight);
    XtPanedSetMinMax((Widget) Row2, (int)boxHeight, (int)boxHeight);

    }
    messsource = PseudoDiskSourceCreate("");
    MessArgs[0].value = (XtArgVal)messsource;
    MessArgs[1].value = (XtArgVal)XtAsciiSinkCreate(outer, NULL, 0);
    messwidget =  XtCreateManagedWidget("messageWindow", textWidgetClass,
		outer, MessArgs, XtNumber(MessArgs));
    XtPanedSetMinMax((Widget) messwidget, 40, 40);

    labelwindow = XtCreateManagedWidget("labelWindow",labelWidgetClass, 
		outer, labelArgs, XtNumber(labelArgs)); 
    XtPanedSetMinMax((Widget)labelwindow, 14, 14);
    TextArgs[0].value = (XtArgVal)source;
    TextArgs[1].value = (XtArgVal)XtAsciiSinkCreate(outer, NULL, 0);
    textwindow =  XtCreateManagedWidget("editWindow", textWidgetClass, 
		outer, TextArgs, XtNumber(TextArgs));
    XtPanedSetRefigureMode(outer, TRUE); 
}

struct _app_resources app_resources;

#define offset(field) XtOffset(struct _app_resources*, field)
static XtResource resources[] = {
   {"editInPlace", "EditInPlace", XtRBoolean, sizeof(int),
         offset(editInPlace), XtRString, "False"},
   {"enableBackups", "EnableBackups", XtRBoolean, sizeof(int),
         offset(enableBackups), XtRString, "FALSE"},
   {"backupNamePrefix", "BackupNamePrefix", XtRString, sizeof(char *),
         offset(backupNamePrefix),XtRString, ""},
   {"backupNameSuffix", "BackupNameSuffix", XtRString, sizeof(char *),
         offset(backupNameSuffix),XtRString, ".BAK"}
};
#undef offset

main(argc, argv)
  int argc;
  char **argv;
{
    static Dimension width, height; /* init'd to 0 */
    Arg args[3];
    backedup = 0;
    saved = 0;
    filename = malloc(1000);
    searchstring = malloc(1000);
    replacestring = malloc(1000);
    toplevel = XtInitialize( "xedit", "Xedit", NULL, 0, &argc, argv);
    XtSetArg(args[0], XtNwidth, &width);
    XtSetArg(args[1], XtNheight, &height);
    XtGetValues(toplevel, args, XtNumber(args));
    if (width == 0) width = DEFWIDTH;
    if (height == 0) height = DEFHEIGHT;
    XtSetArg(args[0], XtNwidth, width);
    XtSetArg(args[1], XtNheight, height);
    XtSetArg(args[2], XtNinput, True);
    XtSetValues(toplevel, args, (Cardinal)3);

    XtGetApplicationResources(toplevel, &app_resources, resources,
                                  XtNumber(resources),NULL, 0);
    CurDpy = XtDisplay(toplevel);
    dsource = PseudoDiskSourceCreate(""); 
    asource = PseudoDiskSourceCreate("");
    source = CreatePSource(dsource, asource);  
    if(argv[1]) strcpy(filename, argv[1]);
    makeButtonsAndBoxes();
    XtRealizeWidget(toplevel);
    XDefineCursor(XtDisplay(toplevel),XtWindow(toplevel),
         XCreateFontCursor( XtDisplay(toplevel), XC_left_ptr));
    if (strlen(filename)) DoLoad();
    XtMainLoop();
}

