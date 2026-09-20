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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	main.c,v $
 * Revision 2.3  91/02/08  16:05:04  mrt
 * 	Added a task_set_notify_port to PORT_NULL in the
 * 	initialization, to avoid notify messages building up.
 * 	[91/02/08            mrt]
 * 
 * Revision 2.2  90/01/24  23:33:41  mrt
 * 	Changed include of sys/message.h to mach/message.h and 
 * 	sys/error.h to mach/error.h
 * 	[90/01/22            mrt]
 * 
 * 	Created
 * 	[90/01/18            dlb]
 * 
 */
/*
 *	File: 	main.c
 *	Author:	David Black, Carnegie Mellon University
 *	Date:	Jan 1990
 *
 *	Main file for cpu server
 */

#include <mach.h>
#include <mach/message.h>
#include <mach/error.h>
#include <sys/time.h>

#include <servers/netname.h>

#include <stdio.h>

#include "request.h"

main(argc, argv)
int	argc;
char	*argv[];
{
    mach_error_t	code;
    int			count;
    port_t		port;
    extern int	run_time_queue();

    /*
     *	Initialize everything.
     */
    port_allocate(task_self(), &server_port);
    port_set_allocate(task_self(), &server_port_set);
    port_set_add(task_self(), server_port_set, server_port);
    task_set_notify_port(task_self(),PORT_NULL);

    request_init();
    processor_init();
    mutex_init(&log_file_lock);

    /*
     *	Fork off second cthread.  It should immediately wait on time_queue.
     */
    time_thread = (thread_t)0;
    cthread_detach(cthread_fork(run_time_queue, 0));
    while (time_thread == (thread_t)0)
    		;
    printf("main: started time_thread.\n");
    fflush(stdout);

    /*
     *  Register our name with name-service, then loop processing requests.
     *  Make sure the name service is there first; be patient.
     */
    count = 0;
    while (1) {
	if (netname_look_up(name_server_port,"","NM_ACTIVE",&port)
	    == KERN_SUCCESS) {
		break;
	}
	if (++count > 1000) {
	    printf ("Could not contact name_server\n");
	    exit(1);
	}
	sleep(3);
    }
	
	
    code = netname_check_in(name_server_port, "cpu_server", PORT_NULL,
		server_port);
    if (code != ERR_SUCCESS) {
	mach_error("netname_check_in", code);
	exit(1);
    }
    print_time();
    printf("main: checked in port as 'cpu_server'\n");
    fflush(stdout);

    server_loop();
}


/*
 *	server_loop() - read and process messages.
 */

struct	cpu_msg {
    msg_header_t	head;
    int			space[200];
} in_msg, out_msg;

server_loop()
{
    mach_error_t	retcode;
    boolean_t		ok, cpu_request_server();

    while (1) {
	in_msg.head.msg_size = sizeof(in_msg);
	in_msg.head.msg_local_port = server_port_set;

	retcode = msg_receive(&in_msg, MSG_OPTION_NONE, 0);
	if (retcode == RCV_SUCCESS) {
	    ok = cpu_request_server(&in_msg, &out_msg);
	    
	    /*
	     *	All actions have replies.
	     */
	    if (ok) {
		retcode = msg_send(&out_msg, MSG_OPTION_NONE, 0);
    
		if (retcode != SEND_SUCCESS) {
		    printf("server_loop: error %s at msg_send.\n",
			mach_error_string(retcode));
		}
	    }
	    else {
		printf("server_loop: server didn't recognize message.\n");
		fflush(stdout);
	    }
	}
	else {
		printf("server_loop: error %s at msg_receive.\n",
			mach_error_string(retcode));
		fflush(stdout);
	}
    }
}


/*
 *	Utility routine to put timestamp on log file.
 *	Caller must hold log file lock.
 */
print_time()
{
	struct timeval	tv;
	struct timezone tz;
	char	*timestring, *scan;

	gettimeofday(&tv, &tz);

	timestring = ctime(&tv.tv_sec);

	scan = timestring;

	while(*scan != '\n')
		scan++;

	*scan = ' ';

	printf("%s", timestring);
}
