/* 
 * crtstub : stubs for displays not linked into the server
 */

#include "crtstub.h"

ScreenInit(catseyeScreenInit,"CatsEye")
ScreenInfo(catseyeScreenInfo,"CatsEye")
ScreenClose(catseyeScreenClose,"CatsEye");

	/* el sleaso stubs */
ceWholeGlyph() {  }
#if hp9000s800
unsigned int *fireye_mode_reg; /*  for toggling fireye cycles */
unsigned int *fireye_bank_switch; /*  for toggling ctl_space and address */
unsigned int fireye_magic_number; /*  for toggling bank_switch register */
#endif
