#ifndef lint
static char rcs_id[] = "$XConsortium: pick.c,v 2.23 88/09/06 17:23:28 jim Exp $";
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

/* pick.c -- handle a pick subwidget. */

#include "xmh.h"

#define WTlabel		labelWidgetClass
#define WTbutton	commandWidgetClass
#define WTtextentry	asciiStringWidgetClass

#define	RTfrom		0
#define	RTto		1
#define	RTcc		2
#define RTdate		3
#define	RTsubject	4
#define	RTsearch	5
#define	RTother		6
#define	RTignore	7

#define FIRSTROWTYPE		RTfrom
#define LASTUSEFULROWTYPE	RTother
#define NUMROWTYPE		(RTignore+1)

static int stdwidth = -1;	/* Width to make text fields, and other
				   things that want to be the same width as
				   text fields. */

static char *TypeName[NUMROWTYPE];

typedef struct {
   WidgetClass	type;		/* Encode what type of Widget this is. */
   Widget 	widget;		/* The widget id itself. */
   struct _RowListRec *row;	/* Which row this widget is in. */
   short	hilite;		/* Whether to hilight (if button subwidget) */
   char		*ptr;		/* Data (if text subwidget) */
} FormEntryRec, *FormEntry;

typedef struct _RowListRec {
   short	type;		/* Encode what type of list this is. */
   Widget	widget;		/* Widget containing this row */
   short	numwidgets;	/* How many widgets in this list. */
   FormEntry 	*wlist;		/* List of widgets. */
   struct _GroupRec *group;	/* Which group this is in. */
} RowListRec, *RowList;

typedef struct _GroupRec {
   short	 numrows;	/* How many rows of widget. */
   Widget	widget;		/* Widget containing this group */
   RowList	*rlist;		/* List of widget rows. */
   struct _FormBoxRec *form;	/* Which form this is in. */
} GroupRec, *Group;

typedef struct _FormBoxRec {
   Widget outer;	/* Outer widget (contains scrollbars if any) */
   Widget inner;	/* Inner widget (contains master form) */
   short numgroups;	/* How many groups of form entries we have. */
   Group *glist;	/* List of form groups. */
   struct _PickRec *pick; /* Which pick this is in. */
} FormBoxRec, *FormBox;

typedef struct _PickRec {
   Scrn scrn;			/* Scrn containing this pick. */
   Widget label;		/* Widget with label for this pick. */
   Toc toc;			/* Toc for folder being scanned. */
   FormBox general;		/* Form for general info about this pick. */
   FormBox details;		/* Form for details about this pick. */
   Widget errorwidget;		/* Pop-up error widget. */
} PickRec;


static FormEntry CreateWidget();

InitPick()
{
    TypeName[RTfrom]	= "From:";
    TypeName[RTto]	= "To:";
    TypeName[RTcc]	= "Cc:";
    TypeName[RTdate]	= "Date:";
    TypeName[RTsubject] = "Subject:";
    TypeName[RTsearch]	= "Search:";
    TypeName[RTother]	= NULL;
}

static PickFlipColors(widget)
Widget widget;
{
    static Arg arglist[] = {
	{XtNforeground, NULL},
	{XtNbackground, NULL},
    };
    Pixel foreground, background;

    arglist[0].value = (XtArgVal)&foreground;
    arglist[1].value = (XtArgVal)&background;
    XtGetValues(widget, arglist, XtNumber(arglist));
    arglist[0].value = (XtArgVal)background;
    arglist[1].value = (XtArgVal)foreground;
    XtSetValues(widget, arglist, XtNumber(arglist));
}


static PrepareToUpdate(form)
  FormBox form;
{
    XtFormDoLayout(form->inner, FALSE);
}

static ExecuteUpdate(form)
  FormBox form;
{
    XtFormDoLayout(form->inner, TRUE);
    XtManageChild(form->inner);
    XtManageChild(form->outer);
}

static void AddLabel(row, text, usestd)
  RowList row;
  char *text;
  int usestd;
{
    Widget widget;
    static Arg arglist[] = {
	{XtNlabel, NULL},
	{XtNborderWidth, (XtArgVal) 0},
	{XtNjustify, (XtArgVal) XtJustifyRight},
	{XtNwidth, (XtArgVal) NULL}
    };

    arglist[0].value = (XtArgVal) text;
    arglist[XtNumber(arglist) - 1].value = (XtArgVal) stdwidth;
    CreateWidget(row, WTlabel, arglist,
		 usestd ? XtNumber(arglist) : XtNumber(arglist) - 1);
}


static void AddButton(row, text, func, hilite)
  RowList row;
  char *text;
  void (*func)();
  int hilite;
{
    FormEntry entry;
    static Arg args[] = {
	{XtNlabel, NULL},
    };

    args[0].value = (XtArgVal)text;
    entry = CreateWidget( row, WTbutton, args, XtNumber(args) );
    XtAddCallback( entry->widget, XtNcallback, func, (caddr_t)entry );
    entry->hilite = hilite;
    if (hilite) PickFlipColors(entry->widget);
}


static void AddTextEntry(row, str)
  RowList row;
  char *str;
{
    static Arg arglist[] = {
	{XtNstring, (XtArgVal) NULL},
	{XtNwidth, (XtArgVal) NULL},
	{XtNlength, (XtArgVal) 300},
	{XtNtextOptions, (XtArgVal)(resizeWidth | resizeHeight)},
	{XtNeditType, (XtArgVal)XttextEdit},
    };
    char *ptr;
    FormEntry entry;
    ptr = XtMalloc(310);
    arglist[0].value = (XtArgVal) ptr;
    arglist[1].value = (XtArgVal) stdwidth;
    (void) strcpy(ptr, str);
    entry = CreateWidget( row, WTtextentry, arglist, XtNumber(arglist) );
    entry->ptr = ptr;
}


static void ChangeTextEntry(entry, str)
FormEntry entry;
char *str;
{
    static Arg arglist[] = {
	{XtNtextSource, (XtArgVal) NULL}
    };
    Arg arglist2[3];
    XtTextSource source;
    if (strcmp(str, entry->ptr) == 0) return;
    arglist[0].value = (XtArgVal)&source;
    XtGetValues(entry->widget, arglist, XtNumber(arglist));
    XtStringSourceDestroy(source);
    (void) strcpy(entry->ptr, str);
    XtSetArg( arglist2[0], XtNstring, entry->ptr );
    XtSetArg( arglist2[1], XtNlength, 300 );
    XtSetArg( arglist2[2], XtNeditType, XttextEdit );
    source = XtStringSourceCreate(entry->widget, arglist2, XtNumber(arglist2));
    XtTextSetSource(entry->widget, source, (XtTextPosition) 0);
}


/* ARGSUSED */
static void ExecYesNo(w, closure, call_data)
    Widget w;			/* unused */
    caddr_t closure;		/* FormEntry */
    caddr_t call_data;		/* unused */
{
    FormEntry entry = (FormEntry)closure;
    RowList row = entry->row;
    int i;
    if (!entry->hilite) {
	entry->hilite = TRUE;
	PickFlipColors(entry->widget);
	for (i = 0; i < row->numwidgets; i++)
	    if (entry == row->wlist[i])
		break;
	if (i > 0 && row->wlist[i-1]->type == WTbutton)
	    i--;
	else
	    i++;
	entry = row->wlist[i];
	entry->hilite = FALSE;
	PickFlipColors(entry->widget);
    }
}



/* ARGSUSED */
static void ExecRowOr(w, closure, call_data)
    Widget w;			/* unused */
    caddr_t closure;		/* FormEntry */
    caddr_t call_data;		/* unused */
{
    FormEntry entry = (FormEntry)closure;
    RowList row = entry->row;
    FormBox form = row->group->form;
    PrepareToUpdate(form);
    DeleteWidget(entry);
    AddLabel(row, "or", FALSE);
    AddTextEntry(row, "");
    AddButton(row, "Or", ExecRowOr, FALSE);
    ExecuteUpdate(form);
}
    

/* ARGSUSED */
static void ExecGroupOr(w, closure, call_data)
    Widget w;			/* unused */
    caddr_t closure;		/* FormEntry */
    caddr_t call_data;		/* unused */
{
    FormBox form = ((FormEntry)closure)->row->group->form;
/* %%%    XUnmapWindow(theDisplay, XtWindow(form->inner)); */
    PrepareToUpdate(form);
    AddDetailGroup(form);
    ExecuteUpdate(form);
/* %%%    XtMapWidget(form->inner); */
}

static char **argv;
static int argvsize;


static AppendArgv(ptr)
  char *ptr;
{
    argvsize++;
    argv = ResizeArgv(argv, argvsize);
    argv[argvsize - 1] = MallocACopy(ptr);
}

static EraseLast()
{
    argvsize--;
    XtFree((char *) argv[argvsize]);
    argv[argvsize] = 0;
}



static ParseRow(row)
  RowList row;
{
    int     result = FALSE;
    int i;
    FormEntry entry;
    char   str[1000];
    if (row->type > LASTUSEFULROWTYPE)
	return FALSE;
    for (i = 3; i < row->numwidgets; i += 2) {
	entry = row->wlist[i];
	if (*(entry->ptr)) {
	    if (!result) {
		result = TRUE;
		if (row->wlist[1]->hilite)
		    AppendArgv("-not");
		AppendArgv("-lbrace");
	    }
	    switch (row->type) {
		case RTfrom: 
		    AppendArgv("-from");
		    break;
		case RTto: 
		    AppendArgv("-to");
		    break;
		case RTcc: 
		    AppendArgv("-cc");
		    break;
		case RTdate: 
		    AppendArgv("-date");
		    break;
		case RTsubject: 
		    AppendArgv("-subject");
		    break;
		case RTsearch: 
		    AppendArgv("-search");
		    break;
		case RTother: 
		    AppendArgv(sprintf(str, "--%s", row->wlist[2]->ptr));
		    break;
	    }
	    AppendArgv(entry->ptr);
	    AppendArgv("-or");
	}
    }
    if (result) {
	EraseLast();
	AppendArgv("-rbrace");
	AppendArgv("-and");
    }
    return result;
}
	    

static ParseGroup(group)
  Group group;
{
    int found = FALSE;
    int i;
    for (i=0 ; i<group->numrows ; i++)
	found |= ParseRow(group->rlist[i]);
    if (found) {
	EraseLast();
	AppendArgv("-rbrace");
	AppendArgv("-or");
	AppendArgv("-lbrace");
    }
    return found;
}



static void DestroyErrorWidget(w, client_data, call_data)
    Widget w;			/* unused */
    caddr_t client_data;	/* Pick */
    caddr_t call_data;		/* unused */
{
    Pick pick = (Pick)client_data;
    if (pick->errorwidget) {
	XtDestroyWidget(pick->errorwidget);
	pick->errorwidget = NULL;
    }
}

static void MakeErrorWidget(pick, str)
Pick pick;
char *str;
{
    Arg args[1];

    DestroyErrorWidget(NULL, (caddr_t)pick, NULL);
    XtSetArg( args[0], XtNlabel, str );
    pick->errorwidget = XtCreateWidget( "error", dialogWidgetClass,
				        pick->scrn->widget,
				        args, XtNumber(args) );

    XtDialogAddButton( pick->errorwidget, "OK",
		       DestroyErrorWidget, (caddr_t)pick );

    XtRealizeWidget( pick->errorwidget );
    CenterWidget( pick->scrn->widget, pick->errorwidget );
    XtMapWidget( pick->errorwidget );
}


/* ARGSUSED */
static void ExecOK(w, closure, call_data)
    Widget w;			/* unused */
    caddr_t closure;		/* FormEntry */
    caddr_t call_data;		/* unused */
{
    Pick pick = ((FormEntry)closure)->row->group->form->pick;
    Toc toc = pick->toc;
    FormBox details = pick->details;
    FormBox general = pick->general;
    Group group = general->glist[0];
    RowList row0 = group->rlist[0];
    RowList row1 = group->rlist[1];
    RowList row2 = group->rlist[2];
    char *fromseq = row0->wlist[3]->ptr;
    char *toseq = row0->wlist[1]->ptr;
    char *fromdate = row1->wlist[1]->ptr;
    char *todate = row1->wlist[3]->ptr;
    char *datefield = row1->wlist[5]->ptr;
    short removeoldmsgs = row2->wlist[1]->hilite;
    char str[1000];
    int i, found;
    char *folderpath;

    DestroyErrorWidget(NULL, (caddr_t)pick, NULL);
    if (strcmp(toseq, "all") == 0) {
	MakeErrorWidget(pick, "Can't create a sequence called \"all\".");
	return;
    }
    if (TocGetSeqNamed(toc, fromseq) == NULL) {
	(void) sprintf(str, "Sequence \"%s\" doesn't exist!", fromseq);
	MakeErrorWidget(pick, str);
	return;
    }
    argv = MakeArgv(1);
    argvsize = 0;
    AppendArgv("pick");
    AppendArgv(folderpath = TocMakeFolderName(toc));
    XtFree(folderpath);
    AppendArgv(fromseq);
    AppendArgv("-sequence");
    AppendArgv(toseq);
    if (removeoldmsgs)
	AppendArgv("-zero");
    else
	AppendArgv("-nozero");
    if (*datefield) {
	AppendArgv("-datefield");
	AppendArgv(datefield);
    }
    if (*fromdate) {
	AppendArgv("-after");
	AppendArgv(fromdate);
	AppendArgv("-and");
    }
    if (*todate) {
	AppendArgv("-before");
	AppendArgv(todate);
	AppendArgv("-and");
    }
    found = FALSE;
    AppendArgv("-lbrace");
    AppendArgv("-lbrace");
    for (i=0 ; i<details->numgroups ; i++)
	found |= ParseGroup(details->glist[i]);
    EraseLast();
    EraseLast();
    if (found) AppendArgv("-rbrace");
    else if (*fromdate || *todate) EraseLast();
    if (app_resources.debug) {
	for (i=0 ; i<argvsize ; i++)
	    (void) fprintf(stderr, "%s ", argv[i]);
	(void) fprintf(stderr, "\n");
    }
    (void) DoCommand(argv, (char *) NULL, "/dev/null");
    TocReloadSeqLists(toc);
    TocChangeViewedSeq(toc, TocGetSeqNamed(toc, toseq));
    DestroyScrn(pick->scrn);
    for (i=0 ; i<argvsize ; i++) XtFree((char *) argv[i]);
    XtFree((char *) argv);
}


/* ARGSUSED */
static void ExecCancel(w, closure, call_data)
    Widget w;			/* unused */
    caddr_t closure;		/* FormEntry */
    caddr_t call_data;		/* unused */
{
    Pick pick = ((FormEntry)closure)->row->group->form->pick;
    Scrn scrn = pick->scrn;
    (void) DestroyScrn(scrn);
}



static FormEntry CreateWidget(row, class, args, num_args)
  RowList row;
  WidgetClass class;
  ArgList args;
  Cardinal num_args;
{
    static Arg arglist[] = {
	{XtNfromHoriz, (XtArgVal)NULL},
	{XtNresizable, (XtArgVal)TRUE},
	{XtNtop, (XtArgVal) XtChainTop},
	{XtNleft, (XtArgVal) XtChainLeft},
	{XtNbottom, (XtArgVal) XtChainTop},
	{XtNright, (XtArgVal) XtChainLeft},
    };
    ArgList merged_args;
    FormEntry entry;

    row->numwidgets++;
    row->wlist = (FormEntry *)
	XtRealloc((char *) row->wlist,
		  (unsigned) row->numwidgets * sizeof(FormEntry));
    entry = XtNew(FormEntryRec);
    entry->row = row;
    entry->type = class;
    row->wlist[row->numwidgets - 1] = entry;
    if (row->numwidgets > 1)
	arglist[0].value = (XtArgVal) row->wlist[row->numwidgets - 2]->widget;
    else
	arglist[0].value = (XtArgVal) NULL;

    merged_args = XtMergeArgLists( args, num_args, arglist, XtNumber(arglist) );

    entry->widget = XtCreateManagedWidget( NULL, class, row->widget,
					   merged_args,
					   num_args + XtNumber(arglist) );
			
    XtFree( (caddr_t)merged_args );
    return entry;
}
    

static DeleteWidget(entry)
  FormEntry entry;
{
    RowList row = entry->row;
    int i;
    XtDestroyWidget(entry->widget);
    if (entry->type == WTtextentry)
	XtFree((char *) entry->ptr);
    for (i = 0; i < row->numwidgets; i++)
	if (row->wlist[i] == entry)
	    break;
    row->numwidgets--;
    for (; i < row->numwidgets; i++)
	row->wlist[i] = row->wlist[i + 1];
}


/* Figure out how wide text fields and labels should be so that they'll all
   line up correctly, and be big enough to hold everything, but not too big. */

static void FindStdWidth()
{
    stdwidth = 100;		/* %%% HACK! */
}


static RowList AddRow(group, type)
  Group group;
  int type;
{
    static Arg arglist[] = {
	{XtNborderWidth, (XtArgVal) 0},
	{XtNfromVert, (XtArgVal) NULL},
	{XtNresizable, (XtArgVal) TRUE},
	{XtNtop, (XtArgVal) XtChainTop},
	{XtNleft, (XtArgVal) XtChainLeft},
	{XtNbottom, (XtArgVal) XtChainTop},
	{XtNright, (XtArgVal) XtChainLeft}
    };
    RowList row;
    group->numrows++;
    group->rlist = (RowList *)
	XtRealloc((char *) group->rlist,
		  (unsigned) group->numrows * sizeof(RowList));
    group->rlist[group->numrows - 1] = row =
	(RowList) XtMalloc(sizeof(RowListRec));
    row->type = type;
    row->numwidgets = 0;
    row->wlist = (FormEntry *) XtMalloc(1);
    row->group = group;
    if (group->numrows > 1)
	arglist[1].value = (XtArgVal)group->rlist[group->numrows - 2]->widget;
    else
	arglist[1].value = (XtArgVal) NULL;
    row->widget = XtCreateWidget( NULL, formWidgetClass, group->widget,
				  arglist, XtNumber(arglist) );
    if (type == RTignore) return row;
    AddButton(row, "Pick", ExecYesNo, TRUE);
    AddButton(row, "Skip", ExecYesNo, FALSE);
    if (TypeName[type])
	AddLabel(row, TypeName[type], TRUE);
    else
	AddTextEntry(row, "");
    AddTextEntry(row, "");
    AddButton(row, "Or", ExecRowOr, FALSE);
    XtManageChild(row->widget);
    return row;
}


static Group AddGroup(form)
  FormBox form;
{
    static Arg arglist[] = {
	{XtNborderWidth, (XtArgVal) 0},
	{XtNfromVert, (XtArgVal) NULL},
	{XtNresizable, (XtArgVal) TRUE},
	{XtNtop, (XtArgVal) XtChainTop},
	{XtNleft, (XtArgVal) XtChainLeft},
	{XtNbottom, (XtArgVal) XtChainTop},
	{XtNright, (XtArgVal) XtChainLeft}
    };
    Group group;
    form->numgroups++;
    form->glist = (Group *)
	XtRealloc((char *) form->glist,
		  (unsigned) form->numgroups * sizeof(Group));
    form->glist[form->numgroups - 1] = group =
	(Group) XtMalloc(sizeof(GroupRec));
    group->numrows = 0;
    group->form = form;
    group->rlist = (RowList *) XtMalloc(1);
    if (form->numgroups > 1)
	arglist[1].value = (XtArgVal)form->glist[form->numgroups - 2]->widget;
    else
	arglist[1].value = (XtArgVal)NULL;
    group->widget = XtCreateWidget( NULL, formWidgetClass, form->inner,
				    arglist, XtNumber(arglist) );
    return group;
}



static AddDetailGroup(form)
  FormBox form;
{
    Group group;
    RowList row;
    int     type;
    if (form->numgroups > 0) {
	group = form->glist[form->numgroups - 1];
	row = group->rlist[group->numrows - 1];
	DeleteWidget(row->wlist[0]);
	AddLabel(row, "- or -", FALSE);
    }
    group = AddGroup(form);
    for (type = FIRSTROWTYPE; type <= LASTUSEFULROWTYPE; type++)
	(void) AddRow(group, type);
    row =  AddRow(group, RTignore);
    AddButton(row, "- Or -", ExecGroupOr, FALSE);
    XtManageChild(row->widget);
    XtManageChild(group->widget);
}


static AddGeneralGroup(form)
  FormBox form;
{
    Group group;
    RowList row;
    Widget widgetList[4];
    group = AddGroup(form);
    row =  AddRow(group, RTignore);
    widgetList[0] = row->widget;
    AddLabel(row, "Creating sequence:", FALSE);
    AddTextEntry(row, "");
    AddLabel(row, "with msgs from sequence:", FALSE);
    AddTextEntry(row, "");
    row =  AddRow(group, RTignore);
    widgetList[1] = row->widget;
    AddLabel(row, "Date range:", FALSE);
    AddTextEntry(row, "");
    AddLabel(row, " - ", FALSE);
    AddTextEntry(row, "");
    AddLabel(row, "Date field:", FALSE);
    AddTextEntry(row, "");
    row =  AddRow(group, RTignore);
    widgetList[2] = row->widget;
    AddLabel(row, "Clear old entries from sequence?", FALSE);
    AddButton(row, "Yes", ExecYesNo, TRUE);
    AddButton(row, "No", ExecYesNo, FALSE);
    row =  AddRow(group, RTignore);
    widgetList[3] = row->widget;
    AddButton(row, "OK", ExecOK, FALSE);
    AddButton(row, "Cancel", ExecCancel, FALSE);
    XtManageChildren(widgetList, XtNumber(widgetList));
    XtManageChild(group->widget);
}


static void InitGeneral(pick, fromseq, toseq)
Pick pick;
char *fromseq, *toseq;
{
    RowList row;
    row = pick->general->glist[0]->rlist[0];
    ChangeTextEntry(row->wlist[1], toseq);
    ChangeTextEntry(row->wlist[3], fromseq);
}


static void CleanForm(form)
FormBox form;
{
    int i, j, k;
    Group group;
    RowList row;
    FormEntry entry;
    for (i=0 ; i<form->numgroups ; i++) {
	group = form->glist[i];
	for (j=0 ; j<group->numrows ; j++) {
	    row = group->rlist[j];
	    for (k=0 ; k<row->numwidgets ; k++) {
		entry = row->wlist[k];
		if (entry->type == WTtextentry)
		    ChangeTextEntry(entry, "");
	    }
	}
    }
}


static FormBox MakeAForm(pick, position)
Pick pick;
int position;
{
    static Arg arglist1[] = {
	{XtNallowHoriz, (XtArgVal)TRUE},
	{XtNallowVert, (XtArgVal)TRUE},
/* %%%	{XtNallowResize, (XtArgVal)TRUE}, */
	{XtNmin, 50},
	{XtNmax, 1500}
    };
    static Arg arglist2[] = {
	{XtNborderWidth, (XtArgVal) 0}
    };
    FormBox result;
    result = (FormBox) XtMalloc(sizeof(FormBoxRec));
    result->outer = XtCreateWidget( "pick", viewportWidgetClass,
				    pick->scrn->widget,
				    arglist1, XtNumber(arglist1) );
    result->inner = XtCreateWidget( "form", formWidgetClass, result->outer,
				    arglist2, XtNumber(arglist2) );
    result->pick = pick;
    result->numgroups = 0;
    result->glist = (Group *) XtMalloc(1);
    return result;
}



AddPick(scrn, toc, fromseq, toseq)
  Scrn scrn;
  Toc toc;
  char *fromseq, *toseq;
{
    Pick pick;
    FormBox general, details;
    char str[100];
    int height;

    if (scrn->pick) {
	pick = scrn->pick;
	CleanForm(pick->details);
	CleanForm(pick->general);
    } else {
	pick = scrn->pick = (Pick) XtMalloc(sizeof(PickRec));
	pick->scrn = scrn;
	pick->errorwidget = NULL;

	pick->label = CreateTitleBar(scrn, 0);

	pick->details = details = MakeAForm(pick, 1);
	pick->general = general = MakeAForm(pick, 2);
	FindStdWidth();

	XtPanedSetRefigureMode(scrn->widget, False);
	PrepareToUpdate(details);
	AddDetailGroup(details);
	ExecuteUpdate(details);
	PrepareToUpdate(general);
	AddGeneralGroup(general);
	ExecuteUpdate(general);
#ifdef notdef
	height = general->inner->core.height;
	if (general->inner->core.width > scrn->widget->core.width)
	    height += XtScrollMgrGetThickness(general->outer);
	XtPanedSetMinMax(scrn->widget, general->outer, height, height);
	XtPanedSetMinMax(scrn->widget, general->outer, 10, 10000);
#endif notdef
	XtPanedSetRefigureMode(scrn->widget, True);
    }
    pick->toc = toc;
    InitGeneral(pick, fromseq, toseq);
    (void) sprintf(str, "Pick: %s", TocName(toc));
    ChangeLabel(pick->label, str);
    StoreWindowName(scrn, str);
}
