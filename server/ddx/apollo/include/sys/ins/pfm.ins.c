/*  PFM.INS.C, /sys/ins, ers, 02/28/85
   process fault manager include file

   Changes:
      06/10/86 bpd  add space in pfm_$cleanup_rec 
      05/12/86 bpd  added new status codes
      04/02/86 lwa  Change enum to short enum.
      02/28/85 ers  Added pfm_$fh_opt_set_t decl.
      09/14/83 jrw  updated for sr7.
      11/10/81 ers  original coding. */

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


/*  Status codes:  */

#define pfm_$module_code            0x03040000
#define pfm_$bad_rls_order          (pfm_$module_code + 1)
#define pfm_$invalid_cleanup_rec    (pfm_$module_code + 2)
#define pfm_$cleanup_set            (pfm_$module_code + 3)
#define pfm_$cleanup_set_signalled  (pfm_$module_code + 4)
#define pfm_$no_space               (pfm_$module_code + 5)
#define pfm_$cleanup_not_found      (pfm_$module_code + 6)
#define pfm_$fh_not_found           (pfm_$module_code + 7)
#define pfm_$fh_wrong_level         (pfm_$module_code + 8)
#define pfm_$fork_not_found         (pfm_$module_code + 9)
#define pfm_$nl_goto_in_progress    (pfm_$module_code + 10)
#define pfm_$corrupted_stack        (pfm_$module_code + 11)


/*  General constants:  */

#define pfm_$all_faults     ((long) 0)


typedef struct {
        int pad[16];
} pfm_$cleanup_rec;

typedef char pfm_$name[32];

typedef struct {
        short pattern ;
        status_$t status ;
} pfm_$fault_rec_t ;


typedef short pfm_$fh_opt_set_t;

#define pfm_$fh_backstop  1             /* option "set" elements */
#define pfm_$fh_multi_level 2


typedef short enum {
        pfm_$continue_fault_handling,
        pfm_$return_to_faulting_code
} pfm_$fh_func_val_t ;

typedef char *pfm_$fh_handle_t ;

typedef pfm_$fh_func_val_t (*pfm_$fault_func_p_t) () ;


/* --------------------------------------------------------------------- */
/* C L E A N U P - eatablish a cleanup handler for faults */
/* returns pfm_$cleanup_set when handler is first established; returns
    status signalled by the fault when clenup is to be executed.
    The cleanup handler runs inhibited; the inhibit count is one
    greater in the handler than when the handler was established.
    After performing whatever cleanup is desired, the
    handler has several exit options:
        1) exit by calling pfm_$signal with same or different
            status.  The next cleanup handler (if any)
            will be executed.
        2) exit by calling pgm_$exit.  The next cleanup
            handler (if any) will be executed.
        3) exit by calling pfm_$enable, then returning to 
            its caller.  This does NOT restart at the point of the
            fault.
        4) re-establish the handler by calling pfm_$reset_cleanup,
            then continue execution.
*/
std_$call status_$t pfm_$cleanup();

/* --------------------------------------------------------------------- */
/* R E S E T _ C L E A N U P - reset cleanup handler */
/* Called from within a cleanup handler, this call causes
    a previously established cleanup handler to be re-established.
    A subsequent fault will cause another return from pfm_$cleanup with
    the new fault status. This call also increments the inhibit count by
    one, restoring the value in force when the handler was established.
*/
std_$call void pfm_$reset_cleanup();

/* --------------------------------------------------------------------- */
/* R L S _ C L E A N U P - pop the top of the cleanup handler stack */
 /* this record may not be the top of the stack, because a descendant
    of the caller established a cleanup handler and didn't release
    it.  This is OK; we release all handlers below this one as well.
    However, we return a status on it. */

std_$call void pfm_$rls_cleanup();

/* --------------------------------------------------------------------- */
/* S I G N A L - exit from a program, invoking cleanup as it goes */
std_$call void pfm_$signal();

/* --------------------------------------------------------------------- */
/* I N H I B I T- inhibit asyncronous faults */
std_$call void pfm_$inhibit();

/* --------------------------------------------------------------------- */
/* E N A B L E - enable asynchronous faults */
std_$call void pfm_$enable();

/* -------------------------------------------------------------------- */
/* E R R O R _ T R A P - simulate a fault */
std_$call void pfm_$error_trap();

/* -------------------------------------------------------------------- */
/* E S T A B L I S H _ F A U L T _ H A N D L E R - make a fault handler */
std_$call pfm_$fh_handle_t pfm_$establish_fault_handler () ;

/* -------------------------------------------------------------------- */
/* R E L E A S E _ F A U L T _ H A N D L E R - release a fault handler  */
std_$call void pfm_$release_fault_handler () ;

