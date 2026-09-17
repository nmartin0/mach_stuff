/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

#include "exit.h"
#include "th.h"

static boolean_t	th_inited = FALSE;

unsigned
th_init()
{
    register th_t	th;

    prio_init();
    sched_init();

    (unsigned)th = malloc(TH_STACK_SIZE);

    th->thread = mach_thread_self();

    th->reply_port = MACH_PORT_NULL;

    th_inited = TRUE;

    return ((unsigned)th + TH_STACK_SIZE - sizeof (int));
}

th_t
th_alloc(func, arg)
void	(*func)();
int	arg;
{
    register kern_return_t	result;
    register th_t		th;
    thread_state_regs_t		regs;
    thread_state_frame_t	frame;

    (unsigned)th = malloc(TH_STACK_SIZE);

    result = thread_create(mach_task_self(), &th->thread);
    if (result != KERN_SUCCESS)
	exit();

    sched_thread_setup(th->thread);

    regs.r_sp = (unsigned)th + TH_STACK_SIZE;
    *--(int *)regs.r_sp = arg;
    --(int *)regs.r_sp;

    (void) thread_set_state(th->thread,
			    THREAD_STATE_REGS,
			    (thread_state_t)&regs,
                            THREAD_STATE_REGS_COUNT);

    frame.f_normal.f_fmt = STKFMT_NORMAL;
    (void (*)())frame.f_normal.f_pc = func;
    frame.f_normal.f_sr = 0;

    (void) thread_set_state(th->thread,
			    THREAD_STATE_FRAME,
			    (thread_state_t)&frame,
                            THREAD_STATE_FRAME_COUNT);

    (void) thread_resume(th->thread);

    return (th);
}

th_t
th_self()
{
    int			x;
    register unsigned	stack = (unsigned)&x;

    if (!th_inited || stack < (unsigned)th_self)
	return ((th_t) 0);

    return ((th_t)(stack & ~(TH_STACK_SIZE - 1)));
}
