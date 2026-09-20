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
 *	Use CTHREAD_STACK_OFFSET.
 *
 * $Log:	thread.c,v $
 * Revision 2.1.1.1  92/12/10  21:07:55  af
 * 	Created.
 * 	[92/05/31            af]
 * 
 * 
 */
/*
 * alpha/thread.c
 *
 * Cproc startup for ALPHA Cthreads implementation.
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
	thread_t	 thread;
	integer_t	 routine;
{
	register integer_t			*top;
	struct alpha_thread_state		state;
	register struct alpha_thread_state	*ts;
	kern_return_t				r;

	/*
	 * Set up ALPHA call frame and registers.
	 */
	ts = &state;
	bzero((char *) ts, sizeof(struct alpha_thread_state));

	top = (integer_t *) (child->stack_base + child->stack_size);
	top = (integer_t *) ((long)top - CTHREAD_STACK_OFFSET);

	/*
	 * Set pc & pv to procedure entry, pass one arg in register,
	 * allocate room for 6 regsave on the stack frame (sanity).
	 */
	ts->pc = routine;
	ts->r27 = routine;
	ts->r16 = (integer_t) child;
	ts->r30 = (integer_t) (top - 6);	/* see ARG_SAVE in csw.s */


	MACH_CALL(thread_set_state(thread,ALPHA_THREAD_STATE,(thread_state_t) &state,ALPHA_THREAD_STATE_COUNT),r);
}
