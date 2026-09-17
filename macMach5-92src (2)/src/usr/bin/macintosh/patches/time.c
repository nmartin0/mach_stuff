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

#include <device/device_types.h>

#include <exit.h>
#include <emul.h>
#include <prio.h>

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Timer.h>

#include <OSUtils.h>

QHdr	TMQ;

extern mach_port_t	master_device_port;

static mach_port_t	time_ports;
static device_t		time_device;

#define TaskActive	0x8000

void
time_server()
{
    register mach_msg_return_t	result;
    register			prio;
    struct {
	mach_msg_header_t	head;
	mach_msg_type_t		return_codeType;
	kern_return_t		return_code;
	mach_msg_type_t		dataType;
	char			data[128];
    } request_msg;

    if (mach_port_allocate(mach_task_self(),
			   MACH_PORT_RIGHT_PORT_SET,
			   &time_ports) != KERN_SUCCESS)
	exit();

    sched_set_prio(PRIO_LOW);

    while (TRUE) {
	request_msg.head.msgh_local_port = time_ports;
	request_msg.head.msgh_size = sizeof (request_msg);
	result = mach_msg_receive(&request_msg);

	prio = prio_enter(PRIO_LOW);

	time_intr(request_msg.head.msgh_local_port);

	prio_exit(prio);
    }
}

void
initTIMER()
{
    (void) device_open(master_device_port,
		       D_READ|D_WRITE,
		       "time",
		       &time_device);

    (void) th_alloc(time_server, 0);
}

void
time_intr(port)
register mach_port_t	port;
{
    register TMTaskPtr	tp = (TMTaskPtr)TMQ.qHead;

    while (tp != 0) {
	if (port == tp->tmCount) {
	    if (tp->qType & TaskActive) {
		tp->qType &= ~TaskActive;

		if (tp->tmAddr)
		    asm volatile ("movl %0,a1; jsr %1@"
				  :
				  : "r" (tp), "a" (tp->tmAddr)
				  : "a0", "a1", "a2", "a3",
				    "d0", "d1", "d2", "d3" );
	    }

	    break;
	}

	tp = (TMTaskPtr)tp->qLink;
    }
}

InsTime(regs)
os_reg_t	regs;
{
    register TMTaskPtr	tp = (TMTaskPtr)regs.a_0;

    (void) mach_port_allocate(mach_task_self(),
			      MACH_PORT_RIGHT_RECEIVE,
			      &tp->tmCount);

    (void) mach_port_insert_right(mach_task_self(),
				  tp->tmCount, tp->tmCount,
				  MACH_MSG_TYPE_MAKE_SEND);

    (void) mach_port_move_member(mach_task_self(),
				 tp->tmCount,
				 time_ports);

    tp->qType &= ~TaskActive;

    (void) enqueue((QElemPtr)tp, &TMQ);

    regs.d_0 = noErr;
}

PrimeTime(regs)
os_reg_t	regs;
{
    register TMTaskPtr	tp = (TMTaskPtr)regs.a_0;
    register long	count = (long)regs.d_0;

    tp->qType |= TaskActive;

    (void) device_read_request_inband(time_device,
				      tp->tmCount,
				      0, count, 0);

    regs.d_0 = noErr;
}

RmvTime(regs)
os_reg_t	regs;
{
    register TMTaskPtr	tp = (TMTaskPtr)regs.a_0;

    (void) dequeue((QElemPtr)tp, &TMQ);

    tp->qType &= ~TaskActive;

    (void) device_read_request_inband(time_device,
				      tp->tmCount,
				      0, 0, 0);

    (void) mach_port_destroy(mach_task_self(), tp->tmCount);

    tp->tmCount = 0;

    regs.d_0 = noErr;
}
