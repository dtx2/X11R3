#include "copyright.h"

/* $XConsortium: XKeyBind.c,v 11.44 88/10/09 12:18:19 rws Exp $ */
/* Copyright 1985, 1987, Massachusetts Institute of Technology */

/* Beware, here be monsters (still under construction... - JG */

#define NEED_EVENTS
#include "Xlib.h"
#include "Xlibint.h"
#include "Xutil.h"
#include "keysym.h"
#include <stdio.h>

static ComputeMaskFromKeytrans();

struct XKeytrans {
	struct XKeytrans *next;/* next on list */
	char *string;		/* string to return when the time comes */
	int len;		/* length of string (since NULL is legit)*/
	KeySym key;		/* keysym rebound */
	unsigned int state;	/* modifier state */
	KeySym *modifiers;	/* modifier keysyms you want */
	int mlen;		/* length of modifier list */
};

static KeySym KeyCodetoKeySym(dpy, keycode, col)
     register Display *dpy;
     KeyCode keycode;
     int col;
{
     int ind;
     /*
      * if keycode not defined in set, this should really be impossible.
      * in any case, if sanity check fails, return NoSymbol.
      */
     if (col < 0 || col > dpy->keysyms_per_keycode) return (NoSymbol);
     if (keycode < dpy->min_keycode || keycode > dpy->max_keycode) 
       return(NoSymbol);

     ind = (keycode - dpy->min_keycode) * dpy->keysyms_per_keycode + col;
     return (dpy->keysyms[ind]);
}

KeySym XKeycodeToKeysym(dpy, kc, col)
    Display *dpy;
    KeyCode kc;
    int col;
{
     if (dpy->keysyms == NULL)
         Initialize(dpy);
     return (KeyCodetoKeySym(dpy, kc, col));
}

KeyCode XKeysymToKeycode(dpy, ks)
    Display *dpy;
    KeySym ks;
{
    int         i;

     if (dpy->keysyms == NULL)
         Initialize(dpy);
    for (i = dpy->min_keycode; i <= dpy->max_keycode; i++) {
	int         j;

	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    int ind = (i - dpy->min_keycode) * dpy->keysyms_per_keycode + j;

	    if (ks == dpy->keysyms[ind])
		return (i);
	}
    }
    return (0);
}

KeySym XLookupKeysym(event, col)
     register XKeyEvent *event;
     int col;
{
     if (event->display->keysyms == NULL)
         Initialize(event->display);
     return (KeyCodetoKeySym(event->display, event->keycode, col));
}

static
InitModMap(dpy)
    Display *dpy;
{
    register XModifierKeymap *map;
    register int i, j;
    KeySym sym;
    register struct XKeytrans *p;

    dpy->modifiermap = map = XGetModifierMapping(dpy);
    if (dpy->keysyms == NULL)
	Initialize(dpy);
    LockDisplay(dpy);
    /* If any Lock key contains Caps_Lock, then interpret as Caps_Lock,
     * else if any contains Shift_Lock, then interpret as Shift_Lock,
     * else ignore Lock altogether.
     */
    dpy->lock_meaning = NoSymbol;
    /* Lock modifiers are in the second row of the matrix */
    for (i = map->max_keypermod; i < 2*map->max_keypermod; i++) {
        for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    sym = KeyCodetoKeySym(dpy, map->modifiermap[i], j);
	    if (sym == XK_Caps_Lock) {
		dpy->lock_meaning = XK_Caps_Lock;
		break;
	    } else if (sym == XK_Shift_Lock) {
		dpy->lock_meaning = XK_Shift_Lock;
	    }
	}
    }
    for (p = dpy->key_bindings; p; p = p->next)
        ComputeMaskFromKeytrans(dpy, p);
    UnlockDisplay(dpy);
}

XRefreshKeyboardMapping(event)
     register XMappingEvent *event;
{
     extern void XFreeModifiermap();

     if(event->request == MappingKeyboard) {
	/* XXX should really only refresh what is necessary
	 * for now, make initialize test fail
	 */
	    LockDisplay(event->display);
	    if (event->display->keysyms != NULL) {
	         Xfree ((char *)event->display->keysyms);
	         event->display->keysyms = NULL;
	    }
	    UnlockDisplay(event->display);
     }
     if(event->request == MappingModifier) {
	    LockDisplay(event->display);
	    if (event->display->modifiermap != NULL) {
		XFreeModifiermap(event->display->modifiermap);
		event->display->modifiermap = NULL;
	    }
	    UnlockDisplay(event->display);
	    /* go ahead and get it now, since initialize test may not fail */
	    InitModMap(event->display);
     }
}

/*ARGSUSED*/
static InitTranslationList(dpy)
    Display *dpy;
{
	/* not yet implemented */
	/* should read keymap file and initialize list */
}

/*ARGSUSED*/
int XUseKeymap(filename) 
    char *filename;
{
  /* not yet implemented */
}

static Initialize(dpy)
Display *dpy;
{
    register KeySym *keysyms, *sym, *old, *endp, *temp;
    int per, n;

    /* 
     * lets go get the keysyms from the server.
     */
    if (dpy->keysyms == NULL) {
	n = dpy->max_keycode - dpy->min_keycode + 1;
	keysyms = XGetKeyboardMapping (dpy, (KeyCode) dpy->min_keycode,
				       n, &per);
	/* need at least two per keycode, to have room for case conversion */
	if (per == 1) {
	    temp = (KeySym *) Xmalloc((unsigned)(n * sizeof(KeySym) * 2));
	    for (sym = temp, old = keysyms, endp = keysyms + n; old < endp;) {
		*sym++ = *old++;
	        *sym++ = NoSymbol;
	    }
	    Xfree((char *)keysyms);
	    keysyms = temp;
	    per = 2;
	    n <<= 1;
	}
	/* convert singleton Latin 1 alphabetics */
	for (sym = keysyms, endp = keysyms + n*per; sym < endp; sym += per) {
	  if (*(sym + 1) != NoSymbol) continue;
	  if ((*sym >= XK_A) && (*sym <= XK_Z)) {
	      *(sym + 1) = *sym;
	      *sym += (XK_a - XK_A);
	      }
	  else if ((*sym >= XK_a) && (*sym <= XK_z)) {
	      *(sym + 1) = *sym - (XK_a - XK_A);
	      }
	  else if ((*sym >= XK_Agrave) && (*sym <= XK_Odiaeresis)) {
	      *(sym + 1) = *sym;
	      *sym += (XK_agrave - XK_Agrave);
	      }
	  else if ((*sym >= XK_agrave) && (*sym <= XK_odiaeresis)) {
	      *(sym + 1) = *sym - (XK_agrave - XK_Agrave);
	      }
	  else if ((*sym >= XK_Ooblique) && (*sym <= XK_Thorn)) {
	      *(sym + 1) = *sym;
	      *sym += (XK_oslash - XK_Ooblique);
	      }
	  else if ((*sym >= XK_oslash) && (*sym <= XK_thorn)) {
	      *(sym + 1) = *sym - (XK_oslash - XK_Ooblique);
	      }
 	}
	LockDisplay(dpy);
	dpy->keysyms = keysyms;
	dpy->keysyms_per_keycode = per;
	UnlockDisplay(dpy);
    }
    if (dpy->modifiermap == NULL) {
        InitModMap(dpy);
    }
    if (dpy->key_bindings == NULL) InitTranslationList(dpy);
}

static int KeySymRebound(event, buf, symbol)
    register XKeyEvent *event;
    char *buf;
    KeySym symbol;
{
    register struct XKeytrans *p;

    for (p = event->display->key_bindings; p; p = p->next) {
	if ((event->state == p->state) && (symbol == p->key)) {
		bcopy (p->string, buf, p->len);
		return p->len;
		}
	}
    return -1;
}
  
/*ARGSUSED*/
int XLookupString (event, buffer, nbytes, keysym, status)
     register XKeyEvent *event;
     char *buffer;	/* buffer */
     int nbytes;	/* space in buffer for characters */
     KeySym *keysym;
     XComposeStatus *status;	/* not implemented */
{
     register Display *dpy;
     register KeySym symbol, usymbol;
     int length = 0;
     char buf[BUFSIZ];
     unsigned char c;
     unsigned long hiBytes;

     dpy = event->display;
     if (dpy->keysyms == NULL)
         Initialize(dpy);

     symbol = KeyCodetoKeySym(dpy, event->keycode, 0);

     /* apply Shift first */
     if ((event->state & ShiftMask) ||
	 ((event->state & LockMask) && (dpy->lock_meaning == XK_Shift_Lock))) {
	     usymbol = KeyCodetoKeySym(dpy, event->keycode, 1);
	     if (usymbol != NoSymbol)
	          symbol = usymbol;
     }
     /* then apply Caps, as protocol suggests*/
     /* XXX this should really work for all character sets */
     if ((event->state & LockMask) && (dpy->lock_meaning == XK_Caps_Lock)) {
	    if (symbol >= XK_a && symbol <= XK_z)
	          symbol -= (XK_a - XK_A);
	    else if (symbol >= XK_agrave && symbol <= XK_odiaeresis)
	          symbol -= (XK_agrave - XK_Agrave);
	    else if (symbol >= XK_oslash && symbol <= XK_thorn)
	          symbol -= (XK_oslash - XK_Ooblique);
     }

     if (keysym != NULL) *keysym = symbol;

     /*
      * see if symbol rebound, if so, return that string.
      * if special Keyboard, convert to ascii.  handle control.
      */
     if ((length = KeySymRebound(event, buf, symbol)) == -1) {
	    c = symbol & 0xFF;
	    hiBytes = symbol >> 8;
            if ( ((hiBytes != 0) && (hiBytes != 0xFF))
		|| IsModifierKey(symbol)   || IsCursorKey(symbol)
		|| IsPFKey (symbol)      || IsFunctionKey(symbol)
		|| IsMiscFunctionKey(symbol)
		|| (symbol == XK_Multi_key) || (symbol == XK_Kanji)) {
		return 0;
	    } else {
		if (symbol == XK_KP_Space)
		   c = XK_space & 0xFF; /* patch encoding botch */
		/* if X keysym, convert to ascii by grabbing low 7 bits */
		if (hiBytes == 0xFF) c &= 0x7F;
		/* only apply Control key if it makes sense, else ignore it */
		if (event->state & ControlMask) {
		    if ((c >= '@' && c <= '\177') || c == ' ') c &= 0x1F;
		    else if (c == '2') c = '\000';
		    else if (c >= '3' && c <= '7') c -= ('3' - '\033');
		    else if (c == '8') c = '\177';
		    else if (c == '/') c = '_' & 0x1F;
		}
	    }
	    buf[0] = c;
     	    length = 1;
      }
      if (length > nbytes) length = nbytes;
      bcopy (buf, buffer, length);
      return (length);
}

XRebindKeysym (dpy, keysym, mlist, nm, str, nbytes)
    Display *dpy;
    KeySym keysym;
    KeySym *mlist;
    int nm;		/* number of modifiers in mlist */
    unsigned char *str;
    int nbytes;
{
    register struct XKeytrans *tmp, *p;
    int nb;

    if (dpy->keysyms == NULL)
    	Initialize(dpy);
    LockDisplay(dpy);
    tmp = dpy->key_bindings;
    dpy->key_bindings = p = (struct XKeytrans *)Xmalloc(sizeof(struct XKeytrans));
    p->next = tmp;	/* chain onto list */
    p->string = (char *) Xmalloc(nbytes);
    bcopy (str, p->string, nbytes);
    p->len = nbytes;
    nb = sizeof (KeySym) * nm;
    p->modifiers = (KeySym *) Xmalloc(nb);
    bcopy (mlist, p->modifiers, nb);
    p->key = keysym;
    p->mlen = nm;
    ComputeMaskFromKeytrans(dpy, p);
    UnlockDisplay(dpy);
    return;
}

_XFreeKeyBindings (dpy)
    Display *dpy;
{
    register struct XKeytrans *p, *np;

    for (p = dpy->key_bindings; p; p = np) {
        np = p->next;
	Xfree(p->string);
	Xfree((char *)p->modifiers);
	Xfree((char *)p);
    }   
}

/*
 * given a KeySym, returns the first keycode containing it, if any.
 */
static CARD8 FindKeyCode(dpy, code)
    register Display *dpy;
    register KeySym code;
{

    register KeySym *kmax = dpy->keysyms + 
	(dpy->max_keycode - dpy->min_keycode + 1) * dpy->keysyms_per_keycode;
    register KeySym *k = dpy->keysyms;
    while (k < kmax) {
	if (*k == code)
	    return(((k - dpy->keysyms)
		/ dpy->keysyms_per_keycode) + dpy->min_keycode);
	k += 1;
	}
    return 0;
}

	
/*
 * given a list of modifiers, computes the mask necessary for later matching.
 * This routine must lookup the key in the Keymap and then search to see
 * what modifier it is bound to, if any.
 */
static ComputeMaskFromKeytrans(dpy, p)
    Display *dpy;
    register struct XKeytrans *p;
{
    register int i;
    register CARD8 code;
    register XModifierKeymap *m = dpy->modifiermap;

    p->state = 0;
    for (i = 0; i < p->mlen; i++) {
	/* if not found, then not on current keyboard */
	if ((code = FindKeyCode(dpy, p->modifiers[i])) == 0)
		continue;
	/* code is now the keycode for the modifier you want */
	{
	    register int j;

	    for (j = 0; j < (m->max_keypermod<<3); j++) {
		if (code == m->modifiermap[j])
		    p->state |= (1<<(j/m->max_keypermod));
	    }
	}
    }
    return;
}
