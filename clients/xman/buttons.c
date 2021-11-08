/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: buttons.c,v 1.3 89/01/06 18:41:51 kit Exp $
 * $Header: buttons.c,v 1.3 89/01/06 18:41:51 kit Exp $
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
 * Created:   October 27, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: buttons.c,v 4.5 88/12/19 13:46:34 kit Exp $";
#endif

#include "globals.h"

/* The files with the icon bits in them. */

#include "icon_open.h"
#include "icon_help.h"
#include "iconclosed.h"

static void AddManpageCallbacks();
ManpageGlobals * InitPsuedoGlobals();

/*	Function Name: MakeTopMenuWidget
 *	Description: This funtion creates the top menu, in a shell widget.
 *	Arguments: none.
 *	Returns: the top level widget
 */

#define TOPBUTTONS 3
#define TOPARGS 5

void
MakeTopMenuWidget()
{
  Widget top;			/* top box widget. */
  Widget menu;			/* Top Box and menu. */
  Menu topbox;			/* The menu structure for this box. */
  Button topbuttons[TOPBUTTONS]; /* The button structure for these buttons. */
  Arg arglist[TOPARGS];		/* An argument list */
  int i;			/* A Counter. */
  Cardinal num_args = 0;	/* The number of arguments. */
  static char * name[]  = {	/* The names of the buttons. */
    "Help",
    "Quit",
    "Manual Page"
    };

/* create the top icon. */

  num_args = 0;
  XtSetArg(arglist[num_args], XtNiconPixmap,
	   XCreateBitmapFromData( XtDisplay(initial_widget), 
				 XtScreen(initial_widget)->root,
				 iconclosed_bits, iconclosed_width,
				 iconclosed_height));
  num_args++;
  XtSetArg(arglist[num_args], XtNallowShellResize, TRUE); 
  num_args++;

  top = XtCreatePopupShell(TOPBOXNAME, topLevelShellWidgetClass, 
			   initial_widget, arglist, num_args);

  /* Set up the manual structure. */

  topbox.number = TOPBUTTONS;
  topbox.name = "Manual Browser";
  topbox.label_args = NULL;
  topbox.label_num = (unsigned int) 0;
  topbox.box_args = NULL;
  topbox.box_num = (unsigned int) 0;
  topbox.button_args = NULL;
  topbox.button_num = (unsigned int) 0;
  topbox.buttons = topbuttons;
  topbox.callback = TopCallback;

/*
 * Set up the button structures, by assiging each one a name and a 
 * width, this puts either one button on a line or two depending on
 * the length of the string that will go into the buttons. See Menu.c for 
 * details.
 */

  for ( i = 0 ; i < TOPBUTTONS; i ++) {
    topbuttons[i].name = name[i];
    topbuttons[i].number_per_line = strlen(topbox.name)/strlen(name[i]);
    if (topbuttons[i].number_per_line > 1 ) /* Max of 2 buttons per line. */
      topbuttons[i].number_per_line = 2;
  }

  help_widget = NULL;		/* We have not seen the help yet. */

  menu = MakeMenu(top,&topbox, NULL); /* Create the menu. */
  XtManageChild(menu);		/* Manage the menu. */

  XtRealizeWidget(top);
  XtMapWidget(top);
  AddCursor(top, resources.cursors.top);
}

/*	Function Name: CreateManpage
 *	Description: Creates a new manpage.
 *	Arguments: none.
 *	Returns: none.
 */

void
CreateManpage()
{
  ManpageGlobals * man_globals;	/* The psuedo global structure. */

  man_globals = InitPsuedoGlobals();
  CreateManpageWidget(man_globals, MANNAME, TRUE);
  StartManpage( man_globals,  OpenHelpfile(man_globals) );
}

/*	Function Name: InitPsuedoGlobals
 *	Description: Initializes the psuedo global variables.
 *	Arguments: none.
 *	Returns: a pointer to a new pseudo globals structure.
 */

ManpageGlobals * 
InitPsuedoGlobals()
{
  ManpageGlobals * man_globals;
  int i;

  man_globals = (ManpageGlobals *) malloc(sizeof(ManpageGlobals));
  if (man_globals == NULL)
    PrintError("Not Enough Memory to open man_globals");

/* Initialize the number of screens that will be shown */

  man_globals->both_shown = resources.both_shown_initial;

  for ( i = 0 ; i < sections ; i++) 
    man_globals->manpagewidgets.box[i] = NULL;
  
  return(man_globals);
}
  
/*	Function Name: CreateManpageWidget
 *	Description: Creates a new manual page widget.
 *	Arguments: man_globals - a new man_globals structure.
 *                 name         - name of this shell widget instance.
 *                 full_instance - if true then create a full manpage,
 *                                 otherwise create stripped down version
 *                                 used for help.
 *	Returns: none
 */

#define MANPAGEARGS 10

void
CreateManpageWidget(man_globals, name, full_instance)
ManpageGlobals * man_globals;
char * name;
Boolean full_instance;
{
  int font_height;
  Arg arglist[MANPAGEARGS];	/* An argument list for widget creation */
  Cardinal num_args;		/* The number of arguments in the list. */
  Widget top, pane, section,search;	/* Widgets */
  ManPageWidgets * mpw = &(man_globals->manpagewidgets);

  num_args = (Cardinal) 0;
  XtSetArg(arglist[num_args], XtNwidth, default_width);
  num_args++; 
  XtSetArg(arglist[num_args], XtNheight, default_height);
  num_args++; 

  top = XtCreatePopupShell(name, topLevelShellWidgetClass, initial_widget,
			   arglist, num_args);

  man_globals->This_Manpage = top; /* pointer to root widget of Manualpage. */
  num_args = 0;
  if (full_instance)
    XtSetArg(arglist[num_args], XtNiconPixmap,
	     XCreateBitmapFromData( XtDisplay(top), XtScreen(top)->root,
				   icon_open_bits, icon_open_width,
				   icon_open_height));
  else 
    XtSetArg(arglist[num_args], XtNiconPixmap,
	     XCreateBitmapFromData( XtDisplay(top), XtScreen(top)->root,
				   icon_help_bits, icon_help_width,
				   icon_help_height));
  num_args++;
  XtSetValues(top, arglist, num_args);

  pane = XtCreateManagedWidget("Manpage_Vpane", vPanedWidgetClass, top, NULL, 
			       (Cardinal) 0);

/* Create top label. */

  man_globals->label = XtCreateManagedWidget(MANNAME, labelWidgetClass,
				pane, NULL, (Cardinal) 0);

  XtPanedSetMinMax( man_globals->label, 2 ,
		   Height( man_globals->label ) );
  XtAddEventHandler(man_globals->label, (Cardinal) EnterWindowMask,
		    FALSE ,PopUpMenu, NULL);

/* Create Directory */

  if (full_instance) {
    num_args = 0;
    XtSetArg(arglist[num_args], XtNallowVert, TRUE);
    num_args++;
    
    mpw->directory = XtCreateWidget(DIRECTORY_NAME, viewportWidgetClass,
				    pane, arglist, num_args);
    
    man_globals->current_directory = INITIAL_DIR;
    MakeDirectoryBox(man_globals, mpw->directory,
		     mpw->box + man_globals->current_directory, 
		     man_globals->current_directory );
    XtManageChild(mpw->box[man_globals->current_directory]);
  }

/* Create Manpage */

  font_height = (resources.fonts.normal->max_bounds.ascent + 
		   resources.fonts.normal->max_bounds.descent);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNallowVert, TRUE);
  num_args++;
  XtSetArg(arglist[num_args], XtNfontHeight, font_height);
  num_args++;

  mpw->manpage = XtCreateWidget(MANUALPAGE, scrollByLineWidgetClass,
				pane, arglist, num_args);

  AddManpageCallbacks(man_globals, mpw->manpage);

/* make popup widgets. */

  MakeTopPopUpWidget(man_globals, man_globals->label, &section, &search,
		     full_instance);
  if (full_instance) {
    MakeDirPopUpWidget(man_globals, section);
    MakeSearchWidget(man_globals, search);
    MakeSaveWidgets(man_globals, mpw->directory, mpw->manpage);
  }
  else {
    man_globals->both_shown = TRUE; /* This is a lie, but keeps it from
				      being allowed to change to non-existant
				      directory. */
    XtSetSensitive(section, FALSE);
    XtSetSensitive(search, FALSE);
    XtSetSensitive(man_globals->put_up_manpage,FALSE);
    XtSetSensitive(man_globals->put_up_directory,FALSE);
    XtSetSensitive(man_globals->both_shown_button, FALSE); 
    XtSetSensitive(man_globals->help_button, FALSE);       
    MakeLong( XtParent(man_globals->help_button) ); /* Fix top menu. */
  }
}

/*	Function Name: StartManpage
 *	Description: Starts up a new manpage.
 *	Arguments: man_globals - the psuedo globals variable.
 *                 help - Is help avaliable?
 *	Returns: none.
 */

static void
StartManpage(man_globals, help)
ManpageGlobals * man_globals;
Boolean help;
{
  Widget dir = man_globals->manpagewidgets.directory;
  Widget manpage = man_globals->manpagewidgets.manpage;
  Widget label = man_globals->label;

/* 
 * If there is a helpfile then put up both screens if both_show is set.
 */

  if (help) {
    strcpy(man_globals->manpage_title, "Xman Help");
    if (man_globals->both_shown) {
      XtManageChild(dir);
      man_globals->dir_shown = TRUE;
      XtPanedSetMinMax(dir, resources.directory_height, 
		       resources.directory_height);
      XtSetSensitive(man_globals->put_up_manpage,FALSE);
      XtSetSensitive(man_globals->put_up_directory,FALSE);
      ChangeLabel(man_globals->both_shown_button, SHOW_ONE);
      MakeLong( XtParent(man_globals->help_button) ); /* Fix top menu. */
      ChangeLabel(label,
		  man_globals->section_name[man_globals->current_directory]);
    }
    else {
      ChangeLabel(label,man_globals->manpage_title);
    }
    XtManageChild(manpage);
    man_globals->dir_shown = FALSE;
  }
/*
 * Since There is no help file, put up directory and do not allow change
 * to manpage, show both, or help.
 */
  else {			
    XtManageChild(dir);
    man_globals->dir_shown = TRUE;
    XtSetSensitive(man_globals->put_up_manpage, FALSE);
    XtSetSensitive(man_globals->both_shown_button, FALSE);    
    XtSetSensitive(man_globals->help_button, FALSE);    
    MakeLong( XtParent(man_globals->help_button) ); /* Fix top menu. */
    man_globals->both_shown = FALSE;
    ChangeLabel(label,
		man_globals->section_name[man_globals->current_directory]);
  }

/*
 * Start 'er up, and change the cursor.
 */

  XtRealizeWidget(  man_globals->This_Manpage );
  XtMapWidget( man_globals->This_Manpage );

  AddCursor( man_globals->This_Manpage, resources.cursors.manpage);
  XtPanedSetMinMax(dir, 1, 10000); 
}

/*	Function Name: AddManpageCallbacks
 *	Description: adds callback and event handler to manual page widget.
 *	Arguments: man_globals - the psuedo globals structure for each manpage.
 *                 w - the manual page widget.
 *	Returns: none
 */

static void
AddManpageCallbacks(man_globals, w)
ManpageGlobals * man_globals;
Widget w;
{
  
/*
 * Allocate the memory structure that PrintManpage uses to determine what
 * text to print for the manpage.
 */

  man_globals->memory_struct = (MemoryStruct *) malloc(sizeof(MemoryStruct));
  if (man_globals->memory_struct == NULL)
    PrintError("Could not allocate space for the memory_struct.");

/*
 * Initialize to NULL, telling InitManpage not to try to free this memory. 
 */

  man_globals->memory_struct->top_line = NULL;
  man_globals->memory_struct->top_of_page = NULL;
  
  XtAddCallback(w, XtNcallback,
		PrintManpage, (caddr_t) man_globals->memory_struct);

/* 
 * We would also like to be notified of button press events in
 * The manual page, this allows us to switch to the directory on middle
 * button press.
 */

  XtAddEventHandler(w, (unsigned int) ButtonPressMask | ButtonReleaseMask,
		    FALSE, ManpageButtonPress, (caddr_t) man_globals);

}

/*	Function Name: MakeTopPopUpWidget
 *	Description: Creates the pop up menu for Xman.
 *	Arguments: man_globals - the manpage psuedo globals file.
 *                 widget - this popup shell's parent.
 *                 command, section - the widgets that corrospond to these
 *                                    two command buttons.
 *                 is_manpage - TRUE = manpage; FALSE = help widget.
 *	Returns: the parent widget of the popup menu that is created.
 */


/* The method used for creating all these popup menus is very similar
 * to that used for the top level buttons see the functions: MakeTopMenuWidget
 * and MakeMenu for details.
 */

#define TOPPOPUPBUTTONS 9

void
MakeTopPopUpWidget(man_globals,widget,section,search, is_manpage)
ManpageGlobals * man_globals;
Widget widget,*section,*search;
Boolean is_manpage;
{
  Menu popup;			/* The menu structure. */
  Widget popup_shell;		/* the pop up widget's shell. */
  Widget pupwidget;		/* The popup menu widget. */
  Button buttons[TOPPOPUPBUTTONS];	/* The menu buttons structure. */
  Arg args[1];			/* The argument list. */
  int i;
  Cardinal num_args = 0;	/* The number of arguments. */
  static char * name[] = {	/* Names of the buttons. */
    "Change Section",
    "Display Directory",
    "Display Manual Page",
    "Help",
    "Search",
    "Show Both Screens",
    "Remove This Manpage",
    "Open New Manpage",
    "Quit",
  };

  if (!is_manpage)
    name[6] = "Remove Help";
  else			    /* since manpages are often started after help. */
    name[6] = "Remove This Manpage";
    
  popup_shell = XtCreatePopupShell(POPUPNAME, overrideShellWidgetClass, 
				   widget, NULL, (Cardinal) 0);
  XtSetArg(args[num_args],XtNjustify,XtJustifyLeft); 
  num_args++;

  popup.number = TOPPOPUPBUTTONS;
  popup.name = "Xman Options";
  popup.label_args = NULL;
  popup.label_num = 0;
  popup.box_args = NULL;
  popup.box_num = 0;
  popup.button_args = args;
  popup.button_num = num_args;
  popup.buttons = buttons;
  popup.callback = TopPopUpCallback;

  for ( i = 0 ; i < TOPPOPUPBUTTONS; i ++) {
    buttons[i].name = name[i];
    buttons[i].number_per_line = 0; /* zero means do not resize, this is
				       done later in the function MakeLong. */
  }

/*
 * This is not too pretty but it works.  The numbers of the children are the
 * order that they were in the name list, and the label is child 0.
 */

  pupwidget = MakeMenu(popup_shell,&popup,
					 (caddr_t) man_globals);
/* Boy this is ugly, but I can't think of a better method. */

  *section =Child(pupwidget, 1);
  man_globals->put_up_directory = Child(pupwidget, 2);
  man_globals->put_up_manpage = Child(pupwidget, 3);
  man_globals->help_button = Child(pupwidget, 4);
  *search = Child(pupwidget, 5);
  man_globals->both_shown_button = Child(pupwidget, 6);

/* Pop down when you leave the menu window. */

  XtAddEventHandler(popup_shell, (Cardinal) LeaveWindowMask,
		    FALSE, PopDown, NULL);
  XtManageChild((Widget) pupwidget); /* Manage it. */

/*
 * Make all children of the button box that contains the menu the same length
 * as the longest child.
 */

  MakeLong((Widget) pupwidget);

  XtRealizeWidget(popup_shell);	/* Realize it and change its cursor. */
  AddCursor(popup_shell,resources.cursors.top);
}

/*	Function Name: MakeDirPopUpWidget
 *	Description: Creates the pop up menu for Xman.
 *	Arguments: man_globals - the manpage psuedo globals file.
 *                 widget - any widget
 *	Returns: the parent widget of the popup menu that is created.
 */

/* The method used for creating all these popup menus is very similar
 * to that used for the top level buttons see the functions: MakeTopMenuWidget
 * and MakeMenu for details.
 */

void
MakeDirPopUpWidget(man_globals,widget)
ManpageGlobals * man_globals;
Widget widget;
{
  Menu popup;			/* The menu structure. */
  Widget popup_shell;		/* the pop up widget's shell. */
  Widget pupwidget;		/* The popup menu widget. */
  Button buttons[MAXSECT];	/* The menu buttons structure. */
  Arg args[1];			/* The argument list. */
  int i;			/* A counter. */
  Cardinal num_args = 0;	/* The number of arguments. */

  popup_shell = XtCreatePopupShell( SPOPUPNAME, overrideShellWidgetClass,
				   widget, NULL, (Cardinal) 0);
  XtSetArg(args[num_args],XtNjustify,XtJustifyLeft); 
  num_args++;

  popup.number = sections;
  popup.name = "Manual Sections";
  popup.label_args = NULL;
  popup.label_num = (Cardinal) 0;
  popup.box_args = NULL;
  popup.box_num = (Cardinal) 0;
  popup.button_args = args;
  popup.button_num = num_args;
  popup.buttons = buttons;
  popup.callback = DirPopUpCallback;

  for ( i = 0 ; i < sections; i ++) {
    buttons[i].name = manual[i].blabel;
    buttons[i].number_per_line = 0;
  }

  pupwidget = MakeMenu(popup_shell,&popup, (caddr_t) man_globals);
  XtAddEventHandler(popup_shell, (Cardinal) LeaveWindowMask,
		    FALSE, PopDown, NULL);
  XtManageChild(pupwidget);
  (void) MakeLong(pupwidget);
  XtRealizeWidget(popup_shell);
  AddCursor(popup_shell,resources.cursors.top);
}

/*	Function Name: CreateManpageName
 *	Description: Creates the manual page name for a given item.
 *	Arguments: entry - the entry to convert.
 *	Returns: the manual page properly allocated.
 */

/*
 * If the filename is foo.3     - Create an entry of the form:  foo
 * If the filename is foo.3X11 
 * or foo.cX11.stuff            - Create an entry of the form:  foo(X11)
 */

char *
CreateManpageName(entry)
char * entry;
{
  char * cp;
  char page[BUFSIZ];

  ParseEntry(entry, NULL, NULL, page);

  cp = index(page, '.');
  if ( (cp[2] != '\0') && (cp[2] != '.') ) {
    *cp++ = '(';
    while( (cp[1] != '\0') && (cp[1] != '.') ) {
      *cp = cp[1]; cp++;
    }
    *cp++ = ')';
    *cp = '\0';
  }
  else
    *cp = '\0';  

  return(StrAlloc(page));
}

/*	Function Name: CreateList
 *	Description: this function prints a label in the directory list
 *	Arguments: section - the manual section.
 *	Returns: none
 */

static char **
CreateList(section)
{
  char ** ret_list, **current;
  int count;

  ret_list = (char **) malloc((manual[section].nentries + 1)*sizeof (char *));
  if (ret_list == NULL)
    PrintError("Could not allocate space for a directory list.");

  for (current = ret_list, count = 0 ; count < manual[section].nentries ;
       count++, current++)
    *current = CreateManpageName(manual[section].entries[count]);
 
  *current = NULL;		/* NULL terminate the list. */
  return(ret_list);
}

/*	Function Name: MakeDirectoryBox
 *	Description: make a directory box.
 *	Arguments: man_globals - the psuedo global structure for each manpage.
 *                 parent - this guys parent widget.
 *                 dir_disp - the directory display widget.
 *                 section - the section number.
 *	Returns: none.
 */

void
MakeDirectoryBox(man_globals,parent,dir_disp,section)
ManpageGlobals *man_globals;
Widget parent, *dir_disp;
int section;
{
  Arg arglist[10];
  Cardinal num_args;
  char * name, label_name[BUFSIZ];

  if (*dir_disp != NULL)	/* If we have one, don't make another. */
    return;

  name = manual[section].blabel;   /* Set the section name */
  sprintf(label_name,"Directory of: %s",name);
  man_globals->section_name[section] = StrAlloc(label_name);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNlist, CreateList(section));
  num_args++;
  XtSetArg(arglist[num_args], XtNborderWidth, 0);
  num_args++;
  XtSetArg(arglist[num_args], XtNfont, resources.fonts.directory);
  num_args++;
  
  *dir_disp = XtCreateWidget(DIRECTORY_NAME, listWidgetClass, parent,
			     arglist, num_args);
  
  XtAddCallback(*dir_disp, XtNcallback,
		DirectoryHandler, (caddr_t) man_globals);
  XtAddEventHandler(*dir_disp, (Cardinal) ButtonPressMask | ButtonReleaseMask, 
		    FALSE, GotoManpage ,(caddr_t) man_globals);
}

/*	Function Name: MakeSaveWidgets.
 *	Description: This functions creates two popup widgets, the please 
 *                   standby widget and the would you like to save widget.
 *	Arguments: man_globals - the psuedo globals structure for each man page
 *                 standby_parent - the parent for the standby widget.
 *                 save_parent - the parent for the save widget.
 *	Returns: none.
 */

void
MakeSaveWidgets(man_globals, standby_parent, save_parent)
ManpageGlobals *man_globals;
Widget standby_parent,save_parent;
{
  Widget box, shell, label, button; /* misc. widgets. */
  Arg arglist[10];		/* The arglist. */
  Cardinal num_args;		/* the number of args. */
  Dimension form_width;		/* The width of the form widget. */
  
/* make the please stand by popup widget. */

  num_args = 0;
  shell = XtCreatePopupShell( "PleaseStandBy", transientShellWidgetClass,
			      standby_parent, arglist, num_args);

  box = XtCreateWidget("PleaseStandByBox",boxWidgetClass,shell,
		       arglist,num_args);

/*  XtSetArg(arglist[num_args], XtNborderWidth, 0);
  num_args++; */
  man_globals->standby1 = XtCreateManagedWidget("Formatting Manual Page,",
						labelWidgetClass,box,
						arglist,num_args);

  man_globals->standby2 = XtCreateManagedWidget("Please Stand By...",
						labelWidgetClass,box,
						arglist,num_args);
  XtManageChild(box);
  XtRealizeWidget(shell);
  AddCursor(shell,resources.cursors.top);

  num_args = 0;
  shell = XtCreatePopupShell("likeToSave",transientShellWidgetClass,
			     save_parent,arglist,num_args);

  box = XtCreateWidget("likeToSave",formWidgetClass,shell,
		       arglist,num_args);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNborderWidth, 0);
  num_args++;

  XtSetArg(arglist[num_args], XtNlabel, 
	   "Would you like to save this formatted manual page?");
  num_args++;
  label = XtCreateManagedWidget("label", labelWidgetClass, box, 
				arglist, num_args);

/* Find out what the width of the form is. */

  num_args = 0;
  XtSetArg(arglist[num_args], XtNwidth, &form_width);
  num_args++;

  XtGetValues(box,arglist,num_args); 

  form_width /= 2;

  num_args = 0; 
  XtSetArg(arglist[num_args], XtNwidth, form_width);
  num_args++;
  XtSetArg(arglist[num_args], XtNfromVert, label);
  num_args++;

/* 
 * We are constraining the width of these guys so be careful in setting the 
 * values of FILE_SAVE and CANCEL_FILE_SAVE.
 */

  button = XtCreateManagedWidget(FILE_SAVE,commandWidgetClass,box,
				 arglist,num_args);
  XtAddCallback(button, XtNcallback, SaveCallback, (caddr_t) man_globals);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNwidth, form_width);
  num_args++;
  XtSetArg(arglist[num_args], XtNfromVert, label);
  num_args++;
  XtSetArg(arglist[num_args], XtNfromHoriz, button);
  num_args++;
 
  button =  XtCreateManagedWidget(CANCEL_FILE_SAVE,commandWidgetClass,box,
			 arglist,num_args);
  XtAddCallback(button, XtNcallback, SaveCallback, (caddr_t) man_globals);

  XtManageChild(box);
  XtRealizeWidget(shell);
  AddCursor(shell,resources.cursors.top);
}
