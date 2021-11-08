/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: pages.c,v 1.3 89/01/06 18:42:26 kit Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   November 10 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: pages.c,v 4.5 88/12/19 13:48:09 kit Exp $";
#endif

#include <ctype.h>
#include "globals.h"

#define ADD_MORE_MEM 100
#define CHAR_PER_LINE 40

#define IS_HELP(w) ( XtParent(XtParent(w)) == help_widget )

/* 
 * A little design philosophy is probabally wise to include at this point.
 *
 * One of the things that I has hoped to achieve with xman is to make the
 * viewing of manpage not only easy for the nieve user, but also fast for
 * the experienced user, I wanted to be able to use it too.  To achieve this
 * I end up sacrificing a bit of start up time for the manual data structure.
 * As well as, the overhead of reading the entire file before putting it up 
 * on the display.  This is actually hardly even noticeable since most manual
 * pages are shots, one to two pages - the notable exception is of course csh,
 * but then that should be broken up anyway. 
 *
 * METHOD:
 *
 * I allocate a chunk of space that is the size of the file, plus a null for
 * debugging.  Then copiesthe file into this chunk of memory.  I then allocate
 * an array of char*'s that are assigned to the beginning of each line.  Yes,
 * this means that I have to read the file twice, and could probabally be more
 * clever about it, but once it is in memory the second read is damn fast.
 * There are a few obsucrities here about guessing the number of lines and
 * reallocing if I guess wrong, but other than that it is pretty straight 
 * forward.
 *
 *                                         Chris Peterson
 *                                         1/27/88
 */

/*	Function Name: InitManpage
 *	Description: This function reads a file and sets it up for 
 *                   display by PrintManpage.
 *	Arguments: man_globals - the psuedo global structure for this man page.
 *                 widget - the scrolled window widget.
 *                 file - a file pointer to the file that we are opening.
 *	Returns: none.
 */

void
InitManpage(man_globals,widget,file)
ManpageGlobals * man_globals;
Widget widget;
FILE * file;
{
  char *page,*top_of_page;	/* a pointer to the manpage, 
				   stored in dynamic memory. */
  char **line_pointer,**top_line; /* pointers to beginnings of the 
				     lines of the file. */
  int nlines;			/* the current number of allocated lines. */
  Arg arglist[5];		/* the arglist. */
  Cardinal num_args;		/* the number of arguments. */
  struct stat fileinfo;		/* file information from fstat. */
  MemoryStruct * memory_struct;	/* the memory information to pass to the 
				   next file. */
  
  memory_struct = man_globals->memory_struct;

  if ( memory_struct->top_line != NULL) {
    free(memory_struct->top_line);
    free(memory_struct->top_of_page);
  }

/*
 * Get file size and allocate a chunk of memory for the file to be 
 * copied into.
 */

  if (fstat(fileno(file), &fileinfo))
    PrintError("Failure in fstat");

  page = (char *) malloc(fileinfo.st_size + 1);	/* leave space for the NULL */
  if (page == NULL)
    PrintError("Could not allocate space for new manpage");
  top_of_page = page;

/* 
 * Allocate a space for a list of pointer to the beginning of each line.
 */

  nlines = fileinfo.st_size/CHAR_PER_LINE;
  line_pointer = (char**) malloc( nlines * sizeof(char *) );
  if (line_pointer == NULL)
    PrintError("Could not allocate space for new manpage");
  top_line = line_pointer;

  *line_pointer++ = page;

/*
 * Copy the file into memory. 
 */

  fread(page,sizeof(char),fileinfo.st_size,file); 

/* put NULL at end of buffer. */

  *(page + fileinfo.st_size) = '\0';

/*
 * Go through the file setting a line pointer to the character after each 
 * new line.  If we run out of line pointer space then realloc that space
 * with space for more lines.
 */

  while (*page != '\0') {

    if ( *page == '\n' ) {
      *line_pointer++ = page + 1;

      if (line_pointer >= top_line + nlines) {
	top_line = (char **) realloc( top_line, 
			      (nlines + ADD_MORE_MEM) * sizeof(char *) );
	line_pointer = top_line + nlines;
	nlines += ADD_MORE_MEM;
      }
    }
    page++;
  }
   
/*
 *  Realloc the line pointer space to take only the minimum amount of memory
 */

  nlines = line_pointer - top_line - 1;
  top_line = (char **) realloc(top_line,nlines * sizeof(char *));

/*
 * Store the memory pointers into a structure that will be returned with
 * the widget callback.
 */

  memory_struct->top_line = top_line;
  memory_struct->top_of_page = top_of_page;

  num_args = 0;
  XtSetArg(arglist[num_args], XtNlines, nlines);
  num_args++;
    
  XtSetValues(widget,arglist,num_args);
}


/*	Function Name: PrintManpage
 *	Description: This function prints a man page into the window provided
 *                   by the ScrollByLine widget.
 *	Arguments: w - the ScrollByLine widget.
 *                 structure_pointer - a pointer to the ManpageStruct.
 *                 data - a pointer to the call data.
 *	Returns: none.
 */

/* ARGSUSED */

void
PrintManpage(w,struct_pointer,data)
Widget w;
caddr_t struct_pointer,data;
{
  MemoryStruct * info;		/* Memory containg the file to display. */
  ScrollByLineStruct * scroll_info; /* Information from widget telling me
				       where to display it.*/
  register char *bufp;		/* Generic char pointer */
  char **line;			/* Pointer to the line array. */
  int current_line;		/* the number of the currrent line */
  char buf[BUFSIZ],*c;		/* Misc. characters */
  int col,			/* Location of the current column, not
				   currently used, but could be added.*/
    height,			/* Height of the font */
    width,			/* Width of a tab stop. */
    cur_pos;			/* Current postion of the cursor, in pixels */
  Boolean italicflag = FALSE;	/* Print text in italics?? */
  Boolean first;		/* First line of a manual page??? */
  int y_loc;			/* Vertical postion of text on screen. */
  GC bold_gc,normal_gc,italic_gc; /* A gc for each font. */
  Display * disp;		/* convient disp pointer. */
  Window window;		/* convient window pointer. */
  Pixel fgcolor;		/* The foreground pixel colo(u)r. */
  Arg arglist[10];		/* An argument list. */
  Cardinal num_args;		/* The number of arguments. */

/* Cast some pointers */

  scroll_info = (ScrollByLineStruct *) data;
  info = (MemoryStruct *) struct_pointer;

  current_line = scroll_info->start_line;

/* Use get values to get the foreground colors from the widget. */

  num_args = 0;
  XtSetArg(arglist[num_args], XtNforeground, &fgcolor);
  num_args++;
  XtGetValues(w,arglist,num_args);

/* set the first character to print and the first line. */

  line = scroll_info->start_line + info->top_line;
  c = *line;

  disp = XtDisplay(w);
  window = XtWindow(XtScrollByLineWidget(w));

/*
 * Find out how tall the font is.
 * 
 * NOTE: if you use fonts of drastically different heights you will lose.
 * This must also be the same height used by the scroll by line widget.
 * perhaps the more clever thing to do is have the scrollby line widget 
 * export the font height.
 */

  height = (resources.fonts.normal->max_bounds.ascent + 
		   resources.fonts.normal->max_bounds.descent); 

  bold_gc = XCreateGC(disp,window,0,NULL);
  XSetForeground(disp,bold_gc,(unsigned long) fgcolor);
  XSetFont(disp,bold_gc,resources.fonts.bold->fid);

  normal_gc = XCreateGC(disp,window,0,NULL);
  XSetForeground(disp,normal_gc,(unsigned long) fgcolor);
  XSetFont(disp,normal_gc,resources.fonts.normal->fid);

  italic_gc = XCreateGC(disp,window,0,NULL);
  XSetForeground(disp,italic_gc,(unsigned long) fgcolor);
  XSetFont(disp,italic_gc,resources.fonts.italic->fid);

 /* Width of a tab stop. */
  width =  XTextWidth(resources.fonts.normal,"        ",8);

/*
 * Because XDrawString uses the bottom of the text as a position
 * reference, add the height from the top of the font to the baseline
 * to the ScollByLine position reference.
 */

  y_loc = scroll_info->location + resources.fonts.normal->max_bounds.ascent;

/*
 * Ok, here's the more than mildly heuristic man page formatter.
 * We put chars into buf until either a font change or newline
 * occurs (at which time we flush it to the screen.)
 */

  for(first = FALSE,buf[0] = '\0',bufp = buf,col=INDENT;;) {
    if (current_line % NLINES == 3 && !IS_HELP(w) )
      first = TRUE;

    switch(*c) {

    case '\0':		      /* If we reach the end of the file then return */
      XFreeGC(disp,bold_gc);
      XFreeGC(disp,normal_gc);
      XFreeGC(disp,italic_gc);
      return;

    case '\n':
      *bufp = '\0';
      if (*bufp != buf[0]) {
	if(italicflag)		/* print the line as italic. */
	  XDrawString(disp,window,italic_gc,col,y_loc,buf,strlen(buf));
	else {
	  if(first) {		/* boldify first line [header] */
	    XDrawString(disp,window,bold_gc,col,y_loc,buf,strlen(buf));
	    first = FALSE;
	  }
	  else {		/* if not first line */
	    if((col == INDENT) && Boldify(buf)) 
	      
	      /* boldify "keywords" like NAME */
	      
	      XDrawString(disp,window,bold_gc,col,y_loc,buf,strlen(buf));
	    else		/* print in normal font. */
	      XDrawString(disp,window,normal_gc,col,y_loc,buf,
			  strlen(buf));
	  }
	}
      }
/* 
 * If we have painted the required number of lines then we should also return.
 */
      if (current_line++ == scroll_info->start_line +
	                    scroll_info->num_lines ) {
	XFreeGC(disp,bold_gc);
	XFreeGC(disp,normal_gc);
	XFreeGC(disp,italic_gc);
	return;
      }
      col = INDENT;		
      bufp = buf;
      *bufp = '\0';
      italicflag = 0;
      y_loc += height;
      break;

/*
 * This tab handling code is not very clever it moves the cursor over
 * to the next boundry of eight (8) spaces, as calculated in width just
 * before the printing loop started.
 */

    case '\t':			/* TAB */
      *bufp = '\0';

      if (italicflag)
	XDrawString(disp,window,italic_gc,col,y_loc,buf,strlen(buf));
      else
	XDrawString(disp,window,normal_gc,col,y_loc,buf,strlen(buf));
      bufp = buf; 
      italicflag = 0;
      cur_pos = XTextWidth(resources.fonts.normal,buf,strlen(buf)) + col;
      col = cur_pos - (cur_pos % width) + width;
      break;

    case '\033':		/* ignore esc sequences for now */
      c++;			/* should always be esc-x */
      break;

/* 
 * Overstrike code supplied by: cs.utexas.edu!ut-emx!clyde@rutgers.edu 
 * Since my manual pages do not have overstrike I couldn't test this.
 */

    case BACKSPACE:		/* Backspacing for nroff bolding */
      if (c[-1] == c[1] && c[1] != BACKSPACE) {	/* overstriking one char */
	*--bufp = '\0';		/* Zap 1st instance of char to bolden */
	if (bufp > &buf[0]) {
	  if (italicflag) {
	    XDrawString(disp,window,italic_gc,col,y_loc,buf,strlen(buf));
	    col += XTextWidth(resources.fonts.italic,buf,strlen(buf));
	  }
	  else {
	    XDrawString(disp,window,normal_gc,col,y_loc,buf,strlen(buf));
	    col += XTextWidth(resources.fonts.normal,buf,strlen(buf));
	  }
	  bufp = buf;
	}
	/*
	 * It is painful writing one bold char at a time but
	 * it is the only safe way to do it.  We trust that the library
	 * buffers these one-char draw requests and sends one large
	 * request packet.
	 */
	*bufp = c[1];
	XDrawString(disp,window,bold_gc,col,y_loc,bufp,1);
	col += XTextWidth(resources.fonts.bold,bufp,1);
	*bufp = '\0';
	first = FALSE;

	/*
	 *     Nroff bolding looks like:
	 *	 	     C\bC\bC\bCN...
	 * c points to ----^      ^
	 * it needs to point to --^
	 */
	while (*c == BACKSPACE && c[-1] == c[1])
	  c += 2;
	c--;		/* Back up to previous char */
      }
      else {
	if ((c[-1] == 'o' && c[1] == '+')          /* Nroff bullet */
	    || (c[-1] == '+' && c[1] == 'o')) {	   /* Nroff bullet */
	  /* I would prefer to put a 'bullet' char here */
	  *bufp++ = 'o';
	  c++;
	}
	else {		/* 'real' backspace - back up output ptr */
	  bufp--;
	}
      }
      break;

/* End of contributed overstrike code. */
  
   case '_':			/* look for underlining [italicize] */
      c++;
      if(*c != BACKSPACE) {

	/* oops real underscore, fall through to default. */
	c--;
      }
      else {
	if(!italicflag) {	/* font change? */
	  *bufp = '\0';
	  XDrawString(disp,window,normal_gc,col,y_loc,
		      buf,strlen(buf));
	  col += XTextWidth(resources.fonts.normal,buf,strlen(buf));
	  bufp = buf;
	  *bufp = '\0';
	  italicflag = 1;
	}
	c++;
	*bufp++ = *c;
	break;
      }

    default:
      if(italicflag) {			/* font change? */
	*bufp = '\0';
	XDrawString(disp,window,italic_gc,col,y_loc,buf,strlen(buf));
	col += XTextWidth(resources.fonts.italic,buf,strlen(buf));	
	bufp = buf;
	*bufp = '\0';
	italicflag = 0;
      }
      *bufp++ = *c;
      break;
    }
    c++;
  }
}

/*	Function Name: Boldify
 *	Description: look for keyword.
 *	Arguments: sp - string pointer.
 *	Returns: 1 if keyword else 0.
 */

Boolean
Boldify(sp)
register char *sp;
{
  register char *sp_pointer;
  int length,count;

/* 
 * If there are not lower case letters in the line the assume it is a
 * keyword and boldify it in PrintManpage.
 */

  length = strlen(sp);
  for (sp_pointer = sp, count = 0; count < length; sp_pointer++,count++) 
    if ( !isupper(*sp_pointer) && !isspace(*sp_pointer) )
      return(0);
  return(1);
}


