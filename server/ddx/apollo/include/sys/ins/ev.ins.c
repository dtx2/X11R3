/* EV.INS.C, /sys/ins, jrw, 03/11/85
   environment variable manipulation package

   Changes:
      03/11/85 jrw  /sys/ins version created.
      07/22/84 jrw  original coding.  */

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


#define ev_$string_max      256     /* max name/value string len */

typedef char ev_$string_t [ev_$string_max] ;    /* name/value string type */
typedef ev_$string_t *ev_$string_ptr_t ;        /* pointer to above */



/* Procedures and Functions: */


/* EV_$SET_VAR sets the named variable to the given value.  If a variable
 * with the same name exists, it is overwritten.  If namelen is less than
 * or equal to zero, or the variable is privileged (such as USER) it returns
 * false, otherwise it returns true.
 *
 * boolean
 * ev_$set_var (name, namelen, value, valuelen)  
 * char name [] ;                                   * variable name *
 * short namelen ;                                  * name length *
 * char value [] ;                                  * value string *
 * short valuelen ;                                 * value length *
 */
std_$call boolean ev_$set_var () ;



/* EV_$GET_VAR returns the value associated with the named variable.
 * True is returned as the function value if the variable exists, false
 * is returned otherwise.
 *
 * boolean
 * ev_$get_var (name, namelen, value_p, valuelen)   * returns non-zero if found *
 * char name [] ;                                   * variable name *
 * short namelen ;                                  * name length *
 * ev_$string_ptr_t value_p ;                       * ptr to value *
 * short valuelen ;                                 * value length *
 */
std_$call boolean ev_$get_var () ;



/* EV_$EXIST_VAR returns true if the given name exists, false otherwise.
 *
 * boolean
 * ev_$exist_var (name, namelen)                    * returns non-zero if found *
 * char name [] ;                                   * variable name *
 * short namelen ;                                  * name length *
 */
std_$call boolean ev_$exist_var () ;



/* EV_$DELETE_VAR deletes the named variable.  *
 *
 * boolean
 * ev_$delete_var (name, namelen)                   * returns non-zero if found *
 * char name [] ;                                   * variable name *
 * short namelen ;                                  * name length *
 */
std_$call boolean ev_$delete_var () ;



/* EV_$READ_VAR_ENTRY reads the environment variable specified by the index
 * from the list and returns its name and value.  The first index is 1.
 *
 * boolean
 * ev_$read_var_entry (ix, name_p, namelen, value_p, valuelen) * returns non-zero if found *
 * short ix ;                                       * variable index *
 * ev_$string_ptr_t name_p ;                        * pointer to name *
 * short namelen ;                                  * name length *
 * ev_$string_ptr_t value_p ;                       * pointer to value *
 * short valuelen ;                                 * value length *
 */
std_$call boolean ev_$read_var_entry () ;

#eject
