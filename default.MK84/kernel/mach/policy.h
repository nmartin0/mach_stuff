/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	policy.h,v $
 * Revision 2.6  93/11/17  17:43:48  dbg
 * 	Moved names for real-time policies to mach/realtime_policy.h.
 * 	Added common scheduling attributes for background, timesharing,
 * 	fixed-priority policies.
 * 	[93/05/10            dbg]
 * 
 * 	Added real-time and background policies.
 * 	[93/04/09            dbg]
 * 
 * Revision 2.5  93/01/14  17:46:25  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.4  91/05/14  16:58:29  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:22  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:11  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:37  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:51:22  rpd]
 * 
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 	Created.
 * 	[89/07/25  18:47:00  dlb]
 * 
 * Revision 2.3  89/10/15  02:05:50  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:40:53  dlb
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 
 */

#ifndef	_MACH_POLICY_H_
#define _MACH_POLICY_H_

/*
 *	mach/policy.h
 *
 *	Definitions for scheduing policy.
 */
#include <mach/machine/vm_types.h>	/* for integer_t */

#define	POLICY_BACKGROUND		0
#define	POLICY_TIMESHARE		1
#define POLICY_FIXEDPRI			2

/*
 *	Parameters and limits for standard policies.
 */
typedef	natural_t	*policy_param_t;
					/* varying array of unsigned integers */
#define	POLICY_PARAM_MAX_COUNT	(1024)	/* maximum array size */

/*
 *	Background threads have no parameters or limit values.
 */

/*
 *	Timesharing threads:
 *
 *	Policy parameter and limit values include only the
 *	base priority for the thread.
 */
struct policy_param_timeshare {
	natural_t	priority;	/* base priority for thread */
};
#define	POLICY_PARAM_TIMESHARE_COUNT \
	(sizeof(struct policy_param_timeshare)/sizeof(natural_t))

/*
 *	Information for timesharing threads includes
 *	the base, current, and maximum (limit) priorities.
 */
struct policy_info_timeshare {
	natural_t	base_priority;	/* base priority */
	natural_t	cur_priority;	/* current priority, including
					   aging */
	natural_t	max_priority;	/* maximum priority allowed */
};
#define	POLICY_INFO_TIMESHARE_COUNT \
	(sizeof(struct policy_info_timeshare)/sizeof(natural_t))

/*
 *	Fixed-priority threads:
 *
 *	Policy parameters include the priority, and
 *	whether the thread is preemptible by others at
 *	the same priority.
 */
struct policy_param_fixedpri {
	natural_t	priority;	/* priority of thread */
	/* boolean_t */
	    natural_t	no_preempt;	/* TRUE for non-preemptive
					   (FIFO) scheduling
					   FALSE for preemptinve
					   (round-robin) scheduling */
};
#define	POLICY_PARAM_FIXEDPRI_COUNT \
	(sizeof(struct policy_param_fixedpri)/sizeof(natural_t))

/*
 *	Info for fixed-priority threads also includes the
 *	maximum allowed priority.
 */
struct policy_info_fixedpri {
	natural_t	priority;
	/* boolean_t */
	    natural_t	no_preempt;
	natural_t	max_priority;
};
#define	POLICY_INFO_FIXEDPRI_COUNT \
	(sizeof(struct policy_info_fixedpri)/sizeof(natural_t))

#endif /* _MACH_POLICY_H_ */
