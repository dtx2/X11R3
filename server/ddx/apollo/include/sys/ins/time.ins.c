/*  TIME.INS.C  /sys/ins, ers, 09/29/82
   customer-callable timer routines

   Changes:
      04/02/86 lwa  Change enum to short enum.
      09/29/82 ers  original coding.
*/

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

#define time_$no_q_entry  0x000D0001  /* error from time_$advance */
#define time_$not_found   0x000D0002  /* entry to be canceled not found */
#define time_$wait_quit   0x000D0003  /* wait interrupted by quit fault */
#define time_$bad_int     0x000D0004  /* bad timer interrupt */
#define time_$bad_key     0x000D0005  /* bad key to time_$get_ec */

typedef short enum {time_$relative, time_$absolute} time_$rel_abs_t;

typedef short enum {time_$clockh_key} time_$key_t;

std_$call void time_$clock();  /* read the whole clock */
std_$call void time_$wait();
std_$call void time_$get_ec();

#eject
