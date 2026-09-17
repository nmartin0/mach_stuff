/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <mach/exception.h>

#include "exit.h"
#include "prio.h"

static mach_port_t	exception_port, old_exception_port;

void
exception_init()
{
    kern_return_t	result;
    void		exception_server();

    result = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_RECEIVE,
				&exception_port);
    if (result != KERN_SUCCESS)
	exit();

    result = mach_port_insert_right(mach_task_self(),
				    exception_port, exception_port,
				    MACH_MSG_TYPE_MAKE_SEND);
    if (result != KERN_SUCCESS)
	exit();

    result = task_get_exception_port(mach_task_self(),
				     &old_exception_port);
    if (result != KERN_SUCCESS)
	exit();

    result = task_set_exception_port(mach_task_self(),
				     exception_port);
    if (result != KERN_SUCCESS)
	exit();

    (void) th_alloc(exception_server, 0);
}

void
exception_server()
{
    register mach_msg_return_t	result;
    struct {
	mach_msg_header_t	head;
	mach_msg_type_t		threadType;
	mach_port_t		thread;
	mach_msg_type_t		taskType;
	mach_port_t		task;
	mach_msg_type_t		exceptionType;
	int			exception;
	mach_msg_type_t		codeType;
	int			code;
	mach_msg_type_t		subcodeType;
	int			subcode;
    } request_msg;
    struct {
	mach_msg_header_t	head;
	mach_msg_type_t		RetCodeType;
	kern_return_t		RetCode;
    } reply_msg;

    (void) thread_set_exception_port(mach_thread_self(), old_exception_port);

    sched_set_prio(PRIO_MAX);

    while (TRUE) {
	request_msg.head.msgh_local_port = exception_port;
	request_msg.head.msgh_size = sizeof (request_msg);
	result = mach_msg_receive(&request_msg);
	if (result == RCV_SUCCESS) {
	    if (!exc_server(&request_msg, &reply_msg)
		|| reply_msg.RetCode != KERN_SUCCESS)
		reply_msg.RetCode = exception_raise(old_exception_port,
						    request_msg.thread,
						    request_msg.task,
						    request_msg.exception,
						    request_msg.code,
						    request_msg.subcode);
	    if (reply_msg.RetCode != MIG_NO_REPLY)
		(void) mach_msg_send(&reply_msg);

	    (void) mach_port_deallocate(mach_task_self(), request_msg.thread);
	    (void) mach_port_deallocate(mach_task_self(), request_msg.task);
	}
	else
	    exit();
    }
}

/*
 * Handle an exception.
 */
kern_return_t
catch_exception_raise(mach_port_t		exception_port,
		      mach_port_t		thread,
		      mach_port_t		task,
		      register			exception,
		      int			code,
		      int			subcode)
{
    if (task != mach_task_self())
	return (KERN_FAILURE);

    switch (exception) {
      case EXC_BAD_INSTRUCTION:
	if (code == EXC_MAC2_PRIVILEGE_VIOLATION)
	    return (priv_inst(thread));
	break;

      case EXC_BAD_ACCESS:
	return (access_fault(thread, code, subcode));
    }

    return (KERN_FAILURE);
}
