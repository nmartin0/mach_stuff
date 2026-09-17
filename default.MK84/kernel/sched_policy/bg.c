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
 * $Log:	bg.c,v $
 * Revision 2.2  93/11/17  18:35:57  dbg
 * 	Add per-policy scheduling parameters.
 * 	[93/05/11            dbg]
 * 
 * 	Moved common operations to kern/run_queues.c.
 * 	[93/04/10            dbg]
 * 
 * 	Created.
 * 	[93/04/09            dbg]
 * 
 */

/*
 *	Background thread scheduling.
 *
 *	Also provides 'null' parameter/limit routines, for policies
 *	that do not have parameters or limits.
 */

#include <mach/boolean.h>

#include <kern/macro_help.h>
#include <kern/ast.h>
#include <kern/kalloc.h>
#include <kern/run_queues.h>
#include <kern/sched_policy.h>
#include <kern/processor.h>
#include <kern/thread.h>

/*
 *	Uses a single run queue.
 */
struct bg_run_queue {
	struct run_queue rq;		/* common structure */
	queue_head_t	bg_queue;
};
typedef	struct bg_run_queue *	bg_run_queue_t;

#define	bg_runq(rq)	((struct bg_run_queue *)(rq))
#define	bg_count	rq.rq_count

/*
 *	The background policy must have NO per-thread scheduling
 *	parameters.  Using background policy for thread depression
 *	assumes that the thread`s per-policy scheduling parameters
 *	will be untouched.
 */

/*
 *	Choose thread.
 */
thread_t
bg_thread_dequeue(
	run_queue_t	runq)
{
	bg_run_queue_t	rq = bg_runq(runq);
	queue_entry_t	elt;
	processor_t	processor;

	assert(rq->bg_count > 0);
	assert(!queue_empty(&rq->bg_queue));

	dequeue_head_macro(&rq->bg_queue, elt);
	rq->bg_count--;

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
boolean_t bg_thread_enqueue(
	run_queue_t	runq,
	thread_t	thread,
	boolean_t	may_preempt)
{
	register bg_run_queue_t	rq;
	register queue_t	q;

	rq = bg_runq(runq);

	q = &rq->bg_queue;
	enqueue_tail_macro(q, (queue_entry_t) thread);
	rq->bg_count++;

	return FALSE;	/* never preempts */
}

void bg_thread_remqueue(
	run_queue_t	runq,
	thread_t	thread)
{
	bg_run_queue_t	rq = bg_runq(runq);
		
	remqueue(&rq->bg_queue, (queue_entry_t) thread);
	rq->bg_count--;
}

/*
 *	Context switch check for background threads.
 *	Schedule processor round-robin among background
 *	threads.
 */
boolean_t bg_csw_needed(
	run_queue_t	runq,
	thread_t	thread)
{
	if (current_processor()->first_quantum)
	    return FALSE;

	return TRUE;
}

kern_return_t
null_runq_set_limit(
	run_queue_t	runq,
	policy_param_t	limit,
	natural_t	count)
{
	if (count == 0)
	    return KERN_SUCCESS;
	return KERN_FAILURE;	/* no parameters */
}

kern_return_t
null_runq_get_limit(
	run_queue_t	runq,
	policy_param_t	limit,
	natural_t	*count)
{
	*count = 0;
	return KERN_SUCCESS;
}

kern_return_t
null_thread_set_limit(
	thread_t	thread,
	policy_param_t	limit,
	natural_t	count)
{
	if (count == 0)
	    return KERN_SUCCESS;
	return KERN_FAILURE;		/* no parameters */
}

kern_return_t
null_thread_set_param(
	thread_t	thread,
	policy_param_t	param,
	natural_t	count,
	boolean_t	new_policy,
	boolean_t	check_limits)
{
	if (count == 0)
	    return KERN_SUCCESS;
	return KERN_FAILURE;		/* no parameters */
}

kern_return_t
null_thread_get_param(
	thread_t	thread,
	policy_param_t	param,
	natural_t	*count)
{
	*count = 0;
	return KERN_SUCCESS;
}

kern_return_t
null_task_set_param(
	task_t		task,
	policy_param_t	param,
	natural_t	count)
{
	if (count == 0)
	    return KERN_SUCCESS;
	return KERN_FAILURE;		/* no parameters */
}

kern_return_t
null_task_get_param(
	task_t		task,
	policy_param_t	param,
	natural_t	*count)
{
	*count = 0;
	return KERN_SUCCESS;
}

extern struct sched_policy	bg_sched_policy;	/* forward */

run_queue_t
bg_run_queue_alloc(void)
{
	bg_run_queue_t rq;

	rq = (bg_run_queue_t) kalloc(sizeof(struct bg_run_queue));

	run_queue_init(&rq->rq, &bg_sched_policy);

	queue_init(&rq->bg_queue);

	return &rq->rq;
}

void
bg_run_queue_free(
	run_queue_t	runq)
{
	kfree((vm_offset_t) runq, sizeof(struct bg_run_queue));
}

#if	MACH_KDB
#include <ddb/db_output.h>
void bg_thread_db_print(
	thread_t	thread)
{
	db_printf("BG     ");
}
#endif

/*
 *	Statically allocated policy structure.
 */
struct sched_policy	bg_sched_policy = {
    {
	/* sched_ops */
	bg_thread_dequeue,
	bg_thread_enqueue,
	bg_thread_remqueue,

	bg_csw_needed,
	ast_check,
	0,			/* no update_priority */

	bg_run_queue_alloc,
	bg_run_queue_free,

	null_runq_set_limit,
	null_runq_get_limit,
	null_thread_set_limit,
	null_thread_set_param,
	null_thread_get_param,
	null_task_set_param,
	null_task_get_param,

#if	MACH_KDB
	bg_thread_db_print
#endif
    },
	POLICY_BACKGROUND,
	"background"
};

