/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	emul_machdep.c,v $
 * Revision 2.5  94/03/25  18:21:25  mrt
 * 	Corrected include of mach/mach.h to mach.h.
 * 	Added casts.
 * 	[93/11/17            mrt]
 * 
 * Revision 2.4  92/02/02  13:01:04  rpd
 * 	Removed MAP_UAREA code.
 * 	[92/01/30            rpd]
 * 
 * Revision 2.3  91/12/19  20:25:27  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:40:49  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:05:53  rwd]
 * 
 * Revision 2.5  90/10/25  15:06:38  rwd
 * 	Initialize eflags on exec instead of letting it get a random
 * 	value off the stack.
 * 	[90/09/19            rwd]
 * 
 * Revision 2.4  90/08/06  15:30:39  rwd
 * 	Added pid_by_task.
 * 	[90/07/31            rwd]
 * 	Include sys/types.h
 * 	[90/07/17            rwd]
 * 
 * Revision 2.3  90/06/02  14:36:14  rpd
 * 	Converted to new IPC.
 * 	[90/06/02            rpd]
 * 
 * Revision 2.2  90/05/21  13:46:11  dbg
 * 	i386 version.
 * 	[90/04/23            dbg]
 * 
 * Revision 2.3  89/10/17  11:24:11  rwd
 * 	Added syscall init_process(-41).  Mach_Init needs this.
 * 	[89/09/29            rwd]
 * 
 * 	Removed reference to ERROR
 * 	[89/09/26            rwd]
 * 
 * Revision 2.2.1.1  89/09/21  20:35:37  dbg
 * 	Add interrupt return parameter to all calls.
 * 	[89/09/21            dbg]
 * 
 * Revision 2.2  89/08/31  16:28:19  rwd
 * 	No special case for syscall needed.  Fix up user stack whenever
 * 	ERESTART occurs.
 * 	[89/08/24            rwd]
 * 	Changed to reflect change in trampline code that pushes syscall
 * 	# on stack.
 * 	[89/08/23            rwd]
 * 
 * 	Make ERESTART code work and special case for syscall
 * 	[89/08/21            rwd]
 * 
 * Revision 2.1  89/08/04  14:04:49  rwd
 * Created.
 * 
 * 22-Jun-89  Randall Dean (rwd) at Carnegie-Mellon University
 *	Added copyright and history to file by dbg.  Fixed register
 *	format.
 *
 */

/*
 * Take/Return from signals - machine-dependent.
 */
#include <mach.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <machine/eflags.h>
#include <machine/vmparam.h>
#include <bsd_msg.h>	/* error code definitions */

#include "syscall_table.h"

/* Mach micro-kernel knows about SYS_sigvec */
#ifndef SYS_sigvec
#define SYS_sigvec	108
#endif

extern mach_port_t	our_bsd_server_port;

vm_offset_t	sigreturnaddr;	/* signal trampoline, set by sigvec */

#define	E_JUSTRETURN	255	/* return without changing registers */

/*
 * User's registers are stored on partially on the user's stack
 * and partially on the emulator's stack.
 */
struct emul_regs_2 {
	int	edx;
	int	ecx;
	int	eax;
	int	eflags;
	int	eip;
};

struct emul_regs {
	int	ebp;
	int	edi;
	int	esi;
	int	ebx;
	struct emul_regs_2 *uesp;
				/* pointer to rest of user registers */
};

void	take_signals();	/* forward */

/*
 * Change system call table for sigvec to point to
 * our own routine.
 */
int	i386_sigvec();	/* forward */
struct sysent i386_syscall_sigvec =
	{ E_CHANGE_REGS, i386_sigvec };

int	emul_low_entry = -9;
int	emul_high_entry = 181;

extern emul_common();

void
emul_setup(task)
	task_t	task;
{
	register int i;
	register kern_return_t	rc;

	for (i = emul_low_entry;
	     i <= emul_high_entry;
	     i++) {
		rc = task_set_emulation(task,
					(vm_address_t) emul_common,
					i);
	}
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-33);
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-34);
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-41);
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-52);

	/*
	 * Change sigvec to point to our routine - the signal
	 * trampoline address is passed in VERY strangely.
	 */
	sysent[SYS_sigvec] = i386_syscall_sigvec;
}

/*
 * System calls enter here.
 */
void
emul_syscall(regs)
	register struct emul_regs *regs;
{
	register int	syscode;
	register int	error;
	register struct sysent *callp;
	int		rval[2];
	boolean_t	interrupt = FALSE;
	register struct emul_regs_2 *regs2,*regs2_mv;
	register int	*args;

	regs2 = regs->uesp;
	args = (int *)(regs2+1);	/* args on stack top */
	args++;	/* point to first argument - skip return address */

	syscode = regs2->eax;

	if (syscode == 0) {
	    /*
	     * Indirect system call.
	     */
	    syscode = *args++;
	}

	/*
	 * Find system call table entry for the system call.
	 */
	if (syscode >= nsysent)
	    callp = &sysent[63];	/* nosysent */
	else if (syscode >= 0)
	    callp = &sysent[syscode];
	else {
	    /*
	     * Negative system call numbers are CMU extensions.
	     */
	    if (syscode == -33)
		callp = &sysent_task_by_pid;
	    else if (syscode == -34)
		callp = &sysent_pid_by_task;
	    else if (syscode == -41)
		callp = &sysent_init_process;
	    else if (syscode == -59)
		callp = &sysent_htg_ux_syscall;
	    else if (syscode < -ncmusysent)
		callp = &sysent[63];	/* nosysent */
	    else
		callp = &cmusysent[-syscode];
	}

	/*
	 * Set up the initial return values.
	 */
	rval[0] = 0;
	rval[1] = regs2->edx;

	/*
	 * Call the routine, passing arguments according to the table
	 * entry.
	 */
	switch (callp->nargs) {
	    case 0:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				rval);
		break;
	    case 1:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0],
				rval);
		break;
	    case 2:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1],
				rval);
		break;
	    case 3:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2],
				rval);
		break;
	    case 4:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2], args[3],
				rval);
		break;
	    case 5:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2], args[3], args[4],
				rval);
		break;
	    case 6:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2],
				args[3], args[4], args[5],
				rval);
		break;

	    case -1:	/* generic */
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				syscode,
				args,
				rval);
		break;

	    case -2:	/* pass registers to modify */
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args,
				rval,
				regs);
		regs2 = regs->uesp;	/* if changed */
		break;
	}

	/*
	 * Set up return values.
	 */

	switch (error) {
	    case E_JUSTRETURN:
		/* Do not alter registers */
		break;

	    case 0:
		/* Success */
		regs2->eflags &= ~EFL_CF;
		regs2->eax = rval[0];
		regs2->edx = rval[1];
		break;

	    case ERESTART:
		/* restart call */
		regs2->eip -= 7;
		break;

	    default:
		/* error */
		regs2->eflags |= EFL_CF;
		regs2->eax = error;
		break;
	}

	/*
	 * Handle interrupt request
	 */
	if (error == ERESTART || error == EINTR || interrupt)
	    take_signals(regs);
}

/*
 * Exec starts here to save registers.
 */
struct execa {
    char	*fname;
    char	**argp;
    char	**envp;
};

int
e_execv(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register struct execa	*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	struct execa		execa;

	execa.fname = argp->fname;
	execa.argp  = argp->argp;
	execa.envp  = (char **)0;

	return (e_execve(serv_port, interrupt, &execa, rval, regs));
}

int
e_execve(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register struct execa	*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register struct emul_regs_2 *regs2;
	int		entry[2];
	unsigned int	entry_count;
	vm_offset_t	arg_addr;
	register int	error;

	/*
	 * Do not have to save user registers on old stack;
	 * they will all be cleared.
	 */

	/*
	 * Call exec.  If error, return without changing registers.
	 */
	entry_count = 2;
	error = e_exec_call(serv_port,
			    interrupt,
			    argp->fname,
			    argp->argp,
			    argp->envp,
			    &arg_addr,
			    entry,
			    &entry_count);
	if (error)
	    return (error);

	/*
	 * Put new user stack just below arguments.
	 */
	regs2 = ((struct emul_regs_2 *)arg_addr) - 1;

	regs2->eip = entry[0];
	regs2->eflags = EFL_USER_SET;
	regs->uesp = regs2;

	/*
	 * Return to new stack.
	 */
	return (E_JUSTRETURN);
}


/*
 * Take a signal.
 */
void
take_signals(regs)
	register struct emul_regs *regs;
{
	struct emul_regs_2	save_regs2;
	register struct emul_regs_2	*regs2;

	register struct sigcontext *scp;
	register struct sigframe {
	    int		(*sf_retadr)();
	    int		sf_signum;
	    int		sf_code;
	    struct sigcontext *sf_scp;
	    struct sigcontext *sf_scpcopy;	/* for return */
	} *fp;

	int	old_mask, old_onstack, sig, code, handler, new_sp;
	boolean_t	interrupt;

	/*
	 * Get anything valuable off user stack first.
	 */
	save_regs2 = *regs->uesp;

	/*
	 * Get the signal to take from the server.  It also
	 * switches the signal mask and the stack, so we must
	 * be off the old user stack before calling it.
	 */
	(void) Bsd1_take_signal(our_bsd_server_port,
			&interrupt,
			&old_mask,
			&old_onstack,
			&sig,
			&code,
			&handler,
			&new_sp);

	/*
	 * If there really were no signals to take, return.
	 */
	if (sig == 0)
	    return;

	/*
	 * Put the signal context and signal frame on the signal stack.
	 */
	if (new_sp == 0) {
	    /*
	     * Build signal frame and context on user's stack.
	     */
	    new_sp = (int)regs->uesp;
	}

	/* The following kludge detects when we're executing an old binary
	 * that was linked with the original signal implementation. Once the
	 * world is relinked this check and the routines osendsig() and
	 * osigreturn() should be removed and the library code in sigvec()
	 * should be fixed to not set the high order bit.
	 * -- lance & tommy 15-mar-89
	 */
	if (((unsigned long) sigreturnaddr & 0x80000000) == 0) {
	  new_sp /= 0;	/* CRASH */
	  return;
	}

	scp = ((struct sigcontext *)new_sp) - 1;
	fp  = ((struct sigframe *)scp) - 1;

	/*
	 * Build the argument list for the signal handler.
	 */
	fp->sf_signum = sig;
	fp->sf_code = code;
	fp->sf_scp = scp;

	/*
	 * Build the stack frame to be used to call sigreturn.
	 */
	fp->sf_scpcopy = scp;
	fp->sf_retadr = (int (*)())
		((unsigned long) sigreturnaddr & ~0x80000000);

	/*
	 * Build the signal context to be used by sigreturn.
	 */
	scp->sc_onstack = old_onstack;
	scp->sc_mask = old_mask;

/*	scp->sc_gs = ... */
/*	scp->sc_fs = ... */
/*	scp->sc_es = ... */
/*	scp->sc_ds = ... */

	scp->sc_edi = regs->edi;
	scp->sc_esi = regs->esi;
	scp->sc_ebp = regs->ebp;
	scp->sc_esp = 0;			/* kernel stack XXX */
	scp->sc_ebx = regs->ebx;
	scp->sc_edx = save_regs2.edx;
	scp->sc_ecx = save_regs2.ecx;
	scp->sc_eax = save_regs2.eax;
/*	scp->sc_trapno = ... */
	scp->sc_err = 0;
	scp->sc_eip = save_regs2.eip;
/*	scp->sc_cs = ... */
	scp->sc_efl = save_regs2.eflags;
	scp->sc_uesp = (int)(regs->uesp+1);	/* user sp after return */
/*	scp->sc_ss = ... */

	/*
	 * Set up the new stack and handler addresses.
	 */
	regs2 = ((struct emul_regs_2 *)fp) - 1;
	*regs2 = save_regs2;
	regs->uesp = regs2;
	regs2->eip = handler;
}

int
e_sigreturn(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	struct emul_regs_2		save_regs2;
	register struct emul_regs_2	*regs2;
	register int			rc;
	struct sigcontext		sc;
	struct a {
	    struct sigcontext *sigcp;
	} *uap = (struct a *)argp;

	/*
	 * Copy in signal context and regs2.
	 */
	save_regs2 = *regs->uesp;
	sc = *uap->sigcp;

	/*
	 * Change signal stack and mask.  If new signals are pending,
	 * do not take them until we switch user stack.
	 */
	rc = Bsd1_sigreturn(serv_port,
			interrupt,
			sc.sc_onstack & 01,
			sc.sc_mask);

	/*
	 * Change registers.
	 */
	regs2 = ((struct emul_regs_2 *)sc.sc_uesp) - 1;

	*regs2 = save_regs2;
	regs->uesp = regs2;

/*	... = sc.sc_gs; */
/*	... = sc.sc_fs; */
/*	... = sc.sc_es; */
/*	... = sc.sc_ds; */
	regs->edi = sc.sc_edi;
	regs->esi = sc.sc_esi;
	regs->ebp = sc.sc_ebp;
	regs->ebx = sc.sc_ebx;
	regs2->edx = sc.sc_edx;
	regs2->ecx = sc.sc_ecx;
	regs2->eax = sc.sc_eax;
	regs2->eip = sc.sc_eip;
/*	... = sc.sc_cs; */
	regs2->eflags = (sc.sc_efl & ~EFL_USER_CLEAR) | EFL_USER_SET;
/*	... = sc.sc_ss; */

	return (E_JUSTRETURN);
}

/*
 * Compatibility with 4.2 chmk $139 used by longjmp()
 */
e_osigcleanup()
{
}

/*
 * Sigvec hides the signal trampoline address in a VERY
 * strange place.
 */
int
i386_sigvec(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register int		*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register int	rc;
	register vm_offset_t	sigtramp;

	sigtramp = regs->uesp->edx;	/* (!) */

	rc = e_sigvec(serv_port,
			interrupt,
			argp[0],			/* signo */
			(struct sigvec *)argp[1],	/* nsv */
			(struct sigvec *)argp[2],	/* osv */
			sigtramp);

	if (rc == 0)
	    sigreturnaddr = sigtramp;

	return (rc);
}

/*
 * Wait has a weird parameter passing mechanism.
 */
int
e_wait(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register struct emul_regs_2 *regs2 = regs->uesp;

	int	new_args[2];

#define	EFL_ALLCC	(EFL_CF|EFL_PF|EFL_ZF|EFL_SF)

	if ((regs2->eflags & EFL_ALLCC) == EFL_ALLCC) {
	    new_args[0] = regs2->ecx;	/* options */
	    new_args[1] = regs2->edx;	/* rusage_p */
	}
	else {
	    new_args[0] = 0;
	    new_args[1] = 0;
	}
	return (emul_generic(serv_port, interrupt,
			SYS_wait, &new_args[0], rval));
}

int
e_fork(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register struct emul_regs_2 *regs2 = regs->uesp;
	register int error;

	struct i386_thread_state	child_regs;
	int				seg_regs[6];
	
	extern int	child_fork();
	extern void	get_seg_regs();

	/*
	 * Set up registers for child.  It resumes on its own stack.
	 */
	get_seg_regs(seg_regs);

	child_regs.gs = seg_regs[5];
	child_regs.fs = seg_regs[4];
	child_regs.es = seg_regs[3];
	child_regs.ds = seg_regs[2];
	child_regs.edi = regs->edi;
	child_regs.esi = regs->esi;
	child_regs.ebp = regs->ebp;
     /*	child_regs.esp */
	child_regs.ebx = regs->ebx;
	child_regs.edx = regs2->edx;
	child_regs.ecx = regs2->ecx;
	child_regs.eax = regs2->eax;
	child_regs.eip = (int)child_fork;
	child_regs.cs = seg_regs[0];
	child_regs.efl = regs2->eflags;
	child_regs.uesp = (int)regs->uesp;
	child_regs.ss = seg_regs[1];
	
	/* FP regs!!!! */

	/*
	 * Create the child.
	 */
	error = Bsd1_fork(serv_port, interrupt,
			 (int *)&child_regs,
			 i386_THREAD_STATE_COUNT,
			 &rval[0]);

	if (error == 0)
	    rval[1] = 0;

	return (error);
}

vm_offset_t
set_arg_addr(arg_size)
	vm_size_t	arg_size;
{
	/*
	 * Round arg size to fullwords
	 */
	arg_size = (arg_size + NBPW-1) & ~(NBPW - 1);

	/*
	 * Put argument list at top of stack.
	 */
	return (USRSTACK - arg_size);
}
