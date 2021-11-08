/*  TODM -- Send the X server a message (actually a bogus keystroke) to make it release the display,
            return to the DM, and wait for the restart message (from tox) */

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


#include "sys/ins/base.ins.c"
#include "sys/ins/time.ins.c"

typedef struct {
	unsigned x : 16,
		 y : 16;
} idm_$position_t;

typedef struct {
	time_$clock_t timestamp;
	linteger locus_ID;
	idm_$position_t position;
	linteger ev_type;
	idm_$position_t data;
} idm_$event_t;

std_$call void idm_$process_event();

main()
{
    idm_$event_t event;
    status_$t status;

    time_$clock( event.timestamp );
    event.locus_ID = 0;
    event.position.x = 500;
    event.position.y = 500;
    event.ev_type = 0;
    event.ev_type = 0x00000afd;
    event.data.x = 0;
    event.data.y = 0;
    idm_$process_event( (long)0, event, status );
}
