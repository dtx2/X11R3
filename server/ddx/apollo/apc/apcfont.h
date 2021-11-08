/*  FONT.INS.CPAS, /us/ins, po, 04/01/86
   font file manager include file

   Changes:
       04/01/86  ers Change enum to short enum.
       02/05/85  po  created from font.ins.pas
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

/*

%IFDEF us_font_ins_pas %THEN
    %EXIT
%ELSE
    %VAR us_font_ins_pas
%ENDIF
*/

#define        font_$int_table_full         0x04090001 
#define        font_$bad_font_file          0x04090002 
#define        font_$bad_version            0x04090003 
#define        font_$bad_id                 0x04090004 
#define        font_$bad_value_id           0x04090005 
#define        font_$not_in_font            0x04090006 
#define        font_$no_room                0x04090007 
#define        font_$char_too_big           0x04090008 
#define        font_$value_out_of_range     0x04090009 
#define        font_$invalid_code           0x0409000A 
#define        font_$no_image               0x0409000B 
#define        font_$read_only              0x0409000C 
#define        font_$new_image_required     0x0409000D 
#define        font_$table_full             0x0409000E 

#define        font_$max_font_size          131072 


typedef short   font_$id_t;           

typedef short enum {font_$version, font_$chars_in_font, font_$raster_lines,
                 font_$image_size, font_$max_height, font_$max_width,
                 font_$h_spacing, font_$v_spacing, font_$space_size,
                 font_$max_right, font_$max_up, font_$max_down} 
                 font_$value_id_t;

typedef struct {              /* character descriptor */
         short     left;
         short     right;
         short     up;
         short     down;
         pinteger  width;
         }  font_$char_info_t;

typedef pinteger font_$char_image_t[font_$max_font_size/2];  /* linearized character image */
typedef pinteger *font_$char_image_ptr_t;

typedef short enum  {font_$read, font_$read_write} font_$rw_key_t;



/*  FONT_$INIT initializes the font manager.  */

std_$call void   font_$init () ;  



/*  FONT_$CREATE creates a font file and opens it.  Returned is the font id on which the
   file is open.  The file will contain default header info.  */

std_$call void   font_$create () ;  



/*  FONT_$OPEN opens a font file and returns the font id.  */

std_$call void   font_$open () ;  



/*  FONT_$UPDATE updates the in-memory copy of a font file.  */

std_$call void   font_$update () ;  



/*  FONT_$CLOSE closes a font file.  */

std_$call void   font_$close () ;  



/*  FONT_$GET_VALUE returns a value from the font file header.  */

std_$call void   font_$get_value () ;  



/*  FONT_$SET_VALUE writes a caller-supplied value into the font file header.  */

std_$call void   font_$set_value () ;  



/*  FONT_$GET_CHAR_INFO returns the descriptor table entry for a given character.  */

std_$call void   font_$get_char_info () ;  



/*  FONT_$GET_CHAR_IMAGE returns the character image for a given character code.  */

std_$call void   font_$get_char_image () ;  



/*  FONT_$SET_CHAR_INFO sets the character info for the given character.  */

std_$call void   font_$set_char_info ( ) ;  



/*  FONT_$SET_CHAR_IMAGE sets the image for the specified character.  */

std_$call void   font_$set_char_image () ;  



/*  FONT_$DELETE_CHAR deletes a character from the font table.  */

std_$call void   font_$delete_char () ;  



/*  FONT_$IMAGE_REQUIRED is a boolean function which returns true if the
   image for the specified character is required before the font file is
   closed.  */

std_$call boolean  font_$image_required ();



/*  FONT_$STRING_LENGTH determines the number of pixels occupied in the horizontal
   direction by the given string.  Also, the required starting offset is returned.
   (This is greater than zero only if the character string extends to the left of
   the margin.)  */

std_$call void   font_$string_length () ;  

#eject
