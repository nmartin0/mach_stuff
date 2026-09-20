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
 * $Log:	server_loop.c,v $
 * Revision 2.6  92/02/02  13:02:37  rpd
 * 	Fixed cthread_fork argument types.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.5  91/12/19  20:29:04  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  91/03/09  14:32:26  rpd
 * 	Added exception_port.
 * 	[91/01/12            rpd]
 * 
 * Revision 2.3  90/12/04  21:55:48  rpd
 * 	Added dead_task_server for dead-name notifications.
 * 	[90/12/04            rpd]
 * 
 * Revision 2.2  90/09/08  00:20:05  rwd
 * 	Use cthread_data fot ut.
 * 	[90/09/06            rwd]
 * 	Convert to new cthread semantics.
 * 	[90/08/16            rwd]
 * 
 */
/*
 *	File:	./server_loop.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <sys/types.h>
#include <sys/time.h>
#include <mach.h>
#include <mach/message.h>
#include <mig_errors.h>
#include <ux_user.h>

boolean_t	dead_task_server();

struct mutex	*master_lock;
mach_port_t	server_port_set = MACH_PORT_NULL;
mach_port_t	dead_task_notify = MACH_PORT_NULL;
mach_port_t	exception_port = MACH_PORT_NULL;

struct ux_task *
ut_self()
{
	return (struct ux_task *) cthread_data(cthread_self());
}


bsd_master_init()
{
	master_lock = (struct mutex *) mutex_alloc();
	mutex_init(master_lock);
}

bsd_master(ut)
	struct ux_task *ut;
{
	mutex_lock(master_lock);
	cthread_set_data(cthread_self(), (any_t)ut);
}

bsd_release()
{
	mutex_unlock(master_lock);
}

int		server_kthread_min		= 2;
int		server_kthread_max		= 4;
int		server_cthread_min		= 2;
int		server_cthread_current		= 0;
struct mutex	server_cthread_mutex		= MUTEX_INITIALIZER;
extern int	server_loop();

int server_init()
{
	kern_return_t error;

	error = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
				   &server_port_set);
	if (error) {
		mach_error("server_init: port_allocate", error);
		panic("server_init");
	}

	error = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
				   &dead_task_notify);
	if (error) {
		mach_error("server_init: port_allocate", error);
		panic("server_init");
	}

	error = mach_port_move_member(mach_task_self(),
				      dead_task_notify, server_port_set);
	if (error) {
		mach_error("server_init: move_member", error);
		panic("server_init");
	}

	error = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
				   &exception_port);
	if (error) {
		mach_error("server_init: port_allocate", error);
		panic("server_init");
	}

	/* task_set_special_port doesn't allow MACH_MSG_TYPE_MAKE_SEND */

	error = mach_port_insert_right(mach_task_self(), exception_port,
				       exception_port,
				       MACH_MSG_TYPE_MAKE_SEND);
	if (error) {
		mach_error("server_init: insert_right", error);
		panic("server_init");
	}

	error = mach_port_move_member(mach_task_self(),
				      exception_port, server_port_set);
	if (error) {
		mach_error("server_init: move_member", error);
		panic("server_init");
	}

	cthread_set_kernel_limit(cthread_kernel_limit() + server_kthread_max);
	cthread_detach(cthread_fork((any_t (*)()) server_loop, (any_t) 0));
}

void
server_cthread_busy()
{
	cthread_msg_busy(server_port_set,
			 server_kthread_min,
			 server_kthread_max);
	mutex_lock(&server_cthread_mutex);
	if (--server_cthread_current < server_cthread_min) {
	    mutex_unlock(&server_cthread_mutex);
	    cthread_detach(cthread_fork((any_t (*)()) server_loop, (any_t) 0));
	} else {
	    mutex_unlock(&server_cthread_mutex);
	}
}

void
server_cthread_active()
{
	cthread_msg_active(server_port_set,
			   server_kthread_min,
			   server_kthread_max);
	mutex_lock(&server_cthread_mutex);
	++server_cthread_current;
	mutex_unlock(&server_cthread_mutex);
}

int
server_loop()
{
	struct ux_task *ut;

	kern_return_t ret;
	union request_msg {
	    mach_msg_header_t	hdr;
	    mig_reply_header_t	death_pill;
	    char		space[8192];
	} msg_buffer_1, msg_buffer_2;

	mach_msg_header_t * request_ptr;
	mig_reply_header_t * reply_ptr;
	mach_msg_header_t * tmp;

	char	name[64];

	sprintf(name, "server thread %x", cthread_self());
	cthread_set_name(cthread_self(), name);

	server_cthread_active();
	server_cthread_busy();
	server_cthread_active();

	request_ptr = &msg_buffer_1.hdr;
	reply_ptr = &msg_buffer_2.death_pill;


	do {
	    ret = cthread_mach_msg(request_ptr, MACH_RCV_MSG,
				   0, sizeof msg_buffer_1, server_port_set,
				   MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL,
				   server_kthread_min,
				   server_kthread_max);
	    if (ret != MACH_MSG_SUCCESS)
		panic("server_loop: receive", ret);
	    while (ret == MACH_MSG_SUCCESS) {
		ut = (struct ux_task *)request_ptr->msgh_local_port;
		bsd_master(ut);
		if (!bsd_1_server(request_ptr, &reply_ptr->Head))
		if (!ux_generic_server(ut, request_ptr, &reply_ptr->Head))
		if (!exc_server(request_ptr, &reply_ptr->Head))
		if (!dead_task_server(request_ptr, &reply_ptr->Head))
		{}

		bsd_release();

		if (reply_ptr->Head.msgh_remote_port == MACH_PORT_NULL) {
		    /* no reply port, just get another request */
		    break;
		}

		if (reply_ptr->RetCode == MIG_NO_REPLY) {
		    /* deallocate reply port right */
		    (void) mach_port_deallocate(mach_task_self(),
					reply_ptr->Head.msgh_remote_port);
		    break;
		}

		ret = cthread_mach_msg(&reply_ptr->Head,
				       MACH_SEND_MSG|MACH_RCV_MSG,
				       reply_ptr->Head.msgh_size,
				       sizeof msg_buffer_2,
				       server_port_set,
				       MACH_MSG_TIMEOUT_NONE,
				       MACH_PORT_NULL,
				       server_kthread_min,
				       server_kthread_max);
		if (ret != MACH_MSG_SUCCESS) {
		    if (ret == MACH_SEND_INVALID_DEST) {
			/* deallocate message */
#if 0
			mach_msg_destroy(reply_ptr);
#else 0
			(void) mach_port_destroy(mach_task_self(),
					reply_ptr->Head.msgh_remote_port);
#endif 0
		    } else
			panic("server_loop: rpc", ret);
		}

		tmp = request_ptr;
		request_ptr = (mach_msg_header_t *) reply_ptr;
		reply_ptr = (mig_reply_header_t *) tmp;
	    }

	} while (1);
}

server_sleep(a, b)
int a,b;
{
	struct ux_task *ut = ut_self();
	bsd_release();
	server_cthread_busy();
	tv_sleep(a,b);
	server_cthread_active();
	bsd_master(ut);
}
