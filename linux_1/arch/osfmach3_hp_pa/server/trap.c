/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/exception.h>
#include <mach/thread_status.h>
#include <mach/mach_port.h>
#include <mach/mach_host.h>
#include <mach/mach_traps.h>
#include <mach/mach_interface.h>
#include <mach/message.h>
#include <mach/exc_server.h>

#include <osfmach3/mach_init.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/parent_server.h>
#include <osfmach3/assert.h>
#include <osfmach3/uniproc.h>
#include <osfmach3/server_thread.h>
#include <osfmach3/serv_port.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/sys.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/interrupt.h>
#include <asm/syscalls.h>
#include <hp_pa/psw.h>

extern struct sysent sys_call_table[];
extern char *sys_call_name_table[];	
extern void syscall_trace(void);

extern int do_signal(unsigned long oldmask, struct pt_regs *, long);
extern void do_page_fault(struct pt_regs *, unsigned long, long);
extern void do_debug(int, struct pt_regs *, long);

extern boolean_t in_kernel;
extern boolean_t use_activations;
extern mach_port_t exc_subsystem_port;
extern mach_port_t server_exception_port;
extern boolean_t server_exception_in_progress;

#if	CONFIG_OSFMACH3_DEBUG
#define SYSCALLTRACE 1
#endif	/* CONFIG_OSFMACH3_DEBUG */

#if	SYSCALLTRACE
int syscalltrace = 0;
int syscalltrace_regs = 0;
#endif	/* SYSCALLTRACE */

void
mach_trap_init(void)
{
	osfmach3_trap_init(EXCEPTION_STATE, MACHINE_THREAD_STATE);
}

#define TASK_BY_PID_TRAPNO	-33

kern_return_t
catch_exception_raise_state(
	mach_port_t			port,
	exception_type_t		exc_type,
	exception_data_t		code,
	mach_msg_type_number_t		code_count,
	int				*flavor,
	thread_state_t			old_state,
	mach_msg_type_number_t		icnt,
	thread_state_t			new_state,
	mach_msg_type_number_t		*ocnt)
{
	struct hp700_thread_state *in_state;
	struct hp700_thread_state *out_state;
	unsigned long trapno;
	struct pt_regs *regs;
	struct sysent *sys_ent;
	long error_code = 0 ;
	int i;
	mach_port_t trap_port_name;
	boolean_t dont_mess_with_state = FALSE;
	struct server_thread_priv_data *priv_datap;
	osfmach3_jmp_buf jmp_buf;
#if	SYSCALLTRACE
	char thread_name[100];
#endif	/* SYSCALLTRACE */

	priv_datap = server_thread_get_priv_data(cthread_self());
	while (!priv_datap->activation_ready) {
		/*
		 * The cthread hasn't been wired to the activation yet.
		 * Wait until its' done (see new_thread_activation()).
		 */
		server_thread_yield(10);
	}

	ASSERT(*flavor == HP700_THREAD_STATE);
	in_state = (struct hp700_thread_state *) old_state;
	out_state = (struct hp700_thread_state *) new_state;

	if (port == server_exception_port) {
		extern void die_if_kernel(char *str,
					  struct pt_regs *regs,
					  long err);
		extern void console_verbose(void);

		/*
		 * Oops: a server exception...
		 */
		console_verbose();
		server_exception_in_progress = TRUE;
		printk("*** LINUX SERVER EXCEPTION ***\n");
		switch (exc_type) {
		    case 0:
			printk("*** EXCEPTION 0 (fake interrupt)\n");
			break;
		    case EXC_BAD_ACCESS:
			printk("*** EXC_BAD_ACCESS\n");
			break;
		    case EXC_BAD_INSTRUCTION:
			printk("*** EXC_BAD_INSTRUCTION\n");
			break;
		    case EXC_ARITHMETIC:
			printk("*** EXC_ARITHMETIC\n");
			break;
		    case EXC_EMULATION:
			printk("*** EXC_EMULATION\n");
			break;
		    case EXC_SOFTWARE:
			printk("*** EXC_SOFTWARE\n");
			break;
		    case EXC_BREAKPOINT:
			printk("*** EXC_BREAKPOINT\n");
			break;
		    case EXC_SYSCALL:
			printk("*** EXC_SYSCALL\n");
			break;
		    case EXC_MACH_SYSCALL:
			printk("*** EXC_MACH_SYSCALL\n");
			break;
		    case EXC_RPC_ALERT:
			printk("*** EXC_RPC_ALERT\n");
			break;
		    default:
			printk("*** UNKNOWN EXCEPTION %d\n", exc_type);
			break;
		}
		for (i = 0; i < code_count; i++) {
			printk("*** CODE[%d] = %d (0x%x)\n",
			       i, code[i], code[i]);
		}

		die_if_kernel("LINUX SERVER EXCEPTION",
			      (struct pt_regs*)in_state,
			      0);
		panic("LINUX SERVER EXCEPTION");
		server_exception_in_progress = FALSE;
		return KERN_SUCCESS;	/* just in case... */
	}

	if (use_activations) {
		/*
		 * We come here directly as the result of a thread
		 * migration and short-circuited exception RPC.
		 */
		mutex_lock(&uniproc_antechamber_mutex);
		uniproc_enter();
		mutex_unlock(&uniproc_antechamber_mutex);

		/*
		 * As an optimization, we only do the setjmp
		 * if it hasn't been left ready-to-use by a previous
		 * use of this routine from this c-thread.
		 *
		 * If we're not called from the same context as the
		 * last call (jmp_buf not at the same place in the
		 * stack), we need to do a setjmp again to update
		 * the jump buffer.
		 */
		if (priv_datap->jmp_buf != &jmp_buf) {
			priv_datap->jmp_buf = &jmp_buf;
			if (osfmach3_setjmp(priv_datap->jmp_buf)) {
				/*
				 * The user task is being terminated.
				 */
				uniproc_exit();
				return MIG_NO_REPLY;
			}
		}
	}
		
	ASSERT(current == FIRST_TASK);
	trap_port_name = (mach_port_t) serv_port_name(port);
	uniproc_switch_to(current, (struct task_struct *) (trap_port_name - 1));

	current->osfmach3.thread->under_server_control = TRUE;

	if(in_state == out_state) {
		regs = (struct pt_regs*)in_state;
	}
	else {
		regs = &current->osfmach3.thread->regs;
		osfmach3_state_to_ptregs(in_state, regs);
	}

	current->osfmach3.thread->regs_ptr = regs;

	if (current == FIRST_TASK) {
		/*
		 * The server is processing one of its own syscalls.
		 * Don't mess with the task's registers.
		 */
		printk("current = FIRST TASK took a syscall or exception !!!! \n");
	}

	if (exc_type == EXC_SYSCALL) {
		goto syscall;
	}

	/*
	 * That's a real user exception, not a syscall exception.
	 */
	trapno = ~0UL;

	/*
	 * Forward the exception if a debugger requested it.
	 * Don't do that for "fake interrupt" exceptions of course !
	 */
	if (exc_type != 0) {
		osfmach3_trap_forward(exc_type, code, code_count, flavor,
				      old_state, &icnt,
				      new_state, ocnt);
		if (current->osfmach3.thread->exception_completed) {
			/*
			 * The exception has been fully processed
			 * by the debugger, so just return to user mode
			 * without processing it again.
			 */
			dont_mess_with_state = TRUE;
			goto ret_from_exception;
		}
		/*
		 * The debugger has sent the exception back to us.
		 * Go on with the exception processing.
		 */
	}

#if	SYSCALLTRACE
	sprintf(thread_name, "[P%d %s] EXC %d",
		current->pid,
		current->comm,
		exc_type);
	cthread_set_name(cthread_self(), thread_name);
	if (syscalltrace == -1 || syscalltrace == current->pid) {
		printk("  [P%d %s] EXC %d(%d, %d)\n",
		       current->pid, current->comm, 
		       exc_type,
		       code_count > 0 ? code[0] : 0,
		       code_count > 1 ? code[1] : 0);
		if (syscalltrace_regs)
			show_regs(regs);
	}
#endif	/* SYSCALLTRACE */
	if (code_count > 0) {
		error_code = code[0];
	}  

	switch (exc_type) {
	    case 0:
		/* fake interrupt: nothing to do */
		current->osfmach3.thread->in_fake_interrupt = TRUE;
		break;
	    case EXC_BAD_ACCESS:
		current->osfmach3.thread->fault_address = 
			code_count > 1 ? code[1] : 0;
		do_page_fault(regs,
			      current->osfmach3.thread->fault_address,
			      error_code);
		break;
	    case EXC_SYSCALL:
	    case EXC_BAD_INSTRUCTION:
	    case EXC_ARITHMETIC:
	    case EXC_EMULATION:
	    case EXC_SOFTWARE:
	    case EXC_BREAKPOINT:
	    case EXC_MACH_SYSCALL:
		do_debug(exc_type, regs, error_code);
		break;
	    case EXC_RPC_ALERT:
		/* this shouldn't reach the server */
		panic("catch_exception_raise_state: "
		      "EXC_RPC_ALERT");
	    default:
		printk("Unknown exception %d  code_count %d\n",
		       exc_type, code_count);
		for (i = 0; i < code_count; i++) {
			printk("code[%d] = %d (0x%x)\n",
			       i, code[i], code[i]);
		}
		panic("catch_exception_raise_state: "
		      "unknown exception");
	}

	goto ret_from_exception;

    syscall:
	trapno = regs->state.t1;

#if	SYSCALLTRACE
	if (trapno >= NR_syscalls || sys_call_name_table[trapno] == NULL) {
		sprintf(thread_name, "[P%d %s] %ld",
			current->pid,
			current->comm,
			trapno);
	} else {
		sprintf(thread_name, "[P%d %s] %s",
			current->pid,
			current->comm,
			sys_call_name_table[trapno]);
	}
	cthread_set_name(cthread_self(), thread_name);

	if (syscalltrace == -1 || syscalltrace == current->pid) {
		if (trapno >= NR_syscalls ||
		    sys_call_name_table[trapno] == NULL) {
			printk("  [P%d %s] %ld(%x, %x, %x, %x)\n",
			       current->pid, current->comm, 
			       trapno,
			       (int) in_state->arg0, (int) in_state->arg1,
			       (int) in_state->arg2, (int) in_state->arg3);
		} else {
			printk("  [P%d %s] %s(%x, %x, %x, %x)\n",
			       current->pid, current->comm, 
			       sys_call_name_table[trapno],
			       (int) in_state->arg0, (int) in_state->arg1,
			       (int) in_state->arg2, (int) in_state->arg3);
		}
		if (syscalltrace_regs) {
			show_regs(regs);
		}
	}
#endif	/* SYSCALLTRACE */

	if (trapno >= NR_syscalls) {
		if (trapno == (unsigned long) TASK_BY_PID_TRAPNO) 
			printk("task_by_pid_trap not implemented\n");

		error_code = -ENOSYS;
		goto ret_from_sys_call;
	} else {
		sys_ent = &sys_call_table[trapno];
		if (sys_ent->sys_call == NULL) {
			error_code = -ENOSYS;
			printk("syscall %d not implemented yet...\n", trapno);
			goto ret_from_sys_call;
		}
	}

	if (current->flags & PF_TRACESYS) {
		if (trapno != (unsigned long) TASK_BY_PID_TRAPNO) {
			syscall_trace();
		}
	}

	switch(sys_ent->sys_narg) {
	case 0:
		error_code = (*sys_ent->sys_call)(regs);
		break;
	case 1:
		error_code = (*sys_ent->sys_call)(regs->state.arg0, regs);
		break;
	case 2:
		error_code = (*sys_ent->sys_call)(regs->state.arg0, regs->state.arg1, regs);
		break;
	case 3:
		error_code = (*sys_ent->sys_call)(regs->state.arg0, regs->state.arg1, regs->state.arg2, regs);
		break;
	case 4:
		error_code = (*sys_ent->sys_call)(regs->state.arg0, regs->state.arg1, regs->state.arg2, regs->state.arg3, regs);
		break;
	case 5: {
		unsigned long a0;

		a0 = __get_user((void*)(regs->state.sp - 52), sizeof(void*));
		error_code = (*sys_ent->sys_call)(regs->state.arg0, regs->state.arg1, regs->state.arg2, regs->state.arg3, a0, regs);
		break;
	}
	case 6: {
		unsigned long a0, a1;

		a0 = __get_user((void*)(regs->state.sp - 52), sizeof(void*));
		a1 = __get_user((void*)(regs->state.sp - 56), sizeof(void*));
		error_code = (*sys_ent->sys_call)(regs->state.arg0, regs->state.arg1, regs->state.arg2, regs->state.arg3, a0, a1, regs);
		break;
	}
	default:
		printk("wrong number of arguments...\n");
	}

 ret_from_sys_call:

	if ((long) error_code < 0 &&
	    (- (long) error_code) <= _LAST_ERRNO &&
	    regs->state.flags & SS_INSYSCALL) {
		regs->state.ret0 = - (long) error_code;
		regs->state.iioq_head += 8;
		regs->state.iioq_tail += 8;
	} 
	else {
		regs->state.ret0 = error_code;
	}

	if (current->flags & PF_TRACESYS) {
		if (trapno != (unsigned long) TASK_BY_PID_TRAPNO)
			syscall_trace();
	}

 ret_from_exception:

	if (intr_count == 0 && (bh_active & bh_mask)) {
		intr_count++;
		do_bottom_half();
		intr_count--;
	}

	current->osfmach3.thread->under_server_control = FALSE;

	if (current != FIRST_TASK) {
		if (current->signal & ~current->blocked) {
			do_signal(current->blocked, regs, error_code);
		}
	}

#if	SYSCALLTRACE
	if (syscalltrace == -1 || syscalltrace == current->pid) {
		if (trapno >= NR_syscalls) {
			printk("  [P%d %s] %ld ==> %d (0x%x) %s\n",
			       current->pid, current->comm, 
 			       trapno,
			       (int) regs->state.ret0, (int) regs->state.ret0,
			       ((long) error_code < 0) ? "**ERROR**" : "");
		} else {
			printk("  [P%d %s] %s ==> %d (0x%x) %s\n",
			       current->pid, current->comm, 
			       sys_call_name_table[trapno],
			       (int) regs->state.ret0, (int) regs->state.ret0,
			       ((long) error_code < 0) ? "**ERROR**" : "");
		}
		if (syscalltrace_regs) 
			show_regs(regs);
	}

	cthread_set_name(cthread_self(), "ux_server_loop");
#endif	/* SYSCALLTRACE */

	if (!dont_mess_with_state) {
		osfmach3_ptregs_to_state(regs, out_state);
		*ocnt = icnt;
	}

	current->osfmach3.thread->regs_ptr = &current->osfmach3.thread->regs;

	uniproc_switch_to(current, FIRST_TASK);
	ASSERT(current == FIRST_TASK);

	if (use_activations) {
		uniproc_exit();
	}

	return KERN_SUCCESS;
}

kern_return_t
serv_callback_fake_interrupt(
	mach_port_t	trap_port)
{
	struct task_struct 	*task;
	struct hp700_thread_state state;
	mach_msg_type_number_t	state_count;
	kern_return_t		kr;
	thread_state_flavor_t	flavor;
	mach_port_t		trap_port_name;

	trap_port_name = (mach_port_t) serv_port_name(trap_port);
	task = (struct task_struct *) (trap_port_name - 1);
	while (task->osfmach3.thread->fake_interrupt_count > 1) {
		if (task->state == TASK_ZOMBIE) {
			task->osfmach3.thread->fake_interrupt_count--;
			return KERN_SUCCESS;
		}
		osfmach3_yield();
	}
	if (task->state == TASK_ZOMBIE) {
		task->osfmach3.thread->fake_interrupt_count--;
		return KERN_SUCCESS;
	}

	state_count = HP700_THREAD_STATE_COUNT;
	server_thread_blocking(FALSE);
	kr = thread_get_state(task->osfmach3.thread->mach_thread_port,
			      HP700_THREAD_STATE,
			      (thread_state_t) &state,
			      &state_count);
	server_thread_unblocking(FALSE);
	if (kr != KERN_SUCCESS) {
		if (kr != MACH_SEND_INVALID_DEST &&
		    kr != KERN_INVALID_ARGUMENT) {
			MACH3_DEBUG(1, kr,
				    ("serv_callback_fake_interrupt(0x%p): "
				     "thread_get_state(0%x)",
				     task,
				     task->osfmach3.thread->mach_thread_port));
			task->osfmach3.thread->fake_interrupt_count--;
			return kr;
		}
	}

	flavor = HP700_THREAD_STATE;
	if (use_activations)
		uniproc_exit();
	kr = catch_exception_raise_state(trap_port,
					 0,	/* exception type */
					 0,	/* codes[] */
					 0,	/* code_count */
					 &flavor,
					 (thread_state_t) &state,
					 HP700_THREAD_STATE_COUNT,
					 (thread_state_t) &state,
					 &state_count);
	if (use_activations)
		uniproc_enter();
	if (kr != KERN_SUCCESS) {
		if (kr == MIG_NO_REPLY) {
			/*
			 * Task has been terminated.
			 * Just return.
			 */
			return KERN_SUCCESS;
		}
		MACH3_DEBUG(1, kr, ("serv_callback_fake_interrupt(0x%p): "
				    "catch_exception_raise_state",
				    task));
		ASSERT(task->osfmach3.thread->in_fake_interrupt);
		ASSERT(task->osfmach3.thread->fake_interrupt_count > 0);
		task->osfmach3.thread->in_fake_interrupt = FALSE;
		task->osfmach3.thread->fake_interrupt_count--;
		return kr;
	}
	ASSERT(flavor == HP700_THREAD_STATE);

	kr = thread_set_state(task->osfmach3.thread->mach_thread_port,
			      HP700_THREAD_STATE,
			      (thread_state_t) &state,
			      state_count);
	if (kr != KERN_SUCCESS) {
		if (kr != MACH_SEND_INVALID_DEST &&
		    kr != KERN_INVALID_ARGUMENT) {
			MACH3_DEBUG(1, kr,
				    ("serv_callback_fake_interrupt(0x%p): "
				     "thread_set_state(0x%x)",
				     task,
				     task->osfmach3.thread->mach_thread_port));
			ASSERT(task->osfmach3.thread->in_fake_interrupt);
			ASSERT(task->osfmach3.thread->fake_interrupt_count > 0);
			task->osfmach3.thread->in_fake_interrupt = FALSE;
			task->osfmach3.thread->fake_interrupt_count--;
			return kr;
		}
	}

	ASSERT(task->osfmach3.thread->in_fake_interrupt);
	ASSERT(task->osfmach3.thread->fake_interrupt_count > 0);
	task->osfmach3.thread->in_fake_interrupt = FALSE;
	task->osfmach3.thread->fake_interrupt_count--;

	kr = thread_resume(task->osfmach3.thread->mach_thread_port);
	if (kr != KERN_SUCCESS) {
		if (kr != MACH_SEND_INVALID_DEST &&
		    kr != KERN_INVALID_ARGUMENT) {
			MACH3_DEBUG(1, kr,
				    ("serv_callback_fake_interrupt(0x%p): "
				     "thread_resume(0x%x)",
				     task,
				     task->osfmach3.thread->mach_thread_port));
		}
	}

	return KERN_SUCCESS;
}

kern_return_t
serv_callback_fake_interrupt_rpc(
	mach_port_t	trap_port)
{
	return serv_callback_fake_interrupt(trap_port);
}

kern_return_t
osfmach3_thread_set_state(
	mach_port_t	thread_port,
	struct pt_regs	*regs)
{
	kern_return_t		kr;
	struct hp700_thread_state	state;
	mach_msg_type_number_t	state_count;

	state_count = HP700_THREAD_STATE_COUNT;
	server_thread_blocking(FALSE);
	kr = thread_get_state(thread_port,
			      HP700_THREAD_STATE,
			      (thread_state_t) &state,
			      &state_count);
	server_thread_unblocking(FALSE);
	if (kr != KERN_SUCCESS) {
		if (kr != MACH_SEND_INVALID_DEST &&
		    kr != KERN_INVALID_ARGUMENT) {
			MACH3_DEBUG(1, kr, ("osfmach3_thread_set_state: "
					    "thread_get_state(0%x)",
					    thread_port));
			return kr;
		}
	}

	/*
	 * The following registers might have been modified by
	 * the Linux server.
	 */
	state.flags = regs->state.flags;
	state.r1 = regs->state.r1;
	state.rp = regs->state.rp;
	state.r3 = regs->state.r3;
	state.r4 = regs->state.r4;
	state.r5 = regs->state.r5;
	state.r6 = regs->state.r6;
	state.r7 = regs->state.r7;
	state.r8 = regs->state.r8;
	state.r9 = regs->state.r9;
	state.r10 = regs->state.r10;
	state.r11 = regs->state.r11;
	state.r12 = regs->state.r12;
	state.r13 = regs->state.r13;
	state.r14 = regs->state.r14;
	state.r15 = regs->state.r15;
	state.r16 = regs->state.r16;
	state.r17 = regs->state.r17;
	state.r18 = regs->state.r18;
	state.t4 = regs->state.t4;
	state.t3 = regs->state.t3;
	state.t2 = regs->state.t2;
	state.t1 = regs->state.t1;
	state.arg3 = regs->state.arg3;
	state.arg2 = regs->state.arg2;
	state.arg1 = regs->state.arg1;
	state.arg0 = regs->state.arg0;
	state.dp = regs->state.dp;
	state.ret0 = regs->state.ret0;
	state.ret1 = regs->state.ret1;
	state.sp = regs->state.sp;
	state.r31 = regs->state.r31;
	state.iioq_head = regs->state.iioq_head;
	state.iioq_tail = regs->state.iioq_tail;
	state.sar = regs->state.sar;

	/* 
	 * what about space registers and ipsw ?
	 */

	kr = thread_set_state(thread_port,
			      HP700_THREAD_STATE,
			      (thread_state_t) &state,
			      HP700_THREAD_STATE_COUNT);
	if (kr != KERN_SUCCESS) {
		if (kr != MACH_SEND_INVALID_DEST &&
		    kr != KERN_INVALID_ARGUMENT) {
			MACH3_DEBUG(1, kr, ("osfmach3_thread_set_state: "
					    "thread_set_state(0x%x)",
					    thread_port));
			return kr;
		}
	}

	return KERN_SUCCESS;
}

kern_return_t
osfmach3_trap_unwind(
	mach_port_t			trap_port,
	exception_type_t		exc_type,
	exception_data_t		code,
	mach_msg_type_number_t		code_count,
	int				*flavor,
	thread_state_t			old_state,
	mach_msg_type_number_t		icnt,
	thread_state_t			new_state,
	mach_msg_type_number_t 		*ocnt)
{
	printk("trap_unwind not implemented\n");
	return KERN_SUCCESS;
}

