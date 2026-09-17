/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

/*
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 */

#include <linux/autoconf.h>

#include <mach/mach_interface.h>

#include <osfmach3/mach3_debug.h>

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <asm/a.out.h>

#ifdef	CONFIG_OSFMACH3_DEBUG
#define FAULT_DEBUG	1
#endif	/* CONFIG_OSFMACH3_DEBUG */

#if	FAULT_DEBUG
int fault_debug = 0;
#endif	/* FAULT_DEBUG */

extern void die_if_kernel(char *, struct pt_regs *, long);
extern void do_page_fault(struct pt_regs *, unsigned long, unsigned long);

/*
 * This routine handles page faults.  It determines the address,
 * and the problem, and then passes it off to one of the appropriate
 * routines.
 *
 * The error_code parameter just the same as in the i386 version:
 *
 *	bit 0 == 0 means no page found, 1 means protection fault
 *	bit 1 == 0 means read, 1 means write
 *	bit 2 == 0 means kernel, 1 means user-mode
 */
void do_page_fault(struct pt_regs *regs, unsigned long address, unsigned long error_code)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	unsigned long stack_base;

	stack_base = STACK_TOP - current->rlim[RLIMIT_STACK].rlim_cur;

#if	FAULT_DEBUG
	if (fault_debug) {
		printk("do_page_fault: fault at 0x%lx\n", address);
	}
#endif	/* FAULT_DEBUG */

	down(&mm->mmap_sem);

	if((address < stack_base) || (address >= STACK_TOP)) 
		goto bad_area;

	vma = find_vma_intersection(mm, stack_base, address);

	if(!vma)
		goto bad_area;

	if (vma->vm_flags & VM_GROWSDOWN) {      /* VM_GROWSUP would be the proper word! */
		if (! expand_stack(vma, address)) {
#if	FAULT_DEBUG
			if (fault_debug) {
				printk("do_page_fault: vma: start=0x%lx,end=0x%lx,off=0x%lx\n",
				       vma->vm_start, vma->vm_end, vma->vm_offset);
			}
#endif	/* FAULT_DEBUG */
			
			up(&mm->mmap_sem);
			return;
		}
	}

/*
 * Something tried to access memory that isn't in our memory map..
 * Fix it, but check if it's kernel or user first..
 */
bad_area:
	up(&mm->mmap_sem);
#if	FAULT_DEBUG
	if (fault_debug) {
		printk("do_page_fault: bad area, SIGSEGV\n");
	}
#endif	/* FAULT_DEBUG */

	if (current != &init_task) {
		current->osfmach3.thread->fault_address = address;
		current->osfmach3.thread->error_code = error_code;
		current->osfmach3.thread->trap_no = 14;
		force_sig(SIGSEGV, current);
		return;
	}
	die_if_kernel("Oops", regs, error_code);
	do_exit(SIGKILL);
}


