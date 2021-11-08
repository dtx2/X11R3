/* T R A I T - trait system calls and defns -- glw -- 10/24/85 */

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

/* CHANGES
    03/24/86 mishkin enum => short enum.
    02/06/86 mishkin Fix C coding errors.
    10/25/85 glw  Initial coding. Adapted from /sys/ins/trait.ins.pas

*/

#define trait_$subsys               0x13
#define trait_$modc                 0x01
#define trait_$module_code          0x13010000

#define trait_$too_many_dcls        0x13010001
#define trait_$no_rws               0x13010002

typedef void (*trait_$operation_proc_t)();

typedef struct {
            uid_$t  uid;
            trait_$operation_proc_t unimp;
        } op;

typedef struct { 
            short   op_cnt;
            uid_$t  trait_uid;
            op      ops[1];
        } trait_$t;

typedef short enum { 
              trait_$kind_local,                        /* use this epv for local objects */
              trait_$kind_near_remote,                  /* use this epv for remote objects on MY internet net */
              trait_$kind_far_remote,                   /* use this epv for remote objects outside MY internet net */
              trait_$kind_4,
              trait_$kind_5,
              trait_$kind_6,
              trait_$kind_7,
              trait_$kind_8,
              trait_$kind_9,
              trait_$kind_10,
              trait_$kind_11,
              trait_$kind_12,
              trait_$kind_13,
              trait_$kind_14,
              trait_$kind_15,
              trait_$kind_16,
              trait_$kind_17,
              trait_$kind_18,
              trait_$kind_19,
              trait_$kind_20,
              trait_$kind_21,
              trait_$kind_22,
              trait_$kind_24,
              trait_$kind_25,
              trait_$kind_26,
              trait_$kind_27,
              trait_$kind_28,
              trait_$kind_29,
              trait_$kind_30,
              trait_$kind_31,
              trait_$kind_32
            } trait_$kind_val_t;

typedef long int trait_$kind_t;

typedef char *trait_$epv;

/*
     T R A I T _ $ M G R _ D C L    
        associate an EPV with an object type UID
        and the circumstances under which that
        EPV should be used.
*/

std_$call void trait_$mgr_dcl();


