/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: menu.c,v 1.3 89/01/06 18:42:20 kit Exp $
 * $oHeader: menu.c,v 4.0 88/08/31 22:12:49 kit Exp $
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
 * Created:   November 2, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: menu.c,v 4.2 88/12/19 13:47:51 kit Exp $";
#endif

/* std headers */

#include <stdio.h>

/* X header files */

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>

/* Toolkit headers */

#include <X11/Command.h>
#include <X11/Label.h>
#include <X11/Box.h>

/* Menu's specific header. */

#include "menu.h"

void exit();			/* Keep Saber happy. */
char * malloc();

/*	Function Name: MakeMenu
 *	Description: This function makes a menu with a  box full of
 *                   command buttons.
 *	Arguments: w - the widget that is the parent of the Menu.
 *                 menu - a pointer to the menu structure.
 *	Returns: none.
 */

Widget
MakeMenu(w,menu,data)
Widget w;
Menu * menu;
caddr_t data;
{
  MenuCallbackStruct * menu_data;
  static Arg arglist[MENUARGS];
  static int i, max_width;
  static Dimension border, h_space;
  static Cardinal num_args;
  Widget box,label;
  static char menu_trans_table[] = "<Expose>:   unhighlight()";
  XtTranslations translations;	/* The translation table to add to the
				   menu buttons.*/

  translations = XtParseTranslationTable(menu_trans_table);

/* Create Box */

  box = XtCreateWidget(menu->name,boxWidgetClass,w,
		       menu->box_args,menu->box_num);

/* make sure there is room in the arglist for the label args. */

  if (menu->label_num + num_args > MENUARGS) {
    printf("Menu: there is not enough space for the label args, recompile\n");
    printf("with larger value for MENUARGS.\n");
    exit(1);
  }

/* set up args for label (no border) */

  num_args = 0;

  XtSetArg(arglist[num_args], XtNborderWidth, 0);
  num_args++;

  num_args = MergeArglists(menu->label_args,menu->label_num,arglist,num_args);

/* create label widget */

  label = XtCreateWidget(menu->name,labelWidgetClass,box,arglist,num_args);
  XtManageChild(label);

  max_width = (int) Width(label);

/* Get hspace of  box, and borderwidth of default. */

  num_args = 0;

  XtSetArg(arglist[num_args], XtNhSpace, &h_space);
  num_args++;

  XtSetArg(arglist[num_args], XtNborderWidth, &border);
  num_args++;

  XtGetValues(box,arglist,num_args);
  
/* Check to see of we have made any changes to the command buttons border
   width. */

  i = 0;
  while ( i < menu->button_num ) {
    if (menu->button_args[i].name == XtNborderWidth) {
      border = (Dimension) menu->button_args[i].value;
      i = 1000;	/* We're done exit loop. */
    }
  i++;
  }

  for (i = 0 ; i < menu->number ; i++) {
    Widget command;
    int width;

/* define width of buttons */

    if (menu->buttons[i].number_per_line == 0) 
      width = 0;		/* label widget uses default. */
    else 
      width = ( max_width - ( ( h_space + 2 * border) *
			     (menu->buttons[i].number_per_line - 1) )
	       ) / menu->buttons[i].number_per_line ;

/*
 * All buttons will call back to the function menu->callback;
 */

    menu_data = (MenuCallbackStruct *) malloc(sizeof(MenuCallbackStruct));
    if (menu_data == NULL) {
      fprintf(stderr,"Out of memory in menu.c\n");
      exit(1);
    }
    menu_data->data = data;
    menu_data->number = i;

/* Set and merge arglists */

    num_args = 0;
    XtSetArg(arglist[num_args], XtNwidth, width);
    num_args++; 

    num_args = MergeArglists(menu->button_args,menu->button_num,
			     arglist,num_args);

    if ( (num_args + menu->button_num) >= MENUARGS ) {
      printf("Menu: there is not enough space for the label args,\n");
      printf("recompile with larger value for MENUARGS.\n");
      exit(1);
    }
      
    command = XtCreateManagedWidget(menu->buttons[i].name,
				    commandWidgetClass, box,
				    arglist, num_args);
    XtAddCallback(command, XtNcallback, menu->callback, (caddr_t) menu_data);
    XtAddCallback(command, XtNdestroyCallback, ButtonDestroyed, 
		  (caddr_t) menu_data);
    XtOverrideTranslations(command, translations);
  }
  return(box);
}


/*	Function Name: MergeArglists
 *	Description: This function merges two arglists.
 *	Arguments: from,num_from - the number and list of args for the source.
 *                 to,num_to - the number and list of argument for the
 *                             destination.
 *	Returns: new number of argument in to.
 */

/* Note: This function will be very unhappy with you if 'to' is not
 *       large enough to contain 'from', you will end up with pointers 
 *       in space.
 */

int
MergeArglists(from,num_from,to,num_to)
Arg from[],to[];
Cardinal num_from,num_to;
{
  int i,j;			/* a counter. */

/* When there are two similar values ignore the from value. */

  i = 0;
  while ( i < num_from ) {	
    j = 0;
    while ( j < num_to ) {
      if ( !strcmp(from[i].name,to[j].name) ) {
	/* if they are the same then goto next on the from list,
	   i.e. ignore this entry, do not add to to list. */
	i++;
	if ( i > num_from)
	  j = num_to + 100;
	else {
	  j = 0;
	  continue;
	}
      }
      j++;
    }
    /* add to to list */
    if (j < num_to + 100) {
      to[num_to].value = from[i].value;
      to[num_to].name = from[i].name;
      num_to++;
      i++;
    }
  }
  return(num_to);
}

/*	Function Name: ButtonDestroyed
 *	Description: This function frees the MenuCallbackStructure.
 *	Arguments:  w - the widget that has been destroyed.
 *                  pointer - a pointer to some data to free.
 *                  junk - nothing useful.
 *	Returns: 
 */

/* ARGSUSED */

void
ButtonDestroyed(w,pointer,junk)
Widget w;
caddr_t pointer,junk;
{
  free(pointer);
}
