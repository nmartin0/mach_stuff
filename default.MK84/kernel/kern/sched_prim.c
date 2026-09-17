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
 * $Log:	sched_prim.c,v $
 * Revision 2.25  93/11/17  17:23:36  dbg
 * 	Replaced evc_notify_abort with evc_wait_interrupt, called only
 * 	if thread is halted (like mach_msg_interrupt).
 * 	[93/08/20            dbg]
 * 
 * 	Break up thread lock.
 * 	[93/05/26            dbg]
 * 
 * 	Removed thread->depress_timer.
 * 	[93/04/08            dbg]
 * 
 * 	Changed timeouts to seconds/nanoseconds.
 * 
 * 	Restructured:
 * 	kern/sched_prim.c	thread state machine,
 * 				wait queues,
 * 				context switch primitives
 * 	kern/run_queues.c	run queues
 * 	kern/priority.c		Mach time-sharing priority computation
 * 	sched_policy/mk_ts.c	thread_setrun, context switch check
 * 	[93/01/28            dbg]
 * 
 * 
 * Revision 2.23  93/01/14  17:36:13  danner
 * 	Added assert_wait argument casts.
 * 	[93/01/12            danner]
 * 
 * 	Added ANSI function prototypes.
 * 	[92/12/28            dbg]
 * 
 * 	64bit cleanup. Proper spl typing.
 * 	NOTE NOTE NOTE: events must be 'vm_offset_t' and dont you forget it.
 * 	[92/12/01            af]
 * 
 * 	Added function prototypes.
 * 	[92/11/23            dbg]
 * 
 * 	Fixed locking in do_thread_scan: thread lock must be taken
 * 	before runq lock.  Removed obsolete swap states from
 * 	documentation of state machine.
 * 	[92/10/29            dbg]
 * 
 * Revision 2.22  92/08/05  18:05:25  jfriedl
 * 	Added call to  machine_idle in idle loop if power_save option is
 * 	selected.
 * 	[92/08/05            mrt]
 * 
 * Revision 2.21  92/07/20  13:32:37  cmaeda
 * 	Fast tas support: Check if current thread is in ras during 
 *	thread_block.
 * 	[92/05/11  14:34:42  cmaeda]
 * 
 * Revision 2.20  92/05/21  17:15:31  jfriedl
 * 	Added void to fcns that yet needed it.
 * 	Added init to 'restart_needed' in do_thread_scan().
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.19  92/02/19  16:06:35  elf
 * 	Added argument to compute_priority.  We dont always want to do
 * 	a reschedule right away.
 * 	[92/01/19            rwd]
 * 
 * Revision 2.18  91/09/04  11:28:26  jsb
 * 	Add a temporary hack to thread_dispatch for i860 support:
 * 	don't panic if thread->state is TH_WAIT.
 * 	[91/09/04  09:24:43  jsb]
 * 
 * Revision 2.17  91/08/24  11:59:46  af
 * 	Final form of do_priority_computation was missing a pair of parentheses.
 * 	[91/07/19            danner]
 * 
 * Revision 2.16  91/07/31  17:47:27  dbg
 * 	When re-invoking the running thread (thread_block, thread_run):
 * 	. Mark the thread interruptible.
 * 	. If there is a continuation, call it instead of returning.
 * 	[91/07/26            dbg]
 * 
 * 	Fix timeout race.
 * 	[91/05/23            dbg]
 * 
 * 	Revised scheduling state machine.
 * 	[91/05/22            dbg]
 * 
 * Revision 2.15  91/05/18  14:32:58  rpd
 * 	Added check_simple_locks to thread_block and thread_run.
 * 	[91/05/02            rpd]
 * 	Changed recompute_priorities to use a private timer.
 * 	Changed thread_timeout_setup to initialize depress_timer.
 * 	[91/03/31            rpd]
 * 
 * 	Updated thread_invoke to check stack_privilege.
 * 	[91/03/30            rpd]
 * 
 * Revision 2.14  91/05/14  16:46:16  mrt
 * 	Correcting copyright
 * 
 * Revision 2.13  91/05/08  12:48:33  dbg
 * 	Distinguish processor sets from run queues in choose_pset_thread!
 * 	Remove (long dead) 'someday' code.
 * 	[91/04/26  14:43:26  dbg]
 * 
 * Revision 2.12  91/03/16  14:51:09  rpd
 * 	Added idle_thread_continue, sched_thread_continue.
 * 	[91/01/20            rpd]
 * 
 * 	Allow swapped threads on the run queues.
 * 	Added thread_invoke, thread_select.
 * 	Reorganized thread_block, thread_run.
 * 	Changed the AST interface; idle_thread checks for ASTs now.
 * 	[91/01/17            rpd]
 * 
 * Revision 2.11  91/02/05  17:28:57  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:16:51  mrt]
 * 
 * Revision 2.10  91/01/08  15:16:38  rpd
 * 	Added KEEP_STACKS support.
 * 	[91/01/06            rpd]
 * 	Added thread_continue_calls counter.
 * 	[91/01/03  22:07:43  rpd]
 * 
 * 	Added continuation argument to thread_run.
 * 	[90/12/11            rpd]
 * 	Added continuation argument to thread_block/thread_continue.
 * 	Removed FAST_CSW conditionals.
 * 	[90/12/08            rpd]
 * 
 * 	Removed thread_swap_tick.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.9  90/09/09  14:32:36  rpd
 * 	Removed do_pset_scan call from sched_thread.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.8  90/08/07  22:22:54  rpd
 * 	Fixed casting of a volatile comparison: the non-volatile should
 *	be casted or else.
 * 	Removed silly set_leds() mips thingy.
 * 	[90/08/07  15:56:08  af]
 * 
 * Revision 2.7  90/08/07  17:58:55  rpd
 * 	Removed sched_debug; converted set_pri, update_priority,
 * 	and compute_my_priority to real functions.
 * 	Picked up fix for thread_block, to check for processor set mismatch.
 * 	Record last processor info on all multiprocessors.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.6  90/06/02  14:55:49  rpd
 * 	Updated to new scheduling technology.
 * 	[90/03/26  22:16:03  rpd]
 * 
 * Revision 2.5  90/01/11  11:43:51  dbg
 * 	Check for cpu shutting down on exit from idle loop - next_thread
 * 	will be THREAD_NULL in this case.
 * 	[90/01/03            dbg]
 * 
 * 	Make sure cpu is marked active on all exits from idle loop.
 * 	[89/12/11            dbg]
 * 
 * 	Removed more lint.
 * 	[89/12/05            dbg]
 * 
 * 	DLB's scheduling changes in thread_block don't work if partially
 * 	applied; a thread can run in two places at once.  Revert to old
 * 	code, pending a complete merge.
 * 	[89/12/04            dbg]
 * 
 * Revision 2.4  89/11/29  14:09:11  af
 * 	On Mips, delay setting of active_threads inside load_context,
 * 	or we might take exceptions in an embarassing state.
 * 	[89/11/03  17:00:04  af]
 * 
 * 	Long overdue fix: the pointers that the idle thread uses to check
 * 	for someone to become runnable are now "volatile".  This prevents
 * 	smart compilers from overoptimizing (e.g. Mips).
 * 
 * 	While looking for someone to run in the idle_thread(), rotate
 * 	console lights on Mips to show we're alive [useful when machine
 * 	becomes catatonic].
 * 	[89/10/28            af]
 * 
 * Revision 2.3  89/09/08  11:26:22  dbg
 * 	Add extra thread state cases to thread_switch, since it may now
 * 	be called by a thread about to block.
 * 	[89/08/22            dbg]
 * 
 * 19-Dec-88  David Golub (dbg) at Carnegie-Mellon University
 *	Changes for MACH_KERNEL:
 *	. Import timing definitions from sys/time_out.h.
 *	. Split uses of PMAP_ACTIVATE and PMAP_DEACTIVATE into
 *	  separate _USER and _KERNEL macros.
 *
 * Revision 2.8  88/12/19  02:46:33  mwyoung
 * 	Corrected include file references.  Use <kern/macro_help.h>.
 * 	[88/11/22            mwyoung]
 * 	
 * 	In thread_wakeup_with_result(), only lock threads that have the
 * 	appropriate wait_event.  Both the wait_event and the hash bucket
 * 	links are only modified with both the thread *and* hash bucket
 * 	locked, so it should be safe to read them with either locked.
 * 	
 * 	Documented the wait event mechanism.
 * 	
 * 	Summarized ancient history.
 * 	[88/11/21            mwyoung]
 * 
 * Revision 2.7  88/08/25  18:18:00  mwyoung
 * 	Corrected include file references.
 * 	[88/08/22            mwyoung]
 * 	
 * 	Avoid unsigned computation in wait_hash.
 * 	[88/08/16  00:29:51  mwyoung]
 * 	
 * 	Add priority check to thread_check; make queue index unsigned,
 * 	so that checking works correctly at all.
 * 	[88/08/11  18:47:55  mwyoung]
 * 
 * 11-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Support ast mechanism for threads.  Thread from local_runq gets
 *	minimum quantum to start.
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Moved logic to detect and clear next_thread[] dispatch to
 *	idle_thread() from thread_block().
 *	Maintain first_quantum field in thread instead of runrun.
 *	Changed preempt logic in thread_setrun.
 *	Avoid context switch if current thread is still runnable and
 *	processor would go idle as a result.
 *	Added scanner to unstick stuck threads.
 *
 * Revision 2.6  88/08/06  18:25:03  rpd
 * Eliminated use of kern/mach_ipc_defs.h.
 * 
 * 10-Jul-88  David Golub (dbg) at Carnegie-Mellon University
 *	Check for negative priority (BUG) in thread_setrun.
 *
 * Revision 2.5  88/07/20  16:39:35  rpd
 * Changed "NCPUS > 1" conditionals that were eliminating dead
 * simple locking code to MACH_SLOCKS conditionals.
 * 
 *  7-Jul-88  David Golub (dbg) at Carnegie-Mellon University
 *	Split uses of PMAP_ACTIVATE and PMAP_DEACTIVATE into separate
 *	_USER and _KERNEL macros.
 *
 * 15-Jun-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Removed excessive thread_unlock() occurrences in thread_wakeup.
 *	Problem discovered and solved by Richard Draves.
 *
 * Historical summary:
 *
 *	Redo priority recomputation. [dlb, 29 feb 88]
 *	New accurate timing. [dlb, 19 feb 88]
 *	Simplified choose_thread and thread_block. [dlb, 18 dec 87]
 *	Add machine-dependent hooks in idle loop. [dbg, 24 nov 87]
 *	Quantum scheduling changes. [dlb, 14 oct 87]
 *	Replaced scheduling logic with a state machine, and included
 *	 timeout handling. [dbg, 05 oct 87]
 *	Deactivate kernel pmap in idle_thread. [dlb, 23 sep 87]
 *	Favor local_runq in choose_thread. [dlb, 23 sep 87]
 *	Hacks for master processor handling. [rvb, 12 sep 87]
 *	Improved idle cpu and idle threads logic. [dlb, 24 aug 87]
 *	Priority computation improvements. [dlb, 26 jun 87]
 *	Quantum-based scheduling. [avie, dlb, apr 87]
 *	Improved thread swapper. [avie, 13 mar 87]
 *	Lots of bug fixes. [dbg, mar 87]
 *	Accurate timing support. [dlb, 27 feb 87]
 *	Reductions in scheduler lock contention. [dlb, 18 feb 87]
 *	Revise thread suspension mechanism. [avie, 17 feb 87]
 *	Real thread handling [avie, 31 jan 87]
 *	Direct idle cpu dispatching. [dlb, 19 jan 87]
 *	Initial processor binding. [avie, 30 sep 86]
 *	Initial sleep/wakeup. [dbg, 12 jun 86]
 *	Created. [avie, 08 apr 86]
 */
/*
 *	File:	sched_prim.c
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1986
 *
 *	Scheduling primitives
 *
 */

#include <cpus.h>
#include <fast_tas.h>

#include <mach/boolean.h>
#include <mach/kern_return.h>

#include <ipc/mach_msg.h>

#include <kern/ast.h>
#include <kern/counters.h>
#include <kern/cpu_number.h>
#include <kern/lock.h>
#include <kern/mach_timer.h>
#include <kern/processor.h>
#include <kern/queue.h>
#include <kern/sched_policy.h>
#include <kern/sched_prim.h>
#include <kern/stack.h>
#include <kern/thread.h>
#include <kern/thread_swap.h>

#include <machine/machspl.h>	/* For def'n of splsched() */

/*
 *	State machine
 *
 * states are combinations of:
 *  R	running
 *  W	waiting (or on wait queue)
 *  S	suspended (or will suspend)
 *  N	non-interruptible
 *
 * init	action 
 *	assert_wait	thread_block	clear_wait	suspend	resume
 *
 * R	RW,  RWN	R;   setrun	-		RS	-
 * RS	RWS, RWNS	S;		-		-	R
 *			 suspend_wakeup
 * RN	RWN		RN;  setrun	-		RNS	-
 * RNS	RWNS		RNS; setrun	-		-	RN
 *
 * RW			W		R		RWS	-
 * RWN			WN		RN		RWNS	-
 * RWS			WS;		RS		-	RW
 *			 suspend_wakeup
 * RWNS			WNS		RNS		-	RWN
 *
 * W					R;   setrun	WS	-
 * WN					RN;  setrun	WNS	-
 * WNS					RNS; setrun	-	WN
 *
 * S					-		-	R
 * WS					S		-	W
 */

/*
 *	Waiting protocols and implementation:
 *
 *	Each thread may be waiting for exactly one event; this event
 *	is set using assert_wait().  That thread may be awakened either
 *	by performing a thread_wakeup_prim() on its event,
 *	or by directly waking that thread up with clear_wait().
 *
 *	The implementation of wait events uses a hash table.  Each
 *	bucket is queue of threads having the same hash function
 *	value; the chain for the queue (linked list) is the run queue
 *	field.  [It is not possible to be waiting and runnable at the
 *	same time.]
 *
 *	Locks on both the thread and on the hash buckets govern the
 *	wait event field and the queue chain field.  Because wakeup
 *	operations only have the event as an argument, the event hash
 *	bucket must be locked before any thread.
 *
 *	Scheduling operations may also occur at interrupt level; therefore,
 *	interrupts below splsched() must be prevented when holding
 *	thread or hash bucket locks.
 *
 *	The wait event hash table declarations are as follows:
 */

#define NUMQUEUES	59

queue_head_t		wait_queue[NUMQUEUES];
decl_simple_lock_data(,	wait_lock[NUMQUEUES])

/* NOTE: we want a small positive integer out of this */
#define wait_hash(event) \
	((int)((((integer_t)(event) < 0) ? ~(integer_t)(event) \
					 :  (integer_t)(event)) % NUMQUEUES))

void sched_init(void)
{
	register int i;

	for (i = 0; i < NUMQUEUES; i++) {
		queue_init(&wait_queue[i]);
		simple_lock_init(&wait_lock[i]);
	}
}

/*
 *	Routine:	thread_will_wait
 *	Purpose:
 *		Assert that the thread intends to block.
 *		An event is not supplied; the thread
 *		will be awakened with clear_wait or
 *		thread_go.
 */

void
thread_will_wait(
	thread_t thread)
{
	spl_t s;

	s = splsched();
	thread_sched_lock(thread);

	assert(thread->wait_result = -1);	/* for later assertions */
	thread->state |= TH_WAIT;

	thread_sched_unlock(thread);
	splx(s);
}

/*
 *	Routine:	thread_will_wait_with_timeout
 *	Purpose:
 *		Assert that the thread intends to block,
 *		with a timeout.
 */
void thread_timeout(
	void	*param);	/*  forward */

void
thread_will_wait_with_timeout(
	thread_t thread,
	mach_msg_timeout_t msecs)
{
	time_spec_t	interval;
	spl_t s;

	milliseconds_to_time_spec(msecs, interval);

	s = splsched();
	thread_sched_lock(thread);

	assert(thread->wait_result = -1);	/* for later assertions */
	thread->state |= TH_WAIT;

	thread->timer.te_fcn = thread_timeout;
	timer_elt_enqueue(&thread->timer, interval, FALSE);

	thread_sched_unlock(thread);
	splx(s);
}

/*
 *	Routine:	thread_go
 *	Purpose:
 *		Start a thread running.
 *	Conditions:
 *		IPC locks may be held.
 */

void
thread_go(
	thread_t thread)
{
	int state;
	spl_t s;

	s = splsched();
	thread_sched_lock(thread);

	timer_elt_remove(&thread->timer);

	state = thread->state;
	switch (state & TH_SCHED_STATE) {

	    case TH_WAIT | TH_SUSP | TH_UNINT:
	    case TH_WAIT	   | TH_UNINT:
	    case TH_WAIT:
		/*
		 *	Sleeping and not suspendable - put
		 *	on run queue.
		 */
		thread->state = (state &~ TH_WAIT) | TH_RUN;
		thread->wait_result = THREAD_AWAKENED;
		thread_setrun(thread, TRUE);
		break;

	    case	  TH_WAIT | TH_SUSP:
	    case TH_RUN | TH_WAIT:
	    case TH_RUN | TH_WAIT | TH_SUSP:
	    case TH_RUN | TH_WAIT	    | TH_UNINT:
	    case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
		/*
		 *	Either already running, or suspended.
		 */
		thread->state = state & ~TH_WAIT;
		thread->wait_result = THREAD_AWAKENED;
		break;

	    default:
		/*
		 *	Not waiting.
		 */
		break;
	}

	thread_sched_unlock(thread);
	splx(s);
}

/*
 *	assert_wait:
 *
 *	Assert that the current thread is about to go to
 *	sleep until the specified event occurs.
 */
void assert_wait(
	event_t		event,
	boolean_t	interruptible)
{
	register queue_t	q;
	register int		index;
	register thread_t	thread;
#if	MACH_SLOCKS
	register simple_lock_t	lock;
#endif	/* MACH_SLOCKS */
	spl_t			s;

	thread = current_thread();
	if (thread->wait_event != 0) {
		panic("assert_wait: already asserted event %#x\n",
		      thread->wait_event);
	}
 	s = splsched();
	if (event != 0) {
		index = wait_hash(event);
		q = &wait_queue[index];
#if	MACH_SLOCKS
		lock = &wait_lock[index];
#endif	/* MACH_SLOCKS */
		simple_lock(lock);
		thread_sched_lock(thread);
		enqueue_tail(q, (queue_entry_t) thread);
		thread->wait_event = event;
		if (interruptible)
			thread->state |= TH_WAIT;
		else
			thread->state |= TH_WAIT | TH_UNINT;
		thread_sched_unlock(thread);
		simple_unlock(lock);
	}
	else {
		thread_sched_lock(thread);
		if (interruptible)
			thread->state |= TH_WAIT;
		else
			thread->state |= TH_WAIT | TH_UNINT;
		thread_sched_unlock(thread);
	}
	splx(s);
}

/*
 *	clear_wait:
 *
 *	Clear the wait condition for the specified thread.  Start the thread
 *	executing if that is appropriate.
 *
 *	parameters:
 *	  thread		thread to awaken
 *	  result		Wakeup result the thread should see
 *	  interrupt_only	Don't wake up the thread if it isn't
 *				interruptible.
 */
void clear_wait(
	register thread_t	thread,
	int			result,
	boolean_t		interrupt_only)
{
	register int		index;
	register queue_t	q;
#if	MACH_SLOCKS
	register simple_lock_t	lock;
#endif	/* MACH_SLOCKS */
	register event_t	event;
	spl_t			s;

	s = splsched();
	thread_sched_lock(thread);
	if (interrupt_only && (thread->state & TH_UNINT)) {
		/*
		 *	can`t interrupt thread
		 */
		thread_sched_unlock(thread);
		splx(s);
		return;
	}

	event = thread->wait_event;
	if (event != 0) {
		thread_sched_unlock(thread);
		index = wait_hash(event);
		q = &wait_queue[index];
#if	MACH_SLOCKS
		lock = &wait_lock[index];
#endif	/* MACH_SLOCKS */
		simple_lock(lock);
		/*
		 *	If the thread is still waiting on that event,
		 *	then remove it from the list.  If it is waiting
		 *	on a different event, or no event at all, then
		 *	someone else did our job for us.
		 */
		thread_sched_lock(thread);
		if (thread->wait_event == event) {
			remqueue(q, (queue_entry_t)thread);
			thread->wait_event = 0;
			event = 0;		/* cause to run below */
		}
		simple_unlock(lock);
	}
	if (event == 0) {
		register int	state = thread->state;

		timer_elt_remove(&thread->timer);

		switch (state & TH_SCHED_STATE) {
		    case	  TH_WAIT | TH_SUSP | TH_UNINT:
		    case	  TH_WAIT	    | TH_UNINT:
		    case	  TH_WAIT:
			/*
			 *	Sleeping and not suspendable - put
			 *	on run queue.
			 */
			thread->state = (state &~ TH_WAIT) | TH_RUN;
			thread->wait_result = result;
			thread_setrun(thread, TRUE);
			break;

		    case	  TH_WAIT | TH_SUSP:
		    case TH_RUN | TH_WAIT:
		    case TH_RUN | TH_WAIT | TH_SUSP:
		    case TH_RUN | TH_WAIT	    | TH_UNINT:
		    case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
			/*
			 *	Either already running, or suspended.
			 */
			thread->state = state &~ TH_WAIT;
			thread->wait_result = result;
			break;

		    default:
			/*
			 *	Not waiting.
			 */
			break;
		}
	}
	thread_sched_unlock(thread);
	splx(s);
}

/*
 *	thread_wakeup_prim:
 *
 *	Common routine for thread_wakeup, thread_wakeup_with_result,
 *	and thread_wakeup_one.
 *
 */
void thread_wakeup_prim(
	event_t		event,
	boolean_t	one_thread,
	int		result)
{
	register queue_t	q;
	register int		index;
	register thread_t	thread, next_th;
#if	MACH_SLOCKS
	register simple_lock_t	lock;
#endif	/* MACH_SLOCKS */
	spl_t			s;
	register int		state;

	index = wait_hash(event);
	q = &wait_queue[index];
	s = splsched();
#if	MACH_SLOCKS
	lock = &wait_lock[index];
#endif	/* MACH_SLOCKS */
	simple_lock(lock);
	thread = (thread_t) queue_first(q);
	while (!queue_end(q, (queue_entry_t)thread)) {
		next_th = (thread_t) queue_next((queue_t) thread);

		if (thread->wait_event == event) {
			thread_sched_lock(thread);
			remqueue(q, (queue_entry_t) thread);
			thread->wait_event = 0;
			timer_elt_remove(&thread->timer);

			state = thread->state;
			switch (state & TH_SCHED_STATE) {

			    case	  TH_WAIT | TH_SUSP | TH_UNINT:
			    case	  TH_WAIT	    | TH_UNINT:
			    case	  TH_WAIT:
				/*
				 *	Sleeping and not suspendable - put
				 *	on run queue.
				 */
				thread->state = (state &~ TH_WAIT) | TH_RUN;
				thread->wait_result = result;
				thread_setrun(thread, TRUE);
				break;

			    case	  TH_WAIT | TH_SUSP:
			    case TH_RUN | TH_WAIT:
			    case TH_RUN | TH_WAIT | TH_SUSP:
			    case TH_RUN | TH_WAIT	    | TH_UNINT:
			    case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
				/*
				 *	Either already running, or suspended.
				 */
				thread->state = state &~ TH_WAIT;
				thread->wait_result = result;
				break;

			    default:
				panic("thread_wakeup");
				break;
			}
			thread_sched_unlock(thread);
			if (one_thread)
				break;
		}
		thread = next_th;
	}
	simple_unlock(lock);
	splx(s);
}

/*
 *	thread_sleep:
 *
 *	Cause the current thread to wait until the specified event
 *	occurs.  The specified lock is unlocked before releasing
 *	the cpu.  (This is a convenient way to sleep without manually
 *	calling assert_wait).
 */
void thread_sleep(
	event_t		event,
	simple_lock_t	lock,
	boolean_t	interruptible)
{
	assert_wait(event, interruptible);	/* assert event */
	simple_unlock(lock);			/* release the lock */
	thread_block(CONTINUE_NULL);		/* block ourselves */
}

/*
 *	Routine:	thread_handoff
 *	Purpose:
 *		Switch to a new thread (new), leaving the current
 *		thread (old) blocked.  If successful, moves the
 *		kernel stack from old to new and returns as the
 *		new thread.  An explicit continuation for the old thread
 *		must be supplied.
 *
 *		NOTE:  Although we wakeup new, we don't set new->wait_result.
 *	Returns:
 *		TRUE if the handoff happened.
 */

boolean_t
thread_handoff(
	register thread_t old,
	register continuation_t continuation,
	register thread_t new)
{
	spl_t s;

	assert(current_thread() == old);

	/*
	 *	XXX Dubious things here:
	 *	I don't check the idle_count on the processor set.
	 *	No scheduling priority or policy checks.
	 *	I assume the new thread is interruptible.
	 */

	s = splsched();
	thread_sched_lock(new);

	/*
	 *	The first thing we must do is check the state
	 *	of the threads, to ensure we can handoff.
	 *	This check uses current_processor()->processor_set,
	 *	which we can read without locking.
	 */

	if ((old->stack_privilege == current_stack()) ||
	    (new->state != (TH_WAIT|TH_SWAPPED)) ||
	     !check_processor_set(new) ||
	     !check_bound_processor(new)) {
		thread_sched_unlock(new);
		splx(s);

		counter_always(c_thread_handoff_misses++);
		return FALSE;
	}

	timer_elt_remove(&new->timer);

	new->state = TH_RUN;
	thread_sched_unlock(new);

#if	NCPUS > 1
	new->last_processor = current_processor();
#endif	/* NCPUS > 1 */

	ast_context(new, cpu_number());
	timer_switch(&new->system_timer);

	/*
	 *	stack_handoff is machine-dependent.  It does the
	 *	machine-dependent components of a context-switch, like
	 *	changing address spaces.  It updates active_threads.
	 */

	stack_handoff(old, new);

	/*
	 *	Now we must dispose of the old thread.
	 *	This is like thread_continue, except
	 *	that the old thread isn't waiting yet.
	 */

	thread_sched_lock(old);
	old->swap_func = continuation;
	assert(old->wait_result = -1);		/* for later assertions */

	if (old->state == TH_RUN) {
		/*
		 *	This is our fast path.
		 */

		old->state = TH_WAIT|TH_SWAPPED;
	}
	else if (old->state == (TH_RUN|TH_SUSP)) {
		/*
		 *	Somebody is trying to suspend the thread.
		 */

		old->state = TH_WAIT|TH_SUSP|TH_SWAPPED;
		if (old->suspend_wait) {
			/*
			 *	Someone wants to know when the thread
			 *	really stops.
			 */
			old->suspend_wait = FALSE;
			thread_sched_unlock(old);
			thread_wakeup((event_t) &old->suspend_wait);
			goto after_old_thread;
		}
	} else
		panic("thread_handoff");

	thread_sched_unlock(old);
    after_old_thread:
	splx(s);

	counter_always(c_thread_handoff_hits++);
	return TRUE;
}

/*
 *	[internal]
 *
 *	Stop running the current thread and start running the new thread.
 *	If continuation is non-zero, and the current thread is blocked,
 *	then it will resume by executing continuation on a new stack.
 *	Returns TRUE if the hand-off succeeds.
 *	Assumes splsched.
 */

boolean_t thread_invoke(
	register thread_t old_thread,
	continuation_t	  continuation,
	register thread_t new_thread)
{
	register int	mycpu = cpu_number();

	/*
	 *	Check for invoking the same thread.
	 */
	if (old_thread == new_thread) {
	    /*
	     *	Mark thread interruptible.
	     *	Run continuation if there is one.
	     */
	    thread_sched_lock(new_thread);
	    new_thread->state &= ~TH_UNINT;
	    thread_sched_unlock(new_thread);

	    if (continuation != (void (*)(void)) 0) {
		(void) spl0();

		/*
		 * Check for asynchronous kernel activities
		 * here - we expected to context switch.
		 */
		AST_KERNEL_CHECK(mycpu);
		call_continuation(continuation);
		/*NOTREACHED*/
	    }
	    return TRUE;
	}

	/*
	 *	Check for stack-handoff.
	 */
	thread_sched_lock(new_thread);
	if ((old_thread->stack_privilege != current_stack()) &&
	    (continuation != (void (*)(void)) 0))
	{
	    switch (new_thread->state & TH_SWAP_STATE) {
		case TH_SWAPPED:

		    new_thread->state &= ~(TH_SWAPPED | TH_UNINT);
		    thread_sched_unlock(new_thread);

#if	NCPUS > 1
		    new_thread->last_processor = current_processor();
#endif	/* NCPUS > 1 */

		    /*
		     *	Set up ast context of new thread and
		     *	switch to its timer.
		     */
		    ast_context(new_thread, mycpu);
		    timer_switch(&new_thread->system_timer);

		    stack_handoff(old_thread, new_thread);

		    /*
		     *	We can dispatch the old thread now.
		     *	This is like thread_dispatch, except
		     *	that the old thread is left swapped
		     *	*without* freeing its stack.
		     *	This path is also much more frequent
		     *	than actual calls to thread_dispatch.
		     */

		    thread_sched_lock(old_thread);
		    old_thread->swap_func = continuation;

		    switch (old_thread->state) {
			case TH_RUN	      | TH_SUSP:
			case TH_RUN	      | TH_SUSP | TH_HALTED:
			case TH_RUN | TH_WAIT | TH_SUSP:
			    /*
			     *	Suspend the thread
			     */
			    old_thread->state = (old_thread->state & ~TH_RUN)
						| TH_SWAPPED;
			    if (old_thread->suspend_wait) {
				old_thread->suspend_wait = FALSE;
				thread_sched_unlock(old_thread);
				thread_wakeup(
					(event_t) &old_thread->suspend_wait);
				goto after_old_thread;
			    }
			    break;

			case TH_RUN	      | TH_SUSP | TH_UNINT:
			case TH_RUN			| TH_UNINT:
			case TH_RUN:
			    /*
			     *	We can`t suspend the thread yet,
			     *	or it`s still running.
			     *	Put back on a run queue.
			     */
			    old_thread->state |= TH_SWAPPED;
			    thread_setrun(old_thread, FALSE);
			    break;

			case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
			case TH_RUN | TH_WAIT		| TH_UNINT:
			case TH_RUN | TH_WAIT:
			    /*
			     *	Waiting, and not suspendable.
			     */
			    old_thread->state = (old_thread->state & ~TH_RUN)
						| TH_SWAPPED;
			    break;

			case TH_RUN | TH_IDLE:
			    /*
			     *	Drop idle thread -- it is already in
			     *	idle_thread_array.
			     */
			    old_thread->state = TH_RUN | TH_IDLE | TH_SWAPPED;
			    break;

			default:
			    panic("thread_invoke");
		    }
		    thread_sched_unlock(old_thread);
		after_old_thread:

		    /*
		     *	call_continuation calls the continuation
		     *	after resetting the current stack pointer
		     *	to recover stack space.  If we called
		     *	the continuation directly, we would risk
		     *	running out of stack.
		     */

		    counter_always(c_thread_invoke_hits++);
		    (void) spl0();
		    AST_KERNEL_CHECK(mycpu);
		    call_continuation(new_thread->swap_func);
		    /*NOTREACHED*/
		    return TRUE; /* help for the compiler */

		case TH_SW_COMING_IN:
		    /*
		     *	Waiting for a stack
		     */
		    thread_swapin(new_thread);
		    thread_sched_unlock(new_thread);
		    counter_always(c_thread_invoke_misses++);
		    return FALSE;

		case 0:
		    /*
		     *	Already has a stack - can`t handoff.
		     */
		    break;
	    }
	}

	else {
	    /*
	     *	Check that the thread is swapped-in.
	     */
	    if (new_thread->state & TH_SWAPPED) {
		if ((new_thread->state & TH_SW_COMING_IN) ||
		    !stack_alloc_try(new_thread, thread_continue))
		{
		    thread_swapin(new_thread);
		    thread_sched_unlock(new_thread);
		    counter_always(c_thread_invoke_misses++);
		    return FALSE;
		}
	    }
	}

	new_thread->state &= ~(TH_SWAPPED | TH_UNINT);
	thread_sched_unlock(new_thread);

	/*
	 *	Thread is now interruptible.
	 */
#if	NCPUS > 1
	new_thread->last_processor = current_processor();
#endif	/* NCPUS > 1 */

	/*
	 *	Set up ast context of new thread and switch to its timer.
	 */
	ast_context(new_thread, mycpu);
	timer_switch(&new_thread->system_timer);

	/*
	 *	switch_context is machine-dependent.  It does the
	 *	machine-dependent components of a context-switch, like
	 *	changing address spaces.  It updates active_threads.
	 *	It returns only if a continuation is not supplied.
	 */
	counter_always(c_thread_invoke_csw++);
	old_thread = switch_context(old_thread, continuation, new_thread);

	/*
	 *	We're back.  Now old_thread is the thread that resumed
	 *	us, and we have to dispatch it.
	 */
	thread_dispatch(old_thread);

	return TRUE;
}

/*
 *	thread_continue:
 *
 *	Called when the current thread is given a new stack.
 *	Called at splsched.
 */
no_return thread_continue(
	register thread_t old_thread)
{
	/*
	 *	We must dispatch the old thread and then
	 *	call the current thread's continuation.
	 *	There might not be an old thread, if we are
	 *	the first thread to run on this processor.
	 */

	if (old_thread != THREAD_NULL)
		thread_dispatch(old_thread);
	(void) spl0();

	/*
	 *	Before starting the new thread, check for
	 *	asynchronous kernel activities.
	 */
	AST_KERNEL_CHECK(cpu_number());

	(*current_thread()->swap_func)();

	/*NOTREACHED*/
}


/*
 *	thread_block:
 *
 *	Block the current thread.  If the thread is runnable
 *	then someone must have woken it up between its request
 *	to sleep and now.  In this case, it goes back on a
 *	run queue.
 *
 *	If a continuation is specified, then thread_block will
 *	attempt to discard the thread's kernel stack.  When the
 *	thread resumes, it will execute the continuation function
 *	on a new kernel stack.
 */

void thread_block(
	continuation_t	continuation)
{
	int	mycpu = cpu_number();
	register thread_t thread = active_threads[mycpu];
	register processor_t myprocessor = cpu_to_processor(mycpu);
	register thread_t new_thread;
	spl_t s;

	check_simple_locks();

	s = splsched();

#if	FAST_TAS
	{
	    extern void recover_ras(thread_t);

	    if (csw_needed(thread, myprocessor))
		recover_ras(thread);
	}
#endif	/* FAST_TAS */
	
	ast_off(mycpu, AST_BLOCK);

	do
		new_thread = thread_select(myprocessor);
	while (!thread_invoke(thread, continuation, new_thread));

	/*
	 *	Check for asynchronous kernel activities before
	 *	resuming the new thread.  We already hold splhigh.
	 */
	AST_KERNEL_CHECK_HIGH(mycpu);

	splx(s);
}

/*
 *	thread_run:
 *
 *	Switch directly from the current thread to a specified
 *	thread.  Both the current and new threads must be
 *	runnable.
 *
 *	If a continuation is specified, then thread_block will
 *	attempt to discard the current thread's kernel stack.  When the
 *	thread resumes, it will execute the continuation function
 *	on a new kernel stack.
 */
void thread_run(
	continuation_t		continuation,
	register thread_t	new_thread)
{
	int	mycpu = cpu_number();
	register thread_t thread = active_threads[mycpu];
	register processor_t myprocessor = cpu_to_processor(mycpu);
	spl_t	s;

	check_simple_locks();

	s = splsched();

	while (!thread_invoke(thread, continuation, new_thread))
		new_thread = thread_select(myprocessor);

	/*
	 *	Check for asynchronous kernel activities before
	 *	resuming the new thread.  We already hold splhigh.
	 */
	AST_KERNEL_CHECK_HIGH(mycpu);

	splx(s);
}

/*
 *	Dispatches a running thread that is not	on a runq.
 *	Called at splsched.
 */

void thread_dispatch(
	register thread_t	thread)
{
	/*
	 *	If we are discarding the thread's stack, we must do it
	 *	before the thread has a chance to run.
	 */

	thread_sched_lock(thread);

	if (thread->swap_func != 0) {
		assert((thread->state & TH_SWAP_STATE) == 0);
		thread->state |= TH_SWAPPED;
		stack_free(thread);
	}

	switch (thread->state &~ TH_SWAP_STATE) {
	    case TH_RUN		  | TH_SUSP:
	    case TH_RUN		  | TH_SUSP | TH_HALTED:
	    case TH_RUN | TH_WAIT | TH_SUSP:
		/*
		 *	Suspend the thread
		 */
		thread->state &= ~TH_RUN;
		if (thread->suspend_wait) {
		    thread->suspend_wait = FALSE;
		    thread_sched_unlock(thread);
		    thread_wakeup((event_t) &thread->suspend_wait);
		    return;
		}
		break;

	    case TH_RUN		  | TH_SUSP | TH_UNINT:
	    case TH_RUN			    | TH_UNINT:
	    case TH_RUN:
		/*
		 *	No reason to stop.  Put back on a run queue.
		 */
		thread_setrun(thread, FALSE);
		break;

	    case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
	    case TH_RUN | TH_WAIT	    | TH_UNINT:
	    case TH_RUN | TH_WAIT:
		/*
		 *	Waiting, and not suspended.
		 */
		thread->state &= ~TH_RUN;
		break;

	    case TH_RUN | TH_IDLE:
		/*
		 *	Drop idle thread -- it is already in
		 *	idle_thread_array.
		 */
		break;

	    default:
		panic("thread_dispatch");
	}
	thread_sched_unlock(thread);
}


/*
 *	Thread timeout routine, called when timer expires.
 *	Called at splsoftclock.
 */
void thread_timeout(
	void	*param)
{
	thread_t	thread = (thread_t) param;

	assert(thread->timer.set == TELT_UNSET);

	clear_wait(thread, THREAD_TIMED_OUT, FALSE);
}

/*
 *	thread_set_timeout:
 *
 *	Set a timer for the current thread, if the thread
 *	is ready to wait.  Must be called between assert_wait()
 *	and thread_block().
 */
 
void thread_set_timeout(
	int	msecs)		/* timeout interval in milliseconds */
{
	thread_t	thread = current_thread();
	time_spec_t	interval;
	spl_t s;

	milliseconds_to_time_spec(msecs, interval);

	s = splsched();
	thread_sched_lock(thread);
	if ((thread->state & TH_WAIT) != 0) {
		thread->timer.te_fcn = thread_timeout;
		timer_elt_enqueue(&thread->timer, interval, FALSE);
	}
	thread_sched_unlock(thread);
	splx(s);
}

/*
 *	Halt a thread at a clean point, leaving it suspended.
 *
 *	must_halt indicates whether thread must halt.
 *
 */
kern_return_t thread_halt(
	register thread_t	thread,
	boolean_t		must_halt)
{
	register thread_t	cur_thread = current_thread();
	register kern_return_t	ret;
	spl_t	s;
#if	MACH_HOST
	processor_set_t		old_pset = PROCESSOR_SET_NULL;
#endif

	if (thread == cur_thread)
		panic("thread_halt: trying to halt current thread.");

	/*
	 *	If must_halt is FALSE, then a check must be made for
	 *	a cycle of halt operations.
	 */
	if (!must_halt) {
		/*
		 *	Grab both thread locks.
		 */
		s = splsched();
		if ((vm_offset_t)thread < (vm_offset_t)cur_thread) {
			thread_sched_lock(thread);
			thread_sched_lock(cur_thread);
		}
		else {
			thread_sched_lock(cur_thread);
			thread_sched_lock(thread);
		}

		/*
		 *	If target thread is already halted, grab a hold
		 *	on it and return.
		 */
		if (thread->state & TH_HALTED) {
			thread->suspend_count++;
			thread_sched_unlock(cur_thread);
			thread_sched_unlock(thread);
			splx(s);
			return KERN_SUCCESS;
		}

		/*
		 *	If someone is trying to halt us, we have a potential
		 *	halt cycle.  Break the cycle by interrupting anyone
		 *	who is trying to halt us, and causing this operation
		 *	to fail; retry logic will only retry operations
		 *	that cannot deadlock.  (If must_halt is TRUE, this
		 *	operation can never cause a deadlock.)
		 */
		if (cur_thread->ast & AST_HALT) {
			thread_wakeup_with_result(&cur_thread->suspend_wait,
				THREAD_INTERRUPTED);
			thread_sched_unlock(thread);
			thread_sched_unlock(cur_thread);
			splx(s);
			return KERN_FAILURE;
		}

		thread_sched_unlock(cur_thread);
	
	}
	else {
		/*
		 *	Lock thread and check whether it is already halted.
		 */
		s = splsched();
		thread_sched_lock(thread);
		if (thread->state & TH_HALTED) {
			thread->suspend_count++;
			thread_sched_unlock(thread);
			splx(s);
			return KERN_SUCCESS;
		}
	}

	/*
	 *	Suspend thread - inline version of thread_hold() because
	 *	thread is already locked.
	 */
	thread->suspend_count++;
	thread->state |= TH_SUSP;

	/*
	 *	If someone else is halting it, wait for that to complete.
	 *	Fail if wait interrupted and must_halt is false.
	 */
	while ((thread->ast & AST_HALT) && (!(thread->state & TH_HALTED))) {
		thread->suspend_wait = TRUE;
		thread_sleep(&thread->suspend_wait,
			simple_lock_addr(thread->sched_lock), TRUE);

		if (thread->state & TH_HALTED) {
			splx(s);
			return KERN_SUCCESS;
		}
		if ((current_thread()->wait_result != THREAD_AWAKENED)
		    && !(must_halt)) {
			splx(s);
			thread_release(thread);
			return KERN_FAILURE;
		}
		thread_sched_lock(thread);
	}

	/*
	 *	Otherwise, have to do it ourselves.
	 */
		
	thread_ast_set(thread, AST_HALT);

	while (TRUE) {
	  	/*
		 *	Wait for thread to stop.
		 */
		thread_sched_unlock(thread);
		splx(s);

		ret = thread_dowait(thread, must_halt);

		/*
		 *	If the dowait failed, so do we.  Drop AST_HALT, and
		 *	wake up anyone else who might be waiting for it.
		 */
		if (ret != KERN_SUCCESS) {
			s = splsched();
			thread_sched_lock(thread);
			thread_ast_clear(thread, AST_HALT);
			thread_wakeup_with_result(&thread->suspend_wait,
				THREAD_INTERRUPTED);
			thread_sched_unlock(thread);
			splx(s);

			thread_release(thread);
			break;
		}

		/*
		 *	Clear any interruptible wait.
		 */
		clear_wait(thread, THREAD_INTERRUPTED, TRUE);

		/*
		 *	If the thread's at a clean point, we're done.
		 *	Don't need a lock because it really is stopped.
		 */
		if (thread->state & TH_HALTED) {
			ret = KERN_SUCCESS;
			break;
		}

		/*
		 *	If the thread is at a nice continuation,
		 *	or a continuation with a cleanup routine,
		 *	call the cleanup routine.
		 */
		if (((thread->swap_func == mach_msg_continue ||
		      thread->swap_func == mach_msg_receive_continue) &&
		     mach_msg_interrupt(thread))
		 || thread->swap_func == thread_exception_return
		 || thread->swap_func == thread_bootstrap_return
		 || evc_wait_interrupt(thread)) {
			s = splsched();
			thread_sched_lock(thread);
			thread->state |= TH_HALTED;
			thread_ast_clear(thread, AST_HALT);
			thread_sched_unlock(thread);
			splx(s);

			ret = KERN_SUCCESS;
			break;
		}

		/*
		 *	Force the thread to stop at a clean
		 *	point, and arrange to wait for it.
		 *
		 *	Set it running, so it can notice.  Override
		 *	the suspend count.  We know that the thread
		 *	is suspended and not waiting.
		 *
		 *	Since the thread may hit an interruptible wait
		 *	before it reaches a clean point, we must force it
		 *	to wake us up when it does so.  This involves some
		 *	trickery:
		 *	  We mark the thread SUSPENDED so that thread_block
		 *	will suspend it and wake us up.
		 *	  We mark the thread RUNNING so that it will run.
		 *	  We mark the thread UN-INTERRUPTIBLE (!) so that
		 *	some other thread trying to halt or suspend it won't
		 *	take it off the run queue before it runs.  Since
		 *	dispatching a thread (the tail of thread_invoke) marks
		 *	the thread interruptible, it will stop at the next
		 *	context switch or interruptible wait.
		 */

#if	MACH_HOST
		/*
		 *	But first, if the thread is a member of
		 *	an empty processor set, we must move it
		 *	to the default processor set so it can
		 *	execute.
		 */
		if (old_pset == PROCESSOR_SET_NULL)
			old_pset = thread_assign_if_empty(thread);
				/* donates a reference */
#endif

		s = splsched();
		thread_sched_lock(thread);
		if ((thread->state & TH_SCHED_STATE) != TH_SUSP)
			panic("thread_halt");
		thread->state |= TH_RUN | TH_UNINT;
		thread_setrun(thread, FALSE);

		/*
		 *	Continue loop and wait for thread to stop.
		 */
	}

#if	MACH_HOST
	/*
	 *	Reassign thread to its original processor set.
	 */
	if (old_pset != PROCESSOR_SET_NULL) {
		(void) thread_assign(thread, old_pset);
		pset_deallocate(old_pset);	/* remove extra reference */
	}
#endif

	/*
	 *	Thread is now halted.
	 */

	return ret;
}

/*
 *	Thread calls this routine on exit from the kernel when it
 *	notices a halt request.
 */
void thread_halt_self(
	continuation_t	continuation)
{
	register thread_t	thread = current_thread();
	spl_t	s;

	/*
	 *	Thread was asked to halt - show that it
	 *	has done so.
	 */
	s = splsched();
	thread_sched_lock(thread);
	thread->state |= TH_HALTED;
	thread_ast_clear(thread, AST_HALT);
	thread_sched_unlock(thread);
	splx(s);

	counter(c_thread_halt_self_block++);
	thread_block(continuation);

	/*
	 *	thread_release resets TH_HALTED.
	 */
}

/*
 *	thread_hold:
 *
 *	Suspend execution of the specified thread.
 *	This is a recursive-style suspension of the thread, a count of
 *	suspends is maintained.
 */
void thread_hold(
	register thread_t	thread)
{
	spl_t	s;

	s = splsched();
	thread_sched_lock(thread);
	thread->suspend_count++;
	thread->state |= TH_SUSP;
	thread_sched_unlock(thread);
	splx(s);
}

/*
 *	thread_dowait:
 *
 *	Wait for a thread to actually enter stopped state.
 *
 *	must_halt argument indicates if this may fail on interruption.
 *	This is FALSE only if called from thread_abort via thread_halt.
 */
kern_return_t
thread_dowait(
	register thread_t	thread,
	boolean_t		must_halt)
{
	register boolean_t	need_wakeup;
	register kern_return_t	ret = KERN_SUCCESS;
	spl_t			s;

	if (thread == current_thread())
		panic("thread_dowait");

	/*
	 *	If a thread is not interruptible, it may not be suspended
	 *	until it becomes interruptible.  In this case, we wait for
	 *	the thread to stop itself, and indicate that we are waiting
	 *	for it to stop so that it can wake us up when it does stop.
	 *
	 *	If the thread is interruptible, we may be able to suspend
	 *	it immediately.  There are several cases:
	 *
	 *	1) The thread is already stopped (trivial)
	 *	2) The thread is runnable (marked RUN and on a run queue).
	 *	   We pull it off the run queue and mark it stopped.
	 *	3) The thread is running.  We wait for it to stop.
	 */

	need_wakeup = FALSE;
	s = splsched();
	thread_sched_lock(thread);

	for (;;) {
	    switch (thread->state & TH_SCHED_STATE) {
		case			TH_SUSP:
		case	      TH_WAIT | TH_SUSP:
		    /*
		     *	Thread is already suspended, or sleeping in an
		     *	interruptible wait.  We win!
		     */
		    break;

		case TH_RUN	      | TH_SUSP:
		    /*
		     *	The thread is interruptible.  If we can pull
		     *	it off a runq, stop it here.
		     */
		    if (rem_runq(thread) != RUN_QUEUE_HEAD_NULL) {
			thread->state &= ~TH_RUN;
			need_wakeup = thread->suspend_wait;
			thread->suspend_wait = FALSE;
			break;
		    }
#if	NCPUS > 1
		    /*
		     *	The thread must be running, so make its
		     *	processor execute ast_check().  This
		     *	should cause the thread to take an ast and
		     *	context switch to suspend for us.
		     */
		    cause_ast_check(thread->last_processor);
#endif	/* NCPUS > 1 */

		    /*
		     *	Fall through to wait for thread to stop.
		     */

		case TH_RUN	      | TH_SUSP | TH_UNINT:
		case TH_RUN | TH_WAIT | TH_SUSP:
		case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
		case	      TH_WAIT | TH_SUSP | TH_UNINT:
		    /*
		     *	Wait for the thread to stop, or sleep interruptibly
		     *	(thread_block will stop it in the latter case).
		     *	Check for failure if interrupted.
		     */
		    thread->suspend_wait = TRUE;
		    thread_sleep(&thread->suspend_wait,
				simple_lock_addr(thread->sched_lock), TRUE);
		    thread_sched_lock(thread);
		    if ((current_thread()->wait_result != THREAD_AWAKENED) &&
			    !must_halt) {
			ret = KERN_FAILURE;
			break;
		    }

		    /*
		     *	Repeat loop to check thread`s state.
		     */
		    continue;
	    }
	    /*
	     *	Thread is stopped at this point.
	     */
	    break;
	}

	thread_sched_unlock(thread);
	splx(s);

	if (need_wakeup)
	    thread_wakeup(&thread->suspend_wait);

	return ret;
}

void thread_release(
	register thread_t	thread)
{
	spl_t			s;

	s = splsched();
	thread_sched_lock(thread);
	if (--thread->suspend_count == 0) {
		thread->state &= ~(TH_SUSP | TH_HALTED);
		if ((thread->state & (TH_WAIT | TH_RUN)) == 0) {
			/* was only suspended */
			thread->state |= TH_RUN;
			thread_setrun(thread, TRUE);
		}
	}
	thread_sched_unlock(thread);
	splx(s);
}

kern_return_t thread_suspend(
	register thread_t	thread)
{
	register boolean_t	hold;
	spl_t			s;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	hold = FALSE;
	s = splsched();
	thread_sched_lock(thread);
	if (thread->user_stop_count++ == 0) {
		hold = TRUE;
		thread->suspend_count++;
		thread->state |= TH_SUSP;
	}
	thread_sched_unlock(thread);
	splx(s);

	/*
	 *	Now  wait for the thread if necessary.
	 */
	if (hold) {
		if (thread == current_thread()) {
			/*
			 *	We want to call thread_block on our way out,
			 *	to stop running.
			 */
			s = splsched();
			ast_on(cpu_number(), AST_BLOCK);
			splx(s);
		} else
			(void) thread_dowait(thread, TRUE);
	}
	return KERN_SUCCESS;
}


kern_return_t thread_resume(
	register thread_t	thread)
{
	register kern_return_t	ret;
	spl_t			s;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	ret = KERN_SUCCESS;

	s = splsched();
	thread_sched_lock(thread);
	if (thread->user_stop_count > 0) {
	    if (--thread->user_stop_count == 0) {
		if (--thread->suspend_count == 0) {
		    thread->state &= ~(TH_SUSP | TH_HALTED);
		    if ((thread->state & (TH_WAIT | TH_RUN)) == 0) {
			    /* was only suspended */
			    thread->state |= TH_RUN;
			    thread_setrun(thread, TRUE);
		    }
		}
	    }
	}
	else {
		ret = KERN_FAILURE;
	}

	thread_sched_unlock(thread);
	splx(s);

	return ret;
}

/*
 *	Special version of thread_suspend that is only
 *	used by timer expiration.  This is called from
 *	an AST, so it cannot block.
 *
 *	The thread may still be running or in an uninterruptible
 *	wait on exit.  User-level code must determine that the
 *	thread has stopped (by calling thread_abort or thread_get_status).
 */
void thread_suspend_nowait(
	thread_t	thread)
{
	spl_t		s;

	assert(thread != THREAD_NULL);

	/*
	 *	Increment the user-visible suspend count.
	 */
	s = splsched();
	thread_sched_lock(thread);
	if (thread->user_stop_count++ == 0) {
	    /*
	     *	Not suspended yet - do it now.
	     */
	    thread->suspend_count++;
	    thread->state |= TH_SUSP;

	    if (thread == current_thread()) {
		/*
		 *	Stop the thread on the way out of the kernel
		 */
		ast_on(cpu_number(), AST_BLOCK);
	    }
	    else {
		/*
		 *	If the thread is on a run queue, pull it off.
		 */
		switch (thread->state & TH_SCHED_STATE) {
		    case TH_WAIT | TH_SUSP:
			/*
			 *	Already stopped
			 */
			break;

		    case TH_RUN | TH_SUSP:
			if (rem_runq(thread) != RUN_QUEUE_HEAD_NULL) {
			    thread->state &= ~TH_RUN;
			    assert(!thread->suspend_wait);
			    break;
			}
#if	NCPUS > 1
			cause_ast_check(thread->last_processor);
#endif
			/* fall through to ... */

		    default:
			/*
			 *	Running, or in an uninterruptible wait.
			 *	We can`t do anything more, since we
			 *	cannot block.
			 */
			break;
		}
	    }
	}
	thread_sched_unlock(thread);
	splx(s);
}

