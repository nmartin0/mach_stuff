/*
 *	This computes a 16bit one's complement checksum.
 *	It's in MIPS assembly code.
 *	I ripped it off from the unix server source code
 *	so it's actually under the mach copyright.  (See below.)
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1992 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	misc_asm.s,v $
 * Revision 2.3  90/03/14  21:28:07  rwd
 * 	Added INCLUDE_VERSION.
 * 	[90/01/24            af]
 * 
 * Revision 2.2  89/11/29  15:28:45  af
 * 	Created.
 * 	[89/11/06            af]
 * 
 */

#include "asm.h"
/*
 *	Object:
 *		in_checksum			EXPORTED absolute
 *
 *	Arguments:
 *		addr				unsigned short*
 *		len				unsigned
 *		init_cksum			unsigned short
 *
 * Calculates a 16 bit ones-complement checksum.
 * Note that for a big-endian machine, this routine always adds even
 * address bytes to the high order 8 bits of the 16 bit checksum and
 * odd address bytes are added to the low order 8 bits of the 16 bit checksum.
 * For little-endian machines, this routine always adds even address bytes
 * to the low order 8 bits of the 16 bit checksum and the odd address bytes
 * to the high order 8 bits of the 16 bit checksum.
 */
LEAF(in_checksum)
	move	v0,a2		# copy previous checksum
	beq	a1,zero,4f	# count exhausted
	and	v1,a0,1
	beq	v1,zero,2f	# already on a halfword boundry
	lbu	t8,0(a0)
	addu	a0,1
#if	BYTE_MSF
#else	BYTE_MSF
	sll	t8,8
#endif	BYTE_MSF
	addu	v0,t8
	subu	a1,1
	b	2f

1:	lhu	t8,0(a0)
	addu	a0,2
	addu	v0,t8
	subu	a1,2
2:	bge	a1,2,1b
	beq	a1,zero,3f	# no trailing byte
	lbu	t8,0(a0)
#if	BYTE_MSF
	sll	t8,8
#endif	BYTE_MSF
	addu	v0,t8
3:	srl	v1,v0,16	# add in all previous wrap around carries
	and	v0,0xffff
	addu	v0,v1
	srl	v1,v0,16	# wrap-arounds could cause carry, also
	addu	v0,v1
	and	v0,0xffff
4:	j	ra
	END(in_checksum)
