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

#define SYS_wait3 SYS_wait

/*
SYSCALL(wait3)
	rts
*/

ENTRY(wait3) /* pid = wait3(status, options, rusage) */
	movl	sp@(8),d0
	movl	sp@(12),d1
	movl	#SYS_wait3,sp@-
	orb	#0x1f,cc
	trap	#0
	bcc 	noerror
	jmp 	cerror
noerror:
	tstl	sp@(4)
	beqs	skip
	movl	sp@(4),a0
	movl	d1,a0@
skip:
	rts

