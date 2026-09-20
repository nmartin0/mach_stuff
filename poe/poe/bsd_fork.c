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
 * $Log:	bsd_fork.c,v $
 * Revision 2.9  94/03/25  18:21:48  mrt
 * 	Changed emulator_name to emulator_path and init_name to init_path.
 * 	These may be changed by main as the result of input args.
 * 	[94/03/14            mrt]
 * 
 * Revision 2.8  92/02/02  13:01:37  rpd
 * 	Removed old IPC vestiges.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.7  91/12/19  20:27:14  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.6  91/06/18  13:06:29  jjc
 * 	Initialized ut_status in spawn_ux_task().
 * 	[91/05/16            jjc]
 * 
 * Revision 2.5  91/03/09  14:32:04  rpd
 * 	Added exception_port.
 * 	[91/01/12            rpd]
 * 
 * Revision 2.4  90/12/04  21:55:07  rpd
 * 	Added ut_init_process.
 * 	[90/12/04            rpd]
 * 
 * 	Changed ux_task_create to request a dead-name notification.
 * 	[90/12/04            rpd]
 * 	Initialize ut_comm in ux_task_create.
 * 	[90/11/25            rpd]
 * 
 * 	Added lookup_ux_task_by_task.
 * 	[90/11/24            rpd]
 * 
 * Revision 2.3  90/09/27  13:53:52  rwd
 * 	Must copy emulator to writeable string for bsd_lookup.
 * 	[90/09/08            rwd]
 * 
 * Revision 2.2  90/09/08  00:07:07  rwd
 * 	Pass rootdevice argument to init program.
 * 	[90/09/05            rwd]
 * 	Use port_allocate_name.
 * 	[90/09/03            rwd]
 * 	First checkin
 * 	[90/08/31  14:16:23  rwd]
 * 
 */
/*
 *	File:	./bsd_fork.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <fnode.h>
#include <fentry.h>
#include <ux_user.h>
#include <sys/file.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <loader_info.h>
#include <sys/reboot.h>

extern mach_port_t server_port_set;
extern mach_port_t dead_task_notify;
extern mach_port_t exception_port;

struct ux_task *ut_first = 0;
int next_pid = 1; /* too many assumptions about pid 0 */

char emulator_path[128]	= "/mach_servers/poe_emulator";  /* may be changed */
char emulator_name[]    = "/poe_emulator";
char init_path[128]	= "/mach_servers/poe_init"; /* may be changed */
char init_name[]	= "/poe_init";

extern int boothowto;
extern char root_name[];

ut_free(ut)
	struct ux_task *ut;
{
	if (ut->ut_next) {
		ut->ut_next->ut_prev = ut->ut_prev;
	}
	if (ut->ut_prev) {
		ut->ut_prev->ut_next = ut->ut_next;
	} else {
		ut_first = ut->ut_next;
	}
	free(ut);
}

struct ux_task *
lookup_ux_task(server_port)
	mach_port_t server_port;
{
	struct ux_task *ut;

#if 0
	for (ut = ut_first; ut; ut = ut->ut_next) {
		if (ut->ut_server == server_port) {
			return ut;
		}
	}
	return 0;
#else 0
	return (struct ux_task *)server_port;
#endif 0
}

int
lookup_ux_task_by_pid(pid, utp)
	int pid;
	struct ux_task **utp;
{
	struct ux_task *ut;

	for (ut = ut_first; ut; ut = ut->ut_next) {
		if (ut->ut_pid == pid) {
			*utp = ut;
			return 0;
		}
	}
printf("AAA.1\n");
	return ESRCH;
}

int
lookup_ux_task_by_task(task, utp)
	mach_port_t task;
	struct ux_task **utp;
{
	struct ux_task *ut;

	for (ut = ut_first; ut; ut = ut->ut_next) {
		if (ut->ut_task == task) {
			*utp = ut;
			return 0;
		}
	}

	return ESRCH;
}

foreach_ux_user(func, a1, a2, a3, a4, a5, a6)
	int (*func)();
	int a1, a2, a3, a4, a5, a6;
{
	struct ux_task *ut;

	for (ut = ut_first; ut; ut = ut->ut_next) {
		(*func)(ut, a1, a2, a3, a4, a5, a6);
	}
}

/*
 *	creates a mach task with one thread and create/sets the
 *	new tasks exception_port, the notification_port, and bootstrap port.
 *	ut_parent is set when it is called
 *	from fork_ux_task, not set when called from spawn_ux_task.
 */

ux_task_create(ut)
	struct ux_task *ut;
{
	mach_port_t previous;
	kern_return_t error;

	ut->ut_task = MACH_PORT_NULL;
	ut->ut_thread = MACH_PORT_NULL;
	ut->ut_server = MACH_PORT_NULL;
	if (ut->ut_parent) {
		bcopy(ut->ut_parent->ut_comm, ut->ut_comm, sizeof ut->ut_comm);
		error = task_create(ut->ut_parent->ut_task, TRUE,&ut->ut_task);
		if (error) {
			mach_error("ux_task_create: task_create", error);
			goto bad;
		}
		/*
		 *	The task will inherit an exception port.
		 */
	} else {
		bzero(ut->ut_comm, sizeof ut->ut_comm);
		error = task_create(mach_task_self(), FALSE, &ut->ut_task);
		if (error) {
			mach_error("ux_task_create: task_create", error);
			goto bad;
		}
		error = task_set_exception_port(ut->ut_task, exception_port);
		if (error) {
			mach_error("ux_task_create: exception_port", error);
			goto bad;
		}
	}
	error = mach_port_request_notification(mach_task_self(), ut->ut_task,
					       MACH_NOTIFY_DEAD_NAME, FALSE,
					       dead_task_notify,
					       MACH_MSG_TYPE_MAKE_SEND_ONCE,
					       &previous);
	if (error || previous) {
		mach_error("request_notification", error);
		goto bad;
	}
	error = thread_create(ut->ut_task, &ut->ut_thread);
	if (error) {
		mach_error("thread_create", error);
		goto bad;
	}

	/* allocate the port by which POE will identify this task, ut.ut_server */

	error = mach_port_allocate_name(mach_task_self(),
					MACH_PORT_RIGHT_RECEIVE,
					(mach_port_t)ut);
	if (error) {
		mach_error("ux_task_create: port_allocate", error);
		goto bad;
	}
	ut->ut_server = (mach_port_t)ut;
	error = mach_port_insert_right(mach_task_self(), ut->ut_server,
				       ut->ut_server, MACH_MSG_TYPE_MAKE_SEND);
	if (error) {
		mach_error("ux_task_create: mach_port_insert_right", error);
		goto bad;
	}

	/* Messages sent by the task to its bootstrap port will go to POE */

	error = task_set_bootstrap_port(ut->ut_task, ut->ut_server);
	if (error) {
		mach_error("ux_task_create: task_set_bootstrap_port", error);
		goto bad;
	}
	/* move into POE's server_port_set */
	error = mach_port_move_member(mach_task_self(),
				      ut->ut_server, server_port_set);
	if (error) {
		mach_error("ux_task_create: port_move_member", error);
		goto bad;
	}
	
	ut->ut_pid = next_pid++;   /* new task gets next avaiable pid */
	ut->ut_next = ut_first;
	ut->ut_prev = 0;
	if (ut_first) {
		ut_first->ut_prev = ut;
	}
	ut_first = ut;
	return 0;
bad:
	if (ut->ut_server) {
		mach_port_deallocate(mach_task_self(), ut->ut_server);
	}
	if (ut->ut_thread) {
		thread_terminate(ut->ut_thread);
	}
	if (ut->ut_task) {
		task_terminate(ut->ut_task);
	}
	return EAGAIN;
}
/*
 * spawn_ux_task 
 *	allocs and inits a struct ux_task
 *	called only for the initial process, by spawn
 */
spawn_ux_task(utp)
	struct ux_task **utp;
{
	extern struct fnode *root_fn;
	int fd, error;
	struct ux_task *ut;
	extern int mig_loop();

	ut = (struct ux_task *) malloc(sizeof(*ut));
	bzero(ut, sizeof(*ut));

	/* create a new task with no parent */
	error = ux_task_create(ut);
	if (error) {
		free(ut);
		return error;
	}

	/* protection stuff */
	ut->ut_ngids = 1;
	ut->ut_gids[0] = 1;
	ut->ut_ruid = 0;
	ut->ut_euid = 0;
	ut->ut_rgid = 1;
	ut->ut_egid = 1;

	/* file system stuff */
	bzero(ut->ut_fd, sizeof(ut->ut_fd));
	ut->ut_cdir = root_fn;
	ut->ut_rdir = root_fn;
	FOP_INCR(ut->ut_cdir);
	FOP_INCR(ut->ut_rdir);
	ut->ut_umask = 022;

	/* signal stuff */
	ut->ut_init_process = FALSE; /* changed by spawn ??? */
	ut->ut_sigmask = 0;
	ut->ut_sigstack.ss_sp = 0;
	ut->ut_sigstack.ss_onstack = 0;
	bzero(ut->ut_sigvec, NSIG * sizeof(struct sigvec));

	/* pgrp stuff */
	ut->ut_pgrp = ut->ut_pid; /* ??? */
	ut->ut_status.w_status = 0;
	condition_init(&ut->ut_hasstatus_condition);
	condition_init(&ut->ut_signal_condition);

	*utp = ut;
	return 0;
}

/* spawn  - called only by main.c to start the init process
 */
spawn()
{	
	int error;
	struct ux_task *ut;
	struct loader_info li;
	vm_offset_t arg_addr;
	struct fnode *fn;
	vm_offset_t stack_start = STACK_END - STACK_SIZE;
	char arg_buf[512];
	char *cp;
	int len;
	char def_arg[3];
	int n_arg;
	char *tmp_str;
	int tmp_len;

	/* create and intialize the struct ux_task */
	error = spawn_ux_task(&ut);
	if (error) {
		printf("spawn: %d\n", error);
		return error;
	}
	/* make init a special init process */
	ut->ut_init_process = TRUE;

	/* look up the emulator file */
	tmp_len = strlen(emulator_path) + 1;
	tmp_str = (char *)malloc(tmp_len);
	bcopy(emulator_path, tmp_str, tmp_len);
	error = bsd_lookup(ut->ut_cdir, tmp_str, &fn, TRUE);
	if (error) {
		printf("spawn:bsd_lookup\n");
		return error;
	}
	error = ex_get_header(fn, &li);
	if (error) {
		printf("spawn:ex_get_header\n");
		return error;
	}
	error = machine_adjust_for_emulator(&li);
	if (error) {
		return error;
	}

	error = load_program_file(ut, fn, &li);
	if (error) {
		return error;
	}
	error = vm_allocate(ut->ut_task, &stack_start, STACK_SIZE, FALSE);
	if (error) {
		return error;
	}
	/* argv[0] name of init program */
	cp = arg_buf;
	len = strlen(init_path) + 1;
	bcopy(init_path, cp, len);
	cp += len;

	/* argv[1]  -s if bootflags -s was set */
	def_arg[0]='x';
	def_arg[1]='x';
	def_arg[2]=0;
	if ( (boothowto & RB_SINGLE) == RB_SINGLE ) {
		def_arg[0]='-';
		def_arg[1]='s';
	}
	bcopy(def_arg, cp, 3);
	cp += 3;

	/* argv[2] name of root partition */
	len = strlen(root_name) + 1;
	bcopy(root_name, cp, len);
	cp += len;
	n_arg = 3;
 
	error = load_program_args(ut->ut_task, n_arg, &arg_addr, arg_buf,
				  cp - arg_buf, 0);
	if (error) {
		return error;
	}

	xx_thread_set_state(ut->ut_thread, &li, arg_addr);
	error = thread_resume(ut->ut_thread);
	if (error) {
		mach_error("thread_resume", error);
		return error;
	}
	return 0;
}
/*
 *	called by Bsd1_fork, intialized the ut structure
 */
fork_ux_task(ut, utcp, new_state, new_state_size)
	struct ux_task *ut;  	/* parent task */
	struct ux_task **utcp;  /* child task, returned */
	thread_state_t new_state;
	int new_state_size;
{
	struct ux_task *utc;
	int fd, error;

	utc = (struct ux_task *) malloc(sizeof(*utc));
	bcopy(ut, utc, sizeof(*ut));
	utc->ut_parent = ut;
	error = ux_task_create(utc);
	if (error) {
		free(utc);
		return error;
	}
	for (fd = 0; fd < DTABLESIZE; fd++) {
		if (utc->ut_fd[fd]) {
			fe_incr(utc->ut_fd[fd]);
		}
	}
	FOP_INCR(utc->ut_cdir);
	FOP_INCR(utc->ut_rdir);
	/* XXX sig stuff changes, too? */
	utc->ut_child = 0;
	utc->ut_sibling = ut->ut_child;
	ut->ut_child = utc;

	error = thread_dup(utc->ut_thread, new_state, new_state_size,
			   ut->ut_pid, 1);
	if (error) {
		mach_error("fork_ux_task: thread_dup", error);
		return EAGAIN;
	}
	error = thread_resume(utc->ut_thread);
	if (error) {
		mach_error("fork_ux_task: thread_resume", error);
		return EAGAIN;
	}
	*utcp = utc;
	return 0;
}

/* called as a result of a fork syscall by user */
Bsd1_fork(proc_port, interrupt, new_state, new_state_size, child_pid)
	mach_port_t proc_port;
	boolean_t *interrupt;
	thread_state_t new_state;
	int new_state_size;
	int *child_pid;
{
	struct ux_task *ut, *utc;
	int error;

	error = ut_lookup("bsd_fork", proc_port, &ut, interrupt);
	if (error) {
		return error;
	}
	dprintf("()\n");
	error = fork_ux_task(ut, &utc, new_state, new_state_size);
	if (error) {
		return error;
	}
	*child_pid = utc->ut_pid;
	return 0;
}
