{  FONT.PAS, /us/lib/font, jrw, 03/27/86
   font file manager

******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.
     
                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************

   Changes:
      02/27/87 schulert get full pathname of font, to allow update thru links
      08/22/86 schulert change max_chars_in_font from 127 to 128
      03/27/86 gms  eliminated compiler warnings.
      03/16/84 dpg  changed open not to check any raster line limits for font.
      03/16/84 dpg  exported update procedure.
      03/16/84 dpg  changed open to make a temporary copy, update to modify the
                    temporary copy, and close to do a make_permanent.
      12/31/83 tvc  increased max number of fonts to 3
      12/26/82 jrw  corrected treatment of char-code 0 image in "update" procedure.
      04/04/82 jrw  added font_$string_length.
      03/22/82 jrw  added %apollo directive.
      03/20/82 jrw  close now deletes font file if we created it, and no update was
                    specified
      01/02/82 jrw  added update argument to font_$close.
      12/07/81 jrw  original coding.  }


MODULE  font ;


DEFINE  font_$init,
        font_$create,
        font_$open,
        font_$update,
        font_$close,
        font_$get_value,
        font_$set_value,
        font_$get_char_info,
        font_$get_char_image,
        font_$set_char_info,
        font_$set_char_image,
        font_$delete_char,
        font_$image_required,
        font_$string_length ;


%apollo ;

%nolist ;
%include '/sys/ins/base.ins.pas' ;
%include '/sys/ins/name.ins.pas' ;
%include '/sys/ins/ms.ins.pas' ;
%include '/sys/ins/smdu.ins.pas' ;
%include '/sys/ins/rws.ins.pas' ;
%list ;


%include 'apcfont.p.h' ;
{  Module-wide, private declarations:  }


CONST   max_fid = 3 ;
        max_font_size = font_$max_font_size ;
        max_chars_in_font = 128 ;

        max_char_width = 224 ;
        max_line_width = 224 ;
        max_v_spacing = 1022 ;
        max_h_spacing = 1022 ;

        font_version_no    = 1 ;
        default_h_spacing  = 1 ;
        default_v_spacing  = 3 ;
        default_space_size = 1 ;

        nil_thread = -1 ;               { in char_desc linked list }
        page_size = 1024;

TYPE    create_char_t = (create_char_cc, must_exist_cc) ;

        char_desc_t = RECORD
            valid:  boolean ;                       { true if this entry valid }
            image_required:  boolean ;              { true if image update reqd }
            info:  font_$char_info_t ;              { up, down, left, right, width }
            x_pos:  integer ;                       { x position in original font tbl }
            y_pos:  integer ;                       { y position in original font tbl }
            new_x_pos:  integer ;                   { x position in new font tbl }
            new_y_pos:  integer ;                   { y position in new font tbl }
            image_p:  font_$char_image_ptr_t ;      { to image }
            x_dim:  integer ;                       { image x-dimension, words }
            link:  integer                          { to next entry }
            END ;

        fid_entry_t = PACKED RECORD
            font_p:  smd_$font_table_ptr_t ;         { mapped font file pointer }
            n_chars:  integer ;                     { # chars in font }
            len_mapped:  linteger ;                 { font file length mapped }
            rw:  font_$rw_key_t ;                   { read or read-write }
            create:  boolean ;                      { true if we've created font file }
            orig_image_p:  font_$char_image_ptr_t ; { copied font image }
            c_h_spacing:  integer ;                 { current horiz spacing }
            c_v_spacing:  integer ;                 { current vert spacing }
            c_space_size:  integer ;                { current space size }
            c_max_up:  integer ;                    { ditto... }
            c_max_down:  integer ;
            c_max_right:  integer ;
            c_max_height:  integer ;
            c_max_width:  integer ;
            desc:  ARRAY [0..127] OF char_desc_t ;  { individual character descriptors }
            font_name:  name_$pname_t ;             { name of font file }
            font_name_len:  integer ;               { length of the name }
            END ;


VAR     fid_list:  STATIC ARRAY [1..max_fid] OF fid_entry_t ;
        mask:  ARRAY [1..16] OF pinteger ;
%eject ;
{  GET_FONT_ID searches the font table for a free entry and returns its index.  Zero
   is returned and the status set to table_full if no entry is available.  }


FUNCTION    get_font_id (
                OUT status:  status_$t               { returned status }
            ):  font_$id_t ;  INTERNAL ;


VAR     i:  integer ;


BEGIN
    FOR i := 1 TO max_fid DO
        IF fid_list [i].font_p = nil THEN BEGIN
            get_font_id := i ;
            status.all := 0 ;
            RETURN
            END ;

    status.all := font_$int_table_full ;
    get_font_id := 0

END ;   { get_font_id }






{  UNMAP unmaps a font and frees the font id list entry.  The caller must ensure that
   the font id is valid.  }


PROCEDURE   unmap (
                IN  fid:  font_$id_t
            ) ;  INTERNAL ;


VAR     status:  status_$t ;



BEGIN
    WITH fid_list [fid] DO BEGIN
        ms_$unmap (font_p, len_mapped, status) ;
        font_p := nil
        END

END ;   { unmap }
%eject ;
{  VERIFY_FID verifies a font id.  A non-zero status is returned if the font id
   is invalid.  }


PROCEDURE   verify_fid (
                IN  fid:  font_$id_t ;              { font id to verify }
                IN  rw_key:  font_$rw_key_t ;       { read or read-write access reqd }
                OUT status:  status_$t               { returned status }
            ) ;  INTERNAL ;


BEGIN
    IF (fid >= 1) AND (fid <= max_fid) THEN
        IF fid_list [fid].font_p <> nil THEN BEGIN
            IF (rw_key = font_$read) OR (fid_list [fid].rw = font_$read_write) THEN
                status.all := 0
            ELSE
                status.all := font_$read_only ;
            RETURN
            END ;

    status.all := font_$bad_id

END ;   { verify_fid }






{  VERIFY_CODE verifies that the character code is valid.  }


PROCEDURE   verify_code (
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                IN  create_char:  create_char_t ;   { create or must exist }
                OUT status:  status_$t               { returned status }
            ) ;  INTERNAL ;


BEGIN
    WITH fid_list [fid], font_p^ DO
        IF (code >= 0) AND (code <= 127) THEN
            IF (create_char = create_char_cc) OR
               desc [code].valid THEN BEGIN
                status.all := 0 ;
                RETURN
                END ;

    status.all := font_$not_in_font

END ;   { verify_code }
%eject ;
{  RECOMPUTE_HEADER recomputes the maximum h/w/u/d/r values to be saved in the header.
   Also, the number of characters in the font is recomputed.  }


PROCEDURE   recompute_header (
                IN  fid:  font_$id_t
            ) ;  INTERNAL ;


VAR     code:  integer ;


BEGIN
    WITH fid_list [fid] DO BEGIN
        n_chars := 0 ;
        FOR code := 0 TO 127 DO WITH desc [code]: d  DO
            IF d.valid THEN BEGIN
                n_chars := n_chars + 1 ;

                IF n_chars = 1 THEN BEGIN
                    c_max_height := d.info.up + d.info.down ;
                    c_max_width := d.info.width ;
                    c_max_up := d.info.up ;
                    c_max_down := d.info.down ;
                    c_max_right := d.info.right
                    END

                ELSE BEGIN
                    IF d.info.up + d.info.down > c_max_height THEN
                        c_max_height := d.info.up + d.info.down ;
                    IF d.info.width > c_max_width THEN c_max_width := d.info.width ;
                    IF d.info.up > c_max_up THEN c_max_up := d.info.up ;
                    IF d.info.down > c_max_down THEN c_max_down := d.info.down ;
                    IF d.info.right > c_max_right THEN c_max_right := d.info.right
                    END   { not 1st char }
                END ;   { char exists }

        IF n_chars = 0 THEN BEGIN   { zero maxima if no characters }
            c_max_height := 0 ;
            c_max_width := 0 ;
            c_max_up := 0 ;
            c_max_down := 0 ;
            c_max_right := 0
            END
        END   { with }

END ;   { recompute_header }
%eject ;
{  EXTW transforms a signed 8-bit value into a signed 16 bit value.  }


FUNCTION    extw (
                IN  c:  char                        { value to be transformed }
            ):  integer ;  INTERNAL ;


VAR     i:  pinteger ;


BEGIN
    i := ord (c) ;
    IF i <= 127 THEN extw := i
    ELSE extw := i - 256

END ;   { extw }
%eject ;
{  INIT_FID_ENTRY initializes the font id table entry for a font.  We're called by
   the create and open procedures.  Copy the font image, so that the descriptor
   table can be rebuilt.  Also, initialize the character descriptor table in the
   fid entry.  }


FUNCTION    init_fid_entry (
                IN  fid:  font_$id_t                { font id }
            ):  boolean ;  INTERNAL ;


VAR     i:  integer ;
        j:  integer ;
        sp:  RECORD CASE integer OF
            1:  (p:  font_$char_image_ptr_t) ;
            2:  (u:  univ_ptr) ;
            3:  (l:  linteger)
            END ;
        nw, windex:  integer32 ;


BEGIN
    WITH fid_list [fid], font_p^ DO BEGIN
        n_chars := 0 ;
        c_h_spacing := h_spacing ;
        c_v_spacing := v_spacing ;
        c_space_size := space_size ;
        create := false ;

        FOR i := 0 TO 127 DO WITH desc [i]: d  DO BEGIN    { initialize our descriptor table }
            j := ord (index_table [i]) ;
            IF j = 0 THEN d.valid := false
            ELSE WITH desc_table [j]: dt DO BEGIN
                d.valid := true ;
                d.image_required := false ;
                d.info.left := extw (dt.left) ;
                d.info.right := extw (dt.right) ;
                d.info.up := extw (dt.up) ;
                d.info.down := extw (dt.down) ;
                d.info.width := ord (dt.width) ;
                d.x_pos := ord (dt.x_pos) ;
                d.y_pos := dt.y_pos ;
                d.new_x_pos := 0 ;
                d.new_y_pos := 0 ;
                d.image_p := nil ;
                d.x_dim := 0
                END   { with }
            END ;   { for }

        recompute_header (fid) ;   { compute current maxima, # chars in font }


        nw := (font_p^.image_size + 1) DIV 2 ;   { setup to copy font image }
        sp.u := font_p ;
        sp.l := sp.l + font_p^.image_offset ;

        orig_image_p := rws_$alloc_rw (nw * 2) ;   { get space in free storage }
        IF orig_image_p = nil THEN BEGIN
            init_fid_entry := false ;
            RETURN
            END ;

        FOR windex := 1 TO nw DO
            orig_image_p^ [windex] := sp.p^ [windex]
        END ;

    init_fid_entry := true

END ;   { init_fid_entry }
%eject ;
{  FONT_$INIT initializes the font manager.  }


PROCEDURE   font_$init ;


VAR     i:  integer ;


BEGIN
    FOR i := 1 TO max_fid DO fid_list [i].font_p := nil ;

    mask [1] := 16#8000 ;
    FOR i := 2 TO 16 DO mask [i] := mask [i - 1] + rshft (16#8000, i - 1)

END ;   { font_$init }
%eject ;
{  FONT_$CREATE creates a font file and opens it.  Returned is the font id on which the
   file is open.  The file will contain default header info.  }

PROCEDURE   font_$create (*
                IN  name:  UNIV name_$pname_t ;     { font file name }
                IN  len:  pinteger ;                { name length }
                OUT fid:  font_$id_t ;              { font id }
                OUT status:  status_$t               { returned status }
            *) ;


VAR     i:  integer ;

BEGIN
    fid := get_font_id (status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid] DO BEGIN

        FOR i := 1 TO len DO font_name [i] := name [i] ;
        font_name_len := len ;

        font_p := ms_$crmapl (font_name, font_name_len, 0, max_font_size, ms_$nr_xor_1w, status) ;
        IF status.all <> 0 THEN BEGIN
            font_p := nil ;
            RETURN
            END ;

        len_mapped := max_font_size;
        rw := font_$read_write ;

        WITH font_p^ DO BEGIN
            version := font_version_no ;
            image_offset := smd_$font_header_size + smd_$font_index_table_size ;
            chars_in_font := 0 ;
            raster_lines := 0 ;
            image_size := 0 ;
            max_width := 0 ;
            max_height := 0 ;
            v_spacing := default_v_spacing ;
            h_spacing := default_h_spacing ;
            space_size := default_space_size ;
            max_right := 0 ;
            max_up := 0 ;
            max_down := 0 ;
            FOR i := 0 TO 127 DO
                index_table [i] := chr (0) ;

            ms_$truncate (font_p, image_offset, status) ;
            IF status.all <> 0 THEN BEGIN
                unmap (fid) ;
                RETURN
                END
            END   { with font_p^ }
        END ;   { with fid_list [fid] }

    IF NOT init_fid_entry (fid) THEN
        status.all := font_$no_room ;

    fid_list [fid].create := true

END ;   { font_$create }
%eject ;
{  FONT_$OPEN opens a font file and returns the font id.  }

PROCEDURE   font_$open (*
                IN  name:  UNIV name_$pname_t ;     { font file name }
                IN  len:  pinteger ;                { name length }
                IN  rw_key:  font_$rw_key_t ;       { read or read-write }
                OUT fid:  font_$id_t ;              { font id }
                OUT status:  status_$t               { returned status }
            *) ;


CONST
        dir_string = '/sys/dm/fonts/' ;
        dir_string_len = sizeof(dir_string) ;
        name_string_len = name_$pnamlen_max - dir_string_len ;

VAR     i:  integer32 ;
        font_pname: PACKED RECORD CASE boolean OF
            false: (two_part: PACKED RECORD
                        dir_part: ARRAY [1..dir_string_len] OF char ;
                        name_part: ARRAY [1..name_string_len] OF char
                        END) ;
            true:  (one_part: name_$pname_t)
            END ;
        font_pname_len: pinteger ;
        orig_p:  smd_$font_table_ptr_t ;
        fp, op:  RECORD CASE integer OF
            1:  (p:  font_$char_image_ptr_t) ;
            2:  (u:  univ_ptr)
            END ;
        nw:  integer32 ;


BEGIN
    fid := get_font_id (status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid] DO BEGIN
        FOR i := 1 TO len DO font_pname.one_part [i] := name [i] ;
        font_pname_len := len ;
        font_p := ms_$mapl (font_pname, font_pname_len, 0, max_font_size, ms_$nr_xor_1w, ms_$r, false,
                            len_mapped, status) ;
        IF (status.all = name_$not_found) AND
           (name [1] IN ['a'..'z', 'A'..'Z', '$', '\']) THEN WITH font_pname.two_part DO BEGIN
            dir_part := dir_string ;
            FOR i := 1 TO len DO name_part [i] := name [i] ;
            font_pname_len := dir_string_len + len ;
            font_p := ms_$mapl (font_pname, font_pname_len, 0, max_font_size, ms_$nr_xor_1w, ms_$r, false,
                                len_mapped, status) ;
            END ;

        IF status.all <> 0 THEN BEGIN
            font_p := nil ;
            RETURN
            END ;

        name_$get_path (font_pname.one_part, font_pname_len,
                        font_name,           font_name_len,  status) ;
        IF status.all <> 0 THEN BEGIN
            unmap (fid) ;
            RETURN
            END ;

        WITH font_p^ DO BEGIN
            IF version <> font_version_no THEN BEGIN
                unmap (fid) ;
                status.all := font_$bad_version ;
                RETURN
                END ;

            IF (chars_in_font > max_chars_in_font) OR
               (max_width > max_char_width) OR
               (v_spacing > max_v_spacing) OR
               (h_spacing > max_h_spacing) OR
               (max_right > max_char_width) THEN BEGIN

                unmap (fid) ;
                status.all := font_$bad_font_file ;
                RETURN
                END

            END ;   { with font_p^ }

        rw := rw_key ;
        IF rw = font_$read_write THEN BEGIN
            orig_p := font_p ;
            font_p := ms_$crtemp (font_name, font_name_len, 0, len_mapped, ms_$nr_xor_1w, status) ;
            IF status.all <> 0 THEN BEGIN
                font_p := orig_p ;
                unmap (fid) ;
                RETURN
                END ;

            op.u := orig_p ;
            fp.u := font_p ;
            nw := (len_mapped + 1) DIV 2 ;
            FOR i := 1 TO nw DO
                fp.p^ [i] := op.p^ [i] ;

            ms_$unmap (orig_p, len_mapped, status)
            END ;
        END ;   { with fid_list }

    IF NOT init_fid_entry (fid) THEN
        status.all := font_$no_room

END ;   { font_$open }
%eject ;
{  FONT_$GET_VALUE returns a value from the font file header.  }


PROCEDURE   font_$get_value (*
                IN  fid:  font_$id_t ;              { font id }
                IN  value_id:  font_$value_id_t ;   { which value, please }
                OUT value:  linteger ;              { returned value }
                OUT status:  status_$t               { returned status }
            *) ;


BEGIN
    verify_fid (fid, font_$read, status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid], font_p^ DO CASE value_id OF

font_$version:
        value := version ;

font_$chars_in_font:
        value := n_chars ;

font_$raster_lines:
        value := raster_lines ;

font_$image_size:
        value := image_size ;

font_$max_height:
        value := c_max_height ;

font_$max_width:
        value := c_max_width ;

font_$v_spacing:
        value := c_v_spacing ;

font_$h_spacing:
        value := c_h_spacing ;

font_$space_size:
        value := c_space_size ;

font_$max_right:
        value := c_max_right ;

font_$max_up:
        value := c_max_up ;

font_$max_down:
        value := c_max_down ;


OTHERWISE
        status.all := font_$bad_value_id

        END   { case }

END ;   { font_$get_value }
%eject ;
{  FONT_$SET_VALUE writes a caller-supplied value into the font file header.  }


PROCEDURE   font_$set_value (*
                IN  fid:  font_$id_t ;              { font id }
                IN  value_id:  font_$value_id_t ;   { which value, please }
                IN  value:  linteger ;              { value }
                OUT status:  status_$t               { returned status }
            *) ;


BEGIN
    verify_fid (fid, font_$read_write, status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid]  DO CASE value_id OF

font_$v_spacing:
        c_v_spacing := value ;

font_$h_spacing:
        c_h_spacing := value ;

font_$space_size:
        c_space_size := value ;


OTHERWISE
        status.all := font_$bad_value_id

        END   { case }

END ;   { font_$set_value }
%eject ;
{  FONT_$GET_CHAR_INFO returns the descriptor table entry for a given character.  }


PROCEDURE   font_$get_char_info (*
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT info:  font_$char_info_t ;      { returned character info }
                OUT status:  status_$t               { returned status }
            *) ;


BEGIN
    verify_fid (fid, font_$read, status) ;
    IF status.all <> 0 THEN RETURN ;

    verify_code (fid, code, must_exist_cc, status) ;
    IF status.all <> 0 THEN RETURN ;

    info := fid_list [fid].desc [code].info

END ;   { font_$get_char_info }
%eject ;
{  EXTRACT_IMAGE extracts a font image and returns it in the given array.
   The caller must ensure that the array is large enough to hold the entire image.  }


PROCEDURE   extract_image (
                IN  image_p:  font_$char_image_ptr_t ;  { char image base address }
                IN  x_start:  integer ;             { x-offset, from start of raster line }
                IN  y_start:  integer ;             { y-offset, from 'top' raster line }
                IN  image_width:  integer ;         { # words in 1 image raster line }
                IN  c_width:  integer ;             { character width, bits }
                IN  c_height:  integer ;            { character height, bits }
                OUT c_image:  UNIV font_$char_image_t ; { returned image }
                IN  c_image_width:  integer         { width of supplied char image, words }
            ) ;  INTERNAL ;


VAR     ip:  RECORD CASE integer OF
            1:  (p:  font_$char_image_ptr_t) ;
            2:  (l:  linteger) ;
            END ;

        start_bit:  integer ;
        word:  pinteger ;
        i:  integer ;
        source_ix:  integer ;
        dest_ix:  integer ;
        n_bits:  integer ;


{  First, compute a pointer to the word containing the first bit of the character.  }

BEGIN
    ip.p := image_p ;
    ip.l := ip.l + (y_start * image_width * 2) + ((x_start DIV 16) * 2) ;
    start_bit := x_start MOD 16 ;   { starting bit #, l -> r, 0 -> 15 }


{  Copy each line.  }

    FOR i := 0 TO c_height - 1 DO BEGIN
        dest_ix := (i * c_image_width) + 1 ;
        source_ix := 1 ;
        n_bits := c_width ;

        REPEAT
            word := lshft (ip.p^ [source_ix], start_bit) ;   { get 1st part of next word }
            source_ix := source_ix + 1 ;
            IF n_bits > (16 - start_bit) THEN    { if # bits left to go > 16 and... }
                IF start_bit > 0 THEN            { didn't start at the 1st bit, ... }
                    word := word + rshft (ip.p^ [source_ix], 16 - start_bit) ;
                                                 { ...get info from next word }

            IF n_bits < 16 THEN
                word := word & mask [n_bits] ;
            n_bits := n_bits - 16 ;
            c_image [dest_ix] := word ;
            dest_ix := dest_ix + 1
            UNTIL n_bits <= 0 ;

        ip.l := ip.l + (image_width * 2)   { point to 1st word on next raster line }
        END

END ;   { extract_image }
%eject ;
{  FONT_$GET_CHAR_IMAGE returns the character image for a given character code.  }

PROCEDURE   font_$get_char_image (*
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT image:  UNIV font_$char_image_t ;   { returned char image }
                IN  max_width:  pinteger ;          { max width (implies dimension) }
                IN  max_height:  pinteger ;         { max height }
                OUT status:  status_$t               { returned status }
            *) ;


VAR     c_width:  integer ;
        c_height:  integer ;
        c_image_width:  integer ;

BEGIN
    verify_fid (fid, font_$read, status) ;
    IF status.all <> 0 THEN RETURN ;

    verify_code (fid, code, must_exist_cc, status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid].desc [code] DO BEGIN
        c_width := info.left + info.right ;
        c_height := info.up + info.down
        END ;

    IF (c_width > max_width) OR (c_height > max_height) THEN BEGIN
        status.all := font_$char_too_big ;
        RETURN
        END ;

    c_image_width := (max_width + 15) DIV 16 ;   { calc dimension of supplied array }

    WITH fid_list [fid], font_p^, desc [code]: d  DO BEGIN
        IF d.image_required THEN BEGIN   { no up-to-date image }
            status.all := font_$no_image ;
            RETURN
            END ;

        IF (d.image_p <> nil) THEN BEGIN   { extract from updated image }
            extract_image (d.image_p, 0, 0, d.x_dim, c_width, c_height,
                           image, c_image_width) ;
            RETURN
            END ;

        extract_image (orig_image_p, d.x_pos, d.y_pos, 14, c_width, c_height,
                       image, c_image_width)
        END   { with }

END ;   { font_$get_char_image }
%eject ;
{  SAVE_IMAGE saves a character image in the font table or in free storage.  }


PROCEDURE   save_image (
                IN  image_p:  font_$char_image_ptr_t ;  { char image base address }
                IN  x_start:  integer ;             { x-offset, from start of raster line }
                IN  y_start:  integer ;             { y-offset, from 'top' raster line }
                IN  image_width:  integer ;         { # words in 1 image raster line }
                IN  c_width:  integer ;             { character width, bits }
                IN  c_height:  integer ;            { character height, bits }
                IN  c_image:  UNIV font_$char_image_t ; { image to save }
                IN  c_image_width:  integer         { width of supplied char image, words }
            ) ;  INTERNAL ;


VAR     ip, ip_save:  RECORD CASE integer OF
            0:  (p:  font_$char_image_ptr_t) ;
            1:  (ip:  ^integer) ;
            2:  (lp:  ^linteger) ;
            3:  (l:  linteger) ;
            END ;

        start_bit:  integer ;
        word:  pinteger ;
        i:  integer ;
        source_ix:  integer ;
        n_bits:  integer ;
        iword:  pinteger ;
        imask:  pinteger ;
        lword:  integer32 ;
        lmask:  integer32 ;


{  First, compute a pointer to the word containing the first bit of the character.  }

BEGIN
    ip.p := image_p ;
    ip.l := ip.l + (y_start * image_width * 2) + ((x_start DIV 16) * 2) ;
    start_bit := x_start MOD 16 ;   { starting bit #, l -> r, 0 -> 15 }


{  Copy each line.  }

    FOR i := 0 TO c_height - 1 DO BEGIN
        source_ix := (i * c_image_width) + 1 ;
        n_bits := c_width ;
        ip_save := ip ;

        REPEAT
            IF n_bits >= 16 THEN lmask := 16#FFFF
            ELSE lmask := mask [n_bits] ;
            lword := c_image [source_ix] & lmask ;
            lword := lshft (lword, 16 - start_bit) ;
            lmask := 16#FFFFFFFF - lshft (lmask, 16 - start_bit) ;

            IF start_bit + n_bits >= 16 THEN   { need to touch 2 words }
                ip.lp^ := (ip.lp^ & lmask) ! lword
            ELSE BEGIN      { touch only 1 word }
                iword := rshft (lword, 16) ;
                imask := rshft (lmask, 16) ;
                ip.ip^ := (ip.ip^ & imask) ! iword
                END ;

            source_ix := source_ix + 1 ;
            ip.l := ip.l + 2 ;
            n_bits := n_bits - 16
            UNTIL n_bits <= 0 ;

        ip.l := ip_save.l + (image_width * 2)
        END

END ;   { save_image }
%eject ;
{  FONT_$SET_CHAR_INFO sets the character info for the given character.  }


PROCEDURE   font_$set_char_info (*
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                IN  info:  font_$char_info_t ;      { character info }
                OUT status:  status_$t               { returned status }
            *) ;


VAR     image_required:  boolean ;
        c_height:  integer ;
        c_width:  integer ;



FUNCTION    abs (
                IN  i:  integer
            ):  integer ;

BEGIN
    IF i >= 0 THEN abs := i ELSE abs := -i

END ;   { abs }



{  FONT_$SET_CHAR_INFO.VERIFY_INFO compares the info supplied by the caller with
   the existing character info and sets the "image_required" flag if the width
   or height is being changed.  }


PROCEDURE   verify_info (
                IN  cur_info:  font_$char_info_t ;
                OUT image_required:  boolean
            ) ;


BEGIN
    image_required := ((cur_info.left + cur_info.right) <> (info.left + info.right)) OR
                      ((cur_info.up + cur_info.down) <> (info.up + info.down))

END ;   { font_$set_char_info.verify_info }
%eject ;
{  FONT_$SET_CHAR_INFO starts here.  }

BEGIN
    verify_fid (fid, font_$read_write, status) ;
    IF status.all <> 0 THEN RETURN ;

    verify_code (fid, code, create_char_cc, status) ;
    IF status.all <> 0 THEN BEGIN
        status.all := font_$invalid_code ;
        RETURN
        END ;


{ Verify values in caller-supplied info.  }

    WITH info DO BEGIN
        IF (abs (left) > 127) OR (abs (right) > 127) OR
           (abs (up) > 127) OR (abs (down) > 127) OR
           (width > 127) THEN BEGIN
            status.all := font_$value_out_of_range ;
            RETURN
            END ;

        c_height := left + right ;
        c_width := up + down ;
        IF (c_height <= 0) OR (c_height > 224) OR
           (c_width <= 0) OR (c_width > 224) THEN BEGIN
            status.all := font_$value_out_of_range ;
            RETURN
            END
        END ;


{  Save info, determine whether a new image is required:  }

    WITH fid_list [fid], font_p^, desc [code]: d  DO BEGIN
        IF d.valid THEN BEGIN   { char exists now }
            verify_info (d.info, image_required) ;
            d.image_required := d.image_required OR image_required ;
            d.info := info
            END

        ELSE IF n_chars >= max_chars_in_font THEN BEGIN   { font is full }
            status.all := font_$table_full ;
            RETURN
            END

        ELSE BEGIN
            n_chars := n_chars + 1 ;   { through here if char is to be created }
            d.valid := true ;
            d.image_required := true ;
            d.info := info ;
            d.x_pos := 0 ;
            d.y_pos := 0 ;
            d.new_x_pos := 0 ;
            d.new_y_pos := 0 ;
            d.image_p := nil ;
            d.x_dim := 0
            END ;

        IF d.info.right > c_max_right THEN c_max_right := d.info.right ;
        IF d.info.up > c_max_up THEN c_max_up := d.info.up ;
        IF d.info.down > c_max_down THEN c_max_down := d.info.down ;
        IF d.info.width > c_max_width THEN c_max_width := d.info.width ;
        IF d.info.up + d.info.down > c_max_height THEN c_max_height := d.info.up + d.info.down

        END   { with }

END ;   { font_$set_char_info }
%eject ;
{  FONT_$SET_CHAR_IMAGE sets the image for the specified character.  }


PROCEDURE   font_$set_char_image (*
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                IN  image:  UNIV font_$char_image_t ;   { char image }
                IN  width:  pinteger ;              { width, rasters (implies dimension) }
                IN  height:  pinteger ;             { height, rasters }
                OUT status:  status_$t               { returned status }
            *) ;


VAR     c_width:  pinteger ;
        c_height:  pinteger ;


BEGIN
    verify_fid (fid, font_$read_write, status) ;
    IF status.all <> 0 THEN RETURN ;

    verify_code (fid, code, must_exist_cc, status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid], font_p^, desc [code]: d  DO BEGIN

        c_width := d.info.left + d.info.right ;   { compute actual character w & h }
        c_height := d.info.up + d.info.down ;

        IF (width < c_width) OR (height < c_height) THEN BEGIN
            status.all := font_$value_out_of_range ;
            RETURN
            END ;

        d.image_required := false ;

        d.x_dim := (c_width + 15) DIV 16 ;
        d.image_p := rws_$alloc_rw (d.x_dim * c_height * 2) ;
        extract_image (addr (image), 0, 0, ((width + 15) DIV 16), c_width, c_height,
                       d.image_p^, d.x_dim)
        END   { with }

END ;   { font_$set_char_image }
%eject ;
{  FONT_$DELETE_CHAR deletes a character from the font table.  }


PROCEDURE   font_$delete_char (*
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT status:  status_$t               { returned status }
            *) ;


BEGIN
    verify_fid (fid, font_$read_write, status) ;
    IF status.all <> 0 THEN RETURN ;

    verify_code (fid, code, must_exist_cc, status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid] DO BEGIN
        desc [code].valid := false ;   { wipe out descriptor table entry }
        n_chars := n_chars - 1
        END ;   { with }

    recompute_header (fid)

END ;   { font_$delete_char }
%eject ;
{  UPDATE updates the font file to contain all changes made by the caller.  Our caller
   is responsible for ensuring that the font file is open for read/write.  }


PROCEDURE   update (
                IN  fid:  font_$id_t ;              { font id }
                OUT status:  status_$t               { returned status }
            ) ;  INTERNAL ;


VAR     code:  integer ;
        last_code:  integer ;
        i:  integer ;
        nc:  integer ;
        thread:  integer ;
        c_width:  integer ;
        c_height:  integer ;
        line_width:  integer ;
        line_height:  integer ;
        line_no:  integer ;
        c_image_offset:  integer ;
        new_image_p:  font_$char_image_ptr_t ;
        ip:  PACKED RECORD CASE integer OF
            1:  (p:  font_$char_image_ptr_t) ;
            2:  (u:  univ_ptr) ;
            3:  (l:  linteger)
            END ;

        image:  ARRAY [1..224] OF ARRAY [1..14] OF pinteger ;



{  Ensure that there are no outstanding partial updates.  Also, link used
   characters (for sort, later).  }

BEGIN
    WITH fid_list [fid], font_p^  DO BEGIN
        thread := nil_thread ;
        FOR code := 0 TO 127 DO WITH desc [code]: d  DO
            IF d.valid THEN   { there's a character here }
                IF d.image_required THEN BEGIN
                    status.all := font_$new_image_required ;
                    RETURN
                    END

                ELSE BEGIN
                    IF thread = nil_thread THEN thread := code
                    ELSE desc [last_code].link := code ;
                    last_code := code ;
                    d.link := nil_thread
                    END ;



{  Update vertical and horizontal spacing, and space size.  }

        h_spacing := c_h_spacing ;
        v_spacing := c_v_spacing ;
        space_size := c_space_size ;


{  Compute image offset, new image pointer.  Also, truncate file and zero-fill
   end of 1st page.  }

        c_image_offset := smd_$font_header_size + smd_$font_index_table_size +
                          (smd_$v1_fdte_size * n_chars) ;
        ip.u := font_p ;
        ip.l := ip.l + c_image_offset ;
        new_image_p := ip.p ;

        ms_$truncate (font_p, page_size, status) ;
        IF status.all <> 0 THEN RETURN ;
        IF c_image_offset < page_size THEN BEGIN
            i := (page_size - c_image_offset) DIV 2 ;
            FOR i := 1 TO i DO new_image_p^ [i] := 0
            END ;


{  Generate new image:  }

        code := thread ;
        line_no := 0 ;
        REPEAT   { for each line }
            line_height := 0 ;
            line_width := 0 ;

            WHILE code <> nil_thread DO WITH desc [code]: d  DO BEGIN   { for each character }
                c_width := d.info.left + d.info.right ;
                c_height := d.info.up + d.info.down ;
                IF c_width + line_width > max_line_width THEN EXIT ;   { won't fit }

                IF d.image_p <> nil THEN   { use updated image }
                    save_image (new_image_p, line_width, line_no, 14, c_width,
                                c_height, d.image_p^, d.x_dim)

                ELSE BEGIN   { use original image }
                    extract_image (orig_image_p, d.x_pos, d.y_pos, 14, c_width,
                                   c_height, image, 14) ;
                    save_image (new_image_p, line_width, line_no, 14, c_width,
                                c_height, image, 14)
                    END ;   { use orig image }

                d.new_x_pos := line_width ;   { save for later implant into descr table }
                d.new_y_pos := line_no ;
                line_width := line_width + c_width ;
                IF line_height < c_height THEN line_height := c_height ;
                code := d.link
                END ;   { for each char on this line }

            IF line_width = 0 THEN EXIT ;   { all done }
            line_no := line_no + line_height
            UNTIL false ;


{  Rebuild index and descriptor tables:  }

        nc := 0 ;
        FOR code := 0 TO 127 DO WITH desc [code]: d  DO
            IF NOT d.valid THEN index_table [code] := chr (0)
            ELSE BEGIN
                nc := nc + 1 ;   { bump character count }
                index_table [code] := chr (nc) ;
                WITH desc_table [nc]: dt  DO BEGIN
                    dt.left := chr (d.info.left & 16#FF) ;
                    dt.right := chr (d.info.right & 16#FF) ;
                    dt.up := chr (d.info.up & 16#FF) ;
                    dt.down := chr (d.info.down & 16#FF) ;
                    dt.width := chr (d.info.width & 16#FF) ;
                    dt.x_pos := chr (d.new_x_pos & 16#FF) ;
                    dt.y_pos := d.new_y_pos
                    END
                END ;   { char exists }


{  Rebuild header:  }

        chars_in_font := n_chars ;
        raster_lines := line_no ;
        IF line_no <= (65535 DIV 28) THEN
            image_size := line_no * 28
        ELSE image_size := 65535 ;
        image_offset := c_image_offset ;
        recompute_header (fid) ;

        max_height := c_max_height ;
        max_width := c_max_width ;
        max_right := c_max_right ;
        max_up := c_max_up ;
        max_down := c_max_down ;

        ms_$truncate (font_p, image_offset + image_size, status)

        END   { with }

END ;   { update }
%eject ;
{  FONT_$UPDATE updates the in-memory copy of a font file.  }


PROCEDURE   font_$update (*
                IN  fid:  font_$id_t ;              { font id }
                OUT status:  status_$t               { returned status }
            *) ;
BEGIN
    verify_fid (fid, font_$read_write, status) ;
    IF status.all <> 0 THEN RETURN ;
    update (fid, status)

END ;   { font_$update }
%eject ;
{  FONT_$CLOSE closes a font file.  }


PROCEDURE   font_$close (*
                IN  fid:  font_$id_t ;              { font id }
                IN  update_file:  boolean ;         { true to update file }
                OUT status:  status_$t               { returned status }
            *) ;


BEGIN
    verify_fid (fid, font_$read, status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid] DO BEGIN
        IF update_file AND (rw = font_$read_write) THEN BEGIN
            update (fid, status) ;
            IF (status.all = 0) AND NOT create THEN
                ms_$mk_permanent(font_p, [ms_$mk_bak], font_name, font_name_len, status) ;
            IF status.all <> 0 THEN RETURN
            END ;

        unmap (fid) ;

        IF (NOT update_file) AND create THEN
           name_$delete_file (font_name, font_name_len, status)

        END ;

END ;   { font_$close }
%eject ;
{  FONT_$IMAGE_REQUIRED is a boolean function which returns true if the
   image for the specified character is required before the font file is
   closed.  }


FUNCTION    font_$image_required (*
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT status:  status_$t
            ):  boolean *) ;


BEGIN
    verify_fid (fid, font_$read, status) ;
    IF status.all <> 0 THEN RETURN ;

    verify_code (fid, code, must_exist_cc, status) ;
    IF status.all <> 0 THEN RETURN ;

    WITH fid_list [fid].desc [code] DO
        font_$image_required := valid AND image_required

END ;   { font_$image_required }
%eject ;
{  FONT_$STRING_LENGTH determines the number of pixels occupied in the horizontal
   direction by the given string.  Also, the required starting offset is returned.
   (This is greater than zero only if the character string extends to the left of
   the margin.)  }


PROCEDURE   font_$string_length (*
                IN  fid:  font_$id_t ;              { font id }
                IN  str:  UNIV string ;             { string to measure }
                IN  len:  integer ;                 { length of string }
                OUT npix:  integer ;                { # pixels used }
                OUT offs:  integer ;                { offset for left margin }
                OUT status:  status_$t               { returned status }
            *) ;


VAR     i:  integer ;
        info:  font_$char_info_t ;


BEGIN
    verify_fid (fid, font_$read, status) ;
    IF status.all <> 0 THEN RETURN ;

    offs := 0 ;
    npix := 0 ;

    WITH fid_list [fid] DO FOR i := 1 TO len DO BEGIN
        font_$get_char_info (fid, ord (str [i]), info, status) ;

        IF status.all = font_$not_in_font THEN
            npix := npix + c_space_size

        ELSE IF status.all <> 0 THEN RETURN

        ELSE WITH info DO BEGIN
            IF left > npix THEN BEGIN    { offset is required }
                offs := offs + (left - npix) ;
                npix := npix + (left - npix)
                END ;

            npix := npix + width + c_h_spacing
            END   { with }
        END ;   { for }

    status.all := 0

END ;   { font_$string_length }
