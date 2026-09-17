/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	sched.h,v $
 * Revision 2.6  93/11/17  17:21:04  dbg
 * 	Moved most functions to policy-specific routines.  This header
 * 	file now describes the periodic scheduling events.
 * 	[93/05/11            dbg]
 * 
 * Revision 2.5  91/05/14  16:46:04  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:28:50  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:16:37  mrt]
 * 
 * Revision 2.3  90/08/07  17:58:47  rpd
 * 	Picked up fix to MACH_FIXPRI version of csw_needed.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.2  90/06/02  14:55:44  rpd
 * 	Updated to new scheduling technology.
 * 	[90/03/26  22:15:22  rpd]
 * 
 * Revision 2.1  89/08/03  15:52:50  rwd
 * Created.
 * 
 * 20-Oct-88  David Golub (dbg) at Carnegie-Mellon University
 *	Use macro_help to avoid lint.
 *
 * 11-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Make csw_needed a macro here.  Ignore first_quantum for local_runq.
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	No more runrun.
 *
 * 18-May-88  David Black (dlb) at Carnegie-Mellon University
 *	Added shutdown queue for shutdown thread.
 *
 * 29-Mar-88  David Black (dlb) at Carnegie-Mellon University
 *	SIMPLE_CLOCK: added sched_usec for drift compensation.
 *
 * 25-Mar-88  David Black (dlb) at Carnegie-Mellon University
 *	Added sched_load and related constants.  Moved thread_timer_delta
 *	here because it depends on sched_load.
 *
 * 19-Feb-88  David Black (dlb) at Carnegie-Mellon University
 *	Added sched_tick and shift definitions for more flexible ageing.
 *
 * 18-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Removed conditionals, purged history.
 */
/*
 *	File:	sched.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	Periodic scheduling events.
 */

#ifndef	_KERN_SCHED_H_
#define _KERN_SCHED_H_

#include <simple_clock.h>

/*
 *	sched_tick increments once a second.  Used to age priorities.
 */

extern unsigned	sched_tick;

#if	SIMPLE_CLOCK
/*
 *	sched_usec is an exponential average of number of microseconds
 *	in a second for clock drift compensation.
 */

extern int	sched_usec;
#endif	/* SIMPLE_CLOCK */

/*
 *	Initialize the periodic scheduling calculations.
 */
extern void	init_sched_calculations(void);

#endif	/* _KERN_SCHED_H_ */
