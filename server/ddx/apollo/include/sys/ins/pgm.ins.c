/* pgm.ins.c, /sys/ins, ers, 09/29/82
   program manager insert file for C 

   changes:
    07/23/86 douros add pgm_$mode_default
    02/28/86 gms added not_a_program, process_vforked status.
    01/16/84 cas changed value of pgm_$wait from 0 to 1
    09/29/82 ers original coding. */

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


#define pgm_$module_code     0x03050000
#define pgm_$no_arg          (pgm_$module_code+0x1)
#define pgm_$arg_too_big     (pgm_$module_code+0x2)
#define pgm_$bad_connv       (pgm_$module_code+0x3)
#define pgm_$not_a_program   (pgm_$module_code+0x4)
#define pgm_$process_vforked (pgm_$module_code+0x5)

#define pgm_$max_severity    15               /* highest severity level */
#define pgm_$ok              0
#define pgm_$true            0
#define pgm_$false           1
#define pgm_$warning         2
#define pgm_$error           3
#define pgm_$output_invalid  4
#define pgm_$internal_fatal  5
#define pgm_$program_faulted 6

typedef short pgm_$mode;

#define pgm_$wait 1
#define pgm_$back_ground 2

typedef char pgm_$name[128];

typedef struct {
    short len;
    char chars[128];
} pgm_$arg;

typedef int *pgm_$argv[128];
typedef pgm_$argv *pgm_$argv_ptr;
typedef stream_$id_t pgm_$connv[128];

typedef struct {
	char *p;
} pgm_$proc;
typedef short enum {pgm_$child_proc} pgm_$ec_key;

/* --------------------------------------------------------------------- */
/* I N V O K E - invoke a program */
std_$call void pgm_$invoke();

/* --------------------------------------------------------------------- */
/* G E T _ A R G S  - get pointer to argument vector */
std_$call void pgm_$get_args();

/* --------------------------------------------------------------------- */
/* G E T _ A R G - retrieve a command line argument */
std_$call short pgm_$get_arg();

/* --------------------------------------------------------------------- */
/* D E L _ A R G - delete a command line argument */
std_$call void pgm_$del_arg();

/* --------------------------------------------------------------------- */
/* E X I T - exit from a program, invoking cleanup as it goes */
std_$call void pgm_$exit();

/* --------------------------------------------------------------------- */
/* S E T _ S E V E R I T Y - set the severity level */
std_$call void pgm_$set_severity();

/* -------------------------------------------------------------------- */
/* G E T _ E C - get an eventcount to wait for child process to finish */
std_$call void pgm_$get_ec();

/* -------------------------------------------------------------------- */
/* P R O C _ W A I T - wait for pm_$invoke'ed process to finish, and return status */
std_$call void  pgm_$proc_wait();

/* -------------------------------------------------------------------- */
/* G E T _ P U I D - get the uid of a process */
std_$call void pgm_$get_puid();

/* -------------------------------------------------------------------- */
/* M A K E _ O R P H A N --- make a process into an orphan */
std_$call void pgm_$make_orphan();
