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
 * $Log:	eventcount.c,v $
 * Revision 2.13  93/11/17  17:08:47  dbg
 * 	Changed evc_notify_abort to evc_wait_interrupt, since it is only
 * 	used by thread_halt (to 'interrupt' an evc_wait).  It knows
 * 	about the evc_wait continuation.
 * 	[93/08/20            dbg]
 * 
 * 	Break up thread lock.  Use thread_will_wait and thread_go
 * 	to change thread state.
 * 	[93/05/26            dbg]
 * 
 * 	Declare contination functions as returning 'no_return'.
 * 
 * 	Added return code to evc_init.  Fixed locking in evc_signal.
 * 
 * 	Removed include of kern/sched.h.  Added ANSI prototypes.
 * 	Removed simpler_thread_setrun: we'll fix thread_setrun
 * 	instead.
 * 	[93/05/21            dbg]
 * 
 * Revision 2.12  93/08/10  15:11:22  mrt
 * 	Added evc_wait_clear.  Always clears count before blocking.
 * 	[93/03/22            cmaeda]
 * 
 * Revision 2.11  93/05/15  18:54:40  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.10  93/02/01  09:50:07  danner
 * 	Remove evc_signal panic for suspension case.
 * 	[93/01/28            danner]
 * 
 * Revision 2.9  93/01/24  13:18:45  danner
 * 	rename notify routines; prototypes.
 * 	[93/01/22            danner]
 * 
 * Revision 2.8  93/01/21  12:21:49  danner
 * 	Added evc_notify_thread_destroy_bkpt and
 * 	evc_notify_thread_destroy.
 * 	[93/01/20            bershad]
 * 
 * Revision 2.7  93/01/14  17:33:59  danner
 * 	Typecast assert_wait arguments.
 * 	[93/01/12            danner]
 * 	Fixes for multiprocessor usage.
 * 	[92/11/16            jfriedl]
 * 	Fixed two-for-one bug in evc_wait, per jcb's report.
 * 	See comments in code.
 * 	[92/12/15            af]
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.6  92/08/03  17:36:55  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.5  92/05/21  17:13:12  jfriedl
 * 	Added void type to functions that needed it.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.4  92/01/03  20:40:02  dbg
 * 	Made user-safe with a small translation table.
 * 	This all will be refined at a later time.
 * 	[91/12/27            af]
 * 
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
#include <kern/memory.h>
#include <kern/processor.h>
#include <kern/queue.h>
#include <kern/sched_policy.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <machine/machspl.h>	/* For def'n of splsched() */

#include <kern/eventcount.h>

#define	MAX_EVCS	10		/* xxx for now */
evc_t	all_eventcounters[MAX_EVCS];

/*
 * Initialization
 */
kern_return_t
evc_init(evc_t	ev)
{
	int i;

	bzero(ev, sizeof(*ev));

	/* keep track of who is who */
	for (i = 0; i < MAX_EVCS; i++)
		if (all_eventcounters[i] == 0) break;
	if (i == MAX_EVCS) {
#if 0
		printf("Too many eventcounters\n");
#endif
		return KERN_RESOURCE_SHORTAGE;
	}

	all_eventcounters[i] = ev;
	ev->ev_id = i;
	ev->sanity = ev;
	ev->waiting_thread = THREAD_NULL;
	simple_lock_init(&ev->lock);

	return KERN_SUCCESS;
}

/*
 * Finalization
 */
void
evc_destroy(evc_t	ev)
{
	evc_signal(ev);
	ev->sanity = 0;
	if (all_eventcounters[ev->ev_id] == ev)
		all_eventcounters[ev->ev_id] = 0;
	ev->ev_id = -1;
}

/*
 * Just so that we return success, and give
 * up the stack while blocked
 */
no_return
evc_continue(void)
{
	thread_syscall_return(KERN_SUCCESS);
	/* NOTREACHED */
}

/*
 * User-trappable
 */
kern_return_t evc_wait(natural_t ev_id)
{
	spl_t		s;
	kern_return_t	ret;
	evc_t		ev;

	if ((ev_id >= MAX_EVCS) ||
	    ((ev = all_eventcounters[ev_id]) == 0) ||
	    (ev->ev_id != ev_id) || (ev->sanity != ev))
		return KERN_INVALID_ARGUMENT;

	s = splsched();
	simple_lock(&ev->lock);
		/*
		 * The values assumed by the "count" field are
		 * as follows:
		 *	0	At initialization time, and with no
		 *		waiting thread means no events pending;
		 *		with waiting thread means the event
		 *		was signalled and the thread not yet resumed
		 *	-1	no events, there must be a waiting thread
		 *	N>0	no waiting thread means N pending,
		 *		with waiting thread N-1 pending.
		 *	
		 */
		if (ev->count > 0) {
			ev->count--;
			ret = KERN_SUCCESS;
		} else {
			if (ev->waiting_thread == THREAD_NULL) {
				thread_t thread = current_thread();
				ev->count--;
				ev->waiting_thread = thread;
				thread_will_wait(thread);
				simple_unlock(&ev->lock);
				thread_block(evc_continue);
				/* NOTREACHED */
			}
			ret = KERN_NO_SPACE; /* XX */
		}
	simple_unlock(&ev->lock);
	splx(s);
	return ret;
}

/*
 * User-trappable
 */
kern_return_t evc_wait_clear(natural_t ev_id)
{
	spl_t		s;
	evc_t		ev;

	if ((ev_id >= MAX_EVCS) ||
	    ((ev = all_eventcounters[ev_id]) == 0) ||
	    (ev->ev_id != ev_id) || (ev->sanity != ev))
		return KERN_INVALID_ARGUMENT;

	s = splsched();
	simple_lock(&ev->lock);

		/*
		 * The values assumed by the "count" field are
		 * as follows:
		 *	0	At initialization time, and with no
		 *		waiting thread means no events pending;
		 *		with waiting thread means the event
		 *		was signalled and the thread not yet resumed
		 *	-1	no events, there must be a waiting thread
		 *	N>0	no waiting thread means N pending,
		 *		with waiting thread N-1 pending.
		 *	
		 */
	        /*
		 *  Note that we always clear count before blocking.
		 */
		if (ev->waiting_thread == THREAD_NULL) {
			thread_t thread = current_thread();
			ev->count = -1;
			ev->waiting_thread = thread;
			thread_will_wait(thread);
			simple_unlock(&ev->lock);
			thread_block(evc_continue);
			/* NOTREACHED */
		}

	simple_unlock(&ev->lock);
	splx(s);
	return KERN_NO_SPACE; /* XX */
}

/*
 * Called exclusively from interrupt context
 */
void
evc_signal(evc_t ev)
{
    thread_t thread;
    spl_t    s;

    if (ev->sanity != ev)
      return;

    s = splsched();
    simple_lock(&ev->lock);
    ev->count++;
    thread = ev->waiting_thread;
    if (thread != THREAD_NULL)
    {
	ev->waiting_thread = 0;
	thread_go(thread);
    }

    simple_unlock(&ev->lock);
    splx(s);
}

/*
 * Interrupt an evc_wait.  Called by thread_abort.
 * Returns TRUE if the thread was waiting.
 *
 * Thread is suspended on entry.
 *
 * (This stuff needs to be fixed.)
 */
boolean_t evc_wait_interrupt(thread_t thread)
{
    int i;
    evc_t ev;
    spl_t s;

    if (thread->swap_func != evc_continue)
	return FALSE;

    s = splsched();
    for (i = 0; i < MAX_EVCS; i++)  {
	ev = all_eventcounters[i];
	if (ev)  {
	    simple_lock(&ev->lock);
	    if (ev->waiting_thread == thread)  
	      {
		ev->waiting_thread = 0;
		/* Removal of a waiting thread has to bump the count by one */
		ev->count++;
	      }
	    simple_unlock(&ev->lock);
	}
    }
    splx(s);

    thread->swap_func = thread_exception_return;
    thread_set_syscall_return(thread, KERN_ABORTED);
    return TRUE;
}

