/* MUTEX.INS.C, /SYS/INS, dej, 05/13/85
   User mode mutex lock Routines

   Changes :
      05/13/85 jabs original code */

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

#define mutex_$wait_forever -1L

typedef struct {
    boolean  lock_byte;
    ec2_$eventcount_t lock_ec;
} mutex_$lock_rec_t;

std_$call boolean mutex_$lock();
std_$call void mutex_$unlock();
std_$call void mutex_$init();
