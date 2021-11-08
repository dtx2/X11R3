/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: menu.h,v 1.2 88/09/06 17:48:16 jim Exp $
 * $Athena: menu.h,v 4.0 88/08/31 22:12:57 kit Exp $
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

/* defined the functions */

Widget MakeMenu();
int MergeArglists();
void ButtonDestroyed();

#define MENUARGS 20		/* You may have to watch this one. */

typedef void (*menufunctioncall)();	/* function pointer typedef */

typedef struct _Button {
  char * name;			/* The name of this button. */
  int number_per_line;		/* The size of this button. */
} Button;

typedef struct _Menu {
  char * name;			/* The name of this menu. */
  Arg * box_args,		/* The specific args for the box. */
    * label_args,		/* The specific args for the label. */
    * button_args;		/* The specific args for the buttons. */
  int number;			/* Then number of buttons. */
  Cardinal box_num,		/* The number of args for the box. */
    label_num,			/* The number of args for the label. */
    button_num;			/* The number of args for the buttons. */
  Button * buttons;		/* The button structure for the buttons. */
  menufunctioncall callback;	/* The callback function for this menu. */
} Menu;

typedef struct _MenuCallbackStruct {
  int number;			/* The callback identifier. */
  caddr_t data;			/* Other data to hang off this sucker. */
} MenuCallbackStruct;
