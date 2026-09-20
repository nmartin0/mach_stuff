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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	boot.c,v $
 * Revision 2.7  94/04/13  14:49:32  mrt
 * 	Added some comments.
 * 	[94/04/12            mrt]
 * 
 * Revision 2.6  92/02/02  13:01:30  rpd
 * 	Removed task_by_unix_pid definition; use task_by_pid instead.
 * 	Changed mach_privileged_host_port to mach_host_priv_self.
 * 	Changed mach_device_server_port to mach_master_device_port.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.4  91/03/09  14:31:53  rpd
 * 	Reset the exception port.
 * 	[91/01/12            rpd]
 * 
 * Revision 2.3  90/12/04  21:54:48  rpd
 * 	Fixed signal bug.
 * 	[90/11/25            rpd]
 * 
 * Revision 2.2  90/09/08  00:06:26  rwd
 * 	First Checkin.
 * 	[90/09/07  17:15:46  rwd]
 * 
 */
/*
 *	File:	./boot.c
 *	Author:	Randall W. Dean
 *
 *	Copyright (c) 1990 Randall W. Dean
 */
/* 
 *  Program that can be used to bootstrap some other
 *  server program. It will give the out the privileged 
 *  ports to the first task that asks using the same
 *  message interface that the kernel does. This program
 *  can be run on the UX server to boot poe as as second
 *  server or can be run on poe to boot something else
 *  e.g. the multi-server's configuration server.
 *  argv[1] is the program that will be exec'ed.
 *  Any other arguments will be passed on to the child.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <mach.h>
#include <mach/message.h>

int begin_flag = FALSE;

begin()
{
	begin_flag = TRUE;
}

main(argc, argv)
int argc;
char *argv[];
{
	int pid, wpid, ppid;
	int mask;
	kern_return_t kr;
	mach_port_t bport;
	mach_port_t btask;

	/* ppid = parent pid */
	ppid = getpid();
	signal(SIGUSR1, begin);
	if (!(pid=fork())) {
	    /*  child process */
	    kill(ppid, SIGUSR1);
	    mask = sigblock(sigmask(SIGUSR1));
	    while (!begin_flag) sigpause(mask &~ sigmask(SIGUSR1));
	    (void) sigsetmask(mask);
	    execve(argv[1], &argv[1], 0);
	    perror("Exec failure");
	    exit(1);
	}

	/* parent */
	mask = sigblock(sigmask(SIGUSR1));
	while (!begin_flag) sigpause(mask &~ sigmask(SIGUSR1));
	(void) sigsetmask(mask);

	/*
	 *	Allocate a port for the child to talk to us 
	 */
	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
				&bport);
	if (kr != KERN_SUCCESS) {
		mach_error("mach_port_allocate",kr);
		kill(pid, SIGKILL);
		exit(1);
	}

	kr = mach_port_insert_right(mach_task_self(), bport, bport,
				    MACH_MSG_TYPE_MAKE_SEND);
	if (kr != KERN_SUCCESS) {
		mach_error("mach_port_insert_right", kr);
		kill(pid, SIGKILL);
		exit(1);
	}

	/* btask = child's task port  */
	btask = task_by_pid(pid);
	if (btask == MACH_PORT_NULL) {
		printf("task_by_pid\n");
		kill(pid, SIGKILL);
		exit(1);
	}

	/*
	 *	Set the child's bootstrap port to our port, so we get
	 *	the request for the privileged host/device ports.
	 */

	kr = task_set_special_port(btask, TASK_BOOTSTRAP_PORT, bport);
	if (kr != KERN_SUCCESS) {
		mach_error("task_set_special_port", kr);
		kill(pid, SIGKILL);
		exit(1);
	}

	/*
	 *	Set the exception port to null, so that exceptions
	 *	drop into the kernel debugger (when it is enabled).
	 */

	kr = task_set_special_port(btask, TASK_EXCEPTION_PORT, MACH_PORT_NULL);
	if (kr != KERN_SUCCESS) {
		mach_error("task_set_special_port", kr);
		kill(pid, SIGKILL);
		exit(1);
	}

	/* syscall 13 is Bsd_ttyc(flag)
	 *		flag = 0 does tty_suspend
	 *		flag = 1 does tty_resume
	 */
	syscall(13,0);
	kill(pid, SIGUSR1);

	{
	    struct imsg {
		mach_msg_header_t	hdr;
		mach_msg_type_t		port_desc_1;
		mach_port_t		port_1;
		mach_msg_type_t		port_desc_2;
		mach_port_t		port_2;
	    } imsg;

	    mach_port_t privileged_host_port;
	    mach_port_t device_server_port;

	    privileged_host_port = mach_host_priv_self();
	    device_server_port = mach_master_device_port();

	    /*
	     * Wait for the port-request message
	     */
	    while (kr=mach_msg(&imsg.hdr, MACH_RCV_MSG,
			       0, sizeof imsg.hdr, bport,
			       MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL)
						!= MACH_MSG_SUCCESS) {
		mach_error("mach_msg",kr);
	    }


	    /*
	     * Send back the host and device ports
	     */

	    imsg.hdr.msgh_bits = MACH_MSGH_BITS_COMPLEX |
		MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(imsg.hdr.msgh_bits), 0);
	    /* msgh_size doesn't need to be initialized */
	    /* leave msgh_remote_port untouched */
	    imsg.hdr.msgh_local_port = MACH_PORT_NULL;
	    imsg.hdr.msgh_id += 100;	/* this is a reply msg */

	    imsg.port_desc_1.msgt_name = MACH_MSG_TYPE_COPY_SEND;
	    imsg.port_desc_1.msgt_size = 32;
	    imsg.port_desc_1.msgt_number = 1;
	    imsg.port_desc_1.msgt_inline = TRUE;
	    imsg.port_desc_1.msgt_longform = FALSE;
	    imsg.port_desc_1.msgt_deallocate = FALSE;
	    imsg.port_desc_1.msgt_unused = 0;

	    imsg.port_1	= privileged_host_port;

	    imsg.port_desc_2 = imsg.port_desc_1;

	    imsg.port_2	= device_server_port;

	    kr = mach_msg(&imsg.hdr, MACH_SEND_MSG,
			  sizeof imsg, 0, MACH_PORT_NULL,
			  MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	    if (kr != MACH_MSG_SUCCESS) {
		mach_error("mach_msg", kr);
		syscall(13,1);
		exit(1);
	    }
	}

	while((wpid = wait(0)) <= 0);
	syscall(13,1);  /* tty_resume */

	if (wpid == -1) {
	    printf("wait no children\n");
	    kill(pid, SIGKILL);
	    exit(1);
	}

	if (wpid != pid) {
	    printf("wait wrong child\n");
	    kill(pid, SIGKILL);
	    exit(1);
	}

	exit(0);
}

