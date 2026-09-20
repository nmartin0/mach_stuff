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
 * 31-Mar-93  Randall Dean (rwd) at Carnegie-Mellon University
 *	Written.
 *
 * $Log:	call.c,v $
 * Revision 2.4.2.1  92/06/22  11:49:28  rwd
 * 	Added new stubs.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.4  91/05/14  17:56:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:19:20  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:40:44  mrt]
 * 
 * Revision 2.2  90/01/19  14:36:50  rwd
 * 	Created.  Routines to replace thread_* and cthread_call_on.
 * 	[90/01/03            rwd]
 * 
 */

#include <cthreads.h>
#include <cthread_filter.h>
#include "cthread_internals.h"

#ifdef THREAD_CALLS
#define MAXNAMESIZE 128

extern void *malloc(unsigned long);
extern void free(void *);
extern void bcopy(void *, void *, unsigned long);

typedef struct {
    vm_address_t external;
    vm_address_t internal;
    vm_size_t object_size;
    vm_size_t alloc_size;
    boolean_t mapped;
    boolean_t inuse;
    unsigned int timestamp;
    struct task_list *tl;
} addr_pair, *ap_t;

typedef struct dthread {
    struct task_list *tl;
    addr_pair cthread;
    struct dthread *next;
    boolean_t reused;
    struct real_thread *rt;
    addr_pair name;
} *dthread_t;

typedef struct real_thread {
    struct real_thread *next;
    thread_t thread;
    thread_state state;
    dthread_t dt;
} *rt_t;

typedef struct task_list {
    mach_port_t task;
    addr_pair sstruct;
    unsigned int dt_timestamp;
    dthread_t dt;
    struct task_list *next;
    int number_rts;
    unsigned int rt_timestamp;
    rt_t rt;
} *tl_t;

private tl_t master_tl = (tl_t)0;
private unsigned int global_timestamp = 1;
private struct mutex call_lock = MUTEX_INITIALIZER;

private inline void ts_update()
{
    global_timestamp++;
}

private inline vm_address_t trunc(vm_address_t a)
{
    return (a & ~(vm_page_size - 1));
}

private inline vm_address_t round(vm_address_t a)
{
    return (trunc(a + vm_page_size - 1));
}

private inline void ap_zero(ap_t ap)
{
    ap->inuse = FALSE;
}

private void ap_init(tl_t tl, ap_t ap, vm_address_t addr, vm_size_t size)
{
    if (!ap->inuse) {
	ap->tl = tl;
	ap->external = addr;
	ap->timestamp = global_timestamp - 1;
	ap->object_size = size;
	ap->internal = (vm_address_t)0;
	ap->alloc_size = round(addr + size - trunc(addr));
	ap->mapped = FALSE;
	ap->inuse = TRUE;
    }
}

private inline vm_address_t ap_external(ap_t ap)
{
    return (ap->external);
}

private inline boolean_t ap_match(ap_t ap, vm_address_t addr)
{
    return (ap->inuse && ap->external == addr);
}

private kern_return_t ap_writeback(ap_t ap)
{
    kern_return_t ret = KERN_SUCCESS;
    if (ap->inuse && ap->mapped)
	ret = vm_write(ap->tl->task, trunc(ap->external),
		       ap->internal,
		       ap->alloc_size);
    return ret;

}

private kern_return_t ap_cleanup(ap_t ap)
{
    kern_return_t ret = KERN_SUCCESS;
    if (ap->inuse && ap->mapped)
	ret = vm_deallocate(mach_task_self(), ap->internal, ap->alloc_size);
    ap->mapped = FALSE;
    return ret;
}

private kern_return_t ap_validate(ap_t ap, vm_offset_t offset, 
				  vm_size_t size)
{
    kern_return_t ret = KERN_SUCCESS;
    if (!ap->inuse)
	return KERN_FAILURE;
    if (offset + size > ap->object_size)
	return KERN_INVALID_ADDRESS;
    if (ap->timestamp == global_timestamp)
	return ret;
    ret = ap_cleanup(ap);
    if (ret == KERN_SUCCESS) {
	ret = vm_read(ap->tl->task,
		      trunc(ap->external),
		      ap->alloc_size,
		      &ap->internal,
		      &ap->alloc_size);
	ap->timestamp = global_timestamp;
	ap->mapped = TRUE;
    }
    return ret;
}

private kern_return_t ap_ptr(ap_t ap, vm_offset_t offset, vm_size_t size,
		     void **ptr)
{
    kern_return_t ret;
    ret = ap_validate(ap, offset, size);
    if (ret == KERN_SUCCESS)
	*ptr = (void *)(ap->internal + offset + 
			(ap->external - trunc(ap->external)));
    return ret;
}


private kern_return_t cthread_get_threads(tl_t tl)
{
    dthread_t dt;
    kern_return_t ret = KERN_SUCCESS;
    cthread_status_t css;

    if (tl->dt_timestamp == global_timestamp) return (KERN_SUCCESS);
    for(dt=tl->dt;dt!=(dthread_t)0;dt=dt->next) {
	ap_cleanup(&dt->cthread);
	dt->reused = FALSE;
    }
    ret = ap_ptr(&tl->sstruct, 0, sizeof(struct cthread_status_struct), 
	       (void **)(&css));
    if (ret == KERN_SUCCESS) {
	cthread_t ct;
	for(ct = css->cthread_list; ret == KERN_SUCCESS; ct = ct->list) {
	    if (ct == NO_CTHREAD) break;
	    for(dt=tl->dt;dt!=(dthread_t)0;dt=dt->next)
		if (ap_match(&dt->cthread, (vm_address_t)ct)) break;
	    if (!dt) {
		dt = (dthread_t)malloc(sizeof(struct dthread));
		dt->tl = tl;
		ap_zero(&dt->cthread);
		ap_zero(&dt->name);
		ap_init(tl, &dt->cthread, (vm_address_t)ct, 
			sizeof(struct cthread));
		dt->next = tl->dt;
		tl->dt = dt;
	    }
	    dt->reused = TRUE;
	    ret = ap_ptr(&dt->cthread, 0, sizeof(struct cthread), 
			 (void **)&ct);
	}
    }
    if (ret == KERN_SUCCESS) {
	dthread_t *dtp;
	for(dtp = &tl->dt;*dtp;)
	    if ((*dtp)->reused == FALSE) {
		dthread_t dtd = *dtp;
		*dtp = dtd->next;
		ap_cleanup(&dtd->cthread);
		free((void *)dtd);
	    } else {
		dtp = &((*dtp)->next);
	    }
    }
    tl->dt_timestamp = global_timestamp;
    return (ret);
}

private kern_return_t cthread_get_real_threads(tl_t tl)
{
    kern_return_t ret;
    thread_array_t thread_list;
    int thread_count, i;
    rt_t rt;
    int count;

    if (tl->rt_timestamp == global_timestamp) return (KERN_SUCCESS);
    ret = task_threads(tl->task, &thread_list, &thread_count);
    if (ret != KERN_SUCCESS)
	return (ret);
    tl->number_rts = thread_count;
    for(i=0;i<thread_count;i++) {
	for(rt = tl->rt; rt != (rt_t)0; rt=rt->next)
	    if (rt->thread == thread_list[i]) break;
	if (rt == (rt_t)0) {
	    rt = (rt_t)malloc(sizeof(*rt));
	    rt->next = tl->rt;
	    tl->rt = rt;
	    rt->thread = thread_list[i];
	}
	rt->dt = (dthread_t)0;
	count = STATE_COUNT;
	ret = thread_get_state(rt->thread, STATE_FLAVOR,
			       (thread_state_t)&rt->state,
			       &count);
    }
    return (ret);
}


private kern_return_t cthread_find_threads(tl_t tl)
{
    kern_return_t ret = KERN_SUCCESS;
    dthread_t dt;
    rt_t rt;
    cthread_t ct;

    ret = cthread_get_threads(tl);
    if (ret != KERN_SUCCESS)
	return (ret);
    ret = cthread_get_real_threads(tl);
    if (ret != KERN_SUCCESS)
	return (ret);
    for(dt=tl->dt;dt != (dthread_t)0; dt=dt->next)
	dt->rt = (rt_t)0;
    for(rt = tl->rt; rt != (rt_t)0; rt=rt->next) {
	for(dt=tl->dt;dt != (dthread_t)0; dt=dt->next) {
	    ret = ap_ptr(&dt->cthread, 0, sizeof(struct cthread),(void **)&ct);
	    if (ret != KERN_SUCCESS)
		return (ret);
	    if (STATE_STACK(&rt->state) >= ct->stack_base &&
		STATE_STACK(&rt->state) <= ct->stack_base + ct->stack_size)
		break;
	}
	if (dt) {
	    rt->dt = dt;
	    dt->rt = rt;
	} else {
	    printf("ACK 1\n"); /* no idea here */
	    return (KERN_FAILURE);
	}
    }
    tl->rt_timestamp = global_timestamp;
    return (ret);
}

void cthread_thread_calls_start()
{
    ts_update();
}

void cthread_thread_calls_end()
{
}

kern_return_t cthread_init_thread_calls(task_t task, vm_address_t address)
{
    cthread_status_t css;
    kern_return_t ret;
    tl_t tl;

    mutex_lock(&call_lock);
    ts_update();
    tl = master_tl;
    while(tl!=(tl_t)0) {
	if (tl->task == task) break;
	tl = tl->next;
    }
    if (tl == (tl_t)0) {
	tl = (tl_t)malloc(sizeof(struct task_list));
	ap_zero(&tl->sstruct);
	tl->next = master_tl;
	tl->task = task;
	tl->rt = (rt_t)0;
	tl->dt = (dthread_t)0;
	tl->number_rts = 0;
	tl->rt_timestamp = global_timestamp - 1;
	tl->dt_timestamp = global_timestamp - 1;
	ap_zero(&tl->sstruct);
	master_tl = tl;
    }
    ap_cleanup(&tl->sstruct);
    ap_zero(&tl->sstruct);
    ap_init(tl, &tl->sstruct, (vm_address_t)address, 
	    sizeof(struct cthread_status_struct));
    ret = ap_ptr(&tl->sstruct, 0, sizeof(struct cthread_status_struct), 
	       (void **)(&css));
    if (ret == KERN_SUCCESS) {
	if (css->version != CTHREADS_VERSION)
	    ret = KERN_FAILURE;
    }
    mutex_unlock(&call_lock);
    return ret;
}

kern_return_t cthread_get_state(cthread_t thread, int flavor,
				thread_state *state, int *size)
{
    dthread_t dt = (dthread_t)thread;
    kern_return_t ret = KERN_SUCCESS;
    mutex_lock(&call_lock);
    ret = cthread_find_threads(dt->tl);
    if (ret == KERN_SUCCESS) {
	if (dt->rt) {
	    bcopy((void *)&dt->rt->state, (void *)state, sizeof(thread_state));
	} else {
	    struct cthread *ct;
	    struct cthread context;
	    addr_pair ap;
	    int size, maxsize;
	    ret = ap_ptr(&dt->cthread, 0, sizeof(struct cthread), (void **)&ct);
	    if (ret != KERN_SUCCESS) {
		mutex_unlock(&call_lock);
		return (ret);
	    }
	    maxsize = ct->stack_base + ct->stack_size - ct->context;
	    size = (maxsize % vm_page_size) + vm_page_size;
	    if (size > maxsize) size = maxsize;
	    ap_zero(&ap);
	    ap_init(dt->tl, &ap, ct->context, size);
	    ret = ap_ptr(&ap, 0, size, (void **)&context.context);
	    if (ret != KERN_SUCCESS) {
		mutex_unlock(&call_lock);
		return (ret);
	    }
	    /* This is faulty.  It assumes default filter */
	    CTHREAD_FILTER(&context, CTHREAD_FILTER_GET_STATE,
			   state,
			   0,
			   0,
			   ct->context);
	    ap_cleanup(&ap);
	}
    }
    mutex_unlock(&call_lock);
    return ret;
}

kern_return_t cthread_set_state(cthread_t thread, int flavor,
				thread_state *state, int size)
{
    dthread_t dt = (dthread_t)thread;
    kern_return_t ret = KERN_SUCCESS;
    mutex_lock(&call_lock);
    ret = cthread_find_threads(dt->tl);
    if (ret == KERN_SUCCESS) {
	if (dt->rt) {
	    bcopy((void *)state, (void *)&dt->rt->state, sizeof(thread_state));
	    ret = thread_set_state(dt->rt->thread, STATE_FLAVOR,
				   (thread_state_t)&dt->rt->state,
				   size);
	} else {
	    struct cthread *ct;
	    struct cthread context;
	    int msize, maxsize;
	    addr_pair ap;
	    ret = ap_ptr(&dt->cthread, 0, sizeof(struct cthread), (void **)&ct);
	    if (ret != KERN_SUCCESS) {
		mutex_unlock(&call_lock);
		return (ret);
	    }
	    maxsize = ct->stack_base + ct->stack_size - ct->context;
	    msize = (maxsize % vm_page_size) + vm_page_size;
	    if (msize > maxsize) msize = maxsize;
	    ap_zero(&ap);
	    ap_init(dt->tl, &ap, ct->context, msize);
	    ret = ap_ptr(&ap, 0, size, (void **)&context.context);
	    if (ret != KERN_SUCCESS) {
		mutex_unlock(&call_lock);
		return (ret);
	    }
	    CTHREAD_FILTER(&context, CTHREAD_FILTER_SET_STATE,
			   state,
			   0,
			   0,
			   ct->context);
	    ret = ap_writeback(&ap);
	    ap_cleanup(&ap);
	}
    }
    mutex_unlock(&call_lock);
    return ret;
}

kern_return_t cthread_info(cthread_t thread, cthread_info_t *info)
{
    dthread_t dt = (dthread_t)thread;
    kern_return_t ret = KERN_SUCCESS;
    mutex_lock(&call_lock);
    ret = cthread_find_threads(dt->tl);
    if (ret == KERN_SUCCESS) {
	cthread_t ct;
	ret = ap_ptr(&dt->cthread, 0, sizeof(*ct), (void **)&ct);
	if (ret == KERN_SUCCESS) {
	    info->name = (char *)0;
	    if (ct->name) {
		char *name;
		ap_init(dt->tl, &dt->name, (vm_address_t)ct->name,
			MAXNAMESIZE);
		ret = ap_ptr(&dt->name, 0, MAXNAMESIZE, (void **)&name);
		if (ret == KERN_SUCCESS) {
		    info->name = malloc(strlen(name)+1);
		    strcpy(info->name, name);
		}
		ret = KERN_SUCCESS;
	    }
	    info->remote_cthread = ap_external(&dt->cthread);
	    info->running = (dt->rt != (rt_t)0);
	    info->runnable = (!info->running) && 
		(ct->state & CTHREAD_RUNNABLE);
	    info->waiter = (ct->state & CTHREAD_WAITER);
	}
    }
    mutex_unlock(&call_lock);
    return ret;
}

kern_return_t cthread_abort(cthread_t thread)
{
    dthread_t dt = (dthread_t)thread;
    kern_return_t ret = KERN_SUCCESS;
    mutex_lock(&call_lock);
    ret = cthread_find_threads(dt->tl);
    if (ret == KERN_SUCCESS) {
	if (dt->rt) {
	    ret = thread_abort(dt->rt->thread);
	} else {
	}
    }
    mutex_unlock(&call_lock);
    return ret;
}

kern_return_t cthread_resume(cthread_t thread)
{
    dthread_t dt = (dthread_t)thread;
    kern_return_t ret = KERN_SUCCESS;
    mutex_lock(&call_lock);
    ret = cthread_find_threads(dt->tl);
    if (ret == KERN_SUCCESS) {
	if (dt->rt) {
	    ret = thread_resume(dt->rt->thread);
	} else {
	}
    }
    mutex_unlock(&call_lock);
    return ret;
}

kern_return_t cthread_suspend(cthread_t thread)
{
    dthread_t dt = (dthread_t)thread;
    kern_return_t ret = KERN_SUCCESS;
    mutex_lock(&call_lock);
    ret = cthread_find_threads(dt->tl);
    if (ret == KERN_SUCCESS) {
	if (dt->rt) {
	    ret = thread_suspend(dt->rt->thread);
	} else {
	}
    }
    mutex_unlock(&call_lock);
    return ret;
}

kern_return_t cthread_threads(task_t task, cthread_t **list, 
			      int *count)
{
    kern_return_t ret;
    tl_t tl;
    dthread_t dt;

    mutex_lock(&call_lock);
    tl = master_tl;
    while(tl!=(tl_t)0) {
	if (tl->task == task) break;
	tl = tl->next;
    }
    if (tl == (tl_t)0) {
	mutex_unlock(&call_lock);
	return (KERN_FAILURE);
    }

    ret = cthread_get_threads(tl);

    if (ret == KERN_SUCCESS) {
	int number_threads = 0;
	for(dt = tl->dt ; dt ; dt = dt->next)
	    number_threads++;
	*count = number_threads;
	*list = (cthread_t *)malloc(sizeof(cthread_t)*number_threads);
	for(number_threads=0,dt=tl->dt;dt;dt=dt->next)
	    (*list)[number_threads++] = (cthread_t)dt;
    }
    ret = cthread_get_real_threads(tl);
    mutex_unlock(&call_lock);
    return (ret);
}

cthread_t cthread_thread_to_cthread(thread_t thread)
{
    kern_return_t ret;
    tl_t tl;
    
    mutex_lock(&call_lock);
    for(tl=master_tl;tl!=(tl_t)0;tl=tl->next) {
	if ((ret = cthread_find_threads(tl))== KERN_SUCCESS) {
	    rt_t rt;
	    for(rt=tl->rt;rt!=(rt_t)0;rt=rt->next)
		if (rt->thread == thread) {
		    mutex_unlock(&call_lock);
		    return((cthread_t)rt->dt);
		}
	}
    }
    mutex_unlock(&call_lock);
    return (NO_CTHREAD);
}

#endif THREAD_CALLS
