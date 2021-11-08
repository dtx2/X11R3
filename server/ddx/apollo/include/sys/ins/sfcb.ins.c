/*  SFCB.INS.C, /SYS/INS, GLW, 10/26/85 */

/*  Extensible streams shared file control block allocator  */

/*
    CHANGES
        10/13/86 mishkin added sfcb_$hash_lock_stuck status
        02/13/86 mishkin remove nested include of mutex.ins.c
        02/06/86 mishkin correct use_count to be short, not int
        10/26/85 glw    original coding.  Adapted from /us/ins/sfcb.ins.pas
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

#define sfcb_$modc            0x01290000
#define sfcb_$lock_stuck      0x01290001
#define sfcb_$no_space        0x01290002
#define sfcb_$bad_free        0x01290003
#define sfcb_$hash_lock_stuck 0x01290004

typedef struct {
            uid_$t            tuid;
            xoid_$t           xoid;
            short             use_count;
            mutex_$lock_rec_t slock;
            struct sfcb_$header_t    *link
        } sfcb_$header_t;     

typedef sfcb_$header_t *sfcb_$ptr_t;


/*
    G E T 

    Called by type manager to obtain shared storage for an object whose
    use needs to be coordinated across processes.  Hashes the type_uid
    and the obj_xoid to locate an existing sfcb for this object, if any.
    If found, its use_count is incremented.  If not found, a new one is
    created, with a TOTAL size (including the header part) as indicated
    by the size parameter, all of the IN arguments are stored in it, and
    its use count is set to one.  In any case it is returned with the
    lock set.
*/

std_$call void sfcb_$get() ;

/*
    F R E E 

    Called, with the lock already set, by type managers' close operation
    (typically).  Decrements the use_count.  If it reaches zero, the sfcb
    is removed from the hash table, unlocked, and released.  Otherwise, it
    is simply unlocked.
*/

std_$call void sfcb_$free() ;

