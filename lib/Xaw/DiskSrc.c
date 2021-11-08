#ifndef lint
static char Xrcsid[] = "$XConsortium: DiskSrc.c,v 1.20 88/10/18 12:29:54 swick Exp $";
#endif lint


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* File: DiskSource.c */
/* Documentation for source specfic routine semantics may be found in the
 * TextPrivate.h file.
 */

#include <stdio.h>
#include <sys/param.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xmu.h>
#include <X11/TextP.h>

#define TMPSIZ 32		/* bytes to allocate for tmpnam */

extern char *tmpnam();
void bcopy();

/** private DiskSource definitions **/

typedef struct _DiskSourceData {
	/* resources */
    char       *fileName;
	/* private data */
    Boolean	is_tempfile;
    FILE *file;		
    XtTextPosition position, 	/* file position of first char in buffer */
 		   length; 	/* length of file */
    char *buffer;		/* piece of file in memory */
    int charsInBuffer;		/* number of bytes used in memory */
} DiskSourceData, *DiskSourcePtr;

#define bufSize 1000

#define Increment(data, position, direction)\
{\
    if (direction == XtsdLeft) {\
	if (position > 0) \
	    position -= 1;\
    }\
    else {\
	if (position < data->length)\
	    position += 1;\
    }\
}

static XtResource diskResources[] = {
    {XtNfile, XtCFile, XtRString, sizeof (char *),
        XtOffset(DiskSourcePtr, fileName), XtRString, NULL},
};

static XtResource sourceResources[] = {
    {XtNeditType, XtCEditType, XtREditMode, sizeof(int), 
        XtOffset(XtTextSource, edit_mode), XtRString, "read"},
};

static char Look(data, position, direction)
  DiskSourcePtr data;
  XtTextPosition position;
  XtTextScanDirection direction;
{

    if (direction == XtsdLeft) {
	if (position == 0)
	    return('\n');
	else {
	    FillBuffer(data, position - 1);
	    return(data->buffer[position - data->position - 1]);
	}
    }
    else {
	if (position == data->length)
	    return('\n');
	else {
	    FillBuffer(data, position);
	    return(data->buffer[position - data->position]);
	}
    }
}



static XtTextPosition DiskReadText (src, pos, text, length)
  XtTextSource src;
  XtTextPosition pos;	/** starting position */
  XtTextBlock *text;	/** RETURNED: text read in */
  int length;		/** max number of bytes to read **/
{
    XtTextPosition count;
    DiskSourcePtr data;

    data = (DiskSourcePtr) src->data;
    FillBuffer(data, pos);
    text->firstPos = pos;
    text->ptr = data->buffer + (pos - data->position);
    count = data->charsInBuffer - (pos - data->position);
    text->length = (length > count) ? count : length;
    return pos + text->length;
}

/*
 * this routine reads text starting at "pos" into memory.
 * Contains heuristic for keeping the read position centered in the buffer.
 */
static int FillBuffer (data, pos)
  DiskSourcePtr data;
  XtTextPosition pos;
{
    long readPos;
    if ((pos < data->position ||
	    pos >= data->position + data->charsInBuffer - 100) &&
	    data->charsInBuffer != data->length) {
	if (pos < (bufSize / 2))
	    readPos = 0;
	else
	    if (pos >= data->length - bufSize)
		readPos = data->length - bufSize;
	    else
		if (pos >= data->position + data->charsInBuffer - 100)
		    readPos = pos - (bufSize / 2);
		else
		    readPos = pos;
	(void) fseek(data->file, readPos, 0);
	data->charsInBuffer = fread(data->buffer, sizeof(char), bufSize,
				data->file);
	data->position = readPos;
    }
}

/*
 * This is a dummy routine for read only disk sources.
 */
/*ARGSUSED*/  /* keep lint happy */
static int DummyReplaceText (src, startPos, endPos, text)
  XtTextSource src;
  XtTextPosition startPos, endPos;
  XtTextBlock *text;
{
    return(EditError);
}


/*
 * This routine will only append to the end of a source.  If incorrect
 * starting and ending positions are given, an error will be returned.
 */
static int DiskAppendText (src, startPos, endPos, text)
  XtTextSource src;
  XtTextPosition startPos, endPos;
  XtTextBlock *text;
{
    long topPosition = 0;
    char *tmpPtr;
    DiskSourcePtr data;
    data = (DiskSourcePtr) src->data;
    if (startPos != endPos || endPos != data->length)
        return (PositionError);
    /* write the new text to the end of the file */
    if (text->length > 0) {
	(void) fseek(data->file, data->length, 0);
	(void) fwrite(text->ptr, sizeof(char), text->length, data->file);
    } else
	/* if the delete key was hit, blank out last char in the file */
	if (text->length < 0) {
		(void) fseek(data->file, data->length-1, 0);
		(void) fwrite(" ", sizeof(char), 1, data->file);
	}
    /* need this in case the application trys to seek to end of file. */
     (void) fseek(data->file, topPosition, 2);	
     
    /* put the new text into the buffer in memory */
    data->length += text->length;
    if (data->charsInBuffer + text->length <= bufSize) {
/**** NOTE: need to check if text won't fit in the buffer ***/
	if (text->length > 0) {
		tmpPtr = data->buffer + data->charsInBuffer;
		bcopy(text->ptr, tmpPtr, text->length);
	}
	data->charsInBuffer += text->length;
    } else
	FillBuffer(data, data->length - text->length);

    return (EditDone);
}


static int DiskSetLastPos (src, lastPos)
  XtTextSource src;
  XtTextPosition lastPos;
{
    ((DiskSourceData *)(src->data))->length = lastPos;
}

/*
 * This routine will start at
 * the "pos" position of the source and scan in the appropriate
 * direction until it finds something of the right sType.  It returns 
 * the new position.  If upon reading it hits the end of the buffer
 * in memory, it will refill the buffer.
 */
static XtTextPosition DiskScan (src, pos, sType, dir, count, include)
  XtTextSource 	 src;
  XtTextPosition pos;
  XtTextScanType sType;
  XtTextScanDirection  dir;
  int     	 count;
  Boolean	 include;
{
    DiskSourcePtr data;
    XtTextPosition position;
    int     i, whiteSpace;
    char    c;

    data = (DiskSourcePtr) src->data;
    position = pos;
    switch (sType) {
	case XtstPositions: 
	    if (!include && count > 0)
		count -= 1;
	    for (i = 0; i < count; i++) {
		Increment(data, position, dir);
	    }
	    break;
	case XtstWhiteSpace: 
	    for (i = 0; i < count; i++) {
		whiteSpace = 0;
		while (position >= 0 && position <= data->length) {
		    FillBuffer(data, position);
		    c = Look(data, position, dir);
		    whiteSpace = (c == ' ') || (c == '\t') || (c == '\n');
		    if (whiteSpace)
			break;
		    Increment(data, position, dir);
		}
		if (i + 1 != count)
		    Increment(data, position, dir);
	    }
	    if (include)
		Increment(data, position, dir);
	    break;
	case XtstEOL: 
	    for (i = 0; i < count; i++) {
		while (position >= 0 && position <= data->length) {
		    if (Look(data, position, dir) == '\n')
			break;
		    Increment(data, position, dir);
		}
		if (i + 1 != count)
		    Increment(data, position, dir);
	    }
	    if (include) {
	    /* later!!!check for last char in file # eol */
		Increment(data, position, dir);
	    }
	    break;
	case XtstAll: 
	    if (dir == XtsdLeft)
		position = 0;
	    else
		position = data->length;
    }
    return(position);
}

static Boolean ConvertSelection(d, src, selection, target,
				type, value, length, format)
  Display* d;
  XtTextSource src;
  Atom *selection, *target, *type;
  caddr_t *value;
  unsigned long *length;
  int *format;
{
    DiskSourcePtr data = (DiskSourcePtr)src->data;
    if (*selection != XA_PRIMARY) return False;
    if (*target == XA_TARGETS(d)) {
	*value = XtMalloc(sizeof(Atom));
	*(Atom*)*value = XA_FILENAME(d);
	*type = XA_ATOM;
	*length = 1;
	*format = 32;
	return True;
    } else if (*target == XA_FILENAME(d)) {
#ifdef unix
	if (*data->fileName != '/') {
	    char fullname[MAXPATHLEN+2];		/* +2 for getcwd */
	    char *p = data->fileName;
	    int len;
#ifdef SYSV
	    extern char *getcwd();
#define get_current_directory(buf,len) getcwd (buf, len)
#else
	    extern char *getwd();
#define get_current_directory(buf,len) getwd (buf)
#endif

	    if (get_current_directory (fullname, sizeof fullname) == NULL)
	      fullname[0] = '\0';

	    while (*p == '.') {
		if (*++p == '/') {
		    p++;
		    continue;
		}
		if (*p == '.' && *++p == '/') {
		    char *d = rindex(fullname, '/');
		    if (d != NULL) *d = '\0';
		    p++;
		    continue;
		}
		else break;
	    }
	    if (fullname[len=strlen(fullname)] != '/')
		fullname[len++] = '/';
	    strcpy(&fullname[len], p);
	    *value = XtNewString(fullname);
	}
	else
#endif /*unix*/
	    *value = XtNewString(data->fileName);
	*type = XA_STRING;
	*length = strlen(*value);
	*format = 8;
	return True;
    }
    /* else */
    return False;
}


/******* Public routines **********/

XtTextSource XtDiskSourceCreate(parent, args, num_args)
    Widget	parent;
    ArgList	args;
    Cardinal	num_args;
{
    XtTextSource src;
    DiskSourcePtr data;
    long topPosition = 0;

    src = XtNew(XtTextSourceRec);

    XtGetSubresources (parent, (caddr_t)src, XtNtextSource, XtCTextSource,
		       sourceResources, XtNumber(sourceResources),
		       args, num_args);

    src->Read = DiskReadText;
    src->SetLastPos = DiskSetLastPos;
    src->Scan = DiskScan;
    src->SetSelection = NULL;
    src->ConvertSelection = ConvertSelection;
    data = XtNew(DiskSourceData);
    src->data = (caddr_t)data;

    XtGetSubresources (parent, (caddr_t)data, XtNtextSource, XtCTextSource,
        diskResources, XtNumber(diskResources),
	args, num_args);

    if (data->fileName == NULL) {
	data->fileName = tmpnam (XtMalloc((unsigned)TMPSIZ));
	data->is_tempfile = TRUE;
    } else
        data->is_tempfile = FALSE;

    switch (src->edit_mode) {
        case XttextRead:
           if ((data->file = fopen(data->fileName, "r")) == 0)
                XtError("Cannot open source file in XtDiskSourceCreate");
            src->Replace = DummyReplaceText;
            break;
        case XttextAppend:
            if ((data->file = fopen(data->fileName, "r+")) == 0)
                XtError("Cannot open source file in XtDiskSourceCreate");
            src->Replace = DiskAppendText;
            break;
        default:
            if ((data->file = fopen(data->fileName, "r")) == 0)
                XtError("Cannot open source file in XtDiskSourceCreate");
            src->Replace = DummyReplaceText;
    }
    (void) fseek(data->file, topPosition, 2);  
    data->length = ftell (data->file);  
    data->buffer = (char *) XtMalloc((unsigned)bufSize);
    data->position = 0;
    data->charsInBuffer = 0;
    return src;
}

void XtDiskSourceDestroy (src)
  XtTextSource src;
{
    DiskSourcePtr data;
    data = (DiskSourcePtr) src->data;
    XtFree((char *) data->buffer);
    if (data->is_tempfile) {
        unlink(data->fileName);
	XtFree((char *) data->fileName);
    }
    XtFree((char *) src->data);
    XtFree((char *) src);
}
