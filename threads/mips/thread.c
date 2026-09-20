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
 * 15-Dec-92  Randall Dean (rwd) at Carnegie-Mellon University
 *	Modify to reflect use of CTHREAD_STACK_OFFSET for aligning stacks.
 *
 * 15-Dec-92  Randall Dean (rwd) at Carnegie-Mellon University
 *	Delint.
 *
 * $Log:	thread.c,v $
 * Revision 2.5  91/05/14  17:58:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/14  14:20:59  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:39:03  mrt]
 * 
 * Revision 2.3  89/12/08  19:49:44  rwd
 * 	Changed to take an arbitrary starting routine and an arbitrary
 * 	thread, for use with rwd's new merged coroutine/thread kernel
 * 	implementation.  Removed conditionals.
 * 	[89/12/06            af]
 * 
 * Revision 2.2  89/11/29  14:19:12  af
 * 	Created.
 * 	[89/07/06            af]
 * 
 */
/*
 * mips/thread.c
 *
 * Cproc startup for MIPS Cthreads implementation.
 */

#include <cthreads.h>
#include "cthread_internals.h"

#include <mach/mach.h>

/*
 * C library imports:
 */
extern bzero();

/*
 * Set up the initial state of a MACH thread
 * so that it will invoke routine(child)
 * when it is resumed.
 */
void
cproc_setup(child, thread, routine)
	register cproc_t child;
	int thread;
	int routine;
{
	register int *top = (int *) (child->stack_base + child->stack_size);
	struct mips_thread_state state;
	register struct mips_thread_state *ts = &state;
	kern_return_t r;
	extern int _gp;		/* ld(1) defines this */

	/*
	 * Set up MIPS call frame and registers.
	 * See MIPS Assembly Language Reference Book.
	 */
	bzero((char *) ts, sizeof(struct mips_thread_state));

	/*
	 * Set pc to procedure entry, pass one arg in register,
	 * allocate the standard 4 regsave stack frame.
	 * Give as GP value to the thread the same we have.
	 */
	top = (int *)((int)top - CTHREAD_STACK_OFFSET);
	ts->pc = routine;
	ts->r4 = (int) child;
	ts->r29 = ((int) top) - 4 * sizeof(int);
	ts->r28 = (int) &_gp;

	MACH_CALL(thread_set_state(thread,MIPS_THREAD_STATE,(thread_state_t) &state,MIPS_THREAD_STATE_COUNT),r);
}
