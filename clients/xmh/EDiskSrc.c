#ifndef lint
static char rcs_id[] = "$XConsortium: EDiskSrc.c,v 2.16 88/10/23 12:57:57 swick Exp $";
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

/* File: EDiskSource.c */

/* This is a rather simplistic text source.  It allows editing of a disk file.
   It divides the file into blocks of BUFSIZE characters.  No block is
   actually read in from disk until referenced; each block will be read at
   most once.  Thus, the entire file can eventually be read into memory and
   stored there.  When the save function is called, everything gets written
   back. */
   

#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/Text.h>
#include <sys/stat.h>
#include <X11/TextP.h>

#include "xmh.h"



/* Private definitions. */

#define BUFSIZE	512		/* Number of chars to store in each piece. */

extern char *strcpy();
extern long lseek();

/* The structure corresponding to each piece of the file in memory. */

typedef struct {
    char *buf;			/* Pointer to this piece's data. */
    int length;			/* Number of chars used in buf. */
    int maxlength;		/* Number of chars allocated in buf. */
    long start;			/* Position in file corresponding to piece. */
} PieceRec, *PiecePtr;



/* The master structure for the source. */

typedef struct _EDiskDataRec {
    int file;			/* File descriptor. */
    char *name;			/* Name of file. */
    TextWidget *widgets;	/* Array of widgets displaying this source. */
    int numwidgets;		/* Number of entries in above. */
    int length;			/* Number of characters in file. */
    int origlength;		/* Number of characters originally in file. */
    int numpieces;		/* How many pieces the file is divided in. */
    PiecePtr piece;		/* Pointer to array of pieces. */
    short changed;		/* If changes have not been written to disk. */
    short everchanged;		/* If changes have ever been made. */
    short startsvalid;		/* True iff the start fields in pieces
				   are correct. */
    short eversaved;		/* If we've ever saved changes to the file. */
    void (*func)();		/* Function to call when a change is made. */
    caddr_t param;		/* Parameter to pass to above function. */
    Boolean hasselection;	/* Whether we own the selection. */
    XtTextPosition left, right; /* Left and right extents of selection. */
    int checkpointed;		/* Filename to store checkpoints. */
    int checkpointchange;	/* TRUE if we've changed since checkpointed. */
}  EDiskDataRec, *EDiskData;



/* Refigure the starting location of each piece, which is the sum of the
   lengths of all the preceding pieces. */

static CalculateStarts(data)
  EDiskData data;
{
    int     i;
    PiecePtr piece;
    long    start = 0;
    for (i = 0, piece = data->piece; i < data->numpieces; i++, piece++) {
	piece->start = start;
	start += piece->length;
    }
    data->startsvalid = TRUE;
}



/* Read in the text for the ith piece. */

static ReadPiece(data, i)
  EDiskData data;
  int i;
{
    PiecePtr piece = data->piece + i;
    piece->maxlength = piece->length;
    piece->buf = XtMalloc((unsigned) piece->maxlength);
    (void)lseek(data->file, (long) i * BUFSIZE, 0);
    (void)read(data->file, piece->buf, piece->length);
}



/* Figure out which piece corresponds to the given position.  If this source
   has never had any changes, then this is a simple matter of division;
   otherwise, we have to search for the specified piece.  (Currently, this is
   a simple linear search; a binary one would probably be better.) */

static PiecePtr PieceFromPosition(data, position)
  EDiskData data;
  XtTextPosition position;
{
    int     i;
    PiecePtr piece;
    if (!data->everchanged) {
	i = position / BUFSIZE;
	if (i >= data->numpieces) i = data->numpieces - 1;
	piece = data->piece + i;
    } else {
	if (!data->startsvalid)
	    CalculateStarts(data);
	for (i = 0, piece = data->piece; i < data->numpieces - 1; i++, piece++)
	    if (position < piece->start + piece->length)
		break;
    }
    if (!piece->buf) ReadPiece(data, i);
    return piece;
}


/* Return the given position, coerced to be within the range of legal positions
   for the given source. */

static XtTextPosition CoerceToLegalPosition(data, position)
  EDiskData data;
  XtTextPosition position;
{
    return (position < 0) ? 0 :
		 ((position > data->length) ? data->length : position);
}


#ifdef notdef
/*ARGSUSED*/
static Boolean Convert(w, desiredtype, type, value, length)
Widget w;
Atom desiredtype;
Atom *type;
caddr_t *value;
int *length;
{
    TextWidget widget = (TextWidget) w;
    XtTextSource source = widget->text.source;
    EDiskData data = (EDiskData) source->data;
    XtTextBlock block;
    XtTextPosition position, lastpos;
    *type = (Atom) FMT8BIT;		/* Only thing we know! */
    if (data == NULL || !data->hasselection) return FALSE;
    *length = data->right - data->left;
    *value = XtMalloc((unsigned) *length + 1);
    position = data->left;
    while (position < data->right) {
	lastpos = position;
	position = (*source->Read)(source, position, &block,
				   data->right - position);
	bcopy(block.ptr, (*value) + lastpos - data->left, position - lastpos);
    }
    return TRUE;
}

/*ARGSUSED*/
static void LoseSelection(w, selection)
Widget w;
Atom selection;
{
    TextWidget widget = (TextWidget) w;
    EDiskData data = (EDiskData) widget->text.source->data;
    if (data && data->hasselection)
	(*widget->text.source->SetSelection)(widget->text.source, 1, 0);
}


/* Semi-public definitions */

static void AddWidget(source, widget)
XtTextSource source;
TextWidget widget;
{
    EDiskData data = (EDiskData) source->data;
    data->numwidgets++;
    data->widgets = (TextWidget *)
	XtRealloc((char *) data->widgets,
		  (unsigned) (sizeof(TextWidget) * data->numwidgets));
    data->widgets[data->numwidgets - 1] = widget;
    if (data->hasselection && data->numwidgets == 1)
	XtSelectionGrab((Widget) data->widgets[0], XA_PRIMARY,
			Convert, LoseSelection);
}

static void RemoveWidget(source, widget)
XtTextSource source;
TextWidget widget;
{
    EDiskData data = (EDiskData) source->data;
    int i;
    for (i=0 ; i<data->numwidgets ; i++) {
	if (data->widgets[i] == widget) {
	    data->numwidgets--;
	    data->widgets[i] = data->widgets[data->numwidgets];
	    if (i == 0 && data->numwidgets > 0 && data->hasselection)
		XtSelectionGrab((Widget) data->widgets[0], XA_PRIMARY,
				Convert, LoseSelection);
	    return;
	}
    }
}
#endif notdef

/* Reads in the text for the given range.  Will not read past a piece boundary.
   Returns the position after the last character that was read. */

static XtTextPosition Read(source, position, block, length)
XtTextSource source;
XtTextPosition position;	/* Starting position to read. */
XtTextBlock *block;		/* RETURN - the text read. */
int length;			/* (maxmimum) number of bytes to read. */
{
    EDiskData data = (EDiskData) source->data;
    PiecePtr piece;
    int count;

    if (position < data->length) {
        block->firstPos = position;
	piece = PieceFromPosition(data, position);
	block->ptr = piece->buf + (position - piece->start);
	count = piece->length - (position - piece->start);
	block->length = (count < length) ? count : length;
    } else {
        block->firstPos = 0;
	block->length = 0;
	block->ptr = "";
    }
    block->format = FMT8BIT;
    return position + block->length;
}




/* Replace the text between startPos and endPos with the given text.  Returns
   EditDone on success, or EditError if this source is read-only.  */


static int Replace(source, startPos, endPos, block)
  XtTextSource source;
  XtTextPosition startPos, endPos;
  XtTextBlock *block;
{
    EDiskData data = (EDiskData) source->data;
    PiecePtr piece, piece2;
    int oldlength = endPos - startPos;
    int i;
    int del = block->length - oldlength;
    if (del == 0 && oldlength == 0)
	return EditDone;
    if (source->edit_mode != XttextEdit)
	return EditError;
    for (i=0 ; i<data->numwidgets ; i++) {
	XtTextDisableRedisplay(data->widgets[i], TRUE);
#ifdef notdef
	if (data->hasselection)
	    Xt_TextSetHighlight(data->widgets[i], data->left, data->right,
				Normal);
#endif notdef
    }
    data->length += del;
    piece = PieceFromPosition(data, startPos);
    if (oldlength > 0) {
	piece2 = PieceFromPosition(data, endPos - 1);
	if (piece != piece2) {
	    oldlength = endPos - piece2->start;
	    piece2->length -= oldlength;
	    for (i = 0; i < piece2->length; i++)
		piece2->buf[i] = piece2->buf[i + oldlength];
	    for (piece2--; piece2 > piece; piece2--)
		piece2->length = 0;
	    oldlength = piece->length - (startPos - piece->start);
	    del = block->length - oldlength;
	}
    }
    data->changed = data->everchanged = data->checkpointchange = TRUE;
    data->startsvalid = FALSE;
    piece->length += del;
    if (piece->length > piece->maxlength) {
	do
	    piece->maxlength *= 2;
	while (piece->length > piece->maxlength);
	piece->buf = XtRealloc(piece->buf, (unsigned)piece->maxlength);
    }
    if (del < 0)		/* insert shorter than delete, text getting
				   shorter */
	for (i = startPos - piece->start; i < piece->length; i++)
	    piece->buf[i] = piece->buf[i - del];
    else
	if (del > 0)		/* insert longer than delete, text getting
				   longer */
	    for (i = piece->length - del - 1;
		    i >= startPos - piece->start;
		    i--)
		piece->buf[i + del] = piece->buf[i];
    if (block->length)	/* do insert */
	for (i = 0; i < block->length; ++i)
	    piece->buf[startPos - piece->start + i] = block->ptr[i];
    if (data->hasselection) {
	if (data->left > startPos || (data->left == startPos && del > 0))
	    data->left += del;
	if (data->right > startPos || (data->right == startPos && del < 0))
	    data->right += del;
	if (data->left >= data->right) data->hasselection = FALSE;
    }
    for (i=0 ; i<data->numwidgets ; i++) {
	XtTextInvalidate(data->widgets[i], startPos, startPos+del-1);
#ifdef notdef
	if (data->hasselection)
	    Xt_TextSetHighlight(data->widgets[i], data->left, data->right,
				Selected);
#endif notdef
	XtTextEnableRedisplay(data->widgets[i]);
    }
    if (data->func) (*data->func)(data->param);
    return EditDone;
}


/* Utility macro used in Scan.  Used to determine what the next character
   "after" index is, where "after" is either left or right, depending on
   what kind of search we're doing. */

#define Look(index, c)\
{									\
    if ((dir == XtsdLeft && index <= 0) ||				\
	    (dir == XtsdRight && index >= data->length))		\
	c = 0;								\
    else {								\
	if (index + doff < piece->start ||				\
		index + doff >= piece->start + piece->length)		\
	    piece = PieceFromPosition(data, index + doff);		\
	c = piece->buf[index + doff - piece->start];			\
    }									\
}



/* Scan the source.  This gets complicated, but should be explained wherever
   it is that we documented sources. */

static XtTextPosition Scan(source, position, sType, dir, count, include)
  XtTextSource source;
  XtTextPosition position;
  XtTextScanType sType;
  XtTextScanDirection dir;
  int count;
  Boolean include;
{
    EDiskData data = (EDiskData) source->data;
    XtTextPosition index;
    PiecePtr piece;
    char c;
    int ddir, doff, i, whiteSpace;
    ddir = (dir == XtsdRight) ? 1 : -1;
    doff = (dir == XtsdRight) ? 0 : -1;

    index = position;
    piece = data->piece;
    if (!piece->buf) ReadPiece(data, 0);
    switch (sType) {
	case XtstPositions:
	    if (!include && count > 0)
		count--;
	    index = CoerceToLegalPosition(data, index + count * ddir);
	    break;
	case XtstWhiteSpace:
/*	case XtstWordBreak: */
	    for (i = 0; i < count; i++) {
		whiteSpace = -1;
		while (index >= 0 && index <= data->length) {
		    Look(index, c);
		    if ((c == ' ') || (c == '\t') || (c == '\n')) {
			if (whiteSpace < 0) whiteSpace = index;
		    } else if (whiteSpace >= 0)
			break;
		    index += ddir;
		}
	    }
	    if (!include) {
		if (whiteSpace < 0 && dir == XtsdRight)
		    whiteSpace = data->length;
		index = whiteSpace;
	    }
	    index = CoerceToLegalPosition(data, index);
	    break;
	case XtstEOL:
	    for (i = 0; i < count; i++) {
		while (index >= 0 && index <= data->length) {
		    Look(index, c);
		    if (c == '\n')
			break;
		    index += ddir;
		}
		if (i < count - 1)
		    index += ddir;
	    }
	    if (include)
		index += ddir;
	    index = CoerceToLegalPosition(data, index);
	    break;
	case XtstAll:
	    if (dir == XtsdLeft)
		index = 0;
	    else
		index = data->length;
	    break;
    }
    return index;
}

#ifdef notdef
static Boolean GetSelection(source, left, right)
XtTextSource source;
XtTextPosition *left, *right; 
{
    EDiskData data = (EDiskData) source->data;
    if (data->hasselection && data->left < data->right) {
	*left = data->left;
	*right = data->right;
	return TRUE;
    }
    data->hasselection = FALSE;
    return FALSE;
}


static void SetSelection(source, left, right)
XtTextSource source;
XtTextPosition left, right; 
{
    EDiskData data = (EDiskData) source->data;
    int i;
    for (i=0 ; i<data->numwidgets; i++) {
	XtTextDisableRedisplay(data->widgets[i], FALSE);
	if (data->hasselection)
	    Xt_TextSetHighlight(data->widgets[i], data->left, data->right,
				Normal);
	if (left < right)
	    Xt_TextSetHighlight(data->widgets[i], left, right,
				Selected);
	XtTextEnableRedisplay(data->widgets[i]);
    }
    data->hasselection = (left < right);
    data->left = left;
    data->right = right;
    if (data->numwidgets > 0) {
	Widget widget = (Widget) data->widgets[0];
	if (data->hasselection)
	    XtSelectionGrab(widget, XA_PRIMARY, Convert, LoseSelection);
	else
	    XtSelectionUngrab(widget, XA_PRIMARY);
    }
}
#endif notdef



/* Public definitions. */

/* Create a new text source from the given file, and the given edit mode. */

XtTextSource XtCreateEDiskSource(name, can_edit)
char *name;
Boolean can_edit;
{
    XtTextSource  source;
    EDiskData data;
    struct stat stat;
    int     i;
    source = XtNew(XtTextSourceRec);
    data = XtNew(EDiskDataRec);
    source->data = (caddr_t) data;
#ifdef notdef
    source->AddWidget = AddWidget;
    source->RemoveWidget = RemoveWidget;
#endif /*notdef*/
    source->Read = Read;
    source->Replace = Replace;
    source->Scan = Scan;
#ifdef notdef
    source->GetSelection = GetSelection;
    source->SetSelection = SetSelection;
#else
    source->SetSelection = NULL;
    source->ConvertSelection = NULL;
#endif /*notdef*/
    source->edit_mode = can_edit ? XttextEdit : XttextRead;
    data->numwidgets = 0;
    data->widgets = XtNew(TextWidget);
    data->hasselection = FALSE;
    data->left = data->right = 0;
    data->file = myopen(name, O_RDONLY | O_CREAT, 0666);
    data->name = strcpy(XtMalloc((unsigned)strlen(name) + 1), name);
    (void)fstat(data->file, &stat);
    data->length = data->origlength = stat.st_size;
    data->numpieces = (data->length + BUFSIZE - 1) / BUFSIZE;
    if (data->numpieces < 1)
	data->numpieces = 1;
    data->piece = (PiecePtr)
	XtMalloc((unsigned) data->numpieces * sizeof(PieceRec));
    for (i = 0; i < data->numpieces; i++) {
	data->piece[i].buf = 0;
	data->piece[i].length = BUFSIZE;
	data->piece[i].start = i * BUFSIZE;
    }
    data->piece[data->numpieces - 1].length -=
	data->numpieces * BUFSIZE - data->length;
    data->startsvalid = TRUE;
    data->changed = data->everchanged = data->eversaved = FALSE;
    data->checkpointed = data->checkpointchange = FALSE;
    data->func = NULL;
    if (data->length == 0) {
	data->piece[0].buf = XtMalloc((unsigned) 1);
	data->piece[0].maxlength = 1;
    }
    return source;
}



/* Change the editing mode of a source. */

void XtEDiskChangeEditable(source, can_edit)
XtTextSource source;
Boolean can_edit;
{
    source->edit_mode = can_edit ? XttextEdit : XttextRead;
}


/* Get whether the given source is editable. */

Boolean XtEDiskGetEditable(source)
XtTextSource source;
{
    return (source->edit_mode == XttextEdit);
}



/* Call the given function with the given parameter whenever the source is
   changed. */

XtEDiskSetCallbackWhenChanged(source, func, param)
XtTextSource source;
void (*func)();
caddr_t param;
{
    EDiskData data = (EDiskData) source->data;
    data->func = func;
    data->param = param;
}


/* Return whether any unsaved changes have been made. */

XtEDiskChanged(source)
  XtTextSource source;
{
    return ((EDiskData) source->data)->changed;
}


/* Save any unsaved changes. */
/* returns 1 if successful, 0 if no save needed (errno = 0)
   or error occurred */

XtEDiskSaveFile(source)
  XtTextSource source;
{
    EDiskData data = (EDiskData) source->data;
    PiecePtr piece;
    int     i, needlseek;
    char str[1024];
    if (!data->changed) {
	errno = 0;
	return 0;
    }
    if (!data->eversaved) {
	(void)myclose(data->file);
	if ((data->file = myopen(data->name, O_RDWR, 0666)) < 0) {
	    data->file = myopen(data->name, O_RDONLY, 0666);
	    return 0;
	}
	data->eversaved = TRUE;	/* Open file in a mode where we can write. */
    }
    data->changed = data->checkpointchange = FALSE;
    if (!data->startsvalid)
	CalculateStarts(data);
    for (i = 0, piece = data->piece; i < data->numpieces; i++, piece++) {
	if (!(piece->buf) && i * BUFSIZE != piece->start)
	    ReadPiece(data, i);
    }
    needlseek = TRUE;
    for (i = 0, piece = data->piece; i < data->numpieces; i++, piece++) {
	if (piece->buf) {
	    if (needlseek) {
		(void) lseek(data->file, (long) piece->start, 0);
		needlseek = FALSE;
	    }
	    (void) write(data->file, piece->buf, piece->length);
	}
	else
	    needlseek = TRUE;
    }
    if (data->length < data->origlength)
	(void)ftruncate(data->file, data->length);
    data->origlength = data->length;
    if (data->checkpointed) {
	(void) sprintf(str, "%s.CKP", data->name);
	(void) unlink(str);
	data->checkpointed = FALSE;
    }
    return 1;
}


/* Save into a new file. */

XtEDiskSaveAsFile(source, filename)
  XtTextSource source;
  char *filename;
{
    EDiskData data = (EDiskData) source->data;
    int     i;
    PiecePtr piece;
    if (strcmp(filename, data->name) != 0) {
	for (i = 0, piece = data->piece; i < data->numpieces; i++, piece++)
	    if (!(piece->buf))
		ReadPiece(data, i);
	data->eversaved = FALSE;
	data->origlength = 0;
	data->file = creat(filename, 0666);
	data->eversaved = TRUE;
	data->origlength = 0;
	XtFree(data->name);
	data->name = strcpy(XtMalloc((unsigned) strlen(filename) + 1),
			    filename);
	data->changed = TRUE;
    }
    XtEDiskSaveFile(source);
}



/* Checkpoint this file. */

XtEDiskMakeCheckpoint(source)
  XtTextSource source;
{
    EDiskData data = (EDiskData) source->data;
    char name[1024];
    int fid, i;
    PiecePtr piece;
    if (!data->checkpointchange) return; /* No changes to save. */
    (void) sprintf(name, "%s.CKP", data->name);
    fid = creat(name, 0600);
    if (fid < 0) return;
    for (i = 0, piece = data->piece; i < data->numpieces; i++, piece++) {
	if (!(piece->buf)) ReadPiece(data, i);
	(void) write(fid, piece->buf, piece->length);
    }
    (void)close(fid);
    data->checkpointed = TRUE;
    data->checkpointchange = FALSE;
}


/* We're done with this source, free its storage. */

XtDestroyEDiskSource(source)
  XtTextSource source;
{
    EDiskData data = (EDiskData) source->data;
    int i;
    char *ptr;
    (void)myclose(data->file);
    for (i=0 ; i<data->numpieces ; i++)
	if (ptr = data->piece[i].buf) XtFree(ptr);
    XtFree((char *)data->piece);
    XtFree((char *)data->name);
    XtFree((char *)data);
    XtFree((char *)source);
}
