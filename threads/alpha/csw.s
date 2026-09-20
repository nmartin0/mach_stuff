
/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * 09-Feb-93  Randall Dean (rwd) at Carnegie-Mellon University
 *	Continuation version.
 *
 * $Log:	csw.s,v $
 * Revision 2.1.1.3  93/01/16  12:35:56  af
 * 	Locks are longs now.  Put MBs before and after releasing
 * 	locks.
 * 	[93/01/15            af]
 * 
 * Revision 2.1.1.2  92/12/22  03:00:50  af
 * 	Fixed bug in cproc_prepare, it was not setting up
 * 	the PV register properly.  Safer GP usage too.
 * 	ANSIfied comments.
 * 
 * Revision 2.1.1.1  92/12/10  21:06:44  af
 * 	Created.
 * 	[92/05/31            af]
 * 
 */
/*
 * alpha/csw.s
 *
 * Context switch and cproc startup for ALPHA COROUTINE implementation.
 */
#include <mach/alpha/asm.h>

	.text
	.align	4

#define	CSW_IMASK	\
	IM_S0|IM_S1|IM_S2|IM_S3|IM_S4|IM_S5|IM_S6|IM_RA|IM_GP

#define ARG_SAVE	(6*8)
#define SAVED_S0	(6*8)
#define SAVED_S1	(7*8)
#define SAVED_S2	(8*8)
#define SAVED_S3	(9*8)
#define SAVED_S4	(10*8)
#define SAVED_S5	(11*8)
#define SAVED_S6	(12*8)
#define SAVED_GP	(13*8)
#define SAVED_PC	(14*8)
#define CONT_ARG	(15*8)
#define CONT_PC		(16*8)
#define SAVED_BYTES	(17*8)

/*
 * cthread_filter(con, type, a1, a2, a3, a4)
 */

LEAF(cthread_filter, 6)
	bne	a1, 1f

/* IN */

	subq	sp,SAVED_BYTES,sp	/* allocate space for registers */
					/* Save them registers */
	stq	ra,SAVED_PC(sp)
	stq	gp,SAVED_GP(sp)
	stq	s0,SAVED_S0(sp)
	stq	s1,SAVED_S1(sp)
	stq	s2,SAVED_S2(sp)
	stq	s3,SAVED_S3(sp)
	stq	s4,SAVED_S4(sp)
	stq	s5,SAVED_S5(sp)
	stq	s6,SAVED_S6(sp)

	stq	sp,0(a0)		/* save current sp */
	move	a0, a3			/* get arg */
	subq	sp,ARG_SAVE,sp		/* regsave (sanity) */
	j	a2

1:	subq	a1, 1, a1
	bne	a1, 2f

/* COMPRESS */

	ldq	t0,0(a0)		# get context
	ldq	s1,20(sp)		# new context
	ldq	t1,16(sp)		# lock
	ldq	s0,0(s1)		# indirect
	stq	a2,CONT_PC(t0)
	stq	a3,CONT_ARG(t0)
	move	sp,s0
	.set	noreorder
	mb
	stq	zero,0(t1)		/* release lock */
	mb
	.set	reorder
	ldq	t0,CONT_PC(sp)
	ldq	a0,CONT_ARG(sp)
	subq	sp,ARG_SAVE,sp		/* regsave (sanity) */
	j	t0


2:	subq	a1, 1, a1
	bne	a1, 3f

/* DECOMPRESS */

3:	subq	a1, 1, a1
	bne	a1, 4f

/* OUT */

	ldq	sp,0(a0)		/* restore next sp */
	move	v0, a2			/* get return value */
					/* Reload them registers */
	ldq	ra,SAVED_PC(sp)
	ldq	gp,SAVED_GP(sp)
	ldq	s0,SAVED_S0(sp)
	ldq	s1,SAVED_S1(sp)
	ldq	s2,SAVED_S2(sp)
	ldq	s3,SAVED_S3(sp)
	ldq	s4,SAVED_S4(sp)
	ldq	s5,SAVED_S5(sp)
	ldq	s6,SAVED_S6(sp)
	addq	sp,SAVED_BYTES,sp
	RET

4:
/* PREPARE */
	ldq	t0,0(a0)		# PREPARE
	stq	a2,CONT_PC(t0)
	stq	a3,CONT_ARG(t0)
	j	ra

	END(cproc_switch)

/*
 *	unsigned long
 *	cthread_sp()
 *
 *	Returns the current stack pointer.
 */

LEAF(cthread_sp,0)
	mov	sp, v0
	RET
	END(cthread_sp);
