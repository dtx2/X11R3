#ifndef lint
static char Xrcsid[] = "$XConsortium: Error.c,v 1.19 88/09/06 16:27:47 jim Exp $";
/* $oHeader: Error.c,v 1.6 88/08/31 17:46:14 asente Exp $ */
#endif lint

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include <X11/Xlib.h>
#include <stdio.h>
#include "IntrinsicI.h"

/* The error handlers in the application context aren't used since we can't
   come up with a uniform way of using them.  If you can, remove the
   GLOBALERRORS definition. */

#define GLOBALERRORS

#ifdef GLOBALERRORS
static XrmDatabase errorDB = NULL;
static Boolean error_inited = FALSE;
static void _XtDefaultErrorMsg(), _XtDefaultWarningMsg(), 
	_XtDefaultError(), _XtDefaultWarning();
static XtErrorMsgHandler errorMsgHandler = _XtDefaultErrorMsg;
static XtErrorMsgHandler warningMsgHandler = _XtDefaultWarningMsg;
static XtErrorHandler errorHandler = _XtDefaultError;
static XtErrorMsgHandler warningHandler = _XtDefaultWarning;
#endif GLOBALERRORS

XrmDatabase *XtGetErrorDatabase()
{
#ifdef GLOBALERRORS
    return &errorDB;
#else
    return XtAppGetErrorDatabase(_XtDefaultAppContext());
#endif GLOBALERRORS
}

XrmDatabase *XtAppGetErrorDatabase(app)
	XtAppContext app;
{
#ifdef GLOBALERRORS
	return &errorDB;
#else
	return &app->errorDB;
#endif GLOBALERRORS
}

void XtGetErrorDatabaseText(name,type,class,defaultp, buffer, nbytes)
    register char *name, *type,*class;
    char *defaultp;
    char *buffer;
    int nbytes;
{
#ifdef GLOBALERRORS
    XtAppGetErrorDatabaseText(NULL,
	    name,type,class,defaultp, buffer, nbytes, NULL);
#else
    XtAppGetErrorDatabaseText(_XtDefaultAppContext(),
	    name,type,class,defaultp, buffer, nbytes, NULL);
#endif GLOBALERRORS
}

void XtAppGetErrorDatabaseText(app, name,type,class,defaultp,
	buffer, nbytes, db)
    XtAppContext app;
    register char *name, *type,*class;
    char *defaultp;
    char *buffer;
    int nbytes;
    XrmDatabase db;
{
    String type_str;
    XrmValue result;
    char temp[BUFSIZ];

#ifdef GLOBALERRORS
    if (error_inited == FALSE) {
        _XtInitErrorHandling (&errorDB);
        error_inited = TRUE;
    }
#else
    if (app->error_inited == FALSE) {
        _XtInitErrorHandling (&app->errorDB);
        app->error_inited = TRUE;
    }
#endif GLOBALERRORS
    (void) sprintf(temp, "%s.%s", name, type);
    if (db == NULL) {
#ifdef GLOBALERRORS
	(void) XrmGetResource(errorDB, temp, class, &type_str, &result);
#else
	(void) XrmGetResource(app->errorDB, temp, class, &type_str, &result);
#endif GLOBALERRORS
    } else (void) XrmGetResource(db, temp, class, &type_str, &result);
    if (result.addr) {
        (void) strncpy (buffer, result.addr, nbytes);
        if (result.size < nbytes) buffer[result.size] = 0;
    } else (void) strncpy(buffer, defaultp, nbytes);
}

_XtInitErrorHandling (db)
    XrmDatabase *db;
{
    XrmDatabase errordb;

    errordb = XrmGetFileDatabase(ERRORDB);
    XrmMergeDatabases(errordb, db);
}

static void _XtDefaultErrorMsg (name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
    char buffer[1000],message[1000];
    XtGetErrorDatabaseText(name,type,class,defaultp, buffer, 1000);
/*need better solution here, perhaps use lower level printf primitives? */
    if (num_params == NULL) XtError(buffer);
    else {
        (void) sprintf(message, buffer, params[0], params[1], params[2],
		params[3], params[4], params[5], params[6], params[7],
		params[8], params[9]);
	XtError(message);
    }
}

static void _XtDefaultWarningMsg (name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{

    char buffer[1000],message[1000];
    XtGetErrorDatabaseText(name,type,class,defaultp, buffer, 1000);
/*need better solution here*/
    if (num_params == NULL) XtWarning(buffer);
    else {
        (void) sprintf(message, buffer, params[0], params[1], params[2],
		params[3], params[4], params[5], params[6], params[7],
		params[8], params[9]);
	XtWarning(message); 
   }
}

void XtErrorMsg(name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
#ifdef GLOBALERRORS
    (*errorMsgHandler)(name,type,class,defaultp,params,num_params);
#else
    XtAppErrorMsg(_XtDefaultAppContext(),name,type,class,
	    defaultp,params,num_params);
#endif GLOBALERRORS
}

void XtAppErrorMsg(app, name,type,class,defaultp,params,num_params)
    XtAppContext app;
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
#ifdef GLOBALERRORS
    (*errorMsgHandler)(name,type,class,defaultp,params,num_params);
#else
    (*app->errorMsgHandler)(name,type,class,defaultp,params,num_params);
#endif GLOBALERRORS
}

void XtWarningMsg(name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
#ifdef GLOBALERRORS
    (*warningMsgHandler)(name,type,class,defaultp,params,num_params);
#else
    XtAppWarningMsg(_XtDefaultAppContext(),name,type,class,
	    defaultp,params,num_params);
#endif GLOBALERRORS
}

void XtAppWarningMsg(app,name,type,class,defaultp,params,num_params)
    XtAppContext app;
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
#ifdef GLOBALERRORS
    (*warningMsgHandler)(name,type,class,defaultp,params,num_params);
#else
    (*app->warningMsgHandler)(name,type,class,defaultp,params,num_params);
#endif GLOBALERRORS
}

void XtSetErrorMsgHandler(handler)
    XtErrorMsgHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) errorMsgHandler = handler;
    else errorMsgHandler  = _XtDefaultErrorMsg;
#else
    XtAppSetErrorMsgHandler(_XtDefaultAppContext(), handler);
#endif GLOBALERRORS
}

void XtAppSetErrorMsgHandler(app,handler)
    XtAppContext app;
    XtErrorMsgHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) errorMsgHandler = handler;
    else errorMsgHandler  = _XtDefaultErrorMsg;
#else
    if (handler != NULL) app->errorMsgHandler = handler;
    else app->errorMsgHandler  = _XtDefaultErrorMsg;
#endif GLOBALERRORS
}

void XtSetWarningMsgHandler(handler)
    XtErrorMsgHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) warningMsgHandler  = handler;
    else warningMsgHandler = _XtDefaultWarningMsg;
#else
    XtAppSetWarningMsgHandler(_XtDefaultAppContext(),handler);
#endif GLOBALERRORS
}

void XtAppSetWarningMsgHandler(app,handler)
    XtAppContext app;
    XtErrorMsgHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) warningMsgHandler  = handler;
    else warningMsgHandler = _XtDefaultWarningMsg;
#else
    if (handler != NULL) app->warningMsgHandler  = handler;
    else app->warningMsgHandler = _XtDefaultWarningMsg;
#endif GLOBALERRORS
}

static void _XtDefaultError(message)
    String message;
{
    extern void exit();

    (void)fprintf(stderr, "X Toolkit Error: %s\n", message);
    exit(1);
}

static void _XtDefaultWarning(message)
    String message;
{
       (void)fprintf(stderr, "X Toolkit Warning: %s\n", message); 
    return;
}

void XtError(message)
    String message;
{
#ifdef GLOBALERRORS
    (*errorHandler)(message);
#else
    XtAppError(_XtDefaultAppContext(),message);
#endif GLOBALERRORS
}

void XtAppError(app,message)
    XtAppContext app;
    String message;
{
#ifdef GLOBALERRORS
    (*errorHandler)(message);
#else
    (*app->errorHandler)(message);
#endif GLOBALERRORS
}

void XtWarning(message)
    String message;
{
#ifdef GLOBALERRORS
    (*warningHandler)(message);
#else
    XtAppWarning(_XtDefaultAppContext(),message);
#endif GLOBALERRORS
}

void XtAppWarning(app,message)
    XtAppContext app;
    String message;
{
#ifdef GLOBALERRORS
    (*warningHandler)(message);
#else
    (*app->warningHandler)(message);
#endif GLOBALERRORS
}

void XtSetErrorHandler(handler)
    XtErrorHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) errorHandler = handler;
    else errorHandler  = _XtDefaultError;
#else
    XtAppSetErrorHandler(_XtDefaultAppContext(),handler);
#endif GLOBALERRORS
}

void XtAppSetErrorHandler(app,handler)
    XtAppContext app;
    XtErrorHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) errorHandler = handler;
    else errorHandler  = _XtDefaultError;
#else
    if (handler != NULL) app->errorHandler = handler;
    else app->errorHandler  = _XtDefaultError;
#endif GLOBALERRORS
}

void XtSetWarningHandler(handler)
    XtErrorHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) warningHandler = handler;
    else warningHandler = _XtDefaultWarning;
#else
    XtAppSetWarningHandler(_XtDefaultAppContext(),handler);
#endif GLOBALERRORS
}

void XtAppSetWarningHandler(app,handler)
    XtAppContext app;
    XtErrorHandler handler;
{
#ifdef GLOBALERRORS
    if (handler != NULL) warningHandler  = handler;
    else warningHandler = _XtDefaultWarning;
#else
    if (handler != NULL) app->warningHandler  = handler;
    else app->warningHandler = _XtDefaultWarning;
#endif GLOBALERRORS
}

void _XtSetDefaultErrorHandlers(errMsg, warnMsg, err, warn)
	XtErrorMsgHandler *errMsg, *warnMsg;
	XtErrorHandler *err, *warn;
{
#ifndef GLOBALERRORS
	*errMsg = _XtDefaultErrorMsg;
	*warnMsg = _XtDefaultWarningMsg;
	*err = _XtDefaultError;
	*warn = _XtDefaultWarning;
#endif GLOBALERRORS
}
