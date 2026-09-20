/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

/*
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/config.h>

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/ptrace.h>
#include <linux/unistd.h>

#include <osfmach3/assert.h>
#include <osfmach3/mach3_debug.h>

#include <asm/pgtable.h>
#include <hp_pa/psw.h>

#define _S(nr) (1<<((nr)-1))

#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

#if	CONFIG_OSFMACH3_DEBUG
#define SIGNAL_DEBUG	1
#endif	/* CONFIG_OSFMACH3_DEBUG */

#ifdef	SIGNAL_DEBUG
int signal_debug = 0;
#endif	/* SIGNAL_DEBUG */

asmlinkage int sys_waitpid(pid_t, unsigned long *, int);
asmlinkage int do_signal(unsigned long, struct pt_regs *, long);

/*
 * atomically swap in the new signal mask, and wait for a signal.
 */
asmlinkage int sys_sigsuspend(unsigned long set, struct pt_regs *regs)
{
	unsigned long mask;

	mask = current->blocked;
	current->blocked = set & _BLOCKABLE;
	regs->state.arg0 = -EINTR;

	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (do_signal(mask, regs, 0)) {
			/*
			 * If a signal handler needs to be called,
			 * do_signal() has set arg0 to the signal number (the
			 * first argument of the signal handler), so don't
			 * overwrite that with EINTR !
			 * In the other cases, do_signal() doesn't touch 
			 * arg0, so it's still set to -EINTR (see above).
			 */
			return regs->state.arg0;
		}
	}
}

asmlinkage int sys_sigreturn(struct pt_regs *regs)
{
	struct sigcontext_struct context;
	unsigned long ctxpt;

#ifdef	SIGNAL_DEBUG
	if (signal_debug) {
		printk("sigreturn to %s \n", current->comm);
	}
#endif	/* SIGNAL_DEBUG */

	ctxpt = regs->state.sp - (128 + sizeof(context));

	if (verify_area(VERIFY_READ, (void *)ctxpt, sizeof(context)))
		goto badframe;

	memcpy_fromfs(&context, (void *)ctxpt, sizeof(context));

	current->blocked = context.oldmask & _BLOCKABLE;

	regs->state.flags = context.sig_state.flags;
	regs->state.r1 = context.sig_state.r1;
	regs->state.rp = context.sig_state.rp;
	regs->state.r3 = context.sig_state.r3;
	regs->state.r4 = context.sig_state.r4;
	regs->state.r5 = context.sig_state.r5;
	regs->state.r6 = context.sig_state.r6;
	regs->state.r7 = context.sig_state.r7;
	regs->state.r8 = context.sig_state.r8;
	regs->state.r9 = context.sig_state.r9;
	regs->state.r10 = context.sig_state.r10;
	regs->state.r11 = context.sig_state.r11;
	regs->state.r12 = context.sig_state.r12;
	regs->state.r13 = context.sig_state.r13;
	regs->state.r14 = context.sig_state.r14;
	regs->state.r15 = context.sig_state.r15;
	regs->state.r16 = context.sig_state.r16;
	regs->state.r17 = context.sig_state.r17;
	regs->state.r18 = context.sig_state.r18;
	regs->state.t4 = context.sig_state.t4;
	regs->state.t3 = context.sig_state.t3;
	regs->state.t2 = context.sig_state.t2;
	regs->state.t1 = context.sig_state.t1;
	regs->state.arg3 = context.sig_state.arg3;
	regs->state.arg2 = context.sig_state.arg2;
	regs->state.arg1 = context.sig_state.arg1;
	regs->state.arg0 = context.sig_state.arg0;
	regs->state.ret1 = context.sig_state.ret1;
	regs->state.sp = context.sig_state.sp;
	regs->state.r31 = context.sig_state.r31;
	regs->state.sar = context.sig_state.sar;
	regs->state.iioq_head = LOWER_PRIV(context.sig_state.iioq_head);
	regs->state.iioq_tail = LOWER_PRIV(context.sig_state.iioq_tail);
	regs->state.ipsw = (context.sig_state.ipsw & ~PSW_USER_RESET) | PSW_USER_SET;
	regs->state.fpu = context.sig_state.fpu;

#if 0
	if(regs->state.fpu) {
		kern_return_t kr;
		ASSERT(regs->state.fpu == 1);

		ctxpt += sizeof(context.sig_state);
		memcpy_fromfs(&context.sig_fstate, (void *)ctxpt, sizeof(context.sig_fstate));

		kr = thread_set_state(current->osfmach3.thread->mach_thread_port,
				      HP700_FLOAT_STATE,
				      (thread_state_t)&context.sig_fstate,
				      HP700_FLOAT_STATE_COUNT);
		if (kr != KERN_SUCCESS) {
			MACH3_DEBUG(0, kr, ("copy_thread: thread_set_state"));
		}
	}
#endif

	return context.sig_state.ret0;

badframe:
	do_exit(SIGSEGV);
}

static void setup_frame(struct sigaction * sa,
	struct pt_regs * regs, int signr,
	unsigned long oldmask)
{
	unsigned long newsp;
	void *tramp;
	struct sigcontext_struct *sigcontext;
	int sigreturn_size;
	
	sigreturn_size = (4 * sizeof(void*)) + sizeof (*sigcontext) + 128;

	if (verify_area(VERIFY_WRITE, (const void *)regs->state.sp, sigreturn_size))
		do_exit(SIGSEGV);

	newsp = regs->state.sp;
	tramp = (void *)newsp;
	newsp += 4 * sizeof(void *);
	sigcontext = (struct sigcontext_struct *)newsp;
	newsp += (sizeof(*sigcontext) + 128);

	memcpy_tofs(sigcontext, regs, sizeof(*regs));

	put_fs_long(oldmask, &sigcontext->oldmask); 

#if 0
	if(regs->state.fpu) {
		kern_return_t kr;
		mach_msg_type_number_t	count;
		struct hp700_float_state float_state;

		ASSERT(regs->state.fpu == 1);

		count = HP700_FLOAT_STATE_COUNT;
		kr = thread_get_state(current->osfmach3.thread->mach_thread_port,
				      HP700_FLOAT_STATE,
				      (thread_state_t)&float_state, &count);
		if (kr != KERN_SUCCESS) {
			MACH3_DEBUG(0, kr, ("copy_thread: thread_get_state"));
		}

		memcpy_tofs(&sigcontext->sig_fstate, &float_state, sizeof(float_state));
	}
#endif

	put_fs_long(0x00001820, tramp);    /* mtsp r0,sr0 */
	put_fs_long(0x20200801, tramp+4);  /* ldil -0x40000000,r1  */
	put_fs_long(0xe4200008, tramp+8);  /* ble 4(sr0,r1) */
	put_fs_long(0x341600ee, tramp+12);  /* ldi __NR_sigreturn,t1 */

#if 0
	flush_instruction_cache_range(current->mm,
				      (unsigned long) tramp,
				      (unsigned long) (tramp + 16));
#else
	flush_cache(regs->state.sr4, tramp, tramp+12);
#endif


	regs->state.arg0 = signr;
	regs->state.rp = LOWER_PRIV((unsigned long)tramp);
	regs->state.sp = newsp;
	regs->state.iioq_head = LOWER_PRIV((unsigned long)sa->sa_handler);
	regs->state.iioq_tail = regs->state.iioq_head + 4;
	regs->state.ipsw = PSW_USER_SET;
}

/*
 * OK, we're invoking a handler
 */	
static void handle_signal(unsigned long signr, struct sigaction *sa,
	unsigned long oldmask, struct pt_regs * regs, long result)
{
	/* are we from a system call? */
	if (regs->state.flags & SS_INSYSCALL) {
#ifdef SIGNAL_DEBUG
		if (signal_debug) 
			printk("%s send signal in syscall \n", current->comm);
#endif
		/* If so, check system call restarting.. */
		switch (result) {
		case -ERESTARTNOHAND:
			regs->state.ret0 = EINTR;
			break;
		case -ERESTARTSYS:
			if (!(sa->sa_flags & SA_RESTART)) {
				regs->state.ret0 = EINTR;
				break;
			}
			/* fallthrough */
		case -ERESTARTNOINTR:
			regs->state.iioq_head -= 4;
			regs->state.iioq_tail -= 4;
		}
	}
#ifdef SIGNAL_DEBUG
	else if (signal_debug) {
		printk("%s send signal in trap \n", current->comm);
	}
#endif
	/* set up the stack frame */
	setup_frame(sa, regs, signr, oldmask);

	if (sa->sa_flags & SA_ONESHOT)
		sa->sa_handler = NULL;
	if (!(sa->sa_flags & SA_NOMASK))
		current->blocked |= (sa->sa_mask | _S(signr)) & _BLOCKABLE;
}

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 *
 * Note that we go through the signals twice: once to check the signals that
 * the kernel can handle, and then we build all the user-level signal handling
 * stack-frames in one go after that.
 */
asmlinkage int do_signal(unsigned long oldmask, struct pt_regs * regs, long result)
{
	unsigned long mask = ~current->blocked;
	unsigned long signr;
	struct sigaction * sa;
	int bitno;

	while ((signr = current->signal & mask)) {
		for (bitno = 0;  bitno < 32;  bitno++) {
			if (signr & (1<<bitno)) break;
		}
		signr = bitno;
		current->signal &= ~(1<<signr);  /* Clear bit */
		sa = current->sig->action + signr;
		signr++;

		if ((current->flags & PF_PTRACED) && signr != SIGKILL) {
			current->exit_code = signr;
			current->state = TASK_STOPPED;
			notify_parent(current, SIGCHLD);
			schedule();
			if (!(signr = current->exit_code))
				continue;
			current->exit_code = 0;
			if (signr == SIGSTOP)
				continue;
			if (_S(signr) & current->blocked) {
				current->signal |= _S(signr);
				continue;
			}
			sa = current->sig->action + signr - 1;
		}
		if (sa->sa_handler == SIG_IGN) {
			if (signr != SIGCHLD)
				continue;
			/* check for SIGCHLD: it's special */
			while (sys_waitpid(-1,NULL,WNOHANG) > 0)
				/* nothing */;
			continue;
		}
		if (sa->sa_handler == SIG_DFL) {
			if (current->pid == 1)
				continue;
			switch (signr) {
			case SIGCONT: case SIGCHLD: case SIGWINCH:
				continue;

			case SIGTSTP: case SIGTTIN: case SIGTTOU:
				if (is_orphaned_pgrp(current->pgrp))
					continue;
			case SIGSTOP:
				if (current->flags & PF_PTRACED)
					continue;
				current->state = TASK_STOPPED;
				current->exit_code = signr;
				if (!(current->p_pptr->sig->action[SIGCHLD-1].sa_flags & 
						SA_NOCLDSTOP))
					notify_parent(current, SIGCHLD);
				schedule();
				continue;

			case SIGQUIT: case SIGILL: case SIGTRAP:
			case SIGABRT: case SIGFPE: case SIGSEGV:
				if (current->binfmt && current->binfmt->core_dump) {
					if (current->binfmt->core_dump(signr, regs))
						signr |= 0x80;
				}
				/* fall through */
			default:
				current->signal |= _S(signr & 0x7f);
				current->flags |= PF_SIGNALED;
				do_exit(signr);
			}
		}
		handle_signal(signr, sa, oldmask, regs, result);
		return 1;
	}

	/* Did we come from a system call? */
	if (regs->state.flags & SS_INSYSCALL) {
		/* Restart the system call - no handlers present */
		if (result == -ERESTARTNOHAND ||
		    result == -ERESTARTSYS ||
		    result == -ERESTARTNOINTR) {
			regs->state.iioq_head -= 4;
			regs->state.iioq_tail -= 4;
		}
	}

	return 0;
}
