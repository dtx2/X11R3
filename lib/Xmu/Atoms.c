/* $XConsortium: Atoms.c,v 1.4 88/10/07 16:35:09 swick Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * This file contains routines to cache atoms, avoiding multiple
 * server round-trips.
 *
 * Public entry points:
 *
 *	XmuMakeAtom		creates & initializes an opaque AtomRec
 *	XmuInternAtom		fetches an Atom from cache or Display
 *	XmuInternStrings	fetches multiple Atoms as strings
 *	XmuGetAtomName		returns name of an Atom
 *	XmuNameOfAtom		returns name from an AtomPtr
 */

#include <X11/copyright.h>

#define _Atoms_c_
#include "Xmu.h"

typedef struct _DisplayRec {
    struct _DisplayRec* next;
    Display *dpy;
    Atom atom;
} DisplayRec;

struct _AtomRec {
    char *name;
    DisplayRec* head;
};

typedef struct _CacheEntry {
    struct _CacheEntry* next;
    Display *dpy;
    char *name;
} CacheEntry;

static CacheEntry** cache = NULL;
static max_atom = 0;
static Boolean inited = False;

static CacheEntry* CacheLookup(d, atom)
    Display *d;
    Atom atom;
{
    CacheEntry* entry;
    if (atom > max_atom) {
	cache = (CacheEntry**)
	    XtRealloc((char*)cache, ((int)atom+1)*sizeof(CacheEntry*));
	for (; max_atom < atom; max_atom++) cache[(int)max_atom] = NULL;
	cache[(int)max_atom] = NULL;
    }
    entry = cache[(int)atom];
    while (entry != NULL) {
	if (entry->dpy == d) break;
	entry = entry->next;
    }
    return entry;
}


static CacheEntry* CacheEnter(d, atom, name)
    Display *d;
    Atom atom;
    char *name;
{
    CacheEntry* entry = CacheLookup(d, atom);
    if (entry == NULL) {
	entry = XtNew(CacheEntry);
	entry->next = cache[(int)atom];
	entry->dpy = d;
	entry->name = name;
	cache[(int)atom] = entry;
    }
    return entry;
}


static void _DeclareAtoms()
{
    /* note: this list must contain the same elements as Xmu.h */
    static struct _AtomRec
	__XA_TEXT, __XA_TIMESTAMP, __XA_LIST_LENGTH, __XA_LENGTH,
	__XA_TARGETS, __XA_CHARACTER_POSITION, __XA_DELETE, __XA_HOSTNAME,
	__XA_IP_ADDRESS, __XA_DECNET_ADDRESS, __XA_USER, __XA_CLASS,
	__XA_NAME, __XA_CLIENT_WINDOW, __XA_ATOM_PAIR, __XA_SPAN,
	__XA_NET_ADDRESS, __XA_NULL, __XA_FILENAME, __XA_OWNER_OS,
	__XA_CLIPBOARD;

#if defined(__STDC__) && !defined(UNIXCPP)
#define DeclareAtom(atom, text) \
    _##atom = &__##atom; __##atom.name = text; __##atom.head = NULL;
#else
#define DeclareAtom(atom, text) \
    _/**/atom = &__/**/atom; __/**/atom.name = text; __/**/atom.head = NULL;
#endif

    DeclareAtom(XA_TEXT,		"TEXT"			)
    DeclareAtom(XA_TIMESTAMP,		"TIMESTAMP"		)
    DeclareAtom(XA_LIST_LENGTH,		"LIST_LENGTH"		)
    DeclareAtom(XA_LENGTH,		"LENGTH"		)
    DeclareAtom(XA_TARGETS,		"TARGETS"		)
    DeclareAtom(XA_CHARACTER_POSITION,	"CHARACTER_POSITION"	)
    DeclareAtom(XA_DELETE,		"DELETE"		)
    DeclareAtom(XA_HOSTNAME,		"HOSTNAME"		)
    DeclareAtom(XA_IP_ADDRESS,		"IP_ADDRESS"		)
    DeclareAtom(XA_DECNET_ADDRESS,	"DECNET_ADDRESS"	)
    DeclareAtom(XA_USER,		"USER"			)
    DeclareAtom(XA_CLASS,		"CLASS"			)
    DeclareAtom(XA_NAME,		"NAME"			)
    DeclareAtom(XA_CLIENT_WINDOW,	"CLIENT_WINDOW"		)
    DeclareAtom(XA_ATOM_PAIR,		"ATOM_PAIR"		)
    DeclareAtom(XA_SPAN,		"SPAN"			)
    DeclareAtom(XA_NET_ADDRESS,		"NET_ADDRESS"		)
    DeclareAtom(XA_NULL,		"NULL"			)
    DeclareAtom(XA_FILENAME,		"FILENAME"		)
    DeclareAtom(XA_OWNER_OS,		"OWNER_OS"		)
    DeclareAtom(XA_CLIPBOARD,		"CLIPBOARD"		)

#undef DeclareAtom
}



/******************************************************************

  Public procedures

 ******************************************************************/


AtomPtr XmuMakeAtom(name)
    char* name;
{
    AtomPtr ptr = XtNew(struct _AtomRec);
    ptr->name = name;
    ptr->head = NULL;
    return ptr;
}

char* XmuNameOfAtom(atom_ptr)
    AtomPtr atom_ptr;
{
    return atom_ptr->name;
}


Atom XmuInternAtom(d, atom_ptr)
    Display *d;
    AtomPtr atom_ptr;
{
    DisplayRec* display_rec;
    if (!inited) {
	_DeclareAtoms();
	inited = True;
    }
    if (atom_ptr == NULL)
	XtErrorMsg( "uninitializedAtom", "xmuInternAtom", "XmuError",
		    "AtomPtr was not initialized", NULL, 0 );
    for (display_rec = atom_ptr->head; display_rec != NULL;
	 display_rec = display_rec->next) {
	if (display_rec->dpy == d)
	    return display_rec->atom;
    }
    display_rec = XtNew(DisplayRec);
    display_rec->next = atom_ptr->head;
    atom_ptr->head = display_rec;
    display_rec->dpy = d;
    display_rec->atom = XInternAtom(d, atom_ptr->name, False);
    CacheEnter(d, display_rec->atom, atom_ptr->name);
    return display_rec->atom;
}


char *XmuGetAtomName(d, atom)
    Display *d;
    Atom atom;
{
    CacheEntry* entry;
    if (atom == 0) return "(BadAtom)";
    entry = CacheLookup(d, atom);
    if (entry == NULL)
	entry = CacheEnter(d, atom, XGetAtomName(d, atom));
    return entry->name;
}

/* convert (names, count) to a list of atoms. Caller allocates list */
void XmuInternStrings(d, names, count, atoms)
    Display *d;
    String *names;
    Cardinal count;
    Atom *atoms;		/* return */
{
    static AtomPtr* cache = NULL;
    static Cardinal cache_size = 0, last = 0;
    int i;

    if (count > cache_size) {
	cache_size += count;
	cache = (AtomPtr*)XtRealloc(cache, cache_size*sizeof(AtomPtr));
    }
    for (i = 0; i < count; i++) {
	register int p;
	for (p = 0; p < last; p++) {
	    if (strcmp(names[i], XmuNameOfAtom(cache[p])) == 0) {
		break;
	    }
	}
	if (p == last) {
	    if (last == cache_size) {
		cache = (AtomPtr*)XtRealloc(cache, (++cache_size)*sizeof(AtomPtr));
	    }
	    cache[last++] = XmuMakeAtom(names[i]);
	}
	atoms[i] = XmuInternAtom(d, cache[p]);
    }
    return;
}
