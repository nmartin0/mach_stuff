/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	request.h,v $
 * Revision 2.3  91/02/14  14:33:00  mrt
 * 	Changed to new Mach copyright
 * 
 * Revision 2.2  90/01/24  23:33:52  mrt
 * 	Changed include of kern/mach_types.h to mach/mach_types.h
 * 	[90/01/22            mrt]
 * 
 * 	Created
 * 	[90/01/18            dlb]
 * 
 */
/*
 *	File: 	request.h
 *	Author:	David Black, Carnegie Mellon University
 *	Date:	Jan 1990
 *
 *	Request type definitions.
 */

#include <mach/mach_types.h>
#include <cthreads.h>
#include "queue.h"

#include "assign.h"
#include "cpu_server.h"

#define	MAX_SETS	64		/* max sets in a request */

struct	request {
    	queue_chain_t	all_req;		/* doubly linked list */
	queue_chain_t	active_req;		/* another d-l list */
	cpu_request_t	port;			/* corresponding port */
	int		total_processors;	/* total requested */
	run_processor_t	processor_ids;		/* processor ids */
	int		set_count;		/* sets involved */
	processor_set_t	sets[MAX_SETS];		/* the sets themselves */
	int		assigned_processors;	/* assignment requests */
	int		processors[MAX_SETS];	/* corresponding requests */
	int		options;		/* options for this request */
	int		run_time;		/* elapsed time requested */
	int		total_time;		/* total time to consume */
	int		event_time;		/* time of next event */
	int		state;			/* current state */
	port_t		notify_port;		/* port for client notify */
	boolean_t	end_notify_done;	/* final notify done ? */
	task_t		task;			/* task to suspend */
	boolean_t	resume_needed;		/* resume needed on assign? */
	struct mutex	lock;			/* cthreads lock */
	int		ref_count;		/* reference count */
};

typedef	struct request		request_data_t;
typedef struct request		*request_t;

/*
 *	State definitions
 */

#define	REQUEST_INACTIVE	0
#define REQUEST_ACTIVATED	1
#define	REQUEST_ASSIGNED	2
#define REQUEST_DEAD		3

/*
 *	NOTE: times in client interface are in seconds, internal times
 *	are in tenths.
 */

/*
 *	List of all requests
 */

queue_head_t	all_requests;
struct mutex	all_requests_lock;

queue_head_t	free_requests;
struct mutex	free_requests_lock;

#define REQUEST_NULL	(request_t)0

#define request_lock(request)	mutex_lock(&request->lock)
#define request_unlock(request)	mutex_unlock(&request->lock)

/*
 *	queue of requests with times.
 */

queue_head_t	time_queue;
struct mutex	time_queue_lock;
struct condition time_queue_condition;

thread_t	time_thread;

struct mutex	log_file_lock;	/* both threads log information */

/*
 *	Useful ports
 */
 
port_t	server_port;
port_set_name_t	server_port_set;

/*
 *	Internal options.  Clients invoke these by calling different
 *	interface routines.
 */

#define CPU_OPTION_NOTIFY	0x4
#define CPU_OPTION_SUSPEND	0x8
