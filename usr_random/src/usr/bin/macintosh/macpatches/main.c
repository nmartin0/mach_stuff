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

#include "exit.h"
#include "bootstrap.h"
#include "prio.h"

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Files.h>
#include <mac2os/Globals.h>

#include "OSUtils.h"

#define DrvQHdr			(*(QHdrPtr)0x308)

mach_port_t	master_device_port;
host_priv_t	host_priv_port;

/*
 * main program.  Sets up/installs
 * patches.  On return, ROM entry
 * point is executed.
 */
main()
{
    (void) exception_init();

    (void) emulation_init();

    (void) input_init();

    (void) clock_init();

    /*
     * Post disk inserted events for
     * all drives containing disks.
     */
    {
	register DrvQEl		*drv;
	register unsigned char	*stats;

	drv = (DrvQElPtr)DrvQHdr.qHead;
	while (drv) {
	    stats = ((unsigned char *)drv) - 4;		/* XXX */
	    if (stats[1])
		(void) PostEvent(7, drv->dQDrive);
	    drv = (DrvQElPtr)drv->qLink;
	}
    }
}

setup()
{
    mach_port_t		bootstrap_port, reply_port;
    bootstrap_msg_t	bootstrap_msg;

    (void) task_get_bootstrap_port(mach_task_self(), &bootstrap_port);
    reply_port = mig_get_reply_port();

    bootstrap_msg.head.msgh_bits =
	MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
    bootstrap_msg.head.msgh_remote_port = bootstrap_port;
    bootstrap_msg.head.msgh_local_port = reply_port;
    bootstrap_msg.head.msgh_kind = MACH_MSGH_KIND_NORMAL;
    if (mach_msg((mach_msg_header_t *)&bootstrap_msg,
                 MACH_SEND_MSG | MACH_RCV_MSG,
		 sizeof(bootstrap_msg.head),
		 sizeof(bootstrap_msg),
                 reply_port,
		 MACH_MSG_TIMEOUT_NONE,
                 MACH_PORT_NULL) != MACH_MSG_SUCCESS)
	exit();

    (void) mach_port_deallocate(mach_task_self(), bootstrap_port);

    master_device_port = bootstrap_msg.MasterDevicePort;
    host_priv_port = bootstrap_msg.HostPrivPort;
}

#ifdef notdef
    {
	unsigned char			block[512];
	extern unsigned			inst_count;
	extern mapped_time_value_t	*time;
	mapped_time_value_t		save1, save2;

	do {
	    save1 = *time;
	} while (save1.seconds != save1.check_seconds);
	
	pblk.io.ioVRefNum = BootDrive;
	pblk.io.ioRefNum =0xffde;
	pblk.io.ioBuffer = block;
	pblk.io.ioReqCount = sizeof (block);
	pblk.io.ioPosMode = fsFromStart;
	pblk.io.ioPosOffset = 0;
	asm("movl %0,a0; .word 0xa002"
	    :
	    : "rm" (&pblk)
	    : "a0", "d0");

	do {
	    save2 = *time;
	} while (save2.seconds != save2.check_seconds);

	stop(&save1, &save2);
    }
#endif
