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
 * $Log:	ux_user.h,v $
 * Revision 2.5  94/03/25  18:23:19  mrt
 * 	Added comments.
 * 	[94/03/25            mrt]
 * 
 * Revision 2.4  91/12/19  20:30:19  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/12/04  21:55:54  rpd
 * 	Added ut_init_process.
 * 	[90/12/04            rpd]
 * 	Added ut_comm.
 * 	[90/11/25            rpd]
 * 
 * Revision 2.2  90/09/08  00:22:28  rwd
 * 	Added hasstatus_condition.
 * 	[90/09/06            rwd]
 * 	First checkin
 * 	[90/08/31  14:04:15  rwd]
 * 
 */
/*
 *	File:	./ux_user.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <ux_param.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <signal.h>
#include <cthreads.h>

#define	UTCOMMLEN	24

struct ux_task {
/*
 *	Mach abstractions
 */
	mach_port_t	ut_server;	/* task's bootstrap port, POE has receive rights */
	task_t		ut_task;	/* kernel port for the task */
	thread_t	ut_thread;	/* thread port for first thread */	
	struct ux_task	*ut_prev;
	struct ux_task	*ut_next;
/*
 *	For debugging
 */
	char		ut_comm[UTCOMMLEN];	/* null-terminated */
/*
 *	Protection / privileges
 */
	int		ut_ngids;
	int		ut_gids[NGIDS];
	int		ut_rgid;
	int		ut_egid;
	int		ut_ruid;
	int		ut_euid;
/*
 *	File system info
 */
	struct fnode	*ut_cdir;
	struct fnode	*ut_rdir;
	struct fentry	*ut_fd[DTABLESIZE];
	int		ut_fdflags[DTABLESIZE];
	int		ut_umask;
/*
 *	Signals, process state, and status
 */
	boolean_t	ut_init_process;
	int		ut_sigmask;
	struct sigstack	ut_sigstack;
	struct sigvec	ut_sigvec[NSIG];
	int		ut_sigpend;
	int		ut_sigtake;
	boolean_t	ut_stopped;
	boolean_t	ut_dead;
	boolean_t	ut_hasstatus;
	boolean_t	ut_traced;
	union wait	ut_status;
	char *		ut_threadstate;
/*
 *	Process id and process group
 */
	int		ut_pid;
	int		ut_pgrp;
/*
 *	Process tree info
 */
	struct ux_task	*ut_parent;
	struct ux_task	*ut_child;
	struct ux_task	*ut_sibling;
/*
 *	Interval timer info
 */
	struct itimerval ut_ritimer;
	struct timeout	*ut_ritimeout;
/*
 *	Memory and other resources
 */
	vm_offset_t	ut_break;
/*
 *	Various conditions governed by master lock
 */
	struct condition ut_hasstatus_condition;
	struct condition ut_signal_condition;
};

extern struct ux_task	*create_ux_task();
extern struct ux_task	*lookup_ux_task();
extern int		lookup_ux_task_by_pid();
