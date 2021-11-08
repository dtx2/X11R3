#ifndef lint
static char rcs_id[] =
    "$XConsortium: screen.c,v 2.30 88/10/18 13:32:23 swick Exp $";
#endif lint
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

/* scrn.c -- management of scrns. */

#include "xmh.h"

/* Fill in the buttons for the view commands. */

static FillViewButtons(scrn)
Scrn scrn;
{
    extern void ExecCloseView(), ExecViewReply(), ExecViewForward();
    extern void ExecViewUseAsComposition(), ExecEditView();
    extern void ExecSaveView(), ExecPrintView();
    ButtonBox buttonbox = scrn->viewbuttons;
    BBoxStopUpdate(buttonbox);
    if (scrn->tocwidget == NULL)
	BBoxAddButton(buttonbox, "close", ExecCloseView, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "reply", ExecViewReply, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "forward", ExecViewForward, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "useAsComp", ExecViewUseAsComposition,
		  999, TRUE, NULL);
    BBoxAddButton(buttonbox, "edit", ExecEditView, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "save", ExecSaveView, 999, FALSE, NULL);
    BBoxAddButton(buttonbox, "print", ExecPrintView, 999, TRUE, NULL);
    BBoxStartUpdate(buttonbox);
}
    


static FillCompButtons(scrn)
Scrn scrn;
{
    extern void ExecCloseView();
    extern void ExecCompReset();
    extern void ExecComposeMessage();
    extern void ExecSaveDraft();
    extern void ExecSendDraft();
    extern void ExecMsgInsertAssoc();
    ButtonBox buttonbox = scrn->viewbuttons;
    BBoxStopUpdate(buttonbox);
    if (scrn->tocwidget == NULL)
	BBoxAddButton(buttonbox, "close", ExecCloseView, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "send", ExecSendDraft, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "reset", ExecCompReset, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "compose", ExecComposeMessage, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "save", ExecSaveDraft, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "insert", ExecMsgInsertAssoc, 999, TRUE, NULL);
    BBoxStartUpdate(buttonbox);
}


/* Figure out which buttons should and shouldn't be enabled in the given
   screen.  This should be called whenever something major happens to the
   screen. */

#define SetButton(buttonbox, name, value) \
    if (value) BBoxEnable(BBoxFindButtonNamed(buttonbox, name)); \
    else BBoxDisable(BBoxFindButtonNamed(buttonbox, name));

void EnableProperButtons(scrn)
Scrn scrn;
{
    int value, changed, reapable;
    if (scrn) {
	switch (scrn->kind) {
	  case STtocAndView:
	    SetButton(scrn->tocbuttons, "inc", TocCanIncorporate(scrn->toc));
	    value = TocHasSequences(scrn->toc);
	    SetButton(scrn->tocbuttons, "openSeq", value);
	    SetButton(scrn->tocbuttons, "addToSeq", value);
	    SetButton(scrn->tocbuttons, "removeFromSeq", value);
	    SetButton(scrn->tocbuttons, "deleteSeq", value);
	    /* Fall through */

	  case STview:
	    value = (scrn->msg != NULL && !MsgGetEditable(scrn->msg));
	    SetButton(scrn->viewbuttons, "edit", value);
	    SetButton(scrn->viewbuttons, "save", scrn->msg != NULL && !value);
	    break;

	  case STcomp:
	    if (scrn->msg != NULL) {
		changed = MsgChanged(scrn->msg);
		reapable = MsgGetReapable(scrn->msg);
		SetButton(scrn->viewbuttons, "send", changed || !reapable);
		SetButton(scrn->viewbuttons, "save", changed || reapable);
		SetButton(scrn->viewbuttons, "insert",
			  scrn->assocmsg != NULL ? TRUE : FALSE);
		if (!changed) MsgSetCallOnChange(scrn->msg,
						 EnableProperButtons,
						 (caddr_t) scrn);
		else MsgClearCallOnChange(scrn->msg);
	    } else {
		SetButton(scrn->viewbuttons, "send", FALSE);
		SetButton(scrn->viewbuttons, "save", FALSE);
		SetButton(scrn->viewbuttons, "insert", FALSE);
	    }
	    break;
	}
    }
}



/* Create subwidgets for a toc&view window. */

static MakeTocAndView(scrn)
Scrn scrn;
{
    extern void ExecCloseScrn();
    extern void ExecComposeMessage();
    extern void ExecOpenFolder();
    extern void ExecOpenFolderInNewWindow();
    extern void ExecCreateFolder();
    extern void ExecDeleteFolder();
    extern void ExecIncorporate();
    extern void ExecNextView();
    extern void ExecPrevView();
    extern void ExecMarkDelete();
    extern void ExecMarkMove();
    extern void ExecMarkCopy();
    extern void ExecMarkUnmarked();
    extern void ExecViewNew();
    extern void ExecTocReply();
    extern void ExecTocForward();
    extern void ExecTocUseAsComposition();
    extern void ExecCommitChanges();
    extern void ExecOpenSeq();
    extern void ExecAddToSeq();
    extern void ExecRemoveFromSeq();
    extern void ExecPick();
    extern void ExecDeleteSeq();
    extern void ExecPrintMessages();
    extern void ExecPack();
    extern void ExecSort();
    extern void ExecForceRescan();
    int i, theight, min, max;
    ButtonBox buttonbox;
    static char *extra[] = {
	"<Btn1Down>(2): open-folder()",
	NULL
    };
    static char *extra2[] = {
	"<Btn1Down>(2): open-sequence()",
	NULL
    };
    static XtTextSelectType sarray[] = {XtselectLine,
					XtselectPosition,
					XtselectWord,
					XtselectAll,
					XtselectNull};
/*    static Arg arglist2[] = {
 *	{XtNselectionArray, (XtArgVal) sarray},
 *	{XtNselectionArrayCount, (XtArgVal) XtNumber(sarray)}
 */

    scrn->folderbuttons = BBoxRadioCreate(scrn, 0, "folders",
					  &(scrn->curfolder));
    scrn->mainbuttons = BBoxCreate(scrn, 1, "folderButtons");
    scrn->toclabel = CreateTitleBar(scrn, 2);
    scrn->tocwidget = CreateTextSW(scrn, 3, "toc", 0);
/* %%%				   arglist2, XtNumber(arglist2)); */
    scrn->seqbuttons = BBoxRadioCreate(scrn, 4, "seqButtons", &scrn->curseq);
    scrn->tocbuttons = BBoxCreate(scrn, 5, "tocButtons");
    scrn->viewlabel = CreateTitleBar(scrn, 6);
    scrn->viewwidget = CreateTextSW(scrn, 7, "view", wordBreak);
    scrn->viewbuttons = BBoxCreate(scrn, 8, "viewButtons");

    buttonbox = scrn->folderbuttons;
    BBoxStopUpdate(buttonbox);
    for (i=0 ; i<numFolders ; i++)
      BBoxAddButton(buttonbox, TocName(folderList[i]), NoOp, 999, TRUE, extra);
    BBoxStartUpdate(buttonbox);

    buttonbox = scrn->mainbuttons;
    BBoxStopUpdate(buttonbox);
    BBoxAddButton(buttonbox, "close", ExecCloseScrn, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "compose", ExecComposeMessage, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "open", ExecOpenFolder, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "openInNew", ExecOpenFolderInNewWindow,
		  999, TRUE, NULL);
    BBoxAddButton(buttonbox, "create", ExecCreateFolder, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "delete", ExecDeleteFolder, 999, TRUE, NULL);
    BBoxStartUpdate(buttonbox);

    buttonbox = scrn->seqbuttons;
    BBoxStopUpdate(buttonbox);
    BBoxAddButton(buttonbox, "all", NoOp, 999, TRUE, extra2);
    BBoxStartUpdate(buttonbox);

    XtTextSetSelectionArray(scrn->tocwidget, sarray);

    buttonbox = scrn->tocbuttons;
    BBoxStopUpdate(buttonbox);
    BBoxAddButton(buttonbox, "inc", ExecIncorporate, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "next", ExecNextView, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "prev", ExecPrevView, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "delete", ExecMarkDelete, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "move", ExecMarkMove, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "copy", ExecMarkCopy, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "unmark", ExecMarkUnmarked, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "viewNew", ExecViewNew, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "reply", ExecTocReply, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "forward", ExecTocForward, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "useAsComp", ExecTocUseAsComposition,
		  999, TRUE, NULL);
    BBoxAddButton(buttonbox, "commit", ExecCommitChanges, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "print", ExecPrintMessages, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "pack", ExecPack, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "sort", ExecSort, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "rescan", ExecForceRescan, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "pick", ExecPick, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "openSeq", ExecOpenSeq, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "addToSeq", ExecAddToSeq, 999, TRUE, NULL);
    BBoxAddButton(buttonbox, "removeFromSeq", ExecRemoveFromSeq,
		  999, TRUE, NULL);
    BBoxAddButton(buttonbox, "deleteSeq", ExecDeleteSeq, 999, TRUE, NULL);
    BBoxStartUpdate(buttonbox);

    FillViewButtons(scrn);

    BBoxLockSize(scrn->folderbuttons);
    BBoxLockSize(scrn->mainbuttons);
    BBoxLockSize(scrn->seqbuttons);
    BBoxLockSize(scrn->tocbuttons);
    BBoxLockSize(scrn->viewbuttons);

    if (app_resources.mailWaitingFlag) {
	static Arg arglist[] = {XtNiconPixmap, NULL};
	arglist[0].value = (XtArgVal) NoMailPixmap;
	XtSetValues(scrn->parent, arglist, XtNumber(arglist));
    }

    XtRealizeWidget(scrn->parent);

    theight = GetHeight((Widget)scrn->tocwidget) +
	GetHeight((Widget)scrn->viewwidget);
    theight = app_resources.defTocPercentage * theight / 100;
    XtPanedGetMinMax((Widget) scrn->tocwidget, &min, &max);
    XtPanedSetMinMax((Widget) scrn->tocwidget, theight, theight);
    XtPanedSetMinMax((Widget) scrn->tocwidget, min, max);
}


MakeView(scrn)
Scrn scrn;
{
    scrn->viewlabel = CreateTitleBar(scrn, 0);
    scrn->viewwidget = CreateTextSW(scrn, 1, "view", wordBreak);
    scrn->viewbuttons = BBoxCreate(scrn, 2, "viewButtons");
    FillViewButtons(scrn);
}


MakeComp(scrn)
Scrn scrn;
{
    scrn->viewlabel = CreateTitleBar(scrn, 0);
    scrn->viewwidget = CreateTextSW(scrn, 1, "comp", wordBreak);
    scrn->viewbuttons = BBoxCreate(scrn, 2, "compButtons");
    FillCompButtons(scrn);
}


/* Create a scrn of the given type. */

Scrn CreateNewScrn(kind)
ScrnKind kind;
{
    int i;
    Position x, y;
    Scrn scrn;
    static Arg arglist[] = {
	{XtNgeometry, NULL},
	{XtNinput, (XtArgVal)True},
    };

    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i]->kind == kind && !scrnList[i]->mapped)
	    return scrnList[i];
    switch (kind) {
       case STtocAndView: arglist[0].value =
			   (XtArgVal)app_resources.defTocGeometry;	break;
       case STview:	  arglist[0].value =
			   (XtArgVal)app_resources.defViewGeometry;	break;
       case STcomp:	  arglist[0].value =
			   (XtArgVal)app_resources.defCompGeometry;	break;
       case STpick:	  arglist[0].value =
			   (XtArgVal)app_resources.defPickGeometry;	break;
    }

    numScrns++;
    scrnList = (Scrn *)
	XtRealloc((char *) scrnList, (unsigned) numScrns*sizeof(Scrn));
    scrn = scrnList[numScrns - 1] = XtNew(ScrnRec);
    bzero((char *)scrn, sizeof(ScrnRec));
    scrn->kind = kind;
    if (numScrns == 1) scrn->parent = toplevel;
    else scrn->parent = XtCreatePopupShell(
				   progName, topLevelShellWidgetClass,
				   toplevel, arglist, XtNumber(arglist));
    scrn->widget =
	XtCreateManagedWidget(progName, vPanedWidgetClass, scrn->parent,
			      NULL, (Cardinal)0);

    switch (kind) {
	case STtocAndView:	MakeTocAndView(scrn);	break;
	case STview:		MakeView(scrn);	break;
	case STcomp:		MakeComp(scrn);	break;
    }

    if (kind != STpick) {
	DEBUG("Realizing...")
	XtRealizeWidget(scrn->parent);
	DEBUG(" done.\n")
	XtSetKeyboardFocus( scrn->parent, scrn->viewwidget );
	XDefineCursor( theDisplay, XtWindow(scrn->parent),
		       app_resources.cursor );
    }
    scrn->mapped = (numScrns == 1);
    return scrn;
}


Scrn NewViewScrn()
{
    return CreateNewScrn(STview);
}

Scrn NewCompScrn()
{
    Scrn scrn;
    scrn = CreateNewScrn(STcomp);
    scrn->assocmsg = (Msg)NULL;
    return scrn;
}

void ScreenSetAssocMsg(scrn, msg)
  Scrn scrn;
  Msg msg;
{
    scrn->assocmsg = msg;
}

/* Destroy the screen.  If unsaved changes are in a msg, too bad. */

void DestroyScrn(scrn)
  Scrn scrn;
{
    XtPopdown(scrn->parent);
    DestroyConfirmWidget();
    TocSetScrn((Toc) NULL, scrn);
    MsgSetScrnForce((Msg) NULL, scrn);
    scrn->mapped = FALSE;
}



void MapScrn(scrn)
Scrn scrn;
{
    if (!scrn->mapped) {
	XtPopup(scrn->parent, XtGrabNone);
	scrn->mapped = TRUE;
    }
}


Scrn ScrnFromWidget(w)
Widget w;
{
    int i;
    while (w && XtClass(w) != vPanedWidgetClass)
	w = XtParent(w);
    if (w) {
	for (i=0 ; i<numScrns ; i++) {
	    if (w == (Widget) scrnList[i]->widget)
		return scrnList[i];
	}
    }
    Punt("ScrnFromWidget failed!");
}
