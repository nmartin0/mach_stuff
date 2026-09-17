/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	startup.c,v $
 * Revision 2.27  93/11/17  17:24:45  dbg
 * 	Remove ancient timestamp facility.
 * 	[93/06/18            dbg]
 * 
 * 	Remove call to mapable_time_init.
 * 	[93/06/07            dbg]
 * 
 * 	Break up thread lock.
 * 	[93/05/26            dbg]
 * 
 * 	Declare continuations as returning 'no_return'.
 * 	Call idle_thread_create to build idle threads.
 * 	Call kernel_reply_kmsg_init.
 * 	Initialize system clock after finding devices, since we now
 * 	probe for the clock.
 * 	Changes for selectable scheduler policies.
 * 	[93/05/21            dbg]
 * 
 * Revision 2.26  93/05/15  18:55:03  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.25  93/01/14  17:36:31  danner
 * 	Invoke printf_init() for multiP interlock setup.
 * 	[92/12/01            af]
 * 
 * Revision 2.24  92/08/03  17:39:15  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.23  92/05/21  17:15:47  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.22  92/01/27  16:43:18  rpd
 * 	Fixed to finish vm/ipc initialization before calling machine_init.
 * 	[92/01/25            rpd]
 * 
 * Revision 2.21  92/01/14  16:44:57  rpd
 * 	Split vm_mem_init into vm_mem_bootstrap and vm_mem_init.
 * 	[91/12/29            rpd]
 * 
 * Revision 2.20  91/09/12  16:38:00  bohman
 * 	Changed launch_first_thread() to set active_stacks[] before
 * 	starting the first thread.
 * 	[91/09/11  17:08:23  bohman]
 * 
 * Revision 2.19  91/08/28  11:14:37  jsb
 * 	Start NORMA ipc and vm systems before user bootstrap.
 * 	[91/08/15  08:33:05  jsb]
 * 
 * Revision 2.18  91/08/03  18:18:59  jsb
 * 	Changed NORMA specific startup again.
 * 	[91/07/24  22:32:51  jsb]
 * 
 * Revision 2.17  91/07/31  17:48:00  dbg
 * 	Remove interrupt_stack_alloc - not all multiprocessors use it.
 * 
 * 	Revised scheduling state machine.
 * 	[91/07/30  17:05:11  dbg]
 * 
 * Revision 2.16  91/06/17  15:47:15  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:51:38  jsb]
 * 
 * Revision 2.15  91/06/06  17:07:27  jsb
 * 	Changed NORMA_IPC specific startup.
 * 	[91/05/14  09:19:55  jsb]
 * 
 * Revision 2.14  91/05/18  14:33:30  rpd
 * 	Added argument to kernel_thread.
 * 	[91/04/03            rpd]
 * 
 * Revision 2.13  91/05/14  16:47:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.12  91/03/16  14:51:45  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 	Removed thread_swappable.
 * 	[91/01/18            rpd]
 * 
 * Revision 2.11  91/02/05  17:29:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:17:40  mrt]
 * 
 * Revision 2.10  91/01/08  15:17:00  rpd
 * 	Swapin the startup thread and idle threads.
 * 	[90/11/20            rpd]
 * 
 * 	Removed swapout_thread.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.9  90/10/12  18:07:34  rpd
 * 	Fixed call to thread_bind in start_kernel_threads.
 * 	Fix from Philippe Bernadat.
 * 	[90/10/10            rpd]
 * 
 * Revision 2.8  90/09/28  16:55:37  jsb
 * 	Added NORMA_IPC support.
 * 	[90/09/28  14:05:00  jsb]
 * 
 * Revision 2.7  90/09/09  14:32:47  rpd
 * 	Fixed setup_main to call pset_sys_init.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.6  90/08/27  22:03:40  dbg
 * 	Pass processor argument to choose_thread.
 * 	Rename cpu_start to cpu_launch_first_thread to avoid conflict
 * 	with processor code.
 * 	[90/07/17            dbg]
 * 
 * Revision 2.5  90/06/02  14:56:12  rpd
 * 	Start sched_thread.  On multiprocessors, start action_thread
 * 	instead of shutdown_thread.
 * 	[90/04/28            rpd]
 * 	Updated to new scheduling technology.
 * 	[90/03/26  22:19:00  rpd]
 * 
 * Revision 2.4  90/01/11  11:44:11  dbg
 * 	If multiprocessor, start shutdown thread.  If uniprocessor,
 * 	don't allocate interrupt stacks.
 * 	[89/12/19            dbg]
 * 
 * 	Add call to start other CPUs if multiprocessor.
 * 	[89/12/01            dbg]
 * 
 * Revision 2.3  89/09/08  11:26:32  dbg
 * 	Move kalloc initialization to vm_init.
 * 	[89/09/06            dbg]
 * 
 * 	Move bootstrap initialization to kern/bootstrap.c.  Initialize
 * 	device service here.
 * 	[89/08/02            dbg]
 * 
 * 	Initialize kalloc package.
 * 	[89/07/11            dbg]
 * 
 * Revision 2.2  89/08/05  16:07:22  rwd
 * 	Call mappable_time_init.
 * 	[89/08/04  18:31:35  rwd]
 * 
 * 23-May-89  David Golub (dbg) at Carnegie-Mellon University
 *	Thread creation must be done by a running thread;
 *	thread_setrun (called by thread_resume & kernel_thread) may look
 *	at current thread.
 *
 * 19-May-89  David Golub (dbg) at Carnegie-Mellon University
 *	Don't export first_task and first_thread; they don't last very
 *	long.
 *
 * 15-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Initial threads are now part of the kernel task, not first-task.
 *	Create the bootstrap task as a kernel task.
 *
 *  8-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Modified for stand-alone MACH kernel.
 *
 *  1-Jul-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */

/*
 *	Mach kernel startup.
 */


#include <xpr_debug.h>
#include <cpus.h>
#include <mach_host.h>
#include <norma_ipc.h>
#include <norma_vm.h>

#include <mach/boolean.h>
#include <mach/machine.h>
#include <mach/task_special_ports.h>
#include <mach/vm_param.h>

#include <ipc/ipc_init.h>

#include <kern/clock.h>
#include <kern/cpu_number.h>
#include <kern/kern_io.h>
#include <kern/kern_kmsg.h>
#include <kern/mach_timer.h>
#include <kern/machine.h>
#include <kern/processor.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/thread_swap.h>
#include <kern/time_out.h>
#include <kern/timer.h>
#include <kern/xpr.h>
#include <kern/zalloc.h>

#include <sched_policy/standard.h>

#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <machine/machspl.h>
#include <machine/pmap.h>
#include <sys/version.h>


/*
 *	Initialization routines in various subsystems.
 */

extern void	vm_mem_init(void);
extern void	vm_mem_bootstrap(void);

extern no_return idle_thread(void);
extern no_return vm_pageout(void);
extern no_return reaper_thread(void);
extern no_return swapin_thread(void);

extern void	bootstrap_create(void);
extern void	device_service_create(void);
extern void	idle_thread_create(processor_t);

no_return cpu_launch_first_thread(thread_t);	/* forward */
no_return start_kernel_threads(void);		/* forward */

extern no_return load_context(thread_t);

/*
 *	Running in virtual memory, on the interrupt stack.
 *	Does not return.  Dispatches initial thread.
 *
 *	Assumes that master_cpu is set.
 */
no_return setup_main(void)
{
	thread_t		startup_thread;

	/*
	 *	Allow printing and panics
	 */
	printf_init();
	panic_init();

	/*
	 *	Get VM running as soon as possible
	 */
	vm_mem_bootstrap();		/* can do 'zinit' now */

	/*
	 *	Initialize the low-level scheduling packages
	 */
	sched_init();
	ast_init();
#if	NCPUS > 1
	action_thread_init();		/* action queue */
#endif

	/*
	 *	Initialize packages that need VM and zones
	 */
	sched_policy_init();		/* set up scheduling policies */
	pset_sys_bootstrap();		/* first pset: needs scheduling
					   policies and quanta */
	ipc_bootstrap();		/* IPC system */

	/*
	 *	Initialize packages that need IPC ports
	 */
	vm_mem_init();			/* rest of VM - needs IPC */
	ipc_init();			/* rest of IPC - needs full VM */
	kernel_reply_kmsg_init();	/* kernel reply messages */

	/*
	 * As soon as the virtual memory system is up, we record
	 * that this CPU is using the kernel pmap.
	 */
	PMAP_ACTIVATE_KERNEL(master_cpu);

	/*
	 *	Set up all of the timers
	 */
	init_timers();			/* internal timers */
	mach_timer_init();		/* user-visible timers */

#if	XPR_DEBUG
	xprbootstrap();
#endif	/* XPR_DEBUG */

	/*
	 *	Call machine-dependent initialization.
	 *	This configures devices, among other things.
	 */
	machine_init();

	machine_info.max_cpus = NCPUS;
	machine_info.memory_size = mem_size;
	machine_info.avail_cpus = 0;
	machine_info.major_version = KERNEL_MAJOR_VERSION;
	machine_info.minor_version = KERNEL_MINOR_VERSION;

	/*
	 *	Initialize the task, thread, and processor set subsystems.
	 */
	task_init();
	thread_init();
	swapper_init();
#if	MACH_HOST
	pset_sys_init();
#endif	/* MACH_HOST */

	/*
	 *	Initialize the periodic scheduling calculations.
	 */
	init_sched_calculations();
	
	/*
	 *	Create a kernel thread to start the other kernel
	 *	threads.  Thread_resume (from kernel_thread) calls
	 *	thread_setrun, which may look at current thread;
	 *	we must avoid this, since there is no current thread.
	 */

	/*
	 *	Enable the fixed priority policy on the default
	 *	processor set.
	 */
	(void) processor_set_policy_add(&default_pset, POLICY_FIXEDPRI,
					0, 0);

	/*
	 *	Set fixed-priority as the default for the
	 *	kernel task.
	 */
    {
	struct policy_param_fixedpri	param;

	param.priority = FP_BASEPRI_SYSTEM;
	param.no_preempt = TRUE;

	(void) task_set_default_policy(kernel_task, &default_pset,
					POLICY_FIXEDPRI,
					(policy_param_t)&param,
					POLICY_PARAM_FIXEDPRI_COUNT,
					FALSE);
    }

	/*
	 * Create the thread, and point it at the routine.
	 */
	(void) thread_create(kernel_task, &startup_thread);
	thread_start(startup_thread, start_kernel_threads);

	/*
	 * Give it a kernel stack.
	 */
	thread_doswapin(startup_thread);

	/*
	 * Pretend it is already running, and resume it.
	 * Since it looks as if it is running, thread_resume
	 * will not try to put it on the run queues.
	 *
	 * We can do all of this without locking, because nothing
	 * else is running yet.
	 */
	startup_thread->state |= TH_RUN;
	(void) thread_resume(startup_thread);

	/*
	 * Start the thread.
	 */
	cpu_launch_first_thread(startup_thread);
	/*NOTREACHED*/
}

/*
 * Now running in a thread.  Create the rest of the kernel threads
 * and the bootstrap task.
 */
no_return start_kernel_threads(void)
{
	register int	i;

	/*
	 *	Create the idle threads and the other
	 *	service threads.
	 */
	for (i = 0; i < NCPUS; i++) {
	    if (machine_slot[i].is_cpu) {
		idle_thread_create(cpu_to_processor(i));
	    }
	}

	(void) kernel_thread(kernel_task, reaper_thread, (char *) 0);
	(void) kernel_thread(kernel_task, swapin_thread, (char *) 0);

#if	NCPUS > 1
	/*
	 *	Create the shutdown thread.
	 */
	(void) kernel_thread(kernel_task, action_thread, (char *) 0);

	/*
	 *	Allow other CPUs to run.
	 */
	start_other_cpus();
#endif	/* NCPUS > 1 */

	/*
	 *	Create the device service.
	 */
	device_service_create();

	/*
	 *	Initialize NORMA ipc system.
	 */
#if	NORMA_IPC
	norma_ipc_init();
#endif	/* NORMA_IPC */

	/*
	 *	Initialize NORMA vm system.
	 */
#if	NORMA_VM
	norma_vm_init();
#endif	/* NORMA_VM */

	/*
	 *	Start the user bootstrap.
	 */
	bootstrap_create();

#if	XPR_DEBUG
	xprinit();		/* XXX */
#endif	/* XPR_DEBUG */

	/*
	 *	Become the pageout daemon.
	 */
	(void) spl0();
	vm_pageout();
	/*NOTREACHED*/
}

#if	NCPUS > 1

/*
 *	Do system initialization for other processors.
 */
no_return slave_main(void)
{
	/*
	 *	Do machine_specific initialization.
	 */
	slave_machine_init();

	/*
	 *	Run the processor`s idle thread first.
	 */
	cpu_launch_first_thread(current_processor()->idle_thread);
}
#endif	/* NCPUS > 1 */

/*
 *	Start up the first thread on a CPU.
 *	First thread is supplied.
 */
no_return cpu_launch_first_thread(
	register thread_t	th)
{
	register int	mycpu;

	mycpu = cpu_number();

	cpu_up(mycpu);

	start_timer(&kernel_timer[mycpu]);

	(void) splhigh();

	enable_clock_interrupts();	/* needs an active thread */
	PMAP_ACTIVATE_KERNEL(mycpu);

	active_threads[mycpu] = th;
	active_stacks[mycpu] = th->kernel_stack;
	thread_sched_lock(th);
	th->state &= ~TH_UNINT;
	thread_sched_unlock(th);
	timer_switch(&th->system_timer);

	PMAP_ACTIVATE_USER(vm_map_pmap(th->task->map), th, mycpu);

	load_context(th);
	/*NOTREACHED*/
}
