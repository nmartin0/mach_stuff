/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: policy.h,v $
 * Revision 1.2.6.2  1996/01/09  19:22:09  devrcs
 * 	Change elements of policy structs from int to integer_t.
 * 	This matches mach_types.defs.
 * 	Include mach/vm_types.h
 * 	[1995/12/01  19:49:23  jfraser]
 *
 * 	Merged '64-bit safe' changes from DEC alpha port.
 * 	[1995/11/21  18:09:18  jfraser]
 *
 * Revision 1.2.6.1  1994/09/23  02:41:37  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:42:19  ezf]
 * 
 * Revision 1.2.2.4  1993/08/04  19:34:23  gm
 * 	CR9523: Added missing include of <mach/boolean.h> header.
 * 	[1993/08/02  18:26:43  gm]
 * 
 * Revision 1.2.2.3  1993/06/29  21:55:46  watkins
 * 	New definitions for scheduling control interfaces.
 * 	[1993/06/29  21:52:55  watkins]
 * 
 * Revision 1.2.2.2  1993/06/09  02:42:54  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:17:50  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:38:27  devrcs
 * 	Added POLICYCLASS_FIXEDPRI definition. Fixed
 * 	invalid_policy() macro.
 * 	[93/03/17            jat]
 * 
 * 	Added POLICY_RR and POLICY_FIFO. Removed
 * 	POLICY_FIXEDPRI.
 * 	[93/01/28            fdr]
 * 
 * 	ansi C conformance changes
 * 	[1993/02/02  18:54:10  david]
 * 
 * Revision 1.1  1992/09/30  02:31:57  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
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
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 */

#ifndef	_MACH_POLICY_H_
#define _MACH_POLICY_H_

/*
 *	mach/policy.h
 *
 *	Definitions for scheduing policy.
 */

#include <mach/boolean.h>
#include <mach/vm_types.h>

/*
 *	Policy definitions.  Policies should be powers of 2,
 *	but cannot be or'd together other than to test for a
 *	policy 'class'.
 */
#define	POLICY_TIMESHARE	1	/* timesharing		*/
#define	POLICY_RR		2	/* fixed round robin	*/
#define POLICY_FIFO		4	/* fixed fifo		*/

/*
 *	Check if policy is of 'class' fixed-priority.
 */
#define	POLICYCLASS_FIXEDPRI	(POLICY_RR | POLICY_FIFO)

/*
 *	Check if policy is valid.
 */
#define invalid_policy(policy)			\
	((policy) != POLICY_TIMESHARE &&	\
	 (policy) != POLICY_RR &&		\
	 (policy) != POLICY_FIFO)


/*
 *	New scheduling control interface
 */
typedef int 				policy_t;
typedef integer_t			*policy_info_t;
typedef integer_t			*policy_base_t;
typedef integer_t			*policy_limit_t;


/*
 * 	Types for TIMESHARE policy
 */
struct policy_timeshare_base {			
	integer_t		base_priority;
};
struct policy_timeshare_limit {
	integer_t		max_priority;
};
struct policy_timeshare_info {
	integer_t		max_priority;
	integer_t		base_priority;
	integer_t		cur_priority;
	boolean_t		depressed;
	integer_t		depress_priority;
};

typedef struct policy_timeshare_base	*policy_timeshare_base_t; 
typedef struct policy_timeshare_limit	*policy_timeshare_limit_t;
typedef struct policy_timeshare_info	*policy_timeshare_info_t;

typedef struct policy_timeshare_base	policy_timeshare_base_data_t; 
typedef struct policy_timeshare_limit	policy_timeshare_limit_data_t;
typedef struct policy_timeshare_info	policy_timeshare_info_data_t;


#define POLICY_TIMESHARE_BASE_COUNT		\
	(sizeof(struct policy_timeshare_base)/sizeof(integer_t))
#define POLICY_TIMESHARE_LIMIT_COUNT		\
	(sizeof(struct policy_timeshare_limit)/sizeof(integer_t))
#define POLICY_TIMESHARE_INFO_COUNT		\
	(sizeof(struct policy_timeshare_info)/sizeof(integer_t))


/*
 *	Types for the ROUND ROBIN (RR) policy
 */
struct policy_rr_base {				
	integer_t		base_priority;
	integer_t		quantum;
};
struct policy_rr_limit {
	integer_t		max_priority;
};
struct policy_rr_info {
	integer_t		max_priority;
	integer_t		base_priority;
	integer_t		quantum;
	boolean_t		depressed;
	integer_t		depress_priority;
};

typedef struct policy_rr_base		*policy_rr_base_t;
typedef struct policy_rr_limit		*policy_rr_limit_t;
typedef struct policy_rr_info		*policy_rr_info_t;

typedef struct policy_rr_base		policy_rr_base_data_t;
typedef struct policy_rr_limit		policy_rr_limit_data_t;
typedef struct policy_rr_info		policy_rr_info_data_t;

#define POLICY_RR_BASE_COUNT		\
	(sizeof(struct policy_rr_base)/sizeof(integer_t))
#define POLICY_RR_LIMIT_COUNT		\
	(sizeof(struct policy_rr_limit)/sizeof(integer_t))
#define POLICY_RR_INFO_COUNT		\
	(sizeof(struct policy_rr_info)/sizeof(integer_t))


/*
 * 	Types for the FIRST-IN-FIRST-OUT (FIFO) policy
 */
struct policy_fifo_base {		
	integer_t		base_priority;
};
struct policy_fifo_limit {
	integer_t		max_priority;
};
struct policy_fifo_info {
	integer_t		max_priority;
	integer_t		base_priority;
	boolean_t		depressed;
	integer_t		depress_priority;
};

typedef struct policy_fifo_base		*policy_fifo_base_t;
typedef struct policy_fifo_limit	*policy_fifo_limit_t;
typedef struct policy_fifo_info		*policy_fifo_info_t;

typedef struct policy_fifo_base		policy_fifo_base_data_t;
typedef struct policy_fifo_limit	policy_fifo_limit_data_t;
typedef struct policy_fifo_info		policy_fifo_info_data_t;

#define POLICY_FIFO_BASE_COUNT		\
	(sizeof(struct policy_fifo_base)/sizeof(integer_t))
#define POLICY_FIFO_LIMIT_COUNT		\
	(sizeof(struct policy_fifo_limit)/sizeof(integer_t))
#define POLICY_FIFO_INFO_COUNT		\
	(sizeof(struct policy_fifo_info)/sizeof(integer_t))


/*
 * 	Aggregate policy types
 */

struct policy_bases {
	policy_timeshare_base_data_t	ts;
	policy_rr_base_data_t		rr;
	policy_fifo_base_data_t		fifo;
};

struct policy_limits {
	policy_timeshare_limit_data_t	ts;
	policy_rr_limit_data_t		rr;
	policy_fifo_limit_data_t	fifo;
};

struct policy_infos {
	policy_timeshare_info_data_t	ts;
	policy_rr_info_data_t		rr;
	policy_fifo_info_data_t		fifo;
};

typedef struct policy_bases		policy_base_data_t;
typedef struct policy_limits		policy_limit_data_t;
typedef struct policy_infos		policy_info_data_t;


#endif /* _MACH_POLICY_H_ */
