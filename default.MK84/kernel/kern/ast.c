/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 * $Log:	ast.c,v $
 * Revision 2.15  93/11/17  17:06:14  dbg
 * 	Context_switch check is now a routine in run_queues.c.
 * 	ast_taken and ast_check both use csw_needed.
 * 
 * 	Added ast_kernel_taken to take ASTs for periodic kernel
 * 	operations.  Added ANSI function prototypes.
 * 	[93/05/21            dbg]
 * 
 * Revision 2.14  93/05/15  18:53:51  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.13  93/01/14  17:33:33  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.12  92/08/03  17:36:32  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.11  92/05/21  17:12:45  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.10  91/08/28  11:14:16  jsb
 * 	Renamed AST_CLPORT to AST_NETIPC.
 * 	[91/08/14  21:39:25  jsb]
 * 
 * Revision 2.9  91/06/17  15:46:48  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:48:46  jsb]
 * 
 * Revision 2.8  91/06/06  17:06:43  jsb
 * 	Added AST_CLPORT.
 * 	[91/05/13  17:34:31  jsb]
 * 
 * Revision 2.7  91/05/14  16:39:48  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/05/08  12:47:06  dbg
 * 	Add volatile declarations where needed.
 * 	[91/04/18            dbg]
 * 
 * 	Add missing argument to ast_on in assign/shutdown case.
 * 	[91/03/21            dbg]
 * 
 * Revision 2.5  91/03/16  14:49:23  rpd
 * 	Cleanup.
 * 	[91/02/13            rpd]
 * 	Changed the AST interface.
 * 	[91/01/17            rpd]
 * 
 * Revision 2.4  91/02/05  17:25:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:11:01  mrt]
 * 
 * Revision 2.3  90/06/02  14:53:30  rpd
 * 	Updated with new processor/processor-set technology.
 * 	[90/03/26  22:02:00  rpd]
 * 
 * Revision 2.2  90/02/22  20:02:37  dbg
 * 	Remove lint.
 * 	[90/01/29            dbg]
 * 
 * Revision 2.1  89/08/03  15:42:10  rwd
 * Created.
 * 
 *  2-Feb-89  David Golub (dbg) at Carnegie-Mellon University
 *	Moved swtch to this file.
 *
 * 23-Nov-88  David Black (dlb) at Carnegie-Mellon University
 *	Hack up swtch() again.  Drop priority just low enough to run
 *	something else if it's runnable.  Do missing priority updates.
 *	Make sure to lock thread and double check whether update is needed.
 *	Yet more cruft until I can get around to doing it right.
 *
 *  6-Sep-88  David Golub (dbg) at Carnegie-Mellon University
 *	Removed all non-MACH code.
 *
 * 11-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	csw_check is now the csw_needed macro in sched.h.  Rewrite
 *	ast_check for new ast mechanism.
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Rewrote swtch to check runq counts directly.
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Delete runrun.  Rewrite csw_check so it can become a macro.
 *
 *  4-May-88  David Black (dlb) at Carnegie-Mellon University
 *	Moved cpu not running check to ast_check().
 *	New preempt priority logic.
 *	Increment runrun if ast is for context switch.
 *	Give absolute priority to local run queues.
 *
 * 20-Apr-88  David Black (dlb) at Carnegie-Mellon University
 *	New signal check logic.
 *
 * 18-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Flushed conditionals, reset history.
 */ 

/*
 *
 *	This file contains routines to check whether an ast is needed.
 *
 *	ast_check() - check whether ast is needed for interrupt or context
 *	switch.  Usually called by clock interrupt handler.
 *
 */

#include <cpus.h>
#include <norma_ipc.h>

#include <kern/ast.h>
#include <kern/counters.h>
#include <kern/cpu_number.h>
#include <kern/queue.h>
#include <kern/run_queues.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <kern/processor.h>
#include <kern/mach_timer.h>	/* timer_ast */
#include <device/net_io.h>	/* net_ast */

#include <machine/machspl.h>	/* for splsched */


volatile ast_t need_ast[NCPUS];

void
ast_init(void)
{
#ifndef	MACHINE_AST
	register int i;

	for (i=0; i<NCPUS; i++)
		need_ast[i] = 0;
#endif	/* MACHINE_AST */
}

void
ast_taken(void)
{
	register thread_t self = current_thread();
	register ast_t reasons;

	/*
	 *	Interrupts are still disabled.
	 *	We must clear need_ast and then enable interrupts.
	 */

	reasons = need_ast[cpu_number()];
	need_ast[cpu_number()] = AST_ZILCH;
	(void) spl0();

	/*
	 *	These actions must not block.
	 */

	if (reasons & AST_TIMER)
		timer_ast();

	if (reasons & AST_NETWORK)
		net_ast();

#if	NORMA_IPC
	if (reasons & AST_NETIPC)
		netipc_ast();
#endif	/* NORMA_IPC */

	/*
	 *	Make darn sure that we don`t call thread_halt_self
	 *	or thread_block from the idle thread.
	 */

	if (self != current_processor()->idle_thread) {

		if (self->ast & AST_HALT) {
			thread_halt_self(thread_exception_return);
			/*NOTREACHED*/
		}

		if (self->ast & AST_TERMINATE) {
			thread_terminate_self();
			/*NOTREACHED*/
		}

		/*
		 *	One of the previous actions might well have
		 *	woken a high-priority thread, so we use
		 *	csw_needed in addition to AST_BLOCK.
		 */

		if ((reasons & AST_BLOCK) ||
		    csw_needed(self, current_processor())) {
			counter(c_ast_taken_block++);
			thread_block(thread_exception_return);
		}
	}
}

/*
 * Take ASTs for periodic kernel activities.
 * Called at splsched.
 * Returns at spl0.
 */
void
ast_kernel_taken(void)
{
	register int	mycpu = cpu_number();
	register ast_t	reasons;

	reasons = need_ast[mycpu];
	need_ast[mycpu] &= ~AST_KERNEL;
	(void) spl0();

	/*
	 *	These actions must not block.
	 */

	if (reasons & AST_TIMER)
		timer_ast();

	if (reasons & AST_NETWORK)
		net_ast();

#if	NORMA_IPC
	if (reasons & AST_NETIPC)
		netipc_ast();
#endif	/* NORMA_IPC */
}

void
ast_check(
	thread_t	thread,		/* always the current thread */
	boolean_t	end_quantum)
{
	register int		mycpu = cpu_number();
	register processor_t	myprocessor;
	spl_t			s = splsched();

	/*
	 *	Check processor state for ast conditions.
	 */
	myprocessor = cpu_to_processor(mycpu);
	switch (myprocessor->state) {
	    case PROCESSOR_OFF_LINE:
	    case PROCESSOR_IDLE:
	    case PROCESSOR_DISPATCHING:
		/*
		 *	No ast.
		 */
	    	break;

#if	NCPUS > 1
	    case PROCESSOR_ASSIGN:
	    case PROCESSOR_SHUTDOWN:
	        /*
		 * 	Need ast to force action thread onto processor.
		 *
		 * XXX  Should check if action thread is already there.
		 */
		ast_on(mycpu, AST_BLOCK);
		break;
#endif	/* NCPUS > 1 */

	    case PROCESSOR_RUNNING:

		/*
		 *	Propagate thread ast to processor.  If we already
		 *	need an ast, don't look for more reasons.
		 */
		ast_propagate(thread, mycpu);
		if (ast_needed(mycpu))
			break;

		/*
		 *	Check for context switch.
		 */
		if ((thread->state & TH_SUSP) ||
		    csw_needed(thread, myprocessor))
		{
		    ast_on(mycpu, AST_BLOCK);
		}
		break;

	    default:
	        panic("ast_check: Bad processor state");
	}

	splx(s);
}
