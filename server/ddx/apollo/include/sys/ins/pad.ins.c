/*   PAD.INS.C, /sys/ins, ERS, 01/03/84
     User Program Interface to Display Manager
                
     Changes:
        07/07/87 nab  added pad_$bw5_1024x800 to pad_$display_type_t.
        10/06/86 ltk  added pad_$is_icon and pad_$force_prompt
        03/25/86 mackin Fixed for cyclical includes.
        03/13/86 ltk  changed enum to short enum
        01/31/86 ltk  added inq_icon_font and set_icon_font calls.
        09/04/84 sjr  added config value for Dn550
        04/18/84 jbt  added sr8 pad calls
        01/03/84 ers  brought into sync with SR7 pad.ins.pas
        09/14/83 ems  added pad_$display_t.  also added procedure call
                      pad_$inq_disp_type. 
        09/10/83 jrw  added pad_$read_edit to pad_$type_t.  also added some
                      status values.
        10/07/82 ers  original coding.
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

#ifndef pad_ins_c
#define pad_ins_c

#define pad_$stream_not_pad   0x02030001   /* proferred stream is not a pad */
#define pad_$not_input        0x02030002   /* operation valid on input pads only */
#define pad_$id_oor           0x02030003   /* stream id out of range */
#define pad_$stream_not_open  0x02030004   /* no stream open on this sid */
#define pad_$not_transcript   0x02030005   /* operation valid on transcript pads only */
#define pad_$not_raw          0x02030006   /* operation requires pad be in raw mode */
#define pad_$voor             0x02030007   /* value out of range */
#define pad_$too_many_fonts   0x02030008   /* too many fonts loaded in this pad */
#define pad_$font_file_err    0x02030009   /* couldn't access font file */
#define pad_$bad_key_name     0x0203000A   /* key name not found */
#define pad_$2mny_input_pads  0x0203000B   /* only one input pad per transcript */
#define pad_$ill_param_comb   0x0203000C   /* conflict in pad_$create call */
#define pad_$not_ascii        0x0203000D   /* existing pad in pad_$create not ascii */
#define pad_$2mny_clients     0x0203000E   /* operation illegal with > 1 client process */
#define pad_$edit_quit        0x0203000F   /* user quit (wc -q) out of edit pane */
#define pad_$no_such_window   0x02030010   /* bad window number in inq/set_view */
#define pad_$no_window        0x02030011   /* window no longer exists */
#define pad_$ill_ptype        0x02030012   /* can't do operation on this type pad */


#define pad_$newline  10                  /* terminates normal input and output lines */
#define pad_$cr  13                       /* return cursor to left edge of pad */
#define pad_$ff  12                       /* justify text to top of window */
#define pad_$bs   8                       /* backspace by one space size */
#define pad_$tab  9                       /* move to next horizontal tab stop */
#define pad_$escape 27            

      /* cursor position report types */

#define pad_$cpr_none  0                  /* no cursor position reporting */
#define pad_$cpr_change  1                /* report only changed position */
#define pad_$cpr_all  2                   /* report on each raw keystroke */
#define pad_$cpr_pick  3                  /* report on stabilized pointing */
#define pad_$cpr_draw  4                  /* report on all touchpad data  */

#define pad_$cpr_flag  0xFF              /* flags cursor position report in input */
#define pad_$no_key  0xFE                /* drawing data only */
#define pad_$left_window  0xFD           /* cursor left window in pick/draw */

#define pad_$max_tabstops  100

typedef short pad_$coordinate_t;    /* for x and y coordinates */

typedef short enum {pad_$transcript, pad_$input, pad_$edit, pad_$read_edit} pad_$type_t;

typedef short enum {pad_$left, pad_$right, pad_$top, pad_$bottom} pad_$side_t;

typedef char pad_$string_t[256];      /* string arg to some functions */
typedef short pad_$tabstop_buf_t[pad_$max_tabstops];  /* tab stop settings */

typedef short enum {pad_$relative, pad_$absolute} pad_$rel_abs_t;

typedef struct {
        short top, left;        /* screen coordinates of upper left of window */
        short width, height;    /* size of window */
} pad_$window_desc_t;

typedef pad_$window_desc_t pad_$window_list_t[10];
typedef char pad_$key_name_t[4];
typedef char pad_$key_def_t[128];

typedef short pad_$cre_opt_t;
     /* bits in pad_$cre_opt_t */
#define pad_$abs_size 1     /* size parameter is absolute */
#define pad_$init_raw 2     /* input pad is initially raw */

typedef short enum {pad_$none,
              pad_$bw_15P,
              pad_$bw_19L,
              pad_$color_display,
              pad_$800_color,
              pad_$color2_display,
              pad_$color3_display,
              pad_$reserved_display,
              pad_$color4_display,
              pad_$bw4_1280x1024,
              pad_$color5_display,
              pad_$bw5_1024x800
             } pad_$display_type_t;

typedef struct {
        short y_coord, x_coord;
} pad_$position_t;

std_$call void pad_$raw();
std_$call void pad_$cooked();
std_$call void pad_$create();                   /* create a new pad */
std_$call void pad_$create_window();            /* create a new pad, and window */
std_$call void pad_$create_frame();
std_$call void pad_$delete_frame();
std_$call void pad_$clear_frame();
std_$call void pad_$close_frame();
std_$call void pad_$move();
std_$call void pad_$set_scale();
std_$call void pad_$load_font();
std_$call void pad_$use_font();
std_$call void pad_$inq_font();
std_$call void pad_$inq_windows();
std_$call void pad_$inq_position();
std_$call void pad_$set_tabs();
std_$call void pad_$cpr_enable();
std_$call void pad_$locate();
std_$call void pad_$dm_cmd();
std_$call void pad_$def_pfk();
std_$call void pad_$edit_wait();
std_$call void pad_$inq_disp_type();
std_$call void pad_$inq_view();
std_$call void pad_$set_view();
std_$call void pad_$inq_kbd();
std_$call void pad_$pop_push_window();
std_$call void pad_$set_border();
std_$call void pad_$create_icon();
std_$call void pad_$make_icon();
std_$call void pad_$icon_wait();
std_$call void pad_$set_full_window();
std_$call void pad_$inq_full_window();
std_$call void pad_$set_icon_pos();
std_$call void pad_$inq_icon();
std_$call void pad_$make_invisible();
std_$call void pad_$select_window();  
std_$call void pad_$set_auto_close();
std_$call void pad_$inq_icon_font();
std_$call void pad_$set_icon_font();
std_$call void pad_$force_prompt();
std_$call boolean  pad_$is_icon();

#endif /* pad_ins_c */
