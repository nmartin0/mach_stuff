/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989 Carnegie Mellon University
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
 * 17-Mar-93  Randall Dean (rwd) at Carnegie-Mellon University
 *	Remove statistics_struct and status_struct.
 *
 * 08-Jan-93  Randall Dean (rwd) at Carnegie-Mellon University
 *	Merge with cproc.c ridding the world of a layer of
 *	complexity.
 *
 * $Log:	cthreads.c,v $
 */
/*
 * 	File:	cthreads.c
 *	Author:	Eric Cooper, Carnegie Mellon University
 *	Date:	July, 1987
 *
 * 	Implementation of fork, join, exit, etc.
 */

#include <cthreads.h>
#include <cthread_filter.h>
#include "cthread_internals.h"
#include <mach/mach_traps.h>
#include <mach/mach_host.h>

/*
 * Define internal lock macros.  These can be disabled when
 * kernel_threads == 1. 
 */

#define cthread_try_lock(p) (spin_lock_locked(p)?0:spin_try_lock((p)))
#define cthread_lock(p) do {if (!cthread_try_lock(p)) spin_lock_solid(p);}while (0)
#define cthread_unlock(p) spin_unlock(p)

/*
 * #define cthread_try_lock(p) 1
 * #define cthread_lock(p)
 * #define cthread_unlock(p)
 */

#define stats_lock(p)
#define stats_unlock(p)

/*
#define stats_lock(p) do {if (!spin_try_lock(p)) spin_lock_solid(p);}while (0)
#define stats_unlock(p) spin_unlock(p)
*/

/*
 * Thread status bits.
 */
#define	T_MAIN		0x1
#define	T_RETURNED	0x2
#define	T_DETACHED	0x4

extern void *malloc (unsigned int);
extern void free(void *);
extern void exit(int ret);
extern void alloc_stack(cthread_t);
extern vm_offset_t cthread_stack_base(cthread_t, int);/* return start of stack */
extern vm_offset_t stack_init(cthread_t);
extern void mig_init(cthread_t);
#ifdef STATISTICS
extern struct cthread_statistics_struct cthread_stats;
#endif

/*
 * Allocate space for cthread strucure and initialize.
 * Ready_lock is held when called
 */

private cthread_t cthread_alloc()
{
    register cthread_t p = (cthread_t) malloc(sizeof(struct cthread));
    
    p->reply_port = MACH_PORT_NULL;
    
    p->wired = MACH_PORT_NULL;
    p->explicitly_wired = FALSE;
    p->state = CTHREAD_RUNNING;
    p->busy = 0;
    p->status = 0;
    p->joiner = NO_CTHREAD;
    p->list = cthread_status.cthread_list;
    cthread_status.cthread_list = p;
    cthread_status.alloc_cthreads++;
#ifdef WAIT_DEBUG
    p->waiting_for = 0;
#endif WAIT_DEBUG
    p->cthread_status = &cthread_status;

    return p;
}

private cthread_t cthread_create(cthread_fn_t func, void *arg)
{
    register cthread_t child;
    register kern_return_t r;
    extern void cthread_setup(cthread_t, thread_t, cthread_fn_t);
    extern void cthread_body(cthread_t);
    thread_t n;
    
    child = cthread_alloc();
    child->func = func;
    child->arg = arg;
    cthread_unlock(&cthread_status.run_lock);
    alloc_stack(child);
    cthread_lock(&cthread_status.n_kern_lock);
    if (cthread_status.max_kernel_threads == 0 ||
	cthread_status.kernel_threads < cthread_status.max_kernel_threads) {
	cthread_status.kernel_threads++;
	cthread_unlock(&cthread_status.n_kern_lock);
	MACH_CALL(thread_create(mach_task_self(), &n), r);
	cthread_setup(child, n, (cthread_fn_t)cthread_body);
	MACH_CALL(thread_resume(n), r);
    } else {
	register cthread_t waiter = NO_CTHREAD;
	register mach_port_t waiter_thread_port;
	cthread_unlock(&cthread_status.n_kern_lock);
	child->state = CTHREAD_BLOCKED|CTHREAD_RUNNABLE;
	child->context = cthread_stack_base(child, CTHREAD_STACK_OFFSET);
	CTHREAD_FILTER(child, CTHREAD_FILTER_PREPARE,
		      cthread_body, child, 0, 0);
	cthread_lock(&cthread_status.run_lock);
	cthread_queue_enq(&cthread_status.run_queue, child);
	waiter = (cthread_t)cthread_dlq_first(&cthread_status.waiting_dlq);
	cthread_unlock(&cthread_status.run_lock);
	if (waiter != NO_CTHREAD && 
	    ((waiter_thread_port = waiter->wired) != MACH_PORT_NULL))
	    MACH_CALL(thread_depress_abort(waiter_thread_port),r);
    }
    return child;
}

/*
 * This is only run on a partial "waiting" stack
 */

private void cthread_idle(cthread_t p)
{
    register cthread_t new = NO_CTHREAD;
    register count;
    kern_return_t r;
    
#ifdef STATISTICS
    stats_lock(&cthread_stats.lock);
    cthread_stats.waiters++;
    stats_unlock(&cthread_stats.lock);
#endif STATISTICS
    
    do {
	count = 0;
#ifdef STATISTICS
	stats_lock(&cthread_stats.lock);
	cthread_stats.idle_swtchs++;
	stats_unlock(&cthread_stats.lock);
#endif STATISTICS
	MACH_CALL(thread_switch(THREAD_NULL, SWITCH_OPTION_DEPRESS, 0),r);
#ifdef BUSY_SPINNING
	while((cthread_queue_head(&cthread_status.run_queue, cthread_t) == NO_CTHREAD) &&
	      (count++<cthread_status.waiter_spin_count));
#endif BUSY_SPINNING
	if (cthread_queue_head(&cthread_status.run_queue, cthread_t)
	    != NO_CTHREAD) {
#ifdef SPIN_POLL_IDLE
	    do {
		if (cthread_try_lock(&cthread_status.run_lock)) {
		    cthread_queue_deq(&cthread_status.run_queue, cthread_t, new);
		    if (!new)
			cthread_unlock(&cthread_status.run_lock);
		}
	    } while  (!new && count++<cthread_status.lock_spin_count &&
		      cthread_queue_head(&cthread_status.run_queue, cthread_t)
		      != NO_CTHREAD);
#ifdef STATISTICS
	    if (!new && count<cthread_status.lock_spin_count) {
		stats_lock(&cthread_stats.lock);
		cthread_stats.idle_lost++;
		stats_unlock(&cthread_stats.lock);
	    }
#endif STATISTICS
#else SPIN_POLL_IDLE
	    cthread_lock(&cthread_status.run_lock);
	    cthread_queue_deq(&cthread_status.run_queue, cthread_t, new);
	    if (!new)
		cthread_unlock(&cthread_status.run_lock);
#endif SPIN_POLL_IDLE
	}
    } while (!new);
#ifdef STATISTICS
    cthread_stats.idle_exit++;
#endif STATISTICS
    cthread_dlq_remove(&cthread_status.waiting_dlq, p, cthread_t, dlq);
    p->wired = MACH_PORT_NULL;
    cthread_queue_enq(&cthread_status.waiters, p);
    p->state = CTHREAD_BLOCKED|CTHREAD_WAITER;
    new->state = CTHREAD_RUNNING;
    CTHREAD_FILTER(p, CTHREAD_FILTER_INTERNAL_BUILD, cthread_idle, p, (void *)&cthread_status.run_lock, &new->context);
}

/*
 * Current cthread is blocked so switch to any ready cthreads, or, if
 * none, go into the wait state. Ready_lock held when called.
 */

private void cthread_block(p, f, a)
    register cthread_t p;
    register void (*f)();
    register void *a;
{
    register cthread_t waiter, new;
    static const char *wait_name = "Waiter";
    
#ifdef STATISTICS
    cthread_stats.blocks++;
#endif STATISTICS
    p->state |= CTHREAD_BLOCKED;
    p->state &= ~CTHREAD_RUNNING;
    if (p->wired != MACH_PORT_NULL) {
	register kern_return_t r;

	cthread_unlock(&cthread_status.run_lock);
	while (p->state & CTHREAD_BLOCKED)
	    MACH_CALL(thread_switch(THREAD_NULL, SWITCH_OPTION_DEPRESS, 0),r);
	f(a);
    } else {
	cthread_queue_deq(&cthread_status.run_queue, cthread_t, new);
	if (new) {
	    new->state = CTHREAD_RUNNING;
	    CTHREAD_FILTER(p, CTHREAD_FILTER_INTERNAL_BUILD, f, a, (void *)&cthread_status.run_lock, &new->context);
	    return; /* never reached */
	} else {
	    cthread_queue_deq(&cthread_status.waiters, cthread_t, waiter);
	    if (waiter == NO_CTHREAD) {
		vm_address_t base;
		kern_return_t r;
#ifdef STATISTICS
		cthread_stats.wait_stacks++;
#endif STATISTICS
		waiter = cthread_alloc();
		waiter->explicitly_wired = TRUE;
		MACH_CALL(vm_allocate(mach_task_self(), &base,
			      cthread_status.wait_stack_size, TRUE), r);
		waiter->stack_base = base;
		waiter->stack_size = cthread_status.wait_stack_size;
		waiter->context = 
		    cthread_stack_base(waiter, CTHREAD_STACK_OFFSET);
		waiter->name = wait_name;
		CTHREAD_FILTER(waiter, CTHREAD_FILTER_PREPARE,
			       cthread_idle, waiter, 0, 0); 
	    }
	    waiter->wired = mach_thread_self();
	    waiter->state = CTHREAD_RUNNING|CTHREAD_WAITER;
	    cthread_dlq_enter(&cthread_status.waiting_dlq, waiter,cthread_t,dlq);
	    CTHREAD_FILTER(p, CTHREAD_FILTER_INTERNAL_BUILD, f, a,
			   (void *)&cthread_status.run_lock,
			   &waiter->context);
	    return; /* never reached */
	}
    }
}

private int cthread_cpus()
{
    host_t host;
    processor_set_t default_pset;
    unsigned int count;
    struct processor_set_basic_info info;

    count = PROCESSOR_SET_BASIC_INFO_COUNT;
    host = mach_host_self();
    processor_set_default(host, &default_pset);
    processor_set_info(default_pset, PROCESSOR_SET_BASIC_INFO,
		       &host, (host_info_t)&info, &count);
    return info.processor_count;
}

int cthread_init()
{
    static int cthreads_started = FALSE;
    register cthread_t p;
    int stack;

    if (cthreads_started)
	return 0;

    p = cthread_alloc();
    cthread_status.cthread_list = p;
    cthread_status.cthread_cthreads = 1;
    cthread_status.alloc_cthreads = 1;
    cthread_status.kernel_threads = 1;
    cthread_dlq_init(&cthread_status.waiting_dlq);
    cthread_status.processors = cthread_cpus();
    if (cthread_status.processors > 1) {
#if CTHREAD_WAITER_SPIN_COUNT
	if (!cthread_status.waiter_spin_count)
	    cthread_status.waiter_spin_count = CTHREAD_WAITER_SPIN_COUNT;
#endif
#if MUTEX_SPIN_COUNT
	if (!cthread_status.mutex_spin_count)
	    cthread_status.mutex_spin_count = MUTEX_SPIN_COUNT;
#endif
#if LOCK_SPIN_COUNT
	if (!cthread_status.lock_spin_count)
	    cthread_status.lock_spin_count = LOCK_SPIN_COUNT;
#endif
    }
    
    p->status |= T_MAIN;
    cthread_set_name((cthread_t)p, "main");
    
    cthreads_started = TRUE;
    
    /*
     * We pass back the new stack which should be switched to
     * by crt0.  This guarantess correct size and alignment.
     */
    stack = (int)stack_init(p);

    mig_init(p);		/* enable multi-threaded mig interfaces */
    
    return stack;
}

/*
 * Used for automatic initialization by crt0.
 * Cast needed since too many C compilers choke on the type void (*)().
 */
int (*_cthread_init_routine)() = (int (*)()) cthread_init;

/*
 * Procedure invoked at the base of each cthread.
 */
void cthread_body(cthread_t self)
{
    if (self->func != (cthread_fn_t)0) {
	cthread_fn_t func = self->func;
	if (cthread_status.max_kernel_threads == 0 && !self->wired)
	    self->wired = mach_thread_self();
	self->func = (cthread_fn_t)0;
	self->result = (*func)(self->arg);
    }
    cthread_lock(&cthread_status.run_lock);
    cthread_status.cthread_cthreads--;
    self->wired = MACH_PORT_NULL;
    self->explicitly_wired = FALSE;
    if (self->status & T_DETACHED) {
	cthread_queue_enq(&cthread_status.cthreads, self);
    } else {
	self->status |= T_RETURNED;
	if (self->joiner) {
	    cthread_t joiner = self->joiner;
	    self->joiner = NO_CTHREAD;
	    if (joiner->wired != MACH_PORT_NULL)
		joiner->state = CTHREAD_RUNNING;
	    else {
		cthread_queue_enq(&cthread_status.run_queue, joiner);
		joiner->state |= CTHREAD_RUNNABLE;
	    }
	}
    }
    cthread_block(self, cthread_body, self);
    return; /* never reached */
}

cthread_t cthread_fork(cthread_fn_t func, void *arg)
{
    register cthread_t p;
    mach_port_t waiter_thread_port;
    kern_return_t r;
    
    cthread_lock(&cthread_status.run_lock);
    cthread_status.cthread_cthreads++;
    cthread_queue_deq(&cthread_status.cthreads, cthread_t, p);
    if (p) {
	cthread_t waiter;
	p->func = func;
	p->arg = arg;
	p->status = 0;
	cthread_queue_enq(&cthread_status.run_queue, p);
	p->state |= CTHREAD_RUNNABLE;
	waiter = (cthread_t)cthread_dlq_first(&cthread_status.waiting_dlq);
	cthread_unlock(&cthread_status.run_lock);
	if (waiter != NO_CTHREAD && 
	    ((waiter_thread_port = waiter->wired) != MACH_PORT_NULL))
	    MACH_CALL(thread_depress_abort(waiter_thread_port),r);
    } else {
	p = cthread_create(func, arg);
    }
    return (cthread_t)p;
}

void cthread_detach(cthread_t t)
{
    register cthread_t p = (cthread_t)t;

    cthread_lock(&cthread_status.run_lock);
    if (p->status & T_RETURNED) {
	cthread_queue_enq(&cthread_status.cthreads, p);
    } else {
	p->status |= T_DETACHED;
    }
    cthread_unlock(&cthread_status.run_lock);
}

void cthread_join_real(cthread_t t)
{
    void *result;
    register cthread_t p = (cthread_t)t;
    register cthread_t self = _cthread_self();

    cthread_lock(&cthread_status.run_lock);
    if (! (p->status & T_RETURNED)) {
	p->joiner = self;
	cthread_block(self, cthread_join_real, p);
	return; /* never reached */
    }
    result = p->result;
    cthread_queue_enq(&cthread_status.cthreads, p);
    cthread_unlock(&cthread_status.run_lock);
    CTHREAD_FILTER(self, CTHREAD_FILTER_USER_INVOKE, result, 0, 0, 0);
}

void cthread_exit(void *result)/*XXX Fix T_MAIN case XXX*/
{
    register cthread_t p = _cthread_self();

    cthread_lock(&cthread_status.run_lock);
    if (p->status & T_MAIN) {
	while (cthread_status.cthread_cthreads > 1) {
	    if (p->wired != MACH_PORT_NULL) {
		kern_return_t r;
		MACH_CALL(thread_switch(THREAD_NULL, SWITCH_OPTION_NONE, 0),r);
	    } else {
		cthread_queue_enq(&cthread_status.run_queue,p);
		p->state |= CTHREAD_RUNNABLE;
		cthread_block(p, cthread_body, p);
	    }
	}
	exit((int) result);
    } else {
	p->result = result;
	/* rewind stack */
	p->context = cthread_stack_base(p, CTHREAD_STACK_OFFSET);
	p->wired = MACH_PORT_NULL;
	p->explicitly_wired = FALSE;
	cthread_queue_enq(&cthread_status.run_queue, p);
	p->state |= CTHREAD_RUNNABLE;
	cthread_block(p, cthread_body, p);
    }
}

/*
 * Used for automatic finalization by crt0.  Cast needed since too many C
 * compilers choke on the type void (*)().
 */
int (*_cthread_exit_routine)() = (int (*)()) cthread_exit;

void cthread_set_name(cthread_t t, const char *name)
{
    ((cthread_t)t)->name = name;
}

const char *cthread_name(cthread_t t)
{
    cthread_t p = (cthread_t)t;

    return (p == NO_CTHREAD
	    ? "idle"
	    : (p->name == 0 ? "?" : p->name));
}

/*
 * Return current value for max kernel threads
 * Note: 0 means no limit
 */

int cthread_kernel_limit()
{
    int ret;
    cthread_lock(&cthread_status.run_lock);
    ret = cthread_status.max_kernel_threads;
    cthread_unlock(&cthread_status.run_lock);
    return ret;
}

/*
 * Set max number of kernel threads
 * Note:	This will not currently terminate existing threads
 * 		over maximum.
 */

void cthread_set_kernel_limit(int n)
{
    cthread_lock(&cthread_status.run_lock);
    if (n == 0 && cthread_status.max_kernel_threads) {
#if 0
	/* Wire all threads */
	/* This is hard.  Should it be done? */
#endif 0
    } else if (n && cthread_status.max_kernel_threads == 0) {
	cthread_t p;
	for(p=cthread_status.cthread_list;p!=NO_CTHREAD;p=p->list)
	    if (!p->explicitly_wired)
		p->wired = MACH_PORT_NULL;
    }
    cthread_status.max_kernel_threads = n;
    cthread_unlock(&cthread_status.run_lock);
}

int cthread_limit()
{
    return cthread_kernel_limit();
}

void cthread_set_limit(int n)
{
    cthread_set_kernel_limit(n);
}

int cthread_count()
{
    return cthread_status.cthread_cthreads;
}

void cthread_fork_prepare()
{
    extern void malloc_fork_prepare();
    register cthread_t p = _cthread_self();

    malloc_fork_prepare();
    
    vm_inherit(mach_task_self(),p->stack_base, p->stack_size, VM_INHERIT_COPY);
    cthread_lock(&cthread_status.n_kern_lock);
    cthread_lock(&cthread_status.port_lock);
    cthread_lock(&cthread_status.run_lock);
}

void cthread_fork_parent()
{
    extern void malloc_fork_parent();
    register cthread_t p = _cthread_self();
    
    malloc_fork_parent();

    cthread_unlock(&cthread_status.run_lock);
    cthread_unlock(&cthread_status.port_lock);
    cthread_unlock(&cthread_status.n_kern_lock);
    vm_inherit(mach_task_self(),p->stack_base, p->stack_size, VM_INHERIT_NONE);
}

void cthread_fork_child()
{
    cthread_t p, l, m;
    extern void malloc_fork_child(),stack_fork_child();
    register port_entry_t pe;
    port_entry_t pet;

    malloc_fork_child();

    stack_fork_child();

    p = _cthread_self();

    cthread_status.cthread_cthreads = 1;
    cthread_status.alloc_cthreads = 1;
    p->status |= T_MAIN;
    cthread_set_name(p, "main");

    vm_inherit(mach_task_self(),p->stack_base, p->stack_size, VM_INHERIT_NONE);

    spin_lock_init(&cthread_status.n_kern_lock);
    spin_lock_init(&cthread_status.run_lock);
    spin_lock_init(&cthread_status.port_lock);

    cthread_status.kernel_threads=0;

    for(l=cthread_status.cthread_list;l!=NO_CTHREAD;l=m) {
	m=l->next;
	if (l!=p)
	    free((void *)l);
    }
    
    cthread_status.cthread_list = p;
    p->next = NO_CTHREAD;
    
    cthread_queue_init(&cthread_status.waiters);
    cthread_queue_init(&cthread_status.cthreads);
    cthread_queue_init(&cthread_status.run_queue);
    cthread_dlq_init(&cthread_status.waiting_dlq);

    for(pe=cthread_status.port_list;pe!=PORT_ENTRY_NULL;pe=pet) {
	pet = pe->next;
	free((void *)pe);
    }

    cthread_status.port_list = PORT_ENTRY_NULL;
    
    mig_init(p);		/* enable multi-threaded mig interfaces */

#ifdef STATISTICS
    cthread_stats.mutex_block = 0;
    cthread_stats.mutex_miss=0;
    cthread_stats.waiters=0;
    cthread_stats.unlock_new=0;
    cthread_stats.blocks=0;
    cthread_stats.notrigger=0;
    cthread_stats.wired=0;
    cthread_stats.wait_stacks = 0;
    cthread_stats.rnone = 0;
    cthread_stats.spin_count = 0;
    cthread_stats.mutex_caught_spin = 0;
    cthread_stats.idle_exit = 0;
    cthread_stats.mutex_cwl = 0;
    cthread_stats.idle_swtchs = 0;
    cthread_stats.idle_lost = 0;
    cthread_stats.umutex_enter = 0;
    cthread_stats.umutex_noheld = 0;
#endif STATISTICS
    
}

mutex_t mutex_alloc() {
    return ((mutex_t) malloc(sizeof(struct mutex)));
}

void mutex_init(mutex_t m)
{
   cthread_queue_init(&(m)->queue);
   spin_lock_init(&(m)->held);
   (m)->trigger=0;
   (m)->name=(char *)0;
}

void mutex_set_name(mutex_t m, const char *x)
{
    (m)->name = (x);
}

const char *mutex_name(mutex_t m)
{
    return ((m)->name != 0 ? (m)->name : "?");
}

void mutex_clear(mutex_t m)
{
    free((void *) (m));
}

void mutex_free(mutex_t m)
{
    mutex_init(m);
}

condition_t condition_alloc()
{
    return ((condition_t) malloc(sizeof(struct condition)));
}

void condition_init(condition_t c)
{
    cthread_queue_init(&(c)->queue);
}

void condition_set_name(condition_t c, const char *x)
{
	(c)->name = (x);
}

const char *condition_name(condition_t c)
{
    return ((c)->name != 0 ? (c)->name : "?");
}

void condition_clear(condition_t c)
{
    condition_broadcast(c);
}

void condition_free(condition_t c)
{
    condition_clear(c);
    free((void *) (c));
}

void *cthread_join(cthread_t t)
{
    CTHREAD_FILTER(_cthread_self(), CTHREAD_FILTER_USER_BUILD,
		   cthread_join_real, t, 0, 0);
    return (void *)0; /* never reached */
}

void mutex_lock_real(mutex_t m)
{
    register int count = cthread_status.mutex_spin_count;
    register cthread_t p = _cthread_self();
    kern_return_t r;
    
#ifdef STATISTICS
    cthread_stats.mutex_miss++;
#endif STATISTICS
#ifdef BUSY_SPINNING
    while (count>=0) {
#endif BUSY_SPINNING
	if (spin_try_lock(&m->held)) {
#ifdef STATISTICS
	    stats_lock(&cthread_stats.lock);
	    cthread_stats.mutex_caught_spin++;
	    stats_unlock(&cthread_stats.lock);
#endif STATISTICS
	    CTHREAD_FILTER(p, CTHREAD_FILTER_USER_INVOKE,0, 0, 0, 0);
	    return; /* never reached */
	}
#ifdef BUSY_SPINNING
	count--;
    }
#endif BUSY_SPINNING
    count = 0;
#ifdef SPIN_POLL_MUTEX
    while(!cthread_try_lock(&cthread_status.run_lock)) {
	if (spin_try_lock(&m->held)) {
#ifdef STATISTICS
	    stats_lock(&cthread_stats.lock);
	    cthread_stats.mutex_cwl++;
	    stats_unlock(&cthread_stats.lock);
#endif STATISTICS
	    CTHREAD_FILTER(p, CTHREAD_FILTER_USER_INVOKE,0, 0, 0, 0);
	    return; /* never reached */
	} else if (count++>cthread_status.lock_spin_count) {
	    count = 0;
	    MACH_CALL(thread_switch(THREAD_NULL, SWITCH_OPTION_DEPRESS, 10),r);
	}
    }
#else SPIN_POLL_MUTEX
    cthread_lock(&cthread_status.run_lock);
#endif SPIN_POLL_MUTEX
    m->trigger = TRUE;
    if (spin_try_lock(&m->held)) {
	m->trigger = (cthread_queue_head(&m->queue, cthread_t) != NO_CTHREAD);
	cthread_unlock(&cthread_status.run_lock);
	CTHREAD_FILTER(p, CTHREAD_FILTER_USER_INVOKE, 0, 0, 0, 0);
	return; /* never reached */
    } else {
	cthread_queue_enq(&m->queue, p);
    }
#ifdef STATISTICS
    cthread_stats.mutex_block++;
#endif STATISTICS
#ifdef WAIT_DEBUG
    p->waiting_for = (int)m;
#endif WAIT_DEBUG
    cthread_block(p, mutex_lock_real, m);
    return; /* never reached */
}

void mutex_lock_solid(mutex_t m) {
    CTHREAD_FILTER(_cthread_self(), CTHREAD_FILTER_USER_BUILD,
		   mutex_lock_real, (m), 0, 0);
}

void mutex_unlock_solid(mutex_t m)
{
    register cthread_t new, waiter = NO_CTHREAD;
    register mach_port_t waiter_thread_port;
    kern_return_t r;
    
#ifdef STATISTICS
    cthread_stats.umutex_enter++;
#endif STATISTICS
    if (!spin_try_lock(&m->held)) {
#ifdef STATISTICS
	cthread_stats.umutex_noheld++;
#endif STATISTICS
	return;
    }
    cthread_lock(&cthread_status.run_lock);
    if (m->trigger) {
	cthread_queue_deq(&m->queue, cthread_t, new);
    } else {
#ifdef STATISTICS
	cthread_stats.notrigger++;
#endif STATISTICS
	new = NO_CTHREAD;
    }
    spin_unlock(&m->held);
    if (new) {
#ifdef STATISTICS
	cthread_stats.unlock_new++;
#endif STATISTICS
	m->trigger = (cthread_queue_head(&m->queue, cthread_t) != NO_CTHREAD);
	if (new->wired != MACH_PORT_NULL) {
	    new->state = CTHREAD_RUNNING;
	    waiter = new;
	} else {
	    cthread_queue_enq(&cthread_status.run_queue, new);
	    new->state |= CTHREAD_RUNNABLE;
	    waiter = (cthread_t)cthread_dlq_first(&cthread_status.waiting_dlq);
	}
    }
    cthread_unlock(&cthread_status.run_lock);
    if (waiter != NO_CTHREAD && 
	((waiter_thread_port = waiter->wired) != MACH_PORT_NULL))
	MACH_CALL(thread_depress_abort(waiter_thread_port),r);
}

void condition_signal(condition_t c)
{
    register cthread_t p, waiter = NO_CTHREAD;
    register mach_port_t waiter_thread_port;
    kern_return_t r;
    
    cthread_lock(&cthread_status.run_lock);
    cthread_queue_deq(&c->queue, cthread_t, p);
    if (p) {
	register mutex_t m = p->cond_mutex;
	m->trigger = TRUE;
	if (m->held) {
#ifdef WAIT_DEBUG
	    p->waiting_for = (int)m;
#endif WAIT_DEBUG
	    cthread_queue_enq(&m->queue, p);
	} else {
	    m->trigger = (cthread_queue_head(&m->queue, cthread_t) != NO_CTHREAD);
	    if (p->wired != MACH_PORT_NULL) {
		p->state = CTHREAD_RUNNING;
		waiter = p;
	    } else {
		cthread_queue_enq(&cthread_status.run_queue, p);
		p->state |= CTHREAD_RUNNABLE;
		waiter = (cthread_t)cthread_dlq_first(&cthread_status.waiting_dlq);
	    }
	}
    }
    cthread_unlock(&cthread_status.run_lock);
    if (waiter != NO_CTHREAD && 
	((waiter_thread_port = waiter->wired) != MACH_PORT_NULL))
	MACH_CALL(thread_depress_abort(waiter_thread_port),r);
}

void condition_broadcast(condition_t c)
{
    register cthread_t p, waiter = NO_CTHREAD;
    register mach_port_t waiter_thread_port;
    kern_return_t r;
    
    cthread_lock(&cthread_status.run_lock);
    while (cthread_queue_head(&c->queue, cthread_t) != NO_CTHREAD) {
	register mutex_t m;
	cthread_queue_deq(&c->queue, cthread_t, p);
	m = p->cond_mutex;
	m->trigger = TRUE;
	if (m->held) {
#ifdef WAIT_DEBUG
	    p->waiting_for = (int)m;
#endif WAIT_DEBUG
	    cthread_queue_enq(&m->queue, p);
	} else {
	    m->trigger = (cthread_queue_head(&m->queue, cthread_t) != NO_CTHREAD);
	    if (p->wired != MACH_PORT_NULL) {
		p->state = CTHREAD_RUNNING;
		waiter = p;
	    } else {
		cthread_queue_enq(&cthread_status.run_queue, p);
		p->state |= CTHREAD_RUNNABLE;
		waiter = (cthread_t)cthread_dlq_first(&cthread_status.waiting_dlq);
	    }
	}
    }
    cthread_unlock(&cthread_status.run_lock);
    if (waiter != NO_CTHREAD && 
	((waiter_thread_port = waiter->wired) != MACH_PORT_NULL))
	MACH_CALL(thread_depress_abort(waiter_thread_port),r);
}

void cond_wait(condition_t c, mutex_t m)
{
    register cthread_t p = _cthread_self();
    register cthread_t new;
    
    cthread_lock(&cthread_status.run_lock);
    cthread_queue_enq(&c->queue, p);
    p->cond_mutex = m;
    
    cthread_queue_deq(&m->queue, cthread_t, new);
    spin_unlock(&m->held);
    
    if (new) {
	m->trigger = (cthread_queue_head(&m->queue, cthread_t) != NO_CTHREAD);
	if (new->wired != MACH_PORT_NULL) {
	    new->state = CTHREAD_RUNNING;
	} else {
	    cthread_queue_enq(&cthread_status.run_queue, new);
	    new->state |= CTHREAD_RUNNABLE;
	}
    }
#ifdef WAIT_DEBUG
    p->waiting_for = (int)c;
#endif WAIT_DEBUG
    cthread_block(p, mutex_lock_real, m);
}

void condition_wait(condition_t c, mutex_t m) {
    CTHREAD_FILTER(_cthread_self(), CTHREAD_FILTER_USER_BUILD,
		   cond_wait, (c), (m), 0);
}

void cthread_return(cthread_t p)
{
    CTHREAD_FILTER(p, CTHREAD_FILTER_USER_INVOKE, 0, 0, 0, 0);
}

void cthread_yield_real()
{
    register cthread_t new, p = _cthread_self();
    kern_return_t r;
    
    if (p->wired != MACH_PORT_NULL) {
	MACH_CALL(thread_switch(THREAD_NULL, SWITCH_OPTION_NONE, 0),r);
	CTHREAD_FILTER(p, CTHREAD_FILTER_USER_INVOKE, 0, 0, 0, 0);
    }
    cthread_lock(&cthread_status.run_lock);
    cthread_queue_deq(&cthread_status.run_queue, cthread_t, new);
    if (new) {
	cthread_queue_enq(&cthread_status.run_queue, p);
	p->state = CTHREAD_BLOCKED|CTHREAD_RUNNABLE;
	new->state = CTHREAD_RUNNING;
	CTHREAD_FILTER(p, CTHREAD_FILTER_INTERNAL_BUILD, cthread_return,
		       p, (void *)&cthread_status.run_lock, &new->context);
    } else {
	cthread_unlock(&cthread_status.run_lock);
	MACH_CALL(thread_switch(THREAD_NULL, SWITCH_OPTION_NONE, 0),r);
    }
}

void cthread_yield() {
    CTHREAD_FILTER(_cthread_self(), CTHREAD_FILTER_USER_BUILD,
		   cthread_yield_real, 0, 0, 0);
}

void cthread_set_data(cthread_t t, void *x) {
    ((cthread_t)t)->data = x;
}

void *cthread_data(cthread_t t) {
    return (((cthread_t)t)->data);
}

/*
 * Use instead of mach_msg in a multi-threaded server so as not
 * to tie up excessive kernel threads.  This uses a simple linked list for
 * ports since this should never be more than a few.
 */

/*
 * A cthread holds a reference to a port_entry even after it receives a
 * message.  This reference is not released until the thread does a
 * cthread_msg_busy.  This allows the fast case of a single mach_msg
 * call to occur as often as is possible.
 */

private port_entry_t get_port_entry(mach_port_t port, int min, int max)
{
    register port_entry_t i;
    
    cthread_lock(&cthread_status.port_lock);
    for(i=cthread_status.port_list;i!=PORT_ENTRY_NULL;i=i->next)
	if (i->port == port) {
	    cthread_unlock(&cthread_status.port_lock);
	    return i;
	}
    i = (port_entry_t)malloc(sizeof(struct port_entry));
    cthread_queue_init(&i->queue);
    i->port = port;
    i->next = cthread_status.port_list;
    cthread_status.port_list = i;
    i->min = min;
    i->max = max;
    i->held = 0;
    cthread_unlock(&cthread_status.port_lock);
    return i;
}

void cthread_msg_busy(mach_port_t port, int min, int max)
{
    register port_entry_t port_entry;
    register cthread_t p = _cthread_self(), waiter = NO_CTHREAD;
    cthread_t new;
    register mach_port_t waiter_thread_port;
    kern_return_t r;
    
    if (p->busy) {
	port_entry = get_port_entry(port, min, max);
	cthread_lock(&port_entry->lock);
	p->busy = 0;
	if (port_entry->held <= port_entry->min) {
	    cthread_queue_deq(&port_entry->queue, cthread_t, new);
	    if (new != NO_CTHREAD){
		cthread_unlock(&port_entry->lock);
		cthread_lock(&cthread_status.run_lock);
		cthread_queue_enq(&cthread_status.run_queue, new);
		new->state |= CTHREAD_RUNNABLE;
		waiter = (cthread_t)cthread_dlq_first(&cthread_status.waiting_dlq);
		new->busy = (int)port_entry;
		cthread_unlock(&cthread_status.run_lock);
	    } else {
		port_entry->held--;
		cthread_unlock(&port_entry->lock);
#ifdef STATISTICS
		stats_lock(&cthread_status.port_lock);
		cthread_stats.rnone++;
		stats_unlock(&cthread_status.port_lock);
#endif STATISTICS
	    }
	} else {
	    port_entry->held--;
	    cthread_unlock(&port_entry->lock);
	}
    }
    if (waiter != NO_CTHREAD && 
	((waiter_thread_port = waiter->wired) != MACH_PORT_NULL))
	MACH_CALL(thread_depress_abort(waiter_thread_port),r);
}

void cthread_msg_active(mach_port_t port, int min, int max)
{
    register cthread_t p = _cthread_self();
    register port_entry_t port_entry;
    
    if (!p->busy) {
	port_entry = get_port_entry(port, min, max);
	if (port_entry == 0) return;
	cthread_lock(&port_entry->lock);
	if (port_entry->held < port_entry->max) {
	    port_entry->held++;
	    p->busy = (int)port_entry;
	}
	cthread_unlock(&port_entry->lock);
    }
}

void cthread_mach_msg_real(struct cthread_mach_msg_struct *args)
{
    register port_entry_t port_entry;
    register cthread_t p = _cthread_self();
    mach_msg_return_t r;
    port_entry_t op = (port_entry_t)p->busy;
    
    port_entry = get_port_entry(args->rcv_name, args->min, args->max);
    
    if (op && (port_entry_t)op != port_entry)
	cthread_msg_busy(op->port, op->min, op->max);
    cthread_lock(&port_entry->lock);
    if (!(port_entry == (port_entry_t)p->busy)) {
	if (port_entry->held >= args->max) {
	    if (args->option & MACH_SEND_MSG) {
		cthread_unlock(&port_entry->lock);
		r = mach_msg(args->header, args->option &~ MACH_RCV_MSG,
			     args->send_size, 0, MACH_PORT_NULL,
			     args->timeout, args->notify);
		if (r != MACH_MSG_SUCCESS) {
		    CTHREAD_FILTER(p, CTHREAD_FILTER_USER_INVOKE, r, 0, 0,0);
		    return; /*never reached*/
		}
		args->option &= ~MACH_SEND_MSG;
		args->send_size = 0;
		cthread_lock(&port_entry->lock);
	    }
	    if (port_entry->held >= args->max) {
		cthread_lock(&cthread_status.run_lock);
		cthread_queue_preq(&port_entry->queue, p);
		cthread_unlock(&port_entry->lock);
#ifdef WAIT_DEBUG
		p->waiting_for = (int)port_entry;
#endif WAIT_DEBUG
		cthread_block(p,cthread_mach_msg_real,args);
		return; /*never reached*/
	    } else {
		port_entry->held++;
		cthread_unlock(&port_entry->lock);
	    }
	} else {
	    port_entry->held++;
	    cthread_unlock(&port_entry->lock);
	}
    } else {
	cthread_unlock(&port_entry->lock);
    }
    p->busy = (int)port_entry;
    r = mach_msg(args->header, args->option,
		 args->send_size, args->rcv_size, args->rcv_name,
		 args->timeout, args->notify);
    CTHREAD_FILTER(p, CTHREAD_FILTER_USER_INVOKE, r, 0, 0, 0);
    return; /*never reached*/
}

kern_return_t cthread_mach_msg(struct cthread_mach_msg_struct *a) {
    CTHREAD_FILTER(_cthread_self(), CTHREAD_FILTER_USER_BUILD, 
		   cthread_mach_msg_real, a , 0, 0);
    return (kern_return_t)KERN_SUCCESS;/* never reached */
}

cthread_t cthread_self() {
    return (cthread_t)_cthread_self();
}

/*
 * Wire a cthread to its current kernel thread
 */

void cthread_wire()
{
    register cthread_t p = _cthread_self();

    p->wired = mach_thread_self();
    p->explicitly_wired = TRUE;
#ifdef STATISTICS
    stats_lock(&cthread_stats.lock);
    cthread_stats.wired++;
    stats_unlock(&cthread_stats.lock);
#endif STATISTICS
}

/*
 * Unwire a cthread.  Deallocate its wait port.
 */

void cthread_unwire()
{
    register cthread_t p = _cthread_self();

    if (p->wired != MACH_PORT_NULL) {
	p->wired = MACH_PORT_NULL;
	p->explicitly_wired = FALSE;
#ifdef STATISTICS
	stats_lock(&cthread_stats.lock);
	cthread_stats.wired--;
	stats_unlock(&cthread_stats.lock);
#endif STATISTICS
    }    
}


void cthread_stats(int file)
{
#ifdef STATISTICS
    fprintf(file,"mutex_miss        %d\n",cthread_stats.mutex_miss); 
    fprintf(file,"mutex_caught_spin %d\n",cthread_stats.mutex_caught_spin);
    fprintf(file,"mutex_cwl         %d\n",cthread_stats.mutex_cwl);
    fprintf(file,"mutex_block       %d\n",cthread_stats.mutex_block);
    fprintf(file,"wait_stacks       %d\n",cthread_stats.wait_stacks);
    fprintf(file,"waiters           %d\n",cthread_stats.waiters);
    fprintf(file,"idle_swtchs       %d\n",cthread_stats.idle_swtchs);
    fprintf(file,"idle_lost         %d\n",cthread_stats.idle_lost);
    fprintf(file,"idle_exit         %d\n",cthread_stats.idle_exit);
    fprintf(file,"unlock_new        %d\n",cthread_stats.unlock_new);
    fprintf(file,"blocks            %d\n",cthread_stats.blocks);
    fprintf(file,"notrigger         %d\n",cthread_stats.notrigger);
    fprintf(file,"wired             %d\n",cthread_stats.wired);
    fprintf(file,"rnone             %d\n",cthread_stats.rnone);
    fprintf(file,"spin_count        %d\n",cthread_stats.spin_count);
    fprintf(file,"umutex_enter      %d\n",cthread_stats.umutex_enter);
    fprintf(file,"umutex_noheld     %d\n",cthread_stats.umutex_noheld);
#endif STATISTICS
}
