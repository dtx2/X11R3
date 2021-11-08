/*  IO_TRAITS.INS.C,    /sys/ins, GLW, 10/25/85     */

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
    Defines the traits (i.e. trait EPVs) that are used to interface the I/O 
    Switch (IOS) to file/device managers 

*/

typedef struct {
             void (*export)() ;          /* Called in parent process before pgm_$invoke of new process   */
             void (*import)() ;          /* Called in child process after pgm_$invoke of new process     */
             void (*pre_fork)() ;        /* Called in parent process before fork                         */
             void (*post_fork)() ;       /* Called in child process after fork                           */
             boolean (*close)() ;        /* True => file to be deleted sp drop name                      */
             void (*get_ec)();   
             ios_$mgr_flag_set (*inq_mgr_flags)();    
             ios_$obj_flag_set (*inq_obj_flags)();
             void (*set_obj_flag)();
             ios_$conn_flag_set (*inq_conn_flags)();
             void (*set_conn_flag)();
             long int (*get)();
             long int (*locate)();
             void (*put)();
             long int (*inq_rec_remainder)();
             void (*seek)() ;
             void (*seek_full_key)() ;
             void (*seek_short_key)() ;
             void (*seek_to_bof)() ;
             void (*seek_to_eof)() ;
             unsigned long int (*inq_short_key)() ;
             void (*inq_full_key)();
             long int (*inq_rec_pos)();
             long int  (*inq_byte_pos)() ;
             void (*truncate)() ;
             long int (*inq_cur_rec_len)() ;
             ios_$rtype_t (*inq_rec_type)() ;
             void (*set_rec_type)() ;
             void (*force_write_file)() ;
             void (*inq_file_attr)() ;
             boolean (*equal)() ;
        } io_$epv;

typedef io_$epv *io_$epv_ptr;

extern int io_$trait ;


/* _____________________________________________________________________________________ */

/*
    IO_OC trait.  This trait is used for the opening and creation of
    normal Apollo object types that do not involve any naming funny
    business.  As long as this trait is used, the I/O Switch will handle
    all of the name_$ operations necessary, and the manager can be
    completely independent of naming.  

*/
   
typedef struct {
             void (*open)() ;
             void (*initialize)() ;
        } io_oc_$epv;

typedef io_oc_$epv *io_oc_$epv_ptr;

extern int io_oc_$trait ;

/* _____________________________________________________________________________________ */

/*
    IO_XOC trait.  This trait is used for the opening and creation
    of objects that require extended naming capability.              

    Three notes:

        1. It is assumed that all pathname processing aside from the
           name_$resolve_afayc that gets us here is done by the manager.
           The switch will do nothing.

        2. Streams that are opened/created by these calls have no
           associated UID, although they do have a type (so they can
           subsequently trait_$bind against auxiliary traits).
           ios_$inq_obj_uid will respond with uid_$nil <<is this OK?>>.

        3. The type-change operations are not defined in this trait.  They
           could be, of course, but they would certainly be different from
           those in the iooc trait.

*/

typedef short enum {                /* For the inq_name operation                                     */
            io_xoc_$resid_name,         /* Return the whole residual                                  */
            io_xoc_$leaf_name           /* Return the leaf part of the residual (if that makes sense) */
        } io_xoc_$name_type_t;

typedef struct {
             void (*open)() ;
             void (*create)() ;
             void (*change_name)() ;
             void (*inq_name)() ;
        } io_xoc_$epv;

typedef io_xoc_$epv *io_xoc_$epv_ptr;

extern int io_xoc_$trait;

std_$call ios_$id_t ios_$connect() ;
