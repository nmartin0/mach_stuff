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
 * $Log:	bp.c,v $
 * Revision 2.2  93/11/17  18:36:15  dbg
 * 	Added CLOCK_SCHED entry, so that BP can be used as the current
 * 	policy for bound threads.
 * 	[93/09/01            dbg]
 * 
 * 	Created.
 * 	[93/06/09            dbg]
 * 
 */

/*
 *	"policy" for bound thread, where IO binding is
 *	unnecessary.
 *
 *	Currently, only used for action thread while
 *	switching/shutting down processors.
 */
#include <cpus.h>
#include <mach_io_binding.h>

#if	NCPUS > 1 && !MACH_IO_BINDING

#include <mach/boolean.h>

#include <kern/macro_help.h>
#include <kern/ast.h>
#include <kern/kalloc.h>
#include <kern/run_queues.h>
#include <kern/sched_policy.h>
#include <kern/processor.h>
#include <kern/thread.h>

#include <sched_policy/bp.h>

/*
 *	Uses a single run queue.
 */
struct bp_run_queue {
	struct run_queue rq;		/* common structure */
	queue_head_t	bp_queue;
	processor_t	bp_processor;	/* enable this policy
					   when threads are bound */
};
typedef	struct bp_run_queue *	bp_run_queue_t;

#define	bp_runq(rq)	((struct bp_run_queue *)(rq))
#define	bp_count	rq.rq_count

/*
 *	Choose thread.
 */
thread_t
bp_thread_dequeue(
	run_queue_t	runq)
{
	bp_run_queue_t	rq = bp_runq(runq);
	queue_entry_t	elt;
	processor_t	processor;

	assert(rq->bp_count > 0);
	assert(!queue_empty(&rq->bp_queue));

	dequeue_head_macro(&rq->bp_queue, elt);
	if (--rq->bp_count == 0) {
	    /*
	     *	No more bound threads - skip this policy
	     */
	    processor = rq->bp_processor;
	    processor->runq.last = processor->processor_set->runq.last;
	}

	processor = current_processor();
	processor->quantum = processor->processor_set->set_quantum;
							/* XXX */
	processor->first_quantum = TRUE;

	return (thread_t) elt;
}

/*
 *	Put a thread onto a run queue in priority order.
 *	Return whether it can preempt the current thread.
 */
boolean_t bp_thread_enqueue(
	run_queue_t	runq,
	thread_t	thread,
	boolean_t	may_preempt)
{
	bp_run_queue_t	rq = bp_runq(runq);

	enqueue_tail_macro(&rq->bp_queue, (queue_entry_t) thread);
	rq->bp_count++;
	rq->bp_processor->runq.last = BOUND_POLICY_INDEX;

	return FALSE;	/* never preempts */
}

void bp_thread_remqueue(
	run_queue_t	runq,
	thread_t	thread)
{
	bp_run_queue_t	rq = bp_runq(runq);
	processor_t	processor;
		
	remqueue(&rq->bp_queue, (queue_entry_t) thread);
	if (--rq->bp_count == 0) {
	    /*
	     *	No more bound threads - skip this policy
	     */
	    processor = rq->bp_processor;
	    processor->runq.last = processor->processor_set->runq.last;
	}
}

boolean_t bp_csw_needed(
	run_queue_t	runq,
	thread_t	thread)
{
	return FALSE;
}

extern struct sched_policy	bp_sched_policy;	/* forward */

/*
 *	bp_run_queue_alloc is not called via the
 *	standard path (pset_add_policy).  It is
 *	only used by processor_init.
 */
run_queue_t
bp_run_queue_alloc(
	processor_t	processor)
{
	bp_run_queue_t rq;

	rq = (bp_run_queue_t) kalloc(sizeof(struct bp_run_queue));

	run_queue_init(&rq->rq, &bp_sched_policy);

	queue_init(&rq->bp_queue);
	rq->bp_processor = processor;

	return &rq->rq;
}

/*
 *	Statically allocated policy structure.
 */
struct sched_policy	bp_sched_policy = {
    {
	/* sched_ops */
	bp_thread_dequeue,
	bp_thread_enqueue,
	bp_thread_remqueue,

	bp_csw_needed,
	ast_check,
	0,			/* no update_priority */

	0,			/* no run_queue_alloc */
	0,			/* no run_queue_free */

	0,			/* no runq_set_limit */
	0,			/* no runq_get_limit */
	0,			/* no thread_set_limit */
	0,			/* no thread_set_param */
	0,			/* no thread_get_param */

#if	MACH_KDB
	0,			/* no thread_db_print */
#endif
    },
	-1,		/* will not match any policy */
	"internal bound thread policy"
};

#endif	/* NCPUS > 1 && !MACH_IO_BINDING */
