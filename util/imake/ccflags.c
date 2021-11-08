#ifdef hpux
#define ccflags "-Wc,-Nd4000,-Ns3000 -DSYSV"
#endif /* hpux */

#ifdef CRAY
#define ccflags "-DSYSV"
#endif /* CRAY */

#if defined(mips) && defined(SYSTYPE_SYSV)
#define ccflags "../../lib/X/mips/mips_sysv_c.c -DSYSV -I../../lib/X//mips -I/usr/include/bsd -lbsd"
#endif /* mips && SYSTYPE_SYSV */

#ifndef ccflags
#define ccflags "-O"
#endif /* ccflags */

main()
{
	write(1, ccflags, sizeof(ccflags) - 1);
	exit(0);
}
