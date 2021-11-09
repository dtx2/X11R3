/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/* $XConsortium: utils.c,v 1.66 88/10/22 13:28:24 keith Exp $ */
#include <stdio.h>
#include "Xos.h"
#include "misc.h"
#include "X.h"
#include "input.h"
#include "opaque.h"
extern char *display;

extern int defaultBackingStore;
extern Bool disableBackingStore;
extern Bool disableSaveUnders;

Bool clientsDoomed = FALSE;
Bool GivingUp = FALSE;
void ddxGiveUp();

extern char *sbrk();

pointer minfree = NULL;

char *dev_tty_from_init = NULL;		/* since we need to parse it anyway */

/* Force connections to close on SIGHUP from init */

AutoResetServer () {
    clientsDoomed = TRUE;
}
// Force connections to close and then exit on SIGTERM, SIGINT
GiveUp() {
    GivingUp = TRUE;
    ddxGiveUp();
    exit(0);
}
void ErrorF(char *f, char *s0, char *s1, char *s2) {
	fprintf(stderr, f, s0, s1, s2);
}
static void AbortServer() {
    extern void AbortDDX();
    AbortDDX();
    fflush(stderr);
    abort();
}
void Error(char *str) {
    perror(str);
}
// This is private to the OS layer.
void Notice() {}
void FatalError(char *f, char *s0, char *s1, char *s2) {
    ErrorF("\nFatal server bug!\n");
    ErrorF(f, s0, s1, s2);
    ErrorF("\n");
    AbortServer();
}
long GetTimeInMillis() {
    struct timeval  tp;
    gettimeofday(&tp, 0);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
// XALLOC - X's internal memory allocator. Why does it return unsigned int * instead of the more common char *?
// Well, if you read K&R you'll see they say that alloc must return a pointer "suitable for conversion" to whatever
// type you really want. In a full-blown generic allocator there's no way to solve the alignment problems without
// potentially wasting lots of space. But we have a more limited problem. We know we're only ever returning pointers
// to structures which will have to be long word aligned. So we are making a stronger guarantee. It might have made
// sense to make Xalloc return char * to conform with people's expectations of malloc, but this makes lint happier.
unsigned long *Xalloc(unsigned long amount) {
    char *malloc();
    register pointer  ptr;
    if (amount == 0) { return( (unsigned long *)NULL); }
    amount = (amount + 3) & ~3; // aligned extra on long word boundary
    if (ptr = (pointer)malloc(amount)) { return ((unsigned long *)ptr); }
    FatalError("Out of memory in Xalloc\n");
}

// Xrealloc - realloc wrap-around, to take care of the "no memory" case, since it would be difficult in many places to
// "back out" on failure.
unsigned long *Xrealloc (pointer ptr, unsigned long amount) {
    char *malloc();
    char *realloc();
    amount = (amount + 3) & ~3;
    if (ptr) {
        if (ptr < minfree) {
            ErrorF("Xrealloc: trying to free static storage\n");
            /* Force a core dump */
            AbortServer();
        }
        ptr = (pointer) realloc (ptr, amount);
    } else {
	    ptr =  (pointer) malloc (amount);
    }
    if (ptr || !amount) { return((unsigned long *)ptr); }
    FatalError ("Out of memory in Xrealloc\n");
}
// Xfree - calls free
void Xfree(pointer ptr) {
    if (ptr == (pointer) NULL) { return; }
    if (ptr >= minfree) { free(ptr); }
}
