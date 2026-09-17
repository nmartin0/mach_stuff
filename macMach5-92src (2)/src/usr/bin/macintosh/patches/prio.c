/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

#include <mach.h>

#include <mach/thread_switch.h>

#include <prio.h>
#include <lock.h>
#include <th.h>
#include <sched.h>

#define THREAD_NULL	MACH_PORT_NULL

int	cur_prio, int_prio;

lock_t	prio_lock;

struct prio {
    thread_t	thread;
} prio_data[PRIO_MAX+1];
typedef struct prio	*prio_t;

void
prio_init()
{
    prio_data[PRIO_NONE].thread = mach_thread_self();
}

int
prio_get()
{
    register	prio;

    lock(prio_lock);

    prio = cur_prio;

    unlock(prio_lock);

    return (prio);
}

int
prio_get_thread(thread)
thread_t	thread;
{
    register	prio;

    lock(prio_lock);

    if (thread == prio_data[cur_prio].thread)
	prio = cur_prio;
    else
	prio = -1;

    unlock(prio_lock);

    return (prio);
}

int
prio_set(new_prio)
register		new_prio;
{
    register		old_prio;

    lock(prio_lock);

    old_prio = cur_prio;

    if (old_prio < new_prio) {
	/* Raising */
	prio_data[new_prio].thread = prio_data[old_prio].thread;
	cur_prio = new_prio;
    }
    else if (old_prio > new_prio) {
	/* Lowering */
	if (old_prio > int_prio)
	    prio_data[old_prio].thread = THREAD_NULL;

	if (new_prio >= int_prio)
	    cur_prio = new_prio;
	else
	    cur_prio = int_prio;
    }

    unlock(prio_lock);

    return (old_prio);
}

int
prio_set_thread(thread, new_prio)
thread_t		thread;
register		new_prio;
{
    register		old_prio;

    lock(prio_lock);

    if (thread == prio_data[cur_prio].thread) {
	old_prio = cur_prio;

	if (old_prio < new_prio) {
	    /* Raising */
	    prio_data[new_prio].thread = thread;
	    cur_prio = new_prio;
	}
	else if (old_prio > new_prio) {
	    /* Lowering */
	    if (old_prio > int_prio)
		prio_data[old_prio].thread = THREAD_NULL;

	    if (new_prio >= int_prio)
		cur_prio = new_prio;
	    else
		cur_prio = int_prio;
	}
    }
    else
	old_prio = -1;

    unlock(prio_lock);

    return (old_prio);
}

int
prio_enter(new_prio)
register		new_prio;
{
    register thread_t	thread;
    register		old_prio;

    lock(prio_lock);

    while (new_prio <= cur_prio) {
	thread = prio_data[cur_prio].thread;

	unlock(prio_lock);

	thread_switch(thread, SWITCH_OPTION_DEPRESS, sched_min_quantum / 4);

	lock(prio_lock);
    }

    old_prio = cur_prio;

    if (thread = prio_data[old_prio].thread)
	(void) thread_suspend(thread);

    prio_data[new_prio].thread = th_self()->thread;

    int_prio = cur_prio = new_prio;

    unlock(prio_lock);

    return (old_prio);
}

void
prio_exit(new_prio)
register		new_prio;
{
    register thread_t	thread;

    lock(prio_lock);

    prio_data[cur_prio].thread = THREAD_NULL;
    prio_data[int_prio].thread = THREAD_NULL;

    if (thread = prio_data[new_prio].thread)
	(void) thread_resume(thread);

    int_prio = cur_prio = new_prio;

    unlock(prio_lock);
}
