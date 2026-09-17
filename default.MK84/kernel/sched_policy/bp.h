/*
 * HISTORY
 * $Log:	bp.h,v $
 * Revision 2.2  93/11/17  18:36:24  dbg
 * 	Created.
 * 	[93/06/09            dbg]
 * 
 */

#ifndef	_SCHED_POLICY_BP_H_
#define	_SCHED_POLICY_BP_H_

/*
 *	Bound thread policy.
 *
 *	Each processor has a separate run queue for threads bound to
 *	that processor.  It is scanned as part of the normal run
 *	queue scan, in thread_select, but only if there are threads
 *	on that run queue.
 */

#include <kern/kern_types.h>
#include <kern/run_queues.h>

#define	BOUND_POLICY_INDEX	(NUM_POLICIES - 1)	/* last in runq */

extern run_queue_t	bp_run_queue_alloc(processor_t);

#endif	/* _SCHED_POLICY_BP_H_ */
