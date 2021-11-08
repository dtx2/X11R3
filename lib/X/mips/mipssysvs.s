/*
 * Copyright 1988 Mips Computer Systems
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Mips not be used in advertising or  
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Mips makes no representations about the  
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */
/* $XConsortium: mipssysvs.s,v 1.3 88/09/06 16:06:49 jim Exp $ */

#include <regdef.h>
#include <asm.h>

/*
 * ffs(word)
 * find first bit set in word (a la VAX instruction)
 * looks at low order bits first, lowest order bit is 1, highest bit is 32
 * no bits returns 0
 */
LEAF(ffs)
	.set	noreorder
	move	v0,zero
	beq	a0,zero,2f		# no bits set, return zero
1:	and	v1,a0,1
	addu	v0,1
	beq	v1,zero,1b
	srl	a0,1			# BDSLOT: shift right to next bit
2:	j	ra
	nop
	.set	reorder
.end ffs

#ifdef notdef
LEAF(ffs)
	move	v1,zero			# initial table offset
	and	v0,a0,0xffff		# check lower halfword
	bne	v0,zero,1f		# bits in lower halfword
	addu	v1,64			# table offset for halfword
	srl	a0,16			# check upper halfword
1:	and	v0,a0,0xff		# check lower byte of halfword
	bne	v0,zero,2f		# bits in lower byte
	addu	v1,32			# table offset for byte
	srl	a0,8			# check upper byte of halfword
2:	and	v0,a0,0xf		# check lower nibble
	bne	v0,zero,3f		# bits in lower nibble
	addu	v1,16			# table offset for nibble
	srl	v0,a0,4			# check upper nibble
	and	v0,0xf
3:	addu	v1,v0			# total table offset
	lbu	v0,ffstbl(v1)		# load bit number from table
	j	ra
.end ffs

	.data
#define NIBBLE(x) \
	.byte	0,       1+(x)*4, 2+(x)*4, 1+(x)*4; \
	.byte	3+(x)*4, 1+(x)*4, 2+(x)*4, 1+(x)*4; \
	.byte	4+(x)*4, 1+(x)*4, 2+(x)*4, 1+(x)*4; \
	.byte	3+(x)*4, 1+(x)*4, 2+(x)*4, 1+(x)*4
ffstbl:
	NIBBLE(0)
	NIBBLE(1)
	NIBBLE(2)
	NIBBLE(3)
	NIBBLE(4)
	NIBBLE(5)
	NIBBLE(6)
	NIBBLE(7)
#endif notdef
