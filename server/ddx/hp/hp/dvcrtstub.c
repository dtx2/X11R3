/* 
 * crtstub : stubs for displays not linked into the server
 */

#include "crtstub.h"

	/* image server */
ScreenInit(davinciScreenInit,"DaVinci")
ScreenInfo(davinciScreenInfo,"DaVinci")
ScreenClose(davinciScreenClose,"DaVinci")

	/* overlay server */
ScreenInit(oDavinciScreenInit,"DaVinci")
ScreenInit(sbDavinciScreenInit,"DaVinci")
ScreenInfo(oDavinciScreenInfo,"DaVinci")
ScreenClose(oDavinciScreenClose,"DaVinci")

	/* el sleaso stubs */
davWholeGlyph() {  }
