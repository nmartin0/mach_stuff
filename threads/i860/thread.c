/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	thread.c,v $
 * Revision 2.3  91/12/10  16:33:19  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:31:53  jsb]
 * 
 * Revision 2.2  91/09/04  11:29:21  jsb
 * 	First checkin. Derived from i386 sources by Intel/SSD.
 * 	[91/09/02  08:42:47  jsb]
 * 
 * Revision 2.3  90/06/02  15:13:53  rpd
 * 	Added definition of cthread_sp.
 * 	[90/06/02            rpd]
 * 
 * Revision 2.2  90/05/03  15:55:03  dbg
 * 	Created (from 68020 version).
 * 	[90/02/05            dbg]
 * 
 */
/*
 * i860/thread.c
 */

#ifndef	lint
static char rcs_id[] = "$Header: thread.c,v 2.3 91/12/10 16:33:19 jsb Exp $";
#endif	not lint


#include <cthreads.h>
#include "cthread_internals.h"


#include <mach/mach.h>

/*
 * C library imports:
 */
extern bzero();

int
cthread_sp()
{
	int x;

	return (int) &x;
}

/*
 * Set up the initial state of a MACH thread
 * so that it will invoke cthread_body(child)
 * when it is resumed.
 */
void
cproc_setup(child, thread, routine)
	register cproc_t child;
	int thread;
	int routine;
{
	register int *top = (int *) (child->stack_base + child->stack_size);
	struct i860_thread_state state;
	register struct i860_thread_state *ts = &state;
	kern_return_t r;
	unsigned int count;

	/*
	 * Set up i860 call frame and registers.
	 */
	count = i860_THREAD_STATE_COUNT;
	MACH_CALL(thread_get_state(thread,i860_THREAD_STATE,(thread_state_t) &state,&count),r);

	ts->pc = routine;
	/* 
	 *  self needs to be a parameter to cthread_body() so assign
	 *  the child to r16.
	 */
	ts->r16 = (int) child; 
	ts->r1 = 0;		/* fake return address */
	/*
	 *  Skip over self which is stored at the top of the stack.
	 */
	ts->sp = (int) top - sizeof(int);
	/*
	 *  Align to 16 byte boundary
	 */
	ts->sp = ts->sp & ~0xf;

	MACH_CALL(thread_set_state(thread,i860_THREAD_STATE,(thread_state_t) &state,i860_THREAD_STATE_COUNT),r);
}
