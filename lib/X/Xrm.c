/*
 * $XConsortium: Xrm.c,v 1.19 88/09/19 13:56:07 jim Exp $
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

#include	<X11/Xlib.h>
#include	"Xlibint.h"
#include	<X11/Xresource.h>
#include	<stdio.h>
#include	<ctype.h>


extern void bzero();

XrmQuark    XrmQString;

typedef	void (*DBEnumProc)();

#define HASHSIZE	64
#define HASHMASK	63
#define HashIndex(quark)	(quark & HASHMASK)

/*
typedef struct _XrmHashBucketRec	*XrmHashBucket;
*/
typedef struct _XrmHashBucketRec {
    XrmHashBucket	next;       /* Next entry in this hash chain	    */
    XrmQuark		quark;      /* Quark for string			    */
    XrmRepresentation   type;       /* Representation of value (if any)     */
    XrmValue		value;      /* Value of this node (if any)	    */
    XrmHashTable	tables[2];  /* Hash table pointers for tight, loose */
} XrmHashBucketRec;

/*
typedef XrmHashBucket	*XrmHashTable;
*/

/*
typedef XrmHashTable XrmSearchList[];
*/


void XrmStringToQuarkList(name, quarks)
    register char 	 *name;
    register XrmQuarkList quarks;   /* RETURN */
{
    register int    i;
    register char   ch;
    static char     oneName[1000];

    if (name != NULL) {
	for (i = 0 ; ((ch = *name) != '\0') ; name++) {
	    if (ch == '.') {
		if (i != 0) {
		    oneName[i] = 0;
		    *quarks = XrmStringToQuark(oneName);
		    quarks++;
		    i = 0;
		}
	    } else {
		oneName[i] = ch;
	    	i++;
	    }
	}
	if (i != 0) {
	    oneName[i] = 0;
	    *quarks = XrmStringToQuark(oneName);
	    quarks++;
	}
    }
    *quarks = NULLQUARK;
}

void XrmStringToBindingQuarkList(name, bindings, quarks)
    register char	    *name;
    register XrmBindingList bindings;   /* RETURN */
    register XrmQuarkList   quarks;     /* RETURN */
{
    register int	i;
    register XrmBinding binding;
    register char       ch;
    char		oneName[1000];

    if (name != NULL) {
	binding = XrmBindTightly;
	for (i = 0 ; ((ch = *name) != '\0') ; name++) {
	    if (ch == '.' || ch == '*') {
		if (i != 0) {
		    /* Found a complete name */
		    *bindings = binding;
		    bindings++;
		    oneName[i] = '\0';
		    *quarks = XrmStringToQuark(oneName);
		    quarks++;
		    i = 0;
		    binding = XrmBindTightly;
		}
		if (ch == '*') binding = XrmBindLoosely;
	    } else {
		oneName[i] = ch;
	    	i++;
	    }
	}
	/* Do last name */
	if (i != 0) {
	    oneName[i] = '\0';
	    *bindings = binding;
	    *quarks = XrmStringToQuark(oneName);
	    quarks++;
	}
    }
    *quarks = NULLQUARK;
} /* XrmStringToBindingQuarkList */


static void PutEntry(bucket, bindings, quarks, type, value)
    register XrmHashBucket	bucket;
    register XrmBindingList	bindings;
    register XrmQuarkList	quarks;
	     XrmRepresentation  type;
    	     XrmValuePtr	value;
{
    register XrmHashBucket	*pBucket;
    register int		binding;
    register int		quark;

    for (; (quark = *quarks) != NULLQUARK; quarks++, bindings++) {
	binding = (int) *bindings;

	/* Allocate new hash table if needed */
	if (bucket->tables[binding] == NULL) {
	    bucket->tables[binding] =
		(XrmHashTable) Xmalloc(sizeof(XrmHashBucket) * HASHSIZE);
	    bzero((char *) bucket->tables[binding],
		sizeof(XrmHashBucket) * HASHSIZE);
	}

	/* Find bucket containing quark if possible */
	pBucket = &(bucket->tables[binding][HashIndex(quark)]);
	bucket = *pBucket;
	while ((bucket != NULL) && (bucket->quark != quark)) {
	    bucket = bucket->next;
	}

	/* Create new bucket if needed */
	if (bucket == NULL) {
	    bucket = (XrmHashBucket) Xmalloc(sizeof(XrmHashBucketRec));
	    bucket->next = *pBucket;
	    *pBucket = bucket;
	    bucket->quark = quark;
	    bucket->type = NULLQUARK;
	    bucket->value.addr = NULL;
	    bucket->value.size = 0;
	    bucket->tables[(int) XrmBindTightly] = NULL;
	    bucket->tables[(int) XrmBindLoosely] = NULL;
	}
    } /* for */

    /* Set value passed in */
    if (bucket->value.addr != NULL) {
	Xfree((char *) bucket->value.addr);
    }
    bucket->type = type;
    bucket->value.size = value->size;
    bucket->value.addr = (caddr_t) Xmalloc(value->size);
    bcopy((char *) value->addr, (char *) bucket->value.addr, (int) value->size);
} /* PutEntry */


static Bool GetEntry(tight, loose, names, classes, type, value)
	     XrmHashTable	tight;
    register XrmHashTable	loose;
    register XrmNameList	names;
    register XrmClassList	classes;
    	     XrmRepresentation  *type;  /* RETURN */
    	     XrmValuePtr	value;  /* RETURN */
{
    register XrmHashBucket	bucket;
    register XrmName		name;
    register XrmClass		class;
	     XrmHashTable       nTight, nLoose;

    /* (tight != NULL || loose != NULL) && names[0] != NULLQUARK */

#define GetEntryLookup(table, q) 					    \
{									    \
    bucket = table[HashIndex(q)];					    \
    while (bucket != NULL) {						    \
	if (bucket->quark == q) {					    \
	    if (names[1] == NULLQUARK) {				    \
		/* Must be leaf node with data, else doesn't match */       \
		if (bucket->value.addr == NULL) {			    \
		    return False;					    \
		} else {						    \
		    *type = bucket->type;				    \
		    *value = bucket->value;				    \
		    return True;					    \
		}							    \
	    } else {							    \
		nTight = bucket->tables[(int) XrmBindTightly];		    \
		nLoose = bucket->tables[(int) XrmBindLoosely];		    \
		if ((nTight != NULL || nLoose != NULL)			    \
		    && GetEntry(nTight, nLoose, names+1, classes+1,	    \
			type, value)) {					    \
		return True;						    \
		}							    \
	    break;							    \
	    }								    \
	}								    \
	bucket = bucket->next;						    \
    }									    \
} /* GetEntryLookup */

    /* Check very first name & class in both tight and loose tables */
    name = *names;
    if (tight != NULL) GetEntryLookup(tight, name);
    if (loose != NULL) GetEntryLookup(loose, name);
    class = *classes;
    if (tight != NULL) GetEntryLookup(tight, class);
    if (loose != NULL) GetEntryLookup(loose, class);

    /* Now check any remaining names and class, but just in loose table */
    if (loose != NULL) {
	names++;
	classes++;
	for (;(name = *names) != NULLQUARK; names++, classes++) {
	    GetEntryLookup(loose, name);
	    class = *classes;
	    GetEntryLookup(loose, class);
	}
    }

    /* Didn't find any of the names or classes in either hash table */
    return False;
} /* GetEntry */


static int numTables;
static int lenTables;

static void GetTables(tight, loose, names, classes, tables)
             XrmHashTable   tight;
    register XrmHashTable   loose;
    register XrmNameList    names;
    register XrmClassList   classes;
	     XrmSearchList  tables;     /* RETURN */
{
    register XrmName	    name;
    register XrmClass       class;
    register XrmHashBucket  bucket;
             XrmHashTable   nTight, nLoose;
	
#define GetTablesLookup(table, q)					    \
{									    \
    bucket = table[HashIndex(q)];					    \
    while (bucket != NULL) {						    \
	if (bucket->quark == q) {					    \
	    nTight = bucket->tables[(int) XrmBindTightly];		    \
	    nLoose = bucket->tables[(int) XrmBindLoosely];		    \
	    if (nTight != NULL || nLoose != NULL) {			    \
		if (names[1] != NULLQUARK) {				    \
		    GetTables(nTight, nLoose, names+1, classes+1, tables);  \
		} else if (nTight != NULL) {				    \
		    if (numTables == lenTables) return;			    \
		    tables[numTables++] = nTight;			    \
		}							    \
		if (nLoose != NULL) {					    \
		    if (numTables == lenTables) return;			    \
		    tables[numTables++] = nLoose;			    \
		}							    \
	    }								    \
	    break;							    \
	}								    \
	bucket = bucket->next;						    \
    }									    \
} /* GetTablesLookup */

    /* Check first name and class in both tight and loose tables */
    name = *names;
    if (tight != NULL) GetTablesLookup(tight, name);
    if (loose != NULL) GetTablesLookup(loose, name);
    class = *classes;
    if (tight != NULL) GetTablesLookup(tight, class);
    if (loose != NULL) GetTablesLookup(loose, class);

    /* Now check any remaining names and class, but just in loose table */
    if (loose != NULL) {
	names++;
	classes++;
	for (; (name = *names) != NULLQUARK; names++, classes++) {
	    GetTablesLookup(loose, name);
	    class = *classes;
	    GetTablesLookup(loose, class);
	}
    }
} /* GetTables */

static XrmDatabase NewDatabase()
{
    register XrmHashBucket   bucket;

    bucket = (XrmHashBucket) Xmalloc(sizeof(XrmHashBucketRec));
    bucket->next = NULL;
    bucket->quark = NULLQUARK;
    bucket->type = NULLQUARK;
    bucket->value.addr = NULL;
    bucket->value.size = 0;
    bucket->tables[(int) XrmBindTightly] = NULL;
    bucket->tables[(int) XrmBindLoosely] = NULL;
    return(bucket);
} /* NewDatabase */

static char *getstring(buf, nchars, dp)
	char *buf;
	register int nchars;
	char **dp;
{
	register char *src = *dp;
	register char *dst = buf;
	register char c;

	if (src == NULL) return NULL;
	if (*src == '\0') return NULL;
	while (--nchars > 0) {
		*dst++ = c = *src++;
		if (c == '\n') {
			*dp = src;
			return (buf);
		}
		if (c == '\0') {
			*dp = src-1;
			return (buf);
		}
	}
	*dst = '\0';
	*dp = src;
	return buf;
}

static void Enum(db, bindings, quarks, count, proc, closure)
    XrmHashBucket   db;
    XrmBindingList  bindings;
    XrmQuarkList    quarks;
    unsigned	    count;
    DBEnumProc      proc;
    caddr_t	    closure;
{
    register int	    i;
    register XrmHashBucket  bucket;
    register XrmHashTable   table;

#define EnumTable(binding)						    \
{									    \
    table = db->tables[(int) binding];					    \
    if (table != NULL) {						    \
	bindings[count] = binding;					    \
	quarks[count+1] = NULLQUARK;					    \
	for (i=0; i < HASHSIZE; i++) {					    \
	    bucket = table[i];						    \
	    while (bucket != NULL) {					    \
		quarks[count] = bucket->quark;				    \
	    	Enum(bucket, bindings, quarks, count+1, proc, closure);     \
		bucket = bucket->next;					    \
	    }								    \
	}								    \
    }									    \
} /* EnumTable */

    if (db == NULL) return;
    EnumTable(XrmBindTightly);
    EnumTable(XrmBindLoosely);

    quarks[count] = NULLQUARK;
    if (db->value.addr != NULL) {
	(*proc)(bindings, quarks, db->type, &(db->value), closure);
    }
}

static void EnumerateDatabase(db, proc, closure)
    XrmHashBucket   db;
    caddr_t     closure;
    DBEnumProc      proc;
{
    XrmBinding  bindings[100];
    XrmQuark	quarks[100];
   
    Enum(db, bindings, quarks, (unsigned)0, proc, closure);
}

static void PrintBindingQuarkList(bindings, quarks, stream)
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    FILE		*stream;
{
    Bool	firstNameSeen;

    for (firstNameSeen = False; (*quarks) != NULLQUARK; bindings++, quarks++) {
	if (*bindings == XrmBindLoosely) {
	    (void) fprintf(stream, "*");
	} else if (firstNameSeen) {
	    (void) fprintf(stream, ".");
	}
	firstNameSeen = True;
	(void) fputs(XrmQuarkToString(*quarks), stream);
    }
}

static void DumpEntry(bindings, quarks, type, value, stream_p)
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    XrmRepresentation   type;
    XrmValuePtr		value;
    caddr_t		stream_p;
{

    register unsigned int	i;
    FILE *stream = (FILE*)stream_p;

    PrintBindingQuarkList(bindings, quarks, stream);
    if (type == XrmQString) {
	register char *p, *src = value->addr;
	fputs(":\t", stream);
	if ((p = index(src, '\n')) == NULL) {
	    fputs(src, stream);
	}
	else {
	    do {
		fputs("\\\n", stream);
		fwrite(src, 1, p - src, stream);
		fputs("\\n", stream);
		src = p + 1;
		p = index(src, '\n');
	    } while (p != NULL);
	    if (*src != '\0') {
		fputs("\\\n", stream);
		fputs(src, stream);
	    }
	}
	(void) putc('\n', stream);
    } else {
	(void) fprintf(stream, "!%s:\t", XrmRepresentationToString(type));
	for (i = 0; i < value->size; i++)
	    (void) fprintf(stream, "%02x", (int) value->addr[i]);
        if (index(value->addr, '\n')) {
           (void) fprintf(stream, ":\t\\\n");
           for (i = 0; value->addr[i]; i++) {
               if (value->addr[i] == '\n') {
                   (void) fprintf(stream, "\\n");
                   if (value->addr[i+1]) (void) fprintf(stream, "\\");
                   (void) fprintf(stream, "\n");
               } else {
                   (void) putc(value->addr[i], stream);
               }
           }
        } else {
           (void) fprintf(stream, ":\t%s\n", value->addr);
        }
     }
}

static void Merge(new, old)
    XrmHashBucket   new, old;
{
    register XrmHashTable   newTable, oldTable;
    XrmHashBucket	    oldBucket;
    register XrmHashBucket  newBucket, nextNewBucket, oldSearchBucket;
    int			    binding;
    register int	    i;

    /* Merge data in new into old, and destroy new in the process */
    /* new # NULL && old # NULL */
    
    /* Merge new value into old value */
    if (new->value.addr != NULL) {
	if (old->value.addr != NULL) {
	    XFree(old->value.addr);
	}
	old->type = new->type;
	old->value = new->value;
    } 

    /* Merge new hash tables into old hash tables */
    for (binding = (int) XrmBindTightly;
         binding <= (int) XrmBindLoosely;
	 binding++) {
	oldTable = old->tables[binding];
	newTable = new->tables[binding];
	if (oldTable == NULL) {
	    old->tables[(int) binding] = newTable;
	} else if (newTable != NULL) {
	    /* Copy each bucket over individually */
	    for (i = 0; i < HASHSIZE; i++) {
		oldBucket = oldTable[i];
		newBucket = newTable[i];
		/* Find each item in newBucket list in the oldBucket list */
		while (newBucket != NULL) {
		    nextNewBucket = newBucket->next;
		    oldSearchBucket = oldBucket;
		    while (   oldSearchBucket != NULL
			   && oldSearchBucket->quark != newBucket->quark) {
			oldSearchBucket = oldSearchBucket->next;
		    }
		    if (oldSearchBucket == NULL) {
			/* Just stick newBucket at head of old bucket list */
			newBucket->next = oldTable[i];
			oldTable[i] = newBucket;
		    } else {
			/* Merge the two */
			Merge(newBucket, oldSearchBucket);
		    }
		    newBucket = nextNewBucket;
		} /* while newBucket != NULL */
	    } /* for i */
	    XFree(newTable);
	} /* if */
    } /* for binding */
    XFree(new);
} /* Merge */

Bool XrmQGetSearchList(db, names, classes, searchList, listLength)
    XrmHashBucket   db;
    XrmNameList	    names;
    XrmClassList    classes;
    XrmSearchList   searchList;	/* RETURN */
    int		    listLength;
{
    XrmHashTable    nTight, nLoose;

    numTables = 0;
    lenTables = listLength;
    if (db != NULL) {
	nTight = db->tables[(int) XrmBindTightly];
	nLoose = db->tables[(int) XrmBindLoosely];
	if (nTight != NULL || nLoose != NULL) {
	    if (*names != NULLQUARK) {
		GetTables(nTight, nLoose, names, classes, searchList);
	    } else if (nTight != NULL) {
		if (numTables == lenTables) return False;
		searchList[numTables++] = nTight;
	    }
	    if (nLoose != NULL) {
		if (numTables == lenTables) return False;
		searchList[numTables++] = nLoose;
	    }
	}
    }
    if (numTables == lenTables) return False;
    searchList[numTables] = NULL;
    return True;
} /* XrmGetSearchList */

Bool XrmQGetSearchResource(searchList, name, class, pType, pVal)
    register XrmSearchList	searchList;
    register XrmName		name;
    register XrmClass		class;
    	     XrmRepresentation	*pType; /* RETURN */
    	     XrmValue		*pVal;  /* RETURN */
{
    register XrmHashBucket	bucket;
    register int    nameHash  = HashIndex(name);
    register int    classHash = HashIndex(class);

    for (; (*searchList) != NULL; searchList++) {
	bucket = (*searchList)[nameHash];
	while (bucket != NULL) {
	    if (bucket->quark == name) {
		if (bucket->value.addr != NULL) {
		    /* Leaf node, it really matches */
		    (*pType) = bucket->type;
		    (*pVal) = bucket->value;
		    return True;
		}
		break;
	    }
	    bucket = bucket->next;
    	}
	bucket = (*searchList)[classHash];
	while (bucket != NULL) {
	    if (bucket->quark == class) {
		if (bucket->value.addr != NULL) {
		    /* Leaf node, it really matches */
		    (*pType) = bucket->type;
		    (*pVal) = bucket->value;
		    return True;
		}
		break;
	    }
	    bucket = bucket->next;
    	}
    }
    (*pType) = NULLQUARK;
    (*pVal).addr = NULL;
    (*pVal).size = 0;
    return False;
} /* XrmQGetSearchResource */


void XrmQPutResource(pdb, bindings, quarks, type, value)
    XrmDatabase		*pdb;
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    XrmRepresentation	type;
    XrmValuePtr		value;
{
    if (*pdb == NULL) *pdb = NewDatabase();
    PutEntry(*pdb, bindings, quarks, type, value);
} /* XrmQPutResource */

void XrmPutResource(pdb, specifier, type, value)
    XrmDatabase     *pdb;
    char	    *specifier;
    char	    *type;
    XrmValuePtr	    value;
{
    XrmBinding	    bindings[100];
    XrmQuark	    quarks[100];

    if (*pdb == NULL) *pdb = NewDatabase();
    XrmStringToBindingQuarkList(specifier, bindings, quarks);
    PutEntry(*pdb, bindings, quarks, XrmStringToQuark(type), value);
} /* XrmPutResource */



void XrmQPutStringResource(pdb, bindings, quarks, str)
    XrmDatabase     *pdb;
    XrmBindingList  bindings;
    XrmQuarkList    quarks;
    char	    *str;
{
    XrmValue	value;

    if (*pdb == NULL) *pdb = NewDatabase();
    value.addr = (caddr_t) str;
    value.size = strlen(str)+1;
    PutEntry(*pdb, bindings, quarks, XrmQString, &value);
} /* XrmQPutStringResource */


static void PutLineResources(pdb, get_line, closure)
    XrmDatabase *pdb;
    char	*((*get_line)(/* buf, count, closure */));
    caddr_t     closure;
{
    register char   *s, *ts, ch;
    char	buf[5000];
    char        *pbuf = buf;
    int		pbuf_size = sizeof(buf);
    Bool	have_entire_value;
    register char   *nameStr, *valStr;
    XrmBinding	    bindings[100];
    XrmQuark	    quarks[100];
    XrmValue	    value;

#define CheckForFullBuffer()						   \
    if (s - pbuf == pbuf_size - 1) { /* if line filled buffer */	   \
	char *obuf = pbuf;						   \
	if (pbuf == buf) {						   \
	    int osize = pbuf_size;					   \
	    pbuf = Xmalloc(pbuf_size *= 2);				   \
	    bcopy(buf, pbuf, osize);					   \
	}								   \
	else								   \
	    pbuf = Xrealloc(pbuf, pbuf_size *= 2);			   \
	s = pbuf + (s - obuf);						   \
	ts = pbuf + (ts - obuf);					   \
	nameStr = pbuf + (nameStr - obuf);				   \
	valStr = pbuf + (valStr - obuf);				   \
	if ((char *)(*get_line)(s, pbuf_size - (s-pbuf), closure) != NULL) \
	    have_entire_value = False;					   \
    }


    for (;;) {
	s = (char *)(*get_line)(pbuf, pbuf_size, closure);
	if (s == NULL) break;

	/* Scan to start of resource name/class specification */
	for (; ((ch = *s) != '\n') && isspace(ch); s++) {};
	if ((ch == '\0') || (ch == '\n') || (ch == '#')) continue;
    
	nameStr = valStr = s;
	ts = s - 1;
	do {
	    have_entire_value = True;
	    /* Scan to end of resource name/class specification */
	    for (; ; s++) {
		if ((ch = *s) == '\0' || ch == '\n')
		    break;
		if (ch == ':') {
		    s++;
		    break;
		}
		if (! isspace(ch))
		    ts = s;
	    }
	    CheckForFullBuffer();
	} while (! have_entire_value);
    
	/* Remove trailing white space from name/class */
	ts[1] = '\0';
	
	/* Scan to start of resource value */
	for (; *s != '\n' && isspace(*s); s++) {};
    
	valStr = ts = s;
	do {
	    have_entire_value = True;
	    /* Scan to end of resource value */
	    for (; ((ch = *s) != '\0' && ch != '\n');s++) {
		if (ch == '\\') {
		    if (*++s == 'n') {	    /* \n becomes LF */
			*ts = '\n';
			ts++;
		    } else if (*s == '\n') {  /* \LF means continue next line */
			if ((char *)(*get_line)(s, pbuf_size - (s-pbuf), closure) == NULL)
			    break;
			s--;
		    } else if (*s != '\0') {
			*ts = *s;
			ts++;
		    } else break;
		} else {
		    *ts = ch;
		    ts++;
		}
	    };
	    CheckForFullBuffer();
	} while (! have_entire_value);
	*ts = '\0';
    
	/* Store it in database */
	value.size = s - valStr + 1;
	value.addr = (caddr_t) valStr;
	
	XrmStringToBindingQuarkList(nameStr, bindings, quarks);
	XrmQPutResource(pdb, bindings, quarks, XrmQString, &value);
    } /* for lines left to process */
    if (pbuf != buf) Xfree(pbuf);
#undef CheckForFullBuffer
} /* PutLineResources */
  
void XrmPutStringResource(pdb, specifier, str)
    XrmDatabase *pdb;
    char	*specifier;
    char	*str;
{
    XrmValue	value;
    XrmBinding	bindings[100];
    XrmQuark	quarks[100];

    if (*pdb == NULL) *pdb = NewDatabase();
    XrmStringToBindingQuarkList(specifier, bindings, quarks);
    value.addr = (caddr_t) str;
    value.size = strlen(str)+1;
    PutEntry(*pdb, bindings, quarks, XrmQString, &value);
} /* XrmPutStringResource */


void XrmPutLineResource(pdb, line)
    XrmDatabase     *pdb;
    char   *line;
{
    PutLineResources(pdb, getstring, (caddr_t) &line);
} 

XrmDatabase XrmGetStringDatabase(data)
    char	    *data;
{
    XrmDatabase     db;

    db = NULL;
    PutLineResources(&db,getstring, (caddr_t) &data);
    return db;
}

XrmDatabase XrmGetFileDatabase(fileName)
    char 	    *fileName;
{
    register FILE   *file;
    XrmDatabase     db;

    if (fileName == NULL)
    	return NULL;

    file = fopen(fileName, "r");
    if (file == NULL) {
	return NULL;
    }

    db = NULL;
    PutLineResources(&db, fgets, (caddr_t) file);
    fclose(file);
    return db;
}

void XrmPutFileDatabase(db, fileName)
    XrmDatabase db;
    char 	*fileName;
{
    FILE	*file;
    
    if ((file = fopen(fileName, "w")) == NULL) return;
    EnumerateDatabase(db, DumpEntry, (caddr_t) file);
    fclose(file);
}


void XrmMergeDatabases(new, into)
    XrmDatabase	new, *into;
{
    if (*into == NULL) {
	*into = new;
    } else if (new != NULL) {
	Merge(new, *into);
    }
}

Bool XrmQGetResource(db, names, classes, pType, pValue)
    XrmHashBucket       db;
    XrmNameList		names;
    XrmClassList 	classes;
    XrmRepresentation	*pType;  /* RETURN */
    XrmValuePtr		pValue;  /* RETURN */
{
    XrmHashTable	tight, loose;

    if (db != NULL) {
	tight = db->tables[(int) XrmBindTightly];
	loose = db->tables[(int) XrmBindLoosely];
	if ((tight != NULL || loose != NULL) && (*names != NULL)
		&& GetEntry(tight, loose, names, classes, pType, pValue)) {
	    /* Found it */
	    return True;
	}
    }
    (*pType) = NULLQUARK;
    (*pValue).addr = NULL;
    (*pValue).size = 0;
    return False;
}

Bool XrmGetResource(db, name_str, class_str, pType_str, pValue)
    XrmHashBucket       db;
    XrmString		name_str;
    XrmString		class_str;
    XrmString		*pType_str;  /* RETURN */
    XrmValuePtr		pValue;      /* RETURN */
{
    XrmName		names[100];
    XrmClass		classes[100];
    XrmRepresentation   fromType;
    Bool		result;

    XrmStringToNameList(name_str, names);
    XrmStringToClassList(class_str, classes);
    result = XrmQGetResource(db, names, classes, &fromType, pValue);
    (*pType_str) = XrmQuarkToString(fromType);
    return result;
} /* XrmGetResource */

void XrmInitialize()
{
    XrmQString = XrmStringToQuark("String");
}

