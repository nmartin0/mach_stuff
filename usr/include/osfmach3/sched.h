/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: sched.h,v $
 * Revision 1.1.2.6  1997/10/30  16:04:11  barbou
 * 	Added prototypes for osfmach3_update_thread_info() and
 * 	osfmach3_update_cpu_load_info().
 * 	[1997/10/30  15:51:25  barbou]
 *
 * Revision 1.1.2.5  1996/11/18  18:15:55  barbou
 * 	Renamed the "count" fields. Added an "exception_completed" field in the
 * 	"osfmach3_mach_thread_struct".
 * 	Added prototypes for osfmach3_do_fork, osfmach3_synchronize_exit,
 * 	osfmach3_trap_forward, osfmach3_trap_unwind, osfmach3_notify_register
 * 	and osfmach3_notify_deregister.
 * 	[1996/11/18  17:13:37  barbou]
 * 
 * Revision 1.1.2.4  1996/10/29  18:27:18  barbou
 * 	Added protos for osfmach3_update_vm_info and osfmach3_update_load_info.
 * 	[96/10/29            barbou]
 * 
 * Revision 1.1.2.3  1996/10/24  15:55:19  barbou
 * 	Added "in_fake_interrupt".
 * 	[1996/10/24  15:18:57  barbou]
 * 
 * Revision 1.1.2.2  1996/09/18  14:24:47  barbou
 * 	Initialize init_osfmach3_thread.regs with machine-dependent INIT_PTREGS.
 * 	[96/09/11            barbou]
 * 
 * Revision 1.1.2.1  1996/09/09  16:58:27  barbou
 * 	Adapted for clone().
 * 	[1996/09/05  17:26:28  barbou]
 * 
 * 	Added prototype for osfmach3_trap_mach_aware.
 * 	[96/09/04            barbou]
 * 
 * 	Fixed osfmach3_trap_init prototype.
 * 	[96/09/04            barbou]
 * 
 * 	Include mach/exception.h.
 * 	[96/09/04            barbou]
 * 
 * 	Added prototypes for osfmach3_trap_setup_task and osfmach3_trap_init.
 * 	[96/09/04            barbou]
 * 
 * 	Define "task_struct_t" before including osfmach3/user_memory.h.
 * 	[96/09/02            barbou]
 * 
 * 	Added task_struct_t type definition from osfmach3/user_memory.h.
 * 	[96/09/02            barbou]
 * 
 * 	Added prototypes for osfmach3_set_priority and mach_trap_terminate_task.
 * 	[96/08/26            barbou]
 * 
 * 	More prototypes.
 * 	[96/08/21            barbou]
 * 
 * 	Added various prototypes.
 * 	[96/08/21            barbou]
 * 
 * 	Got rid of INIT_PTREGS.
 * 	[96/08/21            barbou]
 * 
 * 	Got rid of the osfmach3_mach_machdep_struct.
 * 	[96/08/21            barbou]
 * 
 * 	Created.
 * 	[1996/08/21  15:08:45  barbou]
 * 
 * $EndLog$
 */

#ifndef _OSFMACH3_SCHED_H
#define _OSFMACH3_SCHED_H

#include <mach/port.h>
#include <mach/exception.h>
#include <cthreads.h>
typedef struct task_struct *task_struct_t;
typedef struct osfmach3_mach_task_struct *osfmach3_mach_task_struct_t;
#include <osfmach3/user_memory.h>
#include <osfmach3/segment.h>

#include <linux/time.h>

#include <asm/ptrace.h>

struct osfmach3_mach_task_struct {
	int		mach_task_count;	/* ref count for clone() */
	mach_port_t	mach_task_port;		/* handle on the Mach task */
	boolean_t	mach_aware;		/* task uses Mach services */
	struct user_memory *user_memory;	/* user memory map */
	struct user_memory *um_hint1;
	struct user_memory *um_hint2;
};
#define INIT_OSFMACH3_TASK						      \
{									      \
	1,					/* mach_task_count */	      \
	MACH_PORT_NULL,				/* mach_task_port */	      \
	FALSE,					/* mach_aware */	      \
	NULL,					/* user_memory */	      \
	NULL,					/* um_hint1 */		      \
	NULL,					/* um_hint2 */		      \
}

struct osfmach3_mach_thread_struct {
	int		mach_thread_count;	/* ref count for async ops */
	mach_port_t	mach_thread_port;	/* handle on the Mach thread */
	mach_port_t	mach_trap_port;		/* exception port */
	unsigned long	mach_trap_port_srights;	/* # send rights on trap port */
	struct pt_regs	*regs_ptr;		/* pointer to user regs */
	struct pt_regs	regs;			/* user regs (sometimes) */
	unsigned long	trap_no;
	unsigned long	error_code;
	unsigned long	fault_address;
	unsigned long	reg_fs;			/* dummy fs segment register */
	cthread_t	active_on_cthread;	/* cthread running this task */
	boolean_t	under_server_control;
	boolean_t	in_interrupt_list;
	int		fake_interrupt_count;
	boolean_t	in_fake_interrupt;	/* currently dealing with one */
	queue_chain_t	interrupt_list;
	boolean_t	exception_completed;	/* exc processed externally */
	struct condition mach_wait_channel;	/* to block the Mach thread */
};
#define INIT_OSFMACH3_THREAD						      \
{									      \
	1,					/* count */		      \
	MACH_PORT_NULL,				/* mach_thread_port */	      \
	MACH_PORT_NULL,				/* mach_trap_port */	      \
	0,					/* mach_trap_port_srights */  \
	&init_osfmach3_thread.regs,		/* regs_ptr */		      \
	INIT_PTREGS,				/* regs */		      \
	0,					/* trap_no */		      \
	0,					/* error_code */	      \
	0,					/* fault_address */	      \
	USER_DS,				/* reg_fs */		      \
	CTHREAD_NULL,				/* active_on_cthread */	      \
	FALSE,					/* under_server_control */    \
	FALSE,					/* in_interrupt_list */	      \
	0,					/* fake_interrupt_count */    \
	FALSE,					/* in_fake_interrupt */	      \
	{ &init_osfmach3_thread.interrupt_list,			 	      \
	  &init_osfmach3_thread.interrupt_list }, /* interrupt_list */        \
	FALSE,					/* exception_completed */     \
	CONDITION_INITIALIZER			/* mach_wait_channel */	      \
}

struct osfmach3_task_struct {
	struct osfmach3_mach_task_struct	*task;
	struct osfmach3_mach_thread_struct	*thread;
};

#define INIT_OSFMACH3 \
{ \
	&init_osfmach3_task, \
	&init_osfmach3_thread \
},

extern void osfmach3_setrun(struct task_struct *tsk);
extern void osfmach3_yield(void);

extern int osfmach3_do_fork(struct task_struct *parent,
			    unsigned long clone_flags,
			    unsigned long usp,
			    struct pt_regs *regs);
extern void osfmach3_synchronize_exit(struct task_struct *tsk);
extern void osfmach3_terminate_task(struct task_struct *tsk);
extern void osfmach3_update_thread_info(struct task_struct *tsk);
extern void osfmach3_update_task_info(struct task_struct *tsk);
extern void osfmach3_fork_resume(struct task_struct *p,
				 unsigned long clone_flags);
extern void osfmach3_fork_cleanup(struct task_struct *p,
				  unsigned long clone_flags);
extern void osfmach3_set_priority(struct task_struct *tsk);

extern void osfmach3_trap_init(exception_behavior_t behavior,
			       thread_state_flavor_t flavor);
extern void osfmach3_trap_terminate_task(struct task_struct *tsk);
extern void osfmach3_trap_setup_task(struct task_struct *task,
				     exception_behavior_t behavior,
				     thread_state_flavor_t flavor);
extern void osfmach3_trap_mach_aware(struct task_struct *task,
				     boolean_t mach_aware,
				     exception_behavior_t behavior,
				     thread_state_flavor_t flavor);
extern void osfmach3_trap_forward(exception_type_t exc_type,
				  exception_data_t code,
				  mach_msg_type_number_t code_count,
				  int *flavor,
				  thread_state_t old_state,
				  mach_msg_type_number_t *icnt,
				  thread_state_t new_state,
				  mach_msg_type_number_t *ocnt);
extern kern_return_t osfmach3_trap_unwind(mach_port_t trap_port,
					  exception_type_t exc_type,
					  exception_data_t code,
					  mach_msg_type_number_t code_count,
					  int *flavor,
					  thread_state_t old_state,
					  mach_msg_type_number_t icnt,
					  thread_state_t new_state,
					  mach_msg_type_number_t *ocnt);
extern void osfmach3_notify_register(struct task_struct *tsk);
extern void osfmach3_notify_deregister(struct task_struct *tsk);

extern void osfmach3_update_vm_info(void);
extern void osfmach3_update_load_info(void);
extern void osfmach3_update_cpu_load_info(void);
extern void osfmach3_get_time(struct timeval *xtimep);
extern void osfmach3_set_time(struct timeval *xtimep);

#endif	/* _OSFMACH3_SCHED_H */
