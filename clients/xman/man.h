/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: man.h,v 1.3 89/01/06 18:42:18 kit Exp $
 * $Athena: man.h,v 4.6 89/01/06 12:17:38 kit Exp $
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

/* Std system and C header files */

#include <stdio.h>
#include <X11/Xos.h>
#include <sys/dir.h>
#include <sys/stat.h>

/* X include files */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

/* X toolkit header files */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* Widget header files. */

#include <X11/AsciiText.h>
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Label.h>
#include <X11/List.h>
#include <X11/Scroll.h>
#include <X11/Shell.h>
#include <X11/VPaned.h>
#include <X11/Viewport.h>

/* program specific header files. */

#include "ScrollByL.h"

#include "version.h"
#include "defs.h"
#include "menu.h"

typedef void (*fcall)();	/* function pointer typedef */

typedef struct _XmanFonts {
  XFontStruct * bold,		/* The bold text for the man pages. */
    * normal,			/* The normal text for the man pages. */
    * italic,			/* The italic text for the man pages. */
    * directory;		/* The font for the directory.  */
} XmanFonts;

typedef struct _XmanCursors {
  Cursor top,			/* The top Cursor, default for xman. */
    help,			/* The top cursor for the help menu. */
    manpage,			/* The cursor for the Manpage. */
    search_entry;		/* The cursor for the text widget in the
				   search box.*/
} XmanCursors;

typedef struct _ManPageWidgets {
  Widget manpage,		/* The manual page window (scrolled) */
    directory,			/* The widget in which all directories will
				   appear. */
    box[MAXSECT];		/* The boxes containing the sections. */
} ManPageWidgets;

/*
 * The manual sections and entries
 */

typedef struct tManual {
  char * blabel;		/* The button label. */
  char ** entries;		/* the individual man page file names. */
  int nentries;			/* how many */
} Manual;

/* 
 * The structure with the information about the file
 */

typedef struct _MemoryStruct {
  char * top_of_page;		/* a pointer to the top of the page. */
  char ** top_line;		/* a pointer to the top of the list of
				   lines in the file. */
} MemoryStruct;
   
/* psuedo Globals that are specific to each manpage created. */

typedef struct _ManpageGlobals{
  int current_directory;	/* The directory currently being shown 
				   on this manpage. */
  Boolean dir_shown,		/* True if the directory is then current
				   visable screen */
    both_shown;			/* If true then both the manpage and
				   the directory are to be shown.*/
  Widget label,			/* The label widget at the top of the page. */
    standby1,			/* The two please standby widgets with text */
    standby2,			/*   in them that need to be painted. */
    both_shown_button,		/* The both_shown widget itself. */
    help_button,		/* The help button. */
    put_up_manpage,		/* The button that puts up the manpage. */
    put_up_directory,		/* The button that puts up the directory. */
    text_widget;		/* text widget containing search string. */
  char search_string[SEARCH_STRING_LENGTH];	/* The search string. */
  char manpage_title[80];	/* The label to use for the current manpage. */
  char filename[80];		/* the name of the file that we are
				   currently looking at.*/
  char tmpfile[80];		/* the name of the file in /tmp that
				   we are currently using. */
  char * section_name[MAXSECT];	/* The name of each of the sections */

  ManPageWidgets manpagewidgets; /* The manpage widgets. */

  MemoryStruct * memory_struct; /* The memory struct */

  /* Things to remember when cleaning up whne killing manpage. */

  Widget This_Manpage;		/* a pointer to the root of
				   this manpage. */

} ManpageGlobals;


/* Resource manager sets these. */

typedef struct _Xman_Resources {
  XmanFonts fonts;		/* The fonts used for the man pages. */
  XmanCursors cursors;		/* The cursors for xman. */
  Boolean both_shown_initial;	/* The initial state of the manual pages
				   show two screens or only one. */
  Boolean top_box_active;	        /* Put up the Top Box. */
  int directory_height;	        /* The default height of directory in 
				   both_shown mode. */
  char * help_file;		/* The name of the help file. */
} Xman_Resources;
