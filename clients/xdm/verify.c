/*
 * xdm - display manager daemon
 *
 * $XConsortium: verify.c,v 1.6 88/11/17 19:13:52 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
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
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"

# include	<pwd.h>
# ifdef NGROUPS
# include	<grp.h>
# endif

struct passwd joeblow = {
	"Nobody", "***************"
};

static int * parseGroups ();

Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p;
	char		*crypt ();
	char		**userEnv (), **systemEnv (), **parseArgs ();
	char		*shell, *home;
	char		**argv;

	p = getpwnam (greet->name);
	if (!p || strlen (greet->name) == 0)
		p = &joeblow;
	Debug ("Verify %s %s\n", greet->name, greet->password);
	if (strcmp (crypt (greet->password, p->pw_passwd), p->pw_passwd)) {
		bzero (greet->password, strlen (greet->password));
		Debug ("verify failed\n");
		return 0;
	}
	bzero (greet->password, strlen (greet->password));
	Debug ("verify succeeded\n");
	verify->uid = p->pw_uid;
#ifdef NGROUPS
	getGroups (greet->name, verify, p->pw_gid);
#else
	verify->gid = p->pw_gid;
#endif
	home = p->pw_dir;
	shell = p->pw_shell;
	argv = 0;
	if (d->session)
		argv = parseArgs (argv, d->session);
	if (greet->string)
		argv = parseArgs (argv, greet->string);
	if (!argv)
		argv = parseArgs (argv, "xsession");
	verify->argv = argv;
	verify->userEnviron = userEnv (d, greet->name, home, shell);
	verify->systemEnviron = systemEnv (d, greet->name, home);
	return 1;
}

/*
 * build an execution environment
 */

# define DISPLAY	0
# define HOME		1
# define PATH		2
# define USER		3
# define SHELL		4

# define NENV		5


char	*envname[NENV] = {
 	"DISPLAY",
	"HOME",
 	"PATH",
 	"USER",
	"SHELL",
};

char *
makeEnv (num, value)
char	*value;
{
	char	*result, *malloc ();

	result = malloc (strlen (envname[num]) + strlen (value) + 2);
	sprintf (result, "%s=%s", envname[num], value);
	return result;
}

char **
userEnv (d, user, home, shell)
struct display	*d;
char	*user, *home, *shell;
{
	static char	*userEnvironment[NENV + 1];

	userEnvironment[DISPLAY] = makeEnv (DISPLAY, d->name);
	userEnvironment[HOME] = makeEnv (HOME, home);
	userEnvironment[USER] = makeEnv (USER, user);
	userEnvironment[PATH] = makeEnv (PATH, d->userPath);
	userEnvironment[SHELL] = makeEnv (SHELL, shell);
	userEnvironment[NENV] = 0;
	return userEnvironment;
}

char **
systemEnv (d, user, home)
struct display	*d;
char	*user, *home;
{
	static char	*systemEnvironment[NENV + 1];
	
	systemEnvironment[DISPLAY] = makeEnv (DISPLAY, d->name);
	systemEnvironment[HOME] = makeEnv (HOME, home);
	systemEnvironment[USER] = makeEnv (USER, user);
	systemEnvironment[PATH] = makeEnv (PATH, d->systemPath);
	systemEnvironment[SHELL] = makeEnv (SHELL, d->systemShell);
	systemEnvironment[NENV] = 0;
	return systemEnvironment;
}

char *
getEnv (e, name)
	char	**e;
	char	*name;
{
	int	l = strlen (name);

	while (*e) {
		if (strlen (*e) > l && !strncmp (*e, name, l) &&
			(*e)[l] == '=')
			return (*e) + l + 1;
		++e;
	}
	return 0;
}

#ifdef NGROUPS
groupMember (name, members)
char	*name;
char	**members;
{
	while (*members) {
		if (!strcmp (name, *members))
			return 1;
		++members;
	}
	return 0;
}

getGroups (name, verify, gid)
char			*name;
struct verify_info	*verify;
int			gid;
{
	int		ngroups;
	struct group	*g;
	int		i;

	ngroups = 0;
	verify->groups[ngroups++] = gid;
	setgrent ();
	while (g = getgrent()) {
		/*
		 * make the list unique
		 */
		for (i = 0; i < ngroups; i++)
			if (verify->groups[i] == g->gr_gid)
				break;
		if (i != ngroups)
			continue;
		if (groupMember (name, g->gr_mem)) {
			if (ngroups >= NGROUPS)
				LogError ("%s belongs to more than %d groups, %s ignored\n",
					name, NGROUPS, g->gr_name);
			else
				verify->groups[ngroups++] = g->gr_gid;
		}
	}
	verify->ngroups = ngroups;
	endgrent ();
}
#endif

# define isblank(c)	((c) == ' ' || c == '\t')

char **
parseArgs (argv, string)
char	**argv;
char	*string;
{
	char	*word;
	char	*save;
	int	i;
	char	*malloc (), *realloc (), *strcpy ();;

	i = 0;
	while (argv && argv[i])
		++i;
	if (!argv)
		argv = (char **) malloc (sizeof (char *));
	word = string;
	for (;;) {
		if (!*string || isblank (*string)) {
			if (word != string) {
				argv = (char **) realloc ((char *) argv, (i + 2) * sizeof (char *));
				argv[i] = strncpy (malloc (string - word + 1), word, string-word);
				argv[i][string-word] = '\0';
				i++;
			}
			if (!*string)
				break;
			word = string + 1;
		}
		++string;
	}
	argv[i] = 0;
	return argv;
}
