/* 
 * crtstub : stubs for displays not linked into the server
 */

#include "crtstub.h"

ScreenInit(mrtopcatScreenInit,"mrtopcat")
ScreenInfo(mrtopcatScreenInfo,"mrtopcat")
ScreenClose(mrtopcatScreenClose,"mrtopcat")

	/* el sleaso stubs */
mrtcWholeGlyph() 	 {  }

mrcfbSolidFS()   	 {  }
mrcfbUnnaturalTileFS() 	 {  }
mrcfbUnnaturalStippleFS(){  }
mrcfbSetSpans()		 {  }
mrcfbGetSpans()		 {  }
mrhpfbPolyFillRect()	 {  }
mrcfbPaintAreaNone()     {  }
mrcfbPaintArea32()	 {  }
mrcfbPaintAreaPR()	 {  }
mrcfbPaintAreaOther()  	 {  }
mrcfbCopyWindow()	 {  }

