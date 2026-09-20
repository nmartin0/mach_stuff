/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * 08-Jan-93  Randall Dean (rwd) at Carnegie-Mellon University
 *	Massive simplification of exported STUFF.
 *
 * $Log:	cthread_internals.h,v $
 * Revision 2.13.1.1  92/06/22  11:49:47  rwd
 * 	Add cthread_status struct
 * 	[92/02/10            rwd]
 * 
 * 	Changes for single internal lock version of cproc.c
 * 	[92/01/09            rwd]
 * 
 * Revision 2.13  92/03/06  14:09:24  rpd
 * 	Added yield, defined using thread_switch.
 * 	[92/03/06            rpd]
 * 
 * Revision 2.12  92/03/01  00:40:23  rpd
 * 	Removed exit declaration.  It conflicted with the real thing.
 * 	[92/02/29            rpd]
 * 
 * Revision 2.11  91/08/28  11:19:23  jsb
 * 	Fixed MACH_CALL to allow multi-line expressions.
 * 	[91/08/23            rpd]
 * 
 * Revision 2.10  91/07/31  18:33:33  dbg
 * 	Protect against redefinition of ASSERT.
 * 	[91/07/30  17:33:21  dbg]
 * 
 * Revision 2.9  91/05/14  17:56:24  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/02/14  14:19:42  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:41:02  mrt]
 * 
 * Revision 2.7  90/11/05  14:36:55  rpd
 * 	Added spin_lock_t.
 * 	[90/10/31            rwd]
 * 
 * Revision 2.6  90/09/09  14:34:51  rpd
 * 	Remove special field.
 * 	[90/08/24            rwd]
 * 
 * Revision 2.5  90/06/02  15:13:44  rpd
 * 	Converted to new IPC.
 * 	[90/03/20  20:52:47  rpd]
 * 
 * Revision 2.4  90/03/14  21:12:11  rwd
 * 	Added waiting_for field for debugging deadlocks.
 * 	[90/03/01            rwd]
 * 	Added list field to keep a master list of all cprocs.
 * 	[90/03/01            rwd]
 * 
 * Revision 2.3  90/01/19  14:37:08  rwd
 * 	Keep track of real thread for use in thread_* substitutes.
 * 	Add CPROC_ARUN for about to run and CPROC_HOLD to avoid holding
 * 	spin_locks over system calls.
 * 	[90/01/03            rwd]
 * 	Add busy field to be used by cthread_msg calls to make sure we
 * 	have the right number of blocked kernel threads.
 * 	[89/12/21            rwd]
 * 
 * Revision 2.2  89/12/08  19:53:28  rwd
 * 	Added CPROC_CONDWAIT state
 * 	[89/11/28            rwd]
 * 	Added on_special field.
 * 	[89/11/26            rwd]
 * 	Removed MSGOPT conditionals
 * 	[89/11/25            rwd]
 * 	Removed old debugging code.  Add wired port/flag.  Add state
 * 	for small state machine.
 * 	[89/10/30            rwd]
 * 	Added CPDEBUG code
 * 	[89/10/26            rwd]
 * 	Change TRACE to {x;} else.
 * 	[89/10/24            rwd]
 * 	Rewrote to work for limited number of kernel threads.  This is
 * 	basically a merge of coroutine and thread.  Added
 * 	cthread_receivce call for use by servers.
 * 	[89/10/23            rwd]
 * 
 */
/*
 * cthread_internals.h
 *
 *
 * Private definitions for the C Threads implementation.
 *
 * The cproc structure is used for different implementations
 * of the basic schedulable units that execute cthreads.
 *
 */

#include "options.h"
#include <mach/port.h>
#include <mach/message.h>
#include <mach/thread_switch.h>
#include <setjmp.h>

#define CTHREADS_VERSION 2

#define	cthread_queue_init(q)	((q)->head = (q)->tail = 0)

#define	cthread_queue_enq(q, x) \
	do { \
		(x)->next = 0; \
		if ((q)->tail == 0) \
			(q)->head = (cthread_queue_item_t) (x); \
		else \
			(q)->tail->next = (cthread_queue_item_t) (x); \
		(q)->tail = (cthread_queue_item_t) (x); \
	} while (0)

#define	cthread_queue_preq(q, x) \
	do { \
		if ((q)->tail == 0) \
			(q)->tail = (cthread_queue_item_t) (x); \
		((cthread_queue_item_t) (x))->next = (q)->head; \
		(q)->head = (cthread_queue_item_t) (x); \
	} while (0)

#define	cthread_queue_head(q, t)	((t) ((q)->head))

#define	cthread_queue_deq(q, t, x) \
    do { \
	if (((x) = (t) ((q)->head)) != 0 && \
	    ((q)->head = (cthread_queue_item_t) ((x)->next)) == 0) \
		(q)->tail = 0; \
    } while (0)


struct cthread_dlq_entry {
	struct cthread_dlq_entry	*next;		/* next element */
	struct cthread_dlq_entry	*prev;		/* previous element */
};
typedef	struct cthread_dlq_entry *cthread_dlq_entry_t;
#define CTHREAD_DLQ_INITIALIZER {(cthread_dlq_entry_t)0, (cthread_dlq_entry_t)0}
#define	cthread_dlq_init(q)	((q)->next = (q)->prev = q)
#define	cthread_dlq_first(q)	((q)->next)
#define	cthread_dlq_end(q, qe)	((q) == (qe))
#define	cthread_dlq_empty(q)	cthread_dlq_end((q), cthread_dlq_first(q))
#define cthread_dlq_enter(head, elt, type, field)		\
{ 								\
	register cthread_dlq_entry_t prev;			\
								\
	prev = (head)->prev;					\
	if ((head) == prev) {					\
		(head)->next = (cthread_dlq_entry_t) (elt);	\
	}							\
	else {							\
		((type)prev)->field.next = (cthread_dlq_entry_t)(elt);\
	}							\
	(elt)->field.prev = prev;				\
	(elt)->field.next = head;				\
	(head)->prev = (cthread_dlq_entry_t) elt;		\
}
#define	cthread_dlq_remove(head, elt, type, field)		\
{								\
	register cthread_dlq_entry_t	next, prev;		\
								\
	next = (elt)->field.next;				\
	prev = (elt)->field.prev;				\
								\
	if ((head) == next)					\
		(head)->prev = prev;				\
	else							\
		((type)next)->field.prev = prev;		\
								\
	if ((head) == prev)					\
		(head)->next = next;				\
	else							\
		((type)prev)->field.next = next;		\
}

/*
 * Low-level thread implementation.
 */
struct cthread {
	struct cthread *next;
	int status;
	cthread_fn_t func;
	void *arg;
	void *result;
	const char *name;
	void *data;
	struct cthread *joiner;
	int context;
	struct cthread *list;		/* for master cthread list */
	mutex_t cond_mutex;
#ifdef	WAIT_DEBUG
	int waiting_for;
#endif	WAIT_DEBUG
	volatile int state;			/* current state */
#define CTHREAD_RUNNING		1
#define CTHREAD_RUNNABLE	2
#define CTHREAD_BLOCKED		4
#define CTHREAD_WAITER		8
	mach_port_t wired;		/* is cthread wired to kernel thread */
	boolean_t explicitly_wired;	/* cthread_wire was called on this */
	int busy;			/* used with cthread_msg calls */
	mach_port_t reply_port;		/* for mig_get_reply_port() */
	struct cthread_dlq_entry dlq;
	unsigned int stack_base;
	unsigned int stack_size;
	struct cthread_status_struct *cthread_status;
};

#ifdef	STACK_GROWTH_UP
#define	cthread_ptr(sp) (*(cthread_t_t *) ((sp) & cthread_status.stack_mask))
#else	STACK_GROWTH_UP
#define	cthread_ptr(sp) \
	(* (cthread_t *) ( ((sp) | cthread_status.stack_mask) + 1 \
			      - sizeof(cthread_t *)) )
#endif	STACK_GROWTH_UP

#define	_cthread_self()		((cthread_t) cthread_ptr(cthread_sp()))

#define CTHREAD_FILTER(a, b, c, d, e, f) cthread_filter(  \
 		       (int *)(&(a)->context), (int)(b), (void *)(c), \
		       (void *)(d), (void *)(e), (void *)(f))

/*
 * Macro for MACH kernel calls.
 */
#ifdef CHECK_STATUS
#define	MACH_CALL(expr, ret)	\
	if (((ret) = (expr)) != KERN_SUCCESS) { \
	quit(1, "error in %s at %d: %s\n", __FILE__, __LINE__, \
	     mach_error_string(ret)); \
	} else
#else CHECK_STATUS
#define MACH_CALL(expr, ret) (ret) = (expr)
#endif CHECK_STATUS

#define private static

/*
 * Port_entry's are used by cthread_mach_msg to store information
 * about each port/port_set for which it is managing threads
 */

typedef struct port_entry {
    struct port_entry *next;	/* next port_entry */
    mach_port_t	port;		/* which port/port_set */
    struct cthread_queue queue;	/* queue of runnable threads for
				   this port/port_set */
    int min;			/* minimum number of kernel threads
				   to be used by this port/port_set */
    int max;			/* maximum number of kernel threads
				   to be used by this port/port_set */
    int held;			/* actual number of kernel threads
				   currentlt in use */
    spin_lock_t lock;		/* lock for structure */
} *port_entry_t;

#define PORT_ENTRY_NULL ((port_entry_t) 0)

typedef struct cthread_status_struct {
    int version;
    struct cthread_queue cthreads;
    int cthread_cthreads;
    int alloc_cthreads;
    cthread_t cthread_list;	/* list of all cthreads */
    vm_size_t stack_size;	/* size of user stacks */
    vm_offset_t stack_mask;	/* mask to begining */
    int lock_spin_count;	/* how long to spin trying for spin lock w/o
				   thread_switch ing */
    int processors;		/* actual number of processors in PS */
    vm_size_t wait_stack_size;	/* stack size for idle threads */
    int max_kernel_threads;	/* max kernel threads */
    int kernel_threads;		/* current kernel threads */
    int waiter_spin_count;	/* amount of busy spin on idle thread */
    int mutex_spin_count;       /* amount of busy spin in mutex_lock_solid */
    spin_lock_t n_kern_lock;    /* lock for 2 above */
    spin_lock_t run_lock;	/* the spin lock for all context switches */
    struct cthread_queue run_queue;/* run queue */
    struct cthread_dlq_entry waiting_dlq;
    struct cthread_queue waiters;/* queue of cthreads to run as idle */
    port_entry_t port_list;     /* master list of port_entries */
    spin_lock_t port_lock;      /* lock for above */
} *cthread_status_t;

extern struct cthread_status_struct cthread_status;

#ifdef STATISTICS
typedef struct cthread_statistics_struct {
    spin_lock_t lock; /* only used for some */
    int mutex_block;
    int mutex_miss; 
    int waiters;
    int unlock_new;
    int blocks;
    int notrigger;
    int wired;
    int wait_stacks;
    int rnone;
    int spin_count;
    int mutex_caught_spin;
    int idle_exit;
    int mutex_cwl;
    int idle_swtchs;
    int idle_lost;
    int umutex_enter;
    int umutex_noheld;
} *cthread_stats_t;
#endif STATISTICS
