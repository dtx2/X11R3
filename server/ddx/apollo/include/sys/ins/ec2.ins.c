/* EC2.INS.C, /SYS/INS, ers, 09/14/83
   Level 2 Eventcount Routines 

   Changes :
      04/11/86 lauer Added ec2_$no_more_ecs, ec2_$ec1_not_allocated
      12/06/85 mishkin added ec2_$always_ready_ec.
      09/14/83 ems  added ec2_$wait_svc 
      09/29/82 ers  original code */

    /* error status codes */

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

#define ec2_$no_wait_entries    0x00180001      /* internal table exhausted */
#define ec2_$internal_error     0x00180002      /* woke up but no ecs satisfied */
#define ec2_$wait_quit          0x00180003      /* process quit while waiting */
#define ec2_$bad_eventcount     0x00180004      /* bad eventcount passed in */
#define ec2_$no_more_ecs        0x00180005      /* unable to allocate level 1 eventcount */
#define ec2_$ec1_not_allocated  0x00180006      /* level 1 eventcount not allocated */


typedef ec2_$ptr_t ec2_$ptr_list_t[2];
typedef long ec2_$val_list_t[2];

#define ec2_$always_ready_ec    1

std_$call void ec2_$init();
std_$call long ec2_$read();
std_$call pinteger ec2_$wait();
std_$call pinteger ec2_$wait_svc();
std_$call void ec2_$advance();
