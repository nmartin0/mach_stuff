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
 * $Log:	time_spec.h,v $
 * Revision 2.2  93/11/17  17:48:14  dbg
 * 	Added time_spec_infinite, time_spec_set_infinite.
 * 	[93/05/12            dbg]
 * 
 * 	Added mapped_time_spec_read.
 * 	[93/03/26            dbg]
 * 
 * 	64 bit cleanup.
 * 	[93/02/19            dbg]
 * 
 * 	Merged into microkernel mainline.
 * 
 * 	Created
 * 	[92/07/02	savage]
 * 
 */

/*
 *	Type definitions to measure time in seconds and nanoseconds.
 *
 *	Authors:	Stefan Savage and David Golub, 1992
 */

#ifndef _MACH_TIME_SPEC_H_
#define _MACH_TIME_SPEC_H_

#include <mach/machine/vm_types.h>

/*
 *	Time values are represented as seconds and nanoseconds.
 */
struct time_spec {
	natural_t	seconds;
	natural_t	nanoseconds;
};
typedef struct time_spec		time_spec_t;

/*
 *	Time value available by mapping time.
 *	This contains a check field to allow
 *	asynchronous updates.
 */
struct mapped_time_spec {
	volatile natural_t	seconds;
	volatile natural_t	nanoseconds;
	volatile natural_t	check_seconds;
};
typedef struct mapped_time_spec		mapped_time_spec_t;

/*
 *	void mapped_time_spec_read(time_spec_t&		result,
 *				   mapped_time_spec_t *	mtime)
 *
 *	Read the mapped time_spec value and produce a
 *	consistent time reading.
 *
 *	Results are passed by lvalue.
 */
#define	mapped_time_spec_read(result, mtime) \
	do { \
		(result).seconds = (mtime)->seconds; \
		(result).nanoseconds = (mtime)->nanoseconds; \
	} while ((result).seconds != (mtime)->check_seconds)

/*
 *	Macros to manipulate time values.
 *	Assume that all time values are normalized
 *	(nanoseconds <= 999,999,999).
 *
 *	Results are passed by lvalue.
 */
#define NANOSEC_PER_SEC		(1000000000)

/*
 *	void time_spec_add_nsec(time_spec_t& result,
 *				unsigned int nanosec)
 *
 *	Add number of nanoseconds to a time value.
 */
#define	time_spec_add_nsec(result, nanosec)				\
    (									\
      (void)(								\
	(((result).nanoseconds += (nanosec)) >= NANOSEC_PER_SEC)	\
	&& (								\
	    (result).nanoseconds -= NANOSEC_PER_SEC,			\
	    (result).seconds++						\
	)								\
      )									\
    )

/*
 *	void time_spec_add(time_spec_t& result,
 *			   time_spec_t  addend)
 *
 *	Add addend to result.
 */
#define time_spec_add(result, addend)					\
    (									\
      (void)(								\
	(result).seconds += (addend).seconds,				\
	(((result).nanoseconds += (addend).nanoseconds)			\
	 >= NANOSEC_PER_SEC)						\
	&& (								\
	    (result).nanoseconds -= NANOSEC_PER_SEC,			\
	    (result).seconds++						\
	)								\
      )									\
    )

/*
 *	void time_spec_subtract(time_spec_t& result,
 *				time_spec_t  minuend)
 *
 *	Subtract minuend from result.
 */
#define time_spec_subtract(result, minuend)				\
    (									\
      (void)(								\
	/*								\
	 *	Use comparison - fields are unsigned.			\
	 */								\
	(((result).nanoseconds < (minuend).nanoseconds)			\
	&& (								\
	    (result).nanoseconds += NANOSEC_PER_SEC,			\
	    (result).seconds--						\
	)),								\
	(result).nanoseconds -= (minuend).nanoseconds,			\
	(result).seconds -= (minuend).seconds				\
      )									\
    )

/*
 *	void time_spec_set(time_spec_t& time,
 *			   time_spec_t  newtime)
 *
 *	Set time to newtime.
 */
#define time_spec_set(time, newtime)					\
    (									\
	(time).seconds = (newtime).seconds,				\
	(time).nanoseconds = (newtime).nanoseconds			\
    )

/*
 *	boolean_t time_spec_leq(time_spec_t time1,
 *				time_spec_t time2)
 *
 *	Return TRUE if time1 <= time2.
 */
#define time_spec_leq(time1, time2)					\
	((time1).seconds < (time2).seconds ||				\
	    ((time1).seconds == (time2).seconds &&			\
	     (time1).nanoseconds <= (time2).nanoseconds))

/*
 *	boolean_t time_spec_lt(time_spec_t time1,
 *			       time_spec_t time2)
 *
 *	Return TRUE if time1 < time2.
 */
#define time_spec_lt(time1, time2)					\
	((time1).seconds < (time2).seconds ||				\
	    ((time1).seconds == (time2).seconds &&			\
	     (time1).nanoseconds < (time2).nanoseconds))

/*
 *	void time_spec_set_infinite(time_spec_t& time)
 *
 *	Set time to an "infinite" value.  Infinity
 *	is used to denote no time for periodic timers.
 */
#define time_spec_set_infinite(time)					\
    (									\
	(time).seconds = ~0,						\
	(time).nanoseconds = NANOSEC_PER_SEC - 1			\
    )

/*
 *	boolean_t time_spec_infinite(time_spec_t time)
 *
 *	Return TRUE if time_spec is "infinite".
 */
#define	time_spec_infinite(time)					\
	((time).seconds == ~0 &&					\
	 (time).nanoseconds == NANOSEC_PER_SEC - 1)

/*
 *	boolean_t time_spec_nonzero(time_spec_t time)
 *
 *	Return TRUE if time_spec is not zero.
 */
#define time_spec_nonzero(time)						\
	((time).nanoseconds || (time).seconds)

/*
 *	boolean_t time_spec_valid(time_spec_t time)
 *
 *	Return TRUE if time is a valid time representation.
 */
#define time_spec_valid(time)						\
	((time).nanoseconds < NANOSEC_PER_SEC)

#endif	/* _MACH_TIMESPEC_H_ */
