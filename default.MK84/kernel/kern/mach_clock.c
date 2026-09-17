/* 
 * Mach Operating System
 * Copyright (c) 1993-1988 Carnegie Mellon University
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
 * $Log:	mach_clock.c,v $
 * Revision 2.27  93/11/17  17:14:19  dbg
 * 	Maintain clock->check_seconds to allow reading time without
 * 	locking clock.  Eliminate old 'time' variable.
 * 	[93/06/18            dbg]
 * 
 * 	Just use division to update old mapped-time... pay the
 * 	consequences later...
 * 	[93/06/07            dbg]
 * 
 * 	Added correction_delta, correction_count.  Moved some common
 * 	routines to device/clock_dev.c.
 * 	[93/06/02            dbg]
 * 
 * 	Change mmap routines to return physical address instead of
 * 	physical page number.
 * 	[93/05/24            dbg]
 * 
 * 	Changes for new clocks and timers:
 * 	. Accommodate multiple clocks.
 * 	. Time is now measured in seconds/nanoseconds.
 * 	. Timeout routines are run from ASTs, not softclock
 * 	  interrupts.
 * 	. Use per-scheduling-policy routine to update thread`s
 * 	  usage and scheduling parameters at clock tick.
 * 	Added ANSI function prototypes.
 * 	[93/05/21            dbg]
 * 
 * Revision 2.26  93/08/03  12:31:12  mrt
 * 	Flavor support for sampling.
 * 	[93/07/30  10:21:52  bershad]
 * 
 * Revision 2.25  93/05/15  18:53:40  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.24  93/05/10  17:47:45  rvb
 * 	Rudy asked for this change for xntp and dbg thought it would
 * 	do no harm.  (I think that HZ is pretty always 100 so that this
 * 	code and the previous version always go the tickadj = 1; route.)
 * 	[93/05/10  15:52:26  rvb]
 * 
 * Revision 2.23  93/03/09  10:55:07  danner
 * 	Removed gratuitous casts to ints.
 * 	[93/03/05            af]
 * 
 * Revision 2.22  93/01/27  09:33:55  danner
 * 	take_pc_sample() is void.
 * 	[93/01/25            jfriedl]
 * 
 * Revision 2.21  93/01/24  13:19:29  danner
 * 	Add pc sampling from C Maeda.  Make it conditional on thread or
 * 	task sampling being enabled.
 * 	[93/01/12            rvb]
 * 
 * Revision 2.20  93/01/14  17:35:12  danner
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.19  92/08/03  17:38:09  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.18  92/05/21  17:14:33  jfriedl
 * 	Added void to fcns that yet needed it.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.17  92/03/10  16:26:41  jsb
 * 	Removed NORMA_IPC code.
 * 	[92/01/17  11:38:55  jsb]
 * 
 * Revision 2.16  91/08/03  18:18:56  jsb
 * 	NORMA_IPC: added call to netipc_timeout in hardclock.
 * 	[91/07/24  22:30:22  jsb]
 * 
 * Revision 2.15  91/07/31  17:45:57  dbg
 * 	Fixed timeout race.  Implemented host_adjust_time.
 * 	[91/07/30  17:03:54  dbg]
 * 
 * Revision 2.14  91/05/18  14:32:29  rpd
 * 	Fixed timeout/untimeout to use a fixed-size array of timers
 * 	instead of a zone.
 * 	[91/03/31            rpd]
 * 	Fixed host_set_time to update the mapped time value.
 * 	Changed the mapped time value to include a check field.
 * 	[91/03/19            rpd]
 * 
 * Revision 2.13  91/05/14  16:44:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.12  91/03/16  14:50:45  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 	Use counter macros to track thread and stack usage.
 * 	[91/03/01  17:43:15  rpd]
 * 
 * Revision 2.11  91/02/05  17:27:45  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:14:47  mrt]
 * 
 * Revision 2.10  91/01/08  15:16:22  rpd
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * Revision 2.9  90/11/05  14:31:27  rpd
 * 	Unified untimeout and untimeout_try.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.8  90/10/12  18:07:29  rpd
 * 	Fixed calls to thread_bind in host_set_time.
 * 	Fix from Philippe Bernadat.
 * 	[90/10/10            rpd]
 * 
 * Revision 2.7  90/09/09  14:32:18  rpd
 * 	Use decl_simple_lock_data.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.6  90/08/27  22:02:48  dbg
 * 	Add untimeout_try for multiprocessors.  Reduce lint.
 * 	[90/07/17            dbg]
 * 
 * Revision 2.5  90/06/02  14:55:04  rpd
 * 	Converted to new IPC and new host port technology.
 * 	[90/03/26  22:10:04  rpd]
 * 
 * Revision 2.4  90/01/11  11:43:31  dbg
 * 	Switch to master CPU in host_set_time.
 * 	[90/01/03            dbg]
 * 
 * Revision 2.3  89/08/09  14:33:09  rwd
 * 	Include mach/vm_param.h and use PAGE_SIZE instead of NBPG.
 * 	[89/08/08            rwd]
 * 	Removed timemmap to machine/model_dep.c
 * 	[89/08/08            rwd]
 * 
 * Revision 2.2  89/08/05  16:07:11  rwd
 * 	Added mappable time code.
 * 	[89/08/02            rwd]
 * 
 * 14-Jan-89  David Golub (dbg) at Carnegie-Mellon University
 *	Split into two new files: mach_clock (for timing) and priority
 *	(for priority calculation).
 *
 *  8-Dec-88  David Golub (dbg) at Carnegie-Mellon University
 *	Use sentinel for root of timer queue, to speed up search loops.
 *
 * 30-Jun-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */ 
/*
 *	File:	clock_prim.c
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1986
 *
 *	Clock primitives.
 */
#include <cpus.h>
#include <mach_pcsample.h>
#include <stat_time.h>

#include <mach/boolean.h>
#include <mach/time_value.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>

#include <kern/clock.h>
#include <kern/counters.h>
#include <kern/cpu_number.h>
#include <kern/host.h>
#include <kern/lock.h>
#include <kern/mach_param.h>
#include <kern/mach_timer.h>
#include <kern/memory.h>
#include <kern/processor.h>
#include <kern/quantum.h>
#include <kern/sched_policy.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>

#include <vm/vm_kern.h>
#include <vm/pmap.h>

#include <device/clock_dev.h>
#include <device/clock_status.h>

#include <machine/mach_param.h>	/* HZ */
#include <machine/machspl.h>

#if	MACH_PCSAMPLE
#include <kern/pc_sample.h>
#endif

/*
 * For debugging, count the maximum number of clock ticks skipped
 * between two calls to timer_ast.
 */
#define	TIMER_DEBUG	0

#ifdef	TIMER_DEBUG
time_spec_t	timer_ast_requested_time = { 0, 0 };
time_spec_t	timer_ast_max_time_missed = { 0, 0 };
time_spec_t	timer_ast_total_time_missed = { 0, 0 };
unsigned long	timer_ast_calls = 0;
#endif

/*
 *	Generalized clock support.
 */

mach_clock_t	clock_list = 0;			/* all clocks in system */

/*
 *	Clock queue maintainance.
 */
/*
 *	Insert a timer element on its clock queue, in the
 *	proper place.
 */
#define	timer_elt_insque(head, telt)				\
    MACRO_BEGIN							\
	timer_elt_t	next;					\
								\
	for (next = (timer_elt_t) queue_first(&(head)->chain);	\
	     ;							\
	     next = (timer_elt_t) queue_next(&next->te_chain))	\
	{							\
	    if (!time_spec_leq(next->te_expire_time,		\
			      (telt)->te_expire_time))		\
		break;						\
	}							\
								\
	enqueue_tail_macro((queue_t) next, (queue_entry_t)(telt)); \
    MACRO_END

/*
 *	Remove a timer element from its clock queue.
 */
#define	timer_elt_remque(head, telt)				\
    MACRO_BEGIN							\
	remqueue_macro(&(head)->chain, (queue_entry_t) telt);	\
    MACRO_END

/*
 *	Add a timer element to its clock queue.
 *	if absolute, time is absolute expiration time;
 *	otherwise, time is interval from clock time.
 *	Returns FALSE if expiration time is in the past.
 */
boolean_t timer_elt_enqueue(
	timer_elt_t	elt,
	time_spec_t	time,
	boolean_t	absolute)
{
	mach_clock_t	clock;
	spl_t		s;

	s = splsched();

	clock = elt->te_clock;
	clock_queue_lock(clock);	/* locks time value */

	if (absolute) {
	    elt->te_expire_time = time;
	    elt->te_flags |= TELT_ABSOLUTE;
	}
	else {
	    elt->te_expire_time = clock->time;
	    time_spec_add(elt->te_expire_time, time);
	}
	if (time_spec_leq(elt->te_expire_time, clock->time)) {
	    clock_queue_unlock(clock);
	    splx(s);
	    return FALSE;
	}

	timer_elt_insque(&clock->head, elt);
	elt->te_flags |= TELT_SET;

	clock_queue_unlock(clock);
	splx(s);
	return TRUE;
}

/*
 *	Remove a timer element from its clock queue.
 *	Return TRUE if the timer element was on the queue.
 */
boolean_t timer_elt_dequeue(
	timer_elt_t	telt)
{
	mach_clock_t	clock;
	spl_t		s;

	s = splsched();

	clock = telt->te_clock;
	clock_queue_lock(clock);

	if (telt->te_flags & TELT_SET) {
	    timer_elt_remque(&clock->head, (queue_entry_t)telt);
	    telt->te_flags &= ~(TELT_SET | TELT_ALLOC);
	    clock_queue_unlock(clock);
	    splx(s);
	    return TRUE;
	}
	else {
	    clock_queue_unlock(clock);
	    splx(s);
	    return FALSE;
	}
}

/*
 * If a clock`s time is changed, adjust the time of all relative
 * timers on that clock.
 *
 * Called with the clock locked.
 */
void clock_timer_adjust(
	mach_clock_t	clock,
	time_spec_t	delta)
{
	timer_elt_t	elt;

	queue_iterate(&clock->head.chain, elt, timer_elt_t, te_chain) {
	    if ((elt->te_flags & TELT_ABSOLUTE) == 0) {
		time_spec_add(elt->te_expire_time, delta);
	    }
	}
}

/*
 * Service timers on one clock.
 */
void timer_service(
	mach_clock_t	clock)
{
	void		(*funct)(void *);
	void		*param;
	timer_elt_t	telt;
	spl_t		s;

	/*
	 * Lock the clock queue.
	 */
	s = splsched();
	clock_queue_lock(clock);

	/*
	 * Handle all of the timer elements that have expired.
	 */
	while (telt = (timer_elt_t) queue_first(&clock->head.chain),
	       time_spec_leq(telt->te_expire_time, clock->time))
	{
	    /*
	     * Remove expired element from the queue.
	     * If it is periodic, increment its expiration
	     * time and re-add it.  Otherwise, mark it
	     * as not on the clock queue.
	     */
	    timer_elt_remque(&clock->head, (queue_entry_t)telt);

	    if (telt->te_flags & TELT_PERIODIC) {
		time_spec_add(telt->te_expire_time, telt->te_period);
		timer_elt_insque(&clock->head, telt);
	    }
	    else {
		telt->te_flags = TELT_UNSET;
	    }

	    /*
	     * Drop lock and interrupt protection around
	     * call to timeout routine.  Save the function
	     * and parameter; the timer element is not
	     * accessible once the lock is dropped.
	     */
	    funct = telt->te_fcn;
	    param = telt->te_param;

	    clock_queue_unlock(clock);
	    splx(s);

	    (*funct)(param);

	    s = splsched();
	    clock_queue_lock(clock);
	}

	clock_queue_unlock(clock);
	splx(s);
}

/*
 *	Set a new period for the clock.
 *	Called with clock locked, at splsched.
 */
void set_new_clock_period(
	mach_clock_t	clock)
{
	clock->resolution = clock->new_resolution;
	clock->skew	  = clock->new_skew;

	clock->new_resolution = 0;
	clock->skew = 0;

	/*
	 *	Set the hardware clock.
	 */
	(*clock->ops->set_resolution)(clock);
}

/*
 *	Initialize a clock.
 */
void clock_init(
	mach_clock_t	clock,
	struct clock_ops *ops)
{
	clock->time.seconds	= 0;
	clock->time.nanoseconds	= 0;
	clock->check_seconds	= 0;
	clock->resolution	= 0;
	clock->skew		= 0;
	clock->correction_delta = 0;
	clock->correction_count = 0;
	queue_init(&clock->head.chain);
	time_spec_set_infinite(clock->head.expire_time);
				/* sentinel at end of timer list */
	simple_lock_init(&clock->queue_lock);
	clock->new_resolution	= 0;
	clock->new_skew		= 0;
	clock->mtime		= 0;
	clock->ops		= ops;

	/*
	 *	Add the clock to the list.
	 *	No locking - we find all clocks at system startup,
	 *	and never remove them.
	 */
	clock->next = clock_list;
	clock_list = clock;
}

/*
 *	Clock interrupt for standard clock (not system clock).
 *	Update the clock time, and check for pending timeouts.
 */
void clock_interrupt(
	mach_clock_t	clock)
{
	spl_t	s;

	s = splsched();
	clock_queue_lock(clock);

	/*
	 *	If seconds do not change, only update nanoseconds.
	 *	Otherwise, update check_seconds first so that
	 *	clock can be read without locking.
	 */
    {
	register unsigned int	nsec, sec;

	nsec = clock->time.nanoseconds +
			(clock->resolution + clock->correction_delta);
	if (nsec < NANOSEC_PER_SEC) {
	    clock->time.nanoseconds = nsec;
	}
	else {
	    nsec -= NANOSEC_PER_SEC;
	    sec   = clock->time.seconds + 1;

	    clock->check_seconds = sec;
	    clock->time.nanoseconds = nsec;
	    clock->time.seconds = sec;
	}
	if (clock->correction_count != 0) {
	    if (--clock->correction_count == 0)
		clock->correction_delta = 0;
	}
    }

	/*
	 *	Request a timer AST if timeouts are pending.
	 */
    {
	register timer_elt_t	telt;

	telt = (timer_elt_t)queue_first(&clock->head.chain);
	if (time_spec_leq(telt->te_expire_time, clock->time)) {
	    int		my_cpu = cpu_number();
	    ast_on(my_cpu, AST_TIMER);
#ifdef	TIMER_DEBUG
	    timer_ast_requested_time = sys_clock->time;
#endif
	}
    }

	/*
	 *	Update mapped time.
	 */
	clock_set_mtime(clock);

	/*
	 *	Change the clock resolution here if
	 *	a change was requested.
	 */
	if (clock->new_resolution != 0)
	    set_new_clock_period(clock);

	clock_queue_unlock(clock);
	splx(s);
}

/*
 *	Run timeout code.
 *	Called from AST level.
 */
void timer_ast(void)
{
	/*
	 *	Handle timeouts.
	 */
	mach_clock_t	clock;

#ifdef	TIMER_DEBUG
	/*
	 * Find the maximum number of ticks lost.
	 */
	{
	    spl_t	s;
	    time_spec_t	diff;

	    s = splsched();
	    diff = sys_clock->time;
	    time_spec_subtract(diff, timer_ast_requested_time);
	    if (!time_spec_leq(diff, timer_ast_max_time_missed))
		timer_ast_max_time_missed = diff;
	    timer_ast_calls++;
	    time_spec_add(timer_ast_total_time_missed, diff);
	    splx(s);
	}
#endif
	
	for (clock = clock_list; clock; clock = clock->next)
	    timer_service(clock);
}


/*
 ****************************************************************
 *								*
 *	System clock						*
 *								*
 ****************************************************************
 */
mach_clock_t	sys_clock = 0;		/* machine-dependent code finds it */

/*
 *	This update protocol, with a check value, allows
 *		do {
 *			secs = mtime->seconds;
 *			usecs = mtime->microseconds;
 *		} while (secs != mtime->check_seconds);
 *	to read the time correctly.  (On a multiprocessor this assumes
 *	that processors see each other's writes in the correct order.
 *	We may have to insert fence operations.)
 */

mapped_time_value_t *mtime = 0;

#define update_mapped_time(clock)					\
MACRO_BEGIN								\
	if (mtime != 0) {						\
	    time_spec_t	cur_time;					\
	    clock_read(cur_time, (clock));				\
	    mtime->check_seconds = cur_time.seconds;			\
	    mtime->microseconds = cur_time.nanoseconds / 1000;		\
	    mtime->seconds = cur_time.seconds;				\
	}								\
MACRO_END


/*
 *	Handle clock interrupt for system clock.
 *
 *	This clock maintains the thread run times (if
 *	statistical timing is in use) and the quantum
 *	for the currently running thread, as well as
 *	the seconds/microseconds clock used by old code.
 *	It also runs the system timeout list.
 *
 *	If there are multiple CPUS, this interrupt handler
 *	must be called on each CPU at the same clock rate.
 */

void sys_clock_interrupt(
	boolean_t	usermode)	/* executing user code */
{
	register int		my_cpu = cpu_number();
	register thread_t	thread = current_thread();

	counter(c_clock_ticks++);
	counter(c_threads_total += c_threads_current);
	counter(c_stacks_total += c_stacks_current);

#if	STAT_TIME
	/*
	 *	Increment the thread time, if using
	 *	statistical timing.
	 */
	if (usermode) {
	    timer_bump(&thread->user_timer, sys_clock->resolution);
	}
	else {
	    timer_bump(&thread->system_timer, sys_clock->resolution);
	}
#endif	/* STAT_TIME */

	/*
	 *	Adjust the thread`s priority and check for
	 *	quantum expiration.
	 */

	/*
	 *	We assume that the clock interrupts no more
	 *	frequently than 1/microsecond...
	 */

	clock_quantum_update(thread, my_cpu, sys_clock->resolution >> 10);

#if	MACH_PCSAMPLE > 0
	/*
	 * Take a sample of pc for the user if required.
	 * This had better be MP safe.  It might be interesting
	 * to keep track of cpu in the sample.
	 */
	if (usermode)
	    take_pc_sample_macro(thread, SAMPLED_PC_PERIODIC);

#endif	/* MACH_PCSAMPLE > 0 */

	/*
	 *	Time-of-day and time-out list are updated only
	 *	on the master CPU.
	 */
	if (my_cpu != master_cpu) {
	    return;
	}

	/*
	 *	Update the time and handle timeouts.
	 */
	clock_interrupt(sys_clock);

	/*
	 *	Set the old time-of-day clocks
	 *	(seconds/microseconds)
	 */

	update_mapped_time(sys_clock);
}

/*
 *	Allow clock interrupts on the current CPU.
 */
void enable_clock_interrupts(void)
{
	(*sys_clock->ops->enable_interrupts)(sys_clock);
}

/*
 * Read the time.
 */
kern_return_t
host_get_time(
	host_t		host,
	time_value_t	*current_time)	/* OUT */
{
	time_spec_t temp;

	if (host == HOST_NULL)
	    return KERN_INVALID_HOST;

	clock_read(temp, sys_clock);

	current_time->seconds = temp.seconds;
	current_time->microseconds = temp.nanoseconds / 1000;

	return KERN_SUCCESS;
}

/*
 * Set the time.  Only available to privileged users.
 */
kern_return_t
host_set_time(
	host_t		host,
	time_value_t	new_time)
{
	time_spec_t	temp;

	if (host == HOST_NULL)
	    return KERN_INVALID_HOST;

	temp.seconds = new_time.seconds;
	temp.nanoseconds = new_time.microseconds * 1000;

	(void) clock_setstat(sys_clock,
			     CLOCK_TIME,
			     (dev_status_t) &temp,
			     CLOCK_TIME_COUNT);

	return KERN_SUCCESS;
}

/*
 *	Adjust-time parameters
 */

#if	HZ > 500
int		tickadj = 1;		/* can adjust HZ usecs per second */
#else
int		tickadj = 500 / HZ;	/* can adjust 500 usecs per second */
#endif
int		bigadj = 1000000;	/* adjust 10*tickadj if adjustment
					   > bigadj */

/*
 * Adjust the time gradually.
 */
kern_return_t
host_adjust_time(
	host_t		host,
	time_value_t	new_adjustment,
	time_value_t	*old_adjustment)	/* OUT */
{
	int		ndelta, odelta, tickdelta;
	clock_correction_data_t	old_correction, new_correction;
	clock_resolution_data_t sys_clock_resolution;
	natural_t	count;

	if (host == HOST_NULL)
	    return KERN_INVALID_HOST;

	count = CLOCK_RESOLUTION_COUNT;
	(void) clock_getstat(sys_clock,
			     CLOCK_RESOLUTION,
			     (dev_status_t) &sys_clock_resolution,
			     &count);

	ndelta = new_adjustment.seconds * 1000000
		+ new_adjustment.microseconds;

	if (ndelta > bigadj)
	    tickdelta = 10 * tickadj;
	else
	    tickdelta = tickadj;

	new_correction.delta = tickdelta * 1000;	/* usec -> nsec */
	new_correction.count = ndelta / sys_clock_resolution.resolution;

	count = CLOCK_CORRECTION_COUNT;
	(void) clock_getstat(sys_clock,
			     CLOCK_CORRECTION,
			     (dev_status_t) &old_correction,
			     &count);
	count = CLOCK_CORRECTION_COUNT;
	(void) clock_setstat(sys_clock,
			     CLOCK_CORRECTION,
			     (dev_status_t) &new_correction,
			     count);

	odelta = (old_correction.delta / 1000) * old_correction.count;
			/* nanos -> micros */
	old_adjustment->seconds = odelta / 1000000;
	old_adjustment->microseconds = odelta % 1000000;

	return KERN_SUCCESS;
}

int timeopen()
{
	kern_return_t	kr;
	vm_offset_t	temp;
	kr = kmem_alloc_wired(kernel_map,
			     (vm_offset_t *) &temp,
			     PAGE_SIZE);
	if (kr != KERN_SUCCESS)
	    return kr;
	bzero((void *) temp, PAGE_SIZE);
	mtime = (mapped_time_value_t *) temp;
	update_mapped_time(sys_clock);

	return 0;
}

int timeclose()
{
	vm_offset_t	temp;

	temp = (vm_offset_t) mtime;
	mtime = 0;

	(void) kmem_free(kernel_map,
			 (vm_offset_t) temp,
			 PAGE_SIZE);
	return 0;
}

vm_offset_t timemmap(
	int	dev,
	vm_offset_t offset,
	vm_prot_t prot)
{
	if ((prot & VM_PROT_WRITE) != 0 || offset != 0)
	    return -1;		/* not valid */

	return pmap_extract(pmap_kernel(), (vm_offset_t) mtime);
}

/*
 *	Compatibility timeouts for device drivers.
 *	New code should use set_timeout/reset_timeout and private timers.
 *	These code can't use a zone to allocate timers, because
 *	it can be called from interrupt handlers.
 */

#define NTIMERS		20

timer_elt_data_t timeout_timers[NTIMERS];

int		hz = HZ;		/* number of 'ticks' per second -
					   only used for setting timeout
					   interval for timeout() */

/*
 *	Set timeout.
 *
 *	fcn:		function to call
 *	param:		parameter to pass to function
 *	interval:	timeout interval, in hz.
 */
void timeout(
	void	(*fcn)(void * param),
	void *	param,
	int	interval)
{
	spl_t	s;
	register timer_elt_t elt;
	time_spec_t	expire_time;

	s = splsched();
	clock_queue_lock(sys_clock);
	for (elt = &timeout_timers[0]; elt < &timeout_timers[NTIMERS]; elt++)
	    if (elt->te_flags == TELT_UNSET)
		break;
	if (elt == &timeout_timers[NTIMERS])
	    panic("timeout");

	elt->te_fcn   = fcn;
	elt->te_param = param;
	elt->te_flags = TELT_ALLOC;
	elt->te_clock = sys_clock;
	clock_queue_unlock(sys_clock);
	splx(s);

	expire_time.seconds = interval / hz;
	expire_time.nanoseconds =
			 (interval % hz) * (NANOSEC_PER_SEC / hz);

	(void) timer_elt_enqueue(elt, expire_time, FALSE);
			/* relative to clock */
}

/*
 * Returns a boolean indicating whether the timeout element was found
 * and removed.
 */
boolean_t untimeout(
	register void	(*fcn)(void *),
	register void *	param)
{
	spl_t	s;
	register timer_elt_t elt;

	s = splsched();
	clock_queue_lock(sys_clock);
	queue_iterate(&sys_clock->head.chain, elt, timer_elt_t, te_chain) {

	    if (fcn == elt->te_fcn && param == elt->te_param) {
		/*
		 *	Found it.
		 */
		remqueue(&sys_clock->head.chain, (queue_entry_t)elt);
		elt->te_flags = TELT_UNSET;

		clock_queue_unlock(sys_clock);
		splx(s);
		return TRUE;
	    }
	}
	clock_queue_unlock(sys_clock);
	splx(s);
	return FALSE;
}
