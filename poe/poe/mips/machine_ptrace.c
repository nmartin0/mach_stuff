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
 * $Log:	machine_ptrace.c,v $
 * Revision 2.3  91/12/19  20:28:53  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:19:46  rwd
 * 	This is really a machine dependant file.
 * 	[90/07/14            rwd]
 * 
 */
/*
 *	File:	./mips/machine_ptrace.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <mips/mips_ptrace.h>
#include <ux_user.h>

Bsd_ptrace(ut, rval, request, pid, address, data)
	struct ux_task *ut;
	int rval[2];
	int request;
	int pid;
	int *address;
	int data;
{
	int error;
	unsigned long *word;
	struct ux_task *utc;

dprintf("ptrace(request=%d, pid=%d, address=0x%x, data=0x%x / %d\n",
       request, pid, address, data, data);
	if (request == PT_TRACE_ME) {
		ut->ut_traced = TRUE;
		return 0;
	}
	error = lookup_ux_task_by_pid(pid, &utc);
	if (error) {
		return error;
	}
	if (utc->ut_parent != ut || ! utc->ut_stopped || ! utc->ut_traced) {
		return EINVAL;
	}
	switch (request) {
	case PT_READ_I:
	case PT_READ_D:
		error = copyin(utc, address, sizeof(*word), &word);
		if (error) {
dprintf("ptrace read[0x%x] : error %d\n", address, error);
			return EIO;	/* XXX */
		}
dprintf("ptrace read[0x%x] = 0x%x\n", address, *word);
		rval[0] = (int) *word;
		uncopyin(word, sizeof(*word));
		return 0;

	case PT_READ_U:
dprintf("ptrace read_u[0x%x]\n", address);
{
		struct mips_regs {
			struct mips_thread_state	ts;
			struct mips_float_state		fs;
			struct mips_exc_state		es;
		} *mr;
		int addr = (int) address;
		int *r;

		error = get_threadstate(utc);
		if (error) {
			dprintf("ptrace: error %d", error);
			return error;
		}
		mr = (struct mips_regs *) utc->ut_threadstate;
		if (addr < GPR_BASE) {
			return EIO;
		} else if (addr < GPR_BASE + NGP_REGS) {
			rval[0] = ((int *) &mr->ts)[addr-1];
		} else if (addr < FPR_BASE + NFP_REGS) {
			rval[0] = ((int *) &mr->fs)[addr];
		} else if (addr < SIG_BASE + NSIG_HNDLRS) {
			rval[0] = (int)
			    utc->ut_sigvec[addr - SIG_BASE].sv_handler;
		} else if (addr == MMHI) {
			rval[0] = mr->ts.mdhi;
		} else if (addr == MMLO) {
			rval[0] = mr->ts.mdlo;
		} else if (addr == PC) {
			rval[0] = mr->ts.pc;
		} else if (addr == CAUSE) {
			rval[0] = mr->es.cause;
		} else if (addr == 0x424) {	/* u.u_code */
			rval[0] = 8; /* XXX */
		} else {
			printf("??? u_val %d?\n", addr);
			return EIO;
		}
}
		return 0;

	case PT_WRITE_I:
	case PT_WRITE_D:
dprintf("ptrace write[0x%x, 0x%x]\n", address, data);
		/*
		 * XXX 1. area might not be writable?
		 * XXX 2. cache flush on mips?
		 */
		error = copyout(utc, data, address, sizeof(data));
		if (error) {
			return EIO;	/* XXX */
		}
		rval[0] = (int) word;
		return 0;

	case PT_WRITE_U:
printf("ptrace write_u[0x%x, 0x%x]\n", address, data);
		return 0; /* XXX */

	case PT_CONTINUE:
printf("ptrace continue[addr=0x%x, sig=0x%x]\n", address, data);
/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX update u area */
		if (data < 0 || data > NSIG) {
			return EIO;
		}
		sig_docont(utc, 0);
		if (address != (int *)1) {
			/* XXX should get pc to get set to address... */
		}
		if (data != 0) {
			bsd_sendsig(utc, data);
		}
		return 0;

	case PT_KILL:
printf("ptrace kill\n");
		utc->ut_traced = FALSE; /* XXX */
		bsd_sendsig(utc, SIGKILL);
		return 0;

	case PT_STEP:
printf("ptrace step\n");
		sig_docont(utc, 0);	/* XXX don't know how to step */
		return 0;

	default:
		return EIO;
	}
}
