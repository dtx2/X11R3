#ifndef lint
static char Xrcsid[] = "$XConsortium: StringSrc.c,v 1.20 88/10/18 12:31:59 swick Exp $";
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

/* File: StringSource.c */

#include <sys/types.h>
#include <sys/stat.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Text.h>
#include <X11/TextP.h>

/* Private StringSource Definitions */

#define MAGICVALUE		-1
#define DEFAULTMAXLENGTH	100


typedef struct _StringSourceData {
    char *str;
    XtTextPosition length, maxLength;
    XtTextPosition left, right;		/* selection */
} StringSourceData, *StringSourcePtr;

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

static int magic_value = MAGICVALUE;

static XtResource stringResources[] = {
    {XtNstring, XtCString, XtRString, sizeof (char *),
        XtOffset(StringSourcePtr, str), XtRString, NULL},
    {XtNlength, XtCLength, XtRInt, sizeof (int),
        XtOffset(StringSourcePtr, maxLength), XtRInt, (caddr_t)&magic_value},
};

static XtResource sourceResources[] = {
    {XtNeditType, XtCEditType, XtREditMode, sizeof(XtTextEditType), 
        XtOffset(XtTextSource, edit_mode), XtRString, "read"},
};

char Look(data, position, direction)
  StringSourcePtr data;
  XtTextPosition position;
  XtTextScanDirection direction;
{
/* Looking left at pos 0 or right at position data->length returns newline */
    if (direction == XtsdLeft) {
	if (position == 0)
	    return(0);
	else
	    return(data->str[position-1]);
    }
    else {
	if (position == data->length)
	    return(0);
	else
	    return(data->str[position]);
    }
}

static XtTextPosition StringReadText (src, pos, text, maxRead)
  XtTextSource src;
  int pos;
  XtTextBlock *text;
  int maxRead;
{
    int     charsLeft;
    StringSourcePtr data;

    data = (StringSourcePtr) src->data;
    text->firstPos = pos;
    text->ptr = data->str + pos;
    charsLeft = data->length - pos;
    text->length = (maxRead > charsLeft) ? charsLeft : maxRead;
    return pos + text->length;
}

static int StringReplaceText (src, startPos, endPos, text)
  XtTextSource src;
  XtTextPosition startPos, endPos;
  XtTextBlock *text;
{
    StringSourcePtr data;
    int     i, length, delta;

    data = (StringSourcePtr) src->data;
    switch (src->edit_mode) {
        case XttextAppend: 
	    if (startPos != endPos || endPos!= data->length)
		return (PositionError);
	    break;
	case XttextRead:
	    return (EditError);
	case XttextEdit:
	    break;
	default:
	    return (EditError);
    }
    length = endPos - startPos;
    if ((data->length - length + text->length) > data->maxLength)
        return (EditError);

    delta = text->length - length;
    if (delta < 0)		/* insert shorter than delete, text getting
				   shorter */
	for (i = startPos; i < data->length + delta; ++i)
	    data->str[i] = data->str[i - delta];
    else
	if (delta > 0)	{	/* insert longer than delete, text getting
				   longer */
	    for (i = data->length; i > startPos-1; --i)
		data->str[i + delta] = data->str[i];
	}
    if (text->length != 0)	/* do insert */
	for (i = 0; i < text->length; ++i)
	    data->str[startPos + i] = text->ptr[i];
    data->length = data->length + delta;
    data->str[data->length] = 0;
    return (EditDone);
}

static StringSetLastPos (src, lastPos)
  XtTextSource src;
  XtTextPosition lastPos;
{
    ((StringSourceData *) (src->data))->length = lastPos;
}

static XtTextPosition StringScan (src, pos, sType, dir, count, include)
  XtTextSource	 src;
  XtTextPosition pos;
  XtTextScanType sType;
  XtTextScanDirection dir;
  int		 count;
  Boolean	 include;
{
    StringSourcePtr data;
    XtTextPosition position;
    int     i, whiteSpace;
    char    c;
    int ddir = (dir == XtsdRight) ? 1 : -1;

    data = (StringSourcePtr) src->data;
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
		whiteSpace = -1;
		while (position >= 0 && position <= data->length) {
		    c = Look(data, position, dir);
		    if ((c == ' ') || (c == '\t') || (c == '\n')){
		        if (whiteSpace < 0) whiteSpace = position;
		    } else if (whiteSpace >= 0)
			break;
		    position += ddir;
		}
	    }
	    if (!include) {
		if(whiteSpace < 0 && dir == XtsdRight) whiteSpace = data->length;
		position = whiteSpace;
	    }
	    break;
	case XtstEOL: 
	    for (i = 0; i < count; i++) {
		while (position >= 0 && position <= data->length) {
		    if (Look(data, position, dir) == '\n')
			break;
		    if(((dir == XtsdRight) && (position == data->length)) || 
			(dir == XtsdLeft) && ((position == 0)))
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
    if (position < 0) position = 0;
    if (position > data->length) position = data->length;
    return(position);
}

static void SetSelection(src, left, right, selection)
    XtTextSource src;
    Atom selection;
    XtTextPosition left, right;
{
    ((StringSourcePtr)src->data)->left = left;
    ((StringSourcePtr)src->data)->right = right;
}

/***** Public routines *****/

XtTextSource XtStringSourceCreate (parent, args, argCount)
    Widget parent;
    ArgList args;
    Cardinal argCount;
{
    XtTextSource src;
    StringSourcePtr data;

    src = XtNew(XtTextSourceRec);

    XtGetSubresources (parent, (caddr_t)src, XtNtextSource, XtCTextSource,
        sourceResources, XtNumber(sourceResources), args, argCount);

    src->Read = StringReadText;
    src->Replace = StringReplaceText;
    src->SetLastPos = StringSetLastPos;
    src->Scan = StringScan;
    src->SetSelection = SetSelection;
    src->ConvertSelection = NULL;
    data = XtNew(StringSourceData);
    src->data = (caddr_t)data;
    data->left = data->right = 0;

    XtGetSubresources (parent, (caddr_t)data, XtNtextSource, XtCTextSource,
        stringResources, XtNumber(stringResources), args, argCount);

    if (data->str == NULL) {
	if (data->maxLength == MAGICVALUE) data->maxLength = DEFAULTMAXLENGTH;
	data->str = (char *) XtMalloc ((unsigned)data->maxLength);
	data->length = 0;
    } else {
        data->length = strlen(data->str);
	if (data->maxLength < data->length) data->maxLength = data->length;
    }

    return src;
}

void XtStringSourceDestroy (src)
  XtTextSource src;
{
    XtFree((char *) src->data);
    XtFree((char *) src);
}
