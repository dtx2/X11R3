#include "copyright.h"

/* $XConsortium: XFont.c,v 11.26 88/09/06 16:05:50 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/
#define NEED_REPLIES
#include "Xlibint.h"

XFontStruct *_XQueryFont();

XFontStruct *XLoadQueryFont(dpy, name)
   register Display *dpy;
   char *name;
{
    XFontStruct *font_result;
    register long nbytes;
    Font fid;
    xOpenFontReq *req;

    LockDisplay(dpy);
    GetReq(OpenFont, req);
    nbytes = req->nbytes  = name ? strlen(name) : 0;
    req->fid = fid = XAllocID(dpy);
    req->length += (nbytes+3)>>2;
    Data (dpy, name, nbytes);
    dpy->request--;
    font_result = (_XQueryFont(dpy, fid));
    dpy->request++;
    if (!font_result) {
       /* if _XQueryFont returned NULL, then the OpenFont request got
          a BadName error.  This means that the following QueryFont
          request is guaranteed to get a BadFont error, since the id
          passed to QueryFont wasn't really a valid font id.  To read
          and discard this second error, we call _XReply again. */
        xReply reply;
        (void) _XReply (dpy, &reply, 0, xFalse);
        }
    UnlockDisplay(dpy);
    SyncHandle();
    return font_result;
}

XFreeFont(dpy, fs)
    register Display *dpy;
    XFontStruct *fs;
{ 
    register xResourceReq *req;
    register _XExtension *ext = dpy->ext_procs;

    LockDisplay(dpy);
    while (ext) {		/* call out to any extensions interested */
	if (ext->free_Font != NULL) (*ext->free_Font)(dpy, fs, &ext->codes);
	ext = ext->next;
	}    
    GetResReq (CloseFont, fs->fid, req);
    _XFreeExtData(fs->ext_data);
    if (fs->per_char)
       Xfree ((char *) fs->per_char);
    if (fs->properties)
       Xfree ((char *) fs->properties);
    Xfree ((char *) fs);
    ext = dpy->ext_procs;
    UnlockDisplay(dpy);
    SyncHandle();
}


XFontStruct *_XQueryFont (dpy, fid)	/* Internal-only entry point */
    register Display *dpy;
    Font fid;

{
    register XFontStruct *fs;
    register long nbytes;
    xQueryFontReply reply;
    register xResourceReq *req;
    register _XExtension *ext;

    GetResReq(QueryFont, fid, req);
    if (!_XReply (dpy, (xReply *) &reply,
       ((SIZEOF(xQueryFontReply) - SIZEOF(xReply)) >> 2), xFalse))
	   return (NULL);
    fs = (XFontStruct *) Xmalloc (sizeof (XFontStruct));
    fs->ext_data 		= NULL;
    fs->fid 			= fid;
    fs->direction 		= reply.drawDirection;
    fs->min_char_or_byte2	= reply.minCharOrByte2;
    fs->max_char_or_byte2 	= reply.maxCharOrByte2;
    fs->min_byte1 		= reply.minByte1;
    fs->max_byte1 		= reply.maxByte1;
    fs->default_char 		= reply.defaultChar;
    fs->all_chars_exist 	= reply.allCharsExist;
    fs->ascent 			= cvtINT16toInt (reply.fontAscent);
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
    /* 
     * if no properties defined for the font, then it is bad
     * font, but shouldn't try to read nothing.
     */
    fs->properties = NULL;
    if (fs->n_properties > 0) {
	    nbytes = reply.nFontProps * sizeof(XFontProp);
	    fs->properties = (XFontProp *) Xmalloc (nbytes);
	    nbytes = reply.nFontProps * SIZEOF(xFontProp);
	    _XRead32 (dpy, (char *)fs->properties, nbytes);
    }
    /*
     * If no characters in font, then it is a bad font, but
     * shouldn't try to read nothing.
     */
    /* have to unpack charinfos on some machines (CRAY) */
    fs->per_char = NULL;
    if (reply.nCharInfos > 0){
	    nbytes = reply.nCharInfos * sizeof(XCharStruct);
	    fs->per_char = (XCharStruct *) Xmalloc (nbytes);
	    nbytes = reply.nCharInfos * SIZEOF(xCharInfo);
	    _XRead16 (dpy, (char *)fs->per_char, nbytes);
    }

    ext = dpy->ext_procs;
    while (ext) {		/* call out to any extensions interested */
	if (ext->create_Font != NULL) 
		(*ext->create_Font)(dpy, fs, &ext->codes);
	ext = ext->next;
	}    
    return (fs);
}


XFontStruct *XQueryFont (dpy, fid)
    register Display *dpy;
    Font fid;
{
    XFontStruct *font_result;

    LockDisplay(dpy);
    font_result = _XQueryFont(dpy,fid);
    UnlockDisplay(dpy);
    SyncHandle();
    return font_result;
}

   
   
