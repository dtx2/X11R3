/* 
 * shadow.h : all the extern defs for the cursor shadow layer
 */
 
	/* cfb shadow */
extern int cfbpolytext8(), cfbpolytext16();
extern unsigned int *cfbgetspans();
extern void
  cfbsetspans(), cfbputimage(), cfbgetimage(),
  cfbpolypoint(), cfbpolylines(), cfbpolysegment(),
  cfbpolyrectangle(), cfbpolyarc(), cfbfillpolygon(), cfbpolyfillrect(),
  cfbpolyfillarc(), cfbimagetext8(), cfbimagetext16(), cfbimageglyphblt(),
  cfbpolyglyphblt(), cfbpushpixels(), cfbunnaturaltilefs(),
  cfbunnaturalstipplefs(), cfbsolidfs() ;
extern RegionPtr  cfbcopyarea(), cfbcopyplane();

	/* mi shadow */
extern int mipolytext8(), mipolytext16();
extern unsigned int *migetspans();
extern void
  mifillspans(), misetspans(), miputimage(), migetimage(),
  mipolypoint(), mipolylines(), mipolysegment(),
  mipolyrectangle(), mipolyarc(), mifillpolygon(), mipolyfillrect(),
  mipolyfillarc(), miimagetext8(), miimagetext16(), miimageglyphblt(),
  mipolyglyphblt(), mipushpixels(), miwideline(), miwidedash(), mizeroline();
extern RegionPtr micopyarea(), micopyplane ();

	/* mfb shadow */
extern int mfbpolytext8(), mfbpolytext16();
extern unsigned int *mfbgetspans();
extern void
  mfbfillspans(), mfbsetspans(), mfbputimage(), mfbgetimage(),
  mfbpolypoint(), mfbpolylines(), mfbpolysegment(),
  mfbpolyrectangle(), mfbpolyarc(), mfbfillpolygon(), mfbpolyfillrect(),
  mfbpolyfillarc(), mfbimagetext8(), mfbimagetext16(), mfbimageglyphblt(),
  mfbpolyglyphblt(), mfbpushpixels();
extern RegionPtr mfbcopyarea (), mfbcopyplane ();
	/* misc */
extern void
  hpzerodash(), hpfbpolyfillrect(),
  hppolyfillsolidrect(), hppaintareasolid(), hpzeroline(),
  topcatzeroline(), topcatzerodash(), tcimageglyphblt(),
  davimageglyphblt(), tcpolyfillsolidrect(),
  tcputimage(), catseyezeroline(), catseyezerodash(), ceimageglyphblt(),
  ceputimage(), cepolyfillsolidrect(), hpgetbyteimage(), hpputbyteimage();
extern RegionPtr  tccopyplane(), hpfbcopyarea(),
#ifdef hp9000s300
  hpccopyarea();
#else hp9000s800
  hpccopyarea(), hpcsrxcopyarea();
#endif

	/* the window shadow */
extern void
  cfbpaintareanone(), cfbpaintareapr(),  cfbpaintarea32(), mipaintwindow(),
  tcpaintareasolid(), cepaintareasolid(), cfbpaintareapr(),
  cfbpaintareaother(), cfbcopywindow();

	/* MRC/MRM shadows */
extern int mrcfbpolytext8(), mrcfbpolytext16();
extern unsigned int *mrcfbgetspans();
extern void
  mrcfbsetspans(), mrcfbputimage(), mrcfbgetimage(), 
  mrcfbpolypoint(), mrcfbpolylines(), mrcfbpolysegment(),
  mrcfbpolyrectangle(), mrcfbpolyarc(), mrcfbfillpolygon(), mrcfbpolyfillrect(),
  mrcfbpolyfillarc(), mrcfbimagetext8(), mrcfbimagetext16(), mrcfbimageglyphblt(),
  mrcfbpolyglyphblt(), mrcfbpushpixels(), mrcfbunnaturaltilefs(),
  mrcfbunnaturalstipplefs(), mrcfbsolidfs(),
  mrcfbpaintareanone(), mrcfbpaintareapr(),  mrcfbpaintarea32(), 
  mrcfbpaintareapr(), mrcfbpaintareaother(), mrcfbcopywindow(),

  mrtopcatzeroline(), mrtopcatzerodash(), mrtcimageglyphblt(),
  mrtcpolyfillsolidrect(), mrtcputimage(), mrtcpaintareasolid(),
  mrhpfbpolyfillrect();
extern RegionPtr mrcfbcopyarea(), mrcfbcopyplane();
