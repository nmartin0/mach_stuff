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
 * $Log:	thread.c,v $
 * Revision 2.6  91/07/31  18:40:02  dbg
 * 	Undefine cthread_sp macro around function definition.
 * 	[91/07/23            dbg]
 * 
 * Revision 2.5  91/05/14  17:59:42  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/14  14:21:34  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:39:39  mrt]
 * 
 * Revision 2.3  90/06/02  15:14:23  rpd
 * 	Added definition of cthread_sp.
 * 	[90/04/24            rpd]
 * 
 * Revision 2.2  89/12/08  19:54:50  rwd
 * 	cproc_setup now takes parameter which is routine to start
 * 	executing.
 * 	[89/11/25            rwd]
 * 	Fix MACH_CALL reference
 * 	[89/11/15            rwd]
 * 	Change calling format and remove conditionals
 * 	[89/10/23            rwd]
 * 
 * 	More cleanup for MACH_KERNEL.
 * 	[89/05/16            dbg]
 * 
 * Revision 2.1  89/08/03  17:11:21  rwd
 * Created.
 * 
 * Revision 1.2  89/05/05  19:00:52  mrt
 * 	Cleanup for Mach 2.5
 * 
 */
/*
 * sun/thread.c
 *
 */

#ifndef	lint
static char rcs_id[] = "$Header: thread.c,v 2.6 91/07/31 18:40:02 dbg Exp $";
#endif	not lint


#include <cthreads.h>
#include "cthread_internals.h"


#include <mach/mach.h>

/*
 * C library imports:
 */
extern bzero();

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
	struct sun_thread_state state;
	register struct sun_thread_state *ts = &state;
	kern_return_t r;

	/*
	 * Set up Sun call frame and registers.
	 */
	bzero((char *) ts, sizeof(struct sun_thread_state));
	/*
	 * Inner cast needed since too many C compilers choke on the type void (*)().
	 */
	ts->pc = routine;
	*--top = (int) child;	/* argument to function */
	*--top = 0;
	ts->sp = (int) top;

	MACH_CALL(thread_set_state(thread,SUN_THREAD_STATE_REGS,(thread_state_t) &state,SUN_THREAD_STATE_REGS_COUNT),r);
}

#ifdef	cthread_sp
#undef	cthread_sp
#endif

int
cthread_sp()
{
	int x;

	return (int) &x;
}

