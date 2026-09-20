/* 
 * MACH Operating System
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
 * Revision 2.5  94/03/25  18:21:33  mrt
 * 	Corrected include of mach/mach.h to mach.h
 * 	Added some casts for gcc.
 * 	[93/11/15            mrt]
 * 
 * Revision 2.4  92/02/02  13:01:09  rpd
 * 	Removed MAP_UAREA code.
 * 	[92/01/30            rpd]
 * 
 * Revision 2.3  91/12/19  20:25:35  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:41:09  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:07:17  rwd]
 * 
 * Revision 2.7  90/08/06  15:30:48  rwd
 * 	Include sys/types.h
 * 	[90/07/17            rwd]
 * 
 * Revision 2.6  90/06/19  23:06:51  rpd
 * 	Added pid_by_task.
 * 	[90/06/14            rpd]
 * 
 * Revision 2.5  90/06/02  15:20:49  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  19:29:23  rpd]
 * 
 * Revision 2.4  90/05/21  13:46:47  dbg
 * 	Add set_arg_addr.
 * 	[90/05/17            dbg]
 * 
 * Revision 2.3  90/03/14  21:23:16  rwd
 * 	Check for signals before system call.  Added mapped sigreturn.
 * 	[90/02/27            rwd]
 * 
 * Revision 2.2  89/11/29  15:26:52  af
 * 	Special path for sigreturn.  Fixed screwup in take_signal().
 * 	[89/11/26  11:14:51  af]
 * 
 * 	Taken from David Golub's version from Vax and adapted for Mips.
 * 	[89/11/06            af]
 * 
 */
/*
 * Take/Return from signals - machine-dependent.
 */
#include <mach.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <mips/vmparam.h>
#include <bsd_msg.h>	/* error code definitions */
#include <mach/mips/mips_instruction.h>

#include "syscall_table.h"

extern mach_port_t	our_bsd_server_port;

#define	E_JUSTRETURN	255	/* return without changing registers */
#define	E_SIGRETURN	254	/* return and reload the S registers */

/*
 * User's registers are all stored on the user's stack
 * in an exception frame that is just a sigcontext structure.
 * But we know what follows on the user's stack.
 *
 * The emulator's stack only contains a pointer to the
 * exception frame, and argument save space as required.
 */
struct emul_exception_frame {
	unsigned	onstack;	/* see sigcontext */
	unsigned	mask;
	unsigned	pc;		/* program counter */
	unsigned	zero;		/* wired zero */
	unsigned	at;		/* assembler temporary */
	unsigned	v0;		/* return value 0 */
	unsigned	v1;		/* return value 1 */
	unsigned	a0;		/* arg register 0 */
	unsigned	a1;		/* arg register 1 */
	unsigned	a2;		/* arg register 2 */
	unsigned	a3;		/* arg register 3 */
	unsigned	t0;		/* caller saved 0 */
	unsigned	t1;		/* caller saved 1 */
	unsigned	t2;		/* caller saved 2 */
	unsigned	t3;		/* caller saved 3 */
	unsigned	t4;		/* caller saved 4 */
	unsigned	t5;		/* caller saved 5 */
	unsigned	t6;		/* caller saved 6 */
	unsigned	t7;		/* caller saved 7 */
	unsigned	s0;		/* callee saved 0 */
	unsigned	s1;		/* callee saved 1 */
	unsigned	s2;		/* callee saved 2 */
	unsigned	s3;		/* callee saved 3 */
	unsigned	s4;		/* callee saved 4 */
	unsigned	s5;		/* callee saved 5 */
	unsigned	s6;		/* callee saved 6 */
	unsigned	s7;		/* callee saved 7 */
	unsigned	t8;		/* code generator 0 */
	unsigned	t9;		/* code generator 1 */
	unsigned	k0;		/* kernel reserved 0 */
	unsigned	k1;		/* kernel reserved 1 */
	unsigned	gp;		/* global pointer */
	unsigned	sp;		/* stack pointer */
	unsigned	fp;		/* frame pointer */
	unsigned	ra;		/* return address */

	unsigned	mdlo;		/* low mult result */
	unsigned	mdhi;		/* high mult result */

	unsigned	use_fpa;	/* did we use the fpa */
	unsigned	fpa_regs[32];	/* gp fpa regs */
	unsigned	fpa_csr;
	unsigned	fpa_eir;

	unsigned	cause;		/* machine exception code */
	unsigned	badv;		/* last address fault (virtual) */
	unsigned	badp;		/* ditto, physical (notinyourlife) */

	/* unsigned stack_args[0..5] */
};

struct emul_stack {
	int	save_arg[4];	/* C calling convention */
	struct emul_exception_frame *usp;
};

int	take_signals();	/* forward */

int	emul_low_entry = -9;
int	emul_high_entry = 255;

extern mips_instruction emul_vector_base[][3],
			emul_task_by_pid[][3],
			emul_pid_by_task[][3],
			emul_init_process[][3],
			emul_htg_syscall[][3],
			emul_fork[][3],
			emul_indirect[][3];

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
					emul_vector_base[i],
					i);
	}
	rc = task_set_emulation(task,
			(vm_address_t) emul_task_by_pid,
			-33);
	rc = task_set_emulation(task,
			(vm_address_t) emul_pid_by_task,
			-34);
	rc = task_set_emulation(task,
			(vm_address_t) emul_init_process,
			-41);
	rc = task_set_emulation(task,
			(vm_address_t) emul_htg_syscall,
			-52);
	rc = task_set_emulation(task,
			(vm_address_t) emul_fork,
			2);		/* fork */
	rc = task_set_emulation(task,
			(vm_address_t) emul_fork,
			66);		/* vfork */
	rc = task_set_emulation(task,
			(vm_address_t) emul_indirect,
			0);		/* COULD be a fork */
}

/*
 * System calls enter here.
 */
int
emul_syscall(esp, usp)
	struct emul_stack		*esp;
	struct emul_exception_frame	*usp;
{
	register int	syscode;
	register int	error;
	register struct sysent *callp;
	register int	*args = (int*)(usp + 1);
	int		rval[2];
	boolean_t	interrupt = FALSE;

	syscode = usp->v0;

	/*
	 *	Store on stack args that assembly code did not
	 *	[This is unfortunate, but needed for indirects]
	 */
	args[0] = usp->a0;
	args[1] = usp->a1;
	args[2] = usp->a2;
	args[3] = usp->a3;
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
	rval[1] = usp->v1;

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
				*args,
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
				esp);
		/* if changed */
		usp = esp->usp;
		break;
	}

	/*
	 * Set up return values.
	 */

	switch (error) {
	    case E_JUSTRETURN:
	    case E_SIGRETURN:
		/* Do not alter registers */
		break;


	    case 0:
		/* Success */
		usp->v0 = rval[0];
		usp->v1 = rval[1];
		usp->a3 = 0;
		break;

	    case ERESTART:
		/* restart call */
		usp->pc -= sizeof(mips_instruction);
		break;

	    default:
		/* error */
		usp->v0 = error;
		usp->a3 = 1;
		break;
	}

	/*
	 * Handle interrupt request
	 */
	if (error == ERESTART || error == EINTR || interrupt)
	    return take_signals(esp);
	return (error == E_SIGRETURN) ? 1 : 0;
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
	struct emul_stack	*regs;
{
	return (e_execvx(serv_port, interrupt, argp, FALSE, regs));
}

int
e_execve(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register struct execa	*argp;
	int			*rval;
	struct emul_stack	*regs;
{
	return (e_execvx(serv_port, interrupt, argp, TRUE, regs));
}

int
e_execvx(serv_port, interrupt, argp, use_env, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register struct execa	*argp;
	struct emul_stack	*regs;
	boolean_t		use_env;
{
	int		entry[2];
	unsigned int	entry_count;
	vm_offset_t	arg_addr;
	register int	error;
	struct emul_exception_frame *regs2;

	/*
	 * Call exec.  If error, return without changing registers.
	 */
	entry_count = 2;	/* ??? */
	error = e_exec_call(serv_port,
			    interrupt,
			    argp->fname,
			    argp->argp,
			    (use_env) ? argp->envp : (char **)0,
			    &arg_addr,
			    entry,
			    &entry_count);
	if (error)
	    return (error);

	/*
	 * Put new user stack just below arguments.
	 */
	regs2 = ((struct emul_exception_frame *)arg_addr) - 1;
	regs->usp = regs2;

	/*
	 * Set new pc, and clear frame pointer for traceback.
	 */
	regs2->sp = (unsigned)arg_addr;
	regs2->pc = entry[0];
	regs2->gp = entry[1];	/* cosmetic */
	regs2->ra = 0;		/* cosmetic */
	regs2->fp = 0;		/* cosmetic */

	/*
	 * Return to new stack.
	 */
	return (E_JUSTRETURN);
}


/*
 * Wait has a weird parameter passing mechanism
 * on the VAX, and we have to put up with it.
 */
int
e_wait(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_stack	*regs;
{
	register struct emul_exception_frame *regs2 = regs->usp;
	int	new_args[2], error;

	new_args[0] = argp[1];	/* options */
	new_args[1] = argp[2];	/* rusage_p */

	error = emul_generic(serv_port, interrupt,
			SYS_wait, &new_args[0], rval);
	if (error == 0 && argp[0])
		*((int *)argp[0]) = rval[1];
	return error;
}


/*
 * Take a signal.
 */
unsigned sigtramp;

int
take_signals(regs)
	register struct emul_stack *regs;
{
	register struct emul_exception_frame	*regs2 = regs->usp;
	register struct sigcontext *scp;
	int	old_mask, old_onstack, sig, code, handler, new_sp;
	boolean_t	interrupt;
	struct sigframe {
		unsigned	pc;
		unsigned	a0;
		unsigned	a1;
		unsigned	a2;
		unsigned	a3;
	} *fp;

	/*
	 * Get the signal to take from the server.  It also
	 * switches the signal mask and the stack, so we must
	 * be off the old user stack before calling it.
	 * [That's the bottom line for having an emulator stack]
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
	    return 0;

	if (new_sp != 0) {
	    /*
	     * Copy the sigcontext on new stack
	     */
	    scp = ((struct sigcontext *)new_sp) - 1;
	    *scp = *((struct sigcontext *)regs2);
	} else
	    scp = (struct sigcontext *) regs2;

	/*
	 * Build the stack frame that we'll return to
	 */
	fp  = ((struct sigframe *)scp) - 1;
	fp->pc = sigtramp;
	/*
	 * Args to sigtramp
	 */
	fp->a0 = sig;
	fp->a1 = code;
	fp->a2 = (unsigned)scp;
	fp->a3 = handler;

	/*
	 * Fixup what's missing in the signal context.
	 */
	scp->sc_onstack = old_onstack;
	scp->sc_mask = old_mask;

	/* XXX fpa state */
	scp->sc_ownedfp = 0;
	bzero(scp->sc_fpregs, sizeof scp->sc_fpregs);
	scp->sc_fpc_csr = 0;
	scp->sc_fpc_eir = 0;

	/* XXX sc_cause sc_badv sc_badp */
	scp->sc_cause = 0;
	scp->sc_badvaddr = 0;
	scp->sc_badpaddr = 0;

	return (int)fp;
}

/*
 * New sigreturn.
 */
int
e_sigreturn(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	struct a {
	    struct sigcontext *sigcp;
	} 			*argp;
	int			*rval;
	struct emul_stack	*regs;
{
	register struct emul_exception_frame	*regs2 = regs->usp;
	register int				 rc;
	register struct sigcontext		*scp = argp->sigcp;

	/*
	 * Change signal stack and mask.  If new signals are pending,
	 * do not take them until we switch user stack.
	 */
	rc = Bsd1_sigreturn(serv_port,
			interrupt,
			scp->sc_onstack & 01,
			scp->sc_mask);

	/*
	 * Change registers.
	 */
	
	*regs2 = * ((struct emul_exception_frame *)scp);

	return (E_SIGRETURN);
/*return E_JUSTRETURN;*/
}

/*
 * Compatibility with BSD4.2 chmk $139 used by longjmp().
 */
int
e_osigcleanup(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_exception_frame	*regs;
{
	return (EINVAL);
}



int
e_fork(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_stack	*regs;
{
	register struct emul_exception_frame *regs2 = regs->usp;
	register int error;
	struct mips_thread_state	*child_regs;
	int	saved_pc;
	extern	child_fork();
	
	/*
	 * Set up registers for child.  It resumes on its own stack.
	 * To save too many copies use temporarily our saved state.
	 */
	child_regs = (struct mips_thread_state *)(&regs2->at);
	saved_pc = child_regs->pc;
	child_regs->pc = (int)child_fork;	/* to reinit emulator */
	
	/*
	 * Create the child.
	 */
	error = Bsd1_fork(serv_port, interrupt,
			 child_regs,
			 MIPS_THREAD_STATE_COUNT,
			 &rval[0]);

	child_regs->pc = saved_pc;	/* us, actually */

	if (error == 0)
	    rval[1] = 0;

	return (error);
}

vm_offset_t
set_arg_addr(arg_size)
	vm_size_t	arg_size;
{
	/*
	 * Round argument size to fullwords
	 */
	arg_size = (arg_size + NBPW - 1) & ~(NBPW - 1);

	/*
	 * Put argument list at top of stack.
	 * MipsCo reserved a hold at the top,
	 * and also put the restriction that
	 * the arglist be double-word aligned
	 */
#define UMIPS_HOLE 32
	return ( (USRSTACK - UMIPS_HOLE - arg_size) & ~0xf );
}

