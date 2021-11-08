/*
 * $XConsortium: Selection.h,v 1.8 88/09/06 16:28:56 jim Exp $
 * $oHeader: Selection.h,v 1.4 88/09/01 17:14:14 asente Exp $
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

#ifndef _Selection_h
#define _Selection_h

#define XT_CONVERT_FAIL (Atom)0x80000001

/*
 * Routine to get the value of a selection as a given type.  Returns
 * TRUE if it successfully got the value as requested, FALSE otherwise.  
 * selection is the atom describing the type of selection (e.g. 
 * primary or secondary). value is set to the pointer of the converted 
 * value, with length elements of data, each of size indicated by format.
 * (This pointer will be freed using XtFree when the selection has
 *  been delivered to the requestor.)  target is
 * the type that the conversion should use if possible; type is returned as
 * the actual type returned.  Format should be either 8, 16, or 32, and
 * specifies the word size of the selection, so that Xlib and the server can
 * convert it between different machine types. */

typedef Boolean (*XtConvertSelectionProc)(); /* widget, selection, target,
					      type, value, length, format */
    /* Widget widget; */
    /* Atom *selection; */
    /* Atom *target; */
    /* Atom *type; */	     /* RETURN */
    /* caddr_t *value; */    /* RETURN */
    /* unsigned long *length; */       /* RETURN */
    /* int *format; */	     /* RETURN */

/*
 * Routine to inform a widget that it no longer owns the given selection.
 */

typedef void (*XtLoseSelectionProc)(); /* widget, selection */
    /* Widget widget; */
    /* Atom *selection; */


/*
 * Routine to inform the selection owner when a selection requestor
 * has successfully retrieved the selection value.
 */

typedef void (*XtSelectionDoneProc)(); /* widget, selection, target */
    /* Widget widget; */
    /* Atom *selection; */
    /* Atom *target; */


/*
 * Routine to call back when a requested value has been obtained for a
 *  selection.
 */

typedef void (*XtSelectionCallbackProc)(); /* widget, closure, selection,
					    type, value, length, format,  */
    /* Widget widget; */
    /* caddr_t closure; */
    /* Atom *selection; */
    /* Atom *type; */
    /* caddr_t value; */
    /* unsigned long *length; */
    /* int *format; */
    

/*
 * Set the given widget to own the selection.  The convertProc should
 * be called when someone wants the current value of the selection. If it
 * is not NULL, the
 * losesSelection gets called whenever the window no longer owns the selection
 * (because someone else took it). If it is not NULL, the doneProc gets
 * called when the widget has provided the current value of the selection
 * to a requestor and the requestor has indicated that it has succeeded
 * in reading it by deleting the property.
 */

extern Boolean XtOwnSelection(); /* widget, selection, time, convertProc, losesSelection, doneProc */
    /* Widget widget; */
    /* Atom selection; */
    /* Time time; */
    /* XtConvertSelectionProc convertProc; */
    /* XtLosesSelectionProc losesSelection; */
    /* XtSelectionDoneProc doneProc; */

/*
 * The given widget no longer wants the selection.  If it still owns it, then
 * the selection owner is cleared, and the window's losesSelection is called.
 */

extern void XtDisownSelection(); /* widget, selection, time */
    /* Widget widget; */
    /* Atom selection; */
    /* Time time; */

/*
 * Get the value of the given selection.  
 */

extern void XtGetSelectionValue(); /* widget, selection, target,
				      callback, closure, time */
    /* Widget widget; */
    /* Atom selection; */
    /* Atom target; */
    /* XtSelectionCallbackProc callback; */
    /* caddr_t closure; */
    /* Time time; */

extern void XtGetSelectionValues(); /* widget, selection, targets, count, 
			callback, closures, time */
    /* Widget widget; */
    /* Atom selection; */
    /* Atom *targets; */
    /* int count; */
    /* XtSelectionCallbackProc callback; */
    /* Opaque *closures; */
    /* Time time; */


/* Set the selection timeout value, in units of milliseconds */

extern void XtAppSetSelectionTimeout(); /* app, timeout */
	/* XtAppContext app; */
	/* unsigned long timeout; */

extern void XtSetSelectionTimeout(); /* timeout */
	/* unsigned long timeout; */

 /* Return the selection timeout value, in units of milliseconds */

extern unsigned int XtAppGetSelectionTimeout(); /* app */
	/* XtAppContext app; */

extern unsigned int XtGetSelectionTimeout();

#endif  _Selection_h
