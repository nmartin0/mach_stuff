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

	.globl	curbrk

	.globl	_brk

ENTRY(sbrk) /* char *sbrk(incr) */
	link	a6,#-4		/* space to save curbrk */
	moveq	#~3,d1		/* alignment mask ...111100 */
	movl	curbrk,d0
	addql	#3,d0		/* align curbrk */
	andl	d1,d0
	movl	d0,a6@(-4)	/* save aligned curbrk */
	movl	a6@(8),d0
	addql	#3,d0		/* align incr */
	andl	d1,d0
	addl	a6@(-4),d0	/* add aligned curbrk and incr */
	movl	d0,sp@-
	jbsr	_brk		/* brk(align(curbrk)+align(incr)) */
	moveq	#-1,d1
	cmpl	d0,d1
	jeq	err		/* did brk() fail? */
	movl	a6@(-4),d0	/* return saved curbrk */
err:	unlk	a6
	rts
