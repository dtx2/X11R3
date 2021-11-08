/*
 * xdm - display manager daemon
 *
 * $XConsortium: resource.c,v 1.10 88/11/17 17:05:01 keith Exp $
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
 * resource.c
 */

# include "dm.h"
# include <X11/Xlib.h>
# include <X11/Xresource.h>

/* XtOffset() hack for ibmrt BandAidCompiler */

char	*servers;
int	request_port;
int	debugLevel;
char	*errorLogFile;
int	daemonMode;
char	*pidFile;

# define DM_STRING	0
# define DM_INT		1
# define DM_BOOL	2

/*
 * the following constants are supposed to be set in the makefile from
 * parameters set util/imake.includes/site.def (or *.macros in that directory
 * if it is server-specific).  DO NOT CHANGE THESE DEFINITIONS!
 */
#ifndef DEF_SERVER_LINE 
#define DEF_SERVER_LINE ":0 secure /usr/bin/X11/X :0"
#endif
#ifndef XRDB_PROGRAM
#define XRDB_PROGRAM "/usr/bin/X11/xrdb"
#endif
#ifndef DEF_SESSION
#define DEF_SESSION "/usr/bin/X11/xterm -ls"
#endif
#ifndef DEF_USER_PATH
#define DEF_USER_PATH ":/bin:/usr/bin:/usr/bin/X11:/usr/ucb"
#endif
#ifndef DEF_SYSTEM_PATH
#define DEF_SYSTEM_PATH "/etc:/bin:/usr/bin:/usr/bin/X11:/usr/ucb"
#endif
#ifndef DEF_SYSTEM_SHELL
#define DEF_SYSTEM_SHELL "/bin/sh"
#endif
#ifndef DEF_FAILSAFE_CLIENT
#define DEF_FAILSAFE_CLIENT "/usr/bin/X11/xterm"
#endif
#ifndef DEF_XDM_CONFIG
#define DEF_XDM_CONFIG "/usr/lib/X11/xdm/xdm-config"
#endif
#ifndef CPP_PROGRAM
#define CPP_PROGRAM "/lib/cpp"
#endif

struct dmResources {
	char	*name, *class;
	int	type;
	char	**dm_value;
	char	*default_value;
} DmResources[] = {
"servers",	"Servers", 	DM_STRING,	&servers,
				DEF_SERVER_LINE,
"requestPort",	"RequestPort",	DM_INT,		(char **) &request_port,
				"0",
"debugLevel",	"DebugLevel",	DM_INT,		(char **) &debugLevel,
				"0",
"errorLogFile",	"ErrorLogFile",	DM_STRING,	&errorLogFile,
				"",
"daemonMode",	"DaemonMode",	DM_BOOL,	(char **) &daemonMode,
				"true",
"pidFile",	"PidFile",	DM_STRING,	&pidFile,
				"",
};

# define NUM_DM_RESOURCES	(sizeof DmResources / sizeof DmResources[0])

# define boffset(f)	((char *) &(((struct display *) 0)->f) - (char *) 0)

struct displayResources {
	char	*name, *class;
	int	type;
	int	offset;
	char	*default_value;
} DisplayResources[] = {
"resources",	"Resources",	DM_STRING,	boffset(resources),
				"",
"xrdb",		"Xrdb",		DM_STRING,	boffset(xrdb),
				XRDB_PROGRAM,
"cpp",		"Cpp",		DM_STRING,	boffset(cpp),
				CPP_PROGRAM,
"startup",	"Startup",	DM_STRING,	boffset(startup),
				"",
"reset",	"Reset",	DM_STRING,	boffset(reset),
				"",
"session",	"Session",	DM_STRING,	boffset(session),
				DEF_SESSION,
"openDelay",	"OpenDelay",	DM_INT,		boffset(openDelay),
				"5",
"openRepeat",	"OpenRepeat",	DM_INT,		boffset(openRepeat),
				"5",
"openTimeout",	"OpenTimeout",	DM_INT,		boffset(openTimeout),
				"30",
"terminateServer","TerminateServer",DM_BOOL,	boffset(terminateServer),
				"false",
"userPath",	"Path",		DM_STRING,	boffset(userPath),
				DEF_USER_PATH,
"systemPath",	"Path",		DM_STRING,	boffset(systemPath),
				DEF_SYSTEM_PATH,
"systemShell",	"Shell",	DM_STRING,	boffset(systemShell),
				DEF_SYSTEM_SHELL,
"failsafeClient","FailsafeClient",	DM_STRING,	boffset(failsafeClient),
				DEF_FAILSAFE_CLIENT,
"grabTimeout",	"GrabTimeout",	DM_INT,		boffset(grabTimeout),
				"3",
};

# define NUM_DISPLAY_RESOURCES	(sizeof DisplayResources/\
				 sizeof DisplayResources[0])

XrmDatabase	DmResourceDB;

GetResource (name, class, valueType, valuep, default_value)
char	*name, *class;
int	valueType;
char	**valuep;
char	*default_value;
{
	char	*type;
	XrmValue	value;
	char	*string, *strncpy (), *malloc ();
	int	len;

	if (DmResourceDB && XrmGetResource (DmResourceDB,
		name, class,
		&type, &value))
	{
		string = value.addr;
		len = value.size;
	} else {
		string = default_value;
		len = strlen (string);
	}
	string = strncpy (malloc (len+1), string, len);
	string[len] = '\0';
	Debug ("resource %s value %s\n", name, string);
	switch (valueType) {
	case DM_STRING:
		*(valuep) = string;
		break;
	case DM_INT:
		*((int *) valuep) = atoi (string);
		free (string);
		break;
	case DM_BOOL:
		XmuCopyISOLatin1Lowered (string, string);
		if (!strcmp (string, "true") ||
		    !strcmp (string, "on") ||
		    !strcmp (string, "yes"))
			*((int *) valuep) = 1;
		else if (!strcmp (string, "false") ||
			 !strcmp (string, "off") ||
			 !strcmp (string, "no"))
			*((int *) valuep) = 0;
		free (string);
		break;
	}
}

XrmOptionDescRec optionTable [] = {
{"-server",	".servers",		XrmoptionSepArg,	(caddr_t) NULL },
{"-udpPort",	".requestPort",		XrmoptionSepArg,	(caddr_t) NULL },
{"-error",	".errorLogFile",	XrmoptionSepArg,	(caddr_t) NULL },
{"-resources",	"*resources",		XrmoptionSepArg,	(caddr_t) NULL },
{"-session",	"*session",		XrmoptionSepArg,	(caddr_t) NULL },
{"-debug",	"*debugLevel",		XrmoptionSepArg,	(caddr_t) NULL },
{"-config",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-xrm",	NULL,			XrmoptionResArg,	(caddr_t) NULL },
{"-daemon",	".daemonMode",		XrmoptionNoArg,		"true"         },
{"-nodaemon",	".daemonMode",		XrmoptionNoArg,		"false"        },
};

InitResources (argc, argv)
int	argc;
char	**argv;
{
	char	**a;
	char	*config = 0;

	XrmInitialize ();
	for (a = argv+1; *a; a++) {
		if (!strcmp (*a, "-config")) {
			if (!a[1])
				LogError ("missing config file argument\n");
			else
				config = a[1];
			break;
		}
	}
	if (!config) {
		config = DEF_XDM_CONFIG;
		if (access (config, 4) == -1)
			config = 0;
	}
	if (config) {
		DmResourceDB = XrmGetFileDatabase ( config );
		if (!DmResourceDB)
			LogError ("Can't open resource file %s\n", config );
	}
	XrmParseCommand (&DmResourceDB, optionTable,
 			 sizeof (optionTable) / sizeof (optionTable[0]),
			 "DisplayManager", &argc, argv);
			 
}

LoadDMResources ()
{
	int	i;
	char	name[1024], class[1024];

	for (i = 0; i < NUM_DM_RESOURCES; i++) {
		sprintf (name, "DisplayManager.%s", DmResources[i].name);
		sprintf (class, "DisplayManager.%s", DmResources[i].class);
		GetResource (name, class, DmResources[i].type,
			      DmResources[i].dm_value,
			      DmResources[i].default_value);
	}
}

LoadDisplayResources (d)
struct display	*d;
{
	int	i;
	char	name[1024], class[1024];
	char	nocolon[512];
	char	*colon, *dot;

	colon = d->name;
	dot = nocolon;
	i = 0;
	while (*colon) {
		if (++i >= sizeof (nocolon))
			break;
		if (*colon == ':')
			*dot++ = '.';
		else
			*dot++ = *colon;
		++colon;
	}
	*dot = '\0';
	for (i = 0; i < NUM_DISPLAY_RESOURCES; i++) {
		sprintf (name, "DisplayManager.%s.%s", 
			nocolon, DisplayResources[i].name);
		sprintf (class, "DisplayManager.%s.%s",
			nocolon, DisplayResources[i].class);
		GetResource (name, class, DisplayResources[i].type,
			      ((char *) d) + DisplayResources[i].offset,
			      DisplayResources[i].default_value);
	}
}
