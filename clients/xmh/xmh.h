/*
 *	rcs_id[] = "$XConsortium: xmh.h,v 2.14 88/09/06 17:23:57 jim Exp $";
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

#ifndef _xmh_h
#define _xmh_h
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/AsciiText.h>
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Dialog.h>
#include <X11/Form.h>
#include <X11/Label.h>
#include <X11/Scroll.h>
#include <X11/Shell.h>
#include <X11/Viewport.h>
#include <X11/VPaned.h>

#define DELETEABORTED	-1
#define MARKPOS		4

#define xMargin 2
#define yMargin 2

#define DEBUG(msg) \
	if (app_resources.debug) \
	    {(void)fprintf(stderr, msg); (void)fflush(stderr);}

#define DEBUG1(msg, arg) \
	if (app_resources.debug) \
	    {(void)fprintf(stderr, msg, arg); (void)fflush(stderr);}

#define DEBUG2(msg, arg1, arg2) \
	if (app_resources.debug) \
	    {(void)fprintf(stderr,msg,arg1,arg2); (void)fflush(stderr);}

typedef int * dp;		/* For debugging. */

typedef FILE* FILEPTR;

typedef struct _ButtonRec *Button;
typedef struct _XmhButtonBoxRec *ButtonBox;
typedef struct _TocRec *Toc;
typedef struct _MsgRec *Msg;
typedef struct _PickRec *Pick;

typedef enum {
    Fignore, Fmove, Fcopy, Fdelete
} FateType;

typedef enum {
    STtocAndView,
    STview,
    STcomp,
    STpick
} ScrnKind;

typedef struct _ScrnRec {
   Widget	parent;		/* The parent widget of the scrn */
   Widget	widget;		/* The pane widget for the scrn */
   int		mapped;		/* TRUE only if we've mapped this screen. */
   ScrnKind	kind;		/* What kind of scrn we have. */
   ButtonBox	folderbuttons;	/* Folder buttons. */
   Button	curfolder;	/* Which is the current folder. */
   ButtonBox	mainbuttons;	/* Main xmh control buttons. */
   Widget	toclabel;	/* Toc titlebar. */
   Widget	tocwidget;	/* Toc text. */
   ButtonBox	tocbuttons;	/* Toc control buttons. */
   ButtonBox 	seqbuttons;	/* Sequence buttons. */
   Button	curseq;		/* Which is the current sequence. */
   Widget	viewlabel;	/* View titlebar. */
   Widget	viewwidget;	/* View text. */
   ButtonBox 	viewbuttons;	/* View control buttons. */
   Toc		toc;		/* The table of contents. */
   Msg		msg;		/* The message being viewed. */
   Pick		pick;		/* Pick in this screen. */
   Msg		assocmsg;	/* Associated message for reply, etc. */
} ScrnRec, *Scrn;


typedef struct {
    int nummsgs;
    Msg *msglist;
} MsgListRec, *MsgList;


typedef struct {
   char		*name;		/* Name of this sequence. */
   MsgList	mlist;		/* Messages in this sequence. */
} SequenceRec, *Sequence;


#include "globals.h"
#include "macros.h"
#include "externs.h"
#include "mlist.h"
#include "bbox.h"
#include "msg.h"
#include "toc.h"

#endif _xmh_h
