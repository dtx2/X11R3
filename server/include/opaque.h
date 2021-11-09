/* $XConsortium: opaque.h,v 1.5 88/09/06 15:48:51 jim Exp $ */

extern char *defaultFontPath;
extern char *defaultTextFont;
extern char *defaultCursorFont;
extern char *rgbPath;
extern long MaxClients;
extern int isItTimeToYield;

extern int CloseFont();
extern unsigned long *Xalloc();
extern unsigned long *Xrealloc();
extern void Xfree();
extern void AddResource();
extern void FreeResource();
extern pointer LookupID();
extern int	TimeOutValue;
