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
 * A rudimentary type manager for the keyboard and mouse.  The gpr_io_$get
 * operation is not used to actually get input.  It will only be called from
 * the DOMAIN/IX implementation of the select() call (which is made in
 * WaitForSomething), and only with the preview option.  This allows the
 * server to wake up when an event occurs.
 */

#include "apollo.h"

#include "sys/ins/ec2.ins.c"
#include "sys/ins/ios.ins.c"
#include "sys/ins/io_traits.h"
#include "sys/ins/trait.ins.c"

/*
 * The actual GPR event varibles go here.
 */
gpr_$event_t    apEventType;
unsigned char   apEventData[1];
gpr_$position_t apEventPosition;

/*
 * Pointer to the value field of the GPR eventcount,
 * and pointer to a cell which contains its last known value.
 */
long    *apECV;         /* points to value in GPR_ec */
long    *apLastECV;     /* points to lastGPR_ecValue */

static ec2_$ptr_t  GPR_ec;
static long        lastGPR_ecValue = -1;

extern gpr_$bitmap_desc_t 	LastGPRBitmap;

/*
 * GetGPREvent -- Driver internal code
 *      Try to find a GPR input event.
 *      If cond is TRUE, don't block; instead, return TRUE if we got one,
 *      FALSE if we didn't.  If cond is FALSE, block if necessary (and
 *      then return TRUE).
 *      If consume is TRUE, don't return the same event next time.
 *      If consume is FALSE, we must return the same event next time
 *      (i.e. we are previewing the event).
 */
Bool
GetGPREvent (cond, consume)
    Bool cond;
    Bool consume;
{
    static Bool needNewData = TRUE;

    long        newGPR_ecValue;
    status_$t   status;
    apDisplayDataPtr    pDisp;

    if (needNewData)
    {
        if (cond)
        {
            newGPR_ecValue = GPR_ec->value;
            if (lastGPR_ecValue == newGPR_ecValue)
                return (FALSE);
            else
            {
                gpr_$cond_event_wait (apEventType, apEventData[0], apEventPosition, status);
                if (apEventType == gpr_$no_event)
                {
                    lastGPR_ecValue = newGPR_ecValue;
                    return (FALSE);
                }
                /* unfortunately gpr_$cond_event_wait always forces the bitmap to be the
                   screen bitmap, even though we set it to something else in apcValidateGC.
                   If that happened, we must invalidate the GC */
                pDisp = &apDisplayData[((apPrivPointrPtr) apPointer->devicePrivate)->numCurScreen];
                if (pDisp->display_bitmap != LastGPRBitmap)
                {
                    gpr_$inq_bitmap( LastGPRBitmap, status );
                    if ( pDisp->lastGC )
                        pDisp->lastGC->serialNumber |= GC_CHANGE_SERIAL_BIT;
                }
            }
        }
        else
            gpr_$event_wait (apEventType, apEventData[0], apEventPosition, status);

        if (status.all != status_$ok)
            return (false);
    }

    needNewData = consume;
    return (true);
}

/*
 * gpr_io_$get -- Driver internal code
 *      Implement the get operation of the ios trait for the GPR type manager.
 */
static int
gpr_io_$get (handle_p_p, options_p, buffer_p, buffer_len_p, status_p)
    char                **handle_p_p;
    ios_$put_get_opts_t *options_p;
    char                *buffer_p;
    long                *buffer_len_p;
    status_$t           *status_p;
{
    if ((ios_$preview_opt & *options_p) == 0)
        FatalError ("Gack!  Not preview in gpr_io_$get\n");

    if (GetGPREvent ( ((ios_$cond_opt & *options_p) != 0), FALSE))
    {
        status_p->all = status_$ok;
        return (1);
    }
    else
    {
        status_p->all = ios_$get_conditional_failed;
        return (0);
    }
}

/*
 * gpr_io_$close -- Driver internal code
 *      Implement the close operation of the ios trait for the GPR type manager.
 */
static boolean
gpr_io_$close (handle_p_p, status_p)
    char        **handle_p_p;
    status_$t   *status_p;
{
    status_p->all = status_$ok;
    return (false);
}

/*
 * gpr_io_$inq_conn_flags -- Driver internal code
 *      Implement the inq_conn_flags operation of the ios trait for the GPR type manager.
 */
static ios_$conn_flag_set
gpr_io_$inq_conn_flags (handle_p_p, status_p)
    char        **handle_p_p;
    status_$t   *status_p;
{
    status_p->all = status_$ok;
    return (ios_$cf_tty_mask | ios_$cf_vt_mask);
}

/*
 * gpr_io_$get_ec -- Driver internal code
 *      Implement the get_ec operation of the ios trait for the GPR type manager.
 */
static void
gpr_io_$get_ec (handle_p_p, stream_key_p, ecp_p, status_p)
    char            **handle_p_p;
    ios_$ec_key_t   *stream_key_p;
    ec2_$ptr_t      *ecp_p;
    status_$t       *status_p;
{
    *ecp_p = GPR_ec;
    status_p->all = status_$ok;
}

/*
 * This is the entry point vector (EPV) implementing the ios trait for the GPR type manager.
 */
#define NULL_PROCEDURE_PTR 0
static io_$epv gpr_io_$epv = {
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        gpr_io_$close,
        gpr_io_$get_ec,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        gpr_io_$inq_conn_flags,
        NULL_PROCEDURE_PTR,
        gpr_io_$get,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR,
        NULL_PROCEDURE_PTR
        };

/*
 * MakeGPRStream -- Driver internal code
 *      Initialize the GPR type manager.
 */
int
MakeGPRStream ()
{
    static uid_$t   gpr_io_$uid = { 0x3527949C, 0xF0009BB1 };
    status_$t       status;

    gpr_$get_ec (gpr_$input_ec, GPR_ec, status);

    apECV = &(GPR_ec->value);
    apLastECV = &lastGPR_ecValue;

    trait_$mgr_dcl (gpr_io_$uid, io_$trait, trait_$kind_local, &gpr_io_$epv, status);
    return (ios_$connect ("", (short) 0, gpr_io_$uid, (long) 0, &gpr_io_$epv, status) );
}
