#ifndef lint
static char rcs_id[] = "$XConsortium: ap.c,v 1.12 88/10/20 09:26:12 swick Exp $";
#endif

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

#include "xedit.h"

#define chunk 2048

typedef struct {
    char *buf;
    int size;
    XtTextPosition pos;
    XtTextSource strSrc;
} ApAsSourceData;

/* Private Routines */


static XtTextPosition ApAsGetLastPos(src)
  XtTextSource src;
{
  ApAsSourceData *data;
    data = (ApAsSourceData *)src->data;
    return (XtTextPosition)(*data->strSrc->GetLastPos)(data->strSrc);
}

static ApAsSetLastPos(src, lastPos)
  XtTextSource src;
  XtTextPosition lastPos;
{
}

static XtTextPosition  ApAsRead(src, pos, text, maxRead)
  XtTextSource src;
  int pos;
  XtTextBlock *text;
  int maxRead;
{
  ApAsSourceData *data;
    data = (ApAsSourceData *)src->data;
    return ((*data->strSrc->Read)(data->strSrc, pos, text, maxRead));
}


static Arg stringargs[] = {
  {XtNstring, (XtArgVal) NULL},
  {XtNlength, (XtArgVal) NULL},
  {XtNeditType, (XtArgVal) XttextEdit},
  };

static int ApAsReplace(src, startPos, endPos, text)
  XtTextSource src;
  XtTextPosition startPos, endPos;
  XtTextBlock *text;
{
  ApAsSourceData *data;
  int i;
    data = (ApAsSourceData *)src->data;
    if((startPos!=endPos) || (startPos!=data->pos))
        return 0;
    if((data->pos + text->length) >= data->size){
        while((data->pos + text->length) >= data->size){
            data->size += chunk;
            data->buf = realloc(data->buf, data->size);  /* optimize this!!! */
        } 
        XtStringSourceDestroy(data->strSrc);
	stringargs[0].value = (XtArgVal)data->buf ;
	stringargs[1].value = (XtArgVal)data->size ;
        data->strSrc = (XtTextSource)
  	    XtStringSourceCreate(toplevel,stringargs,XtNumber(stringargs));
    }
    i = (*data->strSrc->Replace)(data->strSrc, startPos, endPos, text);
    data->pos += text->length;
    return (i);
}

static XtTextPosition ApAsScan (src, pos, sType, dir, count, include)
  XtTextSource src;
  XtTextPosition pos;
  XtTextScanType sType;
  XtTextScanDirection dir;
  int count, include;
{
  ApAsSourceData *data;
    data = (ApAsSourceData *)src->data;
    return 
     ((*data->strSrc->Scan)(data->strSrc, pos, sType, dir, count, include));
}


/* Public routines */

XtTextSource TCreateApAsSource ()
{
  XtTextSource src;
  ApAsSourceData *data;
    src = (XtTextSource) malloc(sizeof(XtTextSourceRec));
    src->Read = ApAsRead;
    src->Replace = ApAsReplace; 
    src->GetLastPos = ApAsGetLastPos;
    src->SetLastPos = ApAsSetLastPos;
    src->Scan = ApAsScan;
    src->GetSelection = NULL;
    src->SetSelection = NULL;
    src->ConvertSelection = NULL;
    src->edit_mode = XttextEdit;
    data = (ApAsSourceData *)(malloc(sizeof(ApAsSourceData)));
    data->buf = calloc(chunk,1);
    data->pos = 0;
    data->size = chunk;
    stringargs[0].value = (XtArgVal)data->buf ;
    stringargs[1].value = (XtArgVal)data->size ;
    data->strSrc = (XtTextSource) XtStringSourceCreate (toplevel,stringargs,
				   XtNumber(stringargs));
    src->data = (caddr_t)data;
    return src;
}

TDestroyApAsSource(src)
  XtTextSource src;
{
  ApAsSourceData *data;
    data = (ApAsSourceData *)src->data;
    XtStringSourceDestroy(data->strSrc);
    free(data->buf);
    free(data);
    free(src);
}
