/*
 *	rcs_id[] = "$XConsortium: xedit.h,v 1.15 88/10/18 13:31:07 swick Exp $";
 */

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


#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Dialog.h>
#include <X11/Label.h>
#include <X11/Scroll.h>
#include <X11/AsciiText.h>
#include <X11/Text.h>
#include <X11/TextSrcP.h>
#include <X11/VPaned.h>
#include <X11/Viewport.h>
#include <X11/Cardinals.h>


#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define Feep()			XBell(CurDpy, 50)

#define MakeArg(n, v){  	args[numargs].name = n; 	\
			        args[numargs].value = v;	\
			        numargs++; 			\
		      }


/*	misc externs 	*/
extern XtTextSource TCreateISSource();
extern XtTextSource CreatePSource();
extern XtTextSource TCreateApAsSource();
extern DestroyPSource();
extern PSchanges();
extern TDestroyApAsSource();
extern char *malloc();
extern char *realloc();
extern char *calloc();


/*	externs in xedit.c	*/
extern Widget  searchstringwindow;
extern Widget editbutton;
extern char *filename;
extern char *savedfile;
extern char *loadedfile;
extern Editable;
extern backedup;
extern saved;
extern lastChangeNumber;
extern Widget Row1;
extern char *searchstring;
extern char *replacestring;
extern Widget toplevel;
extern Widget outer;
extern Widget textwindow;
extern Widget messwidget;
extern Widget labelwindow;
extern XtTextSource source, asource, dsource, psource, messsource;
extern XtTextSource PseudoDiskSourceCreate();
extern void PseudoDiskSourceDestroy();

extern struct _app_resources {
    int editInPlace;
    int enableBackups;
    char *backupNamePrefix;
    char *backupNameSuffix;
} app_resources;

extern Display *CurDpy;


/*	externals in util.c 	*/
extern DoLine();
extern DoJump();
extern XeditPrintf();
extern setWidgetValue();
extern getWidgetValue();
extern Widget makeCommandButton();
extern Widget makeBooleanButton();
extern Widget makeStringBox();
extern FixScreen();

/*	externs in commands.c 	*/
extern DoQuit();
extern DoReplaceOne();
extern DoReplaceAll();
extern DoSearchRight();
extern DoSearchLeft();
extern DoUndo();
extern DoUndoMore();
extern DoSave();
extern DoLoad();
extern DoEdit();
