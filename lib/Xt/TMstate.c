#ifndef lint
static char Xrcsid[] = "$XConsortium: TMstate.c,v 1.65 89/01/20 08:05:17 swick Exp $";
/* $oHeader: TMstate.c,v 1.5 88/09/01 17:17:29 asente Exp $ */
#endif lint
/*LINTLIBRARY*/

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

/* TMstate.c -- maintains the state table of actions for the translation 
 *              manager.
 */
#define XK_LATIN1
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include "StringDefs.h"
#include <stdio.h>
#include "IntrinsicI.h"


#define StringToAction(string)	((XtAction) StringToQuark(string))

#define STR_THRESHOLD 25
#define STR_INCAMOUNT 100
#define CHECK_STR_OVERFLOW \
    if (str - *buf > *len - STR_THRESHOLD) {		\
	String old = *buf;				\
	*buf = XtRealloc(old, *len += STR_INCAMOUNT);	\
	str = str - old + *buf;				\
    }

#define ExpandToFit(more) \
    if (str - *buf > *len - STR_THRESHOLD - strlen(more)) { 		\
	String old = *buf;						\
	*buf = XtRealloc(old, *len += STR_INCAMOUNT + strlen(more));	\
	str = str - old + *buf;						\
    }


static void FreeActions(action)
  register ActionPtr action;
{
    while (action != NULL) {
	register ActionPtr next = action->next;

	if (action->params != NULL) {
	    register Cardinal i;

	    for (i=0; i<action->num_params; i++) XtFree(action->params[i]);
	    XtFree((char *)action->params);
	}

	XtFree(action->token);
	XtFree((char *)action);
	action = next;
    }
}

static String PrintModifiers(buf, len, str, mask, mod)
    String *buf;
    int *len;
    String str;
    unsigned long mask, mod;
{
    Boolean notfirst = False;
    CHECK_STR_OVERFLOW;

#if defined(__STDC__) && !defined(UNIXCPP)
#define MASKNAME(modname) modname##Mask
#else
#define MASKNAME(modname) modname/**/Mask
#endif

#define PRINTMOD(modname) \
    if (mask & MASKNAME(modname)) {	 \
	if (! (mod & MASKNAME(modname))) \
	    *str++ = '~';		 \
	else if (notfirst)		 \
	    *str++ = ' ';		 \
	*str = '\0';			 \
	strcat(str, "modname");		 \
	str += strlen(str);		 \
	notfirst = True;		 \
    }

#define CtrlMask ControlMask

    PRINTMOD(Shift);
    PRINTMOD(Ctrl);
    PRINTMOD(Lock);
    PRINTMOD(Mod1);
    PRINTMOD(Mod2);
    PRINTMOD(Mod3);
    PRINTMOD(Mod4);
    PRINTMOD(Mod5);
    PRINTMOD(Button1);
    PRINTMOD(Button2);
    PRINTMOD(Button3);
    PRINTMOD(Button4);
    PRINTMOD(Button5);

#undef MASKNAME
#undef PRINTMOD
#undef CtrlMask

    return str;
}

static String PrintEventType(buf, len, str, event)
    String *buf;
    int *len;
    register String str;
    unsigned long event;
{
    CHECK_STR_OVERFLOW;
    switch (event) {
#define PRINTEVENT(event) case event: (void) sprintf(str, "<event>"); break;
	PRINTEVENT(KeyPress)
	PRINTEVENT(KeyRelease)
	PRINTEVENT(ButtonPress)
	PRINTEVENT(ButtonRelease)
	PRINTEVENT(MotionNotify)
	PRINTEVENT(EnterNotify)
	PRINTEVENT(LeaveNotify)
	PRINTEVENT(FocusIn)
	PRINTEVENT(FocusOut)
	PRINTEVENT(KeymapNotify)
	PRINTEVENT(Expose)
	PRINTEVENT(GraphicsExpose)
	PRINTEVENT(NoExpose)
	PRINTEVENT(VisibilityNotify)
	PRINTEVENT(CreateNotify)
	PRINTEVENT(DestroyNotify)
	PRINTEVENT(UnmapNotify)
	PRINTEVENT(MapNotify)
	PRINTEVENT(MapRequest)
	PRINTEVENT(ReparentNotify)
	PRINTEVENT(ConfigureNotify)
	PRINTEVENT(ConfigureRequest)
	PRINTEVENT(GravityNotify)
	PRINTEVENT(ResizeRequest)
	PRINTEVENT(CirculateNotify)
	PRINTEVENT(CirculateRequest)
	PRINTEVENT(PropertyNotify)
	PRINTEVENT(SelectionClear)
	PRINTEVENT(SelectionRequest)
	PRINTEVENT(SelectionNotify)
	PRINTEVENT(ColormapNotify)
	PRINTEVENT(ClientMessage)
	case _XtEventTimerEventType: (void) sprintf(str,"<EventTimer>"); break;
	case _XtTimerEventType: (void) sprintf(str, "<Timer>"); break;
	default: (void) sprintf(str, "<0x%x>", (int) event);
#undef PRINTEVENT
    }
    str += strlen(str);
    return str;
}

static String PrintCode(buf, len, str, mask, code)
    String *buf;
    int *len;
    register String str;
    unsigned long mask, code;
{
    CHECK_STR_OVERFLOW;
    if (mask != 0) {
	if (mask != (unsigned long)~0L)
	    (void) sprintf(str, "0x%lx:0x%lx", mask, code);
	else (void) sprintf(str, "0x%lx", code);
	str += strlen(str);
    }
    return str;
}

static String PrintLateModifiers(buf, len, str, lateModifiers)
    String *buf;
    int *len;
    register String str;
    LateBindingsPtr lateModifiers;
{
    for (; lateModifiers->keysym != NULL; lateModifiers++) {
	CHECK_STR_OVERFLOW;
	if (lateModifiers->knot) {
	    *str++ = '~';
	    *str = '\0';
	}
	strcat(str, XKeysymToString(lateModifiers->keysym));
	str += strlen(str);
	if (lateModifiers->pair) {
	    *(str -= 2) = '\0';	/* strip "_L" */
	    lateModifiers++;	/* skip _R keysym */
	}
    }
    return str;
}

static String PrintEvent(buf, len, str, event)
    String *buf;
    int *len;
    register String str;
    register Event *event;
{
    str = PrintModifiers(buf, len, str, event->modifierMask, event->modifiers);
    if (event->lateModifiers != NULL)
	str = PrintLateModifiers(buf, len, str, event->lateModifiers);
    str = PrintEventType(buf, len, str, event->eventType);
    str = PrintCode(buf, len, str, event->eventCodeMask, event->eventCode);
    return str;
}

static String PrintParams(buf, len, str, params, num_params)
    String *buf;
    int *len;
    register String str, *params;
    Cardinal num_params;
{
    register Cardinal i;
    for (i = 0; i<num_params; i++) {
	ExpandToFit( params[i] );
	if (i != 0) (void) sprintf(str, ", ");
	str += strlen(str);
	(void) sprintf(str, "\"%s\"", params[i]);
	str += strlen(str);
    }
    return str;
}

static String PrintActions(buf, len, str, actions, quarkTable)
    String *buf;
    int *len;
    register String str;
    register ActionPtr actions;
    XrmQuark* quarkTable;
{
    while (actions != NULL /* && actions->token != NULL */) {
	String proc = XrmQuarkToString(quarkTable[actions->index]);
	ExpandToFit( proc );
        (void) sprintf(str, " %s(", proc);
	str += strlen(str);
	str = PrintParams(buf, len, str, actions->params, (Cardinal)actions->num_params);
	str += strlen(str);
	(void) sprintf(str, ")");
	str += strlen(str);
	actions = actions->next;
    }
    return str;
}

static Boolean ComputeLateBindings(event,eventSeq,computed,computedMask)
    Event *event;
    TMEventPtr eventSeq;
    Modifiers *computed,*computedMask;
{
    int i,j,ref;
    ModToKeysymTable* temp;
    XtPerDisplay perDisplay;
    Display *dpy;
    Boolean found;
    KeySym tempKeysym = NoSymbol;
    dpy = eventSeq->dpy;
    perDisplay = _XtGetPerDisplay(dpy);
    if (perDisplay == NULL) {
        XtAppWarningMsg(_XtDisplayToApplicationContext(dpy),
		"displayError","invalidDisplay","XtToolkitError",
            "Can't find display structure",
            (String *)NULL, (Cardinal *)NULL);
         return FALSE;
    }
    if (perDisplay->modsToKeysyms == NULL) {
        _XtBuildKeysymTable(dpy,perDisplay); /* perDisplay->keysyms*/
        perDisplay ->modsToKeysyms =
            _XtBuildModsToKeysymTable(dpy,perDisplay);
    }
    for (ref=0;event->lateModifiers[ref].keysym != NULL;ref++) {
        found = FALSE;
        for (i=0;i<8;i++) {
            temp = &(perDisplay->modsToKeysyms[i]);
            for (j=0;j<temp->count;j++){
                if (perDisplay->modKeysyms[temp->index+j] ==
                      event->lateModifiers[ref].keysym) {
                    *computedMask = *computedMask | temp->mask;
                    if (!event->lateModifiers[ref].knot)
                        *computed |= temp->mask;
                    tempKeysym = event->lateModifiers[ref].keysym;
                    found = TRUE; break;
                }
            }
            if (found) break;
        }
        if (!found  && !event->lateModifiers[ref].knot)
            if (!event->lateModifiers[ref].pair && (tempKeysym == NoSymbol))
                return FALSE;
        /* if you didn't find the modifier and the modifier must be
           asserted then return FALSE. If you didn't find the modifier
           and the modifier must be off, then it is OK . Don't
           return FALSE if this is the first member of a pair or if
           it is the second member of a pair when the first member
           was bound to a modifier */
    if (!event->lateModifiers[ref].pair) tempKeysym = NoSymbol;
    }
    return TRUE;
}

Boolean _XtRegularMatch(event,eventSeq)
    Event *event;
    TMEventPtr eventSeq;
{
    Modifiers computed =0;
    Modifiers computedMask =0;
    Boolean resolved = TRUE;
    if (event->eventCode != (eventSeq->event.eventCode &
               event->eventCodeMask)) return FALSE;
    if (event->lateModifiers != NULL)
        resolved = ComputeLateBindings(event,eventSeq,&computed,&computedMask);
    if (!resolved) return FALSE;
    computed |= event->modifiers;
    computedMask |= event->modifierMask;

    return ( (computed & computedMask) ==
          (eventSeq->event.modifiers & computedMask));

}


Boolean _XtMatchUsingDontCareMods(event,eventSeq)
    Event *event;
    TMEventPtr eventSeq;
{
    Modifiers modifiers_return;
    KeySym keysym_return;
    Modifiers temp;
    int i;
    Modifiers computed = 0;
    Modifiers computedMask = 0;
    Boolean resolved = TRUE;

    if (event->lateModifiers != NULL)
        resolved = ComputeLateBindings(event,eventSeq,&computed,&computedMask);
    if (!resolved) return FALSE;
    computed |= event->modifiers;
    computedMask |= event->modifierMask;

    if ( (computed & computedMask) ==
        (eventSeq->event.modifiers & computedMask) ) {
	Modifiers least_mod;
        XtTranslateKeycode(eventSeq->dpy,(KeyCode) eventSeq->event.eventCode,
            0,&modifiers_return,&keysym_return);
        if ((keysym_return & event->eventCodeMask)  == event->eventCode ) 
             return TRUE;
        temp = ~computedMask & modifiers_return;
        if (temp == 0) return FALSE;
	for (least_mod = 1; (least_mod & modifiers_return)==0;)
	    least_mod <<= 1;
        for (i = modifiers_return; i >= least_mod; i--)
	    /* all useful combinations of 8 modifier bits */
            if  (temp & i != 0) {
                 XtTranslateKeycode(eventSeq->dpy,(KeyCode)eventSeq->event.eventCode,
                    (Modifiers) i,&modifiers_return,&keysym_return);
                 if (keysym_return  ==
                     (event->eventCode &  event->eventCodeMask)) return TRUE;
            }
     }
    return FALSE;

}
void XtConvertCase(dpy,keysym,lower_return,upper_return)
    Display *dpy;
    KeySym keysym;
    KeySym* lower_return,*upper_return;
{
    XtPerDisplay perDisplay;
    perDisplay = _XtGetPerDisplay(dpy);
    if (perDisplay != NULL && perDisplay->defaultCaseConverter != NULL)
        (*perDisplay->defaultCaseConverter)(dpy,keysym,
            lower_return,upper_return);
}

Boolean _XtMatchUsingStandardMods (event,eventSeq)
    Event *event;
    TMEventPtr eventSeq;
{
    Modifiers modifiers_return;
    KeySym keysym_return;
    Modifiers computed= 0;
    Modifiers computedMask = 0;
    Boolean resolved = TRUE;

    XtTranslateKeycode (eventSeq->dpy,(KeyCode) eventSeq->event.eventCode,
        (Modifiers)(eventSeq->event.modifiers&StandardMask),
        &modifiers_return,&keysym_return);

    if ((event->eventCode & event->eventCodeMask) ==
             (keysym_return & event->eventCodeMask)) {
        if (event->lateModifiers != NULL) 
            resolved = ComputeLateBindings(event,
                eventSeq,&computed,&computedMask);
        if (!resolved) return FALSE;
        computed |= event->modifiers;
        computedMask |= event->modifierMask;

        return (
            ((computed & computedMask) ==
             (eventSeq->event.modifiers & ~modifiers_return &
              computedMask))) ;
    }
    return FALSE;
}

static int MatchEvent(translations, eventSeq) 
  XtTranslations translations;
  register TMEventPtr eventSeq;
{
    register EventObjPtr eventTbl = translations->eventObjTbl;
    register int i;

/*
 * The use of "Any" as a modifier can cause obscure bugs since an incoming
 * event may match the "Any" alternative even though a more specific (and
 * correct) event is in the table. It's hard to know which event in the table
 * to match since either could be correct, depending on the circumstances.
 * It's unfortunate that there isn't a unique identifier for a given event...
 * The "any" should be used only when all else fails, but this complicates
 * the algorithms quite a bit. Relying on the order of the productions in the
 * translation table helps, but is not sufficient, both because the earlier
 * specific event may not apply to the current state, and because we can
 * merge translations, resulting in events in the table that are "out of
 * order"
 */
    for (i=0; i < translations->numEvents; i++) {
        if (eventTbl[i].event.eventType ==
                (eventSeq->event.eventType & 0x7f)
            && (eventTbl[i].event.matchEvent != NULL) 
            && ((*eventTbl[i].event.matchEvent)(
                       &eventTbl[i].event,eventSeq)))
                    return i;
            
    }    
    return (-1);
}

static Boolean IsModifier(event)
    TMEventPtr event;
{
    Display *dpy = event->dpy;
    XtPerDisplay pd = _XtGetPerDisplay(dpy);
    int i,j,index;
    int k =0;
    KeySym keysym;
    ModToKeysymTable* temp;

    if (pd != NULL) {
        if (pd->modsToKeysyms == NULL) {
            _XtBuildKeysymTable(dpy,pd); /* pd->keysyms*/
            pd ->modsToKeysyms =
            _XtBuildModsToKeysymTable(dpy,pd);
        }
        for (;k <pd->keysyms_per_keycode;k++) {
            index = ((event->event.eventCode-dpy->min_keycode)*
                             pd->keysyms_per_keycode)+k;
            keysym = pd->keysyms[index];

            for (i=0;i<8;i++) {
                temp = &(pd->modsToKeysyms[i]);
                for (j=0;j<temp->count;j++){
                    if (pd->modKeysyms[temp->index+j] == keysym) return TRUE;
                }
            }
        }
    }
    return FALSE;
}



/*
 * there are certain cases where you want to ignore the event and stay
 * in the same state.
 */
static Boolean Ignore(event)
    TMEventPtr event;
{
    if (event->event.eventType == MotionNotify
        ||( (event->event.eventType == KeyPress
           || event->event.eventType == KeyRelease)
           && IsModifier(event))
/*
        || event->event.eventType == ButtonPress
	|| event->event.eventType == ButtonRelease
*/
       )
	    return TRUE;
    else
	    return FALSE;
}


static void XEventToTMEvent(event, tmEvent)
    register XEvent *event;
    register TMEventPtr tmEvent;
{
    tmEvent->dpy = event->xany.display;
    tmEvent->event.eventCodeMask = 0;
    tmEvent->event.eventCode = 0;
    tmEvent->event.modifierMask = 0;
    tmEvent->event.modifiers = 0;
    tmEvent->event.eventType = event->type;
    tmEvent->event.lateModifiers = NULL;
    tmEvent->event.matchEvent = NULL;
    tmEvent->event.standard = FALSE;

    switch (event->type) {

	case KeyPress:
	case KeyRelease:
	    tmEvent->event.modifiers = event->xkey.state;
            tmEvent->event.eventCode = event->xkey.keycode;
	    break;

	case ButtonPress:
	case ButtonRelease:
	    tmEvent->event.eventCode = event->xbutton.button;
	    tmEvent->event.modifiers = event->xbutton.state;
	    break;

	case MotionNotify:
	    tmEvent->event.modifiers = event->xmotion.state;
	    break;

	case EnterNotify:
	case LeaveNotify:
	    tmEvent->event.modifiers = event->xcrossing.state;
	    break;

	default:
	    break;
    }
}


static unsigned long GetTime(tm, event)
    XtTM tm;
    register XEvent *event;
{
    switch (event->type) {

        case KeyPress:
	case KeyRelease:
	    return event->xkey.time;

        case ButtonPress:
	case ButtonRelease:
	    return event->xbutton.time;

	default:
	    return tm->lastEventTime;

    }

}


/* ARGSUSED */
static void _XtTranslateEvent (w, closure, event)
    Widget w;
    caddr_t closure;
    register    XEvent * event;
{
    register XtTranslations stateTable = ((XtTM)closure)->translations;
    StatePtr oldState;
    TMEventRec curEvent;
    StatePtr current_state = ((XtTM)closure)->current_state;
    int     index;
    register ActionPtr actions;
    XtBoundActions proc_table = ((XtTM)closure)->proc_table;
    XtBoundAccActions accProcTbl = stateTable->accProcTbl;
    XtTM tm = (XtTM)closure;
/* gross disgusting special case ||| */
    if ((event->type == EnterNotify || event->type == LeaveNotify)
        &&( event->xcrossing.detail == NotifyInferior
        ||  event->xcrossing.mode != NotifyNormal) )
	return;

    XEventToTMEvent (event, &curEvent);

    if (stateTable == NULL) {
        XtAppWarningMsg(XtWidgetToApplicationContext(w),
		"translationError","nullTable","XtToolkitError",
            "Can't translate event through NULL table",
            (String *)NULL, (Cardinal *)NULL);
       return ;
    }
    index = MatchEvent (stateTable, &curEvent);
    if (index == -1)
	/* some event came in that we don't have any states for */
	/* ignore it. */
	return;

    /* are we currently in some state other than ground? */
    if (current_state != NULL) {

	oldState = current_state;

	/* find this event in the current level */
	while (current_state != NULL) {
	    Event *ev;
	    /* does this state's index match? --> done */
	    if (current_state->index == index) break;

	    /* is this an event timer? */
	    ev = &stateTable->eventObjTbl[
		current_state->index].event;
	    if (ev->eventType == _XtEventTimerEventType) {

		/* does the succeeding state match? */
		StatePtr nextState = current_state->nextLevel;

		/* is it within the timeout? */
		if (nextState != NULL && nextState->index == index) {
		    unsigned long time = GetTime(tm, event);
		    unsigned long delta = ev->eventCode;
		    if (delta == 0) delta = stateTable->clickTime;
		    if (tm->lastEventTime + delta >= time) {
			current_state = nextState;
			break;
		    }
		}
	    }

	    /* go to next state */
	    current_state = current_state->next;
	}

	if (current_state == NULL)
	    /* couldn't find it... */
	    if (Ignore(&curEvent)) {
		/* ignore it. */
	        current_state = oldState;
		return;
	    } /* do ground state */
    }

    if (current_state == NULL) {
	/* check ground level */
	current_state = stateTable->eventObjTbl[index].state;
	if (current_state == NULL) return;
    }

    tm->lastEventTime = GetTime (tm, event);

    /* perform any actions */
    actions = current_state->actions;
    while (actions != NULL) {
	/* perform any actions */
        if (actions->index >= 0) {
           if (proc_table[actions->index] != NULL)
              (*(proc_table[actions->index]))(
                w,event, actions->params, &actions->num_params);
        }
        else {
            int temp = -(actions->index+1);
            if (accProcTbl[temp].proc != NULL &&
                    accProcTbl[temp].widget != 0)
              (*(accProcTbl[temp].proc))(
                accProcTbl[temp].widget,
                event, actions->params, &actions->num_params);
        }

	actions = actions->next;
    }

    /* move into successor state */
    ((XtTM)tm)->current_state = current_state->nextLevel;
}

static Boolean EqualEvents(event1, event2)
    Event *event1, *event2;
{
    int i = 0;
    int j = 0;
    int index1,index2;
    if (event1->eventType     == event2->eventType
	&& event1->eventCode     == event2->eventCode
	&& event1->eventCodeMask == event2->eventCodeMask
	&& event1->modifiers     == event2->modifiers
	&& event1->modifierMask  == event2->modifierMask) {
        if (event1->lateModifiers != NULL || event2->lateModifiers != NULL) {
            if (event1->lateModifiers != NULL)
                for (;event1->lateModifiers[i].keysym != NoSymbol;i++) {}
            if (event2->lateModifiers != NULL)
                for (;event2->lateModifiers[j].keysym != NoSymbol;j++) {}
            if (i != j) return FALSE;
            for (index1=0;index1<i;index1++) {
                for (index2=0;index2<i;index2++) {            
                    if( (event1->lateModifiers[index1].keysym ==
                        event2->lateModifiers[index2].keysym)
                       && (event1->lateModifiers[index1].knot ==
                         event2->lateModifiers[index2].knot) ){
                           j--; break;
                     } /*if*/
                }/*for*/
            }/*for*/
            if (j != 0) return FALSE;
        }
        return TRUE;
    }
    return FALSE;

}

static int GetEventIndex(stateTable, event)
    XtTranslations stateTable;
    register EventPtr event;
{
    register int	index;
    register EventObjPtr new;
    register EventObjPtr eventTbl = stateTable->eventObjTbl;

    for (index=0; index < stateTable->numEvents; index++)
        if (EqualEvents(&eventTbl[index].event, &event->event)) return(index);

    if (stateTable->numEvents == stateTable->eventTblSize) {
        stateTable->eventTblSize += 10;
	stateTable->eventObjTbl = (EventObjPtr) XtRealloc(
	    (char *)stateTable->eventObjTbl, 
	    stateTable->eventTblSize*sizeof(EventObjRec));
    }

    new = &stateTable->eventObjTbl[stateTable->numEvents];

    new->event = event->event;
    new->state = NULL;

    return stateTable->numEvents++;
}

static StatePtr NewState(index, stateTable)
    int index;
    XtTranslations stateTable;
{
    register StatePtr state = XtNew(StateRec);

    state->index = index;
    state->nextLevel = NULL;
    state->next = NULL;
    state->actions = NULL;
    state->forw = stateTable->head;
    state->cycle = FALSE;
    stateTable->head = state;
/*
    state->back = NULL;
    if (state->forw != NULL) state->forw->back = state;
*/

    return state;
}

typedef NameValueRec CompiledAction;
typedef NameValueTable CompiledActionTable;

#ifdef lint
Opaque _CompileActionTable(actions, count)
#else
CompiledActionTable _CompileActionTable(actions, count)
#endif
    register struct _XtActionsRec *actions;
    register Cardinal count;
{
    register int i;
    register CompiledActionTable compiledActionTable;

    compiledActionTable = (CompiledActionTable) XtCalloc(
	count+1, (unsigned) sizeof(CompiledAction));

    for (i=0; i<count; i++) {
	compiledActionTable[i].name = actions[i].string;
	compiledActionTable[i].signature = StringToAction(actions[i].string);
	compiledActionTable[i].value = (Value) actions[i].proc;
    }

    compiledActionTable[count].name = NULL;
    compiledActionTable[count].signature = NULL;
    compiledActionTable[count].value = NULL;

#ifdef lint
    return (Opaque) compiledActionTable;
#else
    return compiledActionTable;
#endif
}

static EventMask EventToMask(event)
    EventObjPtr	event;
{
static EventMask masks[] = {
        0,			    /* Error, should never see  */
        0,			    /* Reply, should never see  */
        KeyPressMask,		    /* KeyPress			*/
        KeyReleaseMask,		    /* KeyRelease		*/
        ButtonPressMask,	    /* ButtonPress		*/
        ButtonReleaseMask,	    /* ButtonRelease		*/
        PointerMotionMask	    /* MotionNotify		*/
		| Button1MotionMask
		| Button2MotionMask
		| Button3MotionMask
		| Button4MotionMask
		| Button5MotionMask
		| ButtonMotionMask,
        EnterWindowMask,	    /* EnterNotify		*/
        LeaveWindowMask,	    /* LeaveNotify		*/
        FocusChangeMask,	    /* FocusIn			*/
        FocusChangeMask,	    /* FocusOut			*/
        KeymapStateMask,	    /* KeymapNotify		*/
        ExposureMask,		    /* Expose			*/
        0,			    /* GraphicsExpose, in GC    */
        0,			    /* NoExpose, in GC		*/
        VisibilityChangeMask,       /* VisibilityNotify		*/
        SubstructureNotifyMask,     /* CreateNotify		*/
        StructureNotifyMask,	    /* DestroyNotify		*/
/*		| SubstructureNotifyMask, */
        StructureNotifyMask,	    /* UnmapNotify		*/
/*		| SubstructureNotifyMask, */
        StructureNotifyMask,	    /* MapNotify		*/
/*		| SubstructureNotifyMask, */
        SubstructureRedirectMask,   /* MapRequest		*/
        StructureNotifyMask,	    /* ReparentNotify		*/
/*		| SubstructureNotifyMask, */
        StructureNotifyMask,	    /* ConfigureNotify		*/
/*		| SubstructureNotifyMask, */
        SubstructureRedirectMask,   /* ConfigureRequest		*/
        StructureNotifyMask,	    /* GravityNotify		*/
/*		| SubstructureNotifyMask, */
        ResizeRedirectMask,	    /* ResizeRequest		*/
        StructureNotifyMask,	    /* CirculateNotify		*/
/*		| SubstructureNotifyMask, */
        SubstructureRedirectMask,   /* CirculateRequest		*/
        PropertyChangeMask,	    /* PropertyNotify		*/
        0,			    /* SelectionClear		*/
        0,			    /* SelectionRequest		*/
        0,			    /* SelectionNotify		*/
        ColormapChangeMask,	    /* ColormapNotify		*/
        0,			    /* ClientMessage		*/
        0 ,			    /* MappingNotify		*/
    };

    /* Events sent with XSendEvent will have high bit set. */
    /* !!! This isn't true anymore... fix this ||| */
    unsigned long eventType = event->event.eventType & 0x7f;
    if (eventType == MotionNotify) {
        Modifiers modifierMask = event->event.modifierMask;
        EventMask returnMask = 0;
        Modifiers tempMask;

        if (modifierMask == 0) {
	    if (event->event.modifiers == AnyButtonMask)
		return ButtonMotionMask;
	    else
		return PointerMotionMask;
	}
        tempMask = modifierMask &
	    (Button1Mask | Button2Mask | Button3Mask
	     | Button4Mask | Button5Mask);
        if (tempMask == 0)
	    return PointerMotionMask;
        if ((tempMask & Button1Mask)!=0)
            returnMask |= Button1MotionMask;
        if ((tempMask & Button2Mask) != 0)
            returnMask |= Button2MotionMask;
        if ((tempMask & Button3Mask)!= 0)
            returnMask |= Button3MotionMask;
        if ((tempMask & Button4Mask)!= 0)
            returnMask |= Button4MotionMask;
        if ((tempMask & Button5Mask)!= 0)
            returnMask |= Button5MotionMask;
        return returnMask;
    }
    return ((eventType >= XtNumber(masks)) ?  0 : masks[eventType]);
}
/*** Public procedures ***/

void _XtInstallTranslations(widget, stateTable)
    Widget widget;
    XtTranslations stateTable;
{
    register EventMask	eventMask = 0;
    register Boolean	nonMaskable = FALSE;
    register Cardinal	i;

/*    widget->core.translations = stateTable; */
    if (stateTable == NULL) return;

    for (i = 0; i < stateTable->numEvents; i++) {
	register EventMask mask = EventToMask(&stateTable->eventObjTbl[i]);

	eventMask |= mask;
	nonMaskable |= (mask == 0);
    }

    /* double click needs to make sure that you have selected on both
	button down and up. */

    if (eventMask & ButtonPressMask) eventMask |= ButtonReleaseMask;
    if (eventMask & ButtonReleaseMask) eventMask |= ButtonPressMask;

    XtAddEventHandler(
        widget, eventMask, nonMaskable,
             _XtTranslateEvent, (caddr_t)&widget->core.tm);

}

void XtUninstallTranslations(widget)
    Widget widget;
{
    XtRemoveEventHandler(widget,~0L,TRUE,_XtTranslateEvent,
                     (caddr_t)&widget->core.tm);
    widget->core.tm.translations = NULL;
    if (widget->core.tm.proc_table != NULL)
        XtFree((char *)widget->core.tm.proc_table);
    widget->core.tm.proc_table = NULL;
    widget->core.tm.current_state = NULL;
}


typedef struct _ActionListRec *ActionList;
typedef struct _ActionListRec {
    ActionList		next;
    CompiledActionTable table;
} ActionListRec;

static void ReportUnboundActions(tm, stateTable)
    XtTM tm;
    XtTranslations stateTable;
{
    Cardinal num_unbound;
    char     message[10000];
    register Cardinal num_chars;
    register Cardinal i;

    num_unbound = 0;
    (void) strcpy(message, "Actions not found: ");
    num_chars = strlen(message);

    for (i=0; i < stateTable->numQuarks; i++) {
	if (tm->proc_table[i] == NULL) {
	    String s = XrmQuarkToString(stateTable->quarkTable[i]);
	    if (num_unbound != 0) {
		(void) strcpy(&message[num_chars], ", ");
		num_chars = num_chars + 2;
	    }
	    (void) strcpy(&message[num_chars], s);
	    num_chars += strlen(s);
	    num_unbound++;
	}
    }
    message[num_chars] = '\0';
    if (num_unbound != 0)
        XtWarningMsg("translationError","unboundActions","XtToolkitError",
                  message, (String *)NULL, (Cardinal *)NULL);
}


static int BindActions(tm, compiledActionTable,index)
    XtTM tm;
    CompiledActionTable compiledActionTable;
    Cardinal index;
{
    XtTranslations stateTable=tm->translations;
    int unbound = stateTable->numQuarks;
    int i;

    for ( ; index < stateTable->numQuarks; index++) {
       if (tm->proc_table[index] == NULL) {
           /* attempt to bind it */
           register XrmQuark q = stateTable->quarkTable[index];
           for (i = 0; compiledActionTable[i].name != NULL; i++) {
               if (compiledActionTable[i].signature == q) {
		   tm->proc_table[index] = 
                     (XtActionProc) compiledActionTable[i].value;
                   unbound--;
                   break;
               }
           }
       } else {
           /* already bound, leave it alone */
           unbound--;
       }
     }
     return(unbound);
}



static int BindAccActions(widget,stateTable,
                          compiledActionTable,index,accBindings)
    Widget widget;
    XtTranslations stateTable;
    CompiledActionTable compiledActionTable;
    Cardinal index;
    XtBoundAccActions accBindings;
{
    int unbound = stateTable->accNumQuarks;
    int i;

    for ( ; index < stateTable->accNumQuarks; index++) {
       if (accBindings[index].proc == NULL) {
           /* attempt to bind it */
           register XrmQuark q = stateTable->accQuarkTable[index];
           for (i = 0; compiledActionTable[i].name != NULL; i++) {
               if (compiledActionTable[i].signature == q) {
                   accBindings[index].widget =widget;
		   accBindings[index].proc=
                     (XtActionProc) compiledActionTable[i].value;
                   unbound--;
                   break;
               }
           }
       } else {
           /* already bound, leave it alone */
           unbound--;
       }
     }
     return(unbound);
}


void _XtBindActions(widget,tm,index)
    Widget	    widget;
    XtTM          tm;
    Cardinal        index;
{
    XtTranslations  stateTable=tm->translations;
    register Widget	    w;
    register WidgetClass    class;
    register ActionList     actionList;
    int unbound = -1; /* initialize to non-zero */
    XtAppContext app;

/* ||| Kludge error that Leo depends upon */
    w = widget;
    if (stateTable == NULL) return;
    tm->proc_table= (XtBoundActions) XtCalloc(
                      stateTable->numQuarks,sizeof(XtBoundActions));
    do {
/* ||| */
        class = w->core.widget_class;
        do {
            if (class->core_class.actions != NULL)
             unbound = BindActions(
	        tm,(CompiledActionTable)class->core_class.actions, index);
	    class = class->core_class.superclass;
        } while (unbound != 0 && class != NULL);
/* ||| Kludge error that Leo depends upon */
    w = w->core.parent;
    } while (unbound != 0 && w != NULL);
/* ||| */

    app = XtWidgetToApplicationContext(widget);
    for (actionList = app->action_table;
	 unbound != 0 && actionList != NULL;
	 actionList = actionList->next) {
	unbound = BindActions(tm, actionList->table,index);
    }
    if (unbound != 0) ReportUnboundActions(tm, stateTable);
}

static
void _XtBindAccActions(widget,stateTable,index,accBindings)
    Widget	    widget;
    XtTranslations  stateTable;
    Cardinal        index;
    XtBoundAccActions *accBindings;
{
    register Widget	    w;
    register WidgetClass    class;
    register ActionList     actionList;
    int unbound = -1; /* initialize to non-zero */
    XtBoundAccActions accTemp;
    XtAppContext app;

/* ||| Kludge error that Leo depends upon */
    w = widget;
    if (stateTable == NULL) return;
    accTemp = (XtBoundAccActions) XtCalloc(
                      stateTable->accNumQuarks,sizeof(XtBoundAccActionRec));
do {
/* ||| */
    class = w->core.widget_class;
    do {
        if (class->core_class.actions != NULL)
         unbound = BindAccActions(widget,
	    stateTable,(CompiledActionTable)class->core_class.actions,
                         index,accTemp);
	class = class->core_class.superclass;
    } while (unbound != 0 && class != NULL);
/* ||| Kludge error that Leo depends upon */
w = w->core.parent;
} while (unbound != 0 && w != NULL);
/* ||| */

    app = XtWidgetToApplicationContext(widget);
    for (actionList = app->action_table;
	 unbound != 0 && actionList != NULL;
	 actionList = actionList->next) {
	unbound = BindAccActions(widget,stateTable, actionList->table,
                                index, (XtBoundAccActions) accBindings);
    }
/*    if (unbound != 0) ReportUnboundActions(tm, stateTable);*/
    (*accBindings) = accTemp;
}

void XtAddActions(actions, num_actions)
    XtActionList actions;
    Cardinal num_actions;
{
    XtAppAddActions(_XtDefaultAppContext(), actions, num_actions);
}

void XtAppAddActions(app, actions, num_actions)
    XtAppContext app;
    XtActionList actions;
    Cardinal num_actions;
{
    register ActionList rec;

    rec = XtNew(ActionListRec);
    rec->next = app->action_table;
    app->action_table = rec;
    rec->table = (CompiledActionTable) _CompileActionTable(actions, num_actions);
}

void _XtInitializeStateTable(pStateTable)
    XtTranslations *pStateTable;
{
    register XtTranslations  stateTable;

    (*pStateTable) = stateTable = XtNew(TranslationData);
    stateTable->operation = XtTableReplace;
    stateTable->numEvents = 0;
    stateTable->eventTblSize = 0;
    stateTable->eventObjTbl = NULL;
    stateTable->clickTime = 200; /* ||| need some way of setting this !!! */
    stateTable->head = NULL;
    stateTable->quarkTable =
        (XrmQuark *)XtCalloc(20,(unsigned)sizeof(XrmQuark));
    stateTable->quarkTblSize = 20;
    stateTable->numQuarks = 0;
    stateTable->accNumQuarks = 0;
    stateTable->accQuarkTable = NULL;
    stateTable->accProcTbl= NULL;
    stateTable->accQuarkTblSize = 0;
}

void _XtAddEventSeqToStateTable(eventSeq, stateTable)
    register EventSeqPtr eventSeq;
    XtTranslations stateTable;
{
    register int     index;
    register StatePtr *state;

    if (eventSeq == NULL) return;

    /* initialize event index and state ptr */
    /* note that all states in the event seq passed in start out null */
    /* we fill them in with the matching state as we traverse the list */

    index = GetEventIndex (stateTable, eventSeq);
    state = &stateTable->eventObjTbl[index].state;

    for (;;) {
    /* index is eventIndex for event */
    /* *state is head of state chain for current state */

	while (*state != NULL && (*state)->index != index)
	    state = &(*state)->next;
	if (*state == NULL) *state = NewState (index, stateTable);

	/* *state now points at state record matching event */
	eventSeq->state = *state;

	if (eventSeq->actions != NULL) {
	    if ((*state)->actions != NULL) {
		XtWarningMsg ("translationError","ambigiousActions", 
                           "XtToolkitError",
                           "Overriding earlier translation manager actions.",
                            (String *)NULL, (Cardinal *)NULL);
		FreeActions((*state)->actions);
	    }
	    (*state)->actions = eventSeq->actions;
	}

    /* are we done? */
	eventSeq = eventSeq->next;
	if (eventSeq == NULL) break;
	if (eventSeq->state != NULL) {
	    /* we've been here before... must be a cycle in the event seq. */
	    (*state)->nextLevel = eventSeq->state;
	    (*state)->cycle = TRUE;
	    break;
	}

	if ((*state)->cycle) {

	    /* unroll the loop one state */
	    /* this code hurts my head... ||| think about multiple */
	    /* states pointing at same "next" state record */

	    StatePtr oldNextLevel = (*state)->nextLevel;
	    register StatePtr newNextLevel =
		NewState(oldNextLevel->index, stateTable);

	    newNextLevel->actions = oldNextLevel->actions;
	    newNextLevel->nextLevel = oldNextLevel->nextLevel;
	    newNextLevel->next = oldNextLevel->next;
	    newNextLevel->cycle = TRUE;
	    (*state)->cycle = FALSE;
	    (*state)->nextLevel = newNextLevel;
	}
	state = &(*state)->nextLevel;
	index = GetEventIndex (stateTable, eventSeq);
    }
}


typedef struct _StateMapRec *StateMap;
typedef struct _StateMapRec {
    StatePtr	old, new;
    StateMap	next;
} StateMapRec;

static void MergeStates(old, new, override, indexMap,
                           quarkIndexMap, accQuarkIndexMap,oldTable, stateMap)
    register StatePtr *old, new;
    Boolean override;
    Cardinal *indexMap, *quarkIndexMap,*accQuarkIndexMap;
    XtTranslations oldTable;
    StateMap stateMap;
{
    register StatePtr state;
    StateMap oldStateMap = stateMap;
    ActionRec *a,**aa,*b;

    while (new != NULL) {
	register int index = indexMap[new->index];

	/* make sure old and new match */
	for (state = *old; ; state=state->next) {
	    if (state == NULL) {
		/* corresponding arc doesn't exist, add it */
		state = NewState(index, oldTable);
		state->next = *old;
		*old = state;
		break;
	    }

	    if (state->index == index) /* found it */ break;
	}
    
	/* state and new are pointing at corresponding state records */
	{
	    StateMap temp = XtNew(StateMapRec);
	    temp->next = stateMap;
	    stateMap = temp;
	    temp->old = state;
	    temp->new = new;
	}
    
	/* merge the actions */
	while (state->actions != NULL && override) {
	   a = state->actions;
	   state->actions=a->next;
	   XtFree((char *)a);
	}
      if (state->actions == NULL) {
        aa = &(state->actions);
        b = new->actions;
        while (b != NULL) {
           a = XtNew(ActionRec); 
           a->token = NULL;
           if (b->index >= 0)
               a->index = quarkIndexMap[b->index];
           else
               a->index = -(accQuarkIndexMap[-(b->index+1)]+1);
           a->params = b->params;
           a->num_params=b->num_params;
           a->next = NULL;
           *aa = a;
           aa = &a->next;
           b=b->next;
        }
      }


                     
	if (new->cycle) {
	    /* we're in a cycle, search state map for corresponding state */
	    register StateMap temp;
	    for (
		temp=stateMap;
		temp->new != new->nextLevel;
		temp=temp->next)
	        if (temp == NULL)
                     XtErrorMsg("translationError","mergingTablesWithCycles",
                             "XtToolkitError",
"Trying to merge translation tables with cycles, and can't resolve this cycle."
			     , (String *)NULL, (Cardinal *)NULL);
	    (*old)->nextLevel = temp->old;
	} else if (! (*old)->cycle || override) {
	    if ((*old)->cycle) (*old)->nextLevel = NULL;
	    MergeStates(
	        &(*old)->nextLevel,
		new->nextLevel,
		override,
		indexMap,quarkIndexMap,accQuarkIndexMap,
		oldTable,
		stateMap);
	}
    
	new = new->next;
    }
    while (stateMap != oldStateMap) {
	StateMap temp = stateMap;
	stateMap = stateMap->next;
	XtFree((char *)temp);
    }
}


static void MergeTables(old, new, override,accProcTbl)
    register XtTranslations old, new;
    Boolean override;
    XtBoundAccActions accProcTbl;
{
    register Cardinal i,j,k;
    Cardinal *indexMap,*quarkIndexMap,*accQuarkIndexMap;

    if (new == NULL) return;
    if (old == NULL) {
	XtWarningMsg("translationError","mergingNullTable","XtToolkitError",
            "Old translation table was null, cannot modify.",
	    (String *)NULL, (Cardinal *)NULL);
	return;
    }

    indexMap = (Cardinal *)XtCalloc(new->eventTblSize, sizeof(Cardinal));

    for (i=0; i < new->numEvents; i++) {
	register Cardinal j;
	EventObjPtr newEvent = &new->eventObjTbl[i];

	for (j=0; j < old->numEvents; j++)
	    if (EqualEvents(
	        &newEvent->event, &old->eventObjTbl[j].event)) break;

	if (j==old->numEvents) {
	    if (j == old->eventTblSize) {
		old->eventTblSize += 10;
		old->eventObjTbl = (EventObjPtr) XtRealloc(
		    (char *)old->eventObjTbl, 
		    old->eventTblSize*sizeof(EventObjRec));
	    }
	    old->eventObjTbl[j] = *newEvent;
	    old->eventObjTbl[j].state = NULL;
	    old->numEvents++;
	}
	indexMap[i] = j;
    }
/* merge quark tables */
  quarkIndexMap = (Cardinal *)XtCalloc(new->quarkTblSize, sizeof(Cardinal));


    for (i=0; i < new->numQuarks; i++) {
        register Cardinal j;

       for (j=0; j < old->numQuarks; j++)
            if (old->quarkTable[j] == new->quarkTable[i]) break;
                

       if (j==old->numQuarks) {
            if (j == old->quarkTblSize) {
                old->quarkTblSize += 20;
                old->quarkTable = (XrmQuark*) XtRealloc(
                    (char *)old->quarkTable,
                    old->quarkTblSize*sizeof(int));
                  }
            old->quarkTable[j]=new->quarkTable[i];
            old->numQuarks++;
        }
        quarkIndexMap[i] = j;
    }
/* merge accelerator quark tables */
  accQuarkIndexMap = (Cardinal *)XtCalloc(
      new->accQuarkTblSize, sizeof(Cardinal));
    k = old->accNumQuarks;

    for (i=0,j=old->accNumQuarks; i < new->accNumQuarks; ) {
        if (j == old->accQuarkTblSize) {
            old->accQuarkTblSize += 20;
            old->accQuarkTable = (XrmQuark*) XtRealloc(
                (char *)old->accQuarkTable,
                old->accQuarkTblSize*sizeof(int));
         }
         old->accQuarkTable[j]=new->accQuarkTable[i];
         old->accNumQuarks++;
         accQuarkIndexMap[i++] = j++;
    }

/* merge accelerator action bindings */

    if (old->accProcTbl == NULL) {
        old->accProcTbl = (XtBoundAccActionRec*)XtCalloc(
            old->accQuarkTblSize,sizeof(XtBoundAccActionRec) );
    }
    else old->accProcTbl = (XtBoundAccActionRec*)XtRealloc(
        (char *)old->accProcTbl,
	old->accQuarkTblSize*sizeof(XtBoundAccActionRec) );
    for (i=0/*,k=k*/;i<new->accNumQuarks;){
        old->accProcTbl[k].widget = accProcTbl[i].widget;
        old->accProcTbl[k++].proc = accProcTbl[i++].proc;
    }

    for (i=0; i < new->numEvents; i++)
	MergeStates(
	    &old->eventObjTbl[indexMap[i]].state,
	    new->eventObjTbl[i].state,
	    override,
	    indexMap,quarkIndexMap,accQuarkIndexMap,
	    old,
	    (StateMap) NULL);
   XtFree((char *)indexMap);
   XtFree((char *)quarkIndexMap);
   XtFree((char *)accQuarkIndexMap);
}


void _XtOverrideTranslations(old, new,merged)
    XtTranslations old, new,*merged;
{
    XtTranslations temp;
    if (old == NULL) {
	*merged = new;
	return;
    }
    _XtInitializeStateTable(&temp);
    temp->clickTime = new->clickTime;
    /* merge in new table, overriding any existing bindings from old */
    MergeTables(temp, new, FALSE,new->accProcTbl);
    MergeTables(temp, old, FALSE,old->accProcTbl);
    *merged= temp;
}


void _XtAugmentTranslations(old, new,merged)
    XtTranslations old, new,*merged;
{
    /* merge in extra bindings, keeping old binding if any */
    XtTranslations temp;
    if (old == NULL) {
	*merged = new;
	return;
    }
    _XtInitializeStateTable(&temp);
    temp->clickTime = old->clickTime;
    MergeTables(temp, old, FALSE,old->accProcTbl);
    MergeTables(temp, new, FALSE,new->accProcTbl);
    *merged= temp;
}

/*ARGSUSED*/
static void _MergeTranslations (args, num_args, from, to)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr from,to;
{
    static XtTranslations merged;
    XtTranslations old,new;
    TMkind operation;

    if (*num_args != 0)
	XtWarningMsg("invalidParameters","mergeTranslations","XtToolkitError",
             "MergeTM to TranslationTable needs no extra arguments",
               (String *)NULL, (Cardinal *)NULL);

    old = ((TMConvertRec*)from->addr)->old;
    new = ((TMConvertRec*)from->addr)->new;
    operation = ((TMConvertRec*)from->addr)->operation;
    if (operation == override)
    _XtOverrideTranslations(old, new,&merged);
    else
    if (operation == augment)
    _XtAugmentTranslations(old,new,&merged);
     to->addr= (caddr_t)&merged;
     to->size=sizeof(XtTranslations);
}

void XtOverrideTranslations(widget, new)
    Widget widget;
    XtTranslations new;
{
/*
    MergeTables(widget->core.translations, new, TRUE);
*/
    XrmValue from,to;
    TMConvertRec foo;
    XtTranslations newTable;
    from.addr = (caddr_t)&foo;
    from.size = sizeof(TMConvertRec);
    foo.old = widget->core.tm.translations;
    foo.new = new;
    foo.operation = override;

    XtDirectConvert((XtConverter) _MergeTranslations, (XrmValuePtr) NULL,
	    0, &from, &to);
/*    _XtOverrideTranslations(widget->core.tm.translations, new);*/
      newTable = (*(XtTranslations*)to.addr);
     if (XtIsRealized(widget)) {
            XtUninstallTranslations((Widget)widget);
           ((WindowObj)widget)->win_obj.tm.translations = newTable;
           _XtBindActions(widget,&((WindowObj)widget)->win_obj.tm,0);
           _XtInstallTranslations((Widget)widget,newTable);
    }
    else ((WindowObj)widget)->win_obj.tm.translations = newTable;

}
/* ARGSUSED */
static void RemoveAccelerators(widget,closure,data)
    Widget widget;
    caddr_t closure,data;
{
    int i;
    XtTranslations table = (XtTranslations)closure;
    if (table == NULL) {
        XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		"translation error","nullTable","XtToolkitError",
            "Can't remove accelerators from NULL table",
            (String *)NULL, (Cardinal *)NULL);
        return;
    }
    if (table->accProcTbl == NULL) {
        XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		"translation error","nullTable","XtToolkitError",
            "Tried to remove non-existant accelerators",
            (String *)NULL, (Cardinal *)NULL);
        return;
    }
    for (i=0;i<table->accNumQuarks;i++) {
        if (table->accProcTbl[i].widget == widget)
            table->accProcTbl[i].widget = 0;
    }

}
        

void XtInstallAccelerators(destination,source)
    Widget destination,source;
{
    XtBoundAccActions accBindings;
    if ((!XtIsWindowObject(source)) ||
        source->core.accelerators == NULL) return;
/*    if (source->core.accelerators->accProcTbl == NULL)
 *  %%%
 *  The spec is not clear on when actions specified in accelerators are bound;
 *  The most useful (and easiest) thing seems to be to bind them at this time
 *  (rather than at Realize).  Under the current code the preceeding test
 *  seems always to be True, thus guaranteeing accBindings is always set
 *  before being used below.
 */
        _XtBindAccActions(source,source->core.accelerators,0,&accBindings);
    if (destination->core.tm.translations == NULL) {
	destination->core.tm.translations = source->core.accelerators;
	destination->core.tm.translations->accProcTbl = accBindings;
    }
    else {
	XtTranslations temp;
	_XtInitializeStateTable(&temp);
	temp->clickTime = source->core.accelerators->clickTime;
	if (source->core.accelerators->operation == XtTableOverride) {
	    MergeTables(temp,source->core.accelerators,FALSE,accBindings);
	    MergeTables(temp, destination->core.tm.translations,FALSE,
		   destination->core.tm.translations->accProcTbl);
	}
	else { 
	    MergeTables(temp, destination->core.tm.translations,FALSE,
		   destination->core.tm.translations->accProcTbl);
	    MergeTables(temp,source->core.accelerators,FALSE,accBindings);
	}
	destination->core.tm.translations = temp;
    }
    if (XtIsRealized(destination))
        _XtInstallTranslations(destination,
             destination->core.tm.translations);
    XtAddCallback(source, XtNdestroyCallback,
        RemoveAccelerators,(caddr_t)destination->core.tm.translations);
    if (XtClass(source)->core_class.display_accelerator != NULL){
	 char *buf = XtMalloc(100);
	 int len = 100;
	 String str = buf;
	 int i;
         str[0] = '\0';
	 for (i = 0; i < source->core.accelerators->numEvents;) {
	     str = PrintEvent(&str, &len, str,
			    &source->core.accelerators->eventObjTbl[i].event);
	     if (++i == source->core.accelerators->numEvents) break;
	     else {
		 *str++ = '\n';
		 *str = '\0';
	     }
	 }
         (*(XtClass(source)->core_class.display_accelerator))(source,buf);
	 XtFree(buf);
    }
}         
void XtInstallAllAccelerators(destination,source)
    Widget destination,source;
{
    register int i;
    CompositeWidget cw;

    /* Recurse down normal children */
    if (XtIsComposite(source)) {
        cw = (CompositeWidget) source;
        for (i = 0; i < cw->composite.num_children; i++) {
            XtInstallAllAccelerators(destination,cw->composite.children[i]);
        }
    }

    /* Recurse down popup children */
    if (XtIsWindowObject(source)) {
        for (i = 0; i < source->core.num_popups; i++) {
            XtInstallAllAccelerators(destination,source->core.popup_list[i]);
        }
    }
    /* Finally, apply procedure to this widget */
    XtInstallAccelerators(destination,source);
}

void XtAugmentTranslations(widget, new)
    Widget widget;
    XtTranslations new;
{
    XrmValue from,to;
    TMConvertRec foo;
    XtTranslations newTable;
    from.addr = (caddr_t)&foo;
    from.size = sizeof(TMConvertRec);
    foo.old = widget->core.tm.translations;
    foo.new = new;
    foo.operation = augment;

    XtDirectConvert((XtConverter) _MergeTranslations, (XrmValuePtr) NULL,
	    0, &from, &to);
    newTable = (*(XtTranslations*)to.addr);
    if (XtIsRealized(widget)) {
        XtUninstallTranslations((Widget)widget);
        ((WindowObj)widget)->win_obj.tm.translations = newTable;
        _XtBindActions(widget,&((WindowObj)widget)->win_obj.tm,0);
        _XtInstallTranslations((Widget)widget,newTable);
    }
    else ((WindowObj)widget)->win_obj.tm.translations = newTable;

}

static void PrintState(buf, len, str, state, quarkTable, eot)
    String *buf;
    int *len;
    register String str;
    StatePtr state;
    XrmQuark* quarkTable;
    EventObjPtr eot;
{
    register String old = str;
    /* print the current state */
    if (state == NULL) return;

    str = PrintEvent(buf, len, str, &eot[state->index].event);
    if (state->actions != NULL) {
	int offset = str - *buf;
	CHECK_STR_OVERFLOW;
	(void) sprintf(str, "%s: ", (state->cycle ? "(+)" : ""));
	while (*str) str++;
	(void) PrintActions(buf, len, str, state->actions, quarkTable);
	(void) printf("%s\n", *buf);
	str = *buf + offset; *str = '\0';
    }

    /* print succeeding states */
    if (!state->cycle)
	PrintState(buf, len, str, state->nextLevel, quarkTable, eot);

    str = old; *str = '\0';

    /* print sibling states */
    PrintState(buf, len, str, state->next, quarkTable, eot);
    *str = '\0';

}

#ifdef lint
void TranslateTablePrint(translations)
#else
static void TranslateTablePrint(translations)
#endif
    XtTranslations translations;
{
    register Cardinal i;
    int len = 1000;
    char *buf = XtMalloc(1000);

    for (i = 0; i < translations->numEvents; i++) {
	buf[0] = '\0';
	PrintState(
	   &buf,
	   &len,
	   buf,
	   translations->eventObjTbl[i].state,
           translations->quarkTable,
	   translations->eventObjTbl);
    }
    XtFree(buf);
}

/***********************************************************************
 *
 * Pop-up and Grab stuff
 *
 ***********************************************************************/

static Widget _XtFindPopup(widget, name)
    Widget widget;
    String name;
{
    register Cardinal i;
    register XrmQuark q;
    register Widget w;

    q = XrmStringToQuark(name);

    for (w=widget; w != NULL; w=w->core.parent)
	for (i=0; i<w->core.num_popups; i++)
	    if (w->core.popup_list[i]->core.xrm_name == q)
		return w->core.popup_list[i];

    return NULL;
}

static void _XtMenuPopupAction(widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Boolean spring_loaded;
    register Widget popup_shell;

    if (*num_params != 1)
           XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		    "invalidParameters","xtMenuPopupAction","XtToolkitError",
           "MenuPopup wants exactly one argument",
	   (String *)NULL, (Cardinal *)NULL);

    if (event->type == ButtonPress) spring_loaded = True;
    else if (event->type == EnterNotify) spring_loaded = False;
    else {
	XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		"invalidPopup","unsupportedOperation","XtToolkitError",
"Pop-up menu creation is only supported on ButtonPress or EnterNotify events.",
                  (String *)NULL, (Cardinal *)NULL);
	spring_loaded = False;
    }

    popup_shell = _XtFindPopup(widget, params[0]);
    if (popup_shell == NULL)
            XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		    "invalidPopup","xtMenuPopup","XtToolkitError",
                   "Can't find popup in _XtMenuPopup",
		   (String *)NULL, (Cardinal *)NULL);

    if (spring_loaded) _XtPopup(popup_shell, XtGrabExclusive, TRUE);
    else _XtPopup(popup_shell, XtGrabNonexclusive, FALSE);
}


/*ARGSUSED*/
static void _XtMenuPopdownAction(widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Widget popup_shell;

    if (*num_params == 0) {
	XtPopdown(widget);
    } else if (*num_params == 1) {
	popup_shell = _XtFindPopup(widget, params[0]);
	if (popup_shell == NULL)
            XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		    "invalidPopup","xtMenuPopup","XtToolkitError",
                   "Can't find popup in _XtMenuPopup",
		   (String *)NULL, (Cardinal *)NULL);
	    XtPopdown(popup_shell);
    } else {
	XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		"invalidParameters","xtmenuPopdown","XtToolkitError",
               "XtMenuPopdown called with num_params != 0 or 1",
	       (String *)NULL, (Cardinal *)NULL);
    }
}


void _XtRegisterGrabs(widget,tm)
    Widget widget;
    XtTM  tm;
{
    XtTranslations stateTable=tm->translations;
    unsigned int count;

    if (! XtIsRealized(widget)) return;

    /* walk the widget instance action bindings table looking for */
    /* _XtMenuPopupAction */
    /* when you find one, do a grab on the triggering event */

    if (stateTable == NULL) return;
    for (count=0; count < stateTable->numQuarks; count++) {
       if (tm->proc_table[count] ==
            (XtActionProc)(_XtMenuPopupAction)) {
	    register StatePtr state;
	    /* we've found a "grabber" in the action table. Find the */
	    /* states that call this action. */
	    /* note that if there is more than one "grabber" in the action */
	    /* table, we end up searching all of the states multiple times. */
	    for (state=stateTable->head; state != NULL; state=state->forw) {
		register ActionPtr action;
	        for (
		    action = state->actions;
		    action != NULL;
		    action=action->next) {
		    if (action->index == count) {
			/* this action is a "grabber" */
			register Event *event;
			event = &stateTable->eventObjTbl[state->index].event;
			switch (event->eventType) {
			    case ButtonPress:
			    case ButtonRelease:
				XGrabButton(
				    XtDisplay(widget),
				    (unsigned) event->eventCode,
				    (unsigned) event->modifiers,
				    XtWindow(widget),
				    TRUE,
				    NULL,
				    GrabModeAsync,
				    GrabModeAsync,
				    None,
				    None
				);
				break;
	    
			    case KeyPress:
			    case KeyRelease:
				XGrabKey(
				    XtDisplay(widget),
				    (int) event->eventCode,
				    (unsigned) event->modifiers,
				    XtWindow(widget),
				    TRUE,
				    GrabModeAsync,
				    GrabModeAsync
				);
				break;
	    
			    default:
              XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		    "invalidPopup","unsupportedOperation","XtToolkitError",
"Pop-up menu creation is only supported on ButtonPress or EnterNotify events.",
                  (String *)NULL, (Cardinal *)NULL);
			    break;
			}
		    }
		}
	    }
	}
    }
}

static XtActionsRec tmActions[] = {
    {"MenuPopup", _XtMenuPopupAction},
    {"MenuPopdown", _XtMenuPopdownAction},
};


void _XtPopupInitialize() { XtAddActions(tmActions, XtNumber(tmActions)); }

ModToKeysymTable *_XtBuildModsToKeysymTable(dpy,pd)
    Display *dpy;
    XtPerDisplay pd;
{
    ModToKeysymTable *table;
    int maxCount,i,j,k,tempCount,index;
    KeySym keysym,tempKeysym;
    XModifierKeymap* modKeymap;
    KeyCode keycode;
#define KeysymTableSize 16
    pd->modKeysyms = (KeySym*)XtMalloc(KeysymTableSize*sizeof(KeySym));
    maxCount = KeysymTableSize;
    tempCount = 0;


    table = (ModToKeysymTable*)XtMalloc(8*sizeof(ModToKeysymTable));

    table[0].mask = ShiftMask;
    table[1].mask = LockMask;
    table[2].mask = ControlMask;
    table[3].mask = Mod1Mask;
    table[4].mask = Mod2Mask;
    table[5].mask = Mod3Mask;
    table[6].mask = Mod4Mask;
    table[7].mask = Mod5Mask;
    tempKeysym = 0;

    modKeymap = XGetModifierMapping(dpy);
    for (i=0;i<8;i++) {
        table[i].index = tempCount;
        table[i].count = 0;
        for (j=0;j<modKeymap->max_keypermod;j++) {
            keycode = modKeymap->modifiermap[i*modKeymap->max_keypermod+j];
            if (keycode != 0) {
                for (k=0; k<pd->keysyms_per_keycode;k++) {
                    index = ((keycode-dpy->min_keycode)*
                             pd->keysyms_per_keycode)+k;
                    keysym = pd->keysyms[index];
                    if (keysym != 0 && keysym != tempKeysym ){
                        if (tempCount==maxCount) {
                            maxCount += KeysymTableSize;
                            pd->modKeysyms = (KeySym*)XtRealloc(
                                (char*)pd->modKeysyms,
                                (unsigned) (maxCount*sizeof(KeySym)) );
                        }
                        pd->modKeysyms[tempCount++] = keysym;
                        table[i].count++;
                        tempKeysym = keysym;
                    }
                }
            }
        }
    }
    return table;

}


void _XtBuildKeysymTable(dpy,pd)
    Display* dpy;
    XtPerDisplay pd;
{
    int count;
    KeySym lower_return, upper_return,nbd,*bd;

    count = dpy->max_keycode-dpy->min_keycode+1;
    pd->keysyms = XGetKeyboardMapping(
        dpy,dpy->min_keycode,count,&pd->keysyms_per_keycode);
    if (pd->keysyms_per_keycode > 1)
        nbd = (dpy->max_keycode - dpy->min_keycode + 1) 
            * pd->keysyms_per_keycode;
        for (bd = pd->keysyms; bd < (pd->keysyms + nbd);
             bd += pd->keysyms_per_keycode) {
            if ((*(bd+1)) == NoSymbol) {
                XtConvertCase(dpy,*bd, &lower_return, &upper_return);
                *bd = lower_return;
                *(bd+1) = upper_return;                
            }
       }
}

void XtTranslateKeycode (dpy, keycode, modifiers,
                            modifiers_return, keysym_return)

    Display *dpy;
    KeyCode keycode;
    Modifiers modifiers;
    Modifiers *modifiers_return;
    KeySym *keysym_return;

{
    XtPerDisplay perDisplay;
    perDisplay = _XtGetPerDisplay(dpy);
    if (perDisplay != NULL && perDisplay->defaultKeycodeTranslator != NULL)
        (*perDisplay->defaultKeycodeTranslator)(
            dpy,keycode,modifiers,modifiers_return,keysym_return);
}
KeySym _XtKeyCodeToKeySym(dpy,pd,keycode,col)
    Display* dpy;
    XtPerDisplay pd;
    KeyCode keycode;
    int col;
{
/* copied from Xlib */
    int ind;
     if (pd->keysyms == NULL) {
	 _XtBuildKeysymTable(dpy,pd); /* pd->keysyms*/
     }
     if (col < 0 || col >= pd->keysyms_per_keycode) return (NoSymbol);
     if (keycode < dpy->min_keycode || keycode > dpy->max_keycode)
       return(NoSymbol);

     ind = (keycode - dpy->min_keycode) * pd->keysyms_per_keycode + col;
     return (pd->keysyms[ind]);
}







void XtTranslateKey(dpy, keycode, modifiers,
                            modifiers_return, keysym_return)
    Display *dpy;
    KeyCode keycode;
    Modifiers modifiers;
    Modifiers *modifiers_return;
    KeySym *keysym_return;

{
    XtPerDisplay perDisplay;
    perDisplay = _XtGetPerDisplay(dpy);
    *modifiers_return = StandardMask;
    if ((modifiers & StandardMask) == 0)
        *keysym_return =_XtKeyCodeToKeySym(dpy,perDisplay,keycode,0);
    else if ((modifiers & (ShiftMask | LockMask)) != 0)
	*keysym_return =_XtKeyCodeToKeySym(dpy,perDisplay,keycode,1);
    else
	*keysym_return = NoSymbol;
}

void XtSetKeyTranslator(dpy, translator)

    Display *dpy;
    XtKeyProc translator;

{
    XtPerDisplay perDisplay;
    perDisplay = _XtGetPerDisplay(dpy);
    if (perDisplay != NULL) 
      perDisplay->defaultKeycodeTranslator = translator;
}

/* ARGSUSED */
void XtRegisterCaseConverter(dpy, proc, start, stop)

    Display *dpy;
    XtCaseProc proc;
    KeySym start;
    KeySym stop;

{
    XtPerDisplay perDisplay;
    perDisplay = _XtGetPerDisplay(dpy);
    if (perDisplay != NULL)
         perDisplay->defaultCaseConverter = proc;
}
/* ARGSUSED */
void _XtConvertCase(dpy, keysym, lower_return, upper_return)

    Display *dpy;
    KeySym keysym;
    KeySym *lower_return;
    KeySym *upper_return;

{
    if ((keysym >= XK_a && keysym <= XK_z) ||
       (keysym >= XK_ssharp && keysym <= XK_odiaeresis) || 
       (keysym >= XK_oslash && keysym <= XK_ydiaeresis)) {
       *lower_return = keysym;
       *upper_return = keysym-0x20;
       return;
    }
    if ((keysym >= XK_A && keysym <= XK_Z) ||
         (keysym >= XK_Agrave && keysym <= XK_Odiaeresis) || 
         (keysym >= XK_Ooblique && keysym <= XK_Thorn)) {
        *upper_return = keysym;
        *lower_return = keysym+0x20;
        return;
        }
    *lower_return = keysym;
    *upper_return = keysym;

}
