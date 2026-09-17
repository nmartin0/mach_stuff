/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	thread.h,v $
 * Revision 2.20  93/11/17  17:31:15  dbg
 * 	Added thread_deallocate_nowait for use by AST routines.
 * 	[93/07/21            dbg]
 * 
 * 	Remove ith_lock in favor of thread_lock.
 * 	[93/07/12            dbg]
 * 
 * 	Break up thread lock, to simplify interactions between thread
 * 	lock and processor set lock and to reduce the amount of code
 * 	that runs with interrupts blocked.  There are now three locks:
 * 	. thread_ref_lock	locks the reference count
 * 	. thread_sched_lock	locks fields involved with scheduling
 * 				state machine
 * 	. thread_lock		locks everything else.
 * 	[93/05/26            dbg]
 * 
 * 	Scheduling policy fields are now added at end of thread
 * 	structure by policy configuration.
 * 	[93/05/10            dbg]
 * 
 * 	Changed policy to policy_index.
 * 	[93/05/05            dbg]
 * 
 * 	Removed thread->depress_timer.  It and thread->timer were never
 * 	used simultaneously.  Added sched_policy.
 * 	[93/04/08            dbg]
 * 
 * 	Renamed ipc/ipc_kmsg_queue.h to ipc/ipc_types.h.
 * 	[93/03/29            dbg]
 * 
 * 	Remove include of kern/sched.h.  Add fields for real-time
 * 	threads.
 * 	[93/01/28            dbg]
 * 
 * Revision 2.19  93/08/10  15:12:01  mrt
 * 	Conditionalized atm hooks.
 * 	[93/07/30            cmaeda]
 * 	Included hook for network interface.
 * 	[93/06/09  15:45:03  jcb]
 * 
 * Revision 2.18  93/01/27  09:33:10  danner
 * 	Break latent include circularity by introduction of ipc_kmsg_queue.h. 
 * 
 * Revision 2.17  93/01/24  13:20:32  danner
 * 	Added sample_control to indicate that this thread is to
 * 	have its pc sampled periodically.
 * 	[93/01/12            rvb]
 * 
 * Revision 2.16  93/01/21  14:04:00  danner
 * 	Typo correction.
 * 
 * Revision 2.15  93/01/19  09:01:07  danner
 * 	Correct arguments to natural_t.
 * 	[93/01/19            danner]
 * 
 * Revision 2.14  93/01/14  17:37:12  danner
 * 	Moved actual declaration of struct thread and thread_t to
 * 	kern/kern_types.h, to permit mutually recursive structure
 * 	definitions.  Added ANSI function prototypes.
 * 	[92/12/28            dbg]
 * 	Swapped some fields for better alignment.
 * 	[92/12/01            af]
 * 
 * Revision 2.13  91/08/28  11:14:53  jsb
 * 	Added ith_seqno.
 * 	[91/08/10            rpd]
 * 
 * Revision 2.12  91/07/31  17:49:44  dbg
 * 	Consolidated interruptible, swap_state, halted into state field.
 * 	Revised state machine.
 * 	[91/07/30  17:06:37  dbg]
 * 
 * Revision 2.11  91/05/18  14:34:28  rpd
 * 	Added depress_timer.
 * 	[91/03/31            rpd]
 * 
 * 	Replaced swap_privilege with stack_privilege.
 * 	[91/03/30            rpd]
 * 
 * Revision 2.10  91/05/14  16:48:57  mrt
 * 	Correcting copyright
 * 
 * Revision 2.9  91/03/16  14:52:58  rpd
 * 	Removed ith_saved.
 * 	[91/02/16            rpd]
 * 	Added save-state fields for page faults.
 * 	[91/02/05            rpd]
 * 	Added NCPUS to active_threads declaration.
 * 	Added active_stacks.
 * 	[91/01/28            rpd]
 * 	Added swap_privilege.
 * 	[91/01/18            rpd]
 * 
 * Revision 2.8  91/02/05  17:30:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:19:58  mrt]
 * 
 * Revision 2.7  91/01/08  15:18:11  rpd
 * 	Added saved-state fields for exceptions.
 * 	[90/12/23            rpd]
 * 	Added swap_func.
 * 	[90/11/20            rpd]
 * 
 * Revision 2.6  90/08/27  22:04:23  dbg
 * 	Remove import of thread_modes.h (unneeded).
 * 	[90/07/17            dbg]
 * 
 * Revision 2.5  90/08/07  17:59:15  rpd
 * 	Put last_processor field under NCPUS > 1.
 * 	Removed tmp_address and tmp_object fields.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.4  90/06/02  14:57:07  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:25:34  rpd]
 * 
 * Revision 2.3  90/02/22  20:04:18  dbg
 * 	Add per-thread global VM variables (tmp_address, tmp_object).
 * 		[89/04/29	mwyoung]
 * 
 * Revision 2.2  89/09/08  11:26:59  dbg
 * 	Added simple_rpc_kmsg [rfr].  Set its size to size of
 * 	small Kmsg.  Moved all IPC data structures to end of
 * 	thread structure.
 * 	[89/08/16            dbg]
 * 
 * Revision 2.6  88/10/11  10:26:00  rpd
 * 	Added ipc_data to the thread structure.
 * 	[88/10/10  08:00:16  rpd]
 * 	
 * Revision 2.5  88/08/24  02:47:53  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:24:57  mwyoung]
 * 
 *  1-Sep-88  David Black (dlb) at Carnegie-Mellon University
 *	Change all usage and delta fields to unsigned to prevent
 *	negative priorities.
 *
 * 11-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Split exit_code field into ast and halted fields.
 *	Changed thread_should_halt() macro.
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Replaced preempt_pri field with first_quantum.
 *
 * 19-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Changed 'struct pcb *' to 'pcb_t' to pry this file loose from
 *	old data structures.  Removed include of 'machine/pcb.h' - the
 *	structure definition should be moved to 'machine/thread.h'.
 *
 * Revision 2.4  88/08/06  19:22:05  rpd
 * Declare external variables as "extern".
 * Added macros ipc_thread_lock(), ipc_thread_unlock().
 * 
 * Revision 2.3  88/07/17  18:55:10  mwyoung
 * .
 * 
 * Revision 2.2.1.1  88/06/28  20:53:40  mwyoung
 * Reorganized.  Added thread_t->vm_privilege.
 * 
 * Added current_task() declaration.
 * 
 *
 *  6-Jul-88  David Golub (dbg) at Carnegie-Mellon University
 *	Remove compatibility data structures.  Add per-thread timeout
 *	element.
 *
 * 21-Jun-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Reorganized.  Added thread_t->vm_privilege.
 *
 *  4-May-88  David Golub (dbg) at Carnegie-Mellon University
 *	Remove vax-specific field (pcb physical address).
 *
 * 19-Apr-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added current_task() declaration.
 *
 *  7-Apr-88  David Black (dlb) at Carnegie-Mellon University
 *	MACH_TIME_NEW is now standard.
 *
 *  4-Mar-88  David Black (dlb) at Carnegie-Mellon University
 *	Added usage_save and preempt_pri fields.
 *
 * 19-Feb-88  David Black (dlb) at Carnegie-Mellon University
 *	Deleted unused ticks field.  Rearranged and added MACH_TIME_NEW
 *	fields for scheduler interface.  user_ticks and system_ticks are
 *	not needed under MACH_TIME_NEW.  Change wait_time to sched_stamp.
 *
 * 21-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Replaced swappable boolean with swap_state field.  Swap states
 *	are defined in sys/thread_swap.h.
 *
 * 17-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Added declarations of new routines.
 *
 * 29-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Added declarations of thread_halt() and thread_halt_self().
 *
 * 21-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Added thread_should_halt macro.
 *
 *  9-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Added exit_code for thread termination and interrupt.
 *	Removed ipc_message_waiting and ipc_timer_set.
 *
 *  3-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Added ipc_kernel field to indicate when message buffer is in
 *	kernel address space.  Added exception_clear_port to cache
 *	reply port for reuse in exc rpc.
 *
 *  2-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Removed conditionals, purged history.
 */
/*
 *	File:	thread.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	This file contains the structure definitions for threads.
 *
 */

#ifndef	_KERN_THREAD_H_
#define _KERN_THREAD_H_

#include <hw_footprint.h>
#include <mach_host.h>
#include <mach_ipc_compat.h>
#include <mach_pcsample.h>
#include <mach_rt.h>
#include <net_atm.h>

#include <mach/boolean.h>
#include <mach/thread_info.h>
#include <mach/thread_status.h>
#include <mach/machine/vm_types.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/vm_prot.h>

#include <kern/kern_types.h>

#include <kern/ast.h>
#include <kern/cpu_number.h>
#include <kern/lock.h>
#include <kern/mach_timer.h>
#include <kern/queue.h>
#include <kern/sched_policy.h>
#include <kern/sched_prim.h>	/* event_t, continuation_t */
#include <kern/task.h>		/* for current_space(), current_map() */
#include <kern/timer.h>
#include <machine/thread.h>
#include <ipc/ipc_types.h>

#if	MACH_PCSAMPLE
#include <kern/pc_sample.h>
#endif

/*
 *	Thread structure.
 *
 *	Fields prefixed with S are locked by thread_sched_lock.
 *	Other fields are locked by thread_lock.
 */

struct thread {
	/* Run queues */
	queue_chain_t	links;		/* S current run queue links */
	struct run_queue_head *runq;	/* S run queue p is on SEE BELOW */

/*
 *	NOTE:	The runq field in the thread structure has an unusual
 *	locking protocol.  If its value is RUN_QUEUE_HEAD_NULL, then
 *	it is locked by thread_sched_lock, but if its value is something
 *	else (i.e. a run_queue) then it is locked by that run_queue`s
 *	lock.
 */

	/* Task information */
	task_t		task;		/*   Task to which I belong */
	queue_chain_t	thread_list;	/*   list of threads in task */

	/* Thread bookkeeping */
	queue_chain_t	pset_threads;	/*   list of all threads in proc set*/

	/* Self-preservation */
	int		ref_count;	/*   number of references to me */
	decl_simple_lock_data(,ref_lock)
					/*   lock for ref_count */
	decl_simple_lock_data(,lock)	/*   lock for everything else */

	/* Hardware state */
	pcb_t		pcb;		/*   hardware pcb & machine state */
	vm_offset_t	kernel_stack;	/* S kernel stack:  accurate only
					     if the thread is not swapped
					     and not executing */
	vm_offset_t	stack_privilege;/*   reserved kernel stack */

	/* Swapping information */
	continuation_t	swap_func;	/*   start here after swapin */

	/* Blocking information */
	decl_simple_lock_data(,sched_lock)
					/*   lock for scheduling fields */
	event_t		wait_event;	/* S event we are waiting on */
	int		suspend_count;	/* S internal hold (suspend) count */
	kern_return_t	wait_result;	/* S outcome of wait -
					     may be examined by this thread
					     WITHOUT locking */
	boolean_t	suspend_wait;	/* S someone is waiting for this
					     thread to become suspended */
	int		state;		/* S Thread state: */
/*
 *	Thread states [bits or'ed]
 */
#define TH_WAIT			0x01	/*   thread is queued for waiting */
#define TH_SUSP			0x02	/*   thread has been asked to stop */
#define TH_RUN			0x04	/*   thread is running or on runq */
#define TH_UNINT		0x08	/*   thread is waiting uninteruptibly */
#define	TH_HALTED		0x10	/*   thread is halted at a clean
					     point */
#define TH_IDLE			0x80	/*   thread is an idle thread */

#define	TH_SCHED_STATE	(TH_WAIT|TH_SUSP|TH_RUN|TH_UNINT)

#define	TH_SWAPPED		0x0100	/*   thread has no kernel stack */
#define	TH_SW_COMING_IN		0x0200	/*   thread is waiting for kernel
					     stack */

#define	TH_SWAP_STATE	(TH_SWAPPED | TH_SW_COMING_IN)

	/* Ast/Halt data structures */
	int		ast;    	/* S ast's needed.  See ast.h */
	boolean_t	active;		/*   how alive is the thread */

	/* User-visible scheduling state */
	int		user_stop_count;
					/* S outstanding stops */

	/* Timing data structures */
	timer_data_t	user_timer;	/* S user mode timer */
	timer_data_t	system_timer;	/* S system mode timer */
	timer_save_data_t user_timer_save;
					/* S saved user timer value */
	timer_save_data_t system_timer_save;
					/* S saved sys timer value */
	unsigned int	cpu_delta;	/* S cpu usage since last update */
	unsigned int	sched_delta;	/* S weighted cpu usage since update */
	unsigned int	cpu_usage;	/* S exp. decaying cpu usage [%cpu] */
	unsigned int	sched_usage;	/* S load-weighted cpu usage [sched] */
	unsigned int	sched_stamp;	/* S last time priority was updated */

	/* VM global variables */
	vm_offset_t	recover;	/*   page fault recovery (copyin/out) */
	boolean_t	vm_privilege;	/*   Can use reserved memory? */

	/* IPC data structures */
	thread_t	ith_next;
	thread_t	ith_prev;	/*   IPC thread queue */
	mach_msg_return_t	ith_state;
					/*   send/receive status */
	union ipc_kmsg_return	data;	/*   received message, or max size */
	mach_port_seqno_t	ith_seqno;
					/*   seqno of recvd message */
	struct ipc_kmsg_queue	ith_messages;
					/*   queue of blocked messages */
	ipc_port_t	ith_self;	/*   not a right, doesn't hold ref */
	ipc_port_t	ith_sself;	/*   a send right */
	ipc_port_t	ith_exception;	/*   a send right */
#if	MACH_IPC_COMPAT
	ipc_port_t	ith_reply;	/*   a send right */
#endif	/* MACH_IPC_COMPAT */

	mach_port_t	ith_mig_reply;	/*   reply port for mig */
	ipc_port_t	ith_rpc_reply;	/*   reply port for kernel RPCs */

	/* State saved when thread's stack is discarded */
	union {
		struct {
			mach_msg_header_t *msg;
			mach_msg_option_t option;
			mach_msg_size_t rcv_size;
			mach_msg_timeout_t timeout;
			mach_port_t notify;
			struct ipc_object *object;
			struct ipc_mqueue *mqueue;
		} receive;
		struct {
			ipc_port_t port;
			int exc;
			int code;
			int subcode;
		} exception;
		void *other;		/*   catch-all for other state */
	} saved;

	/* Time-outs */
	timer_elt_data_t timer;		/* S timer for thread */

	/* Processor data structures */
	processor_set_t	processor_set;	/*   assigned processor set */
#if	NCPUS > 1
	processor_t	bound_processor; /*  bound to processor ? */
	processor_t	last_processor; /* S processor this last ran on */
#endif	/* NCPUS > 1 */

#if	MACH_PCSAMPLE
	/* PC sampling */
	sample_control_t pc_sample;	/* S PC sample buffer control */
#endif

#if	MACH_RT
	mach_timer_t	rt_wakeup_timer;
					/*   periodic wakeup timer */
	mach_timer_t	rt_deadline_timer;
					/*   deadline expiration timer */
	ipc_port_t	rt_deadline_port;
					/*   deadline notification port */
#endif

	/* Scheduling information */
	int		policy_index;	/* S current scheduling policy index */
	sched_policy_t	cur_policy;	/* S current scheduling policy vector */
	sched_policy_t	sched_policy;	/* S scheduling policy in effect,
					     when priority not depressed */
	sched_param_data_t
			sched_data; 	/* S per-policy scheduling parameters */
#if	NET_ATM
	nw_ep_owned_t   nw_ep_waited;
#endif	/* NET_ATM */
};

#define	ith_msize	data.msize
#define	ith_kmsg	data.kmsg
#define	ith_wait_result	wait_result

#define	ith_msg		saved.receive.msg
#define	ith_option	saved.receive.option
#define ith_rcv_size	saved.receive.rcv_size
#define ith_timeout	saved.receive.timeout
#define ith_notify	saved.receive.notify
#define ith_object	saved.receive.object
#define ith_mqueue	saved.receive.mqueue

#define	ith_port	saved.exception.port
#define ith_exc		saved.exception.exc
#define ith_exc_code	saved.exception.code
#define ith_exc_subcode	saved.exception.subcode

#define ith_other	saved.other

#ifndef	_KERN_KERN_TYPES_H_
typedef struct thread *thread_t;

#define THREAD_NULL	((thread_t) 0)

typedef	mach_port_t *thread_array_t;
#endif	/* _KERN_KERN_TYPES_H_ */


extern thread_t		active_threads[NCPUS];	/* active threads */
extern vm_offset_t	active_stacks[NCPUS];	/* active kernel stacks */

/*
 *	User routines
 */

extern kern_return_t thread_create(
		task_t		parent_task,
		thread_t	*child_thread);

extern kern_return_t thread_terminate(
		thread_t	thread);

extern kern_return_t thread_suspend(
		thread_t	thread);

extern kern_return_t thread_resume(
		thread_t	thread);

extern kern_return_t thread_abort(
		thread_t	thread);

extern kern_return_t thread_get_state(
		thread_t	thread,
		int		flavor,
		thread_state_t	old_state,
		natural_t	*old_state_count);

extern kern_return_t thread_set_state(
		thread_t	thread,
		int		flavor,
		thread_state_t	new_state,
		natural_t	new_state_count);

extern kern_return_t thread_get_special_port(
		thread_t	thread,
		int		which,
		ipc_port_t	*portp);

extern kern_return_t thread_set_special_port(
		thread_t	thread,
		int		which,
		ipc_port_t	port);

extern kern_return_t thread_info(
		thread_t	thread,
		int		flavor,
		thread_info_t	thread_info_out,
		natural_t	*thread_info_count);

extern kern_return_t thread_assign(
		thread_t	thread,
		processor_set_t	new_pset);

extern kern_return_t thread_assign_default(
		thread_t	thread);

extern kern_return_t	thread_set_policy(
		thread_t	thread,
		processor_set_t	pset,
		int		policy,
		policy_param_t	param,
		natural_t	count);

extern kern_return_t	thread_set_policy_param(
		thread_t	thread,
		boolean_t	set_limit,
		policy_param_t	param,
		natural_t	count);

extern kern_return_t	thread_set_policy_limit(
		thread_t	thread,
		processor_set_t	pset,
		policy_param_t	param,
		natural_t	count);

/*
 *	Kernel-only routines
 */

extern void		thread_init(void);
extern void		thread_reference(thread_t);
extern void		thread_deallocate(thread_t);
extern void		thread_hold(thread_t);
extern kern_return_t	thread_dowait(
		thread_t	thread,
		boolean_t	must_halt);
extern void		thread_release(thread_t);
extern kern_return_t	thread_halt(
		thread_t	thread,
		boolean_t	must_halt);
extern void		thread_halt_self(continuation_t);
extern no_return	thread_terminate_self(void);
extern void		thread_force_terminate(thread_t);
extern void		thread_set_own_priority(
		int		priority);
extern thread_t		kernel_thread(
		task_t		task,
		continuation_t	start,
		void *		arg);
extern void		thread_start(
		thread_t	thread,
		continuation_t	start);

extern void		thread_suspend_nowait(thread_t);
extern void		thread_deallocate_nowait(thread_t);

extern no_return	reaper_thread(void);

#if	MACH_HOST
extern processor_set_t	thread_assign_if_empty(
		thread_t	thread);
#endif	/* MACH_HOST */

extern void		consider_thread_collect(void);

/*
 *	Macro-defined routines
 */

#define thread_pcb(th)		((th)->pcb)

#define thread_lock(th)		simple_lock(&(th)->lock)
#define thread_unlock(th)	simple_unlock(&(th)->lock)
#define	thread_ref_lock(th)	simple_lock(&(th)->ref_lock)
#define	thread_ref_unlock(th)	simple_unlock(&(th)->ref_lock)
#define	thread_sched_lock(th)	simple_lock(&(th)->sched_lock)
#define	thread_sched_unlock(th)	simple_unlock(&(th)->sched_lock)

/*
 *	Machine specific implementations of the current thread macro
 *	designate this by defining CURRENT_THREAD.
 */
#ifndef	CURRENT_THREAD
#define current_thread()	(active_threads[cpu_number()])
#endif	/* CURRENT_THREAD */

#define	current_stack()		(active_stacks[cpu_number()])

#define	current_task()		(current_thread()->task)
#define	current_space()		(current_task()->itk_space)
#define	current_map()		(current_task()->map)

/*
 *	Routines required from the machine-dependent
 *	thread module (usually called machine/pcb.c).
 */
extern void			pcb_module_init(void);

extern void			pcb_init(
			thread_t	thread);
extern void			pcb_terminate(
			thread_t	thread);
extern void			pcb_collect(
			thread_t	thread);
extern kern_return_t		thread_setstatus(
			thread_t	thread,
			int		flavor,
			thread_state_t	tstate,
			natural_t	count);
extern kern_return_t		thread_getstatus(
			thread_t	thread,
			int		flavor,
			thread_state_t	tstate,		/* in/out */
			natural_t	*count);	/* in/out */
extern void			thread_set_syscall_return(
			thread_t	thread,
			kern_return_t	retval);

extern vm_offset_t		user_stack_low(
			vm_size_t	stack_size);

extern vm_offset_t		set_user_regs(
			vm_offset_t	stack_base,
			vm_size_t	stack_size,
			natural_t	*entry,
			vm_size_t	arg_len);

#endif	/* _KERN_THREAD_H_ */
