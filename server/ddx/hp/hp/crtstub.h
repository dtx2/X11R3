/* 
 * crtstub.h : stubs for displays not linked into the server
 */

#define ScreenInit(ScreenInitRoutine,msg)	\
/* Bool */					\
   ScreenInitRoutine(index, pScreen, argc,argv)  \
	/* int index, argc; ScreenPtr pScreen; char **argv; */ \
  { notinstalled(msg); }

#define ScreenInfo(ScreenInfoRoutine,msg) \
/* Bool */					\
   ScreenInfoRoutine(index, argv,argc)		\
	/* int index, argc; char **argv; */	\
  { notinstalled(msg); }

#define ScreenClose(ScreenCloseRoutine,msg)	\
/* Bool */					\
   ScreenCloseRoutine(index, pScreen)		\
	/* int index; ScreenPtr pScreen; */	\
  { notinstalled(msg); }
