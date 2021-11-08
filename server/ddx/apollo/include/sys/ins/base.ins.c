/*  BASE.INS.C, /sys/ins, ers, 04/02/86
    base include file for customer use

   Changes:                            
      05/20/86 mishkin added ios_$id_t, ios_$max, and ios_$seek_key_t.
      04/02/86 joelm added proc1_$n_user_processes
      02/05/86 leduc Fixed declaration of status_$ok to be 0L instead of 0.
      12/06/85 mishkin Added changes for IOS.
      03/18/85 RpS  Added declarations for set-manipulation library calls.
      09/04/84 knw  The "Pascal-ish" boolean true value should be FF, not FFFF.
      03/26/84 jrw  spelling corrections.
      09/15/83 ems  removed async field from status_$t
      08/23/83 knw  Changed the definition of "binteger" to be "unsigned short",
                    instead of "unsigned char".  Altho a binteger implies an 8-
                    bit value, Pascal really stores them in 16 bit integers,
                    EXCEPT in a PACKED RECORD.  Most simple usages of binteger
                    should allocate 16 bits in C.
      11/30/82 jrw  added true, false
      09/28/82 ers  original coding */

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

/*  Pascal-ish booleans:  */

#define false           ((unsigned char) 00)
#define true            ((unsigned char) 0xff)

typedef unsigned char boolean;

/*  System-defined stream id's:  */

#define stream_$stdin   0
#define stream_$stdout  1
#define stream_$errin   2
#define stream_$errout  3

#define ios_$stdin   0
#define ios_$stdout  1
#define ios_$errin   2
#define ios_$errout  3



/*  Common datatypes:  A note about the "binteger" definition:  In Pascal, the
 *                     binteger definition is 0..255; a subrange of integer.
 *                     Although the definition implies an 8-bit datum, Pascal
 *                     allocates a 16-bit integer for it in all cases EXCEPT for
 *                     Pascal "PACKED RECORDS".  Only in that case are only 8 bits
 *                     allocated.  So, most simple C usages for Pascal interfaces
 *                     must also allocate 16 bits.  To make a C structure which
 *                     mimics a Pascal PACKED RECORD, use bit fields.
 */

typedef unsigned short binteger;       /* positive 8 bit integer */
typedef unsigned short pinteger;       /* positive 16 bit integer */
typedef unsigned int linteger;         /* positive 31 bit integer */



/*  Common status datatype returned by most Apollo-supplied procedures:  */

typedef union {
    struct {
        unsigned fail : 1,          /* true if module couldn't handle error */
                 subsys : 7,        /* subsystem code */
                 modc : 8;          /* module code */
        short    code;              /* module specific error */
    } s;
    long all;                       /* used for testing for specific value */
} status_$t;

#define status_$ok  0L              /* returned if called proc successful */


/*  System clock types:  */

typedef unsigned int time_$clockh_t;    /* high 32 bits of time_$clock */

typedef union {
    struct {
        time_$clockh_t high;
        pinteger       low;
    } c1;
    struct {
        pinteger high16;
        unsigned int low32;
    } c2;
} time_$clock_t;


/*  Number of user processes */
 
#define proc1_$n_user_processes 56


/*  Commonly used naming server constants and types:  */

#define name_$pnamlen_max 256       /* max length of pathname */
#define name_$complen_max  32       /* max length of entry name */

typedef char name_$pname_t[name_$pnamlen_max];
typedef char name_$name_t[name_$complen_max];



/*  Commonly used streams types:  */

#define ios_$max  127

typedef short ios_$id_t;            /* open stream identifier */
typedef struct {
        linteger rec_adr;
        linteger byte_adr;
} ios_$seek_key_t;

typedef short stream_$id_t;         /* open stream identifier (same as ios_$id_t) */
typedef union {
    long offset;
    struct {
        linteger rec_adr;
        linteger byte_adr;
        linteger flags;
    } key;
} stream_$sk_t;                     /* seek key returned on most stream calls */

typedef struct {
        long high;
        long low;
} uid_$t;                           /* for type uids returned by streams */

typedef struct {
        long rfu1;
        long rfu2;
        uid_$t uid;
} xoid_$t;

/*  User eventcount definitions */

typedef struct {
        long value;                 /* current ec value */
        pinteger awaiters;          /* first process waiting */
} ec2_$eventcount_t;

typedef ec2_$eventcount_t *ec2_$ptr_t;

extern uid_$t uid_$nil;

/*  Library calls for set manipulation */

std_$call void lib_$init_set();             /* initialize a set */
std_$call void lib_$add_to_set();           /* add an element to a set */
std_$call void lib_$clr_from_set();         /* remove an element from a set */
std_$call boolean lib_$member_of_set();     /* return membership of an element in a set */

#eject
