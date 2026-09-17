/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


/*
 *  Copyright (C) 1995  Linus Torvalds
 */

/*
 * This file handles the architecture-dependent parts of process handling..
 */

#include <mach/mach_interface.h>
#include <mach/mach_port.h>
#include <mach/mach_host.h>

#include <osfmach3/mach_init.h>
#include <osfmach3/server_thread.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/user_memory.h>
#include <hp_pa/psw.h>

#define __KERNEL_SYSCALLS__
#include <stdarg.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/ptrace.h>
#include <linux/malloc.h>
#include <linux/ldt.h>
#include <linux/user.h>
#include <linux/a.out.h>
#include <linux/interrupt.h>
#include <linux/config.h>
#include <linux/unistd.h>
#include <linux/delay.h>

void copy_thread(int nr, unsigned long clone_flags, unsigned long sp, 
		 struct task_struct * p, struct pt_regs * regs)
{
	kern_return_t kr;
	mach_port_t child_task, child_thread;
	struct hp700_thread_state child_state;
	boolean_t inherit;
	boolean_t do_setup_thread;
	struct task_struct *parent;

	parent = p->p_opptr;

	if (clone_flags & CLONE_VM) {
		/*
		 * Use the same Mach task as the parent:
		 * just create a Mach thread.
		 */
		p->osfmach3.task->mach_task_count++;
		child_task = p->osfmach3.task->mach_task_port;
	} else {
		/*
		 * Create a new Mach task.
		 */
		if (clone_flags & CLONING_INIT) {
			/* creating an empty task: don't inherit memory */
			inherit = FALSE;
		} else {
			inherit = TRUE;
		}

		kr = task_create(parent->osfmach3.task->mach_task_port,
				 (ledger_port_array_t) 0, 0,
				 inherit, &child_task);
		if (kr != KERN_SUCCESS) {
			MACH3_DEBUG(0, kr, ("copy_thread: task_create"));
			panic("can't create task.");
		}

		p->osfmach3.task = (struct osfmach3_mach_task_struct *)
			kmalloc(sizeof (*p->osfmach3.task), GFP_KERNEL);
		if (!p->osfmach3.task) {
			panic("copy_thread: "
			      "can't allocate osfmach3_mach_task_struct.\n");
		}
		p->osfmach3.task->mach_task_count = 1;
		p->osfmach3.task->mach_task_port = child_task;
		p->osfmach3.task->mach_aware =
			parent->osfmach3.task->mach_aware;
		user_memory_task_init(p);

	}
	
	/* update the "mm_mach_task" pointer */
	p->mm->mm_mach_task = p->osfmach3.task;

	if (clone_flags & CLONING_KERNEL) {
		child_thread = MACH_PORT_NULL;
		do_setup_thread = TRUE;
	} else if (parent != current) {
		/*
		 * Asynchronous fork: creating a Linux task for an
		 * existing Mach thread.
		 * See catch_exception_raise_state_identity for this
		 * special use of "esp".
		 */
		printk("does it work ?\n");
		child_thread = (mach_port_t) sp;
		do_setup_thread = FALSE;	/* the thread already exists */
	} else {
		kr = thread_create(child_task, &child_thread);
		if (kr != KERN_SUCCESS) {
			MACH3_DEBUG(0, kr, ("copy_thread: thread_create"));
			panic("can't create thread");
		}
		do_setup_thread = TRUE;
	}

	p->osfmach3.thread = (struct osfmach3_mach_thread_struct *)
		kmalloc(sizeof (*p->osfmach3.thread), GFP_KERNEL);

	if (!p->osfmach3.thread) {
		panic("copy_thread: "
		      "can't allocate osfmach3_mach_thread_struct.\n");
	}
	p->osfmach3.thread->mach_thread_count = 1;
	p->osfmach3.thread->mach_thread_port = child_thread;
	p->osfmach3.thread->active_on_cthread = CTHREAD_NULL;
	p->osfmach3.thread->reg_fs = USER_DS;

	if (do_setup_thread) {
		osfmach3_set_priority(p);
	}

	/* setup notification port */
	osfmach3_notify_register(p);

	/* setup exception ports */
	osfmach3_trap_setup_task(p, EXCEPTION_STATE, HP700_THREAD_STATE);

	condition_init(&p->osfmach3.thread->mach_wait_channel);

	/* setup saved regs for ptrace */
	p->osfmach3.thread->regs = parent->osfmach3.thread->regs;
	p->osfmach3.thread->regs_ptr = &p->osfmach3.thread->regs;

	/* setup fake interrupt fields */
	p->osfmach3.thread->under_server_control = FALSE;
	p->osfmach3.thread->in_interrupt_list = FALSE;
	p->osfmach3.thread->fake_interrupt_count = 0;
	p->osfmach3.thread->in_fake_interrupt = FALSE;

	if (clone_flags & CLONING_KERNEL) {
		void **args;

		/*
		 * Creating a kernel "thread" via kernel_thread().
		 * Start a server cthread...
		 */
		ASSERT(sp == 0);
		args = (void **) regs;	/* see kernel_thread(). */
		args[2] = (void *) p;
		server_thread_start(server_thread_bootstrap, (void *) args);
	} else {
		launch_new_ux_server_loop(p);
	}

	if (clone_flags & CLONING_INIT) {
		/*
		 * Creating an empty task: no state yet, so don't initialize
		 * the registers nor resume the thread.
		 */
	} else if (clone_flags & CLONING_KERNEL) {
		/*
		 * Creating a kernel thread: its state will be setup by
		 * the cthread_fork() in server_thread_start().
		 */
	} else if (do_setup_thread) {
#if 1
		mach_msg_type_number_t	count; 

		count = HP700_THREAD_STATE_COUNT;
		kr = thread_get_state(child_thread, HP700_THREAD_STATE,
				      (thread_state_t)&child_state, &count);
		if (kr != KERN_SUCCESS) {
			MACH3_DEBUG(0, kr, ("copy_thread: thread_get_state"));
			panic("can't get thread state");
		}
#endif

		child_state.rp = regs->state.rp;
		child_state.r3 = regs->state.r3;
		child_state.r4 = regs->state.r4;
		child_state.r5 = regs->state.r5;
		child_state.r6 = regs->state.r6;
		child_state.r7 = regs->state.r7;
		child_state.r8 = regs->state.r8;
		child_state.r9 = regs->state.r9;
		child_state.r10 = regs->state.r10;
		child_state.r11 = regs->state.r11;
		child_state.r12 = regs->state.r12;
		child_state.r13 = regs->state.r13;
		child_state.r14 = regs->state.r14;
		child_state.r15 = regs->state.r15;
		child_state.r16 = regs->state.r16;
		child_state.r17 = regs->state.r17;
		child_state.r18 = regs->state.r18;
		child_state.dp = regs->state.dp;
		child_state.ret0 = 0;		/* result from fork */
		child_state.ret1 = regs->state.ret1;
		child_state.sp = sp;
		child_state.r31 = regs->state.r31;
		child_state.iioq_head = regs->state.iioq_head;
		child_state.iioq_tail = regs->state.iioq_tail;
		child_state.ipsw &= ~PSW_CB;
		child_state.fpu = regs->state.fpu;

		kr = thread_set_state(child_thread, HP700_THREAD_STATE,
				      (thread_state_t) &child_state,
				      HP700_THREAD_STATE_COUNT);
		if (kr != KERN_SUCCESS) {
			MACH3_DEBUG(0, kr, ("copy_thread: thread_set_state"));
			panic("can't set thread state");
		}

#if 0
		if(regs->state.fpu) {
			struct hp700_float_state float_state;

			ASSERT(regs->state.fpu == 1);

			count = HP700_FLOAT_STATE_COUNT;
			
			kr = thread_get_state(current->osfmach3.thread->mach_thread_port,
					      HP700_FLOAT_STATE,
					      (thread_state_t)&float_state, &count);
		
			if (kr != KERN_SUCCESS) {
				MACH3_DEBUG(0, kr, ("copy_thread: thread_get_state"));
				panic("Cannot get float state");
				}
			
			kr = thread_set_state(child_thread, HP700_FLOAT_STATE,
					      (thread_state_t)&float_state,
					      HP700_FLOAT_STATE_COUNT);
			if (kr != KERN_SUCCESS) {
				MACH3_DEBUG(0, kr, ("copy_thread: thread_set_state"));
				panic("Cannot set float state");
			}
		}
#endif

	}
}

int
dump_fpu(void)
{
	return (1);
}

void show_regs(struct pt_regs * regs)
{
	printk("FLAGS: 0x%08lx  R1: 0x%08lx    R2: 0x%08lx    R3: 0x%08lx\n",
	       regs->state.flags, regs->state.r1, regs->state.rp, regs->state.r3);
	printk("R4: 0x%08lx    R5: 0x%08lx    R6: 0x%08lx    R7: 0x%08lx\n",
	       regs->state.r4, regs->state.r5, regs->state.r6, regs->state.r7);
	printk("R8: 0x%08lx    R9: 0x%08lx    R10: 0x%08lx    R11: 0x%08lx\n",
	       regs->state.r8, regs->state.r9, regs->state.r10, regs->state.r11);
	printk("R12: 0x%08lx    R13: 0x%08lx    R14: 0x%08lx    R15: 0x%08lx\n",
	       regs->state.r12, regs->state.r13, regs->state.r14, regs->state.r15);
	printk("R16: 0x%08lx    R17: 0x%08lx    R18: 0x%08lx     T4: 0x%08lx\n",
	       regs->state.r16, regs->state.r17, regs->state.r18, regs->state.t4);
	printk("T3: 0x%08lx    T2: 0x%08lx    T1: 0x%08lx    ARG3: 0x%08lx\n",
	       regs->state.t3, regs->state.t2, regs->state.t1, regs->state.arg3);
	printk("ARG2: 0x%08lx    ARG1: 0x%08lx    ARG0: 0x%08lx    DP: 0x%08lx\n",
	       regs->state.arg2, regs->state.arg1, regs->state.arg0, regs->state.dp);
	printk("RET0: 0x%08lx    RET1: 0x%08lx    SP: 0x%08lx    R31: 0x%08lx\n",
	       regs->state.ret0, regs->state.ret1, regs->state.sp, regs->state.r31);
	printk("OQH: 0x%08lx    OQT: 0x%08lx\n", regs->state.iioq_head, regs->state.iioq_tail);
}

void start_thread(struct pt_regs * regs, unsigned long pc, unsigned long sp)
{
	regs->state.sp = sp;
	regs->state.iioq_head = LOWER_PRIV(pc);
	regs->state.iioq_tail = LOWER_PRIV(pc+4);
	regs->state.fpu = 0;
}

asmlinkage int sys_fork(struct pt_regs *regs)
{
	return do_fork(SIGCHLD, regs->state.sp, regs);
}

asmlinkage int sys_execve(unsigned long a0, unsigned long a1, unsigned long a2, struct pt_regs *regs)
{
	int error;
	char * filename;

	error = getname((char *) a0, &filename);
	if (error) {
		return error;
	}

	error = do_execve(filename, (char **) a1, (char **) a2, regs);
	putname(filename);
	return error;
}

