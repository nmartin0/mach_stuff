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
 * $Log:	bsd_signal.c,v $
 * Revision 2.5  91/12/19  20:27:48  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  90/12/04  21:55:30  rpd
 * 	Added bsd_bcastsig.
 * 	[90/12/04            rpd]
 * 
 * 	Removed reaper_condition.
 * 	Changed sig_dokill to use task_terminate instead of makedead.
 * 	[90/12/04            rpd]
 * 
 * Revision 2.3  90/10/01  14:05:59  rwd
 * 	Remove excess printfs.
 * 	[90/09/30            rwd]
 * 
 * Revision 2.2  90/09/08  00:15:49  rwd
 * 	Hasstatus change must notify reaper.  Added signal_condition.
 * 	[90/09/06            rwd]
 * 	First checkin
 * 	[90/08/31  13:34:01  rwd]
 * 
 */
/*
 *	File:	./bsd_signal.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <signal.h>
#include <ux_user.h>

#define	sigmask_boring	(sigmask(SIGURG)|sigmask(SIGCONT)|sigmask(SIGCHLD)|\
			 sigmask(SIGIO)|sigmask(SIGWINCH))

#define	sigmask_stop	(sigmask(SIGSTOP)|sigmask(SIGTSTP))

#define	sigmask_core	(sigmask(SIGQUIT)|sigmask(SIGILL)|sigmask(SIGTRAP)|\
			 sigmask(SIGIOT)|sigmask(SIGEMT)|sigmask(SIGFPE)|\
			 sigmask(SIGBUS)|sigmask(SIGSEGV)|sigmask(SIGSYS))

/*
 * From execve man page:
 *	Ignored signals remain ignored across an execve, but signals
 *	that are caught are reset to their default values.  Blocked
 *	signals remain blocked regardless of changes to the signal
 *	action.  The signal stack is reset to be undefined (see
 *	sigvec(2) for more information).
 */
sig_doexec(ut)
	struct ux_task *ut;
{
	int sig;
	struct sigvec *vec;

	for (sig = 1; sig < NSIG; sig++) {
		vec = &ut->ut_sigvec[sig];
		if (vec->sv_handler != SIG_IGN) {
			vec->sv_handler = SIG_DFL;
		}
	}
	ut->ut_sigstack.ss_sp = 0;
	ut->ut_sigstack.ss_onstack = 0;	/* XXX ??? */
}

static
sig_dohandler(ut, sig)
	struct ux_task *ut;
	int sig;
{
	dprintf("[sig_dohandler(%d,%d)]", ut->ut_pid, sig);
	if (ut->ut_sigtake == 0) {
		dprintf("setting sigtake!\n");
		ut->ut_sigtake = sig;
		condition_signal(&ut->ut_signal_condition);
	} else {
		dprintf("setting sigmask!\n");
		ut->ut_sigpend |= sigmask(sig);
		condition_signal(&ut->ut_signal_condition);
	}
}

static
sig_dostop(ut, sig)
	struct ux_task *ut;
	int sig;
{
	dprintf("[sig_dostop(%d,%d)]", ut->ut_pid, sig);
	if (ut->ut_stopped) {
		/* already stopped, status doesn't change... I think */
		return 0;
	}
	ut->ut_hasstatus = TRUE;
	ut->ut_status.w_stopval = WSTOPPED;
	ut->ut_status.w_stopsig = sig;
	ut->ut_stopped = TRUE;
	task_suspend(ut->ut_task);
	if (ut->ut_parent) {
		condition_signal(&ut->ut_parent->ut_hasstatus_condition);
		/* XXX ... && parent interested? */
		bsd_sendsig(ut->ut_parent, SIGCHLD);
	}
	return 0;
}

sig_docont(ut, sig)
	struct ux_task *ut;
	int sig;
{
	dprintf("[sig_docont(%d,%d)]", ut->ut_pid, sig);
	if (ut->ut_threadstate) {
		free(ut->ut_threadstate);
		ut->ut_threadstate = 0;
	}
	if (ut->ut_stopped) {
		task_resume(ut->ut_task);
		ut->ut_stopped = FALSE;
	}
	return 0;
}

sig_dokill(ut, sig)
	struct ux_task *ut;
	int sig;
{
	dprintf("[sig_dokill(%d,%d)]", ut->ut_pid, sig);
	ut->ut_hasstatus = TRUE;
	ut->ut_status.w_retcode = 0;
	ut->ut_status.w_coredump = 0;	/* XXX */
	ut->ut_status.w_termsig = sig;
	task_terminate(ut->ut_task);
}

bsd_sendsig(ut, sig)
	struct ux_task *ut;
	int sig;
{
	dprintf("[bsd_sendsig(%d,%d)]", ut->ut_pid, sig);
	if (sig <= 0 || sig >= NSIG) {
dprintf("[sendsig:INVALID]\n");
		return EINVAL;
	}
	if (ut->ut_dead) {
dprintf("[sendsig:DEAD]\n");
		return ESRCH;
	}
	if (ut->ut_traced) {
		sig_dostop(ut, sig);
		return 0;
	}
	if (ut->ut_sigmask & sigmask(sig)) {
dprintf("[sendsig:MASKED]\n");
		ut->ut_sigpend |= sigmask(sig);
		condition_signal(&ut->ut_signal_condition);
		return 0;
	}
	if (sig == SIGCONT) {
		sig_docont(ut, sig);
		/* fall through, for handler or IGN */
	}
dprintf("[sig=%d:handler=0x%x]", sig, ut->ut_sigvec[sig].sv_handler);
	if (ut->ut_sigvec[sig].sv_handler == SIG_IGN) {
		return 0;
	}
	if (ut->ut_sigvec[sig].sv_handler != SIG_DFL) {
		sig_dohandler(ut, sig);
		return 0;
	}
	if (sigmask(sig) & sigmask_boring) {
		return 0;
	}
	if (sigmask(sig) & sigmask_stop) {
		sig_dostop(ut, sig);
		return 0;
	}
#if 0
	/* XXX if we support core-dumping... */
	if (sigmask(sig) & sigmask_core) {
		sig_docore(ut, sig);
	}
#endif
	sig_dokill(ut, sig);
	return 0;
}

bsd_sigstack(ut, rval, ss, oss)
	struct ux_task *ut;
	int rval[2];
	struct sigstack *ss, *oss;
{
	if (oss) {
		bcopy(&ut->ut_sigstack, oss, sizeof(*oss));
	}
	if (ss) {
		bcopy(ss, &ut->ut_sigstack, sizeof(*ss));
	}
	return 0;
}

bsd_bcastsig_iter(victim, ut, sig, errorp)
	struct ux_task *victim;
	struct ux_task *ut;
	int sig;
	int *errorp;
{
	int error;

	if ((victim != ut) &&
	    ((ut->ut_euid == 0) ?
	     !victim->ut_init_process :
	     (victim->ut_euid == ut->ut_euid))) {
		error = bsd_sendsig(victim, sig);
		if (error == 0)
			*errorp = 0;
	}
}

bsd_bcastsig(ut, sig)
	struct ux_task *ut;
	int sig;
{
	int error = ESRCH;

	foreach_ux_user(bsd_bcastsig_iter, ut, sig, &error);
	return error;
}

Bsd_kill(ut, rval, pid, sig)
	struct ux_task *ut;
	int rval[2];
	int pid;
	int sig;
{
	int error;

	if (pid == 0) {
		if (ut->ut_pgrp == 0) {
			return ESRCH;
		}
		return Bsd_killpg(ut, rval, ut->ut_pgrp, sig);
	}
	if (pid == -1) {
		return bsd_bcastsig(ut, sig);
	}
	if (pid < 0) {
		return Bsd_killpg(ut, rval, -pid, sig);
	}
	error = lookup_ux_task_by_pid(pid, &ut);
	if (error) {
		return error;
	}
	return bsd_sendsig(ut, sig);
}

int
findsig(mask)
	int mask;
{
	int sig;

	for (sig = 1; sig < NSIG; sig++) {
		if (sigmask(sig) & mask) {
			return sig;
		}
	}
	return 0;
}

Bsd_sigsetmask(ut, rval, mask)
	struct ux_task *ut;
	int rval[2];
	int mask;
{
	int sig;

	mask &= ~(SIGKILL | SIGSTOP | SIGCONT);
	rval[0] = ut->ut_sigmask;
	ut->ut_sigmask = mask;
	sig = findsig(ut->ut_sigpend & ~ut->ut_sigmask);
	if (sig) {
		ut->ut_sigpend &= ~sigmask(sig);
		ut->ut_sigtake = sig;
		condition_signal(&ut->ut_signal_condition);
	}
	return 0;
}

Bsd_sigblock(ut, rval, mask)
	struct ux_task *ut;
	int rval[2];
	int mask;
{
	mask &= ~(SIGKILL | SIGSTOP | SIGCONT);
	rval[0] = ut->ut_sigmask;
	ut->ut_sigmask |= mask;
	return 0;
}

Bsd_sigpause(ut, rval, mask)
	struct ux_task *ut;
	int rval[2];
	int mask;
{
	extern struct mutex *master_lock;
	int savemask;
	int sig;

	savemask = ut->ut_sigmask;
	ut->ut_sigmask = mask;
	for (;;) {
		if (ut->ut_sigtake) {
			ut->ut_sigmask = savemask;
			return EINTR;
		}
		sig = findsig(ut->ut_sigpend & ~ut->ut_sigmask);
		if (sig) {
			break;
		}
		server_cthread_busy();
		condition_wait(&ut->ut_signal_condition, master_lock);
		server_cthread_active();
	}
	ut->ut_sigpend &= ~sigmask(sig);
	ut->ut_sigtake = sig;
	condition_signal(&ut->ut_signal_condition);
huh:
	ut->ut_sigmask = savemask;
	return EINTR;
}

bsd_sigvec(ut, rval, sig, vec, ovec)
	struct ux_task *ut;
	int rval[2];
	int sig;
	struct sigvec *vec, *ovec;
{
	int error;

	if (sig < 1 || sig >= NSIG) {
		return EINVAL;
	}
	if (ovec) {
		bcopy(&ut->ut_sigvec[sig], ovec, sizeof(*ovec));
	}
	if (vec) {
		if (sig == SIGKILL || sig == SIGSTOP) {
			if (vec->sv_handler != SIG_DFL) {
				return EINVAL;
			}
		}
		if (sig == SIGCONT && vec->sv_handler == SIG_IGN) {
			return EINVAL;
		}
		bcopy(vec, &ut->ut_sigvec[sig], sizeof(*vec));
	}
	return 0;
}

Bsd1_sigvec(proc_port, interrupt, sig, have_nsv, nsv, osv)
	mach_port_t proc_port;
	boolean_t *interrupt;
	int sig;
	int have_nsv;
	struct sigvec nsv;
	struct sigvec *osv;
{
	struct ux_task *ut;
	int error;

	error = ut_lookup("bsd_sigvec", proc_port, &ut, interrupt);
	if (error) {
		return error;
	}
	if (have_nsv) {
		dprintf("(sig=%d, {hdl=0x%x,msk=0x%x,flg=0x%x}, osv=0x%x)\n",
			sig, nsv.sv_handler, nsv.sv_mask, nsv.sv_flags, osv);
	} else {
		dprintf("(sig=%d, osv=0x%x)\n", sig, osv);
	}
	if (sig < 1 || sig >= NSIG) {
		return EINVAL;
	}
	*osv = ut->ut_sigvec[sig];
	if (have_nsv) {
		if (sig == SIGKILL || sig == SIGSTOP) {
			if (nsv.sv_handler != SIG_DFL) {
				return EINVAL;
			}
		}
		if (sig == SIGCONT && nsv.sv_handler == SIG_IGN) {
			return EINVAL;
		}
		ut->ut_sigvec[sig] = nsv;
	}
	return 0;
}

Bsd_setpgrp(ut, rval, pid, pgrp)
	struct ux_task *ut;
	int rval[2];
	int pid;
	int pgrp;
{
	int error;

	if (pid != 0) {
		error = lookup_ux_task_by_pid(pid, &ut);
		if (error) {
			return error;
		}
	}
	ut->ut_pgrp = pgrp; /* should validate as pid, etc. */
	return 0;
}

Bsd_getpgrp(ut, rval, pid)
	struct ux_task *ut;
	int rval[2];
	int pid;
{
	rval[0] = ut->ut_pgrp;
	return 0;
}

bsd_killpg_iter(ut, pgrp, sig, foundone)
	struct ux_task *ut;
	int pgrp;
	int sig;
	int *foundone;
{
	int error;

	if (ut->ut_pgrp != pgrp) {
		return;
	}
	error = bsd_sendsig(ut, sig);
	if (error == 0) {
		*foundone = 1;
	}
}

bsd_killpg(pgrp, sig)
	int pgrp;
	int sig;
{
	int foundone = 0;

	foreach_ux_user(bsd_killpg_iter, pgrp, sig, &foundone);
	if (! foundone) {
		return ESRCH;
	}
	return 0;
}

Bsd_killpg(ut, rval, pgrp, sig)
	struct ux_task *ut;
	int rval[2];
	int pgrp;
	int sig;
{
	if (sig <= 0 || sig >= NSIG) {
		return EINVAL;
	}
	return bsd_killpg(pgrp, sig);
}

Bsd1_take_signal(proc_port, interrupt, old_mask, old_onstack, sig, code,
		handler, new_sp)
	mach_port_t proc_port;
	boolean_t *interrupt;
	int *old_mask;
	int *old_onstack;
	int *sig;
	int *code;
	int *handler;
	int *new_sp;
{
	struct ux_task *ut;
	int error;

	*interrupt = FALSE; /* XXX ? */
	error = ut_lookup("bsd_take_signal", proc_port, &ut, 0);
	if (error) {
		return error;
	}
	dprintf("()\n");
	if (ut->ut_sigtake == 0) {
		*sig = 0;
		return 0;
	}
	*sig = ut->ut_sigtake;
	*old_mask = ut->ut_sigmask;
	ut->ut_sigmask |= (ut->ut_sigvec[*sig].sv_mask | sigmask(*sig));
	*old_onstack = FALSE;	/* XXX */
	*new_sp = 0;		/* XXX */
	*code = 0;		/* XXX */
	ut->ut_sigtake = 0;	/* XXX should find next, if one exists */
	*handler = (int) ut->ut_sigvec[*sig].sv_handler;
	return 0;
}

Bsd1_sigreturn(proc_port, interrupt, old_on_stack, old_sigmask)
	mach_port_t proc_port;
	boolean_t *interrupt;
	int *old_on_stack;
	int *old_sigmask;
{
	struct ux_task *ut;
	int error;

	*interrupt = FALSE;
	error = ut_lookup("bsd_sigreturn", proc_port, &ut, 0);
	if (error) {
		return error;
	}
	dprintf("()\n");
	return 0;
}
