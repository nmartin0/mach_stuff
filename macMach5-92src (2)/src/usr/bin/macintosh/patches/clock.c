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
#include <mach/message.h>

#include <exit.h>
#include <prio.h>
#include <th.h>

void
slow_tick_server()
{
    register mach_msg_return_t	result;
    register			prio;
    mach_port_t			clock_port;
    struct {
	mach_msg_header_t	head;
    } msg;

    if (mach_port_allocate(mach_task_self(),
			   MACH_PORT_RIGHT_RECEIVE,
			   &clock_port) != KERN_SUCCESS)
	exit();

    sched_set_prio(PRIO_LOW);

    while (TRUE) {
	result = mach_msg(&msg,
			  MACH_RCV_MSG|MACH_RCV_TIMEOUT,
			  0,
			  sizeof (msg),
			  clock_port,
			  1000,	/* ms */
			  MACH_PORT_NULL);
	if (result != MACH_RCV_TIMED_OUT)
	    exit();

	prio = prio_enter(PRIO_LOW);

	slow_tick();

	prio_exit(prio);
    }
}

void
tick_server()
{
    register mach_msg_return_t	result;
    register			prio;
    mach_port_t			clock_port;
    struct {
	mach_msg_header_t	head;
    } msg;

    if (mach_port_allocate(mach_task_self(),
			   MACH_PORT_RIGHT_RECEIVE,
			   &clock_port) != KERN_SUCCESS)
	exit();

    sched_set_prio(PRIO_LOW);

    while (TRUE) {
	result = mach_msg(&msg,
			  MACH_RCV_MSG|MACH_RCV_TIMEOUT,
			  0,
			  sizeof (msg),
			  clock_port,
			  16,	/* ms */
			  MACH_PORT_NULL);
	if (result != MACH_RCV_TIMED_OUT)
	    exit();

	prio = prio_enter(PRIO_LOW);

	VBL_tick();

	prio_exit(prio);
    }
}

void
clock_init()
{
    (void) th_alloc(slow_tick_server, 0);
    (void) th_alloc(tick_server, 0);
}
