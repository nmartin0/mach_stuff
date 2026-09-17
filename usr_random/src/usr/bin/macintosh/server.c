/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: emul/server/server.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include "server_defs.h"
#include "bootstrap.h"

#include <mach/message.h>

#include <cthreads.h>

#include <mach/mig_errors.h>

#include "macserver.h"

mach_port_t		master_device_port, host_priv_port;

static mach_port_t	except_port_set, pager_port_set;
static mach_port_t	bootstrap_port, exception_port;
memory_object_t		memory_object;

any_t	server_loop();

void
server_main()
{
    kern_return_t	result;

    result = DeviceMaster(mach_host_self(), &master_device_port);
    if (result != KERN_SUCCESS)
	mach_error_exit("get master device port", result);

    result = HostPriv(mach_host_self(), &host_priv_port);
    if (result != KERN_SUCCESS)
	mach_error_exit("get host priv port", result);

    result = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_RECEIVE,
				&bootstrap_port);
    if (result != KERN_SUCCESS)
	mach_error_exit("allocate bootstrap port", result);
    result = mach_port_insert_right(mach_task_self(),
				    bootstrap_port, bootstrap_port,
				    MACH_MSG_TYPE_MAKE_SEND);
    if (result != KERN_SUCCESS)
	mach_error_exit("insert make send right bootstrap port", result);
    result = task_set_bootstrap_port(task, bootstrap_port);
    if (result != KERN_SUCCESS)
	mach_error_exit("set task bootstrap port", result);

    result = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_PORT_SET,
				&except_port_set);
    if (result != KERN_SUCCESS)
	mach_error_exit("allocate exception port set", result);

    result = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_PORT_SET,
				&pager_port_set);
    if (result != KERN_SUCCESS)
	mach_error_exit("allocate pager port set", result);

    result = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_RECEIVE,
				&exception_port);
    if (result != KERN_SUCCESS)
	mach_error_exit("allocate exception port", result);
    result = mach_port_insert_right(mach_task_self(),
				    exception_port, exception_port,
				    MACH_MSG_TYPE_MAKE_SEND);
    if (result != KERN_SUCCESS)
	mach_error_exit("insert make send right exception port", result);
    result = mach_port_move_member(mach_task_self(),
				   exception_port, except_port_set);
    if (result != KERN_SUCCESS)
	mach_error_exit("move exception port", result);
    result = task_set_exception_port(task, exception_port);
    if (result != KERN_SUCCESS)
	mach_error_exit("set task exception port", result);

    result = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_RECEIVE,
				&memory_object);
    if (result != KERN_SUCCESS)
	mach_error_exit("allocate memory object", result);
    result = mach_port_insert_right(mach_task_self(),
				    memory_object, memory_object,
				    MACH_MSG_TYPE_MAKE_SEND);
    if (result != KERN_SUCCESS)
	mach_error_exit("insert make send right memory object", result);
    result = mach_port_move_member(mach_task_self(),
				   memory_object, pager_port_set);
    if (result != KERN_SUCCESS)
	mach_error_exit("move memory object", result);

    cthread_detach(cthread_fork(server_loop,
				(any_t) except_port_set)
		   );

    cthread_detach(cthread_fork(server_loop,
				(any_t) pager_port_set)
		   );

    setup_mappings();

    task_resume(task);

    {
	bootstrap_msg_t		bootstrap_msg;

	bootstrap_msg.head.msgh_local_port = bootstrap_port;
	bootstrap_msg.head.msgh_size = sizeof (bootstrap_msg.head);
	result = mach_msg_receive(&bootstrap_msg);
	if (result != MACH_MSG_SUCCESS)
	    mach_error_exit("msg receive bootstrap port", result);
	bootstrap_msg.head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;
	bootstrap_msg.head.msgh_local_port = MACH_PORT_NULL;
	bootstrap_msg.head.msgh_size = sizeof (bootstrap_msg);
	bootstrap_msg.MasterDevicePortType =
	    (mach_msg_type_t)
		{ MACH_MSG_TYPE_COPY_SEND, 32, 1, TRUE, FALSE, FALSE, 0 };
	bootstrap_msg.MasterDevicePort = master_device_port;
	bootstrap_msg.HostPrivPortType =
	    (mach_msg_type_t)
		{ MACH_MSG_TYPE_COPY_SEND, 32, 1, TRUE, FALSE, FALSE, 0 };
	bootstrap_msg.HostPrivPort = host_priv_port;

	(void) mach_msg_send(&bootstrap_msg);

	(void) mach_port_destroy(mach_task_self(), bootstrap_port);
    }

    console_fix();

    for (;;)
	sigpause(0);
}

any_t
server_loop(port_set)
mach_port_t			port_set;
{
    msg_return_t		result;
    struct {
	mach_msg_header_t	head;
	mach_msg_type_t		RetCodeType;
	kern_return_t		RetCode;
    } *reply_msg;
    struct {
	mach_msg_header_t	head;
    } *request_msg;

    (void) vm_allocate(mach_task_self(),
		       (vm_address_t *)&request_msg,
		       vm_page_size,
		       TRUE);

    (void) vm_allocate(mach_task_self(),
		       (vm_address_t *)&reply_msg,
		       vm_page_size,
		       TRUE);

    for (;;) {
	request_msg->head.msgh_size = vm_page_size;
	request_msg->head.msgh_local_port = port_set;

	result = mach_msg_receive(request_msg);
	if (result != RCV_SUCCESS)
	    other_error_exit("msg_receive error");

	if (request_msg->head.msgh_local_port == exception_port)
	    (void) exc_server(request_msg, reply_msg);
	else if (request_msg->head.msgh_local_port == memory_object)
	    (void) memory_object_server(request_msg, reply_msg);
	else
	    other_error_exit("msg_receive unknown local port");

	if (reply_msg->RetCode != MIG_NO_REPLY)
	    (void) mach_msg_send(reply_msg);
    }
}
