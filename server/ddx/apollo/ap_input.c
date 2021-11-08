/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.

                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/*
 * Functions implementing Apollo-device-independent parts of the driver
 * having to do with input handling.
 */
                    
#include "apollo.h"

#include "sys/ins/tone.ins.c"

/*
 * Pointers to the DeviceRec's for the keyboard and pointer.
 */
DevicePtr       apKeyboard;
DevicePtr       apPointer;

/*
 * Declare lastEventTime here (also used by cursor code).
 */
int             lastEventTime;

/*
 * File descriptor for both keyboard and pointer to give to OS code.
 */
static int      fdApollo = 0;

/*
 * List of externs for the GPR type manager routines.
 */
extern int      MakeGPRStream ();
extern Bool     GetGPREvent ();

/*
 * List of externs for the different versions of the keyboard routines.
 */
extern void     apK2InitAndGetMappings();
#ifdef SWITCHER
extern void     apK2HWInitKbd();
#endif
extern void     apK2HandleKey();
extern void     apK2LegalModifier();

/*
 * List of externs into the cursor code.
 */
extern void     apConstrainPointer();
extern void     apResetPointer();
extern void     apRestoreCursor();
extern void     apMoveCursor();

/*
 * LegalModifier -- DDX interface (server)
 *      Predicate to say whether a given key code is a legal modifier
 *      (e.g. shift, control, ...).
 */
Bool
LegalModifier(key)
    BYTE key;
{
    apPrivKbdPtr    pPrivK;

    pPrivK = (apPrivKbdPtr) apKeyboard->devicePrivate;
    return ( (*(pPrivK->LegalModifierPtr))(key) );
}

/*
 * apBell -- DDX interface (device)
 *      Ring the bell on the given device.
 */
static void
apBell(loud, pDevice)
    DevicePtr pDevice;
    int loud;
{
/*
 *  No volume control
 */
    if (loud > 0)
        tone_$time(((apPrivKbdPtr)(pDevice->devicePrivate))->beepTime);
}

/*
 * apChangeKeyboardControl -- DDX interface (device)
 *      Attempt to set various possible keyboard attributes, most of which
 *      don't exist on Apollos.
 */
static void
apChangeKeyboardControl(pDevice, ctrl)
    DevicePtr pDevice;
    KeybdCtrl *ctrl;
{
    apPrivKbdPtr    pPrivK;

    pPrivK = (apPrivKbdPtr) pDevice->devicePrivate;
/*
 *  No keyclick, bell pitch control, LEDs or autorepeat control
 */
    pPrivK->beepTime.high16 = 0;
    pPrivK->beepTime.low32 = 250 * ctrl->bell_duration;
}

/*
 * apGetMotionEvents -- DDX interface (device)
 *      Return the contents of the mythical motion event buffer.
 */
static int
apGetMotionEvents(buff, start, stop)
    CARD32      start, stop;
    xTimecoord  *buff;
{
/*
 *  No motion buffer
 */
    return 0;
}

/*
 * apChangePointerControl -- DDX interface (device)
 *      Attempt to set various possible pointer attributes.
 *      We don't.
 */
static void
apChangePointerControl(pDevice, ctrl)
    DevicePtr pDevice;
    PtrCtrl   *ctrl;
{
/*
 *  Not implementing mouse threshhold or acceleration factor
 */
}

/*
 * HWInitPointr -- Driver internal code
 *      Perform all "hardware" operations needed to initialize pointer.
 */
static void
HWInitPointr()
{
    gpr_$keyset_t       keyset;
    status_$t           status;

    lib_$init_set (keyset, 256);
    lib_$add_to_set (keyset, 256, 'A');
    lib_$add_to_set (keyset, 256, 'B');
    lib_$add_to_set (keyset, 256, 'C');
    lib_$add_to_set (keyset, 256, 'a');
    lib_$add_to_set (keyset, 256, 'b');
    lib_$add_to_set (keyset, 256, 'c');
    gpr_$enable_input (gpr_$buttons, keyset, status);
    gpr_$enable_input (gpr_$locator_stop, keyset, status);
    gpr_$enable_input (gpr_$locator_update, keyset, status);
}

/*
 * apMouseProc -- DDX interface (device)
 *      Code to initialize, terminate, turn on and turn off the mouse device.
 */
int
apMouseProc(pDev, onoff, argc, argv)
    DevicePtr   pDev;
    int         onoff;
    int         argc;
    char        *argv[];
{
    BYTE                map[4];
    apPrivPointrPtr     pPrivP;
    status_$t           status;

    switch (onoff)
    {
        case DEVICE_INIT: 
            apPointer = pDev;
            pPrivP = (apPrivPointrPtr) Xalloc (sizeof(apPrivPointrRec));
            apPointer->devicePrivate = (pointer) pPrivP;

            pPrivP->numCurScreen = -1;
            pPrivP->pCurCursor = NullCursor;
            pPrivP->hotX = pPrivP->hotY = 0;
            pPrivP->x = pPrivP->y = 0;
            pPrivP->constraintBox.x1 = pPrivP->constraintBox.y1 = 0;
            pPrivP->constraintBox.x2 = pPrivP->constraintBox.y2 = 32767;
            pPrivP->nonInterestBox.x1 = pPrivP->nonInterestBox.y1 = 0;
            pPrivP->nonInterestBox.x2 = pPrivP->nonInterestBox.y2 = 0;

            HWInitPointr();
#ifdef SWITCHER
            pPrivP->apHWInitPointr = HWInitPointr;
#endif
            pDev->on = FALSE;
            map[1] = 1;
            map[2] = 2;
            map[3] = 3;

            InitPointerDeviceStruct(
                apPointer, map, 3, apGetMotionEvents, apChangePointerControl);

            if (!fdApollo)
                fdApollo = MakeGPRStream();
            break;
        case DEVICE_ON: 
            pDev->on = TRUE;
            AddEnabledDevice(fdApollo);
            break;
        case DEVICE_OFF: 
            pDev->on = FALSE;
            break;
        case DEVICE_CLOSE: 
            Xfree (pDev->devicePrivate);
            gpr_$disable_input (gpr_$buttons, status);
            gpr_$disable_input (gpr_$locator_stop, status);
            gpr_$disable_input (gpr_$locator_update, status);
            break;
    }
    return Success;

}

/*
 * apKeybdProc -- DDX interface (device)
 *      Code to initialize, terminate, turn on and turn off the keyboard device.
 */
int
apKeybdProc(pDev, onoff, argc, argv)
    DevicePtr   pDev;
    int         onoff;
    int         argc;
    char        *argv[];
{
    BYTE            map[MAP_LENGTH];
    KeySymsRec      keySyms;
    CARD8           modMap[MAP_LENGTH];
    status_$t       status;
    apPrivKbdPtr    pPrivK;

    switch (onoff)
    {
        case DEVICE_INIT: 
            apKeyboard = pDev;
            apKeyboard->devicePrivate = (pointer) Xalloc (sizeof(apPrivKbdRec));
            pPrivK = (apPrivKbdPtr) apKeyboard->devicePrivate;

            pPrivK->beepTime.high16 = 0;
            pPrivK->beepTime.low32 = 250*250;    /* 1/4 second */

/*
 *  Here is where we must eventually detect which kind of keyboard it is
 *  and call a different initializer depending on which one.
 */

            pPrivK->LegalModifierPtr = apK2LegalModifier;
            pPrivK->HandleKeyPtr = apK2HandleKey;
            apK2InitAndGetMappings (&keySyms, modMap);
#ifdef SWITCHER
            pPrivK->apHWInitKbd = apK2HWInitKbd;
#endif

            pDev->on = FALSE;
            InitKeyboardDeviceStruct(
                    apKeyboard, &keySyms, modMap, apBell,
                    apChangeKeyboardControl);
            Xfree(keySyms.map);

            if (!fdApollo)
                fdApollo = MakeGPRStream();
            break;
        case DEVICE_ON: 
            pDev->on = TRUE;
            AddEnabledDevice(fdApollo);
            break;
        case DEVICE_OFF: 
            pDev->on = FALSE;
            break;
        case DEVICE_CLOSE: 
            Xfree (pDev->devicePrivate);
            gpr_$disable_input(gpr_$keystroke, status);
            break;
    }
    return Success;
}

/*
 * handleButton -- Driver internal code
 *      Translate the Apollo event data into an X event, and make it happen.
 */
static void
handleButton (xEp)
    xEvent      *xEp;
{
    if (apEventData[0] < 'a')
    {
        xEp->u.u.type = ButtonRelease;
        xEp->u.u.detail = apEventData[0] - ('A' - 1);
    }
    else
    {
        xEp->u.u.type = ButtonPress;
        xEp->u.u.detail = apEventData[0] - ('a' - 1);
    }
    (*apPointer->processInputProc) (xEp, apPointer);
}

/*
 * We cheat by looking into this DIX (window.c) variable to see if
 * we should unsave the screen.
 */
extern int screenIsSaved;

/*
 * ProcessInputEvents -- DDX interface (server)
 *      While wasting as little time as possible if no events exist,
 *      get and process all pending input from the Apollo devices.
 */
void
ProcessInputEvents()
{
    xEvent          x;
    int             timeNow = 0;
    status_$t       status;
    apPrivKbdPtr    pPrivK;
    apPrivPointrPtr pPrivP;
    int             newx, newy;

    while (GetGPREvent(TRUE, TRUE))
    {
        if (screenIsSaved == SCREEN_SAVER_ON)
            SaveScreens (SCREEN_SAVER_OFF, ScreenSaverReset);
/*
 *  Since GPR events don't come with time stamps, we have to make up our
 *  own.  We do so by starting at "now" (the time this routine was
 *  called), and adding one millisecond to the time for each subsequent
 *  event.  We take some trouble to avoid time going backwards.
 *  Of course, time stamping at the time of dequeueing is wrong, but ....
 */
        if (!timeNow)
        {
            timeNow = GetTimeInMillis();
            if (timeNow <= lastEventTime) timeNow = lastEventTime + 1;
        }
        else
            timeNow++;

        pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;
        pPrivP->x = apEventPosition.x_coord;
        pPrivP->y = apEventPosition.y_coord;
        apConstrainPointer(pPrivP, &pPrivP->x, &pPrivP->y);

        x.u.keyButtonPointer.time = lastEventTime = timeNow;
        x.u.keyButtonPointer.rootX = apEventPosition.x_coord;
        x.u.keyButtonPointer.rootY = apEventPosition.y_coord;

        switch (apEventType)
        {
            case gpr_$locator_stop:
            case gpr_$locator_update:
                apMoveCursor();
                x.u.u.type = MotionNotify;
                (*apPointer->processInputProc) (&x, apPointer);
                break;
            case gpr_$buttons:
                handleButton (&x);
                break;
            case gpr_$keystroke:
                pPrivK = (apPrivKbdPtr) apKeyboard->devicePrivate;
                (*(pPrivK->HandleKeyPtr)) (&x);
                break;
            default:
                break;
        }
    }

    apRestoreCursor();
}

/*
 * apSaveScreen -- DDX interface (screen)
 *      Control screen saver.  Actually just let DIX do it.
 */
Bool
apSaveScreen(pScreen, on)
    ScreenPtr pScreen;
    int on;
{
    if (on == SCREEN_SAVER_FORCER)
    {
        lastEventTime = GetTimeInMillis();
        return TRUE;
    }
    else
        return FALSE;
}

/*
 * TimeSinceLastInputEvent -- DDX interface (server)
 *      Let the OS code know how long since last input, so the server can
 *      wake up when the screen saver is supposed to go off.
 */
int
TimeSinceLastInputEvent()
{
    int timeNow;

    if (lastEventTime == 0)
        lastEventTime = GetTimeInMillis();
    timeNow = GetTimeInMillis();
    return ( max(0, (timeNow-lastEventTime)) );
}
