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
 * $Log:	run_queues.c,v $
 * Revision 2.2  93/11/17  17:20:25  dbg
 * 	Fixed csw_needed to check per-processor run queue for the
 * 	current thread`s scheduling class on MP.  Made thread_bind
 * 	temporarily bind thread to the BP (bound-thread) policy when
 * 	binding to a particular CPU (the same way that depressing a
 * 	thread binds it to the background policy).  NOTE that this won't
 * 	work if a bound thread can be depressed; but on a symmetrical
 * 	MP, we only bind the action thread.  Fixing this (under
 * 	MACH_IO_BINDING) is left as an exercise for the reader.
 * 	[93/09/01            dbg]
 * 
 * 	Use runq pointers in processor if NCPUS > 1 - processor_shutdown
 * 	does not depend on MACH_HOST.
 * 	[93/07/21            dbg]
 * 
 * 	Break up thread lock.
 * 	[93/05/26            dbg]
 * 
 * 	machparam.h -> machspl.h
 * 	[93/05/21            dbg]
 * 
 * 	Changed policy to policy_index in thread.
 * 	[93/05/05            dbg]
 * 
 * 	Moved common code from thread_setrun and rem_runq here.
 * 	[93/04/10            dbg]
 * 
 * 	Processor set now has one run queue structure per scheduling
 * 	policy.  The actual run queues are policy-specific.
 * 	[93/04/06            dbg]
 * 
 * 	Always enable fixed-priority threads.
 * 	[93/03/27            dbg]
 * 
 * 	Moved routines here from kern/sched_prim.c.
 * 	[93/01/12            dbg]
 * 
 */

/*
 *	Run queue routines.
 *
 *	Contain routines to move threads on and off run queues.
 *	The major routine to add a thread to a run queue, thread_setrun,
 *	is specific to the thread's policy.
 *
 *	If there is no thread to run, the idle thread runs instead.
 *
 *	The runq routines also maintain the processor`s quantum,
 *	which is only used by the timesharing scheduling policy.
 *	It is ignored by all of the others, so setting it is harmless.
 */

#include <cpus.h>
#include <mach_host.h>
#include <mach_io_binding.h>

#include <kern/assert.h>
#include <kern/counters.h>
#include <kern/run_queues.h>
#include <kern/processor.h>
#include <kern/thread.h>
#include <kern/sched.h>			/* sched_tick */
#include <kern/sched_policy.h>
#include <kern/sched_prim.h>
#include <kern/stack.h>
#include <kern/thread_swap.h>
#include <machine/machspl.h>

#if	NCPUS > 1 && !MACH_IO_BINDING
#include <sched_policy/bp.h>
#endif

thread_t choose_idle_thread(
	register processor_t myprocessor,
	processor_set_t	pset);		/* forward */

/*
 *	Initialize the run queue header structure.
 */
void run_queue_head_init(
	run_queue_head_t runq)
{
	int		i;

	simple_lock_init(&runq->lock);
	runq->count = 0;
	for (i = 0; i < NUM_POLICIES; i++)
	    runq->runqs[i] = RUN_QUEUE_NULL;
	runq->last = 0;
}

void run_queue_head_dealloc(
	run_queue_head_t	runq)
{
	int	i;
	run_queue_t	rq;

	for (i = runq->last; i >= 0; i--) {
	    rq = runq->runqs[i];
	    RUNQ_FREE(rq);
	}
}

/*
 *	thread_setrun:
 *
 *	Make thread runnable; dispatch directly onto an idle processor
 *	if possible.  Else put on appropriate run queue (processor
 *	if bound, else processor set).  Caller must have lock on thread.
 *	This is always called at splsched.
 */

void thread_setrun(
	register thread_t	thread,
	boolean_t		may_preempt)
{
	register processor_set_t pset = thread->processor_set;
	run_queue_t		rq;
#if	NCPUS > 1
	register processor_t	processor;
#endif

	/*
	 *	Update priority if needed.
	 */
	if (thread->sched_stamp != sched_tick)
	    UPDATE_PRIORITY(thread);

	assert(thread->runq == RUN_QUEUE_HEAD_NULL);

#if	NCPUS > 1
	/*
	 *	Try to dispatch the thread directly onto an idle processor.
	 */
	if ((processor = thread->bound_processor) != PROCESSOR_NULL) {
	    /*
	     *	Bound, can only run on bound processor.  Have to lock
	     *  processor here because it may not be the current one.
	     */
	    if (processor->state == PROCESSOR_IDLE) {
		simple_lock(&processor->lock);
		pset = processor->processor_set;
		simple_lock(&pset->idle_lock);
		if (processor->state == PROCESSOR_IDLE) {
		    queue_remove(&pset->idle_queue, processor,
			processor_t, processor_queue);
		    pset->idle_count--;
		    processor->next_thread = thread;
		    processor->state = PROCESSOR_DISPATCHING;
		    simple_unlock(&pset->idle_lock);
		    simple_unlock(&processor->lock);
		    return;
		}
		simple_unlock(&pset->idle_lock);
		simple_unlock(&processor->lock);
	    }

#if	MACH_IO_BINDING

	    simple_lock(&processor->runq.lock);
	    rq = processor->runq.runqs[thread->policy_index];
	    (void) THREAD_ENQUEUE(rq, thread, FALSE);
	    processor->runq.count++;
	    thread->runq = &processor->runq;
	    simple_unlock(&processor->runq.lock);

#else	/* MACH_IO_BINDING */


	    simple_lock(&processor->lock);
	    pset = processor->processor_set;
	    simple_lock(&pset->runq.lock);
	    rq = processor->runq.runqs[BOUND_POLICY_INDEX];
	    (void) THREAD_ENQUEUE(rq, thread, FALSE);
	    pset->runq.count++;
	    thread->runq = &pset->runq;
	    simple_unlock(&pset->runq.lock);
	    simple_unlock(&processor->lock);

#endif	/* MACH_IO_BINDING */

	    /*
	     *	Cause ast on processor if processor is on line.
	     */
	    if (processor == current_processor()) {
		ast_on(cpu_number(), AST_BLOCK);
	    }
	    else if (processor->state != PROCESSOR_OFF_LINE) {
		cause_ast_check(processor);
	    }
	    return;
	}

	/*
	 *	Not bound, any processor in the processor set is ok.
	 */
#if	HW_FOOTPRINT
	/*
	 *	But first check the last processor it ran on.
	 */
	processor = thread->last_processor;
	if (processor->state == PROCESSOR_IDLE) {
	    simple_lock(&processor->lock);
	    simple_lock(&pset->idle_lock);
	    if (processor->state == PROCESSOR_IDLE
#if	MACH_HOST
		&& processor->processor_set == pset
#endif	/* MACH_HOST */
	       )
	    {
		queue_remove(&pset->idle_queue, processor,
			      processor_t, processor_queue);
		pset->idle_count--;
		processor->next_thread = thread;
		processor->state = PROCESSOR_DISPATCHING;
		simple_unlock(&pset->idle_lock);
		simple_unlock(&processor->lock);
	        return;
	    }
	    simple_unlock(&pset->idle_lock);
	    simple_unlock(&processor->lock);
	}
#endif	/* HW_FOOTPRINT */

	if (pset->idle_count > 0) {
	    simple_lock(&pset->idle_lock);
	    if (pset->idle_count > 0) {
		queue_remove_first(&pset->idle_queue, processor,
				   processor_t, processor_queue);
		pset->idle_count--;
		processor->next_thread = thread;
		processor->state = PROCESSOR_DISPATCHING;
		simple_unlock(&pset->idle_lock);
		return;
	    }
	    simple_unlock(&pset->idle_lock);
	}

#else	/* NCPUS == 1 */

	/*
	 *	Check for idle processor.
	 */
	if (pset->idle_count > 0) {
	    pset->idle_count = 0;
	    processor_array[0].next_thread = thread;
	    processor_array[0].state = PROCESSOR_DISPATCHING;
	    return;
	}

#endif	/* NCPUS */

	/*
	 *	Queue to processor set.
	 */
	simple_lock(&pset->runq.lock);
	rq = pset->runq.runqs[thread->policy_index];
	may_preempt = THREAD_ENQUEUE(rq, thread, may_preempt);
	pset->runq.count++;
	thread->runq = &pset->runq;
	simple_unlock(&pset->runq.lock);

	/*
	 *	Preemption check.  New thread first must be
	 *	on the current processor`s processor set.
	 */
	if (may_preempt
#if	MACH_HOST
	 && pset == current_processor()->processor_set
#endif
	   )
	{
	    /*
	     *	New thread preempts current thread if it is
	     *	in a higher scheduler class (lower index).
	     *
	     *	Per-policy result is used if new thread and
	     *	current thread are in the same scheduler class.
	     *
	     *	No preemption if new thread is in a lower
	     *	scheduler class than current thread.
	     */
	    if (thread->policy_index < current_thread()->policy_index ||
		(thread->policy_index == current_thread()->policy_index &&
		 may_preempt))
	    {
		/*
		 *	Turn off first_quantum to allow context switch.
		 */
		current_processor()->first_quantum = FALSE;
		ast_on(cpu_number(), AST_BLOCK);
	    }
	}
}

/*
 *	Remove a thread from the run queues.  Returns the
 *	run queue that the thread was on, or RUN_QUEUE_HEAD_NULL
 *	if the thread was not found on the run queues.
 *
 *	Called with the thread locked, at splsched.
 */
run_queue_head_t
rem_runq(
	thread_t	thread)
{
	run_queue_head_t runq;

	runq = thread->runq;

	/*
	 *	If runq is RUN_QUEUE_NULL, the thread will stay out of the
	 *	run_queues because the caller locked the thread.  Otherwise
	 *	the thread is on a runq, but could leave.
	 */
	if (runq != RUN_QUEUE_HEAD_NULL) {
	    simple_lock(&runq->lock);
	    if (runq == thread->runq) {
		/*
		 *	Thread is in a runq and we have a lock on
		 *	that runq.
		 */
		run_queue_t	rq;

#if	NCPUS > 1 && !MACH_IO_BINDING
	    {
		processor_t	processor;
		if ((processor = thread->bound_processor) != PROCESSOR_NULL)
		    rq = processor->runq.runqs[BOUND_POLICY_INDEX];
		else
		    rq = runq->runqs[thread->policy_index];
	    }
#else
		rq = runq->runqs[thread->policy_index];
#endif
		THREAD_REMQUEUE(rq, thread);

		runq->count--;

		thread->runq = RUN_QUEUE_HEAD_NULL;
		simple_unlock(&runq->lock);
	    }
	    else {
		/*
		 *	The thread left the runq before we could
		 * 	lock the runq.  It is not on a runq now, and
		 *	can't move again because this routine's
		 *	caller locked the thread.
		 */
		simple_unlock(&runq->lock);
		runq = RUN_QUEUE_HEAD_NULL;
	    }
	}

	return runq;
}

/*
 *	Select the next thread to run on a processor.
 *
 *	Called at splsched.
 */
thread_t
thread_select(
	processor_t	processor)
{
	thread_t	thread;
	processor_set_t	pset = processor->processor_set;
	run_queue_t	runq;

#if	MACH_IO_BINDING
	/*
	 *	Find the highest priority policy in both the
	 *	local and global run queues.  If they are the
	 *	same, call a policy_specific routine to remove
	 *	the highest_priority thread from both of the
	 *	queues.  If they are different, just pick the
	 *	first thread from the highest_priority policy.
	 */
    {
	int	gi, li;
	run_queue_head_t	rqh;

	simple_lock(&pset->runq.lock);
	simple_lock(&processor->runq.lock);

	assert(pset->runq.last == processor->runq.last);

	for (gi = pset->runq.last; gi >= 0; gi--) {
	    if (pset->runq.runqs[gi].rq_count > 0) {
		break;
	    }
	}
	
	for (li = processor->runq.last; li > gi; li--) {
	    if (processor->runq.runqs[li].rq_count > 0) {
		break;
	    }
	}

	if (li >= 0) {
	    /*
	     *	Found something, in one or both run queues.
	     *	Pick the one with the higher priority.
	     */
	    if (gi > li) {
		/*
		 *	Global run queue wins.
		 */
		rqh = &pset->runq;
		runq = rqh->runqs[gi];
	    }
	    else if (li > gi) {
		/*
		 *	Local run queue wins.
		 */
		rqh = &processor->runq;
		runq = rqh->runqs[li];
	    }
	    else {
		/*
		 *	A tie.  Must call policy-specific function
		 *	to decide between the two run queues.
		 */
		run_queue_t	global_runq, local_runq;

		global_runq = pset->runq.runqs[gi];
		local_runq  = processor->runq.runqs[li];

		if (RUNQ_HEAD_PREEMPT(global_runq, local_runq)) {
		    rqh = &pset->runq;
		    runq = global_runq;
		}
		else {
		    rqh = &processor->runq;
		    runq = local_runq;
		}
	    }
	    thread = THREAD_DEQUEUE(runq);
	    thread->runq = RUN_QUEUE_HEAD_NULL;
	    rqh->count--;
	    simple_unlock(&processor->runq.lock);
	    simple_unlock(&pset->runq.lock);
	    return thread;
	}

	simple_unlock(&processor->runq.lock);
	simple_unlock(&pset->runq.lock);
    }
#else	/* MACH_IO_BINDING */

	/*
	 *	Run down the list of scheduling policies for
	 *	the processor set, until we find one with
	 *	runnable threads.  Then call its choose_thread
	 *	routine.
	 */
    {
	int	i;

	simple_lock(&pset->runq.lock);

#if	NCPUS > 1
	/*
	 *	Look at the per-processor run queue pointers,
	 *	to pick up bound threads.
	 */
	for (i = processor->runq.last; i >= 0; i--) {
	    runq = processor->runq.runqs[i];
	    if (runq->rq_count > 0) {
		/*
		 *	Found one.
		 */
		thread = THREAD_DEQUEUE(runq);
		thread->runq = RUN_QUEUE_HEAD_NULL;
		pset->runq.count--;
		simple_unlock(&pset->runq.lock);
		return thread;
	    }
	}
#else	/* NCPUS == 1 */
	for (i = pset->runq.last; i >= 0; i--) {
	    runq = pset->runq.runqs[i];
	    if (runq->rq_count > 0) {
		/*
		 *	Found one.
		 */
		thread = THREAD_DEQUEUE(runq);
		thread->runq = RUN_QUEUE_HEAD_NULL;
		pset->runq.count--;
		simple_unlock(&pset->runq.lock);
		return thread;
	    }
	}
#endif	/* NCPUS > 1 */
	simple_unlock(&pset->runq.lock);
    }

#endif	/* MACH_IO_BINDING */

	/*
	 *	No threads to run.  Check whether the current
	 *	thread is still runnable on this processor.
	 */
	thread = current_thread();
	if (thread->state == TH_RUN &&
	    check_processor_set(thread) &&
	    check_bound_processor(thread))
	{
	    thread_sched_lock(thread);
	    if (thread->sched_stamp != sched_tick)
		UPDATE_PRIORITY(thread);
	    thread_sched_unlock(thread);

	    return thread;
	}

	/*
	 *	Really nothing to do.
	 */
	return choose_idle_thread(processor, pset);
}

/*
 *	Check for preemption.
 *
 *	The current thread is always preempted by any thread in
 *	a higher-priority scheduling class.  The scheduler`s
 *	CSW_NEEDED routine checks whether the thread is preempted
 *	by a thread in the same class.  Threads in lower priority
 *	scheduling classes cannot preempt the current thread.
 */
boolean_t csw_needed(
	thread_t	thread,
	processor_t	processor)
{
	run_queue_t	runq;
	int		i, cur_index;

	if ((thread->state & TH_SUSP) != 0)
	   return TRUE;

	cur_index = thread->policy_index;

#if	MACH_IO_BINDING
	/*
	 *	Must check both local and global run queues.
	 */
    {
	processor_set_t	pset = processor->processor_set;
	int	gi, li;

	for (gi = pset->runq.last; gi > cur_index; gi--) {
	    if (pset->runq.runqs[gi].rq_count > 0)
		return TRUE;
	}

	for (li = pset->runq.last; li > cur_index; li--) {
	    if (processor->runq.runqs[li].rq_count > 0)
		return TRUE;
	}

	runq = pset->runq.runqs[cur_index];
	if (runq->rq_count > 0 && CSW_NEEDED(runq, thread))
	    return TRUE;

	runq = processor->runq.runqs[cur_index];
	if (runq->rq_count > 0 && CSW_NEEDED(runq, thread))
	    return TRUE;

	return FALSE;

#else	/* MACH_IO_BINDING */

#if	NCPUS > 1
	for (i = processor->runq.last; i > cur_index; i--) {
	    runq = processor->runq.runqs[i];
	    if (runq->rq_count > 0)
		return TRUE;
	}

	runq = processor->runq.runqs[cur_index];
	if (runq->rq_count > 0 && CSW_NEEDED(runq, thread))
	    return TRUE;
	return FALSE;

#else	/* NCPUS == 1 */
    {
	processor_set_t	pset = processor->processor_set;
	for (i = pset->runq.last; i > cur_index; i--) {
	    runq = pset->runq.runqs[i];
	    if (runq->rq_count > 0)
		return TRUE;
	}

	runq = pset->runq.runqs[cur_index];
	if (runq->rq_count > 0 && CSW_NEEDED(runq, thread))
	    return TRUE;
	return FALSE;
    }
#endif	/* NCPUS > 1 */

#endif	/* MACH_IO_BINDING */
}


/*
 *	choose_idle_thread:
 *		set processor idle and choose its idle thread.
 *
 *	myprocessor is always the current
 *	processor, and pset must be its processor set.
 *	This routine sets the processor idle and
 *	returns its idle thread.
 */

thread_t choose_idle_thread(
	register processor_t myprocessor,
	processor_set_t	pset)
{
	/*
	 *	Nothing is runnable, so set this processor idle if it
	 *	was running.  If it was in an assignment or shutdown,
	 *	leave it alone.  Return its idle thread.
	 */
	simple_lock(&pset->idle_lock);
	if (myprocessor->state == PROCESSOR_RUNNING) {
	    myprocessor->state = PROCESSOR_IDLE;

#if	NCPUS > 1
	    queue_enter_first(&(pset->idle_queue), myprocessor,
			processor_t, processor_queue);
	    pset->idle_count++;
#else	/* NCPUS > 1 */
	    pset->idle_count = 1;
#endif	/* NCPUS > 1 */
	}
	simple_unlock(&pset->idle_lock);

	return myprocessor->idle_thread;
}

/*
 *	no_dispatch_count counts number of times processors go non-idle
 *	without being dispatched.  This should be very rare.
 */
int	no_dispatch_count = 0;

/*
 *	This is the idle thread, which just looks for other threads
 *	to execute.
 */

no_return idle_thread_continue(void)
{
	register processor_t myprocessor;
	register thread_t volatile *threadp;
	register volatile int *gcount;
#if	MACH_IO_BINDING
	register volatile int *lcount;
#endif
	register thread_t new_thread;
	register int state;
	int mycpu;
	spl_t s;

	mycpu = cpu_number();
	myprocessor = current_processor();
	threadp = (thread_t volatile *) &myprocessor->next_thread;
#if	MACH_IO_BINDING
	lcount = (volatile int *) &myprocessor->runq.count;
#endif

	while (TRUE) {

#ifdef	MARK_CPU_IDLE
	    MARK_CPU_IDLE(mycpu);
#endif	/* MARK_CPU_IDLE */

#if	MACH_HOST
	    gcount = (volatile int *)
				&myprocessor->processor_set->runq.count;
#else	/* MACH_HOST */
	    gcount = (volatile int *) &default_pset.runq.count;
#endif	/* MACH_HOST */

	    /*
	     *	This cpu will be dispatched (by thread_setrun) by setting
	     *	next_thread to the value of the thread to run next.  Also
	     *	check runq counts.
	     */

	    while ((*threadp == THREAD_NULL) &&
		   (*gcount == 0)
#if	MACH_IO_BINDING
		 && (*lcount == 0)
#endif
		  )
	    {
		/*
		 * check for kernel ASTs while we wait
		 */
		AST_KERNEL_CHECK(mycpu);

		/*
		 * machine_idle is a machine dependent function,
		 * to conserve power.
		 */
#if	POWER_SAVE
		machine_idle(mycpu);
#endif /* POWER_SAVE */
	    }

#ifdef	MARK_CPU_ACTIVE
	    MARK_CPU_ACTIVE(mycpu);
#endif	/* MARK_CPU_ACTIVE */

	    s = splsched();

	    /*
	     *	This is not a switch statement to avoid the
	     *	bounds checking code in the common case.
	     */
retry:
	    state = myprocessor->state;
	    if (state == PROCESSOR_DISPATCHING) {

		/*
		 *	Common case -- cpu dispatched.
		 */
		new_thread = *threadp;
		*threadp = THREAD_NULL;
		myprocessor->state = PROCESSOR_RUNNING;

		/*
		 *  Just use set quantum.  No point in
		 *  checking for shorter local runq quantum;
		 *  csw_needed will handle correctly.
		 */
#if	MACH_HOST
		myprocessor->quantum = new_thread->processor_set->set_quantum;
#else	/* MACH_HOST */
		myprocessor->quantum = default_pset.set_quantum;
#endif	/* MACH_HOST */
		myprocessor->first_quantum = TRUE;
		counter(c_idle_thread_handoff++);
		thread_run_noreturn(idle_thread_continue, new_thread);
		/*NOTREACHED*/
	    }
	    else if (state == PROCESSOR_IDLE) {
		register processor_set_t pset;

		pset = myprocessor->processor_set;
		simple_lock(&pset->idle_lock);
		if (myprocessor->state != PROCESSOR_IDLE) {
		    /*
		     *	Something happened, try again.
		     */
		    simple_unlock(&pset->idle_lock);
		    goto retry;
		}

		/*
		 *	Processor was not dispatched (Rare).
		 *	Set it running again.
		 */
		no_dispatch_count++;
		pset->idle_count--;
#if	NCPUS > 1
		queue_remove(&pset->idle_queue, myprocessor,
			processor_t, processor_queue);
#endif
		myprocessor->state = PROCESSOR_RUNNING;
		simple_unlock(&pset->idle_lock);
		counter(c_idle_thread_block++);
		thread_block_noreturn(idle_thread_continue);
		/*NOTREACHED*/
	    }
	    else if ((state == PROCESSOR_ASSIGN) ||
		     (state == PROCESSOR_SHUTDOWN)) {
		/*
		 *	Changing processor sets, or going off-line.
		 *	Release next_thread if there is one.  Actual
		 *	thread to run is on a runq.
		 */
		if ((new_thread = *threadp)!= THREAD_NULL) {
		    *threadp = THREAD_NULL;
		    thread_sched_lock(new_thread);
		    thread_setrun(new_thread, FALSE);
		    thread_sched_unlock(new_thread);
		}

		counter(c_idle_thread_block++);
		thread_block_noreturn(idle_thread_continue);
		/*NOTREACHED*/
	    }
	    else {
		panic("Idle thread: bad processor state %d (Cpu %d)\n",
				state, mycpu);
	    }

	    splx(s);
	}
}

no_return idle_thread(void)
{
	/*
	 *	Can only call stack_privilege on the current thread.
	 */
	stack_privilege(current_thread());

	/*
	 *	thread_block() to set the processor idle when we
	 *	run next time.
	 */
	counter(c_idle_thread_block++);
	thread_block_noreturn(idle_thread_continue);
	/*NOTREACHED*/
}

/*
 *	Create the idle thread for a CPU.
 */
void idle_thread_create(
	processor_t	processor)
{
	thread_t	thread;

	/*
	 *	Create the thread.
	 */
	{
	    thread_t	temp;
	    (void) thread_create(kernel_task, &temp);
	    thread = temp;
	}

	/*
	 *	set it to the idle policy,
	 *	though it won`t really run it.
	 */
	thread->sched_policy = sched_policy_lookup(POLICY_BACKGROUND);
	thread->cur_policy   = thread->sched_policy;
	thread->policy_index = thread->sched_policy->rank;

	thread_start(thread, idle_thread);	/* start at idle thread */
	thread_doswapin(thread);		/* give it a stack */

	processor->idle_thread = thread;	/* make it this processor`s
						   idle thread */

	thread->state |= TH_RUN | TH_IDLE;	/* mark it as running and
						   idle so that */
	(void) thread_resume(thread);		/* this won`t put it on
						   the run queues */
}

#if	NCPUS > 1
/*
 *	thread_bind:
 *
 *	Force a thread to execute on the specified processor.
 *	If the thread is currently executing, it may wait until its
 *	time slice is up before switching onto the specified processor.
 *
 *	A processor of PROCESSOR_NULL causes the thread to be unbound.
 *	xxx - DO NOT export this to users.
 *
 *	Binding a thread to a particular processor temporarily switches
 *	the thread to running under the BP (bound_processor) scheduling
 *	policy.  Unbinding the thread lets it run its normal policy.
 *
 *	NOTE that this won`t work if a bound thread can be depressed,
 *	since depressing a thread also temporarily switches its
 *	scheduling policy.  However, on a symmetrical multiprocessor,
 *	the only thread that is ever bound is the action thread.
 *	Fixing this (under MACH_IO_BINDING), as well as dealing with
 *	the case of a thread that must be bound to a processor that
 *	is not in its processor set, is left as an exercise for the
 *	reader.
 */
void thread_bind(
	register thread_t	thread,
	processor_t		processor)
{
	spl_t		s;

	s = splsched();
	thread_sched_lock(thread);

	thread->bound_processor = processor;
	if (processor != PROCESSOR_NULL) {
	    /*
	     *	Temporarily bind to BP policy.
	     */
	    extern struct sched_policy	bp_sched_policy;

	    thread->cur_policy = &bp_sched_policy;
	    thread->policy_index = bp_sched_policy.rank;
	}
	else {
	    /*
	     *	Resume normal scheduling.
	     */
	    thread->cur_policy = thread->sched_policy;
	    thread->policy_index = thread->sched_policy->rank;
	}

	thread_sched_unlock(thread);
	splx(s);
}
#endif	/* NCPUS > 1 */

