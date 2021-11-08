/* 
 * crtstub : stubs for displays not linked into the server
 */

#include "crtstub.h"

ScreenInit(renScreenInit,"Renaissance")
ScreenInfo(renScreenInfo,"Renaissance")
ScreenClose(renScreenClose,"Renaissance");

ScreenInit(orenScreenInit,"Renaissance")
ScreenInfo(orenScreenInfo,"Renaissance")
ScreenClose(orenScreenClose,"Renaissance");

renWholeGlyph() {}

#ifdef s9000s800
hpsrxSaveAreas() {}
hpsrxRestoreAreas() {}
#endif

