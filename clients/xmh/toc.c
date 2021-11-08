#ifndef lint
static char rcs_id[] = "$XConsortium: toc.c,v 2.17 88/09/06 17:23:33 jim Exp $";
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

/* toc.c -- handle things in the toc widget. */

#include "xmh.h"
#include "tocintrnl.h"
#include "toc.h"
#include "tocutil.h"
#include <sys/stat.h>
#include <sys/dir.h>

/*	PUBLIC ROUTINES 	*/


static int IsDir(ent)
struct direct *ent;
{
    char str[500];
    struct stat buf;
    if (ent->d_name[0] == '.')
	return FALSE;
    (void) sprintf(str, "%s/%s", app_resources.mailDir, ent->d_name);
    (void) stat(str, &buf);
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}



static void MakeSureFolderExists(namelistptr, numfoldersptr, name)
struct direct ***namelistptr;
int *numfoldersptr;
char *name;
{
    int i;
    extern alphasort();
    char str[200];
    for (i=0 ; i<*numfoldersptr ; i++)
	if (strcmp((*namelistptr)[i]->d_name, name) == 0) return;
    (void) sprintf(str, "%s/%s", app_resources.mailDir, name);
    (void) mkdir(str, 0700);
    *numfoldersptr = scandir(app_resources.mailDir, namelistptr,
			     IsDir, alphasort);
    for (i=0 ; i<*numfoldersptr ; i++)
	if (strcmp((*namelistptr)[i]->d_name, name) == 0) return;
    Punt("Can't create new mail folder!");
}


static void LoadCheckFiles()
{
    FILE *fid;
    char str[1024], *ptr, *ptr2;
    int i;
    (void) sprintf(str, "%s/.xmhcheck", homeDir);
    fid = myfopen(str, "r");
    if (fid) {
	while (ptr = ReadLine(fid)) {
	    while (*ptr == ' ' || *ptr == '\t') ptr++;
	    ptr2 = ptr;
	    while (*ptr2 && *ptr2 != ' ' && *ptr2 != '\t') ptr2++;
	    if (*ptr2 == 0) continue;
	    *ptr2++ = 0;
	    while (*ptr2 == ' ' || *ptr2 == '\t') ptr2++;
	    if (*ptr2 == 0) continue;
	    for (i=0 ; i<numFolders ; i++) {
		if (strcmp(ptr, folderList[i]->foldername) == 0) {
		    folderList[i]->incfile = MallocACopy(ptr2);
		    break;
		}
	    }
	}
	myfclose(fid);
    } else if (app_resources.initialIncFile != NULL) {
        if (*app_resources.initialIncFile != '\0')
	    InitialFolder->incfile = app_resources.initialIncFile;
    } else {
	ptr = getenv("MAIL");
	if (ptr == NULL) ptr = getenv("mail");
	if (ptr == NULL) {
	    ptr = getenv("USER");
	    if (ptr) {
		(void) sprintf(str, "/usr/spool/mail/%s", ptr);
		ptr = str;
	    }
	}
	if (ptr)
	    InitialFolder->incfile = MallocACopy(ptr);
    }
}
	    



/* Read in the list of folders. */

void TocInit()
{
    Toc toc;
    struct direct **namelist;
    int i;
    extern alphasort();
    numFolders = scandir(app_resources.mailDir, &namelist, IsDir, alphasort);
    if (numFolders < 0) {
	(void) mkdir(app_resources.mailDir, 0700);
	numFolders = scandir(app_resources.mailDir, &namelist, IsDir, alphasort);
	if (numFolders < 0)
	    Punt("Can't create or read mail directory!");
    }
    MakeSureFolderExists(&namelist, &numFolders, app_resources.initialFolderName);
    MakeSureFolderExists(&namelist, &numFolders, app_resources.draftsFolderName);
    folderList = (Toc *) XtMalloc((unsigned) numFolders * sizeof(Toc));
    for (i=0 ; i<numFolders ; i++) {
	toc = folderList[i] = TUMalloc();
	toc->foldername = MallocACopy(namelist[i]->d_name);
	XtFree((char *)namelist[i]);
    }
    InitialFolder = TocGetNamed(app_resources.initialFolderName);
    DraftsFolder = TocGetNamed(app_resources.draftsFolderName);
    XtFree((char *)namelist);
    if (app_resources.defNewMailCheck) LoadCheckFiles();
}


/* Create a new folder with the given name. */

Toc TocCreateFolder(foldername)
char *foldername;
{
    Toc toc;
    int i, j;
    char str[500];
    if (TocGetNamed(foldername)) return NULL;
    (void) sprintf(str, "%s/%s", app_resources.mailDir, foldername);
    if (mkdir(str, 0700) < 0) return NULL;
    toc = TUMalloc();
    toc->foldername = MallocACopy(foldername);
    for (i=0 ; i<numFolders ; i++)
	if (strcmp(foldername, folderList[i]->foldername) < 0) break;
    folderList = (Toc *) XtRealloc((char *) folderList,
				   (unsigned) ++numFolders * sizeof(Toc));
    for (j=numFolders - 1 ; j > i ; j--)
	folderList[j] = folderList[j-1];
    folderList[i] = toc;
    return toc;
}



/* Check to see if what folders have new mail, and highlight their
   folderbuttons appropriately. */

void TocCheckForNewMail()
{
    Toc toc;
    Scrn scrn;
    int i, j, hasmail;
    static Arg arglist[] = {XtNiconPixmap, NULL};
    if (!app_resources.defNewMailCheck) return;
    for (i=0 ; i<numFolders ; i++) {
	toc = folderList[i];
	if (toc->incfile) {
	    hasmail =  (GetFileLength(toc->incfile) > 0);
	    if (hasmail != toc->mailpending) {
		toc->mailpending = hasmail;
		for (j=0 ; j<numScrns ; j++) {
		    scrn = scrnList[j];
		    if (scrn->kind == STtocAndView) {
			if (app_resources.mailWaitingFlag
			    && toc == InitialFolder) {
			    arglist[0].value = (XtArgVal)
				hasmail ? NewMailPixmap : NoMailPixmap;
			    XtSetValues(scrn->parent,
					arglist, XtNumber(arglist));
			} else {
			    BBoxChangeBorderWidth(   /* %%% HACK */
			       BBoxButtonNumber(scrnList[j]->folderbuttons, i),
						(unsigned)(hasmail ? 2 : 1));
			}
		    }
		}
	    }
	}
    }
}


/* Recursively delete an entire directory.  Nasty. */

static void NukeDirectory(path)
char *path;
{
    char str[500];
    (void) sprintf(str, "rm -rf %s", path);
    (void) system(str);
}



/* Destroy the given folder. */

void TocDeleteFolder(toc)
Toc toc;
{
    Toc toc2;
    int i, j, w;
    TUGetFullFolderInfo(toc);
    if (TocConfirmCataclysm(toc)) return;
    TocSetScrn(toc, (Scrn) NULL);
    w = -1;
    for (i=0 ; i<numFolders ; i++) {
	toc2 = folderList[i];
	if (toc2 == toc)
	    w = i;
	else if (toc2->validity == valid)
	    for (j=0 ; j<toc2->nummsgs ; j++)
		if (toc2->msgs[j]->desttoc == toc)
		    MsgSetFate(toc2->msgs[j], Fignore, (Toc) NULL);
    }
    if (w < 0) Punt("Couldn't find it in TocDeleteFolder!");
    NukeDirectory(toc->path);
    if (toc->validity == valid) {
	for (i=0 ; i<toc->nummsgs ; i++) {
	    MsgSetScrnForce(toc->msgs[i], (Scrn) NULL);
	    MsgFree(toc->msgs[i]);
	}
	XtFree((char *) toc->msgs);
    }
    XtFree((char *)toc);
    numFolders--;
    for (i=w ; i<numFolders ; i++) folderList[i] = folderList[i+1];
}


/*
 * Display the given toc in the given scrn.  If scrn is NULL, then remove the
 * toc from all scrns displaying it.
 */

void TocSetScrn(toc, scrn)
Toc toc;
Scrn scrn;
{
    int i;
    if (toc == NULL && scrn == NULL) return;
    if (scrn == NULL) {
	for (i=0 ; i<toc->num_scrns ; i++)
	    TocSetScrn((Toc) NULL, toc->scrn[i]);
	return;
    }
    if (scrn->toc == toc) return;
    if (scrn->toc != NULL) {
	for (i=0 ; i<scrn->toc->num_scrns ; i++)
	    if (scrn->toc->scrn[i] == scrn) break;
	if (i >= scrn->toc->num_scrns)
	    Punt("Couldn't find scrn in TocSetScrn!");
	scrn->toc->scrn[i] = scrn->toc->scrn[--scrn->toc->num_scrns];
    }
    scrn->toc = toc;
    if (toc == NULL) {
	TUResetTocLabel(scrn);
	TURedisplayToc(scrn);
	StoreWindowName(scrn, progName);
	EnableProperButtons(scrn);
    } else {
	toc->num_scrns++;
	toc->scrn = (Scrn *) XtRealloc((char *) toc->scrn,
				       (unsigned)toc->num_scrns*sizeof(Scrn));
	toc->scrn[toc->num_scrns - 1] = scrn;
	TUEnsureScanIsValidAndOpen(toc);
	TUResetTocLabel(scrn);
	StoreWindowName(scrn, toc->foldername);
	TURedisplayToc(scrn);

	BBoxSetRadio(scrn->folderbuttons,
		 BBoxFindButtonNamed(scrn->folderbuttons, toc->foldername));

	EnableProperButtons(scrn);
    }
}



/* Remove the given message from the toc.  Doesn't actually touch the file.
   Also note that it does not free the storage for the msg. */

void TocRemoveMsg(toc, msg)
Toc toc;
Msg msg;
{
    Msg newcurmsg;
    MsgList mlist;
    int i;
    if (toc->validity == unknown)
	TUGetFullFolderInfo(toc);
    if (toc->validity != valid)
	return;
    newcurmsg = TocMsgAfter(toc, msg);
    if (newcurmsg) newcurmsg->changed = TRUE;
    newcurmsg = toc->curmsg;
    if (msg == toc->curmsg) {
	newcurmsg = TocMsgAfter(toc, msg);
	if (newcurmsg == NULL) newcurmsg = TocMsgBefore(toc, msg);
	toc->curmsg = NULL;
    }
    toc->length -= msg->length;
    if (msg->visible) toc->lastPos -= msg->length;
    for(i = TUGetMsgPosition(toc, msg), toc->nummsgs--; i<toc->nummsgs ; i++) {
	toc->msgs[i] = toc->msgs[i+1];
	if (msg->visible) toc->msgs[i]->position -= msg->length;
    }
    for (i=0 ; i<toc->numsequences ; i++) {
	mlist = toc->seqlist[i]->mlist;
	if (mlist) DeleteMsgFromMsgList(mlist, msg);
    }

    if (msg->visible && toc->num_scrns > 0 && !toc->needsrepaint)
	TSourceInvalid(toc, msg->position, -msg->length);
    TocSetCurMsg(toc, newcurmsg);
    TUSaveTocFile(toc);
}
    


void TocRecheckValidity(toc)
  Toc toc;
{
    int i;
    if (toc && toc->validity == valid && TUScanFileOutOfDate(toc)) {
	TUScanFileForToc(toc);
	if (toc->source)
	    TULoadTocFile(toc);
	for (i=0 ; i<toc->num_scrns ; i++)
	    TURedisplayToc(toc->scrn[i]);
    }
}


/* Set the current message. */

void TocSetCurMsg(toc, msg)
  Toc toc;
  Msg msg;
{
    Msg msg2;
    int i;
    if (toc->validity != valid) return;
    if (msg != toc->curmsg) {
	msg2 = toc->curmsg;
	toc->curmsg = msg;
	if (msg2)
	    MsgSetFate(msg2, msg2->fate, msg2->desttoc);
    }
    if (msg) {
	MsgSetFate(msg, msg->fate, msg->desttoc);
	if (toc->num_scrns) {
	    if (toc->stopupdate)
		toc->needsrepaint = TRUE;
	    else {
		for (i=0 ; i<toc->num_scrns ; i++)
		    XtTextSetInsertionPoint(toc->scrn[i]->tocwidget,
						msg->position);
	    }
	}
    }
}


/* Return the current message. */

Msg TocGetCurMsg(toc)
Toc toc;
{
    return toc->curmsg;
}




/* Return the message after the given one.  (If none, return NULL.) */

Msg TocMsgAfter(toc, msg)
  Toc toc;
  Msg msg;
{
    int i;
    i = TUGetMsgPosition(toc, msg);
    do {
	i++;
	if (i >= toc->nummsgs)
	    return NULL;
    } while (!(toc->msgs[i]->visible));
    return toc->msgs[i];
}



/* Return the message before the given one.  (If none, return NULL.) */

Msg TocMsgBefore(toc, msg)
  Toc toc;
  Msg msg;
{
    int i;
    i = TUGetMsgPosition(toc, msg);
    do {
	i--;
	if (i < 0)
	    return NULL;
    } while (!(toc->msgs[i]->visible));
    return toc->msgs[i];
}



/* The caller KNOWS the toc's information is out of date; rescan it. */

void TocForceRescan(toc)
  Toc toc;
{
    int i;
    if (toc->num_scrns) {
	toc->viewedseq = toc->seqlist[0];
	for (i=0 ; i<toc->num_scrns ; i++)
	    TUResetTocLabel(toc->scrn[i]);
	TUScanFileForToc(toc);
	TULoadTocFile(toc);
	for (i=0 ; i<toc->num_scrns ; i++)
	    TURedisplayToc(toc->scrn[i]);
    } else {
	TUGetFullFolderInfo(toc);
	(void) unlink(toc->scanfile);
	toc->validity = invalid;
    }
}



/* The caller has just changed a sequence list.  Reread them from mh. */

void TocReloadSeqLists(toc)
Toc toc;
{
    int i;
    TocSetCacheValid(toc);
    TULoadSeqLists(toc);
    TURefigureWhatsVisible(toc);
    for (i=0 ; i<toc->num_scrns ; i++) {
	TUResetTocLabel(toc->scrn[i]);
	EnableProperButtons(toc->scrn[i]);
    }
}


/* Return TRUE if the toc has an interesting sequence. */

int TocHasSequences(toc)
Toc toc;
{
    return toc && toc->numsequences > 1;
}


/* Change which sequence is being viewed. */

void TocChangeViewedSeq(toc, seq)
  Toc toc;
  Sequence seq;
{
    int i;
    if (seq == NULL) seq = toc->viewedseq;
    toc->viewedseq = seq;
    TURefigureWhatsVisible(toc);
    for (i=0 ; i<toc->num_scrns ; i++) {
	if (toc->scrn[i]->seqbuttons)
	    BBoxSetRadio(toc->scrn[i]->seqbuttons,
			 BBoxFindButtonNamed(toc->scrn[i]->seqbuttons,
					     seq->name));
	TUResetTocLabel(toc->scrn[i]);
    }
}


/* Return the sequence with the given name in the given toc. */

Sequence TocGetSeqNamed(toc, name)
Toc toc;
char *name;
{
    int i;
    for (i=0 ; i<toc->numsequences ; i++)
	if (strcmp(toc->seqlist[i]->name, name) == 0)
	    return toc->seqlist[i];
    return (Sequence) NULL;
}


/* Return the sequence currently being viewed in the toc. */

Sequence TocViewedSequence(toc)
Toc toc;
{
    return toc->viewedseq;
}


/* Return the list of messages currently selected. */

MsgList TocCurMsgList(toc)
  Toc toc;
{
    MsgList result;
    XtTextPosition pos1, pos2;
    extern Msg MsgFromPosition();
    if (toc->num_scrns == NULL) return NULL;
    result = MakeNullMsgList();
/*    if ((*toc->source->GetSelection)(toc->source, &pos1, &pos2)) { */
    XtTextGetSelectionPos( toc->scrn[0]->tocwidget, &pos1, &pos2); /* %%% */
    if (pos1 < pos2) {
	pos1 = (*toc->source->Scan)(toc->source, pos1, XtstEOL, XtsdLeft,
				    1, FALSE);
	pos2 = (*toc->source->Scan)(toc->source, pos2, XtstPositions,
				    XtsdLeft, 1, TRUE);
	pos2 = (*toc->source->Scan)(toc->source, pos2, XtstEOL, XtsdRight,
				    1, FALSE);
	while (pos1 < pos2) {
	    AppendMsgList(result, MsgFromPosition(toc, pos1, XtsdRight));
	    pos1 = (*toc->source->Scan)(toc->source, pos1, XtstEOL,
					XtsdRight, 1, TRUE);
	}
    }
    return result;
}



/* Unset the current selection. */

void TocUnsetSelection(toc)
Toc toc;
{
    if (toc->source)
/*	(*toc->source->SetSelection)(toc->source, 1, 0); */
        XtTextUnsetSelection(toc->scrn[0]->tocwidget);
}



/* Create a brand new, blank message. */

Msg TocMakeNewMsg(toc)
Toc toc;
{
    Msg msg;
    static int looping = False;
    TUEnsureScanIsValidAndOpen(toc);
    msg = TUAppendToc(toc, "####  empty\n");
    if (FileExists(MsgFileName(msg))) {
	if (looping++) Punt( "Cannot correct scan file" );
        DEBUG1("**** FOLDER %s WAS INVALID!!!\n", toc->foldername)
	TocForceRescan(toc);
	return TocMakeNewMsg(toc); /* Try again.  Using recursion here is ugly,
				      but what the hack ... */
    }
    CopyFileAndCheck("/dev/null", MsgFileName(msg));
    looping = False;
    return msg;
}


/* Set things to not update cache or display until further notice. */

void TocStopUpdate(toc)
Toc toc;
{
    int i;
    for (i=0 ; i<toc->num_scrns ; i++)
	XtTextDisableRedisplay(toc->scrn[i]->tocwidget, FALSE);
    toc->stopupdate++;
}


/* Start updating again, and do whatever updating has been queued. */

void TocStartUpdate(toc)
Toc toc;
{
    int i;
    if (toc->stopupdate && --(toc->stopupdate) == 0) {
	for (i=0 ; i<toc->num_scrns ; i++) {
	    if (toc->needsrepaint) 
		TURedisplayToc(toc->scrn[i]);
	    if (toc->needslabelupdate)
		TUResetTocLabel(toc->scrn[i]);
	}
	if (toc->needscachesave)
	    TUSaveTocFile(toc);
    }
    for (i=0 ; i<toc->num_scrns ; i++)
	XtTextEnableRedisplay(toc->scrn[i]->tocwidget);
}



/* Something has happened that could later convince us that our cache is out
   of date.  Make this not happen; our cache really *is* up-to-date. */

void TocSetCacheValid(toc)
Toc toc;
{
    TUSaveTocFile(toc);
}


/* Return the full folder pathname of the given toc, prefixed w/'+' */

char *TocMakeFolderName(toc)
Toc toc;
{
    char* name = XtMalloc( strlen(toc->path) + 2 );
    (void)sprintf( name, "+%s", toc->path );
    return name;
}

char *TocName(toc)
Toc toc;
{
    return toc->foldername;
}



/* Given a foldername, return the corresponding toc. */

Toc TocGetNamed(name)
char *name;
{
    int i;
    for (i=0; i<numFolders ; i++)
	if (strcmp(folderList[i]->foldername, name) == 0) return folderList[i];
    return NULL;
}



/* Throw out all changes to this toc, and close all views of msgs in it.
   Requires confirmation by the user. */

int TocConfirmCataclysm(toc)
Toc toc;
{
    int i;
    int found = FALSE;
    char str[500];
    for (i=0 ; i<toc->nummsgs && !found ; i++)
	if (toc->msgs[i]->fate != Fignore) found = TRUE;
    if (found) {
	(void)sprintf(str,"Are you sure you want to remove all changes to %s?",
		      toc->foldername);
	if (!Confirm(toc->scrn[0], str))
	    return DELETEABORTED;
    }
    for (i=0 ; i<toc->nummsgs ; i++)
	MsgSetFate(toc->msgs[i], Fignore, (Toc)NULL);
    for (i=0 ; i<toc->nummsgs ; i++)
	if (MsgSetScrn(toc->msgs[i], (Scrn) NULL)) return DELETEABORTED;
    return 0;
}



/* Commit all the changes in this toc; all messages will meet their 'fate'. */

void TocCommitChanges(toc)
Toc toc;
{
    Msg msg;
    int i, cur;
    char str[100], **argv;
    FateType curfate, fate; 
    Toc desttoc;
    Toc curdesttoc;

    if (toc == NULL) return;
    for (i=0 ; i<toc->nummsgs ; i++) {
	msg = toc->msgs[i];
	fate = MsgGetFate(msg, (Toc *)NULL);
	if (fate != Fignore && fate != Fcopy)
	    if (MsgSetScrn(msg, (Scrn) NULL))
	        return;
    }
    XFlush(XtDisplay(toc->scrn[0]->parent));
    for (i=0 ; i<numFolders ; i++)
	TocStopUpdate(folderList[i]);
    toc->haschanged = TRUE;
    do {
	curfate = Fignore;
	i = 0;
	while (i < toc->nummsgs) {
	    msg = toc->msgs[i];
	    fate = MsgGetFate(msg, &desttoc);
	    if (curfate == Fignore && fate != Fignore) {
		curfate = fate;
		argv = MakeArgv(2);
		switch (curfate) {
		  case Fdelete:
		    argv[0] = MallocACopy("rmm");
		    argv[1] = TocMakeFolderName(toc);
		    cur = 2;
		    curdesttoc = NULL;
		    break;
		  case Fmove:
		  case Fcopy:
		    argv[0] = MallocACopy("refile");
		    cur = 1;
		    curdesttoc = desttoc;
		    break;
		}
	    }
	    if (curfate != Fignore &&
		  curfate == fate && desttoc == curdesttoc) {
		argv = ResizeArgv(argv, cur + 1);
		(void) sprintf(str, "%d", MsgGetId(msg));
		argv[cur++] = MallocACopy(str);
		MsgSetFate(msg, Fignore, (Toc)NULL);
		if (curdesttoc) {
		    (void) TUAppendToc(curdesttoc, MsgGetScanLine(msg));
		    curdesttoc->haschanged = TRUE;
		}
		if (curfate != Fcopy) {
		    TocRemoveMsg(toc, msg);
		    MsgFree(msg);
		    i--;
		}
		if (cur > 40)
		    break;	/* Do only 40 at a time, just to be safe. */
	    } 
	    i++;
	}
	if (curfate != Fignore) {
	    switch (curfate) {
	      case Fmove:
	      case Fcopy:
		argv = ResizeArgv(argv, cur + 4);
		argv[cur++] = MallocACopy(curfate == Fmove ? "-nolink"
				       			   : "-link");
		argv[cur++] = MallocACopy("-src");
		argv[cur++] = TocMakeFolderName(toc);
		argv[cur++] = TocMakeFolderName(curdesttoc);
		break;
	    }
	    if (app_resources.debug) {
		for (i = 0; i < cur; i++)
		    (void) fprintf(stderr, "%s ", argv[i]);
		(void) fprintf(stderr, "\n");
		(void) fflush(stderr);
	    }
	    DoCommand(argv, (char *) NULL, "/dev/null");
	    for (i = 0; argv[i]; i++)
		XtFree((char *) argv[i]);
	    XtFree((char *) argv);
	}
    } while (curfate != Fignore);
    for (i=0 ; i<numFolders ; i++) {
	if (folderList[i]->haschanged) {
	    TocReloadSeqLists(folderList[i]);
	    folderList[i]->haschanged = FALSE;
	}
	TocStartUpdate(folderList[i]);
    }
}



/* Return whether the given toc can incorporate mail. */

int TocCanIncorporate(toc)
Toc toc;
{
    return (toc && (toc == InitialFolder || toc->incfile));
}


/* Incorporate new messages into the given toc. */

void TocIncorporate(toc)
Toc toc;
{
    char **argv;
    char str[100], *file, *ptr;
    Msg msg, firstmessage;
    FILEPTR fid;
    argv = MakeArgv(toc->incfile ? 7 : 5);
    argv[0] = "inc";
    argv[1] = TocMakeFolderName(toc);
    argv[2] = "-width";
    (void) sprintf(str, "%d", app_resources.defTocWidth);
    argv[3] = str;
    if (toc->incfile) {
	argv[4] = "-file";
	argv[5] = toc->incfile;
	argv[6] = "-truncate";
    } else argv[4] = "-truncate";
    file = DoCommandToFile(argv);
    XtFree(argv[1]);
    XtFree((char *)argv);
    TUGetFullFolderInfo(toc);
    if (toc->validity == valid) {
	fid = FOpenAndCheck(file, "r");
	firstmessage = NULL;
	TocStopUpdate(toc);
	while (ptr = ReadLineWithCR(fid)) {
	    if (atoi(ptr) > 0) {
		msg = TUAppendToc(toc, ptr);
		if (firstmessage == NULL) firstmessage = msg;
	    }
	}
	if (firstmessage && firstmessage->visible) {
	    TocSetCurMsg(toc, firstmessage);
	}
	TocStartUpdate(toc);
	(void) myfclose(fid);
    }
    DeleteFileAndCheck(file);
}



/* The given message has changed.  Rescan it and change the scanfile. */

void TocMsgChanged(toc, msg)
Toc toc;
Msg msg;
{
    char **argv, str[100], str2[10], *ptr;
    int length, delta, i;
    FateType fate;
    Toc desttoc;
    if (toc->validity != valid) return;
    fate = MsgGetFate(msg, &desttoc);
    MsgSetFate(msg, Fignore, (Toc) NULL);
    argv = MakeArgv(5);
    argv[0] = "scan";
    argv[1] = TocMakeFolderName(toc);
    (void) sprintf(str, "%d", msg->msgid);
    argv[2] = str;
    argv[3] = "-width";
    (void) sprintf(str2, "%d", app_resources.defTocWidth);
    argv[4] = str2;
    ptr = DoCommandToString(argv);
    XtFree(argv[1]);
    XtFree((char *) argv);
    if (strcmp(ptr, msg->buf) != 0) {
	length = strlen(ptr);
	delta = length - msg->length;
	XtFree(msg->buf);
	msg->buf = ptr;
	msg->length = length;
	toc->length += delta;
	if (msg->visible) {
	    if (delta != 0) {
		for (i=TUGetMsgPosition(toc, msg)+1; i<toc->nummsgs ; i++)
		    toc->msgs[i]->position += delta;
		toc->lastPos += delta;
	    }
	    for (i=0 ; i<toc->num_scrns ; i++)
		TURedisplayToc(toc->scrn[i]);
	}
	MsgSetFate(msg, fate, desttoc);
	TUSaveTocFile(toc);
    } else XtFree(ptr);
}



Msg TocMsgFromId(toc, msgid)
Toc toc;
int msgid;
{
    int h, l, m;
    l = 0;
    h = toc->nummsgs - 1;
    if (h < 0) {
	if (app_resources.debug) {
	    char str[100];
	    (void)sprintf(str, "Toc is empty! folder=%s\n", toc->foldername);
	    DEBUG( str );
	}
	return NULL;
    }
    while (l < h - 1) {
	m = (l + h) / 2;
	if (toc->msgs[m]->msgid > msgid)
	    h = m;
	else
	    l = m;
    }
    if (toc->msgs[l]->msgid == msgid) return toc->msgs[l];
    if (toc->msgs[h]->msgid == msgid) return toc->msgs[h];
    if (app_resources.debug) {
	char str[100];
	(void)sprintf(str,
		      "TocMsgFromId search failed! hi=%d, lo=%d, msgid=%d\n",
		      h, l, msgid);
	DEBUG( str );
    }
    return NULL;
}
