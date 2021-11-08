{  FONT.INS.PAS, /us/ins, jrw, 08/25/83
   font file manager include file

   Changes:
      08/25/83 spf  added us_font_ins_pas declaration
      04/04/82 jrw  added font_$string_length.
      12/07/81 jrw  original coding.  }

{ ******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.
     
                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
****************************************************************** }


%IFDEF us_font_ins_pas %THEN
    %EXIT
%ELSE
    %VAR us_font_ins_pas
%ENDIF

CONST   font_$int_table_full        = 16#04090001 ;
        font_$bad_font_file         = 16#04090002 ;
        font_$bad_version           = 16#04090003 ;
        font_$bad_id                = 16#04090004 ;
        font_$bad_value_id          = 16#04090005 ;
        font_$not_in_font           = 16#04090006 ;
        font_$no_room               = 16#04090007 ;
        font_$char_too_big          = 16#04090008 ;
        font_$value_out_of_range    = 16#04090009 ;
        font_$invalid_code          = 16#0409000A ;
        font_$no_image              = 16#0409000B ;
        font_$read_only             = 16#0409000C ;
        font_$new_image_required    = 16#0409000D ;
        font_$table_full            = 16#0409000E ;


        font_$max_font_size = 131072 ;


TYPE    font_$id_t = integer ;

        font_$value_id_t = (font_$version, font_$chars_in_font, font_$raster_lines,
                            font_$image_size, font_$max_height, font_$max_width,
                            font_$h_spacing, font_$v_spacing, font_$space_size,
                            font_$max_right, font_$max_up, font_$max_down) ;

        font_$char_info_t = RECORD      { character descriptor }
            left:  integer ;
            right:  integer ;
            up:  integer ;
            down:  integer ;
            width:  pinteger
            END ;

        font_$char_image_t = ARRAY [1..(font_$max_font_size DIV 2)] OF pinteger ;  { linearized character image }
        font_$char_image_ptr_t = ^font_$char_image_t ;

        font_$rw_key_t = (font_$read, font_$read_write) ;



{  FONT_$INIT initializes the font manager.  }

PROCEDURE   font_$init ;  EXTERN ;



{  FONT_$CREATE creates a font file and opens it.  Returned is the font id on which the
   file is open.  The file will contain default header info.  }

PROCEDURE   font_$create (
                IN  name:  UNIV name_$pname_t ;     { font file name }
                IN  len:  pinteger ;                { name length }
                OUT fid:  font_$id_t ;              { font id }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$OPEN opens a font file and returns the font id.  }

PROCEDURE   font_$open (
                IN  name:  UNIV name_$pname_t ;     { font file name }
                IN  len:  pinteger ;                { name length }
                IN  rw_key:  font_$rw_key_t ;       { read or read-write }
                OUT fid:  font_$id_t ;              { font id }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$UPDATE updates the in-memory copy of a font file.  }

PROCEDURE   font_$update (
                IN  fid:  font_$id_t ;              { font id }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$CLOSE closes a font file.  }

PROCEDURE   font_$close (
                IN  fid:  font_$id_t ;              { font id }
                IN  update_file:  boolean ;         { true to update file }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$GET_VALUE returns a value from the font file header.  }

PROCEDURE   font_$get_value (
                IN  fid:  font_$id_t ;              { font id }
                IN  value_id:  font_$value_id_t ;   { which value, please }
                OUT value:  linteger ;              { returned value }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$SET_VALUE writes a caller-supplied value into the font file header.  }

PROCEDURE   font_$set_value (
                IN  fid:  font_$id_t ;              { font id }
                IN  value_id:  font_$value_id_t ;   { which value, please }
                IN  value:  linteger ;              { value }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$GET_CHAR_INFO returns the descriptor table entry for a given character.  }

PROCEDURE   font_$get_char_info (
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT info:  font_$char_info_t ;      { returned character info }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$GET_CHAR_IMAGE returns the character image for a given character code.  }

PROCEDURE   font_$get_char_image (
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT image:  UNIV font_$char_image_t ;   { returned char image }
                IN  max_width:  pinteger ;          { max width, rasters (implies dimension) }
                IN  max_height:  pinteger ;         { max height, rasters }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$SET_CHAR_INFO sets the character info for the given character.  }

PROCEDURE   font_$set_char_info (
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                IN  info:  font_$char_info_t ;      { character info }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$SET_CHAR_IMAGE sets the image for the specified character.  }

PROCEDURE   font_$set_char_image (
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                IN  image:  UNIV font_$char_image_t ;   { char image }
                IN  width:  pinteger ;              { width, rasters (implies dimension) }
                IN  height:  pinteger ;             { height, rasters }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$DELETE_CHAR deletes a character from the font table.  }

PROCEDURE   font_$delete_char (
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;



{  FONT_$IMAGE_REQUIRED is a boolean function which returns true if the
   image for the specified character is required before the font file is
   closed.  }

FUNCTION    font_$image_required (
                IN  fid:  font_$id_t ;              { font id }
                IN  code:  integer ;                { character code }
                OUT status:  status_$t
            ):  boolean ;  EXTERN ;



{  FONT_$STRING_LENGTH determines the number of pixels occupied in the horizontal
   direction by the given string.  Also, the required starting offset is returned.
   (This is greater than zero only if the character string extends to the left of
   the margin.)  }

PROCEDURE   font_$string_length (
                IN  fid:  font_$id_t ;              { font id }
                IN  str:  UNIV string ;             { string to measure }
                IN  len:  integer ;                 { length of string }
                OUT npix:  integer ;                { # pixels used }
                OUT offs:  integer ;                { offset for left margin }
                OUT status:  status_$t               { returned status }
            ) ;  EXTERN ;

%eject ;
