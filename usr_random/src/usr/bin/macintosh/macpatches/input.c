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

#include <device/device_types.h>

#include <mac2dev/adb.h>

#include <mac2os/Types.h>
#include <mac2os/DeskBus.h>

#include "exit.h"
#include "prio.h"

extern mach_port_t	master_device_port;

typedef struct adbVars {
    device_t		device;
    ADBDataBlock	db;
} *adbVars_t;

static struct adbVars	adb_info[16];

#define EVENT_SIZE	(sizeof (adb_cmd_t) + 2)

static mach_port_t	reply_port;

typedef struct {
    mach_msg_header_t	head;
    mach_msg_type_t	return_codeType;
    kern_return_t	return_code;
    mach_msg_type_t	dataType;
    char		data[128];
} device_msg_t;

static inline
io_return_t
post_input_request(device, wanted)
device_t	device;
unsigned	wanted;
{
    return (device_read_request_inband(device,
				       reply_port,
				       0, 0,
				       wanted));
}

static
inline
void
adb_call(data, db)
unsigned char	data[];
ADBDataBlock	*db;
{
    asm volatile("moveq #0,d0; movb %0@,d0; lea %0@(1),a0; movl %1,a2; movl %2,a1; jsr a1@"
		 :
		 : "a" (data), "rm" (db->dbDataAreaAddr), "rm" (db->dbServiceRtPtr)
		 : "d0", "d1", "d2", "d3",
		   "a0", "a1", "a2", "a3");
}

static inline
void
input_event(cmd)
register adb_cmd_t	*cmd;
{
    register adbVars_t	v = &adb_info[cmd->reg.addr];
    register		prio;

    prio = prio_enter(PRIO_LOW);

    (void) adb_call(cmd, &v->db);

    if (v->db.origADBAddr == ADB_ADDR_MOUSE)	/* XXX */
	update_cursor();

    (void) prio_exit(prio);

    (void) post_input_request(v->device, EVENT_SIZE);
}

void
input_server()
{
    device_msg_t	msg;
    register adb_cmd_t	*cmd = (adb_cmd_t *)msg.data;

    sched_set_prio(PRIO_LOW);

    while (TRUE) {
	msg.head.msgh_local_port = reply_port;
	msg.head.msgh_size = sizeof (msg);
	if (mach_msg_receive(&msg) != MACH_MSG_SUCCESS)
	    exit();

	if (msg.dataType.msgt_number < 4)
	    stop(msg.data, 0);

	input_event(cmd);
    }
}

void
input_init()
{
    register adbVars_t	v;

    if (mach_port_allocate(mach_task_self(),
			   MACH_PORT_RIGHT_RECEIVE,
			   &reply_port) != KERN_SUCCESS)
	exit();

    (void) mach_port_set_qlimit(mach_task_self(),
				reply_port, MACH_PORT_QLIMIT_MAX);

    v = &adb_info[3];
    GetADBInfo(&v->db, 3);
	
    if (device_open(master_device_port,
		    D_READ,
		    "adb3",
		    &v->device) != D_SUCCESS)
	exit();

    (void) device_set_status(v->device,
			     ADB_FLUSH, 0, 0);

    (void) post_input_request(v->device, EVENT_SIZE);

    v = &adb_info[2];
    GetADBInfo(&v->db, 2);
	
    if (device_open(master_device_port,
		    D_READ,
		    "adb2",
		    &v->device) != D_SUCCESS)
	exit();

    (void) device_set_status(v->device,
			     ADB_FLUSH, 0, 0);

    (void) post_input_request(v->device, EVENT_SIZE);

    (void) th_alloc(input_server, 0);
}
