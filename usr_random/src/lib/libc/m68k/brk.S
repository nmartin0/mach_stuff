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

#define	SYS_brk	17

	.globl	_end
	.globl	curbrk
	.globl  minbrk

	.data

minbrk:	.long	_end		/* won't brk() below this */
curbrk:	.long	_end		/* current brk() address */

	.text

	.globl cerror

err:	jmp cerror

ENTRY(_brk) /* char *_brk(addr) */
	bra	ok		/* skip minbrk test */
ENTRY2(brk) /* char *brk(addr) */
	movl	minbrk,d0
	cmpl	sp@(4),d0	/* check address */
	ble	ok
	movl	minbrk,sp@(4)	/* enforce minimum */
ok:	movl	sp@(4),d0	/* get new address */
	addql	#3,d0
	moveq	#~3,d1
	andl	d1,d0		/* long align */
	movl	d0,sp@(4)
	pea	SYS_brk		/* do it... */
	trap	#0
	jcs	err
	movl	sp@(4),curbrk	/* remember new address */
	movl	sp@(4),d0	/* return new address */
	rts
