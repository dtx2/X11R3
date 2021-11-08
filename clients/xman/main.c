/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: main.c,v 1.4 89/01/06 18:42:08 kit Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   October 22, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: main.c,v 4.5 88/12/19 13:47:28 kit Exp $";
#endif

#include "globals.h"

#if ( !defined(lint) && !defined(SABER)) /* Version can be retrieve */
  static char version[] = XMAN_VERSION;  /* via strings. */
#endif

static void ArgError();

#define Offset(field) (XtOffset(Xman_Resources *, field))

static XtResource my_resources[] = {
  {"manualFontNormal", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     Offset(fonts.normal), XtRString, MANPAGE_NORMAL},
  {"manualFontBold", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     Offset(fonts.bold), XtRString, MANPAGE_BOLD},
  {"manualFontItalic", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     Offset(fonts.italic), XtRString, MANPAGE_ITALIC},
  {"directoryFontNormal", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     Offset(fonts.directory), XtRString, DIRECTORY_NORMAL},
  {"bothShown", XtCBoolean, XtRBoolean, sizeof(Boolean),
     Offset(both_shown_initial), XtRString, "False"},
  {"directoryHeight", "DirectoryHeight", XtRInt, sizeof(int),
     Offset(directory_height), XtRString, "150"},  
  {"topCursor", XtCCursor, XtRCursor, sizeof(Cursor), 
     Offset(cursors.top), XtRString, XMAN_CURSOR},
  {"helpCursor", XtCCursor, XtRCursor, sizeof(Cursor),
     Offset(cursors.help), XtRString, HELP_CURSOR},
  {"manpageCursor", XtCCursor, XtRCursor, sizeof(Cursor),
     Offset(cursors.manpage), XtRString, MANPAGE_CURSOR},
  {"searchEntryCursor", XtCCursor, XtRCursor, sizeof(Cursor),
     Offset(cursors.search_entry), XtRString, SEARCH_ENTRY_CURSOR},
  {"helpFile", XtCFile, XtRString, sizeof(char *),
     Offset(help_file), XtRString, HELPFILE},
  {"topBox", XtCBoolean, XtRBoolean, sizeof(Boolean),
     Offset(top_box_active), XtRString, "True"},
};

/*
 * This is necessary to keep all TopLevel shells from becoming
 * the size that is specified on the command line.
 */

static XrmOptionDescRec xman_options[] = {
{"-geometry", "*topBox.geometry", XrmoptionSepArg, (caddr_t) NULL},
{"=",         "*topBox.geometry", XrmoptionIsArg, (caddr_t) NULL},
{"-pagesize", "*manualBrowser.geometry", XrmoptionSepArg, (caddr_t) NULL},
{"-notopbox", "topBox", XrmoptionNoArg, (caddr_t) "False"},
{"-helpfile", "helpFile", XrmoptionSepArg, (caddr_t) NULL},
{"-bothshown", "bothShown", XrmoptionNoArg, (caddr_t) "True"},
};

/*	Function Name: main
 *	Description: This is the main driver for Xman.
 *	Arguments: argc, argv - the command line arguments.
 *	Returns: return, what return.
 */

void 
main(argc,argv)
char ** argv;
int argc;
{
  initial_widget = XtInitialize(NULL, "XMan", 
				xman_options, XtNumber(xman_options),
				(unsigned int*) &argc,argv);
  if (argc != 1) {
    ArgError(argc, argv);
    exit(42);
  }

  XtGetApplicationResources( initial_widget, (caddr_t) &resources, 
			    my_resources, XtNumber(my_resources),
			    NULL, (Cardinal) 0);

  if (!resources.fonts.normal)
	PrintError("Failed to get the manualFontNormal font");
  if (!resources.fonts.bold)
	resources.fonts.bold = resources.fonts.normal;
  if (!resources.fonts.italic)
	resources.fonts.italic = resources.fonts.bold;
  if (!resources.fonts.directory)
	resources.fonts.directory = resources.fonts.normal;

#ifdef DEBUG
  printf("debugging mode\n");
#endif

/*
 * Set the default width and height.
 * I am not real happy with this method, but it will usually do something
 * reasonable, if not the "right" thing.  It is not a real big issue since
 * it is easy to change the values with resources or command line options.
 * NOTE: if you are in a 100 dpi display you will lose.
 */

  default_width = DEFAULT_WIDTH;
  default_height=DisplayHeight(XtDisplay(initial_widget), 
			       XtDisplay(initial_widget)->default_screen);
  default_height *= 3;
  default_height /= 4;

  sections = Man();

  if (resources.top_box_active) {
    MakeTopMenuWidget();	
  }
  else
    CreateManpage();

/*
 * We need to keep track of the number of manual pages that are shown on
 * the screen so that if this user does not have a top box then when he
 * removes all his manual pages we can kill off the xman process.
 * To make things easier we will consider the top box a shown manual page
 * here, but since you cannot remove it, man_page_shown only goes to zero when
 * no top box is present.
 */

  man_pages_shown = 1;		

  XtMainLoop();
}

/*	Function Name: Quit
 *	Description: closes the display and quits.
 *	Arguments: widget - and widget.
 *	Returns: none.
 */

void
Quit(w)
Widget w;
{
  XCloseDisplay(XtDisplay(w));
  exit(0);
}


/*	Function Name: ArgError
 *	Description:  Prints error message about unknow arguments.
 *	Arguments: argc, argv - args not understood.
 *	Returns: none.
 */

static void 
ArgError(argc, argv)
char ** argv;
int argc;
{
  int i;

  static char **syntax, *syntax_def[] = {
  "-helpfile <filename>",    "Specifies the helpfile to use.",
  "-bothshown",              "Show both the directory and manpage at once.",
  "-notopbox",               "Starts with manpage rather than topbox.",
  "-geometery <geom>",       "Specifies the geometry of the top box.",
  "=<geom>",                 "Specifies the geometry of the top box.",
  "-pagesize <geom>",        "Specifies the geometry of the manual page.",
  "-bw <pixels>",            "Width of all window borders.",
  "-borderwidth <pixels>",   "Width of all window borders.",
  "-bd <color>",             "Color of all window borders.",
  "-bordercolor <color>",    "Color of all window borders.",
  "-fg <color>",             "Foreground color for the application.",
  "-foreground <color>",     "Foreground color for the application.",
  "-bg <color>",             "Background color for the application.",
  "-background <color>",     "Background color for the application.",
  "-display <display name>", "Specify a display that is not the default",
  "-fn <font>",              "Font to be used for button and label text.",
  "-font <font>",            "Font to be used for button and label text.",
  "-name <name>",            "Change the name used for retrieving resources.",
  "-title <name>",           "Change the name without affecting resources.",
  "-xrm <resource>",         "Specifies a resource on the command line.",
  NULL, NULL,
  };
  
  syntax = syntax_def;

  for (i = 1; i < argc ; i++) 
    (void) printf("This argument is unknown to Xman: %s\n", argv[i]);
  
  (void) printf("\nKnown arguments are:\n");

  while ( *syntax != NULL ) {
    printf("%-30s - %s\n", syntax[0], syntax[1]);
    syntax += 2;
  }
}
