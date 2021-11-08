#include "copyright.h"

/* $XConsortium: XFontInfo.c,v 11.14 88/09/06 16:07:22 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/
#define NEED_REPLIES
#include "Xlibint.h"

char **XListFontsWithInfo(dpy, pattern, maxNames, actualCount, info)
register Display *dpy;
char *pattern;  /* null-terminated */
int maxNames;
int *actualCount;
XFontStruct **info;
{       
    register long nbytes;
    register int i;
    register XFontStruct *fs;
    register int size = 0;
    XFontStruct *finfo = NULL;
    char **flist = NULL;
    xListFontsWithInfoReply reply;
    register xListFontsReq *req;

    LockDisplay(dpy);
    GetReq(ListFontsWithInfo, req);
    req->maxNames = maxNames;
    nbytes = req->nbytes = pattern ? strlen (pattern) : 0;
    req->length += (nbytes + 3) >> 2;
    _XSend (dpy, pattern, nbytes);
    /* use _XSend instead of Data, since subsequent _XReply will flush buffer */

    for (i = 0; ; i++) {
	if (!_XReply (dpy, (xReply *) &reply,
	   ((SIZEOF(xListFontsWithInfoReply) - SIZEOF(xGenericReply)) >> 2), xFalse))
		return (NULL);
	if (reply.nameLength == 0)
	    break;
	if ((i + reply.nReplies) >= size) {
	    size = i + reply.nReplies + 1;
	    if (finfo) {
		finfo = (XFontStruct *) Xrealloc ((char *) finfo,
						  sizeof (XFontStruct) * size);
		flist = (char **) Xrealloc ((char *) flist,
					    sizeof (char *) * size);
	    } else {
		finfo = (XFontStruct *) Xmalloc (sizeof (XFontStruct) * size);
		flist = (char **) Xmalloc (sizeof (char *) * size);
	    }
	}
	fs = &finfo[i];

	fs->ext_data 		= NULL;
	fs->per_char		= NULL;
	fs->fid 		= None;
	fs->direction 		= reply.drawDirection;
	fs->min_char_or_byte2	= reply.minCharOrByte2;
	fs->max_char_or_byte2 	= reply.maxCharOrByte2;
	fs->min_byte1 		= reply.minByte1;
	fs->max_byte1 		= reply.maxByte1;
	fs->default_char	= reply.defaultChar;
	fs->all_chars_exist 	= reply.allCharsExist;
	fs->ascent 		= cvtINT16toInt (reply.fontAscent);
	fs->descent 		= cvtINT16toInt (reply.fontDescent);
    
#ifdef MUSTCOPY
	{
	    xCharInfo *xcip;

	    xcip = (xCharInfo *) &reply.minBounds;
	    fs->min_bounds.lbearing = xcip->leftSideBearing;
	    fs->min_bounds.rbearing = xcip->rightSideBearing;
	    fs->min_bounds.width = xcip->characterWidth;
	    fs->min_bounds.ascent = xcip->ascent;
	    fs->min_bounds.descent = xcip->descent;
	    fs->min_bounds.attributes = xcip->attributes;

	    xcip = (xCharInfo *) &reply.maxBounds;
	    fs->max_bounds.lbearing = xcip->leftSideBearing;
	    fs->max_bounds.rbearing = xcip->rightSideBearing;
	    fs->max_bounds.width = xcip->characterWidth;
	    fs->max_bounds.ascent = xcip->ascent;
	    fs->max_bounds.descent = xcip->descent;
	    fs->max_bounds.attributes = xcip->attributes;
	}
#else
	/* XXX the next two statements won't work if short isn't 16 bits */
	fs->min_bounds = * (XCharStruct *) &reply.minBounds;
	fs->max_bounds = * (XCharStruct *) &reply.maxBounds;
#endif /* MUSTCOPY */

	fs->n_properties = reply.nFontProps;
	if (fs->n_properties > 0) {
	    nbytes = reply.nFontProps * sizeof(XFontProp);
	    fs->properties = (XFontProp *) Xmalloc (nbytes);
	    nbytes = reply.nFontProps * SIZEOF(xFontProp);
	    _XRead32 (dpy, (char *)fs->properties, nbytes);
	} else
	    fs->properties = NULL;
	flist[i] = (char *) Xmalloc ((unsigned int) (reply.nameLength + 1));
	flist[i][reply.nameLength] = '\0';
	_XReadPad (dpy, flist[i], (long) reply.nameLength);
    }
    *info = finfo;
    *actualCount = i;
    UnlockDisplay(dpy);
    SyncHandle();
    return (flist);
}

XFreeFontInfo (names, info, actualCount)
char **names;
XFontStruct *info;
int actualCount;
{
	register int i;
	if (names != NULL) {
		for (i = 0; i < actualCount; i++) {
			Xfree (names[i]);
		}
		Xfree((char *) names);
	}
	if (info != NULL) {
		for (i = 0; i < actualCount; i++) {
			if (info[i].properties != NULL) 
				Xfree ((char *) info[i].properties);
			}
		Xfree((char *) info);
	}
}
