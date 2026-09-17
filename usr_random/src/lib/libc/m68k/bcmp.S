/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#include <mac2/SYS.h>

/*
 * This is probably not the best we can do, but it is still 2-10 times
 * faster than the C version in the portable gen directory.
 *
 * Things that might help:
 *	- longword align when possible (only on the 68020)
 *	- use nested DBcc instructions or use one and limit size to 64K
 */
ENTRY(bcmp)
	movl	sp@(4),a0	/* string 1 */
	movl	sp@(8),a1	/* string 2 */
	movl	sp@(12),d0	/* length */
	jeq	bcdone		/* if zero, nothing to do */
	movl	a0,d1
	btst	#0,d1		/* string 1 address odd? */
	jeq	bceven		/* no, skip alignment */
	cmpmb	a0@+,a1@+	/* yes, compare a byte */
	jne	bcnoteq		/* not equal, return non-zero */
	subql	#1,d0		/* adjust count */
	jeq	bcdone		/* count 0, reutrn zero */
bceven:
	movl	a1,d1
	btst	#0,d1		/* string 2 address odd? */
	jne	bcbloop		/* yes, no hope for alignment, compare bytes */
	movl	d0,d1		/* no, both even */
	lsrl	#2,d1		/* convert count to longword count */
	jeq	bcbloop		/* count 0, skip longword loop */
bclloop:
	cmpml	a0@+,a1@+	/* compare a longword */
	jne	bcnoteq		/* not equal, return non-zero */
	subql	#1,d1		/* adjust count */
	jne	bclloop		/* still more, keep comparing */
	andl	#3,d0		/* what remains */
	jeq	bcdone		/* nothing, all done */
bcbloop:
	cmpmb	a0@+,a1@+	/* compare a byte */
	jne	bcnoteq		/* not equal, return non-zero */
	subql	#1,d0		/* adjust count */
	jne	bcbloop		/* still more, keep going */
	rts
bcnoteq:
	moveq	#1,d0
bcdone:
	rts
