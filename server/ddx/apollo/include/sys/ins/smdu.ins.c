/*  SMDU.INS.C, /sys/ins, ers, 09/14/83
    Display driver definitions - for users and display manager
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


#define smd_$operation_ok         0x00000000
#define smd_$illegal_unit         0x00130001
#define smd_$font_not_loaded      0x00130002
#define smd_$font_table_full      0x00130003
#define smd_$illegal_caller       0x00130004
#define smd_$font_too_large       0x00130005
#define smd_$hdmt_unload_err      0x00130006
#define smd_$illegal_direction    0x00130007
#define smd_$unexp_blt_inuse      0x00130008
#define smd_$protocol_viol        0x00130009
#define smd_$too_many_pages       0x0013000A
#define smd_$unsupported_font_ver 0x0013000B
#define smd_$invalid_buffer_size  0x0013000C
#define smd_$display_map_error    0x0013000D
#define smd_$borrow_error         0x0013000E
#define smd_$display_in_use       0x0013000F
#define smd_$access_denied        0x00130010
#define smd_$return_error         0x00130011
#define smd_$not_borrowed         0x00130012
#define smd_$cant_borrow_both     0x00130013
#define smd_$already_borrowed     0x00130014
#define smd_$invalid_pos          0x00130015
#define smd_$invalid_window       0x00130016
#define smd_$invalid_length       0x00130017
#define smd_$invalid_direction    0x00130018
#define smd_$invalid_displacement 0x00130019
#define smd_$invalid_blt_mode     0x0013001A
#define smd_$invalid_blt_ctl      0x0013001B
#define smd_$invalid_bltd_int     0x0013001C
#define smd_$invalid_ir_state     0x0013001D
#define smd_$invalid_blt_coord    0x0013001E
#define smd_$font_not_mapped      0x0013001F
#define smd_$already_mapped       0x00130020
#define smd_$not_mapped           0x00130021
#define smd_$quit_while_waiting   0x00130022
#define smd_$invalid_crsr_number  0x00130023
#define smd_$hdm_full             0x00130024
#define smd_$wait_quit            0x00130025
#define smd_$invalid_key          0x00130026
#define smd_$not_on_color         0x00130027
#define smd_$not_implemented      0x00130028 
#define smd_$invalid_wid          0x00130029 
#define smd_$window_obscured      0x0013002A 
#define smd_$no_more_wids         0x0013002B 
#define smd_$process_not_found    0x0013002C 
#define smd_$disp_acqd            0x0013002D   /* signal not allowed with display acquired */
#define smd_$already_acquired     0x0013002E 
#define smd_$acquire_timeout      0x0013002F

#define smd_$string_size 120             /* max # chars passed to write_str */
#define smd_$max_event_data_size 32      /* max # words in event data buffer */
#define smd_$max_request_size 16         /* max # words passed in req buf */
/* If smd_$max_loaded_fonts changes, you need to change smdlib_data.asm!! */
#define smd_$max_loaded_fonts 8          /* max # loaded fonts */
#define smd_$max_font_length 0x10000     /* max font length, 64KB */

#define smd_$black 0                    /* color values for smd_$color */
#define smd_$white 1

typedef struct {
        short line;
        short column;
} smd_$pos_t;

typedef struct {
           short xs;
           short xe;
           short ys;
           short ye;
} smd_$window_limits_t;

typedef char smd_$string_t[smd_$string_size];

typedef struct {
        pinteger mode;              /* mode register */
        linteger cs;                /* control (source) */
        linteger cd;                /* control (dest) */
        pinteger ssy;               /* start source y */
        pinteger ssx;               /* start source x */
        pinteger sdy;               /* start destination y */
        pinteger sdx;               /* start destination x */
        pinteger esy;               /* end source y */
        pinteger esx;               /* end source x */
        pinteger edy;               /* end destination y */
        pinteger edx;               /* end destination x */
} smd_$blt_regs_t;

typedef short enum {smd_$up, smd_$down, smd_$left, smd_$right} smd_$direction_t;

typedef short enum {
        smd_$input, smd_$signal, smd_$scroll_blt_complete,
        smd_$borrow_req, smd_$return_req, smd_$kbd_return_req,
        smd_$keyswitch_off, smd_$tpad_and_input, smd_$tpad_data,
        smd_$no_event
} smd_$event_t;
typedef union {
    char input_char;
    pinteger dbuf[smd_$max_event_data_size];
    struct {
        smd_$pos_t pos;
        short buttons;      /* non_zero only for bitpad */
    } tpad_data;
    struct {
        char tp_inchar;
        smd_$pos_t ipos;
    } tpad_and_input;
} smd_$event_data_t;

typedef pinteger smd_$request_t[smd_$max_request_size];

typedef pinteger smd_$display_memory_t[1024][64];

typedef smd_$display_memory_t *smd_$display_memory_ptr_t;
typedef pinteger *smd_$display_control_ptr_t;

typedef struct {       /* version 1 font desc table entry */
           char left;                 /* # rasters to left of curr x */
           char right;                /* rasters to right of curr x */
           char up;                   /* rasters above curr y */
           char down;                 /* rasters below curr y */
           char width;                /* character width, rasters */
           char x_pos;                /* horiz pos in font image offs 800 */
           short y_pos;               /* vpos in image, offs fit.y_offset */
} smd_$v1_fdte_t;

typedef struct {       /* version 2 font desc table entry */
           short left;                /* # rasters to left of curr x */
           short right;               /* rasters to right of curr x */
           short up;                  /* rasters above curr y */
           short down;                /* rasters below curr y */
           pinteger width;            /* character width, rasters */
           pinteger x_pos;            /* horiz pos in font image */
           pinteger y_pos;            /* vpos in image, offs fit.y_offset */
           pinteger reserved;         /* future use, to make 16 bytes */
}  smd_$v2_fdte_t;

typedef struct {
            pinteger
                version,               /* font table version # */
                image_offset,       /* image offset from 1st wd of hdr */
                chars_in_font,      /* # characters in font */
                raster_lines,       /* # raster lines reqd for image */
                image_size,         /* image size, bytes */
                max_height,         /* max char height, rasters */
                max_width,          /* max char width, rasters */
                v_spacing,          /* vertical (inter-line) spacing */
                h_spacing,          /* horiz (inter-character) spacing */
                space_size,         /* width of space character */
                max_right,          /* char max right, rasters */
                max_up,             /* char max up, rasters */
                max_down;           /* char max down, rasters */
            char index_table[128];  /* desc tab offsets per char */
            smd_$v1_fdte_t desc_table[128];  /* desc table */
} smd_$font_table_t;

typedef smd_$font_table_t *smd_$font_table_ptr_t;

typedef pinteger smd_$cursor_bitmap_t[16];

typedef short enum {
        smd_$none, 
        smd_$bw_15P, 
        smd_$bw_19L, 
        smd_$color_display, 
        smd_$800_color,
        smd_$color2_display,
        smd_$color3_display,
        smd_$reserved_display,
        smd_$color4_display,
        smd_$bw4_1280x1024,
        smd_$color5_display,
        smd_$bw5_1024x800
} smd_$display_type_t;

typedef short enum {smd_$input_ec, smd_$scroll_blt_ec} smd_$ec_key_t;


#define smd_$font_header_size 26          /* # bytes in font header */
#define smd_$font_index_table_size 128    /* # bytes in index table */
#define smd_$v1_fdte_size 8               /* # bytes in 1 descriptor table entry */


/*  Display mode register declaration.  This is the C template for the
   display hardware mode register.  */

typedef struct {
        unsigned short blt: 1,          /* 8000 - bit blt start / run flag */
                 dummy: 6,              /* 7E00 - pad */
                 noninterlace: 1,       /*  100 - hardware noninterlace mode */
                 from_mm: 1,            /*   80 - from main memory */
                 to_mm: 1,              /*   40 - to main memory */
                 ieof: 1,               /*   20 - interrupt at eof */
                 idone: 1,              /*   10 - interrupt on blt done */
                 nonconform: 1,         /*    8 - nonconforming areas */
                 decr: 1,               /*    4 - decrement mode (whadis?) */
                 clrmode: 1,            /*    2 - clear mode */
                 dispon: 1;             /*    1 - display on if true */
} smd_$display_modereg_t;
#eject
/*  Following are user-mode calls to the display driver.  These may be
   invoked while the display is "borrowed" by the user.  */


/*   SMD_$LOAD_FONT_U is called to load a font into the hidden portion
    of display memory.  The font_id returned by this procedure is used in
    all future calls to write_string to identify the font.  */

std_$call pinteger smd_$load_font_u();


/*   SMD_$UNLOAD_FONT_U is called to unload a font from the hidden
    portion of display memory.  This routine is invoked to make space for
    another font when the hdm (hidden display memory) or a font table
    becomes full.  */

std_$call void  smd_$unload_font_u();


/*  SMD_$LOAD_FONT_FILE_U maps a named font file into memory and loads it
   using the load_font routine described above.  */

std_$call void  smd_$load_font_file_u();


/*  SMD_$UNLOAD_FONT_FILE_U unloads a font and unmaps the associated font
   file.  This routine should be used for each font loaded with the
   load_font_file routine.  */

std_$call void  smd_$unload_font_file_u();


/*  SMD_$WRITE_STRING_U is called to write a text string on the display.  */

std_$call void  smd_$write_string_u();


/*  SMD_$SOFT_SCROLL_U is used to start a soft scroll operation.  The specified
   area of the screen is scrolled, 2 raster lines at a time, until the
   number of raster lines moved is equal to <displacement>.  (If this is
   odd, the last blt will be move the area 1 line.)  This routine returns
   prior to the completion of the scroll.

   Any smd routine which does not require the services of the interrupt
   routine may be called and executed while the scroll is in progress
   without waiting for the scroll to complete.  If an smd routine which
   requires the services of the interrupt handler is called, execution of
   that process is suspended until the scroll completes.  */

std_$call void  smd_$soft_scroll_u();


/*  SMD_$EVENT_WAIT_U causes the calling process to be suspended until a
   screen manager event occurs.  These events are enumerated in the
   smd_$event_t type definition.  User processes will only be notified
   of scroll/blt completion and keyboard input; never of screen manager
   requests.  */

std_$call void  smd_$event_wait_u();


/*  SMD_$COND_EVENT_WAIT_U is a conditional version of event_wait.  It returns
   smd_$no_event, if no event has occurred.  It never waits.   */

std_$call void  smd_$cond_event_wait_u();


/*  SMD_$COND_INPUT_U performs conditional input.  If a character has been
   typed, it is returned in "ch" and the function value is true.  If not,
   the value of "ch" is undefined and the function value is false.  */

std_$call short smd_$cond_input_u();


/*  SMD_$BLT_U is called to start and optionally wait for the completion
   of a display blt operation.

   The procedure will return without waiting for completion if the
   "interrupt when blt done" mode register bit is set.

   The state of the "display on" and "interrupt at end of frame" bits are
   irrelevant, as the default mode register, internal to the smd, is used
   to obtain this information.  */

std_$call void  smd_$blt_u();


/*  SMD_$DRAW_BOX_U is called to draw a box around a window.  Lines are drawn
   to vertically and horizontally connect the 4 given endpoints.  */

std_$call void  smd_$draw_box_u();


/*  SMD_$CLEAR_WINDOW_U is called by the sm to clear the area of the screen
   enclosed within the 4 specified endpoints.  This routine does not return
   until the clear is complete.  (This makes it eligible to interlace with
   soft scrolling.)  */

std_$call void  smd_$clear_window_u();


/*  SMD_$MOVE_KBD_CURSOR_U causes the keyboard cursor to be moved from its
   present position to the position specified.  The display driver's
   knowledge of cursor position is also updated.  */

std_$call void  smd_$move_kbd_cursor_u();


/*  SMD_$CLEAR_KBD_CURSOR_U is used to clear the keyboard-associated cursor
   from the display and inhibit cursor blinking.  */

std_$call void  smd_$clear_kbd_cursor_u();


/*  SMD_$BORROW_DISPLAY_U is called by the user to ask to borrow the display
   memory and a subset of the display driver routines.  */

std_$call void  smd_$borrow_display_u();


/*  SMD_$BORROW_DISPLAY_NC_U is called by the user to ask to borrow the display
   memory and a subset of the display driver routines.  It is similar to the
   normal borrow_display procedure, except that the display is not cleared.  */

std_$call void  smd_$borrow_display_nc_u();


/*  SMD_$RETURN_DISPLAY_U is called by the user to terminate his exclusive
   use of the display and smd routines.  The display is unmapped from his
   address space and further use of the smd routines is disallowed.  */

std_$call void  smd_$return_display_u();


/*  SMD_$OP_WAIT_U waits for the current soft scroll or blt operation to
   complete, then returns.  */

std_$call void smd_$op_wait_u();


/*  SMD_$MAP_DISPLAY_U maps the display memory into the user's address space
   and returns a pointer to the first byte.  */

std_$call void  smd_$map_display_u();


/*  SMD_$UNMAP_DISPLAY_U unmaps the display memory from the user's address
   space.  It must previously have been mapped with SMD_$MAP_DISPLAY_U.  */

std_$call void  smd_$unmap_display_u();


/*  SMD_$VECTOR_INIT_U initializes the vector drawing routines SMD_$DRAW_ABS_U,
   SMD_$DRAW_REL_U, SMD_$MOVE_ABS_U, SMD_$MOVE_REL_U.  */

std_$call void  smd_$vector_init_u();



/*  SMD_$DRAW_ABS_U draws a vector from the current point to the point specified,
   in absolute screen coordinates.  */

std_$call void  smd_$draw_abs_u();


/*  SMD_$DRAW_REL_U draws a vector from the current point to the point specified,
   in pixels relative to the current point.  */

std_$call void  smd_$draw_rel_u();


/*  SMD_$MOVE_ABS_U moves the "current" position to that specified by the x and
   y arguments, in absolute screen coordinates.  */

std_$call void  smd_$move_abs_u();


/*  SMD_$MOVE_REL_U moves the "current" position to that specified by the x and
   y coordinates, relative to the current position.  */

std_$call void  smd_$move_rel_u();

/*  SMD_$COLOR sets the color (black or white/green) of subsequent lines to
   be drawn by draw_abs_u or draw_rel_u   */

std_$call void smd_$color();

/*  SMD_$SET_TP_CURSOR offers touchpad (or other locator device) data to the
   driver, which then displays a different cursor shape, that follows the
   pos data, and eventually delivers the data to the display manager or
   other process owning the display.  */

std_$call void smd_$set_tp_cursor();

/*  SMD_$STOP_TP_CURSOR turns off the touchpad cursor and puts back the blinking
   keyboard cursor, if it is supposed to be up.  */

std_$call void smd_$stop_tp_cursor();

/*  SMD_$TP_ENABLE allows the touchpad cursor to be displayed
   and moved around the screen.  Initial state is off.   */

std_$call void smd_$tp_enable();

/*  SMD_$TP_DISABLE is the routine of choice to call when preparing to
   touch display memory directly in user mode, since it both prevents
   interference from the touchpad cursor and guarantees that the blt
   is clear.  Note that operations within the driver automatically take
   down the tp cursor if it is up, since the caller does not know where
   it is, and hence cannot take it down conditionally.  */

std_$call void smd_$tp_disable();

/*  SMD_$TP_DIRECT sets or clears the tpad_direct bit.

   If tpad_direct is set, then touchpad data does not put a cursor on the
   screen, but delivers the data through smd_$event_wait, as a special
   event type.  If not set, then the cursor is moved around by tp data,
   and the new position will be delivered with the next keystroke, as
   a keystroke with cursor event.   */

std_$call void smd_$tp_direct();


/*  SMD_$LOAD_CRSR_BITMAP - Define a bitmap for one of the internal cursors  */

std_$call void smd_$load_crsr_bitmap();


/*  SMD_$READ_CRSR_BITMAP - Return the bitmap for one of the internal cursors  */

std_$call void smd_$read_crsr_bitmap();


/*  SMD_$SET_QUIT_CHAR defines the character which causes a quit to be
   sent to the borrowing process.  */

std_$call void smd_$set_quit_char();


/*  SMD_$INQ_DISP_TYPE returns the type of the display physically attached to
   the given unit number.   */

std_$call smd_$display_type_t smd_$inq_disp_type();


/*  SMD_$ALLOC_HDM

   Allocate space in hidden display memory.  It returns an smd_$pos_t
   indicating the x/y position in overall display memory, and works for
   both portrait and landscape displays.  The returned space is always
   224 bits wide, and on the landscape display, may wrap across column
   boundaries.  */

std_$call void  smd_$alloc_hdm();


/*  SMD_$FREE_HDM

   Free previously allocated space in hidden display memory   */

std_$call void  smd_$free_hdm();


/*  SMD_$INVERT

   Invert the display, and all subsequent operations   */

std_$call void  smd_$invert();


/*  SMD_$GET_EC exports level 1 eventcounts to user-space.  */

std_$call void  smd_$get_ec();

#eject
