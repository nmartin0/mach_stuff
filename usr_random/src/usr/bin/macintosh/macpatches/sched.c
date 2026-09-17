/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

#include "prio.h"

static processor_set_t		pset;
static processor_set_name_t	pset_name;

extern host_priv_t		host_priv_port;

unsigned			sched_min_quantum;	/* msec */
unsigned			sched_base_priority;

void
sched_init()
{
    host_sched_info_data_t	host_sched_info;
    thread_sched_info_data_t	thread_sched_info;
    unsigned			info_count;

    info_count = HOST_SCHED_INFO_COUNT;
    (void) host_info(mach_host_self(),
		     HOST_SCHED_INFO,
		     &host_sched_info, &info_count);

    sched_min_quantum = host_sched_info.min_quantum / 1000;
    if ((host_sched_info.min_quantum % 1000) >= 500)
	sched_min_quantum++;

    info_count = THREAD_SCHED_INFO_COUNT;
    (void) thread_info(mach_thread_self(),
		       THREAD_SCHED_INFO,
		       (thread_state_t)&thread_sched_info,
                       &info_count);

    sched_base_priority = thread_sched_info.base_priority;
#ifdef debug
    sched_base_priority += PRIO_MAX;
#endif

    (void) processor_set_default(mach_host_self(),
				 &pset_name);

    (void) host_processor_set_priv(host_priv_port,
				   pset_name, &pset);

    (void) processor_set_policy_enable(pset,
				       POLICY_FIXEDPRI);
}

void
sched_thread_setup(thread)
register thread_t	thread;
{
    (void) thread_policy(thread,
			 POLICY_FIXEDPRI,
			 sched_min_quantum);

    (void) thread_max_priority(thread,
			       pset,
			       sched_base_priority - PRIO_MAX);
}

void
sched_set_prio(prio)
register	prio;
{
    (void) thread_priority(mach_thread_self(),
			   sched_base_priority - prio,
			   FALSE);
}
