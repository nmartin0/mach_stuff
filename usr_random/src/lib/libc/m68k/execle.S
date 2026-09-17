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

ENTRY(execle) /* execle(file, arg1, arg2, ..., env); */
	link	a6,#-4
	lea	a6@(12),a0
	movl	a0,a6@(-4)
loop:
	movl	a6@(-4),a0
	addql	#4,a6@(-4)
	tstl	a0@
	jne	loop
	movl	a6@(-4),a0
	movl	a0@,sp@-
	pea	a6@(12)
	movl	a6@(8),sp@-
	jbsr	_execve	    /* execve(file, argv, env) */
	lea	sp@(12),sp
	unlk	a6
	rts
