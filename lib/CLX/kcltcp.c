/* 
 * stream interface to tcp (and unix domain) for kcl for 4.3BSD
 *
 * Copyright (C) 1987 Massachussetts Institute of Technology 
 *
 * Roman Budzianowski - Project Athena/MIT
 *
 */

/* compile command:
 * 
 * 	cc -c kcltcp.c -DVAX -DMAXPAGE=16384 -DVSSIZE=8152 -I${KCLHDIR}
 *
 *      where KCLHDIR is the h subdirectory in the kcl distribution
 *
 */

#include "include.h"

object
open_tcp_stream(host,display)
     object host;		/* host name */
     object display;		/* display number */
{
   object streamTcp;
   int fd;			/* file descriptor */
   int i;
   char hname[BUFSIZ];
   int displaynumber;
   FILE *fout, *fin;
   object streamIn, streamOut, make_stream();

   if (type_of(host) != t_string)
     FEerror("~S is wrong type for host (should be string).",1,host);

   if(type_of(display) != t_fixnum)
     FEerror("~S is wrong type for display (should be integer).",1,display);

   if (host->st.st_fillp > BUFSIZ - 1)
     too_long_file_name(host);
   for (i = 0;  i < host->st.st_fillp;  i++)
     hname[i] = host->st.st_self[i];
   hname[i] = '\0';

   displaynumber = (int) fix(display);

   fd = connect_to_server(hname,displaynumber); 

   if(fd < 0)
     return Cnil;

   streamIn = make_stream(host,fd,smm_input);
   streamOut = make_stream(host,fd,smm_output);

   streamTcp = make_two_way_stream(streamIn,streamOut);

   return(streamTcp);
}

object make_stream(host,fd,smm)
     object host;		/* not really used */
     int fd;			/* file descriptor */
     enum smmode smm;		/* lisp mode */
{
   object stream;
   char *mode;			/* file open mode */
   FILE *fp;			/* file pointer */
   vs_mark;

   switch(smm){
    case smm_input:
      mode = "r";
      break;
    case smm_output:
      mode = "w";
      break;
    default:
      FEerror("make_stream : wrong mode");
   }
   
   fp = fdopen(fd,mode);

   stream = alloc_object(t_stream);
   stream->sm.sm_mode = (short)smm;
   stream->sm.sm_fp = fp;
   fp->_base = BASEFF; 
   stream->sm.sm_object0 = Sstring_char;
   stream->sm.sm_object1 = host;
   stream->sm.sm_int0 = stream->sm.sm_int1 = 0;
   vs_push(stream);
   setbuf(fp, alloc_contblock(BUFSIZ)); 
   vs_reset;
   return(stream);
}
