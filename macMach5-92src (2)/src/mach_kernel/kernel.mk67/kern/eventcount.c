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
 * $Log:	eventcount.c,v $
 * Revision 2.3  91/12/14  14:31:43  jsb
 * 	Replaced gimmeabreak calls with panics.
 * 
 * Revision 2.2  91/12/13  14:54:44  jsb
 * 	Created.
 * 	[91/11/01            af]
 * 
 */
/*
 *	File:	eventcount.c
 *	Author:	Alessandro Forin
 *	Date:	10/91
 *
 *	Eventcounters, for user-level drivers synchronization
 *
 */


#include <cpus.h>

#include <mach/machine.h>
#include <kern/ast.h>
#include <kern/cpu_number.h>
#include <kern/lock.h>
#include <kern/processor.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <machine/machparam.h>	/* For def'n of splsched() */

#include <kern/eventcount.h>

#define	private	/* static */

/*
 * Initialization
 */
void
evc_init(ev)
	evc_t	ev;
{
	bzero(ev, sizeof(*ev));

	ev->sanity = ev;
	ev->waiting_thread = THREAD_NULL;
	simple_lock_init(&ev->lock);
}

/*
 * Finalization
 */
void
evc_destroy(ev)
	evc_t	ev;
{
	evc_signal(ev);
	ev->sanity = 0;
}

/*
 * Just so that we return success, and give
 * up the stack while blocked
 */
private
evc_continue()
{
	thread_syscall_return(KERN_SUCCESS);
	/* NOTREACHED */
}

/*
 * User-trappable
 */
kern_return_t
evc_wait(ev)
	evc_t	ev;
{
	unsigned int	s;
	kern_return_t	ret;

	/* needs thread recover !! */
	if (ev->sanity != ev)
		return KERN_INVALID_ARGUMENT;

	s = splsched();
	simple_lock(&ev->lock);
		if (ev->count) {
			ev->count--;
			ret = KERN_SUCCESS;
		} else {
			if (ev->waiting_thread == THREAD_NULL) {
				ev->waiting_thread = current_thread();
				assert_wait( 0, TRUE);	/* ifnot race */
				simple_unlock(&ev->lock);
#if	EVC_KEEP_STACK
				thread_block((void (*)()) 0);
				splx(s);
				return KERN_SUCCESS;
#else	/* EVC_KEEP_STACK */
				thread_block(evc_continue);
				/* NOTREACHED */
#endif	/* EVC_KEEP_STACK */
			}
			ret = KERN_NO_SPACE; /* XX */
		}
	simple_unlock(&ev->lock);
	splx(s);
	return ret;
}

/*
 * Called exclusively from interrupt context
 */
void
evc_signal(ev)
	evc_t	ev;
{
	register thread_t	thread;
	register int		state;
	unsigned int		s;

	if (ev->sanity != ev)
		return;

	s = splsched();
	simple_lock(&ev->lock);
		ev->count++;
		thread = ev->waiting_thread;
		ev->waiting_thread = 0;
		if (thread == THREAD_NULL)
			goto done;

		/* make thread runnable on this processor */
		/* taken from clear_wait */

		state = thread->state;

/*		reset_timeout_check(&thread->timer);	*/

		switch (state & TH_SCHED_STATE) {
		    case	  TH_WAIT | TH_SUSP | TH_UNINT:
		    case	  TH_WAIT	    | TH_UNINT:
		    case	  TH_WAIT:
			/*
			 *	Sleeping and not suspendable - put
			 *	on run queue.
this is our case
			 */
			thread->state = (state &~ TH_WAIT) | TH_RUN;
			simpler_thread_setrun(thread, TRUE);
			break;

		    case	  TH_WAIT | TH_SUSP:
		    case TH_RUN | TH_WAIT:
		    case TH_RUN | TH_WAIT | TH_SUSP:
		    case TH_RUN | TH_WAIT	    | TH_UNINT:
		    case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
			/*
			 *	Either already running, or suspended.
			 */
			panic("evc_signal.1");
			thread->state = state &~ TH_WAIT;
			break;

		    default:
			/*
			 *	Not waiting.
			 */
			panic("evc_signal.2");
			break;
		}
done:
	simple_unlock(&ev->lock);
	splx(s);
}

/*
 * The scheduler is too messy for my old little brain
 */
simpler_thread_setrun(th, may_preempt)
	thread_t	th;
	boolean_t	may_preempt;
{
#if	NCPU > 1
	/* Don't want to rewrite the whole mess */
	thread_setrun(thread, may_preempt);
#else	/* NCPU > 1 */

	register struct run_queue	*rq;
	register			whichq;

	/*
	 *	XXX should replace queue with a boolean in this case.
	 */
	if (default_pset.idle_count > 0) {
		processor_t	processor;

		processor = (processor_t) queue_first(&default_pset.idle_queue);
		queue_remove(&default_pset.idle_queue, processor,
		processor_t, processor_queue);
		default_pset.idle_count--;
		processor->next_thread = th;
		processor->state = PROCESSOR_DISPATCHING;
		return;
	}
	rq = &(master_processor->runq);
	ast_on(cpu_number(), AST_BLOCK);

	whichq = (th)->sched_pri;
	simple_lock(&(rq)->lock);	/* lock the run queue */
	enqueue_head(&(rq)->runq[whichq], (queue_entry_t) (th));

	if (whichq < (rq)->low || (rq)->count == 0)
		 (rq)->low = whichq;	/* minimize */
	(rq)->count++;
	(th)->runq = (rq);
	simple_unlock(&(rq)->lock);

	/*
	 *	Turn off first_quantum to allow context switch.
	 */
	current_processor()->first_quantum = FALSE;

#endif	/* NCPU > 1 */
}
