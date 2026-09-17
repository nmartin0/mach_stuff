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
 * $Log:	thread.c,v $
 * Revision 2.34  93/11/17  17:30:37  dbg
 * 	If current thread is being terminated and is already inactive,
 * 	hold it.  The thread terminating it may not yet have called
 * 	thread_halt.
 * 	[93/08/26            dbg]
 * 
 * 	Made obsolete priority routines call new routines directly.
 * 	They will work only if the thread is timesharing, or has a
 * 	policy parameter that consists of one integer... but by the time
 * 	there are any more, the obsolete calls should be gone!
 * 	[93/08/18            dbg]
 * 
 * 	Added thread_dealloate_nowait for use by AST routines.
 * 	[93/07/21            dbg]
 * 
 * 	Changed termination protocol to simultaneously clear active and
 * 	remove thread_port association.
 * 	[93/06/29            dbg]
 * 
 * 	Break up thread lock, to simplify interactions between thread
 * 	lock and processor set lock and to reduce the amount of code
 * 	that runs with interrupts blocked.  There are now three locks:
 * 	. thread_ref_lock	locks the reference count
 * 	. thread_sched_lock	locks fields involved with scheduling
 * 				state machine
 * 	. thread_lock		locks everything else.
 * 
 * 	Revise processor set locking.  Order is now:
 * 	thread_lock -> pset_lock -> thread_ref_lock
 * 
 * 	Move calls to evc_notify_abort into thread_halt.
 * 	[93/05/26            dbg]
 * 
 * 	Moved scheduling policy fields to end of thread data
 * 	structure.  The scheduling information is policy-specific.
 * 	[93/05/10            dbg]
 * 
 * 	Declare continuations as not returning.
 * 	[93/05/04            dbg]
 * 
 * 	Initialize rt_period and rt_deadline to largest possible
 * 	time_spec_t.
 * 	[93/04/15            dbg]
 * 
 * 	Removed thread->depress_timer.
 * 	[93/04/08            dbg]
 * 
 * 	Changed global scheduling policy to policy within scheduling
 * 	domain.
 * 	[93/03/31            dbg]
 * 
 * 	Always enable fixed-priority threads.
 * 	[93/03/27            dbg]
 * 
 * 	Thread->sched_data (quantum for fixed-priority threads) is now
 * 	kept in microseconds internally.
 * 
 * 	TIMER_RATE is now USAGE_RATE.
 * 
 * 	Removed include of kern/sched.h.  Moved routines that manipulate
 * 	thread state to kern/sched_prim.c: thread_halt,
 * 	thread_halt_self, thread_hold, thread_dowait, thread_release,
 * 	thread_suspend, thread_resume.
 * 
 * 	Added AST_KERNEL_CHECK to reaper thread.
 * 	[93/01/28            dbg]
 * 
 * Revision 2.33  93/08/10  15:11:43  mrt
 * 	Conditionalized atm hooks.
 * 	[93/07/30            cmaeda]
 * 	Included hooks for network interface.
 * 	[93/06/09  15:44:22  jcb]
 * 
 * Revision 2.32  93/05/15  18:55:33  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.31  93/01/24  13:20:13  danner
 * 	Add call to evc_notify_abort to thread_abort; correct call in
 * 	 thread_terminate. 
 * 	[93/01/22            danner]
 * 
 * 	We must explicitly set "new_thread->pc_sample.buffer = 0;" so
 * 	that we don't think we have a sampling buffer.
 * 	[93/01/13            rvb]
 * 
 * Revision 2.30  93/01/21  12:22:49  danner
 * 	Add call in thread_deallocate to evc_notify_thread_destroy to
 * 	deal with threads terminating while in evc_wait.
 * 	[93/01/20            bershad]
 * 
 * Revision 2.29  93/01/14  17:36:55  danner
 * 	Added ANSI function prototypes.
 * 	[92/12/29            dbg]
 * 
 * 	64bit cleanup. Proper spl typing.
 * 	[92/12/01            af]
 * 
 * 	Changed thread_create to hold both the pset and task locks while
 * 	inserting the new thread in the lists.
 * 	[92/11/20            dbg]
 * 
 * 	Fixed pset lock ordering.  Pset lock must be taken before task
 * 	and thread locks.
 * 	[92/10/28            dbg]
 * 
 * Revision 2.28  92/08/03  17:39:56  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.27  92/05/21  17:16:33  jfriedl
 * 	Appended 'U' to constants that would otherwise be signed.
 * 	Changed name of one of the two local 'thread' variables in
 * 	processor_set_stack_usage() to 'tmp_thread' for cleanness.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.26  92/04/01  19:33:34  rpd
 * 	Restored continuation in AST_TERMINATE case of thread_halt_self.
 * 	[92/03/22            rpd]
 * 
 * Revision 2.25  92/03/10  16:24:22  jsb
 * 	Remove continuation from AST_TERMINATE case of thread_halt_self 
 * 	so that "zombie walks" is reachable.
 * 	[92/02/25            dlb]
 * 
 * Revision 2.24  92/02/19  16:07:03  elf
 * 	Change calls to compute_priority.
 * 	[92/01/19            rwd]
 * 
 * Revision 2.23  92/01/03  20:18:56  dbg
 * 	Make thread_wire really reserve a kernel stack.
 * 	[91/12/18            dbg]
 * 
 * Revision 2.22  91/12/13  14:54:53  jsb
 * 	Removed thread_resume_from_kernel.
 * 	[91/12/12  17:40:12  af]
 * 
 * Revision 2.21  91/08/28  11:14:43  jsb
 * 	Fixed thread_halt, mach_msg_interrupt interaction.
 * 	Added checks for thread_exception_return, thread_bootstrap_return.
 * 	[91/08/03            rpd]
 * 
 * Revision 2.20  91/07/31  17:49:10  dbg
 * 	Call pcb_module_init from thread_init.
 * 	[91/07/26            dbg]
 * 
 * 	When halting a thread: if it is waiting at a continuation with a
 * 	known cleanup routine, call the cleanup routine instead of
 * 	resuming the thread.
 * 	[91/06/21            dbg]
 * 
 * 	Revise scheduling state machine.
 * 	[91/05/22            dbg]
 * 
 * 	Add thread_wire.
 * 	[91/05/14            dbg]
 * 
 * Revision 2.19  91/06/25  10:29:48  rpd
 * 	Picked up dlb's thread_doassign no-op fix.
 * 	[91/06/23            rpd]
 * 
 * Revision 2.18  91/05/18  14:34:09  rpd
 * 	Fixed stack_alloc to use kmem_alloc_aligned.
 * 	[91/05/14            rpd]
 * 	Added argument to kernel_thread.
 * 	[91/04/03            rpd]
 * 
 * 	Changed thread_deallocate to reset timer and depress_timer.
 * 	[91/03/31            rpd]
 * 
 * 	Replaced stack_free_reserved and swap_privilege with stack_privilege.
 * 	[91/03/30            rpd]
 * 
 * Revision 2.17  91/05/14  16:48:35  mrt
 * 	Correcting copyright
 * 
 * Revision 2.16  91/05/08  12:49:14  dbg
 * 	Add volatile declarations.
 * 	[91/04/26  14:44:13  dbg]
 * 
 * Revision 2.15  91/03/16  14:52:40  rpd
 * 	Fixed the initialization of stack_lock_data.
 * 	[91/03/11            rpd]
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 	Removed ith_saved.
 * 	[91/02/16            rpd]
 * 
 * 	Added stack_alloc_max.
 * 	[91/02/10            rpd]
 * 	Added active_stacks.
 * 	[91/01/28            rpd]
 * 	Can't use thread_dowait on the current thread now.
 * 	Added reaper_thread_continue.
 * 	Added stack_free_reserved.
 * 	[91/01/20            rpd]
 * 
 * 	Removed thread_swappable.
 * 	Allow swapped threads on the run queues.
 * 	Changed the AST interface.
 * 	[91/01/17            rpd]
 * 
 * Revision 2.14  91/02/05  17:30:14  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:19:30  mrt]
 * 
 * Revision 2.13  91/01/08  15:17:57  rpd
 * 	Added KEEP_STACKS support.
 * 	[91/01/06            rpd]
 * 	Added consider_thread_collect, thread_collect_scan.
 * 	[91/01/03            rpd]
 * 
 * 	Added locking to the stack package.  Added stack_collect.
 * 	[90/12/31            rpd]
 * 	Changed thread_dowait to discard the stacks of suspended threads.
 * 	[90/12/22            rpd]
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * 	Changed thread_create to make new threads be swapped.
 * 	Changed kernel_thread to swapin the threads.
 * 	[90/11/20            rpd]
 * 
 * 	Removed stack_free/stack_alloc/etc.
 * 	[90/11/12            rpd]
 * 
 * 	Changed thread_create to let pcb_init handle all pcb initialization.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.12  90/12/05  23:29:09  af
 * 
 * 
 * Revision 2.11  90/12/05  20:42:19  af
 * 	Added (temporarily, until we define the right new primitive with
 * 	the RTMach guys) thread_resume_from_kernel() for internal kernel
 * 	use only.
 * 
 * 	Revision 2.7.1.1  90/09/25  13:06:04  dlb
 * 	Inline thread_hold in thread_suspend and thread_release in
 * 	thread_resume (while still holding the thread lock in both).
 * 	This eliminates a long-standing MP bug.
 * 	[90/09/20            dlb]
 * 
 * Revision 2.10  90/11/05  14:31:46  rpd
 * 	Unified untimeout and untimeout_try.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.9  90/10/25  14:45:37  rwd
 * 	Added host_stack_usage and processor_set_stack_usage.
 * 	[90/10/22            rpd]
 * 
 * 	Removed pausing code in stack_alloc/stack_free.
 * 	It was broken and it isn't needed.
 * 	Added code to check how much stack is actually used.
 * 	[90/10/21            rpd]
 * 
 * Revision 2.8  90/10/12  12:34:37  rpd
 * 	Fixed HW_FOOTPRINT code in thread_create.
 * 	Fixed a couple bugs in thread_policy.
 * 	[90/10/09            rpd]
 * 
 * Revision 2.7  90/08/27  22:04:03  dbg
 * 	Fixed thread_info to return the correct count.
 * 	[90/08/23            rpd]
 * 
 * Revision 2.6  90/08/27  11:52:25  dbg
 * 	Remove unneeded mode parameter to thread_start.
 * 	Remove u_zone, thread_deallocate_interrupt.
 * 	[90/07/17            dbg]
 * 
 * Revision 2.5  90/08/07  17:59:04  rpd
 * 	Removed tmp_address, tmp_object fields.
 * 	Picked up depression abort functionality in thread_abort.
 * 	Picked up initialization of max_priority in kernel_thread.
 * 	Picked up revised priority computation in thread_create.
 * 	Removed in-transit check from thread_max_priority.
 * 	Picked up interprocessor-interrupt optimization in thread_dowait.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.4  90/06/02  14:56:54  rpd
 * 	Converted to new IPC and scheduling technology.
 * 	[90/03/26  22:24:06  rpd]
 * 
 * Revision 2.3  90/02/22  20:04:12  dbg
 * 	Initialize per-thread global VM variables.
 * 	Clean up global variables in thread_deallocate().
 * 		[89/04/29	mwyoung]
 * 
 * Revision 2.2  89/09/08  11:26:53  dbg
 * 	Allocate thread kernel stack with kmem_alloc_wired to get
 * 	separate vm_object for it.
 * 	[89/08/18            dbg]
 * 
 * 19-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Removed all non-MACH code.
 *
 * 11-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Rewrote exit logic to use new ast mechanism.  Includes locking
 *	bug fix to thread_terminate().
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	first_quantum replaces preempt_pri.
 *
 * Revision 2.3  88/08/06  18:26:41  rpd
 * Changed to use ipc_thread_lock/ipc_thread_unlock macros.
 * Eliminated use of kern/mach_ipc_defs.h.
 * Added definitions of all_threads, all_threads_lock.
 * 
 *  4-May-88  David Golub (dbg) and David Black (dlb) at CMU
 *	Remove vax-specific code.  Add register declarations.
 *	MACH_TIME_NEW now standard.  Moved thread_read_times to timer.c.
 *	SIMPLE_CLOCK: clock drift compensation in cpu_usage calculation.
 *	Initialize new fields in thread_create().  Implemented cpu usage
 *	calculation in thread_info().  Added argument to thread_setrun.
 *	Initialization changes for MACH_TIME_NEW.
 *
 * 13-Apr-88  David Black (dlb) at Carnegie-Mellon University
 *	Rewrite kernel stack retry code to eliminate races and handle
 *	termination correctly.
 *
 * 19-Feb-88  David Kirschen (kirschen) at Encore Computer Corporation
 *      Retry if kernel stacks exhausted on thread_create
 *
 * 12-Feb-88  David Black (dlb) at Carnegie-Mellon University
 *	Fix MACH_TIME_NEW code.
 *
 *  1-Feb-88  David Golub (dbg) at Carnegie-Mellon University
 *	In thread_halt: mark the victim thread suspended/runnable so that it
 *	will notify the caller when it hits the next interruptible wait.
 *	The victim may not immediately proceed to a clean point once it
 *	is awakened.
 *
 * 21-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Use new swapping state machine.  Moved thread_swappable to
 *	thread_swap.c.
 *
 * 17-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Added new thread interfaces: thread_suspend, thread_resume,
 *	thread_get_state, thread_set_state, thread_abort,
 *	thread_info.  Old interfaces remain (temporarily) for binary
 *	compatibility, prefixed with 'xxx_'.
 *
 * 29-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Delinted.
 *
 * 15-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Made thread_reference and thread_deallocate check for null
 *	threads.  Call pcb_terminate when a thread is deallocated.
 *	Call pcb_init with thread pointer instead of pcb pointer.
 *	Add missing case to thread_dowait.
 *
 *  9-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Rewrote thread termination to have a terminating thread clean up
 *	after itself.
 *
 *  9-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Moved reaper invocation to thread_terminate from
 *	thread_deallocate.  [XXX temporary pending rewrite.]
 *
 *  8-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Added call to ipc_thread_disable.
 *
 *  4-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Set ipc_kernel in kernel_thread().
 *
 *  3-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Rewrote thread_create().  thread_terminate() must throw away
 *	an extra reference if called on the current thread [ref is
 *	held by caller who will not be returned to.]  Locking bug fix
 *	to thread_status.
 *
 * 19-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Eliminated TT conditionals.
 *
 * 30-Oct-87  David Golub (dbg) at Carnegie-Mellon University
 *	Fix race condition in thread_deallocate for thread terminating
 *	itself.
 *
 * 23-Oct-87  David Golub (dbg) at Carnegie-Mellon University
 *	Correctly set thread_statistics fields.
 *
 * 13-Oct-87  David Black (dlb) at Carnegie-Mellon University
 *	Use counts for suspend and resume primitives.
 *
 *  5-Oct-87  David Golub (dbg) at Carnegie-Mellon University
 *	MACH_TT: Completely replaced scheduling state machine.
 *
 * 30-Sep-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added initialization of thread->flags in thread_create().
 *	Added thread_swappable().
 *	De-linted.
 *
 * 30-Sep-87  David Black (dlb) at Carnegie-Mellon University
 *	Rewrote thread_dowait to more effectively stop threads.
 *
 * 11-Sep-87  Robert Baron (rvb) at Carnegie-Mellon University
 *	Initialize thread fields and unix_lock.
 *
 *  9-Sep-87  David Black (dlb) at Carnegie-Mellon University
 *	Changed thread_dowait to count a thread as stopped if it is
 *	sleeping and will stop immediately when woken up.  [i.e. is
 *	sleeping interruptibly].  Corresponding change to
 *	thread_terminate().
 *
 *  4-Aug-87  David Golub (dbg) at Carnegie-Mellon University
 *	Moved ipc_thread_terminate to thread_terminate (from
 *	thread_deallocate), to shut out other threads that are
 *	manipulating the thread via its thread_port.
 *
 * 29-Jul-87  David Golub (dbg) at Carnegie-Mellon University
 *	Make sure all deallocation calls are outside of locks - they may
 *	block.  Moved task_deallocate from thread_deallocate to
 *	thread_destroy, since thread may blow up if task disappears while
 *	thread is running.
 *
 * 26-Jun-87  David Black (dlb) at Carnegie-Mellon University
 *	Added update_priority() call to thread_release() for any thread
 *	actually released.
 *
 * 23-Jun-87  David Black (dlb) at Carnegie-Mellon University
 *	Initialize thread priorities in thread_create() and kernel_thread().
 *
 * 10-Jun-87  Karl Hauth (hauth) at Carnegie-Mellon University
 *	Added code to fill in the thread_statistics structure.
 *
 *  1-Jun-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Added thread_statistics stub.
 *
 * 21-May-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Clear the thread u-area upon creation of a thread to keep
 *	consistent.
 *
 *  4-May-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Call uarea_init to initialize u-area stuff.
 *
 * 29-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Moved call to ipc_thread_terminate into the MACH_TT only branch
 *	to prevent problems with non-TT systems.
 *
 * 28-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Support the thread status information as a MiG refarray.
 *	[NOTE: turned off since MiG is still too braindamaged.]
 *
 * 23-Apr-87  Rick Rashid (rfr) at Carnegie-Mellon University
 *	Moved ipc_thread_terminate to thread_deallocate from
 *	thread_destroy to eliminate having the reaper call it after
 *	the task has been freed.
 *
 * 18-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Added reaper thread for deallocating threads that cannot
 *	deallocate themselves (some time ago).
 *
 * 17-Mar-87  David Golub (dbg) at Carnegie-Mellon University
 *	De-linted.
 *
 * 14-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Panic if no space left in the kernel map for stacks.
 *
 *  6-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Add kernel_thread routine which starts up kernel threads.
 *
 *  4-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Make thread_terminate work.
 *
 *  2-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	New kernel stack allocation mechanism.
 *
 * 27-Feb-87  David L. Black (dlb) at Carnegie-Mellon University
 *	MACH_TIME_NEW: Added timer inits to thread_create().
 *
 * 24-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Rewrote thread_suspend/thread_hold and added thread_wait for new
 *	user synchronization paradigm.
 *
 * 24-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Reorded locking protocol in thread_deallocate for the
 *	all_threads_lock (this allows one to reference a thread then
 *	release the all_threads_lock when scanning the thread list).
 *
 * 31-Jan-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Merged in my changes for real thread implementation.
 *
 * 30-Sep-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Make floating u-area work, maintain list of threads per task.
 *
 *  1-Aug-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added initialization for Mach-style IPC.
 *
 *  7-Jul-86  Rick Rashid (rfr) at Carnegie-Mellon University
 *	Added thread_in_use_chain to keep track of threads which
 *	have been created but not yet destroyed.
 *
 * 31-May-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Initialize thread state field to THREAD_WAITING.  Some general
 *	cleanup.
 *
 */
/*
 *	File:	kern/thread.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young, David Golub
 *	Date:	1986
 *
 *	Thread management primitives implementation.
 */

#include <cpus.h>
#include <hw_footprint.h>
#include <mach_host.h>
#include <mach_kdb.h>
#include <mach_pcsample.h>
#include <mach_rt.h>
#include <simple_clock.h>
#include <mach_debug.h>
#include <net_atm.h>

#include <mach/std_types.h>
#include <mach/policy.h>
#include <mach/thread_info.h>
#include <mach/thread_special_ports.h>
#include <mach/thread_status.h>
#include <mach/time_value.h>
#include <mach/vm_param.h>

#include <kern/ast.h>
#include <kern/clock.h>
#include <kern/counters.h>
#include <kern/ipc_tt.h>
#include <kern/mach_param.h>
#include <kern/processor.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/stack.h>
#include <kern/syscall_subr.h>
#include <kern/thread.h>
#include <kern/thread_swap.h>
#include <kern/host.h>
#include <kern/zalloc.h>
#include <sched_policy/standard.h>
#include <vm/vm_kern.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_port.h>
#include <ipc/mach_msg.h>
#include <machine/machspl.h>		/* for splsched */
#include <machine/thread.h>		/* for MACHINE_STACK */

#if	MACH_RT
#include <kern/rt_thread.h>
#endif

#if	NET_ATM
#include <chips/nw_mk.h>
#endif

thread_t active_threads[NCPUS];
vm_offset_t active_stacks[NCPUS];

struct zone *thread_zone;

queue_head_t		reaper_queue;
decl_simple_lock_data(,	reaper_lock)

/* private */
struct thread	thread_template;

#if	MACH_DEBUG
void stack_init(vm_offset_t stack);	/* forward */
void stack_finalize(vm_offset_t stack);	/* forward */

#define	STACK_MARKER	0xdeadbeefU
boolean_t		stack_check_usage = FALSE;
decl_simple_lock_data(,	stack_usage_lock)
vm_size_t		stack_max_usage = 0;
#endif	/* MACH_DEBUG */

/*
 *	Machine-dependent code must define:
 *		pcb_init
 *		pcb_terminate
 *		pcb_collect
 *
 *	The thread->pcb field is reserved for machine-dependent code.
 */

#ifdef	MACHINE_STACK
/*
 *	Machine-dependent code must define:
 *		stack_alloc_try
 *		stack_alloc
 *		stack_free
 *		stack_handoff
 *		stack_collect
 *	and if MACH_DEBUG:
 *		stack_statistics
 */
#else	/* MACHINE_STACK */
/*
 *	We allocate stacks from generic kernel VM.
 *	Machine-dependent code must define:
 *		stack_attach
 *		stack_detach
 *		stack_handoff
 *
 *	The stack_free_list can only be accessed at splsched,
 *	because stack_alloc_try/thread_invoke operate at splsched.
 */

decl_simple_lock_data(, stack_lock_data)/* splsched only */
#define stack_lock()	simple_lock(&stack_lock_data)
#define stack_unlock()	simple_unlock(&stack_lock_data)

vm_offset_t stack_free_list;		/* splsched only */
unsigned int stack_free_count = 0;	/* splsched only */
unsigned int stack_free_limit = 1;	/* patchable */

unsigned int stack_alloc_hits = 0;	/* debugging */
unsigned int stack_alloc_misses = 0;	/* debugging */
unsigned int stack_alloc_max = 0;	/* debugging */

/*
 *	The next field is at the base of the stack,
 *	so the low end is left unsullied.
 */

#define stack_next(stack) (*((vm_offset_t *)((stack) + KERNEL_STACK_SIZE) - 1))

/*
 *	stack_alloc_try:
 *
 *	Non-blocking attempt to allocate a kernel stack.
 *	Called at splsched with the thread locked.
 */

boolean_t stack_alloc_try(
	thread_t	thread,
	no_return	(*resume)(thread_t))
{
	register vm_offset_t stack;

	stack_lock();
	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	} else {
		stack = thread->stack_privilege;
	}
	stack_unlock();

	if (stack != 0) {
		stack_attach(thread, stack, resume);
		stack_alloc_hits++;
		return TRUE;
	} else {
		stack_alloc_misses++;
		return FALSE;
	}
}

/*
 *	stack_alloc:
 *
 *	Allocate a kernel stack for a thread.
 *	May block.
 */

void stack_alloc(
	thread_t	thread,
	no_return	(*resume)(thread_t))
{
	vm_offset_t stack;
	spl_t s;

	/*
	 *	We first try the free list.  It is probably empty,
	 *	or stack_alloc_try would have succeeded, but possibly
	 *	a stack was freed before the swapin thread got to us.
	 */

	s = splsched();
	stack_lock();
	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	}
	stack_unlock();
	splx(s);

	if (stack == 0) {
		/*
		 *	Kernel stacks should be naturally aligned,
		 *	so that it is easy to find the starting/ending
		 *	addresses of a stack given an address in the middle.
		 */

		if (kmem_alloc_aligned(kernel_map, &stack, KERNEL_STACK_SIZE)
							!= KERN_SUCCESS)
			panic("stack_alloc");

#if	MACH_DEBUG
		stack_init(stack);
#endif	/* MACH_DEBUG */
	}

	stack_attach(thread, stack, resume);
}

/*
 *	stack_free:
 *
 *	Free a thread's kernel stack.
 *	Called at splsched with the thread locked.
 */

void stack_free(
	thread_t thread)
{
	register vm_offset_t stack;

	stack = stack_detach(thread);

	if (stack != thread->stack_privilege) {
		stack_lock();
		stack_next(stack) = stack_free_list;
		stack_free_list = stack;
		if (++stack_free_count > stack_alloc_max)
			stack_alloc_max = stack_free_count;
		stack_unlock();
	}
}

/*
 *	stack_collect:
 *
 *	Free excess kernel stacks.
 *	May block.
 */

void stack_collect(void)
{
	register vm_offset_t stack;
	spl_t s;

	s = splsched();
	stack_lock();
	while (stack_free_count > stack_free_limit) {
		stack = stack_free_list;
		stack_free_list = stack_next(stack);
		stack_free_count--;
		stack_unlock();
		splx(s);

#if	MACH_DEBUG
		stack_finalize(stack);
#endif	/* MACH_DEBUG */
		kmem_free(kernel_map, stack, KERNEL_STACK_SIZE);

		s = splsched();
		stack_lock();
	}
	stack_unlock();
	splx(s);
}
#endif	/* MACHINE_STACK */

/*
 *	stack_privilege:
 *
 *	stack_alloc_try on this thread must always succeed.
 */

void stack_privilege(
	register thread_t thread)
{
	/*
	 *	This implementation only works for the current thread.
	 */

	if (thread != current_thread())
		panic("stack_privilege");

	if (thread->stack_privilege == 0)
		thread->stack_privilege = current_stack();
}

void thread_init(void)
{
	thread_zone = zinit(
			sizeof(struct thread),
			THREAD_MAX * sizeof(struct thread),
			THREAD_CHUNK * sizeof(struct thread),
			FALSE, "threads");

	/*
	 *	Fill in a template thread for fast initialization.
	 *	[Fields that must be (or are typically) reset at
	 *	time of creation are so noted.]
	 */

	/* thread_template.links (none) */
	thread_template.runq = RUN_QUEUE_HEAD_NULL;

	/* thread_template.task (later) */
	/* thread_template.thread_list (later) */
	/* thread_template.pset_threads (later) */

	/* one ref for being alive; one for the guy who creates the thread */
	thread_template.ref_count = 2;
	/* thread_template.ref_lock (later) */
	/* thread_template.lock (later) */

	/* thread_template.pcb (later) */
	thread_template.kernel_stack = (vm_offset_t) 0;
	thread_template.stack_privilege = (vm_offset_t) 0;

	thread_template.swap_func = thread_bootstrap_return;

	/* thread_template.sched_lock (later) */
	thread_template.wait_event = 0;
	/* thread_template.suspend_count (later) */
	thread_template.wait_result = KERN_SUCCESS;
	thread_template.suspend_wait = FALSE;
	thread_template.state = TH_SUSP | TH_SWAPPED;

	thread_template.active = FALSE; /* reset */
	thread_template.ast = AST_ZILCH;

	thread_template.user_stop_count = 1;

	timer_init(&(thread_template.user_timer));
	timer_init(&(thread_template.system_timer));
	thread_template.user_timer_save.low = 0;
	thread_template.user_timer_save.high = 0;
	thread_template.system_timer_save.low = 0;
	thread_template.system_timer_save.high = 0;
	thread_template.cpu_delta = 0;
	thread_template.sched_delta = 0;
	thread_template.cpu_usage = 0;
	thread_template.sched_usage = 0;
	/* thread_template.sched_stamp (later) */

	thread_template.recover = (vm_offset_t) 0;
	thread_template.vm_privilege = FALSE;

	/* thread_template.<IPC structures> (later) */

	/* thread_template.processor_set (later) */
#if	NCPUS > 1
	thread_template.bound_processor = PROCESSOR_NULL;
	/* thread_template.last_processor  (later) */
#endif

#if	MACH_PCSAMPLE
	thread_template.pc_sample.buffer = 0;
	thread_template.pc_sample.seqno = 0;
	thread_template.pc_sample.sampletypes = 0;
#endif

#if	MACH_RT
	thread_template.rt_wakeup_timer = 0;
	thread_template.rt_deadline_timer = 0;
	thread_template.rt_deadline_port = IP_NULL;
#endif	/* MACH_RT */

/*	thread_template.policy_index (later) */
	thread_template.cur_policy = 0;
/*	thread_template.sched_policy (later) */

	/*
	 *	Initialize other data structures used in
	 *	this module.
	 */

	queue_init(&reaper_queue);
	simple_lock_init(&reaper_lock);

#ifndef	MACHINE_STACK
	simple_lock_init(&stack_lock_data);
#endif	/* MACHINE_STACK */

#if	MACH_DEBUG
	simple_lock_init(&stack_usage_lock);
#endif	/* MACH_DEBUG */

	/*
	 *	Initialize any machine-dependent
	 *	per-thread structures necessary.
	 */

	pcb_module_init();
}

kern_return_t thread_create(
	register task_t	parent_task,
	thread_t	*child_thread)		/* OUT */
{
	register thread_t	new_thread;
	register processor_set_t	pset;

	if (parent_task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	/*
	 *	Allocate a thread and initialize static fields
	 */

	new_thread = (thread_t) zalloc(thread_zone);

	if (new_thread == THREAD_NULL)
		return KERN_RESOURCE_SHORTAGE;

	*new_thread = thread_template;

	/*
	 *	Initialize runtime-dependent fields
	 */

	new_thread->task = parent_task;
	simple_lock_init(&new_thread->lock);
	simple_lock_init(&new_thread->ref_lock);
	simple_lock_init(&new_thread->sched_lock);
	new_thread->sched_stamp = sched_tick;
	new_thread->timer.te_param = new_thread;
	new_thread->timer.te_clock = sys_clock;

	/*
	 *	Create a pcb.  The kernel stack is created later,
	 *	when the thread is swapped-in.
	 */
	pcb_init(new_thread);

	ipc_thread_init(new_thread);

#if	NET_ATM
	new_thread->nw_ep_waited = 0;
#endif

	/*
	 *	Find the processor set for the parent task.
	 */
	task_lock(parent_task);
	if (!parent_task->active) {
	    /*
	     *	Parent task is being shut down.  Quit now.
	     */
	    ipc_thread_terminate(new_thread);
	    pcb_terminate(new_thread);
	    zfree(thread_zone, (vm_offset_t) new_thread);

	    return KERN_FAILURE;
	}
	     
	pset = parent_task->processor_set;
	pset_lock(pset);
#if	MACH_HOST
	if (!pset->active) {
	    /*
	     *	Parent task`s processor set is being deallocated.
	     *	Assign thread to default processor set.
	     */
	    pset_unlock(pset);
	    pset = &default_pset;
	    pset_lock(pset);
	}
#endif

	/*
	 *	Set the thread`s scheduling policy and parameters
	 *	to the default for the task.
	 */
	new_thread->processor_set = pset;	/* for sched_ops to work */

	thread_set_initial_policy(new_thread, parent_task);

	/*
	 *	Thread is suspended if the task is.  Add 1 to
	 *	suspend count since thread is created in suspended
	 *	state.
	 */
	new_thread->suspend_count = parent_task->suspend_count + 1;

	/*
	 *	Add the thread to the processor set.
	 *	If the pset is empty, suspend the thread again.
	 */

	pset_reference(pset);		/* thread`s ref to pset */
	pset_add_thread(pset, new_thread);
	if (pset->empty)
		new_thread->suspend_count++;

#if	HW_FOOTPRINT
	/*
	 *	Need to set last_processor.  An idle processor
	 *	would be best, but that requires extra locking
	 *	nonsense.  Go for tail of processors queue to
	 *	avoid master.
	 */
	if (!pset->empty) {
		new_thread->last_processor = 
			(processor_t)queue_first(&pset->processors);
	}
	else {
		/*
		 *	Thread created in empty processor set.  Pick
		 *	master processor as an acceptable legal value.
		 */
		new_thread->last_processor = master_processor;
	}
#else	/* HW_FOOTPRINT */
	/*
	 *	Don't need to initialize because the context switch
	 *	code will set it before it can be used.
	 */
#endif	/* HW_FOOTPRINT */

	/*
	 *	Add the thread to the task`s list of threads.
	 *	The new thread holds another reference to the task.
	 */

	parent_task->ref_count++;

	parent_task->thread_count++;
	queue_enter(&parent_task->thread_list, new_thread, thread_t,
					thread_list);

	/*
	 *	Finally, mark the thread active.
	 */

	new_thread->active = TRUE;

	task_unlock(parent_task);
	pset_unlock(pset);

	ipc_thread_enable(new_thread);

	*child_thread = new_thread;
	return KERN_SUCCESS;
}

unsigned int thread_deallocate_stack = 0;

void thread_deallocate(
	register thread_t	thread)
{
	register task_t	task;
	time_spec_t	user_time, system_time;

	if (thread == THREAD_NULL)
		return;

	/*
	 *	First, check for new count > 0 (the common case).
	 *	Only the thread`s reference count needs to be locked.
	 */
	thread_ref_lock(thread);
	if (--thread->ref_count > 0) {
		thread_ref_unlock(thread);
		return;
	}

#if	NCPUS > 1
	/*
	 *	Count is zero.  However, the task's thread list has
	 *	an implicit reference to the thread, and may make
	 *	new ones.  Its lock also dominates the thread lock.
	 *	To check for this, we temporarily restore the one
	 *	thread reference, unlock the thread, and then lock
	 *	the structures in the proper order (task, then
	 *	thread).
	 */

	thread->ref_count = 1;
	thread_ref_unlock(thread);

	task = thread->task;

	task_lock(task);
	thread_ref_lock(thread);

	if (--thread->ref_count > 0) {
		/*
		 *	Task made an extra reference.
		 */
		thread_ref_unlock(thread);
		task_unlock(task);
		return;
	}
#else	/* NCPUS > 1 */
	task = thread->task;
#endif	/* NCPUS > 1 */

	/*
	 *	Thread has no references - we can remove it.
	 */

	/*
	 *	A couple of quick sanity checks
	 */

	if (thread == current_thread()) {
	    panic("thread deallocating itself");
	}
	if ((thread->state & ~(TH_RUN | TH_HALTED | TH_SWAPPED)) != TH_SUSP)
		panic("unstopped thread destroyed!");

	assert(thread->processor_set == PROCESSOR_SET_NULL);

	/*
	 *	Remove pending timeouts.
	 */
	timer_elt_remove(&thread->timer);

	/*
	 *	Accumulate times for dead threads in task.
	 */
	thread_read_times(thread, &user_time, &system_time);
	time_spec_add(task->total_user_time, user_time);
	time_spec_add(task->total_system_time, system_time);

	/*
	 *	Remove thread from task list.
	 */
	task->thread_count--;
	queue_remove(&task->thread_list, thread, thread_t, thread_list);

	thread_ref_unlock(thread);	/* no more references - safe */
	task_unlock(task);

	/*
	 *	Deallocate the task reference, since we know the thread
	 *	is not running.
	 */
	task_deallocate(thread->task);			/* may block */

	/*
	 *	Clean up any machine-dependent resources.
	 */
	if ((thread->state & TH_SWAPPED) == 0) {
		spl_t	s;
		s = splsched();
		stack_free(thread);
		splx(s);
		thread_deallocate_stack++;
	}

	pcb_terminate(thread);

	/*
	 *	Free the thread data structure.
	 */
	zfree(thread_zone, (vm_offset_t) thread);
}

void thread_reference(
	register thread_t	thread)
{
	if (thread == THREAD_NULL)
		return;

	thread_ref_lock(thread);
	thread->ref_count++;
	thread_ref_unlock(thread);
}

/*
 *	A version of thread_deallocate for use by AST routines,
 *	which cannot block.  If the thread`s reference count
 *	drops to zero, increment it back to one and queue it
 *	for the reaper thread.
 */
void thread_deallocate_nowait(
	thread_t	thread)
{
	thread_ref_lock(thread);
	if (--thread->ref_count > 0) {
	    thread_ref_unlock(thread);
	    return;
	}

	thread->ref_count = 1;
	thread_ref_unlock(thread);

	simple_lock(&reaper_lock);
	enqueue_tail(&reaper_queue, (queue_entry_t) thread);
	simple_unlock(&reaper_lock);
}

/*
 *	Internal code for thread_terminate, used by
 *	thread_terminate, thread_force_terminate, and
 *	thread_terminate_self.
 *
 *	Thread is inactive and halted.
 *
 *	Removes timers.  Removes thread from processor
 *	set.  Deallocates IPC structures, and deallocates
 *	thread.
 */
void thread_terminate_internal(
	thread_t	thread)
{
	processor_set_t	pset;

#if	MACH_RT
	/*
	 *	Shut down and remove the thread`s real-time
	 *	timers, deallocating them if the thread holds
	 *	the only reference.
	 */
	(void) timer_cancel(thread->rt_wakeup_timer, 0);
	(void) timer_cancel(thread->rt_deadline_timer, 0);

	(void) thread_set_periodic_timers(thread, 0, 0);
#endif

	/*
	 *	Halt the thread.
	 */
	(void) thread_halt(thread, TRUE);

	/*
	 *	Clean up the thread`s IPC ports.
	 */
	ipc_thread_terminate(thread);

#if	NET_ATM
	/*
	 *	Clean up ATM connections
	 */
	mk_waited_collect(thread);
#endif
	/*
	 *	Remove the thread from its processor set,
	 *	and deallocate the processor set.
	 */
	thread_lock(thread);
	pset = thread->processor_set;
	if (pset != PROCESSOR_SET_NULL) {
	    pset_lock(pset);
	    pset_remove_thread(pset, thread);
	    thread->processor_set = PROCESSOR_SET_NULL;
	    pset_unlock(pset);
	    thread_unlock(thread);
	    pset_deallocate(pset);
	}
	else {
	    thread_unlock(thread);
	}

	/*
	 *	Deallocate the thread`s reference to
	 *	itself.
	 */
	thread_deallocate(thread);
}

/*
 *	thread_terminate:
 *
 *	Permanently stop execution of the specified thread.
 *
 *	A thread to be terminated must be allowed to clean up any state
 *	that it has before it exits.  The thread is broken out of any
 *	wait condition that it is in, and signalled to exit.  It then
 *	cleans up its state and calls thread_halt_self on its way out of
 *	the kernel.  The caller waits for the thread to halt, terminates
 *	its IPC state, and then deallocates it.
 *
 *	If the caller is the current thread, it must still exit the kernel
 *	to clean up any state (thread and port references, messages, etc).
 *	When it exits the kernel, it then terminates its IPC state and
 *	queues itself for the reaper thread, which will wait for the thread
 *	to stop and then deallocate it.  (A thread cannot deallocate itself,
 *	since it needs a kernel stack to execute.)
 */
kern_return_t thread_terminate(
	register thread_t	thread)
{
	register thread_t	cur_thread = current_thread();
	register task_t		cur_task;
	spl_t			s;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	if (thread == cur_thread) {

	    thread_lock(thread);
	    if (thread->active) {
		/*
		 *	Curreht thread is active, and is terminating
		 *	itself.  Make thread queue itself for reaper
		 *	when exiting kernel.  Reaper will do the rest
		 *	of the work.
		 */
		thread->active = FALSE;
		ipc_thread_disable(thread);

		s = splsched();
		thread_sched_lock(thread);
		thread_ast_set(thread, AST_TERMINATE);	/* queue for reaper */
		thread_sched_unlock(thread);

		ast_on(cpu_number(), AST_TERMINATE);
		splx(s);

		thread_unlock(thread);
		return KERN_SUCCESS;
	    }
	    else {
		/*
		 *	Someone else is already terminating the
		 *	current thread.  Just make it halt.  The
		 *	thread that is calling thread_terminate
		 *	is (or will be) waiting	for this one to
		 *	halt, and will do the rest of the work.
		 *
		 *	Hold the thread, because the thread that
		 *	is calling thread_terminate may not yet
		 *	have called thread_halt on this thread.
		 */
		thread_hold(thread);

		s = splsched();
		thread_sched_lock(thread);
		thread_ast_set(thread, AST_HALT);	/* don`t queue
							   for reaper */
		thread_sched_unlock(thread);

		ast_on(cpu_number(), AST_HALT);
		splx(s);

		thread_unlock(thread);
		return KERN_FAILURE;
	    }
	}

	/*
	 *	Lock both threads and the current task
	 *	to check termination races and prevent deadlocks.
	 */
	cur_task = cur_thread->task;
	task_lock(cur_task);

	if ((vm_offset_t)thread < (vm_offset_t)cur_thread) {
		thread_lock(thread);
		thread_lock(cur_thread);
	}
	else {
		thread_lock(cur_thread);
		thread_lock(thread);
	}

	/*
	 *	If the current thread is being terminated, help out.
	 */
	if (!cur_task->active || !cur_thread->active) {
		thread_unlock(cur_thread);
		thread_unlock(thread);

		task_unlock(cur_task);
		(void) thread_terminate(cur_thread);
		return KERN_FAILURE;
	}
    
	thread_unlock(cur_thread);
	task_unlock(cur_task);

	/*
	 *	Terminate victim thread.
	 */
	if (!thread->active) {
		/*
		 *	Someone else got there first.
		 */
		thread_unlock(thread);

		return KERN_FAILURE;
	}

	/*
	 *	Mark thread inactive, and disable IPC access.
	 */
	thread->active = FALSE;
	ipc_thread_disable(thread);

	thread_unlock(thread);

	/*
	 *	Terminate the thread.
	 */
	thread_terminate_internal(thread);

	return KERN_SUCCESS;
}

/*
 *	thread_force_terminate:
 *
 *	Version of thread_terminate called by task_terminate.  thread is
 *	not the current thread.  task_terminate is the dominant operation,
 *	so we can force this thread to stop.
 */
void
thread_force_terminate(
	register thread_t	thread)
{
	/*
	 *	If thread is already being shut down,
	 *	we don`t have to do anything.
	 */
	thread_lock(thread);
	if (!thread->active) {
	    thread_unlock(thread);
	    return;
	}

	/*
	 *	Mark thread inactive, and shut down its IPC
	 *	control.
	 */
	thread->active = FALSE;
	ipc_thread_disable(thread);

	thread_unlock(thread);

	/*
	 *	Terminate the thread.
	 */
	thread_terminate_internal(thread);
}

no_return
walking_zombie(void)
{
	for (;;)
	    panic("the zombie walks!");
}

/*
 *	A thread can only terminate itself when it has
 *	hit a clean point.  It calls this function to
 *	mark itself as halted, and queue itself for the
 *	reaper thread.  The reaper thread actually
 *	cleans up the thread.
 *
 *	Thread is already marked inactive.
 */
no_return thread_terminate_self(void)
{
	thread_t	thread = current_thread();
	spl_t		s;

	thread_hold(thread);

	s = splsched();
	thread_sched_lock(thread);
	thread->state |= TH_HALTED;
	thread_sched_unlock(thread);
	splx(s);

	simple_lock(&reaper_lock);
	enqueue_tail(&reaper_queue, (queue_entry_t) thread);
	simple_unlock(&reaper_lock);
	thread_wakeup(&reaper_queue);

	counter(c_thread_halt_self_block++);
	thread_block_noreturn(walking_zombie);
	/*NOTREACHED*/
}

/*
 *	Return thread's machine-dependent state.
 */
kern_return_t thread_get_state(
	register thread_t	thread,
	int			flavor,
	thread_state_t		old_state,	/* pointer to OUT array */
	natural_t		*old_state_count)	/*IN/OUT*/
{
	kern_return_t		ret;

	if (thread == THREAD_NULL || thread == current_thread()) {
		return KERN_INVALID_ARGUMENT;
	}

	thread_hold(thread);
	(void) thread_dowait(thread, TRUE);

	ret = thread_getstatus(thread, flavor, old_state, old_state_count);

	thread_release(thread);
	return ret;
}

/*
 *	Change thread's machine-dependent state.
 */
kern_return_t thread_set_state(
	register thread_t	thread,
	int			flavor,
	thread_state_t		new_state,
	natural_t		new_state_count)
{
	kern_return_t		ret;

	if (thread == THREAD_NULL || thread == current_thread()) {
		return KERN_INVALID_ARGUMENT;
	}

	thread_hold(thread);
	(void) thread_dowait(thread, TRUE);

	ret = thread_setstatus(thread, flavor, new_state, new_state_count);

	thread_release(thread);
	return ret;
}

kern_return_t thread_info(
	register thread_t	thread,
	int			flavor,
	thread_info_t		thread_info_out,    /* pointer to OUT array */
	natural_t		*thread_info_count) /*IN/OUT*/
{
	int			state, flags;
	spl_t			s;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	if (flavor == THREAD_BASIC_INFO) {
	    register thread_basic_info_t	basic_info;
	    unsigned int	sleep_time;
	    time_spec_t		user_time, system_time;

	    if (*thread_info_count < THREAD_BASIC_INFO_COUNT) {
		return KERN_INVALID_ARGUMENT;
	    }

	    basic_info = (thread_basic_info_t) thread_info_out;

	    s = splsched();
	    thread_sched_lock(thread);

	    /*
	     *	Update lazy-evaluated scheduler info because someone wants it.
	     *	Grab sleep-time first, since UPDATE_PRIORITY zeros it.
	     */
	    sleep_time = sched_tick - thread->sched_stamp;
	    if ((thread->state & TH_RUN) == 0) {
		UPDATE_PRIORITY(thread);
	    }

	    /* fill in info */

	    thread_read_times(thread, &user_time, &system_time);
	    basic_info->user_time.seconds = user_time.seconds;
	    basic_info->user_time.microseconds =
				user_time.nanoseconds / 1000;
	    basic_info->system_time.seconds = system_time.seconds;
	    basic_info->system_time.microseconds =
				system_time.nanoseconds / 1000;

	    switch (thread->sched_policy->name) {
		case POLICY_BACKGROUND:
		    basic_info->base_priority = 32;
		    basic_info->cur_priority = 32;
		    break;

		case POLICY_TIMESHARE:
		{
		    struct policy_info_timeshare	info;
		    natural_t				count;

		    count = POLICY_INFO_TIMESHARE_COUNT;
		    (void) THREAD_GET_PARAM(thread,
					    (policy_param_t)&info,
					    &count);

		    basic_info->base_priority = info.base_priority;
		    basic_info->cur_priority  = info.cur_priority;
		    break;
		}

		default:
		    basic_info->base_priority = 0;
		    basic_info->cur_priority = 0;
		    break;
	    }

	    /*
	     *	To calculate cpu_usage, first correct for timer rate,
	     *	then for 5/8 ageing.  The correction factor [3/5] is
	     *	(1/(5/8) - 1).
	     */
	    basic_info->cpu_usage = thread->cpu_usage /
					(USAGE_RATE/TH_USAGE_SCALE);
	    basic_info->cpu_usage = (basic_info->cpu_usage * 3) / 5;
#if	SIMPLE_CLOCK
	    /*
	     *	Clock drift compensation.
	     */
	    basic_info->cpu_usage =
		(basic_info->cpu_usage * 1000000)/sched_usec;
#endif	/* SIMPLE_CLOCK */

	    if (thread->state & TH_SWAPPED)
		flags = TH_FLAGS_SWAPPED;
	    else if (thread->state & TH_IDLE)
		flags = TH_FLAGS_IDLE;
	    else
		flags = 0;

	    if (thread->state & TH_HALTED)
		state = TH_STATE_HALTED;
	    else
	    if (thread->state & TH_RUN)
		state = TH_STATE_RUNNING;
	    else
	    if (thread->state & TH_UNINT)
		state = TH_STATE_UNINTERRUPTIBLE;
	    else
	    if (thread->state & TH_SUSP)
		state = TH_STATE_STOPPED;
	    else
	    if (thread->state & TH_WAIT)
		state = TH_STATE_WAITING;
	    else
		state = 0;		/* ? */

	    basic_info->run_state = state;
	    basic_info->flags = flags;
	    basic_info->suspend_count = thread->user_stop_count;
	    if (state == TH_STATE_RUNNING)
		basic_info->sleep_time = 0;
	    else
		basic_info->sleep_time = sleep_time;

	    thread_sched_unlock(thread);
	    splx(s);

	    *thread_info_count = THREAD_BASIC_INFO_COUNT;
	    return KERN_SUCCESS;
	}
	else if (flavor == THREAD_SCHED_INFO) {
	    register thread_sched_info_t	sched_info;

	    if (*thread_info_count < THREAD_SCHED_INFO_COUNT) {
		return KERN_INVALID_ARGUMENT;
	    }

	    sched_info = (thread_sched_info_t) thread_info_out;

	    s = splsched();
	    thread_sched_lock(thread);

	    sched_info->policy = thread->sched_policy->name;
	    switch (thread->sched_policy->name) {
		case POLICY_BACKGROUND:
		    sched_info->base_priority = 32;
		    sched_info->cur_priority = 32;
		    sched_info->max_priority = 32;
		    break;

		case POLICY_TIMESHARE:
		{
		    struct policy_info_timeshare	info;
		    natural_t				count;

		    count = POLICY_INFO_TIMESHARE_COUNT;
		    (void) THREAD_GET_PARAM(thread,
					    (policy_param_t) &info,
					    &count);

		    sched_info->base_priority = info.base_priority;
		    sched_info->cur_priority  = info.cur_priority;
		    sched_info->max_priority  = info.max_priority;
		}
		default:
		    sched_info->base_priority = 0;
		    sched_info->cur_priority = 0;
		    sched_info->max_priority = 0;
		    break;
	    }

	    if (thread->cur_policy != thread->sched_policy) {
		sched_info->depressed = TRUE;
		sched_info->depress_priority = sched_info->cur_priority;
		sched_info->cur_priority = 32;
	    }
	    else {
		sched_info->depressed = FALSE;
		sched_info->depress_priority = -1;
	    }

	    thread_sched_unlock(thread);
	    splx(s);

	    *thread_info_count = THREAD_SCHED_INFO_COUNT;
	    return KERN_SUCCESS;
	}
	else if (flavor == THREAD_POLICY_INFO) {
	    kern_return_t	kr;
	    thread_policy_info_t policy_info;
	    natural_t		policy_info_count;

	    if (*thread_info_count < THREAD_POLICY_INFO_COUNT)
		return KERN_INVALID_ARGUMENT;

	    policy_info = (thread_policy_info_t) thread_info_out;

	    s = splsched();
	    thread_sched_lock(thread);

	    policy_info->policy = thread->sched_policy->name;
	    policy_info->depressed =
		(thread->sched_policy != thread->cur_policy);

	    policy_info_count = *thread_info_count - THREAD_POLICY_INFO_COUNT;
	    if (policy_info_count > 0) {
		/*
		 *	There is room for the detailed scheduling policy
		 *	information.
		 */
		kr = THREAD_GET_PARAM(thread,
				      (policy_param_t) (policy_info + 1),
				      &policy_info_count);
		if (kr == KERN_SUCCESS)
		    *thread_info_count = THREAD_POLICY_INFO_COUNT +
			     policy_info_count;
	    }
	    else {
		*thread_info_count = THREAD_POLICY_INFO_COUNT;
		kr = KERN_SUCCESS;
	    }

	    thread_sched_unlock(thread);
	    splx(s);

	    return kr;
	}

	return KERN_INVALID_ARGUMENT;
}

kern_return_t	thread_abort(
	register thread_t	thread)
{
	if (thread == THREAD_NULL || thread == current_thread()) {
		return KERN_INVALID_ARGUMENT;
	}

	/*
	 *	Try to force the thread to a clean point.
	 *	If the halt operation fails return KERN_ABORTED.
	 *	ipc code will convert this to an ipc interrupted error code.
	 */
	if (thread_halt(thread, FALSE) != KERN_SUCCESS)
		return KERN_ABORTED;

	/*
	 *	If the thread was in an exception, abort that too.
	 */
	mach_msg_abort_rpc(thread);

	/*
	 *	Also abort any depression.
	 */
	if (thread->cur_policy != thread->sched_policy)
	    thread_depress_abort(thread);

	/*
	 *	Then set it going again.
	 */
	thread_release(thread);

	return KERN_SUCCESS;
}

/*
 *	thread_start:
 *
 *	Start a thread at the specified routine.
 *	The thread must	be in a swapped state.
 */

void
thread_start(
	thread_t	thread,
	continuation_t	start)
{
	thread->swap_func = start;
}

/*
 *	kernel_thread:
 *
 *	Start up a kernel thread in the specified task.
 */

thread_t kernel_thread(
	task_t		task,
	continuation_t	start,
	void *		arg)
{
	thread_t	thread;

	(void) thread_create(task, &thread);
	/* release "extra" ref that thread_create gave us */
	thread_deallocate(thread);
	thread_start(thread, start);
	thread->ith_other = arg;

	/*
	 *	We ensure that the kernel thread starts with a stack.
	 *	The swapin mechanism might not be operational yet.
	 */
	thread_doswapin(thread);

	/*
	 *	Start the thread running.
	 */
	(void) thread_resume(thread);
	return thread;
}

/*
 *	reaper_thread:
 *
 *	This kernel thread runs forever looking for threads to destroy
 *	(when they request that they be destroyed, of course).
 */
no_return reaper_thread(void)
{
	register thread_t thread;

	for (;;) {

		simple_lock(&reaper_lock);

		while ((thread = (thread_t) dequeue_head(&reaper_queue))
							!= THREAD_NULL) {
			simple_unlock(&reaper_lock);

			/*
			 *	We have a thread to terminate.
			 */
			thread_terminate_internal(thread);	/* may block */

			/*
			 *	Check for kernel ASTs in loop.
			 */
			AST_KERNEL_CHECK(cpu_number());

			simple_lock(&reaper_lock);
		}

		assert_wait((event_t) &reaper_queue, FALSE);
		simple_unlock(&reaper_lock);
		counter(c_reaper_thread_block++);
		thread_block(reaper_thread);
	}
}

#if	MACH_HOST
/*
 *	thread_assign:
 *
 *	Change processor set assignment.
 *	Caller must hold an extra reference to the thread (if this is
 *	called directly from the ipc interface, this is an operation
 *	in progress reference).  Caller must hold no locks -- this may block.
 */

kern_return_t
thread_assign(
	thread_t	thread,
	processor_set_t	new_pset)
{
	register processor_set_t	old_pset;
	register boolean_t		old_empty, new_empty;

	if (thread == THREAD_NULL || new_pset == PROCESSOR_SET_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	/*
	 *	Suspend the thread and stop it if it's not the current thread.
	 */
	thread_hold(thread);
	if (thread != current_thread())
		(void) thread_dowait(thread, TRUE);

	/*
	 *	Lock the thread, and find its current processor set.
	 */
	thread_lock(thread);
	old_pset = thread->processor_set;
	if (old_pset == new_pset) {
		/*
		 *	Nothing to do.
		 */
		thread_unlock(thread);
		return KERN_SUCCESS;
	}

	/*
	 *	Lock both psets now, use ordering to avoid deadlocks.
	 */
Restart:
	if ((vm_offset_t)old_pset < (vm_offset_t)new_pset) {
	    pset_lock(old_pset);
	    pset_lock(new_pset);
	}
	else {
	    pset_lock(new_pset);
	    pset_lock(old_pset);
	}

	/*
	 *	Check if new_pset is ok to assign to.  If not, reassign
	 *	to default_pset.
	 */
	if (!new_pset->active) {
	    pset_unlock(old_pset);
	    pset_unlock(new_pset);
	    new_pset = &default_pset;
	    goto Restart;
	}

	pset_reference(new_pset);

	/*
	 *	Move the thread.
	 *	Then drop the lock on the old pset and the thread's
	 *	reference to it.
	 */
	thread_change_psets(thread, old_pset, new_pset);

	old_empty = old_pset->empty;
	new_empty = new_pset->empty;

	/*
	 *	Reset policy and priorities if needed.
	 */
	{
	    spl_t	s;

	    s = splsched();
	    thread_sched_lock(thread);
	    thread_enforce_policy_limits(thread, new_pset);
	    thread_sched_unlock(thread);
	    splx(s);
	}

	thread_unlock(thread);

	pset_unlock(old_pset);
	pset_unlock(new_pset);

	pset_deallocate(old_pset);

	/*
	 *	Figure out hold status of thread.  Threads assigned to empty
	 *	psets must be held.  Therefore:
	 *		If old pset was empty release its hold.
	 *		Release our hold from above unless new pset is empty.
	 */

	if (old_empty)
		thread_release(thread);
	if (!new_empty)
		thread_release(thread);

	/*
	 *	If current_thread is assigned, context switch to force
	 *	assignment to happen.  This also causes hold to take
	 *	effect if the new pset is empty.
	 */
	if (thread == current_thread()) {
		spl_t	s;

		s = splsched();
		ast_on(cpu_number(), AST_BLOCK);
		splx(s);
	}

	return KERN_SUCCESS;
}

/*
 *	Thread_assign_if_empty:
 *
 *	Move thread to default processor set if its own is
 *	empty.  Return the thread`s processor set if so.
 *
 *	Thread is already suspended.
 */
processor_set_t
thread_assign_if_empty(
	thread_t	thread)
{
	processor_set_t	old_pset;

	thread_lock(thread);
	old_pset = thread->processor_set;
	if (!old_pset->empty) {
	    /*
	     *	Don`t have to move thread.
	     */
	    thread_unlock(thread);
	    return PROCESSOR_SET_NULL;
	}

	/*
	 *	Lock both psets.
	 */
	if ((vm_offset_t) old_pset < (vm_offset_t) &default_pset) {
	    pset_lock(old_pset);
	    pset_lock(&default_pset);
	}
	else {
	    pset_lock(&default_pset);
	    pset_lock(old_pset);
	}

	assert(default_pset.active);

	pset_reference(&default_pset);

	thread_change_psets(thread, old_pset, &default_pset);

	/*
	 *	Reset policy and priorities if needed.
	 */
	{
	    spl_t	s;

	    s = splsched();
	    thread_sched_lock(thread);
	    thread_enforce_policy_limits(thread, &default_pset);
	    thread_sched_unlock(thread);
	    splx(s);
	}

	thread_unlock(thread);

	pset_unlock(old_pset);
	pset_unlock(&default_pset);

	/*
	 *	Old pset was empty, so release thread.
	 */
	thread_release(thread);

	return old_pset;		/* returns ref */
}

#else	/* MACH_HOST */
kern_return_t
thread_assign(
	thread_t	thread,
	processor_set_t	new_pset)
{
	return KERN_FAILURE;
}
#endif	/* MACH_HOST */

/*
 *	thread_assign_default:
 *
 *	Special version of thread_assign for assigning threads to default
 *	processor set.
 */
kern_return_t
thread_assign_default(
	thread_t	thread)
{
	return thread_assign(thread, &default_pset);
}

/*
 *	thread_get_assignment
 *
 *	Return current assignment for this thread.
 */	    
kern_return_t thread_get_assignment(
	thread_t	thread,
	processor_set_t	*pset)
{
	thread_lock(thread);
	*pset = thread->processor_set;
	pset_reference(*pset);
	thread_unlock(thread);

	return KERN_SUCCESS;
}

/*
 *	[ obsolete ]
 *	thread_priority:
 *
 *	Set priority (and possibly max priority) for thread.
 *
 *	Only works for timesharing policy.
 */
kern_return_t
thread_priority(
	thread_t	thread,
	int		priority,
	boolean_t	set_max)
{
	struct policy_param_timeshare	param;

	param.priority = priority;
	return thread_set_policy_param(thread,
				       set_max,
				       (policy_param_t)&param,
				       POLICY_PARAM_TIMESHARE_COUNT);
}

/*
 *	thread_set_own_priority:
 *
 *	Internal use only; sets the priority of the calling thread,
 *	and makes it fixed priority.
 */
void
thread_set_own_priority(
	int	priority)
{
	thread_t	thread = current_thread();
	struct policy_param_fixedpri	param;

	param.priority = priority;
	param.no_preempt = TRUE;	/* irrelevant for kernel thread */

	(void) thread_set_policy(thread,
				 thread->processor_set,
				 POLICY_FIXEDPRI,
				 (policy_param_t)&param,
				 POLICY_PARAM_FIXEDPRI_COUNT);
}

/*
 *	[ obsolete ]
 *	thread_max_priority:
 *
 *	Reset the max priority for a thread.
 *
 *	Only works for timesharing threads.
 */
kern_return_t
thread_max_priority(
	thread_t	thread,
	processor_set_t	pset,
	int		max_priority)
{
	struct policy_param_timeshare	limit;

	limit.priority = max_priority;
	return thread_set_policy_limit(thread,
				       pset,
				       (policy_param_t)&limit,
				       POLICY_PARAM_TIMESHARE_COUNT);
}

/*
 *	[ obsolete ]
 *	thread_policy:
 *
 *	Set scheduling policy for thread.
 *
 *	Since the old fixed-priority policy is not supported,
 *	this does nothing.
 */
kern_return_t
thread_policy(
	thread_t	thread,
	int		policy,
	int		data)
{
	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	if (policy == POLICY_TIMESHARE)
	    return KERN_SUCCESS;
	else
	    return KERN_FAILURE;
}

/*
 *	thread_wire:
 *
 *	Specify that the target thread must always be able
 *	to run and to allocate memory.
 */
kern_return_t
thread_wire(
	host_t		host,
	thread_t	thread,
	boolean_t	wired)
{
	spl_t		s;

	if (host == HOST_NULL)
	    return KERN_INVALID_ARGUMENT;

	if (thread == THREAD_NULL)
	    return KERN_INVALID_ARGUMENT;

	/*
	 * This implementation only works for the current thread.
	 * See stack_privilege.
	 */
	if (thread != current_thread())
	    return KERN_INVALID_ARGUMENT;

	s = splsched();
	thread_sched_lock(thread);

	if (wired) {
	    thread->vm_privilege = TRUE;
	    stack_privilege(thread);
	}
	else {
	    thread->vm_privilege = FALSE;
/*XXX	    stack_unprivilege(thread); */
	    thread->stack_privilege = 0;
	}

	thread_sched_unlock(thread);
	splx(s);

	return KERN_SUCCESS;
}

/*
 *	thread_collect_scan:
 *
 *	Attempt to free resources owned by threads.
 *	pcb_collect doesn't do anything yet.
 */

void thread_collect_scan(void)
{
#if	0
	register thread_t	thread, prev_thread;
	processor_set_t		pset, prev_pset;

	prev_thread = THREAD_NULL;
	prev_pset = PROCESSOR_SET_NULL;

	simple_lock(&all_psets_lock);
	queue_iterate(&all_psets, pset, processor_set_t, all_psets) {
		pset_lock(pset);
		queue_iterate(&pset->threads, thread, thread_t, pset_threads) {
			spl_t	s = splsched();
			thread_sched_lock(thread);

			/*
			 *	Only collect threads which are
			 *	not runnable and are swapped.
			 */

			if ((thread->state & (TH_RUN|TH_SWAPPED))
							== TH_SWAPPED) {
				thread_reference(thread);
				thread_sched_unlock(thread);
				splx(s);
				pset->ref_count++;
				pset_unlock(pset);
				simple_unlock(&all_psets_lock);

				pcb_collect(thread);

				if (prev_thread != THREAD_NULL)
					thread_deallocate(prev_thread);
				prev_thread = thread;

				if (prev_pset != PROCESSOR_SET_NULL)
					pset_deallocate(prev_pset);
				prev_pset = pset;

				simple_lock(&all_psets_lock);
				pset_lock(pset);
			} else {
				thread_sched_unlock(thread);
				splx(s);
			}
		}
		pset_unlock(pset);
	}
	simple_unlock(&all_psets_lock);

	if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);
	if (prev_pset != PROCESSOR_SET_NULL)
		pset_deallocate(prev_pset);
#endif	/* 0 */
}

boolean_t thread_collect_allowed = TRUE;
unsigned thread_collect_last_tick = 0;
unsigned thread_collect_max_rate = 0;		/* in seconds */

/*
 *	consider_thread_collect:
 *
 *	Called by the pageout daemon when the system needs more free pages.
 */

void consider_thread_collect(void)
{
	/*
	 *	By default, don't attempt thread collection more frequently
	 *	than once a minute.
	 */

	if (thread_collect_max_rate == 0)
		thread_collect_max_rate = 60;

	if (thread_collect_allowed &&
	    (sched_tick >
	     (thread_collect_last_tick + thread_collect_max_rate))) {
		thread_collect_last_tick = sched_tick;
		thread_collect_scan();
	}
}

#if	MACH_DEBUG

vm_size_t stack_usage(
	register vm_offset_t stack)
{
	int i;

	for (i = 0; i < KERNEL_STACK_SIZE/sizeof(unsigned int); i++)
	    if (((unsigned int *)stack)[i] != STACK_MARKER)
		break;

	return KERNEL_STACK_SIZE - i * sizeof(unsigned int);
}

/*
 *	Machine-dependent code should call stack_init
 *	before doing its own initialization of the stack.
 */

void stack_init(
	register vm_offset_t stack)
{
	if (stack_check_usage) {
	    int i;

	    for (i = 0; i < KERNEL_STACK_SIZE/sizeof(unsigned int); i++)
		((unsigned int *)stack)[i] = STACK_MARKER;
	}
}

/*
 *	Machine-dependent code should call stack_finalize
 *	before releasing the stack memory.
 */

void stack_finalize(
	register vm_offset_t stack)
{
	if (stack_check_usage) {
	    vm_size_t used = stack_usage(stack);

	    simple_lock(&stack_usage_lock);
	    if (used > stack_max_usage)
		stack_max_usage = used;
	    simple_unlock(&stack_usage_lock);
	}
}

#ifndef	MACHINE_STACK
/*
 *	stack_statistics:
 *
 *	Return statistics on cached kernel stacks.
 *	*maxusagep must be initialized by the caller.
 */

void stack_statistics(
	unsigned int	*totalp,
	vm_size_t	*maxusagep)
{
	spl_t	s;

	s = splsched();
	stack_lock();
	if (stack_check_usage) {
		vm_offset_t stack;

		/*
		 *	This is pretty expensive to do at splsched,
		 *	but it only happens when someone makes
		 *	a debugging call, so it should be OK.
		 */

		for (stack = stack_free_list; stack != 0;
		     stack = stack_next(stack)) {
			vm_size_t usage = stack_usage(stack);

			if (usage > *maxusagep)
				*maxusagep = usage;
		}
	}

	*totalp = stack_free_count;
	stack_unlock();
	splx(s);
}
#endif	/* MACHINE_STACK */

kern_return_t host_stack_usage(
	host_t		host,
	vm_size_t	*reservedp,
	unsigned int	*totalp,
	vm_size_t	*spacep,
	vm_size_t	*residentp,
	vm_size_t	*maxusagep,
	vm_offset_t	*maxstackp)
{
	unsigned int total;
	vm_size_t maxusage;

	if (host == HOST_NULL)
		return KERN_INVALID_HOST;

	simple_lock(&stack_usage_lock);
	maxusage = stack_max_usage;
	simple_unlock(&stack_usage_lock);

	stack_statistics(&total, &maxusage);

	*reservedp = 0;
	*totalp = total;
	*spacep = *residentp = total * round_page(KERNEL_STACK_SIZE);
	*maxusagep = maxusage;
	*maxstackp = 0;
	return KERN_SUCCESS;
}

kern_return_t processor_set_stack_usage(
	processor_set_t	pset,
	unsigned int	*totalp,
	vm_size_t	*spacep,
	vm_size_t	*residentp,
	vm_size_t	*maxusagep,
	vm_offset_t	*maxstackp)
{
	unsigned int total;
	vm_size_t maxusage;
	vm_offset_t maxstack;

	register thread_t *threads;
	register thread_t tmp_thread;

	unsigned int actual;	/* this many things */
	unsigned int i;

	vm_size_t size, size_needed;
	vm_offset_t addr;

	if (pset == PROCESSOR_SET_NULL)
		return KERN_INVALID_ARGUMENT;

	size = 0; addr = 0;

	for (;;) {
		pset_lock(pset);
		if (!pset->active) {
			pset_unlock(pset);
			return KERN_INVALID_ARGUMENT;
		}

		actual = pset->thread_count;

		/* do we have the memory we need? */

		size_needed = actual * sizeof(thread_t);
		if (size_needed <= size)
			break;

		/* unlock the pset and allocate more memory */
		pset_unlock(pset);

		if (size != 0)
			kfree(addr, size);

		assert(size_needed > 0);
		size = size_needed;

		addr = kalloc(size);
		if (addr == 0)
			return KERN_RESOURCE_SHORTAGE;
	}

	/* OK, have memory and the processor_set is locked & active */

	threads = (thread_t *) addr;
	for (i = 0, tmp_thread = (thread_t) queue_first(&pset->threads);
	     i < actual;
	     i++,
	     tmp_thread = (thread_t) queue_next(&tmp_thread->pset_threads)) {
		thread_reference(tmp_thread);
		threads[i] = tmp_thread;
	}
	assert(queue_end(&pset->threads, (queue_entry_t) tmp_thread));

	/* can unlock processor set now that we have the thread refs */
	pset_unlock(pset);

	/* calculate maxusage and free thread references */

	total = 0;
	maxusage = 0;
	maxstack = 0;
	for (i = 0; i < actual; i++) {
		thread_t thread = threads[i];
		vm_offset_t stack = 0;

		/*
		 *	thread->kernel_stack is only accurate if the
		 *	thread isn't swapped and is not executing.
		 *
		 *	Of course, we don't have the appropriate locks
		 *	for these shenanigans.
		 */

		if ((thread->state & TH_SWAPPED) == 0) {
			int cpu;

			stack = thread->kernel_stack;

			for (cpu = 0; cpu < NCPUS; cpu++)
				if (active_threads[cpu] == thread) {
					stack = active_stacks[cpu];
					break;
				}
		}

		if (stack != 0) {
			total++;

			if (stack_check_usage) {
				vm_size_t usage = stack_usage(stack);

				if (usage > maxusage) {
					maxusage = usage;
					maxstack = (vm_offset_t) thread;
				}
			}
		}

		thread_deallocate(thread);
	}

	if (size != 0)
		kfree(addr, size);

	*totalp = total;
	*residentp = *spacep = total * round_page(KERNEL_STACK_SIZE);
	*maxusagep = maxusage;
	*maxstackp = maxstack;
	return KERN_SUCCESS;
}
#endif	/* MACH_DEBUG */

#if	MACH_KDB
#include <ddb/db_output.h>
/*
 *	Useful in the debugger:
 */
void
thread_stats(void)
{
	register thread_t thread;
	int total = 0, rpcreply = 0;

	queue_iterate(&default_pset.threads, thread, thread_t, pset_threads) {
		total++;
		if (thread->ith_rpc_reply != IP_NULL)
			rpcreply++;
	}

	db_printf("%d total threads.\n", total);
	db_printf("%d using rpc_reply.\n", rpcreply);
}
#endif	/* MACH_KDB */
