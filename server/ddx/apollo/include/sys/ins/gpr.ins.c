/*  gpr.ins.c, /sys/ins, dpg, 09/16/83
    graphics primitive package definitions  */

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

#ifndef gpr_$gpr_ins_c
#define gpr_$gpr_ins_c

#define gpr_$operation_ok               0x00000000
#define gpr_$not_initialized            0x06010001
#define gpr_$already_initialized        0x06010002
#define gpr_$wrong_display_hardware     0x06010003
#define gpr_$illegal_for_frame          0x06010004
#define gpr_$must_borrow_display        0x06010005
#define gpr_$no_attributes_defined      0x06010006
#define gpr_$no_more_space              0x06010007
#define gpr_$dimension_too_big          0x06010008
#define gpr_$dimension_too_small        0x06010009
#define gpr_$bad_bitmap                 0x0601000a
#define gpr_$bad_attribute_block        0x0601000b
#define gpr_$window_out_of_bounds       0x0601000c
#define gpr_$source_out_of_bounds       0x0601000d
#define gpr_$dest_out_of_bounds         0x0601000e
#define gpr_$invalid_plane              0x0601000f
#define gpr_$cant_deallocate            0x06010010
#define gpr_$coord_out_of_bounds        0x06010011
#define gpr_$invalid_color_map          0x06010012
#define gpr_$invalid_raster_op          0x06010013
#define gpr_$bitmap_is_read_only        0x06010014
#define gpr_$internal_error             0x06010015
#define gpr_$font_table_full            0x06010016
#define gpr_$bad_font_file              0x06010017
#define gpr_$invalid_font_id            0x06010018
#define gpr_$window_obscured            0x06010019
#define gpr_$not_in_direct_mode         0x0601001a
#define gpr_$not_in_polygon             0x0601001b
#define gpr_$kbd_not_acq                0x0601001c
#define gpr_$display_not_acq            0x0601001d
#define gpr_$illegal_pixel_values       0x0601001e
#define gpr_$illegal_when_imaging       0x0601001f
#define gpr_$invalid_imaging_format     0x06010020
#define gpr_$must_release_display       0x06010021
#define gpr_$cant_mix_modes             0x06010022
#define gpr_$no_input_enabled           0x06010023
#define gpr_$duplicate_points           0x06010024
#define gpr_$array_not_sorted           0x06010025
#define gpr_$character_not_in_font      0x06010026
#define gpr_$illegal_fill_pattern       0x06010027
#define gpr_$illegal_fill_scale         0x06010028
#define gpr_$incorrect_alignment        0x06010029
#define gpr_$illegal_text_path          0x0601002a
#define gpr_$unable_to_rotate_font      0x0601002b
#define gpr_$font_is_read_only          0x0601002c
#define gpr_$illegal_pattern_length     0x0601002d
#define gpr_$illegal_for_pixel_bitmap   0x0601002e
#define gpr_$too_many_input_windows     0x0601002f
#define gpr_$illegal_software_version   0x06010030
#define gpr_$bitmap_not_a_file_bitmap   0x06010031
#define gpr_$no_color_map_in_file       0x06010032
#define gpr_$incorrect_decomp_tech      0x06010033
#define gpr_$no_reset_decomp_in_pgon    0x06010034
#define gpr_$specific_nonzero_only      0x06010035
#define gpr_$arc_overflow_16bit_bounds  0x06010036
#define gpr_$invalid_virt_dev_id        0x06010037
#define gpr_$bad_decomp_tech            0x06010038
#define gpr_$empty_rop_prim_set         0x06010039
#define gpr_$rop_sets_not_equal         0x0601003A
#define gpr_$must_have_display          0x0601003B
#define gpr_$style_call_not_active      0x0601003C
#define gpr_$no_more_fast_buffers       0x0601003D
#define gpr_$input_buffer_overflow      0x0601003E

#define gpr_$default_list_size 10

#define gpr_$black   ((gpr_$pixel_value_t) 0x000000) /* color value for black */
#define gpr_$white   ((gpr_$pixel_value_t) 0xffffff) /* color value for white */
#define gpr_$red     ((gpr_$pixel_value_t) 0xff0000) /* color value for red */
#define gpr_$green   ((gpr_$pixel_value_t) 0x00ff00) /* color value for green */
#define gpr_$blue    ((gpr_$pixel_value_t) 0x0000ff) /* color value for blue */
#define gpr_$cyan    ((gpr_$pixel_value_t) 0x00ffff) /* color value for cyan (blue + green) */
#define gpr_$magenta ((gpr_$pixel_value_t) 0xff00ff) /* color value for magenta (red + blue) */
#define gpr_$yellow  ((gpr_$pixel_value_t) 0xffff00) /* color value for yellow (red + green) */

#define gpr_$transparent ((gpr_$pixel_value_t)(-1))  /* pixel value for transparent (no change) */
#define gpr_$background  ((gpr_$pixel_value_t)(-2))  /* pixel value for window background */

#define gpr_$string_size 256            /* number of chars in a gpr string */
#define gpr_$max_x_size 8192            /* max # bits in bitmap x dimension */
#define gpr_$max_y_size 8192            /* max # bits in bitmap y dimension */
#define gpr_$highest_plane 7            /* max plane number in a bitmap */
#define gpr_$highest_rgb_plane 31       /* max plane number in a true color bitmap */
#define gpr_$nil_attribute_desc 0l      /* value of a descriptor of nonexistent attributes */
#define gpr_$nil_bitmap_desc 0l         /* value of a descriptor of a nonexistent bitmap */

#define gpr_$max_bmf_group 0            /* max group # in external bitmaps */
#define gpr_$bmf_major_version 1
#define gpr_$bmf_minor_version 1

#define gpr_$rop_zeros                   0  /* symbolic constants for rops. */
#define gpr_$rop_src_and_dst             1
#define gpr_$rop_src_and_not_dst         2
#define gpr_$rop_src                     3
#define gpr_$rop_not_src_and_dst         4
#define gpr_$rop_dst                     5
#define gpr_$rop_src_xor_dst             6
#define gpr_$rop_src_or_dst              7
#define gpr_$rop_not_src_and_not_dst     8
#define gpr_$rop_src_equiv_dst           9
#define gpr_$rop_not_dst                10
#define gpr_$rop_src_or_not_dst         11
#define gpr_$rop_not_src                12
#define gpr_$rop_not_src_or_dst         13
#define gpr_$rop_not_src_or_not_dst     14
#define gpr_$rop_ones                   15

typedef short enum { gpr_$keystroke, gpr_$buttons, gpr_$locator,
                     gpr_$entered_window, gpr_$left_window,
                     gpr_$locator_stop, gpr_$no_event, 
                     gpr_$locator_update, gpr_$dial
} gpr_$event_t;  


typedef unsigned long gpr_$keyset_t[8];    /* this is supposed to be set of char */

typedef short enum { gpr_$ok_if_obs, gpr_$error_if_obs,
                     gpr_$pop_if_obs, gpr_$block_if_obs,
                     gpr_$input_ok_if_obs
} gpr_$obscured_opt_t;

typedef short enum {gpr_$input_ec} gpr_$ec_key_t;

typedef void (*gpr_$rwin_pr_t)();
typedef void (*gpr_$rhdm_pr_t)();

            /* the five ways to use this package */
typedef short enum {
        gpr_$borrow,
        gpr_$frame,
        gpr_$no_display,
        gpr_$direct,
        gpr_$borrow_nc,
        gpr_$direct_rgb,
        gpr_$borrow_rgb,
        gpr_$borrow_rgb_nc                         
} gpr_$display_mode_t;

            /* possible display hardware configurations */
typedef short enum {
        gpr_$bw_800x1024,           /* dn100, dn400 */
        gpr_$bw_1024x800,           /* dn3xx, dn4xx */
        gpr_$color_1024x1024x4,     /* dn6xx */
        gpr_$color_1024x1024x8,     /* dn6xx */
        gpr_$color_1024x800x4,      /* dn550/560 */
        gpr_$color_1024x800x8,      /* dn550/560 */
        gpr_$color_1280x1024x8,     /* dn580 */
        gpr_$color1_1024x800x8,     /* dn570 */
        gpr_$color2_1024x800x4,     /* dn3000c */
        gpr_$bw_1280x1024,          /* dn3000m mono high res */
        gpr_$color2_1024x800x8,     /* dn3000e */  
        gpr_$bw5_1024x800           /* dn3000m mono low  res */
} gpr_$display_config_t;

            /* possible directions for arcs drawn using gpr_$arc_c2p */
typedef short enum {
        gpr_$arc_ccw,               /* counter-clockwise */
        gpr_$arc_cw                 /* clockwise */
} gpr_$arc_direction_t;

            /* possible options on co-incident points in gpr_$arc_c2p */
typedef short enum {
        gpr_$arc_draw_none,        /* draw nothing */
        gpr_$arc_draw_full         /* draw full circle */
} gpr_$arc_option_t;

            /* imaging vs interactive display formats */
typedef short enum {
        gpr_$interactive,
        gpr_$imaging_1024x1024x8,
        gpr_$imaging_512x512x24,
        gpr_$double_bufferx8,
        gpr_$imaging_1280x1024x24,
        gpr_$imaging_windowx24
} gpr_$imaging_format_t;

            /* unique number corresponding to display controller type */
typedef short enum {
        gpr_$ctl_none,         /* none or not applicable */
        gpr_$ctl_mono_1,       /* dn100/400/420/460 */
        gpr_$ctl_mono_2,       /* dn300/320/330 */
        gpr_$ctl_color_1,      /* dn600/660/550/560 */
        gpr_$ctl_color_2,      /* dn580 */
        gpr_$ctl_color_3,      /* dn570, dn570a */
        gpr_$ctl_color_4,      /* dn3000c */
        gpr_$ctl_mono_4,       /* dn3000m mono high res */
        gpr_$ctl_color_5,      /* dn3000e */
        gpr_$ctl_mono_5        /* dn3000m mono low res */
} gpr_$controller_type_t;

            /* unique number corresponding to graphics accelerator processor type */
typedef short enum {
        gpr_$accel_none,        /* none or not applicable */
        gpr_$accel_1            /* 3dga */
} gpr_$accelerator_type_t;

            /* kinds of overlaps between different classes of buffer memory */
typedef short enum {
        gpr_$hdm_with_bitm_ext,     
        gpr_$hdm_with_buffers,      
        gpr_$bitm_ext_with_buffers  
} gpr_$memory_overlap_t;

            /* legal allocated sizes of pixel cells in bitmap sections for direct access */
typedef short enum {
        gpr_$alloc_1,       /* one bit per pixel cell (i.e. by-plane) */
        gpr_$alloc_2,       /* two bits per pixel cell */
        gpr_$alloc_4,       /* four bits per pixel cell */
        gpr_$alloc_8,       /* one byte per pixel cell */
        gpr_$alloc_16,      /* two bytes per pixel cell */
        gpr_$alloc_32       /* four bytes per pixel cell */
} gpr_$access_allocation_t;

            /* kinds of color map implementation */
typedef short enum {
        gpr_$no_invert,         /* not applicable, eg, color or no display */
        gpr_$invert_simulate,   /* simulated in software */
        gpr_$invert_hardware    /* implemented in hardware */
} gpr_$disp_invert_t;

typedef short enum {
        gpr_$rgb_none,
        gpr_$rgb_24             /* Modes for separate values for RGB */
} gpr_$rgb_modes_t;

typedef short gpr_$rgb_modes_set_t;



            /* list of display device characteristics */
typedef struct {
        gpr_$controller_type_t controller_type;    /* type of graphics controller */
        gpr_$accelerator_type_t accelerator_type;  /* type of graphics accelerator */ 
        short x_window_origin;           /* x origin of window screen area in pixels */ 
        short y_window_origin;           /* y origin of window screen area in pixels */ 
        short x_window_size;             /* x dimension of window screen area in pixels */ 
        short y_window_size;             /* y dimension of window screen area in pixels */ 
        short x_visible_size;            /* x dimension of visible screen area in pixels */ 
        short y_visible_size;            /* y dimension of visible screen area in pixels */ 
        short x_extension_size;          /* x dimension of maximum extended bitmap size in pixels */ 
        short y_extension_size;          /* y dimension of maximum extended bitmap size in pixels */ 
        short x_total_size;              /* x dimension of total buffer area in pixels */ 
        short y_total_size;              /* y dimension of total buffer area in pixels */ 
        short x_pixels_per_cm;           /* number of pixels in x dimension per centimeter */ 
        short y_pixels_per_cm;           /* number of pixels in y dimension per centimeter */ 
        short n_planes;                  /* number of planes available */ 
        short n_buffers;                 /* number of display buffers available */ 
        short delta_x_per_buffer;        /* relative displacement of buffers in x */ 
        short delta_y_per_buffer;        /* relative displacement of buffers in y */ 
        short delta_planes_per_buffer;   /* relative displacement of buffers in depth */ 
        unsigned short mem_overlaps;     /* set of overlaps among classes of buffer memory */ 
        short x_zoom_max;                /* maximum pixel-replication zoom factor for x */ 
        short y_zoom_max;                /* maximum pixel-replication zoom factor for y */ 
        short video_refresh_rate;        /* refresh rate in hz */ 
        short n_primaries;               /* number of primary colors (1 -> monochrome; 3 -> color */ 
        short lut_width_per_primary;     /* number of bits in possible shortensity values per primary */ 
        unsigned short avail_formats;    /* set of available shorteractive/imaging formats */ 
        unsigned short avail_access;     /* set of available pixel sizes for direct access */ 
        short access_address_space;      /* number of 1kb pages of address space available for direct access */ 
        gpr_$disp_invert_t invert;       /* INVert implemention */
        short num_lookup_tables;         /* Number of color lookup tables */
        gpr_$rgb_modes_set_t rgb_color;  /* Modes for separate values for RGB */
} gpr_$disp_char_t;

            /* bitmap coordinates */
typedef short gpr_$coordinate_t;

            /* lists of bitmap coordinates */
typedef gpr_$coordinate_t gpr_$coordinate_array_t[16384];

            /* bitmap positions */
typedef struct {
            gpr_$coordinate_t x_coord, y_coord;
} gpr_$position_t;

            /* bitmap offsets */
typedef struct {
            gpr_$coordinate_t x_size, y_size;
} gpr_$offset_t;

            /* windows on a bitmap */
typedef struct {
            gpr_$position_t window_base;
            gpr_$offset_t window_size;
} gpr_$window_t;

typedef gpr_$window_t gpr_$window_list_t[gpr_$default_list_size];

            /* horizontal line segments */
typedef struct {
            gpr_$coordinate_t x_coord_l, x_coord_r, y_coord;
} gpr_$horiz_seg_t;

            /* trapezoids with horizontal bases:  defined as 2 horizontal
              line segments, top and bottom */
typedef struct {
            gpr_$horiz_seg_t top, bot;
} gpr_$trap_t;

            /* lists of trapezoids with horizontal bases */
typedef gpr_$trap_t gpr_$trap_list_t[gpr_$default_list_size];

            /* triangles */
typedef struct {
            gpr_$position_t p1, p2, p3;
            short           winding_no;
} gpr_$triangle_t;

            /* lists of triangles */
typedef gpr_$triangle_t gpr_$triangle_list_t[gpr_$default_list_size];

            /* filling winding number specifications */
typedef short enum {
         gpr_$parity, 
         gpr_$nonzero, 
         gpr_$specific 
} gpr_$winding_set_t;

            /* triangle filling criteria */ 
typedef struct {
         gpr_$winding_set_t  wind_type;
         short               winding_no
} gpr_$triangle_fill_criteria_t;

            /* area decomposition techniques */ 
typedef short enum {
         gpr_$fast_traps,
         gpr_$precise_traps,
         gpr_$non_overlapping_tris,
         gpr_$render_exact
} gpr_$decomp_technique_t;

            /* graphics primitive strings */
typedef char gpr_$string_t[gpr_$string_size];

            /* bitmap plane numbers */
typedef unsigned short gpr_$plane_t;
typedef unsigned short gpr_$rgb_plane_t;

            /* pointer to main mem bitmap */
typedef char *gpr_$plane_ptr_t;

            /* bitmap plane masks (bit vector) */
typedef short gpr_$mask_t; 
typedef linteger gpr_$mask_32_t; 

            /* color values */
typedef linteger gpr_$color_t;

            /* pixel values */
typedef linteger gpr_$pixel_value_t;

            /* arrays of color values */
typedef gpr_$color_t gpr_$color_vector_t[256];

            /* arrays of pixel values */
typedef gpr_$pixel_value_t gpr_$pixel_array_t[131073];

            /* raster operation opcodes */
typedef unsigned short gpr_$raster_op_t;

            /* arrays of raster operation opcodes */
typedef gpr_$raster_op_t gpr_$raster_op_array_t[32];

            /* raster op prim set elements */ 
typedef short enum { gpr_$rop_blt, gpr_$rop_line, gpr_$rop_fill } gpr_$rop_prim_set_elems_t;

            /* raster op prim set */
typedef short gpr_$rop_prim_set_t;

            /* scalar type for directions */
typedef short enum {gpr_$up, gpr_$down, gpr_$left, gpr_$right} gpr_$direction_t;

            /* line drawing styles */
typedef short enum {gpr_$solid, gpr_$dotted} gpr_$linestyle_t;

typedef short gpr_$line_pattern_t[4];

            /* attribute block descriptors */
typedef linteger gpr_$attribute_desc_t;

            /* bitmap descriptors */
typedef linteger gpr_$bitmap_desc_t;

typedef struct {
    short major,minor;
} gpr_$version_t;


typedef struct {
    short n_sects;
    short pixel_size;
    short allocated_size;
    short bytes_per_line;
    linteger       bytes_per_sect;
    char          *storage_offset;
} gpr_$bmf_group_header_t;

typedef gpr_$bmf_group_header_t gpr_$bmf_group_header_array_t[gpr_$max_bmf_group+1];

typedef short enum {gpr_$create,gpr_$update, gpr_$write, gpr_$readonly} gpr_$access_mode_t;

typedef linteger gpr_$virt_dev_id_t;

typedef struct {
    short n_sects;
    short pixel_size;
    short allocated_size;
    short bytes_per_line;
    linteger       bytes_per_sect;
    char          *storage_offset;
} gpr_$group_header_t;

typedef short enum {gpr_$undisturbed_buffer, gpr_$clear_buffer, gpr_$copy_buffer} gpr_$double_buffer_option_t;

typedef union {
            struct {
                linteger dial_number;
                linteger dial_value;
            } gpr_$dial;
        } gpr_$event_data_t;

#eject
/*  initialization and termination.  */



/*  gpr_$init initializes the graphics primitive package.  */

std_$call void   gpr_$init ();



/*  gpr_$terminate terminates this package's operation.  */

std_$call void   gpr_$terminate ();
#eject
/*  set and inquire operations for the display.  */

/*  gpr_$inq_config returns the current display configuration.  */

std_$call void   gpr_$inq_config ();



/* gpr_$inq_disp_characterisitics returns the current display characterisitcs */

std_$call void   gpr_$inq_disp_characteristics ();



/*  gpr_$set_color_map gives new values for the color map.  */

std_$call void   gpr_$set_color_map ();



/*  gpr_$inq_color_map returns current values in the color map.  */

std_$call void   gpr_$inq_color_map ();



/*  gpr_$set_bitmap_file_color_map stores a color map with a bitmap file */

std_$call void   gpr_$set_bitmap_file_color_map ();



/*  gpr_$inq_bitmap_file_color_map returns the color map stored with a bitmap file */

std_$call void   gpr_$inq_bitmap_file_color_map ();



/*  gpr_$set_cursor_pattern loads a cursor pattern.  */

std_$call void   gpr_$set_cursor_pattern ();



/*  gpr_$set_cursor_active specifies whether the cursor should be on or off.  */

std_$call void   gpr_$set_cursor_active ();



/*  gpr_$set_cursor_position gives the position at which the cursor is
   to be displayed, in the current bitmap.  */

std_$call void   gpr_$set_cursor_position ();


/* gpr_$set_origin gives the cursor-relative psoition of its pixel
   which is to be placed at the cursor position.  */

std_$call void   gpr_$set_cursor_origin ();


/*  gpr_$inq_cursor returns information about the cursor.  */

std_$call void   gpr_$inq_cursor ();



/*  gpr_$wait_frame causes the color display hardware to defer processing
   further requests until the next end-of-frame.  */

std_$call void   gpr_$wait_frame ();
#eject
/*  bitmap control functions.  */



/*  gpr_$set_bitmap establishes the current bitmap for subsequent operations.  */

std_$call void   gpr_$set_bitmap ();



/*  gpr_$inq_bitmap returns a descriptor of the current bitmap.  */

std_$call void   gpr_$inq_bitmap ();



/*  gpr_$allocate_bitmap allocates a bitmap in main memory.  */

std_$call void   gpr_$allocate_bitmap ();



/*  gpr_$allocate_bitmap_nc allocates a bitmap in main memory without zeroing it.  */

std_$call void   gpr_$allocate_bitmap_nc ();



/*  gpr_$allocate_hdm_bitmap allocates a bitmap in hidden_display memory.  */

std_$call void   gpr_$allocate_hdm_bitmap ();



/*  gpr_$open_bitmap_file obtains access to an external bitmap  */

std_$call void gpr_$open_bitmap_file ();



/*  gpr_$deallocate_bitmap deallocates a bitmap allocated by allocate_bitmap.  */

std_$call void   gpr_$deallocate_bitmap ();



/*  gpr_$inq_bitmap_pointer returns a pointer to the storage/display memory
   for the given bitmap and the number of words each scan line occupies.
   this information can be used to directly manipulate the bits in the bitmap.  */

std_$call void   gpr_$inq_bitmap_pointer ();



/*  gpr_$inq_bitmap_position returns the position of the upper left corner of
   the specified bitmap. this is normally the screen position, although it does
   have some significance for main memory bitmaps.  it is not meaningful if the
   bitmap is a dm pad (i.e. a frame mode bitmap).                               */

std_$call void  gpr_$inq_bitmap_position ();



/*  gpr_$inq_bm_bit_offset returns the number of bits in the most significant
   part of the first word of each scanline which are not part of the given bitmap.
   in other words, the offset is the number of bits between a 16-bit word boundary
   and the left edge of the bitmap.
   currently this number can only be nonzero for direct graphics bitmaps.  */

std_$call void   gpr_$inq_bm_bit_offset ();



/*  gpr_$select_color_frame selects whether frame 0 (top 1024 lines) or
   frame 1 (bottom 1024 lines) is visible.  normally frame 0 is visible.  */

std_$call void   gpr_$select_color_frame ();



/*  gpr_$remap_color_memory sets the plane of frame 0 of color display memory
   (normally visible) which is mapped at the address returned by inq_bitmap_pointer.  */

std_$call void   gpr_$remap_color_memory ();



/*  gpr_$remap_color_memory_1 sets the plane of frame 1 of color display memory
   (normally hidden) which is mapped at the address returned by inq_bitmap_pointer.  */

std_$call void   gpr_$remap_color_memory_1 ();



/*  gpr_$color_zoom sets the zoom scale factors for the color display.  */

std_$call void   gpr_$color_zoom ();



/*  gpr_$enable_direct_access waits for display hardware to finish current
   operations so that the user can access display memory directly.  */

std_$call void   gpr_$enable_direct_access ();



/*  gpr_$set_bitmap_dimensions changes the size and number of planes
   of the given bitmap.  */

std_$call void   gpr_$set_bitmap_dimensions ();



/*  gpr_$inq_bitmap_dimensions returns the size and number of planes
   of the given bitmap.  */

std_$call void   gpr_$inq_bitmap_dimensions ();



/*  gpr_$allocate_attribute_block allocates an attribute block,
   initialized to default settings.  */

std_$call void   gpr_$allocate_attribute_block ();



/*  gpr_$deallocate_attribute_block deallocates an attribute block allocated
   by allocate_attribute_block.  */

std_$call void   gpr_$deallocate_attribute_block ();



/*  gpr_$set_attribute_block establishes the given attributes as the attributes
   of the current bitmap.  */

std_$call void   gpr_$set_attribute_block ();



/*  gpr_$attribute_block returns as the function value a descriptor of
   the attributes of the given bitmap.  */

std_$call gpr_$attribute_desc_t gpr_$attribute_block ();
#eject
/*  set and inquire operations for bitmap attributes.  */

std_$call void   gpr_$raster_op_prim_set ();

std_$call void   gpr_$inq_raster_op_prim_set ();


/*  gpr_$set_attribute sets an attribute in the current bitmap.
   the list of calls follows.  */

std_$call void   gpr_$set_clip_window ();

std_$call void   gpr_$set_clipping_active ();

std_$call void   gpr_$set_text_font ();

std_$call void   gpr_$set_text_path ();

std_$call void   gpr_$set_coordinate_origin ();

std_$call void   gpr_$set_plane_mask ();

std_$call void   gpr_$set_draw_value ();

std_$call void   gpr_$set_text_value ();

std_$call void   gpr_$set_text_background_value ();

std_$call void   gpr_$set_fill_value ();

std_$call void   gpr_$set_fill_background_value ();

std_$call void   gpr_$set_fill_pattern ();

std_$call void   gpr_$set_raster_op ();

std_$call void   gpr_$set_linestyle ();

std_$call void   gpr_$set_line_pattern ();

std_$call void   gpr_$set_draw_pattern ();

std_$call void   gpr_$set_draw_width ();

std_$call void   gpr_$set_character_width ();

std_$call void   gpr_$set_horizontal_spacing ();

std_$call void   gpr_$set_space_size ();

std_$call void   gpr_$set_raster_op_mask ();



/*  gpr_$inq_attributes returns the current settings of a group
   of attributes for the current bitmap.  */

std_$call void   gpr_$inq_constraints ();

std_$call void   gpr_$inq_text ();

std_$call void   gpr_$inq_text_path ();

std_$call void   gpr_$inq_coordinate_origin ();

std_$call void   gpr_$inq_draw_value ();

std_$call void   gpr_$inq_text_values ();

std_$call void   gpr_$inq_fill_value ();

std_$call void   gpr_$inq_fill_background_value ();

std_$call void   gpr_$inq_fill_pattern ();

std_$call void   gpr_$inq_raster_ops ();

std_$call void   gpr_$inq_linestyle ();

std_$call void   gpr_$inq_line_pattern ();

std_$call void   gpr_$inq_draw_pattern ();

std_$call void   gpr_$inq_line_width ();

std_$call void   gpr_$inq_character_width ();

std_$call void   gpr_$inq_horizontal_spacing ();

std_$call void   gpr_$inq_space_size ();

std_$call void   gpr_$inq_plane_mask_32 ();

std_$call void   gpr_$inq_background ();

std_$call void   gpr_$inq_foreground ();

#eject
/*  drawing operations.  */



/*  gpr_$move sets the cp to the given position.  */

std_$call void   gpr_$move ();



/*  gpr_$inq_cp returns the current position.  */

std_$call void   gpr_$inq_cp ();



/*  gpr_$line draws a line from the cp to the given position
   and sets the cp to the given position.  */

std_$call void   gpr_$line ();



/*  gpr_$polyline does a series of lines, starting from the cp.  */

std_$call void   gpr_$polyline ();



/*  gpr_$multiline does a series of alternate moves and lines. */

std_$call void   gpr_$multiline ();



/*  gpr_$draw_box draws a rectangular box  */

std_$call void   gpr_$draw_box ();



/*  gpr_$arc_3p draw an arc from current point through two points p2 and p3. */

std_$call void   gpr_$arc_3p ();



/*  gpr_$arc_c2p draw an arc centered at point `center', starting
    at current point, going clockwise or counterclockwise according
    to `direction', and ending at point p2, or at the line going through
    `center' and p2.  The current position is updated to the actual
    end point. */

std_$call void   gpr_$arc_c2p ();



/*  gpr_$circle draws a circle of radius around point center. */

std_$call void   gpr_$circle ();



/*  gpr_$circle_filled draws a filled circle of radius around point center. */

std_$call void   gpr_$circle_filled ();



/*  gpr_$spline_cubic_p draws a parametric cubic spline through the control points. */

std_$call void   gpr_$spline_cubic_p ();



/*  gpr_$spline_cubic_x draws a cubic spline as a function of x through the control points. */

std_$call void   gpr_$spline_cubic_x ();



/*  gpr_$spline_cubic_y draws a cubic spline as a function of y through the control points. */

std_$call void   gpr_$spline_cubic_y ();                       
#eject
/*  text operations.  */



/*  gpr_$load_font_file loads a font contained in a file into an appro-
   priate area (based on the current display mode and configuration).  */

std_$call void   gpr_$load_font_file ();



/*  gpr_$unload_font_file unloads a font that has been loaded by
   load_font_file.  */

std_$call void   gpr_$unload_font_file ();



/*  gpr_$text writes text to a bitmap from the current position ("cp").  */

std_$call void   gpr_$text ();



/*  gpr_$inq_text_extent returns the x- and y-offsets the given string
   would span if written with text.  */

std_$call void   gpr_$inq_text_extent ();



/*  gpr_$inq_text_offset returns the x- and y-offsets that must be added to the
   coordinates of the desired upper left pixel of the string to give the pixel
   from which the text call should be made, and the x_offset of the pixel
   which would be the updated cp.  */

std_$call void   gpr_$inq_text_offset ();



/*  gpr_$replicate_font creates and loads a read/write copy of the original font. */

std_$call void   gpr_$replicate_font ();
#eject
/*  data transfer operations.  */



/*  gpr_$clear clears the current bitmap to a given pixel value.  */

std_$call void   gpr_$clear ();



/*  gpr_$read_pixels reads the pixels from the given window of the current
   bitmap and stores them in a pixel array.  */

std_$call void   gpr_$read_pixels ();



/*  gpr_$write_pixels writes the pixels from a pixel array into the given
   window of the current bitmap.  */

std_$call void   gpr_$write_pixels ();
#eject
/*  blt operations.  */



/*  gpr_$pixel_blt moves a rectangle of whole pixels from the source
   bitmap to a position in the current bitmap.  */

std_$call void   gpr_$pixel_blt ();


/*  gpr_$bit_blt moves a rectangle of bits from a plane of the source
   bitmap to a position in a plane of the current bitmap.  */

std_$call void   gpr_$bit_blt ();


/*  gpr_$additive_blt moves a rectangle of bits from a plane of the source
   bitmap to a position in every plane of the current bitmap.  */

std_$call void   gpr_$additive_blt ();
#eject
/*  fill operations.  */



/*  gpr_$rectangle fills a rectangle in the current bitmap.  */

std_$call void   gpr_$rectangle ();



/*  gpr_$trapezoid fills a trapezoid in the current bitmap.  */

std_$call void   gpr_$trapezoid ();



/*  gpr_$multitrapezoid fills a list of trapezoids in the current bitmap.  */

std_$call void   gpr_$multitrapezoid ();



/*  gpr_$triangle fills a triangle in the current bitmap.  */

std_$call void   gpr_$triangle ();



/*  gpr_$multitriangle fills a list of triangles in the current bitmap.  */

std_$call void   gpr_$multitriangle ();



/*  gpr_$start_pgon starts a polygon boundary loop.  */

std_$call void   gpr_$start_pgon ();



/*  gpr_$pgon_polyline defines a series of line segments as part of
   the current polygon boundary loop.  */

std_$call void   gpr_$pgon_polyline ();



/*  gpr_$close_fill_pgon closes and fills the currently open polygon.  */

std_$call void   gpr_$close_fill_pgon ();



/*  gpr_$close_return_pgon closes the currently open polygon,
   decomposes it, and returns the list of trapezoids.  */

std_$call void   gpr_$close_return_pgon ();



/*  gpr_$close_return_pgon_tri closes the currently open polygon,
   decomposes it, and returns the list of triangles.  */

std_$call void   gpr_$close_return_pgon_tri ();



/* gpr_$pgon_decomp_technique establishes the polygon decomposition technique
   to be used.  */

std_$call void   gpr_$pgon_decomp_technique ();

/* gpr_$inq_pgon_decomp_technique returns the polygon decomposition technique
   being used.  */

std_$call void   gpr_$inq_pgon_decomp_technique ();


std_$call void   gpr_$set_triangle_fill_criteria ();
std_$call void   gpr_$inq_triangle_fill_criteria ();
#eject


/* direct graphics */


std_$call void gpr_$set_acq_time_out();


/* gpr_set_obscured_opt sets obscured selection attribute for display in
  direct mode */

std_$call void gpr_$set_obscured_opt();


/* gpr_$acquire_display gives the user exclusive access to all display operations
  in the acquired window */

std_$call boolean gpr_$acquire_display();


/* gpr_$inq_vis_list returns list of visible subwindows when a window is obscured */

std_$call void gpr_$inq_vis_list();


/* gpr_$force_release releases the display regardless of how many times it
  was previously acquired */

std_$call void gpr_$force_release();


/* gpr_$release_display will release the display only if acq_rel_cnt is 1 */

std_$call void gpr_$release_display();


/* gpr_$set_refresh_entry provides two procedures which will refresh the user's
  window and refresh hidden display memory */

std_$call void gpr_$set_refresh_entry();


/* gpr_$inq_refresh_entry returns the two procedures which will refresh the
   window and refresh hidden display memory.  */

std_$call void   gpr_$inq_refresh_entry ();


/* gpr_$set_auto_refresh tells dm to save bitmap and refresh screen when needed */

std_$call void gpr_$set_auto_refresh();


/* gpr_$event_wait hangs until input or time out; returns boolean to indicate
  obscured window */

std_$call boolean gpr_$event_wait();


/* gpr_$cond_event_wait returns immediately and reports if any input */

std_$call boolean gpr_$cond_event_wait();


/* gpr_$enable_input enables event type and selected set of keys to be
   recognized by event_wait */

std_$call void gpr_$enable_input();


/* gpr_$disable_input disables event type */

std_$call void gpr_$disable_input();


/* gpr_$get_ec gets event count */

std_$call void gpr_$get_ec();

/* gpr_$set_input_sid establishes the stream id for gpr input in frame mode
   if not standard input */

std_$call void gpr_$set_input_sid();

/* gpr_$set_window_id sets the character identifying the current displayed
   bitmap for input identification purposes */

std_$call void gpr_$set_window_id();

/* gpr_$inq_window_id returns the character identifying the current displayed
   bitmap for input identification purposes */

std_$call void gpr_$inq_window_id();

#eject

/* imaging format calls */

/* gpr_$set_imaging_format sets the dn600 display format */

std_$call void gpr_$set_imaging_format();

/* gpr_$inq_imaging_format returns the current dn600 display format */

std_$call void gpr_$inq_imaging_format();
#eject

/* blank timeout calls */

/* gpr_$set_blank_timeout sets the timeout value for screen blanking */

std_$call void   gpr_$set_blank_timeout ();

/* gpr_$inq_blank_timeout returns the timeout value for screen blanking */

std_$call void   gpr_$inq_blank_timeout ();
#eject
    
/* gpr_$return_virt_dev_id returns the virtual device id */

std_$call void gpr_$return_virt_dev_id();

/* gpr_$validate_virt_dev restores the virtual device to the  attributes 
   that the gpr state has saved */

std_$call void gpr_$validate_virt_dev();

/* GPR_$SET_PLANE_MASK_32 sets the plane mask for a rgb bitmap */
std_$call void gpr_$set_plane_mask_32();

std_$call void gpr_$allocate_buffer();

std_$call void gpr_$deallocate_buffer();

std_$call void gpr_$select_display_buffer();

std_$call void gpr_$inq_visible_buffer();
               
std_$call void gpr_$make_bitmap_from_array();

/* gpr_$inq_event_data returns additional event data */
std_$call void gpr_$inq_event_data();

#endif /* gpr_$gpr_ins_c */
         
#eject

