#ifndef lint
static char Xrcsid[] = "$XConsortium: CompObj.c,v 1.5 88/09/06 16:26:59 jim Exp $";
/* $oHeader: CompObj.c,v 1.2 88/08/18 15:35:21 asente Exp $ */
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

#define COMPOSITEOBJ
#include "IntrinsicI.h"
#include "StringDefs.h"

externaldef(compositeobjectclassrec) 
                   CompositeObjectClassRec compositeObjectClassRec = {
  { /******* CorePart *******/
    /* superclass	    */	(WidgetClass)&rectObjClassRec,
    /* class_name	    */	"Composite",
    /* widget_size	    */	sizeof(CompositeRec),
    /* class_initialize     */  NULL,
    /* class_part_initialize*/	CompositeClassPartInitialize,
    /* class_inited	    */	FALSE,
    /* initialize	    */	CompositeInitialize,
    /* initialize_hook      */	NULL,		
    /* realize		    */	XtInheritRealize,
    /* actions		    */	NULL,
    /* num_actions	    */	0,
    /* resources	    */	NULL,
    /* num_resources	    */	0,
    /* xrm_class	    */	NULLQUARK,
    /* compress_motion      */	FALSE,
    /* compress_exposure    */	TRUE,
    /* compress_enterleave  */  FALSE,
    /* visible_interest     */	FALSE,
    /* destroy		    */	CompositeDestroy,
    /* resize		    */	NULL,
    /* expose		    */	NULL,
    /* set_values	    */	NULL,
    /* set_values_hook      */	NULL,			
    /* set_values_almost    */	XtInheritSetValuesAlmost,  
    /* get_values_hook      */	NULL,			
    /* accept_focus	    */	NULL,
    /* version		    */	XtVersion,
    /* callback_offsets     */  NULL,
    /* tm_table		    */  NULL,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */	NULL,
    /* extension	    */  NULL
  },
  { /**** CompositePart *****/
    /* geometry_handler     */  NULL,
    /* change_managed       */  NULL,
    /* insert_child	    */  CompositeInsertChild,
    /* delete_child	    */  CompositeDeleteChild,
    /* extension	    */  NULL
    }
};

externaldef(compositeobjectclass) ObjectClass compositeObjectClass =
 (ObjectClass) &compositeObjectClassRec;

