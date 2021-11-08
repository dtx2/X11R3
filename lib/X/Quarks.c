/*
 * $XConsortium: Quarks.c,v 1.13 88/09/19 13:55:48 jim Exp $
 */

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

#include "Xlibint.h"
#include "Xresource.h"

extern void bcopy();


typedef long Signature;

static XrmQuark nextQuark = 1;	/* next available quark number */
static XrmString *quarkToStringTable = NULL;
static int maxQuarks = 0;	/* max names in current quarkToStringTable */
#define QUARKQUANTUM 600;	/* how much to extend quarkToStringTable by */


/* Permanent memory allocation */

#define NEVERFREETABLESIZE 8180
static char *neverFreeTable = NULL;
static int  neverFreeTableSize = 0;

char *Xpermalloc(length)
    unsigned int length;
{
    char *ret;

#ifdef WORD64
    /* round to nearest 8-byte boundary */
    length = (length + 7) & (~7);
#else
    /* round to nearest 4-byte boundary */
    length = (length + 3) & (~3);
#endif /* WORD64 */
    if (neverFreeTableSize < length) {
	neverFreeTableSize =
	    (length > NEVERFREETABLESIZE ? length : NEVERFREETABLESIZE);
	neverFreeTable = Xmalloc(neverFreeTableSize);
    }
    ret = neverFreeTable;
    neverFreeTable += length;
    neverFreeTableSize -= length;
    return(ret);
}


typedef struct _NodeRec *Node;
typedef struct _NodeRec {
    Node 	next;
    Signature	sig;
    XrmQuark	quark;
    XrmString	name;
} NodeRec;

#define HASHTABLESIZE 1024
#define HASHTABLEMASK 1023
static Node nodeTable[HASHTABLESIZE];



/* predefined quarks */

/* Representation types */

XrmQuark  XrmQBoolean;
XrmQuark  XrmQColor;
XrmQuark  XrmQCursor;
XrmQuark  XrmQDims;
XrmQuark  XrmQDisplay;
XrmQuark  XrmQFile;
XrmQuark  XrmQFont;
XrmQuark  XrmQFontStruct;
XrmQuark  XrmQGeometry;
XrmQuark  XrmQInt;
XrmQuark  XrmQPixel;
XrmQuark  XrmQPixmap;
XrmQuark  XrmQPointer;
XrmQuark  XrmQString;
XrmQuark  XrmQWindow;

/* "Enumeration" constants */

XrmQuark  XrmQEfalse;
XrmQuark  XrmQEno;
XrmQuark  XrmQEoff;
XrmQuark  XrmQEon;
XrmQuark  XrmQEtrue;
XrmQuark  XrmQEyes;


static XrmAllocMoreQuarkToStringTable()
{
    unsigned	size;

    maxQuarks += QUARKQUANTUM;
    size = (unsigned) maxQuarks * sizeof(XrmString);
    if (quarkToStringTable == (XrmString *)NULL)
	quarkToStringTable = (XrmString *) Xmalloc(size);
    else
	quarkToStringTable =
		(XrmString *) Xrealloc((char *) quarkToStringTable, size);
}

XrmQuark XrmStringToQuark(name)
    register XrmString name;
{
    register Signature 	sig = 0;
    register Signature	scale = 27;
    register XrmString	tname;
    register Node	np;
    register XrmString	npn;
    	     Node	*hashp;
	     unsigned	strLength;

    if (name == NULL)
	return (NULLQUARK);

    /* Compute string signature (sparse 32-bit hash value) */
    for (tname = name; *tname != '\0'; tname++) {
	sig = sig*scale + (unsigned int) *tname;
    }
    strLength = tname - name + 1;

    /* Look for string in hash table */
    hashp = &nodeTable[sig & HASHTABLEMASK];
    for (np = *hashp; np != NULL; np = np->next) {
	if (np->sig == sig) {
	    for (npn=np->name, tname = name;
	     ((scale = *tname) != 0) && (scale == *npn); ++tname, ++npn) {};
	    if (scale == *npn) {
	        return np->quark;
	    }
	}
    }

    /* Not found, add string to hash table */
    np = (Node) Xpermalloc(sizeof(NodeRec));
    np->next = *hashp;
    *hashp = np;
    np->sig = sig;
    bcopy(name, (np->name = Xpermalloc(strLength)), (int) strLength);
    np->quark = nextQuark;

    if (nextQuark >= maxQuarks)
	XrmAllocMoreQuarkToStringTable();

    quarkToStringTable[nextQuark] = np->name;
    ++nextQuark;
    return np->quark;
}

extern XrmQuark XrmUniqueQuark()
{
    XrmQuark quark;

    quark = nextQuark;
    if (nextQuark >= maxQuarks)
	XrmAllocMoreQuarkToStringTable();
    quarkToStringTable[nextQuark] = NULLSTRING;
    ++nextQuark;
    return (quark);
}


XrmString XrmQuarkToString(quark)
    XrmQuark quark;
{
    if (quark <= 0 || quark >= nextQuark)
    	return NULLSTRING;
    return quarkToStringTable[quark];
}
