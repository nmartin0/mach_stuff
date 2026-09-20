/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * 08-Dec-93  Randall Dean (rwd) at Carnegie-Mellon University
 *	Modify to reflect use of CTHREAD_STACK_OFFSET for aligning stacks.
 * $Log:	thread.c,v $
 * Revision 2.4  92/04/01  10:56:42  rpd
 * 	Moved broken  cthread_sp to csw.s where it can be written
 * 	 correctly in godfearing assembler.
 * 	[92/03/20            danner]
 * 
 * Revision 2.3  92/02/19  14:15:05  elf
 * 	Updated to reflect thread structure.
 * 	[92/02/18  16:41:00  danner]
 * 	Updated setregs to reflect new thread structure.
 * 	[91/09/19  16:58:25  danner]
 * 
 * 	Updated for new thread structure.
 * 	[91/08/25  16:39:33  danner]
 * 
 * Revision 2.2  91/08/24  12:35:41  af
 * 	Created.
 * 	[91/07/19  18:45:28  danner]
 * 
 * Revision 2.1.3.1  91/08/19  13:49:41  danner
 * 	Created by rwd
 * 
 * 
 */
/*
 * luna/thread.c
 *
 * Cproc startup for LUNA88K MTHREAD implementation.
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
	struct luna88k_thread_state state;
	register struct luna88k_thread_state *ts = &state;
	kern_return_t r;

	/*
	 * Set up M88000 call frame and registers.
	 * See M88100 Manual, Object Compatibility Standard/88open,
	 * GNU C compiler documentation, GreenHills C compiler documentation,
	 * etc.
	 */
	bzero((char *) ts, sizeof(struct luna88k_thread_state));
	/*
	 * Set pc(fetch instruction pointer) to location of procedure entry.
	 * Inner cast needed since too many C compilers choke on the type void (*)().
	 */
	ts->sxip = 0;
	ts->snip = 0;
	ts->sfip = routine;
	ts->r[2] = (int) child;	/* argument to function */
	ts->fpcr = 0x1f;	/* matched with locore.s, signal is default */
	ts->r[30] = 0; /* frame pointer, points 0 */
	top = (int *)((int)top - CTHREAD_STACK_OFFSET);
	ts->r[31] = ((int)top - 2*sizeof(int)); /* stack pointer, points arg area */

	MACH_CALL(thread_set_state(thread, \
				   LUNA88K_THREAD_STATE, \
				   (thread_state_t) &state, \
				   LUNA88K_THREAD_STATE_COUNT),
		  r);
}
