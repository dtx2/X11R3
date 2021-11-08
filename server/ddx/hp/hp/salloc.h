	/* salloc : allocate bytes on the runtime stack
	 * Always allocates an even number of bytes (rounding up if need
	 *   be)
	 * SP is is always even
	 * The address of the space is always quad word aligned
	 * How to use:
	 *   salloc_addr & salloc_size gotta be globals somewhere
	 *   salloc(n); ptr = (yourtype *)saddr();
	 * Note: A routine that uses SALLOC() can NOT use register 
	 *   variables because of the way register vars are saved on the 
	 *   stack during function calls.
	 */
extern unsigned char *salloc_addr;
extern unsigned int salloc_size;

#ifdef hp9000s300	/* 68000, 68010, 68020 */
#define SAOK 1

#define SALLOC(n) \
  asm("mov.l %sp,_salloc_addr"); \
  salloc_size = (((unsigned int)(n)+3)&(~3)) +((unsigned int)salloc_addr&2);\
  asm("suba.l _salloc_size,%sp"); asm("mov.l %sp,_salloc_addr")

#define SADDR salloc_addr

#define register	/* no register vars allowed */

#else		/* unknown CPU */
#define SAOK 0
#define SALLOC !!!!!!!!DON'T KNOW HOW TO SALLOC ON THIS MACHINE!!!!!!!!!!
#endif
