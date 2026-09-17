/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

/*
 *  Copyright (C) 1994 by Hamish Macdonald
 *  Taken from linux/kernel/ptrace.c and modified for M680x0.
 *  linux/kernel/ptrace.c is by Ross Biro 1/23/92, edited by Linus Torvalds
 *
 * Adapted from 'linux/arch/m68k/kernel/ptrace.c'
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file README.legal in the main directory of
 * this archive for more details.
 */

#include <linux/autoconf.h>

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/user.h>

#include <hp_pa/psw.h>

static inline struct task_struct * get_task(int pid)
{
	int i;

	for (i = 1; i < NR_TASKS; i++) {
		if (task[i] != NULL && (task[i]->pid == pid))
			return task[i];
	}
	return NULL;
}

/*
 * Get contents of register REGNO in task TASK.
 */
static inline long get_reg(struct task_struct *task, int regno)
{
	unsigned long *regs = (unsigned long*)task->osfmach3.thread->regs_ptr;

	if (regno < PT_FP0) {
		return (regs[regno]);
	} 

	if(regs[PT_FPU]) {
		if(regs[PT_FPU] != 1)
			printk("get_reg wrong offset\n");

		if (regno < PT_FP31) {
			mach_msg_type_number_t regs_count;
			unsigned long buf[HP700_FLOAT_STATE_COUNT];

			regs_count = HP700_FLOAT_STATE_COUNT;
			if(thread_get_state(task->osfmach3.thread->mach_thread_port,
					    HP700_FLOAT_STATE,
					    (thread_state_t)buf, &regs_count)) 
				return 0;

			return buf[regno];
		}
	}

	return (0);
}

/*
 * Write contents of register REGNO in task TASK.
 */
static inline int put_reg(struct task_struct *task, int regno,
			  unsigned long data)
{
	unsigned long *regs = (unsigned long*)task->osfmach3.thread->regs_ptr;

	if (regno < PT_FP0) {
		regs[regno] = data;
	} 
	else { /* Invalid register */
		return (-1);
	}
	return (0);
}

static inline
void
set_single_step(struct task_struct *task)
{
	struct pt_regs *regs = task->osfmach3.thread->regs_ptr;
	regs->state.ipsw |= PSW_R;
	regs->state.rctr = 0;
}

static inline
void
clear_single_step(struct task_struct *task)
{
	struct pt_regs *regs = task->osfmach3.thread->regs_ptr;
	regs->state.ipsw &= ~PSW_R;
}

extern unsigned long get_long(struct task_struct * tsk,
			      struct vm_area_struct * vma,
			      unsigned long addr);
extern void put_long(struct task_struct *tsk,
		     struct vm_area_struct * vma,
		     unsigned long addr,
		     unsigned long data);


static struct vm_area_struct * find_extend_vma(struct task_struct * tsk, unsigned long addr)
{
	struct vm_area_struct * vma;

	addr &= PAGE_MASK;
	vma = find_vma(tsk->mm,addr);
	if (!vma)
		return NULL;
	if (vma->vm_start <= addr) {
		return vma;
	}
	if (!(vma->vm_flags & VM_GROWSDOWN))
		return NULL;
	if (vma->vm_end - addr > tsk->rlim[RLIMIT_STACK].rlim_cur)
		return NULL;
	vma->vm_offset -= vma->vm_start - addr;
	vma->vm_start = addr;
	return vma;
}

/*
 * This routine checks the page boundaries, and that the offset is
 * within the task area. It then calls get_long() to read a long.
 */
static int read_long(struct task_struct * tsk, unsigned long addr,
	unsigned long * result)
{
	struct vm_area_struct * vma = find_extend_vma(tsk, addr);

	if (!vma)
		return -EIO;
	if ((addr & ~PAGE_MASK) > PAGE_SIZE-sizeof(long)) {
		unsigned long low,high;
		struct vm_area_struct * vma_high = vma;

		if (addr + sizeof(long) >= vma->vm_end) {
			vma_high = vma->vm_next;
			if (!vma_high || vma_high->vm_start != vma->vm_end)
				return -EIO;
		}
		low = get_long(tsk, vma, addr & ~(sizeof(long)-1));
		high = get_long(tsk, vma_high, (addr+sizeof(long)) & ~(sizeof(long)-1));
		switch (addr & (sizeof(long)-1)) {
			case 1:
				low >>= 8;
				low |= high << 24;
				break;
			case 2:
				low >>= 16;
				low |= high << 16;
				break;
			case 3:
				low >>= 24;
				low |= high << 8;
				break;
		}
		*result = low;
	} else
		*result = get_long(tsk, vma, addr);
	return 0;
}

/*
 * This routine checks the page boundaries, and that the offset is
 * within the task area. It then calls put_long() to write a long.
 */
static int write_long(struct task_struct * tsk, unsigned long addr,
	unsigned long data)
{
	struct vm_area_struct * vma = find_extend_vma(tsk, addr);

	if (!vma)
		return -EIO;
	if ((addr & ~PAGE_MASK) > PAGE_SIZE-sizeof(long)) {
		unsigned long low,high;
		struct vm_area_struct * vma_high = vma;

		if (addr + sizeof(long) >= vma->vm_end) {
			vma_high = vma->vm_next;
			if (!vma_high || vma_high->vm_start != vma->vm_end)
				return -EIO;
		}
		low = get_long(tsk, vma, addr & ~(sizeof(long)-1));
		high = get_long(tsk, vma_high, (addr+sizeof(long)) & ~(sizeof(long)-1));
		switch (addr & (sizeof(long)-1)) {
			case 0: /* shouldn't happen, but safety first */
				low = data;
				break;
			case 1:
				low &= 0x000000ff;
				low |= data << 8;
				high &= ~0xff;
				high |= data >> 24;
				break;
			case 2:
				low &= 0x0000ffff;
				low |= data << 16;
				high &= ~0xffff;
				high |= data >> 16;
				break;
			case 3:
				low &= 0x00ffffff;
				low |= data << 24;
				high &= ~0xffffff;
				high |= data >> 8;
				break;
		}
		put_long(tsk, vma, addr & ~(sizeof(long)-1),low);
		put_long(tsk, vma_high, (addr+sizeof(long)) & ~(sizeof(long)-1),high);
	} else
		put_long(tsk, vma, addr, data);
	return 0;
}

asmlinkage int sys_ptrace(long request, long pid, long addr, long data)
{
	struct task_struct *child;
	struct user * dummy;

	dummy = NULL;

	if (request == PTRACE_TRACEME) {
		/* are we already being traced? */
		if (current->flags & PF_PTRACED)
			return -EPERM;
		/* set the ptrace bit in the process flags. */
		current->flags |= PF_PTRACED;
		return 0;
	}
	if (pid == 1)		/* you may not mess with init */
		return -EPERM;
	if (!(child = get_task(pid)))
		return -ESRCH;
	if (request == PTRACE_ATTACH) {
		if (child == current)
			return -EPERM;
		if ((!child->dumpable ||
		     (current->uid != child->euid) ||
		     (current->uid != child->uid) ||
		     (current->gid != child->egid) ||
		     (current->gid != child->gid)) && !suser())
			return -EPERM;
		/* the same process cannot be attached many times */
		if (child->flags & PF_PTRACED)
			return -EPERM;
		child->flags |= PF_PTRACED;
		if (child->p_pptr != current) {
			REMOVE_LINKS(child);
			child->p_pptr = current;
			SET_LINKS(child);
		}
		send_sig(SIGSTOP, child, 1);
		return 0;
	}
	if (!(child->flags & PF_PTRACED))
		return -ESRCH;
	if (child->state != TASK_STOPPED) {
		if (request != PTRACE_KILL)
			return -ESRCH;
	}
	if (child->p_pptr != current)
		return -ESRCH;
	
	switch (request)
	{
	case PTRACE_PEEKTEXT:
	case PTRACE_PEEKDATA:
	{
		unsigned long tmp;
		int res;
		
		res = read_long(child, addr, &tmp);
		if (res < 0)
			return res;
		res = verify_area(VERIFY_WRITE, (void *) data, sizeof(long));
		if (!res)
			put_user(tmp, (unsigned long *) data);
		return res;
	}

	/* read the word at location addr in the USER area. */
	case PTRACE_PEEKUSR: {
		unsigned long tmp;
		int res;
			
		if ((addr & 3) || addr < 0 || addr >= sizeof(struct user))
			return -EIO;
		
		res = verify_area(VERIFY_WRITE, (void *) data, sizeof(long));
		if (res)
			return res;
		tmp = 0;  /* Default return condition */
		addr = addr >> 2; /* temporary hack. */
		if (addr < PT_FP31) {
			tmp = get_reg(child, addr);
			put_user(tmp,(unsigned long *) data);
		}
		else
			return -EIO;

		return 0;
	}
		
      /* when I and D space are separate, this will have to be fixed. */
	case PTRACE_POKETEXT: /* write the word at location addr. */
	case PTRACE_POKEDATA:
		return write_long(child,addr,data);

	case PTRACE_SYSCALL: /* continue and stop at next (return from) syscall */
	case PTRACE_CONT: { /* restart after signal. */
		if ((unsigned long) data >= NSIG)
			return -EIO;
		if (request == PTRACE_SYSCALL)
			child->flags |= PF_TRACESYS;
		else
			child->flags &= ~PF_TRACESYS;
		child->exit_code = data;
		wake_up_process(child);
		/* make sure the single step bit is not set. */
		clear_single_step(child);
		return 0;
	}

		/*
		 * make the child exit.  Best I can do is send it a sigkill. 
		 * perhaps it should be put in the status that it wants to 
		 * exit.
		 */
	case PTRACE_KILL: {
		if (child->state == TASK_ZOMBIE) /* already dead */
			return 0;
		wake_up_process(child);
		child->exit_code = SIGKILL;
		/* make sure the single step bit is not set. */
		clear_single_step(child);
		return 0;
	}

	case PTRACE_SINGLESTEP: {  /* set the trap flag. */
		if ((unsigned long) data >= NSIG)
			return -EIO;
		child->flags &= ~PF_TRACESYS;
		set_single_step(child);
		wake_up_process(child);
		child->exit_code = data;
		/* give it a chance to run. */
		return 0;
	}

	case PTRACE_DETACH: { /* detach a process that was attached. */
		if ((unsigned long) data >= NSIG)
			return -EIO;
		child->flags &= ~(PF_PTRACED|PF_TRACESYS);
		wake_up_process(child);
		child->exit_code = data;
		REMOVE_LINKS(child);
		child->p_pptr = child->p_opptr;
		SET_LINKS(child);
		/* make sure the single step bit is not set. */
		clear_single_step(child);
		return 0;
	}

	default:
		printk("ptrace: request %d not implemented\n", (int)request);
		return -EIO;
        }			
}

asmlinkage void syscall_trace(void)
{
	if ((current->flags & (PF_PTRACED|PF_TRACESYS))
			!= (PF_PTRACED|PF_TRACESYS))
		return;
	current->exit_code = SIGTRAP;
	current->state = TASK_STOPPED;
	notify_parent(current, SIGCHLD);
	schedule();
	/*
	 * this isn't the same as continuing with a signal, but it will do
	 * for normal use.  strace only continues with a signal if the
	 * stopping signal is not SIGTRAP.  -brl
	 */
	if (current->exit_code)
		current->signal |= (1 << (current->exit_code - 1));
	current->exit_code = 0;
	return;
}
