/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * $Log:	clock.h,v $
 * Revision 2.2  93/11/17  17:07:22  dbg
 * 	Added check_seconds and clock_time, so that time can be
 * 	read without having to lock the clock.
 * 	[93/06/18            dbg]
 * 
 * 	Added correction_delta, correction_count.
 * 	[93/06/02            dbg]
 * 
 * 	Moved timer element definitions to kern/mach_timer.h.
 * 	[93/04/09            dbg]
 * 
 * 	Moved user-visible definitions to kern/clock.h.
 * 
 * 	Created
 * 	[92/07/12	savage]
 * 
 */
/*
 *	File:	kern/clock.h
 *	Author:
 *		Stefan Savage and David Golub, 1993
 *
 *	This file contains the definition for kernel clock objects.
 *	A clock is a time keeper that increments periodically and
 *	may have an associated queue of timer elements for alarms,
 *	periodic wakeups, and the like.
 */
#ifndef	_KERN_CLOCK_H_
#define	_KERN_CLOCK_H_

#include <mach/time_spec.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <kern/mach_timer.h>
#include <ipc/ipc_port.h>

struct clock_ops;		/* forward */

/*
 *	A clock.
 */
struct mach_clock {
	volatile time_spec_t	time;		/* current time */
	volatile unsigned int	check_seconds;	/* to read time without lock */
	mapped_time_spec_t	*mtime;		/* pointer to mapped time */
	int			resolution;	/* nanoseconds per 'tick' */
	int			skew;		/* difference from desired
						   resolution */
	int			correction_delta;
						/* add to clock every tick */
	int			correction_count;
						/* for this many ticks */
	struct timer_elt_head	head;		/* queue of timers */
	decl_simple_lock_data(,queue_lock)	/* lock for queue */
	int			new_resolution;	/* new resolution requested */
	int			new_skew;	/* skew for new resolution */
	struct clock_ops	*ops;		/* operations on clock */
	struct mach_clock *	next;		/* list of all clocks */
};

typedef struct mach_clock	*mach_clock_t;
typedef struct mach_clock	mach_clock_data_t;

#define CLOCK_NULL		((mach_clock_t)0)
#define D_INFO_CLOCK		2		/* for d_dev_info */

#define clock_queue_head(clock)		(&(clock)->head)
#define clock_queue_lock(clock)		simple_lock(&(clock)->queue_lock)
#define clock_queue_unlock(clock)	simple_unlock(&(clock)->queue_lock)

extern mach_clock_t	convert_device_port_to_clock(ipc_port_t);
extern ipc_port_t	convert_clock_to_device_port(mach_clock_t);

/*
 *	Operations on a clock
 */
struct clock_ops {
	void	(*set_resolution)(mach_clock_t);
					/* set hardware resolution from
					   clock->resolution */
	void	(*set_time)(mach_clock_t, time_spec_t);
					/* set new time */
	void	(*enable_interrupts)(mach_clock_t);
					/* enable interrupts from clock */
};

/*
 *	Initialize a clock
 */
extern void clock_init(mach_clock_t clock, struct clock_ops *ops);

/*
 *	Adjust the timers on a clock if the clock's time is
 *	changed
 */
extern void clock_timer_adjust(mach_clock_t clock, time_spec_t delta);

/*
 *	One of the clocks is the distinguished system time-of-day clock.
 */
extern mach_clock_t		sys_clock;

/*
 *	Normal and system clock interrupt service routines.
 */
extern void
clock_interrupt(
	mach_clock_t	clock);

extern void
sys_clock_interrupt(
	boolean_t	usermode);

/*
 *	Enable system clock interrupts for the current CPU.
 */
extern void
enable_clock_interrupts(void);

/*
 *	void clock_read(time_spec_t& result,
 *			mach_clock_t clock)
 *
 *	Read the clock`s time, without locking the clock,
 *	and produce a consistent time reading.
 *
 *	Results are passed by lvalue.
 */
#define	clock_read(result, clock)				\
	do {							\
	    (result).seconds = (clock)->time.seconds;		\
	    (result).nanoseconds = (clock)->time.nanoseconds;	\
	} while ((result).seconds != (clock)->check_seconds)

/*
 *	Set the mapped time from the current clock reading,
 *	obeying the update protocol (set check_seconds first).
 */
#define	clock_set_mtime(clock) \
    MACRO_BEGIN							\
	mapped_time_spec_t *mtime = (clock)->mtime;		\
	if (mtime != 0) {					\
	    mtime->check_seconds = (clock)->time.seconds;	\
	    mtime->nanoseconds   = (clock)->time.nanoseconds;	\
	    mtime->seconds	 = (clock)->time.seconds;	\
	}							\
    MACRO_END

#endif	/* _KERN_CLOCK_H_ */
