/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: handler.c,v 1.3 89/01/06 18:42:00 kit Exp $
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
 * Created:   October 29, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: handler.c,v 4.6 89/01/06 12:17:27 kit Exp $";
#endif

#include "globals.h"

static void PutUpManpage();

/*	Function Name: TopPopUpCallback
 *	Description: This is the callback function for the callback menu.
 *	Arguments: w - the widget we are calling back from. 
 *                 globals_pointer - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 junk - (call data) not used.
 *	Returns: none.
 */

/* ARGSUSED */

void
TopPopUpCallback(w,pointer,junk)
Widget w;
caddr_t pointer,junk;
{
  int number;
  ManpageGlobals * man_globals; 
  MenuCallbackStruct * menu_struct;
  char * label_str;

  menu_struct = (MenuCallbackStruct *) pointer;

  man_globals = (ManpageGlobals *) menu_struct->data;

  number = menu_struct->number;

  switch(number) {
  
  case 0:			/* section */
  case 4:			/* search */
    XtPopdown( XtParent(XtParent(w)) ); /* pop the top one down */
    PopUpMenu(w,NULL,NULL);
    break;
  case 1:			/* goto dorectory. */
    /* Put Up Directory */
    ChangeLabel(man_globals->label,
		man_globals->section_name[man_globals->current_directory]);
    XtUnmanageChild(man_globals->manpagewidgets.manpage);
    XtManageChild(man_globals->manpagewidgets.directory);
    man_globals->dir_shown = TRUE;
    break;
  case 2:			/* goto manpage. */
    /* Put Up Man Page */
    ChangeLabel(man_globals->label,man_globals->manpage_title);
    XtUnmanageChild(man_globals->manpagewidgets.directory);
    XtManageChild(man_globals->manpagewidgets.manpage);
    man_globals->dir_shown = FALSE;
    break;
  case 3:			/* help */
    PopupHelp();
    break;
  case 5:			/* Toggle Both_Shown State. */
/*
 * I did not have a two state or toggle widget, which is the way this
 * should really be done.  1/22/88 - CDP.
 */
    if (man_globals->both_shown == TRUE) {
      label_str = SHOW_BOTH;
      if (man_globals->dir_shown)
	XtUnmanageChild(man_globals->manpagewidgets.manpage);
      else
	XtUnmanageChild(man_globals->manpagewidgets.directory);
    }
    else {
      Widget manpage = man_globals->manpagewidgets.manpage;
      Widget dir = man_globals->manpagewidgets.directory;

      label_str = SHOW_ONE;
      XtPanedSetMinMax(dir, resources.directory_height, 
		       resources.directory_height);
      if (!man_globals->dir_shown) {
	XtUnmanageChild(manpage);
	XtManageChild(dir);
      }
      XtManageChild(manpage);

      /* Allow it to be any size. */
      XtPanedSetMinMax(dir,1,10000);
    }
    man_globals->both_shown = !man_globals->both_shown;

/*
 *  Pop the menu down, so the user does not see the resizing of the buttons. 
 * XtPopDown is clever enought not do try to popdown unless the window
 * is up so very little time is wasted in the extra popdown call later.
 */
    XtPopdown( XtParent(XtParent(w)) );
    if (man_globals->dir_shown)
      ChangeLabel(man_globals->label,
		  man_globals->section_name[man_globals->current_directory]);
    else
      ChangeLabel(man_globals->label, man_globals->manpage_title);
    ChangeLabel(man_globals->both_shown_button, label_str);
    /* if both are shown there is no need to switch between the two. */
    XtSetSensitive(man_globals->put_up_manpage,!man_globals->both_shown);
    XtSetSensitive(man_globals->put_up_directory,!man_globals->both_shown);
    MakeLong( XtParent(w) );
    break;
  case 6:			/* kill the manpage */
    KillManpage(man_globals);
    if (man_pages_shown == 0)
      Quit(w);
    break;
  case 7:			/* Open New Manual Page. */
    CreateManpage();
    man_pages_shown++;
    break;
  case 8:			/* quit */
    Quit(w);
    break;
  }

  /* We have used this guy, pop down the menu. */
  
  XtPopdown(  XtParent(XtParent(w)) );


}

/*	Function Name: PopUpMenu
 *	Description: This function pops up the popup child of the given widget
 *                   under the cursor.
 *	Arguments: w - the popup menu.
 *                 junk - not used.
 *                 event - the event generated.
 *	Returns: none
 */

/* How far off the top of the widget to have the initial cursor postion. */

#define OFF_OF_TOP 25

/* ARGSUSED */

void
PopUpMenu(w,junk,event)
Widget w;
caddr_t junk;
XEvent * event;
{
  Widget popup = PopupChild(w, 0);		/* The Popup wodget. */
  int x_root,y_root,y_pos,garbage;
  unsigned int mask;
  Window junk_window;

  if (event != NULL) {		/* It is a EnterNotify event. */
    x_root = event->xcrossing.x_root;
    y_root = event->xcrossing.y_root;
  }
  else {			/* Server Call SLOOOOOOOW. */
    XQueryPointer(XtDisplay(w),XtWindow(w),&junk_window,&junk_window,
		  &x_root,&y_root,&garbage,&garbage,&mask);
  }
  y_pos = OFF_OF_TOP - Height(popup)/2 - BorderWidth(popup);
  PositionCenter(popup,x_root,y_root,y_pos,0,2,2);
  XtPopup(popup, XtGrabExclusive);
}

/*	Function Name: PopDown
 *	Description: This function pops down a given widget.
 *	Arguments: w - the popup menu.
 *                 junk - not used.
 *                 event - the event generated.
 *	Returns: none
 */

/* ARGSUSED */

void
PopDown(w,junk,event)
Widget w;
caddr_t junk;
XEvent * event;
{
  if (event->xcrossing.detail != NotifyInferior) {
    XtPopdown(w);
  }
}

/*	Function Name: SearchCallback
 *	Description: This is the callback function for the search buttons.
 *	Arguments: w - the widget we are calling back from. 
 *                 global_pointer - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 junk - (call data) not used.
 *	Returns: none
 */

/* ARGSUSED */
void
SearchCallback(w,global_pointer,junk)
Widget w;
caddr_t global_pointer,junk;
{
  ManpageGlobals * man_globals;
  FILE * file;

  man_globals = (ManpageGlobals *) global_pointer;

  if ( !strcmp(Name(w),MANUALSEARCH) )
    file = DoSearch(man_globals,MANUAL);
  else if ( !strcmp(Name(w),APROPOSSEARCH) )
    file = DoSearch(man_globals,APROPOS);
  else if ( !strcmp(Name(w),CANCEL) ) 
    file = NULL;
  else 
    PrintError("Unknown widget, in Search Box.");

  /* popdown the search widget */

  XtPopdown(  XtParent(XtParent(w)) );

  PutUpManpage(man_globals, file);
}

/*	Function Name: PutUpManpage
 *	Description: Puts the manpage on the display.
 *	Arguments: man_globals - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 file - the file to display.
 *	Returns: none.
 */

static void
PutUpManpage(man_globals, file)
ManpageGlobals * man_globals;
FILE * file;
{
  if (file == NULL)
    return;

  InitManpage(man_globals,man_globals->manpagewidgets.manpage,file);
  fclose(file);
  
  if (man_globals->both_shown) {
    ChangeLabel(man_globals->label, 
		man_globals->section_name[man_globals->current_directory]);
  }
  else {
    XtUnmanageChild(man_globals->manpagewidgets.directory);
    XtManageChild(man_globals->manpagewidgets.manpage);
    XtSetSensitive(man_globals->put_up_manpage,TRUE); 
    ChangeLabel(man_globals->label,man_globals->manpage_title);
    man_globals->dir_shown = FALSE;
  }

  XtSetSensitive(man_globals->both_shown_button,TRUE);
  MakeLong( XtParent(man_globals->both_shown_button) );
  XtResetScrollByLine(man_globals->manpagewidgets.manpage);
}

/*	Function Name: TopCallback
 *	Description: this is the callback function for top menu buttons.
 *	Arguments: w - the widget we are calling back from. 
 *                 number - (closure) the number to switch on.
 *                 junk - (call data) not used.
 *	Returns: none.
 */

/* ARGSUSED */
void
TopCallback(w,pointer,junk)
Widget w;
caddr_t pointer,junk;
{
  MenuCallbackStruct * menu_struct;

  menu_struct = (MenuCallbackStruct *) pointer;

  switch(menu_struct->number) {
  case 0:			/* Help */
    PopupHelp();
    break;
  case 1:			/* Quit */
    Quit(w);
    break;
  case 2:			/* Manual Page */
    CreateManpage();
    man_pages_shown++;
  }
}

/*	Function Name: GotoManpage
 *	Description: This swaps to manual page on button 2 pressed.
 *	Arguments: w - the widget we are calling back from. 
 *                 global_pointer - the pointer to the psuedo global structure
 *                                  associated with this manpage.
 *                 event - the event detected.
 *	Returns: none.
 */

/* ARGSUSED */

void
GotoManpage(w, global_pointer, event)
Widget w;
caddr_t global_pointer;
XEvent * event;
{
  ManpageGlobals * man_globals;

  man_globals = (ManpageGlobals *) global_pointer;

  if ( event->xbutton.button == 2 && event->type == ButtonPress ) {
    /* Change to ManPage. */
    if ( !man_globals->both_shown &&
	XtIsSensitive(man_globals->put_up_manpage) ) {
      ChangeLabel(man_globals->label,man_globals->manpage_title);
      XtUnmanageChild(man_globals->manpagewidgets.directory);
      XtManageChild(man_globals->manpagewidgets.manpage);
      man_globals->dir_shown = FALSE;
    }
  }
}

/*	Function Name: DirectoryHandler
 *	Description: This is the callback function for the directory listings.
 *	Arguments: w - the widget we are calling back from. 
 *                 global_pointer - the pointer to the psuedo global structure
 *                                  associated with this manpage.
 *                 ret_val - return value from the list widget.
 *	Returns: none.
 */

/* ARGSUSED */
void
DirectoryHandler(w, global_pointer, ret_val)
Widget w;
caddr_t global_pointer, ret_val;
{
  FILE * file;			/* The manpage file. */
  ManpageGlobals * man_globals = (ManpageGlobals *) global_pointer;
  XtListReturnStruct * ret_struct = (XtListReturnStruct *) ret_val;

  file = FindFilename(man_globals,
	 manual[man_globals->current_directory].entries[ret_struct->index]);
  PutUpManpage(man_globals, file);
}

/*	Function Name: DirPopUpCallback
 *	Description: This is the callback function for the callback menu.
 *	Arguments: w - the widget we are calling back from. 
 *                 pointer - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 junk - (call data) not used.
 *	Returns: none.
 */

/* ARGSUSED */
void
DirPopUpCallback(w,pointer,junk)
Widget w;
caddr_t pointer,junk;
{
  ManpageGlobals * man_globals; 
  MenuCallbackStruct * menu_struct;
  Widget parent;
  int number;
  int current_box;

  menu_struct = (MenuCallbackStruct *) pointer;
  man_globals = (ManpageGlobals *) menu_struct->data;

  number = menu_struct->number;
  current_box = man_globals->current_directory;

  /* We have used this guy, pop down the menu. */
  
  if (number != current_box) {
    /* This is the only one that we know has a parent. */
    parent = XtParent(man_globals->manpagewidgets.box[INITIAL_DIR]);

    MakeDirectoryBox(man_globals, parent,
		     man_globals->manpagewidgets.box + number, number);
    XtUnmanageChild(man_globals->manpagewidgets.box[current_box]);
    XtManageChild(man_globals->manpagewidgets.box[number]);

    XtListUnhighlight(man_globals->manpagewidgets.box[current_box]);
    ChangeLabel(man_globals->label, man_globals->section_name[number]);
    man_globals->current_directory = number;
  }

  XtPopdown( XtParent(XtParent(w)) );

  /* put up directory. */
  if (!man_globals->both_shown) {
    XtUnmanageChild(man_globals->manpagewidgets.manpage);
    XtManageChild(man_globals->manpagewidgets.directory);
  }
}

/*	Function Name: SaveCallback
 *	Description: This is the callback function for the command buttons in
 *                   the save proc.
 *	Arguments: w - the widget activeted.
 *                 global_pointer - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 junk - the callback and closure date are not used.
 *	Returns: none
 */

/* ARGSUSED */

void
SaveCallback(w,global_pointer,junk)
Widget w;
caddr_t global_pointer,junk;
{
  ManpageGlobals * man_globals;
  char str[100];

  man_globals = (ManpageGlobals *) global_pointer;

/* if we aren't canceling then we should save the file */

  if (strcmp(Name(w),CANCEL_FILE_SAVE)) {
    sprintf(str,"%s %s %s",COPY,man_globals->tmpfile,man_globals->filename);

    if(system(str) != 0)		/* execute copy. */
      PrintError("Something went wrong trying to copy temp file to cat dir.");
  }

  XtPopdown( XtParent(XtParent(w)) );
}

/*	Function Name: ManpageButtonPress
 *	Description: This function will toggle put up the directory on
 *                   button release of the right mouse button.
 *	Arguments:  w - the widget activeted.
 *                 global_pointer - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 junk - the callback and closure date are not used.
 *	Returns: none.
 */

/* ARGSUSED */

void
ManpageButtonPress(w,global_pointer,event)
Widget w;
caddr_t global_pointer;
XEvent *event;
{
  ManpageGlobals * man_globals;

  man_globals = (ManpageGlobals *) global_pointer;

  if ( !man_globals->both_shown && event->xbutton.button == 2 &&
      event->type == ButtonPress) {
    /* Put Up Directory */
    ChangeLabel(man_globals->label,
		man_globals->section_name[man_globals->current_directory]);
    XtUnmanageChild(man_globals->manpagewidgets.manpage);
    XtManageChild(man_globals->manpagewidgets.directory);  
    man_globals->dir_shown = TRUE;
  }
}
    
