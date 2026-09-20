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
/*
 * HISTORY
 * $Log:	csw.s,v $
 * 14-Dec-92  Randall Dean (rwd) at Carnegie-Mellon University
 *	Modify for continuation based version.
 *
 * Revision 2.6.2.1  92/06/22  11:50:18  rwd
 * 	Change to reflect new interfaces needed by single lock
 * 	version of cproc.c
 * 	[92/01/13            rwd]
 * 
 * Revision 2.6  91/05/14  17:57:51  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:20:35  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:38:39  mrt]
 * 
 * Revision 2.4  90/06/02  15:14:09  rpd
 * 	Added definition of cthread_sp.
 * 	[90/04/24            rpd]
 * 
 * Revision 2.3  89/12/08  19:49:17  rwd
 * 	Changes for new cthreads from af
 * 	[89/12/06            rwd]
 * 
 * Revision 2.2  89/11/29  14:18:59  af
 * 	Created.
 * 	[89/07/06            af]
 * 
 */
/*
 * pmax/csw.s
 *
 * Context switch and cproc startup for MIPS COROUTINE implementation.
 */
#include <mach/mips/asm.h>

	.set	noreorder
	.text
	.align	2

#define ARG_SAVE	(4*4)
#define SAVED_S0	(4*4)
#define SAVED_S1	(5*4)
#define SAVED_S2	(6*4)
#define SAVED_S3	(7*4)
#define SAVED_S4	(8*4)
#define SAVED_S5	(9*4)
#define SAVED_S6	(10*4)
#define SAVED_S7	(11*4)
#define SAVED_FP	(12*4)
#define SAVED_PC	(13*4)
#define CONT_ARG	(14*4)
#define CONT_PC		(15*4)
#define SAVED_BYTES	(16*4)

/*
 * cthread_filter(con, type, a1, a2, a3, a4)
 */

LEAF(cthread_filter)
	bne	zero,a1,1f
	li	t0,1

	lw	a1,16(sp)		# get a3
	subu	sp,sp,SAVED_BYTES	# allocate space for 10 registers
					# Save them registers
	sw	ra,SAVED_PC(sp)
	sw	fp,SAVED_FP(sp)
	sw	s0,SAVED_S0(sp)
	sw	s1,SAVED_S1(sp)
	sw	s2,SAVED_S2(sp)
	sw	s3,SAVED_S3(sp)
	sw	s4,SAVED_S4(sp)
	sw	s5,SAVED_S5(sp)
	sw	s6,SAVED_S6(sp)
	sw	s7,SAVED_S7(sp)
	sw	sp,0(a0)
	move	a0,a3
	j	a2			# call routine
	subu	sp,sp,16		# standard argument space

1:	bne	t0,a1,2f		# COMPRESS
	addi	t0,t0,1			# inc for next compare

	lw	t0,0(a0)		# get context
	lw	s1,20(sp)		# new context
	lw	t1,16(sp)		# lock
	lw	s0,0(s1)		# indirect
	sw	a2,CONT_PC(t0)
	sw	a3,CONT_ARG(t0)
	move	sp,s0
	sw	zero,0(t1)		# clear lock
	lw	t0,CONT_PC(sp)
	lw	a0,CONT_ARG(sp)
	j	t0
	subu	sp,sp,16		# standard argument space

2:	bne	t0,a1,3f
	addi	t0,t0,1			# inc for next compare

3:	bne	t0,a1,4f		# OUT
	addi	t0,t0,1			# inc for next compare

	move	v0,a2
	lw	sp,0(a0)
	nop
	lw	ra,SAVED_PC(sp)
	lw	fp,SAVED_FP(sp)
	lw	s0,SAVED_S0(sp)
	lw	s1,SAVED_S1(sp)
	lw	s2,SAVED_S2(sp)
	lw	s3,SAVED_S3(sp)
	lw	s4,SAVED_S4(sp)
	lw	s5,SAVED_S5(sp)
	lw	s6,SAVED_S6(sp)
	lw	s7,SAVED_S7(sp)
	j	ra
	addu	sp,sp,SAVED_BYTES


4:	lw	t0,0(a0)		# PREPARE
	nop
	sw	a2,CONT_PC(t0)
	j	ra
	sw	a3,CONT_ARG(t0)
	END(cthread_filter)

/*
 *	int
 *	cthread_sp()
 *
 *	Returns the current stack pointer.
 */

LEAF(cthread_sp)
	move	v0, sp
	j	ra
	END(cthread_sp);
