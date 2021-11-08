/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: search.c,v 1.3 89/01/06 18:42:28 kit Exp $
 * $oHeader: search.c,v 4.0 88/08/31 22:13:19 kit Exp $
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
 * Created:   November 3, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: search.c,v 4.7 89/01/06 15:59:02 kit Exp $";
#endif

#include "globals.h"

/* Map <CR> and control-M to goto begining of file. */

char search_trans_table[] =
"<Key>0xff0d:   beginning-of-file() \n\
Ctrl<Key>m:    beginning-of-file()";

#define SEARCHARGS 10

FILE * DoManualSearch();
static int BEntrySearch();

/*	Function Name: MakeSearchWidget
 *	Description: This Function Creates the Search Widget.
 *	Arguments: man_globals - the pseudo globas for this manpage.
 *                 w - the widgets parent
 *	Returns: the search widget.
 */

void
MakeSearchWidget(man_globals,parent)
ManpageGlobals * man_globals;
Widget parent;
{
  Widget label,box,command,text;	/* the widgets for various items. */
  Widget popup_shell;		/* the pop up widget's shell. */
  Arg arglist[SEARCHARGS];	/* The arglist and number of args. */
  Cardinal num_args;
  Dimension width, height;	/* The width of the cancel and text button. */
  Dimension h_space;
  XtGeometryResult answer;	/* the answer for the resize request. */
  XtTranslations translations;	/* The translation table to add to the text
				   widget.*/

  num_args = 0;
  XtSetArg(arglist[num_args], XtNallowShellResize, TRUE);
  num_args++;

  popup_shell = XtCreatePopupShell(SEARCHNAME, transientShellWidgetClass, 
				   parent, arglist, num_args);

  box = XtCreateWidget("box",boxWidgetClass, popup_shell,NULL, (Cardinal) 0);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNborderWidth, 0);
  num_args++;

  label = XtCreateWidget("Type string to search for",
			 labelWidgetClass,box,arglist,num_args);
  XtManageChild(label);

  num_args = 0;

  XtSetArg(arglist[num_args], XtNeditType, XttextEdit);
  num_args++;

  strcpy(man_globals->search_string, INIT_SEARCH_STRING);

  XtSetArg(arglist[num_args], XtNstring, man_globals->search_string);
  num_args++;

/* length the user is allowed to make the search string. */

  XtSetArg(arglist[num_args], XtNlength, SEARCH_STRING_LENGTH);
  num_args++;
  XtSetArg(arglist[num_args], XtNcursor, resources.cursors.search_entry);
  num_args++;

  text = XtCreateWidget("textWidgetSearch",asciiStringWidgetClass,
			box, arglist, num_args);
  XtManageChild(text);
  translations = XtParseTranslationTable(search_trans_table);
  XtOverrideTranslations(text, translations);

  man_globals->text_widget = text;
  XtSetKeyboardFocus(box, text);

/* 
 * I am playing a few games here, Make sure that the button with the longest
 * name comes first, otherwise you may lose some text.
 */

  num_args = 0;
  command = XtCreateManagedWidget(MANUALSEARCH,commandWidgetClass,box,
			   arglist, num_args);
  XtAddCallback(command, XtNcallback, SearchCallback, (caddr_t) man_globals);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNwidth, &width); 
  num_args++;
  XtGetValues(command,arglist,num_args);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNwidth, width); 
  num_args++;

  command = XtCreateManagedWidget(APROPOSSEARCH,commandWidgetClass,box,
			   arglist, num_args);
  XtAddCallback(command, XtNcallback, SearchCallback, (caddr_t) man_globals);
  
  XtSetArg(arglist[0], XtNhSpace, &h_space);
  XtGetValues(box, arglist, (Cardinal) 1);

/*
 * The width of the cancel button is 2 command buttons + 2 border_widths
 * + the h_space of the button. 
 */
 
  width = 2 * Width(command);
  width += 2 * BorderWidth(command);
  width += h_space;

  num_args = 0;			
  XtSetArg(arglist[num_args], XtNwidth, width); 
  num_args++;

  command = XtCreateManagedWidget(CANCEL, commandWidgetClass, box,
				  arglist, num_args);
  XtAddCallback(command, XtNcallback, SearchCallback, (caddr_t) man_globals);

/* 
 * Make the text widget 1 line high.
 * 
 * NOTE: This assumes that the text widget uses the same font as the command 
 * widgets.
 */

  height = Height(command);

  answer = XtMakeResizeRequest(text, width, height, &width, &height);
    
  switch(answer) {
  case XtGeometryYes:
  case XtGeometryNo:
    break;
  case XtGeometryAlmost:
    (void) XtMakeResizeRequest(text, width, height, &width, &height);
  }

  XtManageChild(box);
  XtRealizeWidget(popup_shell);
  AddCursor(popup_shell,resources.cursors.search_entry);
}

/*	Function Name: DoSearch
 *	Description: This function performs a search for a man page or apropos
 *                   search upon search string.
 *	Arguments: man_globals - the pseudo globas for this manpage.
 *                 type - the type of search.
 *	Returns: none.
 */

#define LOOKLINES 6

/* 
 * All I do here is to check the string to be sure that we like it, and
 * Then exec the commend via a system() call, the information for a manual
 * search is all in memory, but I was too lazy to write a function to look
 * for it.  If you want one, please write it and send it to me.
 * If nothing is found then I send a warning message to the user, and do
 * nothing.
 */

FILE *
DoSearch(man_globals,type)
ManpageGlobals * man_globals;
int type;
{
  char cmdbuf[BUFSIZ],*mantmp,*manpath;
  char tmp[BUFSIZ],path[BUFSIZ];
  char string_buf[BUFSIZ],*cmp_str;
  char error_buf[BUFSIZ],label[BUFSIZ];
  FILE * file;
  int i,count;
  Boolean flag;

  /* if there was a CR ignore it and every thing after it */

  for (i = 0; i <= strlen(man_globals->search_string);i++) {
    if (man_globals->search_string[i] == '\n')
      man_globals->search_string[i] = '\0';
  }

  /* If the string is empty or starts with a space then do not search */

  if (!strcmp(man_globals->search_string,"") || 
      (man_globals->search_string[0] == ' ')) {
    PrintWarning(man_globals, "You want me to search for what???");
    return(NULL);
  }

  strcpy(tmp,MANTEMP);		/* get a temp file. */
  mantmp = mktemp(tmp);

  /* set the command */

  manpath=getenv("MANPATH");
  if (manpath == NULL || strcmp(manpath,"") == 0)
    strcpy(path,MANDIR);
  else
    strcpy(path,manpath);

  if (type == APROPOS) {
    sprintf(cmdbuf, APROPOSFILTER, path, man_globals->search_string, mantmp);
    sprintf(label,"Results of apropos search on: %s",
	    man_globals->search_string);

    if(system(cmdbuf) != 0) {	/* execute search. */
      sprintf(error_buf,"Something went wrong trying to run %s\n",cmdbuf);
      PrintError(error_buf);
    }

    if((file = fopen(mantmp,"r")) == NULL)
      PrintError("lost temp file? out of temp space?");

    sprintf(string_buf,"%s: nothing appropriate",man_globals->search_string);

    /*
     * Check first LOOKLINES lines for "nothing appropriate".
     */
  
    count = 0;
    cmp_str = "foo";
    flag = FALSE;
    while ( cmp_str != NULL && count <= LOOKLINES ) {
      fgets(cmp_str, 80, file);
      /* strip off the '\n' */
      for (i = 0; i <= strlen(cmp_str);i++) {
	if (cmp_str[i] == '\n')
	  cmp_str[i] = '\0';
      }
      if (!strcmp(cmp_str,string_buf)) {
	flag = TRUE;
	count += LOOKLINES;	/* we've matched finish loop fast. */
      }
      count++;
    }

    /*
     * If the file is less than this number of lines then assume that there is
     * nothing apropriate found. This does not confuse the apropos filter.
     */

    if (flag) {
      fclose(file);
      file = NULL;
      ChangeLabel(man_globals->label,string_buf);
      return(NULL);
    }
  
    strcpy(man_globals->manpage_title,label);
    ChangeLabel(man_globals->label,label);
    fseek(file, 0L, 0);		/* reset file to point at top. */
  }
  else {			/* MANUAL SEACH */
    file = DoManualSearch(man_globals);
    if (file == NULL) {
      sprintf(string_buf,"No manual entry for %s.",man_globals->search_string);
      ChangeLabel(man_globals->label,string_buf);
      return(NULL);
    }
  }
  return(file);
}

/*	Function Name: DoManualSearch
 *	Description: performs a manual search.
 *	Arguments: man_globals - the manual page specific globals.
 *	Returns: the filename of the man page.
 */

#define NO_ENTRY -100

FILE * 
DoManualSearch(man_globals)
ManpageGlobals *man_globals;
{
  char *string;
  int e_num = NO_ENTRY;
  int i, initial_entry;

/* search current section first. */
  
  string = man_globals->search_string;
  i = man_globals->current_directory;
  e_num = BEntrySearch(string, manual[i].entries, manual[i].nentries);

/* search other sections. */

  if (e_num == NO_ENTRY) {
    i = -1;			/* At the exit of the loop i needs to
				   be the one we used. */
    do {
      i++;
      if (i == man_globals->current_directory) i++;
      e_num = BEntrySearch(string, manual[i].entries, manual[i].nentries);
    } while ( (i < sections) && (e_num == NO_ENTRY) );
    if (e_num == NO_ENTRY)
      return(NULL);
/*
 * Manual page found in some other section, unhighlight the current one.
 */
    XtListUnhighlight(
            man_globals->manpagewidgets.box[man_globals->current_directory]);
  }
  else {
    /* 
     * Highlight the element we are searching for if it is in the directory
     * listing currently being shown.
     */
   
    XtListHighlight(man_globals->manpagewidgets.box[i], e_num);
  }
  return(FindFilename(man_globals, manual[i].entries[e_num]));
}

/*	Function Name: BEntrySearch
 *	Description: binary search through entries.
 *	Arguments: string - the string to match.
 *                 first - the first entry in the list.
 *                 number - the number of entries.
 *	Returns: a pointer to the entry found.
 */

static int
BEntrySearch(string, first, number)
char * string;
char ** first;
int number;
{
  int check, cmp, len_cmp, global_number;
  char *head, *tail;
  
  global_number = 0;
  while (TRUE) {

    if (number == 0) {
      return(NO_ENTRY);		/* didn't find it. */
    }

    check = number/2;

    head = rindex(first[ global_number + check ], '/');
    if (head == NULL) 
      PrintError("index failure in BEntrySearch");
    head++;

    tail = rindex(head, '.');
    if (tail == NULL) 
      PrintError("index failure in BEntrySearch");

    cmp = strncmp(string, head, (tail - head));
    len_cmp = strlen(string) - (int) (tail - head);

    if ( cmp == 0 && len_cmp == 0) {
      return(global_number + check);
    }
    else if ( cmp < 0 || ((cmp == 0) && (len_cmp < 0)) ) 
      number = check;
    else /* cmp > 0 || ((cmp == 0) && (len_cmp > 0)) */ {
      global_number += (check + 1);
      number -= ( check + 1 );
    }
  }
}
