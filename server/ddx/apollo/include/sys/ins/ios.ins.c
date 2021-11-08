  /* *** ios.ins.c /sys/ins
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

#define ios_$subs        0x01
#define ios_$modc        0x27

#define ios_$switch_modc 0x01270000

/* -=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-       E R R O R   S T A T U S   C O D E S      -=-=-=-=- */
/* -=-=-=-  all st.code components are greater than zero  -=-=-=-=- */
/* -=-=-=-       (see warning status codes below...)      -=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#define ios_$not_open                 (ios_$switch_modc + 1)  /* operation attempted on unopened stream */
#define ios_$illegal_operation        (ios_$switch_modc + 2)  /* this operation illegal on named stream */
#define ios_$from_id_not_open         (ios_$switch_modc + 3)  /* from stream not open on switch request */
#define ios_$no_avail_target          (ios_$switch_modc + 4)  /* no available target stream to switch to */
#define ios_$target_inuse             (ios_$switch_modc + 5)  /* target id already in use on switch */
#define ios_$illegal_obj_type         (ios_$switch_modc + 6)  /* cannot open a stream for this type of object */
#define ios_$no_more_streams          (ios_$switch_modc + 7)  /* no more streams */
#define ios_$concurrency_violation    (ios_$switch_modc + 8)  /* requested access violates concurrency constraints */
#define ios_$end_of_file              (ios_$switch_modc + 9)  /* end of file */
#define ios_$insuff_memory            (ios_$switch_modc + 10) /* not enough virtual memory */
#define ios_$perm_file_needs_name     (ios_$switch_modc + 11) /* only temporary files may be unnamed */
#define ios_$put_bad_rec_len          (ios_$switch_modc + 12) /* attempted put of wrong length record */
#define ios_$internal_fatal_err       (ios_$switch_modc + 13) /* internal fatal error on table re-verification */
#define ios_$already_exists           (ios_$switch_modc + 14) /* ios_$no_pre_exist specified on ios_$create */
#define ios_$illegal_w_var_lgth_recs  (ios_$switch_modc + 15) /* operation illegal with variable length records
                                                                 is on record boundary */
#define ios_$BOF_err                  (ios_$switch_modc + 17) /* attempted seek beyond BOF; e.g.,offset= ios_$switch_modc +0 */
#define ios_$bad_char_seek            (ios_$switch_modc + 18) /* attempted character seek before start of current
                                                                 (variable length) record */
#define ios_$bad_open_XP              (ios_$switch_modc + 19) /* open_XP must reference a stream that is already
                                                                 open in this process */
#define ios_$bad_count_field_in_file  (ios_$switch_modc + 20) /* count field for current record is bad */
#define ios_$name_not_found           (ios_$switch_modc + 21) /* name not found */
#define ios_$bad_file_hdr             (ios_$switch_modc + 22) /* file hdr is no good ( CRC error ) */
#define ios_$bad_location             (ios_$switch_modc + 23) /* bad location param in create call */
#define ios_$id_oor                   (ios_$switch_modc + 26) /* stream id out-of-range (invalid) */
#define ios_$object_not_found         (ios_$switch_modc + 28) /* object associated with this name not found (may not exist) */
#define ios_$illegal_name_redefine    (ios_$switch_modc + 31) /* attempted name change requires copying file*/
#define ios_$obj_deleted              (ios_$switch_modc + 32) /* file has been deleted */
#define ios_$name_reqd                (ios_$switch_modc + 33) /* ios_$open with no name illegal */
#define ios_$resource_lock_err        (ios_$switch_modc + 36) /* unable to lock resources required to process request */
#define ios_$illegal_pad_create_type  (ios_$switch_modc + 37) /* pad_create illegal with this type of object */
#define ios_$internal_mm_err          (ios_$switch_modc + 38) /* internal fatal error in stream memory mgmt (windowing) */
#define ios_$read_only_err            (ios_$switch_modc + 39) /* attempted write to read-only stream */
#define ios_$something_failed         (ios_$switch_modc + 42) /* partial or complete failure of inquire or
                                                                 redefine (err_mask is non-empty) */
#define ios_$device_must_be_local     (ios_$switch_modc + 43) /* cannot open stream to remote device */
#define ios_$no_rights                (ios_$switch_modc + 44) /* no rights to access object */
#define ios_$insufficient_rights      (ios_$switch_modc + 45) /* insufficient rights for requested access to object */
#define ios_$invalid_data             (ios_$switch_modc + 46) /* bad data in call to vt_$put */
#define ios_$no_table_space           (ios_$switch_modc + 47) /* obsolete */
#define ios_$need_move_mode           (ios_$switch_modc + 48) /* locate operation refused -- try get */
#define ios_$out_of_shared_cursors    (ios_$switch_modc + 50) /* per-mode shared file cursor pool exhausted */
#define ios_$bad_shared_cursor_refcnt (ios_$switch_modc + 51) /* ref cnt on a shared file cursor went below zero */
#define ios_$xp_buf_too_small         (ios_$switch_modc + 52) /* buffer supplied to get_xp too small */
#define ios_$illegal_param_comb       (ios_$switch_modc + 53) /* illegal parameter combination for this operation */
#define ios_$dir_not_found            (ios_$switch_modc + 54) /* couldn't find directory in pathname */
#define ios_$never_closed             (ios_$switch_modc + 55) /* system (or process) crash prevented complete close */
                                                              /*   actually => header all zero */
#define ios_$sio_not_local            (ios_$switch_modc + 57) /* sio object not in /dev */
#define ios_$not_thru_link            (ios_$switch_modc + 58) /* can't create file though link */
#define ios_$object_read_only         (ios_$switch_modc + 60) /* cannot open this object for writing */
#define ios_$put_conditional_failed   (ios_$switch_modc + 61) /* a put conditional operation could not be done now */
#define ios_$get_conditional_failed   (ios_$switch_modc + 62) /* a get conditional operation found no data */
#define ios_$not_at_rec_bndry         (ios_$switch_modc + 63) /* can't do inq_short_key */
#define ios_$cant_change_type         (ios_$switch_modc + 64) /*  */
#define ios_$inq_only_error           (ios_$switch_modc + 65) /* opened for inquire only */
#define ios_$file_not_empty           (ios_$switch_modc + 66) /* type change rejected because file not empty */
#define ios_$buffer_too_big           (ios_$switch_modc + 67) /* this manager does not support this operation */
                                                              /*    for a buffer that is this large */
#define ios_$cant_initialize          (ios_$switch_modc + 68) /* can't initialize an object of this type */
#define ios_$flag_not_supported       (ios_$switch_modc + 69) /* flag not supported by manager */
#define ios_$cant_set_advisory_lock   (ios_$switch_modc + 70) /* advisory lock was already set by someone else */
#define ios_$no_advisory_lock_set     (ios_$switch_modc + 71) /* advisory lock was not already set in UNLOCK */


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=   W A R N I N G   S T A T U S   C O D E S   =-=-=-=-=-=- */
/* -=-=-=-=-=  all st.code components are less than zero  =-=-=-=-=-=- */
/* -=-=-=-=-=      (see error status codes above...)      =-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#define ios_$part_rec_warn    (ios_$switch_modc + 0x0fffc)   /* partial record at eof on close -- warning only  -4  */
#define ios_$cant_delete_old_name (ios_$switch_modc + 0x0fffb) /* new name added but old can't be deleted       -5  */
#define ios_$buffer_too_small (ios_$switch_modc + 0x0fffa)   /* buffer too small on get/locate                  -6  */
#define ios_$full_rec_unavail (ios_$switch_modc + 0x0fff9)   /* GET requested a full record, but only part of   -7  */
                                                           /*   the record was available. The part that was */
                                                           /*   available was returned, but there was */
                                                           /*   still more room in the buffer. */

#define ios_$no_stream 0x7fff

typedef short enum {
        ios_$rec_seek,      /* record-oriented seek */
        ios_$byte_seek      /* byte-oriented seek */
} ios_$seek_type_t;

typedef short enum {
        ios_$relative,      /* seek is relative to current position */
        ios_$absolute       /* seek is relative to BOF */
} ios_$abs_rel_t;

typedef short enum {
        ios_$current,       /* inquiry is for current position */
        ios_$bof,           /* inquire of key to use for BOF */
        ios_$eof            /* inquire of key to use for EOF */
} ios_$pos_opt_t;

typedef short enum {
        ios_$v1,            /* variable lgth records with counts */
        ios_$f2,            /* fixed lgth records (with counts) */
        ios_$undef,         /* no record structure in data;
                               file has header, however */
        ios_$explicit_f2,   /* like F2, but cannot be implicitly
                               changed to V1 by put_rec */
        ios_$f1             /* records without count fields */    
} ios_$rtype_t;

    /* Key type parameter for ios_$get_ec */

typedef short enum {
        ios_$get_ec_key,
        ios_$put_ec_key
} ios_$ec_key_t;

    /* options avail at open or create time, through ios_$open or ios_$create */

#define ios_$no_open_options     ((ios_$open_options_t) 0x00)
#define ios_$no_open_delay_opt   ((ios_$open_options_t) 0x01)
#define ios_$write_opt           ((ios_$open_options_t) 0x02)
#define ios_$unregulated_opt     ((ios_$open_options_t) 0x04)
#define ios_$position_to_eof_opt ((ios_$open_options_t) 0x08)
#define ios_$inquire_only_opt    ((ios_$open_options_t) 0x10)
#define ios_$read_intend_write_opt ((ios_$open_options_t) 0x20)

typedef short ios_$open_options_t;

    /* creation mode specifies what to do if name already exists; only the
      loc_name_only_mode is relevant if the name does NOT exist */

typedef short enum {
        ios_$no_pre_exist_mode, /* exists => error */
        ios_$preserve_mode,     /* exists => preserve old contents */
        ios_$recreate_mode,     /* exists => completely recreate file */
        ios_$truncate_mode,     /* exists => open, then truncate */
        ios_$make_backup_mode,  /* exists => rename to .bak on close */
        ios_$loc_name_only_mode /* name is to specify location only */
} ios_$create_mode_t;

    /* options for data transfer operations */

#define ios_$no_put_get_opts    ((ios_$put_get_opts_t) 0x00)
#define ios_$cond_opt           ((ios_$put_get_opts_t) 0x01)
#define ios_$preview_opt        ((ios_$put_get_opts_t) 0x02)
#define ios_$partial_record_opt ((ios_$put_get_opts_t) 0x04)
#define ios_$no_rec_bndry_opt   ((ios_$put_get_opts_t) 0x08)

typedef short ios_$put_get_opts_t;

    /* There are three kinds of flags: "manager flags", which describe
      attributes of all objects handled by the manager of the object
      referenced by the ID argument; "object flags", which describe
      attributes of the object referenced by the ID argument; and
      "connection flags", which describe attributes of the particular
      stream to the object referenced by the ID argument. */

typedef short enum {            /* Can manager...? */
        ios_$mf_create,         /* Create new files */
        ios_$mf_create_bak,     /* Create ".bak" files */
        ios_$mf_imex,           /* Export streams to new processes */
        ios_$mf_fork,           /* Pass streams to forked processes */
        ios_$mf_force_write,    /* Force files contents to "disk" */
        ios_$mf_write,          /* Write files */
        ios_$mf_seek_abs,       /* Seek using absolute positions */
        ios_$mf_seek_short,     /* Seek using short keys */
        ios_$mf_seek_full,      /* Seek using full keys */
        ios_$mf_seek_byte,      /* Seek to byte positions */
        ios_$mf_seek_rec,       /* Seek to record positions */
        ios_$mf_seek_bof,       /* Seek to beginning of file */
        ios_$mf_rec_type,       /* Support different (or any) type of record */
        ios_$mf_truncate,       /* Truncate files */
        ios_$mf_unregulated,    /* Support unregulated (concurrent) access */
        ios_$mf_sparse,         /* Support sparse files */
        ios_$mf_read_intend_write,  /* Support read-intend-write locking */
} ios_$mgr_flag_t;

typedef long ios_$mgr_flag_set;

#define ios_$mf_create_mask             ((ios_$mgr_flag_set) 0x1)
#define ios_$mf_create_bak_mask         ((ios_$mgr_flag_set) 0x2)
#define ios_$mf_imex_mask               ((ios_$mgr_flag_set) 0x4)
#define ios_$mf_fork_mask               ((ios_$mgr_flag_set) 0x8)
#define ios_$mf_force_write_mask        ((ios_$mgr_flag_set) 0x10)
#define ios_$mf_write_mask              ((ios_$mgr_flag_set) 0x20)
#define ios_$mf_seek_abs_mask           ((ios_$mgr_flag_set) 0x40)
#define ios_$mf_seek_short_mask         ((ios_$mgr_flag_set) 0x80) 
#define ios_$mf_seek_full_mask          ((ios_$mgr_flag_set) 0x100)
#define ios_$mf_seek_byte_mask          ((ios_$mgr_flag_set) 0x200)
#define ios_$mf_seek_rec_mask           ((ios_$mgr_flag_set) 0x400)
#define ios_$mf_seek_bof_mask           ((ios_$mgr_flag_set) 0x800)
#define ios_$mf_rec_type_mask           ((ios_$mgr_flag_set) 0x1000)
#define ios_$mf_truncate_mask           ((ios_$mgr_flag_set) 0x2000)
#define ios_$mf_unregulated_mask        ((ios_$mgr_flag_set) 0x4000)
#define ios_$mf_sparse_mask             ((ios_$mgr_flag_set) 0x8000)
#define ios_$mf_read_intend_write_mask  ((ios_$mgr_flag_set) 0x10000)

typedef short enum {            /* Object...? */
        ios_$of_delete_on_close,/* Will be deleted when closed */
        ios_$of_sparse_ok,      /* Can be seeked and written past EOF */
        ios_$of_ascii,          /* Contains ASCII data */
        ios_$of_ftncc,          /* Contains Fortran carriage control */
        ios_$of_cond,           /* Makes get/put implicitly include ios_$cond */
} ios_$obj_flag_t;

typedef long ios_$obj_flag_set;

#define ios_$of_delete_on_close_mask    ((ios_$obj_flag_set) 0x1)
#define ios_$of_sparse_ok_mask          ((ios_$obj_flag_set) 0x2)
#define ios_$of_ascii_mask              ((ios_$obj_flag_set) 0x4)
#define ios_$of_ftncc_mask              ((ios_$obj_flag_set) 0x8)
#define ios_$of_cond_mask               ((ios_$obj_flag_set) 0x10)

typedef short enum {            /* Connection...? */
        ios_$cf_tty,            /* Behaves like a TTY */
        ios_$cf_ipc,            /* Behaves like an inter-process channel */
        ios_$cf_vt,             /* Behaves like a virtual terminal */
        ios_$cf_write,          /* Can be written to */
        ios_$cf_append,         /* Will have all writes be appends */
        ios_$cf_unregulated,    /* Open mode allows unregulated (concurrent) access */
        ios_$cf_read_intend_write,  /* Open for read-intend-write */
        ios_$cf_sequential      /* Is accessible only by sequential I/O calls */
} ios_$conn_flag_t;

typedef long ios_$conn_flag_set;

#define ios_$cf_tty_mask                ((ios_$conn_flag_set) 0x1)
#define ios_$cf_ipc_mask                ((ios_$conn_flag_set) 0x2)
#define ios_$cf_vt_mask                 ((ios_$conn_flag_set) 0x4)
#define ios_$cf_write_mask              ((ios_$conn_flag_set) 0x8)
#define ios_$cf_append_mask             ((ios_$conn_flag_set) 0x10)
#define ios_$cf_unregulated_mask        ((ios_$conn_flag_set) 0x20)
#define ios_$cf_read_intend_write_mask  ((ios_$conn_flag_set) 0x40)

    /* Options to ios_$inq_path_name */

typedef short enum {
        ios_$leaf_name,
        ios_$wdir_name,
        ios_$ndir_name,
        ios_$node_name,
        ios_$root_name,
        ios_$node_data_flag,
        ios_$resid_name
} ios_$name_type_t;

    /* Options to ios_$set_dir */

typedef short enum {
        ios_$wdir,
        ios_$ndir
} ios_$dir_type_t;

std_$call ios_$id_t ios_$open();
std_$call void ios_$close();
std_$call void ios_$get_ec();
std_$call void ios_$delete();
std_$call void ios_$truncate();
std_$call long ios_$get();
std_$call long ios_$locate();
std_$call void ios_$put();
std_$call long ios_$inq_rec_remainder();
std_$call ios_$mgr_flag_set ios_$inq_mgr_flags();
std_$call ios_$obj_flag_set ios_$inq_obj_flags();
std_$call void ios_$set_obj_flag();
std_$call ios_$conn_flag_set ios_$inq_conn_flags();
std_$call void ios_$set_conn_flag();
std_$call void ios_$seek();
std_$call void ios_$seek_full_key();
std_$call void ios_$seek_short_key();
std_$call void ios_$seek_to_bof();
std_$call void ios_$seek_to_eof();
std_$call long ios_$inq_short_key();
std_$call void ios_$inq_full_key();
std_$call long ios_$inq_rec_pos();
std_$call long ios_$inq_byte_pos();
std_$call long ios_$inq_cur_rec_len();
std_$call ios_$rtype_t ios_$inq_rec_type();
std_$call void ios_$set_rec_type();
std_$call void ios_$force_write_file();
std_$call void ios_$inq_file_attr();
std_$call void ios_$create();
std_$call void ios_$set_type();
std_$call void ios_$change_path_name();
std_$call void ios_$inq_path_name();
std_$call void ios_$inq_type_uid();
std_$call char *ios_$get_handle(); 
std_$call void ios_$set_locate_buffer_size();
std_$call ios_$id_t ios_$switch();
std_$call ios_$id_t ios_$replicate();
std_$call ios_$id_t ios_$dup();
std_$call boolean ios_$equal();
std_$call void ios_$set_dir();
std_$call void ios_$get_dir();
