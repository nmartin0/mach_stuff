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
 * $Log:	bsd_exit.c,v $
 * Revision 2.5  91/12/19  20:27:04  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  91/06/18  13:06:25  jjc
 * 	Eliminated race between Bsd_exit() and dead_task_server() both
 * 	setting the status and flagging ut_hasstatus by just setting
 * 	ut_hasstatus once in makedead() after the task has really died.
 * 	Changed Bsd_exit() to just set the status not ut_hasstatus.
 * 	Removed setting of ut_hasstatus and the status from dead_task_server().
 * 	This bug caused wait() to pass back different status for the same
 * 	child.
 * 	[91/05/16            jjc]
 * 
 * Revision 2.3  90/12/04  21:54:59  rpd
 * 	Removed reaper thread and reaper_condition.
 * 	Added dead_task_server.
 * 	Changed Bsd_exit to use task_terminate instead of makedead.
 * 	Changed makedead to assume the task is already terminated.
 * 	[90/12/04            rpd]
 * 
 * Revision 2.2  90/09/08  00:06:52  rwd
 * 	Remove sleeps in wait and reaper thread.
 * 	[90/09/06            rwd]
 * 	Wire repear thread.
 * 	[90/07/16            rwd]
 * 
 */
/*
 *	File:	./bsd_exit.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fnode.h>
#include <ux_user.h>

extern mach_port_t dead_task_notify;
extern struct mutex *master_lock;

/*
 *  Only place that ut_dead is set true.
 *  This ensures that ut_dead does mean terminated.
 *
 *  Called when the task is terminated, and we have
 *  received a dead-name notification for the task.
 *  Release resources associated with ut,
 *  including zombie children.
 */
makedead(ut)
	struct ux_task *ut;
{
	struct ux_task	*utc;
	struct ux_task	*utn;
	int		i;
	kern_return_t	error;

	ut->ut_dead = TRUE;
	/*
	 * Flag ut_hasstatus here, after the dead_task_server() has
	 * has received the death notification and called us.
	 */
	ut->ut_hasstatus = TRUE;
	for (utc = ut->ut_child; utc; utc = utn) {
		utn = utc->ut_sibling;
		if (utc->ut_dead) {
			ut_free(utc);
		} else {
			utc->ut_parent = 0;
			utc->ut_sibling = 0;
		}
	}
	for (i = 0; i < DTABLESIZE; i++) {
		if (ut->ut_fd[i]) {
			fe_decr(ut->ut_fd[i]);
		}
	}
	FOP_DECR(ut->ut_cdir);
	FOP_DECR(ut->ut_rdir);
	if (ut->ut_parent) {
		condition_signal(&ut->ut_parent->ut_hasstatus_condition);
		bsd_sendsig(ut->ut_parent, SIGCHLD);
	}
	error = mach_port_destroy(mach_task_self(), ut->ut_server);
	if (error)
		mach_error("makedead: port_destroy server", error);
	error = mach_port_deallocate(mach_task_self(), ut->ut_task);
	if (error)
		mach_error("makedead: port_deallocate task", error);
	error = mach_port_deallocate(mach_task_self(), ut->ut_thread);
	if (error)
		mach_error("makedead: port_deallocate thread", error);
	return 0;
}

Bsd_exit(ut, rval, status)
	struct ux_task *ut;
	int rval[2];
	int status;
{
	/*
	 * Just set status and let makedead() flag ut_hasstatus
	 * once we're really dead.
	 */
	ut->ut_status.w_retcode = status;
	ut->ut_status.w_coredump = 0;
	ut->ut_status.w_termsig = 0;
	task_terminate(ut->ut_task);
	return 0;
}

/*
 *  XXX we trash children of exited parents, instead of connecting
 *  XXX them to pid 1...
 */
Bsd_wait(ut, rval, options, rusage)
	struct ux_task *ut;
	int rval[2];
	int options;
	struct rusage *rusage;
{
	int error;
	struct rusage rusage0;
	struct ux_task **utp, *utc;

again:
	if (ut->ut_child == 0) {
		return ECHILD;
	}
	for (utp = &ut->ut_child; utc = *utp; utp = &utc->ut_sibling) {
		if (utc->ut_hasstatus) {
			utc->ut_hasstatus = FALSE;
			if ((options & WUNTRACED) == 0 && ! utc->ut_traced) {
				if (! utc->ut_dead) {
					continue; /* not interested */
				}
			}
			rval[1] = *((int *)&utc->ut_status);
			if (rusage) {
				/* XXX */
				bzero(&rusage0, sizeof(rusage0));
				copyout(ut, &rusage0, rusage, sizeof(rusage0));
			}
			rval[0] = utc->ut_pid;
			if (utc->ut_dead) {
				*utp = utc->ut_sibling;
				ut_free(utc);
			}
			return 0;
		}
	}
	if (options & WNOHANG) {
		rval[0] = 0;
		return 0;
	}
	server_cthread_busy();
	condition_wait(&ut->ut_hasstatus_condition, master_lock);
	server_cthread_active();
	goto again;
}

boolean_t
dead_task_server(request, reply)
	mach_msg_header_t *request;
	mach_msg_header_t *reply;
{
	mach_dead_name_notification_t *not;
	struct ux_task *ut;

	/* is this a dead-name notification for a dead task? */
	if (request->msgh_local_port != dead_task_notify)
		return FALSE;
	not = (mach_dead_name_notification_t *) request;

	if (!lookup_ux_task_by_task(not->not_port, &ut)) {
		/*
		 * Mark task as dead and flag that it has status.
		 * The status should have been set by exit().
		 * If not, it has been initialized in fork().
		 */
		makedead(ut);
	}

	/* free the dead-name reference from the notification */
	(void) mach_port_deallocate(mach_task_self(), not->not_port);

	/* no reply message */
	reply->msgh_remote_port = MACH_PORT_NULL;
	return TRUE;
}
