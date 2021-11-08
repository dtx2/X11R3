/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: man.c,v 1.3 89/01/06 18:42:14 kit Exp $
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
 * Created:   August 10, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: man.c,v 4.5 88/12/19 13:47:35 kit Exp $";
#endif

#include "globals.h"

static char error_buf[BUFSIZ];		/* The buffer for error messages. */

static void SortList(), ReadMandescFile(), SortAndRemove(), InitManual();
static void AddToCurrentSection(), AddNewSection(), AddStandardSections();
static int CmpEntryLabel();

#define SECT_ERROR -1
#define streq(a, b)        ( strcmp((a), (b)) == 0 )

typedef struct _SectionList {
  struct _SectionList * next;	/* link to next elem in list. */
  char * label, *directory;	/* The label and directory that this 
				   section represents. */
  Boolean standard;		/* Is this one of the standard sections? */
} SectionList;

/*	Function Name: Man
 *	Description: Builds a list of all manual directories and files.
 *	Arguments: none. 
 *	Returns: the number of manual sections.
 */

int
Man()
{
  SectionList *list = NULL;
  char *ptr, manpath[BUFSIZ], *path, *current_label;
  int sect;

/* 
 * Get the environment variable MANPATH, and if it doesn't exist then back
 * up to MANDIR.
 */

  ptr = getenv("MANPATH");
  if (ptr == NULL || streq(ptr , "") )
    ptr = MANDIR;
  strcpy(manpath, ptr);

/*
 * Get the list of manual directories in the users MANPATH that we should
 * open to look for manual pages.  The ``mandesc'' file is read here.
 */

  for ( path = manpath ; (ptr = index(path , ':')) != NULL ; path = ++ptr) { 
    *ptr = '\0';
    ReadMandescFile(&list, path);
  }
  ReadMandescFile(&list, path);

  SortList(&list);

  sect = 0;
  InitManual( &(manual[sect]), list->label );
  current_label = NULL;
  while ( list != NULL ) {
    SectionList * old_list;

    if ( current_label == NULL || streq(list->label, current_label) )
      AddToCurrentSection( &(manual[sect]), list->directory);
    else {
      if (manual[sect].nentries == 0) {	/* empty section, re-use it. */
	free(manual[sect].blabel);
	manual[sect].blabel = list->label;
      }
      else {
	if ( ++sect >= MAXSECT ) {
	  sprintf(error_buf, "%s %s", "Too many manual sections, recompile",
		  "with a larger value for MAXSECT.");
	  PrintError(error_buf);
	}
	InitManual( &(manual[sect]), list->label );
      }
      AddToCurrentSection( &(manual[sect]), list->directory);
    }
    /* Save label to see if it matches next entry. */
    current_label = list->label; 
    old_list = list;
    list = list->next;
    free(old_list);		/* free what you allocate. */
  }
  if (manual[sect].nentries != 0)
    sect++;			/* don't forget that last section. */
  
  SortAndRemove(manual, sect);

#ifdef notdef			/* dump info. */
  DumpManual(sect);
#endif
  return(sect);		/* return the number of man sections. */
}    

/*	Function Name: SortList
 *	Description: Sorts the list of sections to search.
 *	Arguments: list - a pointer to the list to sort.
 *	Returns: a sorted list.
 *
 * This is the most complicated part of the entire operation.
 * all sections with the same label must by right next to each other,
 * but the sections that are in the standard list have to come first.
 */

static void
SortList(list)
SectionList ** list;
{
  SectionList * local;
  SectionList *head, *last, *inner, *old;
  
  if (*list == NULL)
    PrintError("No manual sections to read, exiting.");

/* 
 * First step 
 * 
 * Look for standard list items, and more them to the top of the list.
 */

  last = NULL;			/* keep Saber happy. */
  for ( local = *list ; local->next != NULL ; local = local->next) {
    if ( local->standard ) {
      if ( local == *list )	/* top element is already standard. */
	break;
      head = local;

      /* Find end of standard block */
      for ( ; (local->next != NULL) && (local->standard) 
	   ; old = local, local = local->next); 

      last->next = old->next; /* Move the block. */
      old->next = *list;
      *list = head;

      break;			/* First step accomplished. */
    }
    last = local;
  }

/*
 *  Second step
 *
 *  Move items with duplicate labels right next to each other.
 */

  local = *list;
  for ( local = *list ; local->next != NULL ; local = local->next) {
    inner = local->next;
    while ( inner != NULL) {
      if ( streq(inner->label, local->label) && (inner != local->next)) {
	last->next = inner->next; /* Move it to directly follow local. */
	inner->next = local->next;
	local->next = inner;
	inner = last;		/* just so that we keep marching down the
				   tree (this keeps us from looping). */
      }
      last = inner;
      inner = inner->next;
    }
  }
}	

/*	Function Name: ReadMandescFile
 *	Description: Reads the mandesc file, and adds more sections as 
 *                   nescessary.
 *	Arguments: path - path name if the current search directory.
 *                 section_list - pointer to the list of sections.
 *	Returns: TRUE in we should use default sections
 */
  
static void
ReadMandescFile( section_list, path )
SectionList ** section_list;
char * path;
{
  char mandesc_file[BUFSIZ];	/* full path to the mandesc file. */
  FILE * descfile;
  char string[BUFSIZ], local_file[BUFSIZ];
  Boolean use_defaults = TRUE;

  sprintf(mandesc_file, "%s/%s", path, MANDESC);
  if ( (descfile = fopen(mandesc_file, "r")) != NULL) {
    while ( fgets(string, BUFSIZ, descfile) != NULL) {
      string[strlen(string)-1] = '\0';        /* Strip off the CR. */

      if ( streq(string, NO_SECTION_DEFAULTS) ) {
	use_defaults = FALSE;
	continue;
      }

      sprintf(local_file, "%s%c", MAN, string[0]);
      AddNewSection(section_list, path, local_file, (string + 1), FALSE );
    }
    fclose(descfile);
  }
  if (use_defaults)
    AddStandardSections(section_list, path);
}

/*	Function Name: AddStandardSections
 *	Description: Adds all the standard sections to the list for this path.
 *	Arguments: list - a pointer to the section list.
 *                 path - the path to these standard sections.
 *	Returns: none.
 */

static void
AddStandardSections(list, path)
SectionList **list;
char * path;
{
  static char * names[] = {
    "User Commands       (1)",
    "System Calls        (2)",
    "Subroutines         (3)",
    "Devices             (4)",
    "File Formats        (5)",
    "Games               (6)",
    "Miscellaneous       (7)",
    "Sys. Administration (8)",
    "Local               (l)",
    "New                 (n)",
    "Old                 (o)",
    };
  register int i;
  char file[BUFSIZ];

  for (i = 1 ; i <= 8 ; i++) {
    sprintf(file, "%s%d", MAN, i);
    AddNewSection(list, path, file, names[i-1], TRUE);
  }
  i--;
  sprintf(file, "%sl", MAN);
  AddNewSection(list, path, file, names[i++], TRUE);
  sprintf(file, "%sn", MAN);
  AddNewSection(list, path, file, names[i++], TRUE);
  sprintf(file, "%so", MAN);
  AddNewSection(list, path, file, names[i], TRUE);
}

/*	Function Name: AddNewSection
 *	Description: Adds the new section onto the current section list.
 *	Arguments: list - pointer to the section list.
 *                 path - the path to the current manual section.
 *                 file - the file to save.
 *                 label - the current section label.
 *                 standard - one of the standard labels?
 *	Returns: none.
 */

static void
AddNewSection(list, path, file, label, standard)
SectionList **list;
char * path, * label, * file;
Boolean standard;
{
  SectionList * local_list, * end;
  char full_path[BUFSIZ];

/* Allocate a new list element */

  if ( (local_list = (SectionList *) malloc(sizeof(SectionList)) ) == NULL)
    PrintError("Could not allocate Memory in AddNewSection.");

  if (*list != NULL) {
    for ( end = *list ; end->next != NULL ; end = end->next );
    end->next = local_list;
  }
  else 
    *list = local_list;

  local_list->next = NULL;
  local_list->label = StrAlloc(label);
  sprintf(full_path, "%s/%s", path, file);
  local_list->directory = StrAlloc(full_path);
  local_list->standard = standard;
}  

/*	Function Name: AddToCurrentSection
 *	Description: This function gets the names of the manual page
 *                   directories, then closes the directory.
 *	Arguments:  local_manual - a pointer to a manual pages structure.
 *                  path - the path to this directory.
 *	Returns: FALSE if directory could not be opened.
 */

static void
AddToCurrentSection(local_manual, path)
Manual * local_manual;
char * path;
{
  DIR * dir;
  register struct direct *dp;
  register int nentries;
  char full_name[BUFSIZ];

  if((dir = opendir(path)) == NULL) {	
#ifdef DEBUG
    sprintf(error_buf,"Can't open directory %s", path);
    PrintWarning(NULL, error_buf);
#endif DEBUG
    return;
  }
  
  nentries = local_manual->nentries;

  while( (dp = readdir(dir)) != NULL ) {
    char * name = dp->d_name;
    if( (name[0] == '.') || (index(name, '.') == NULL) ) 
      continue;
    if ( nentries >= MAXENTRY ) {
      sprintf(error_buf, "%s %s %s", "Too many manual pages in directory",
	      path, "recompile with A larger value for MAXENTRY.");
      PrintError(error_buf);
    }
    sprintf(full_name, "%s/%s", path, name);
    local_manual->entries[nentries++] = StrAlloc(full_name);
  }
  local_manual->nentries = nentries;
}

/*	Function Name: SortAndRemove
 *	Description: This function sorts all the entry names and
 *                   then removes all the duplicate entries.
 *	Arguments: manual - a pointer to the manual structure.
 *                 number - the number of manual sections.
 *	Returns: an improved manual stucure
 */

static void
SortAndRemove(manual, number)
Manual *manual;
int number;
{
  int i;
  char *l1, *l2;

  for ( i = 0; i < number; i++) { /* sort each section */
    register int j = 0;
    Manual * man = &(manual[i]);

#ifdef DEBUG
  printf("sorting section %d - %s\n", i, man->blabel);
#endif DEBUG

    qsort(man->entries, man->nentries, sizeof( char * ), CmpEntryLabel);

#ifdef DEBUG
    printf("removing from section %d.\n", i);
#endif DEBUG

    if ( (l1 = rindex(man->entries[j], '/')) == NULL)
      PrintError("Internal error while removing duplicate manual pages.");
    j++;

    while (j < (man->nentries - 1) ) {
      l2 = l1;
      if ( (l1 = rindex(man->entries[j], '/')) == NULL)
	PrintError("Internal error while removing duplicate manual pages.");
      if ( streq(l1, l2) ) {
	register int k;
	for( k = j; k < (man->nentries); k++)
	  man->entries[k - 1] = man->entries[k];
	(man->nentries)--;
      }
      else
	j++;
    }
  }
}

/*	Function Name: CmpEntryLabel - used in qsort().
 *	Description: compares to elements by using their labels.
 *	Arguments: e1, e2 - two items to compare.
 *	Returns: an integer >, < or = 0.
 */

static int 
CmpEntryLabel(e1, e2) 
char **e1, **e2;
{
  char *l1, *l2;

/*
 * What we really want to compare is the actual names of the manual pages,
 * and not the full path names.
 */

  if ( (l1 = rindex(*e1, '/')) == NULL)
    PrintError("Internal error while sorting manual pages.");
  if ( (l2 = rindex(*e2, '/')) == NULL)
    PrintError("Internal error while sorting manual pages.");
  return( strcmp(l1, l2) );
}

/*	Function Name: StrAlloc
 *	Description: this function allocates memory for a character string
 *      pointed to by sp and returns its new pointer.
 *	Arguments: sp - a pointer to the string that needs memory.
 *	Returns: a pointer to this string, that is now safely allocated.
 */

char *
StrAlloc(sp) char *sp;
{
  char *ret;

  if((ret = (char *) malloc(strlen(sp)+1)) == NULL) {
    sprintf(error_buf,"Out of memory");
    PrintError(error_buf);
  }
  strcpy(ret,sp);
  return(ret);
}

/*	Function Name: InitManual
 *	Description: Initializes this manual section.
 *	Arguments: l_manual - local copy of the manual structure.
 *                 label - the button label for this section.
 *	Returns: none.
 */

static void
InitManual(l_manual, label)
Manual * l_manual;
char * label;
{
  bzero( l_manual, sizeof(Manual) );	        /* clear it. */
  l_manual->blabel = label;	                /* set label. */
  l_manual->entries = (char **) malloc( sizeof(char *) * MAXENTRY);
  if (l_manual->entries == NULL)
	PrintError("Could not allocate memory in InitManual().");
}
  
#if defined(DEBUG)

/*	Function Name: DumpManual
 *	Description: Debugging function that dumps the entire manual page
 *                   structure.
 *	Arguments: number - the number of sections.
 *	Returns: none.
 */

DumpManual(number)
{
  register int i,j;
  
  for ( i = 0; i < number; i++) {
    printf("label: %s\n", manual[i].blabel);
    for (j = 0; j < manual[i].nentries; j++) 
      printf("%s\n", manual[i].entries[j]);
  }
}

#endif DEBUG
