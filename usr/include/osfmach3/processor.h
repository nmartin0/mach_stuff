/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: processor.h,v $
 * Revision 1.1.2.1  1996/09/09  16:58:18  barbou
 * 	Don't include mach/exception.h.
 * 	[96/09/04            barbou]
 *
 * 	Moved prototype for osfmach3_trap_setup_task to osfmach3/sched.h.
 * 	[96/09/04            barbou]
 *
 * 	Fixed prototype for osfmach3_trap_setup_task.
 * 	[96/09/03            barbou]
 *
 * 	Added prototype for osfmach3_trap_setup_task.
 * 	[1996/09/02  16:26:54  barbou]
 *
 * 	Added prototype for osfmach3_thread_set_state.
 * 	[96/09/02            barbou]
 *
 * 	Added support for thread_saved_pc().
 * 	[96/08/21            barbou]
 *
 * 	Added field "wchan" in "thread_struct".
 * 	[96/08/21            barbou]
 *
 * 	Created.
 * 	[1996/08/21  15:50:22  barbou]
 *
 * $EndLog$
 */

#ifndef __OSFMACH3_PROCESSOR_H
#define __OSFMACH3_PROCESSOR_H

#define EISA_bus__is_a_macro	/* for versions in ksyms.c */
#define MCA_bus__is_a_macro	/* for versions in ksyms.c */

#define wp_works_ok 1
#define wp_works_ok__is_a_macro	/* for versions in ksyms.c */

struct thread_struct {
	unsigned long	wchan;
	unsigned long	saved_pc;
};

#define INIT_MMAP { &init_mm, VM_MIN_ADDRESS, VM_MAX_ADDRESS, PAGE_SHARED, VM_READ | VM_WRITE | VM_EXEC }

#define INIT_TSS { 							\
	0, 		/* wchan */					\
	0 		/* saved_pc */					\
}

extern void start_thread(struct pt_regs *regs,
			 unsigned long pc,
			 unsigned long sp);

/*
 * Return saved PC of a blocked thread.
 */
extern inline unsigned long thread_saved_pc(struct thread_struct *t)
{
	return t->saved_pc;
}

extern kern_return_t osfmach3_thread_set_state(mach_port_t thread_port,
					       struct pt_regs *regs);

#endif	/* __OSFMACH3_PROCESSOR_H */
