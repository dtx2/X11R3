/***********************************************************
Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: fontdir.c,v 1.9 88/10/11 15:05:11 rws Exp $ */

#include <stdio.h>
#include <X11/Xos.h>
#include <sys/param.h>

#include "fontdir.h"

#define  XK_LATIN1
#include <X11/keysymdef.h>

static char *MakeCopy(orig)
    char *orig;
{
    char *copy = (char *) Xalloc(strlen(orig) + 1);
    strcpy(copy, orig);
    return copy;
}

/* 
 * It is the caller's responsibility to avoid calling this if file
 * already is in table.
 */
int AddFileEntry(table, name, isAlias)
    FontTable table;
    char *name;
    Boolean isAlias;
{
    register int idx = table->file.used;

    if (table->file.size == idx) {
	    table->file.size *= 2;
	    table->file.ff = (FontFile)Xrealloc(
		table->file.ff, sizeof(FontFileRec)*(table->file.size));
    }
    table->file.used++;
    table->file.ff[idx].name = MakeCopy(name);
	/* Warning: file names cannot be case smashed */
    table->file.ff[idx].private = NULL;
    table->file.ff[idx].alias = isAlias;
    return idx;
}

/*
 * The value returned is either the entry that matched, or in the case
 * that 'found' is false, where in the table the entry should be inserted.
 */

static int FindNormalNameInFontTable(table, name, found)
    FontTable table;
    char *name;
    Boolean *found;
{
    register int left, right, center, result;
    *found = False;

/*
 * binary search with invariant:
 *	legal search space is in [left .. right - 1];
 */

    left = 0;
    right = table->name.used;
    while (left < right) {
	center = (left + right) / 2;
	result = strcmp(name, table->name.fn[center].name);
	if (result == 0) {
	    *found = True;
	    return center;
	}
	if (result < 0)
	    right = center;
	else
	    left = center + 1;
    }
    return left;
}

static int FindWildNameInFontTable(table, name, firstWild, found)
    FontTable table;
    char *name, *firstWild;
    Boolean *found;
{
    char stub[MAXPATHLEN];
    int low, high, i;
    Boolean ignore;

    *found = False;
    if (firstWild == name) {
	low = 0;
	high = table->name.used;
    } else {
	strncpy(stub, name, firstWild - name);
	stub[firstWild - name] = '\0';
	low = FindNormalNameInFontTable(table, stub, &ignore);
	stub[firstWild - name -1]++;
	high = FindNormalNameInFontTable(table, stub, &ignore);
    }
    for (i = low; i < high; i++) {
	if (Match(name, table->name.fn[i].name)) {
	    *found = True;
	    return i;
	}
    }
    return low;		/* should not be used */
}

int FindNameInFontTable(table, name, found)
    FontTable table;
    char *name;
    Boolean *found;
{
    register char *wildChar;

    for (wildChar = name; *wildChar; wildChar++) {
	if ((*wildChar == XK_asterisk) || (*wildChar == XK_question))
	    return FindWildNameInFontTable(table, name, wildChar, found);
    }
    return FindNormalNameInFontTable(table, name, found);
}

/*
 * This will overwrite a previous entry for the same name. This means that if
 * multiple files have the same font name contained within them, then the last
 * will win.
 */

Boolean AddNameEntry(table, name, index)
    FontTable table;
    char *name;
    int index;
{
    int     i;
    Boolean found;
    register char *lower;

    for (lower = name; *lower; lower++)
    {
	if ((*lower == XK_asterisk) || (*lower == XK_question))
	    return False;
    }
    i = FindNormalNameInFontTable (table, name, &found);
    if (!found) {				/* else just overwrite entry */
	if (table->name.size == table->name.used) {
	    table->name.size *= 2;
	    table->name.fn = (FontName)Xrealloc (
		    table->name.fn, sizeof (FontNameRec) * (table->name.size));
	}
	if (i < table->name.used) {
	    register int j;

	    for (j = table->name.used; j > i; j--) {
		table->name.fn[j] = table->name.fn[j-1];     /* struct copy */
	    }
	}
	table->name.used++;
	table->name.fn[i].name = MakeCopy (name);
    }
    table->name.fn[i].u.index = index;
    return True;
}

FontTable
MakeFontTable(directory, size)
    char *directory;
    int size;
{
    FontTable table;

    table = (FontTable)Xalloc(sizeof(FontTableRec));
    table->directory = MakeCopy(directory);
    table->file.ff = (FontFile)Xalloc(sizeof(FontFileRec)*size);
    table->name.fn = (FontName)Xalloc(sizeof(FontNameRec)*size);
    table->file.size = table->name.size = size;
    table->file.used = table->name.used = 0;
    return table;
}

void FreeFontTable(table)
    FontTable table;
{
    int i;

    Xfree(table->directory);
    for (i = 0; i < table->file.used; i++) {
	Xfree (table->file.ff[i].name);
    }
    for (i = 0; i < table->name.used; i++) {
	Xfree(table->name.fn[i].name);
    }
    Xfree(table->file.ff);
    Xfree(table->name.fn);
    Xfree(table);
}

Boolean Match( pat, string)
    register char	*pat;
    register char	*string;
{
    for (; *pat != '\0'; pat++, string++)
    {
        if (*pat == XK_asterisk)
	{
	    pat++;
	    if (*pat == '\0')
		return True;
	    while (!Match(pat, string))
	    {
		if (*string++ == '\0') 
		    return False;
	    }
	    return True;
	}
        else if (*string == '\0')
            return False;
	else if ((*pat != XK_question) && (*pat != *string))
	    return False;
    }
    return (*string == '\0');
}
