/*
 * Copyright (c) 1998-2001 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
 
#include "fd.h"
#include <kern/sched_prim.h>
#include <kern/task.h>
#include <mach/task.h> /* for semaphore_create() */
#include <mach/semaphore.h> /* for other semaphore functions */
#include <sys/systm.h> /* for panic() */
#include "MkLinux_floppy/dprintf.h"

// #if (!MACH_DEBUG)
// #define printf donone
// #else
// #if (SERIAL_DEBUG)
// #define printf kprintf
// #endif
// #endif

#define NEVENTS 30

#define USE_SEMAPHORES 1

#if (USE_SEMAPHORES == 1)
struct event_table {
	event_t event;
	semaphore_t semptr;
	int used;
	};

void donone(char *,...);

static struct event_table org_mklinux_iokit_swim3_etable[NEVENTS];

mutex_t *org_mklinux_iokit_swim3_etablelock;

void org_mklinux_iokit_swim3_init_event_table(void)
{
int i;

org_mklinux_iokit_swim3_etablelock = mutex_alloc(0);
org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "mutex_alloc\n");
for (i=0; i<NEVENTS; i++) org_mklinux_iokit_swim3_etable[i].used = 0;
}

void org_mklinux_iokit_swim3_etable_free_sem(event_t event)
{
    int i;

    mutex_lock(org_mklinux_iokit_swim3_etablelock);

    for (i=0; i<NEVENTS; i++) {
	if (org_mklinux_iokit_swim3_etable[i].event == event) {
	    if (--org_mklinux_iokit_swim3_etable[i].used) {
		break;
	    } else {
		org_mklinux_iokit_swim3_etable[i].event = 0;
		semaphore_destroy(kernel_task,
		    org_mklinux_iokit_swim3_etable[i].semptr);
		org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "semaphore_destroy\n");
		org_mklinux_iokit_swim3_etable[i].used = 0;
	    }
	}
    }
    mutex_unlock(org_mklinux_iokit_swim3_etablelock);
}

void org_mklinux_iokit_swim3_dispose_event_table(void)
{
int i;

    for (i=0; i<NEVENTS; i++) {
	if (org_mklinux_iokit_swim3_etable[i].used) {
		org_mklinux_iokit_swim3_etable_free_sem(org_mklinux_iokit_swim3_etable[i].event);
	}
    }
    mutex_free(org_mklinux_iokit_swim3_etablelock);
    org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "mutex_free\n");
}

semaphore_t org_mklinux_iokit_swim3_etable_get_sem(event_t event)
{
    int i;

    mutex_lock(org_mklinux_iokit_swim3_etablelock);
    for (i=0; i<NEVENTS; i++) {
	if ((org_mklinux_iokit_swim3_etable[i].used) &&
	    (org_mklinux_iokit_swim3_etable[i].event == event)) {
		org_mklinux_iokit_swim3_etable[i].used++;
		mutex_unlock(org_mklinux_iokit_swim3_etablelock);
		return org_mklinux_iokit_swim3_etable[i].semptr;
	}
    }
    mutex_unlock(org_mklinux_iokit_swim3_etablelock);
    return NULL;
}

semaphore_t org_mklinux_iokit_swim3_etable_new_sem(event_t event)
{
    int i;

    /* @@@ FIXME DAG: Make this atomic! @@@ */
    mutex_lock(org_mklinux_iokit_swim3_etablelock);
    for (i=0; i<NEVENTS; i++) {
	if (!(org_mklinux_iokit_swim3_etable[i].used)) {
		org_mklinux_iokit_swim3_etable[i].used = 1;
		if (semaphore_create(kernel_task,
		    &org_mklinux_iokit_swim3_etable[i].semptr,
		    SYNC_POLICY_FIFO, 0) != KERN_SUCCESS) {
			panic("semaphore_create failed!\n");
		}
		org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "semaphore_create\n");
		org_mklinux_iokit_swim3_etable[i].event = event;
		mutex_unlock(org_mklinux_iokit_swim3_etablelock);
		return org_mklinux_iokit_swim3_etable[i].semptr;
	}
    }
    mutex_unlock(org_mklinux_iokit_swim3_etablelock);
    return NULL;
}

#endif

int non_bsd_sleep(void *event, unsigned long msec)
{
#if (USE_SEMAPHORES == 0)
assert_wait((event_t) event, THREAD_INTERRUPTIBLE);

/* If msec is zero, we're not imitating a timeout/sleep combo, so just
   go to sleep */
if (msec) thread_set_timer(msec, NSEC_PER_USEC & 1000);

return thread_block((void (*)(void)) 0);
#error WE_SHUOLD_NOT_BE_HERE
#else

    semaphore_t mysem;
    mach_timespec_t wait_time;

    if (!(mysem = org_mklinux_iokit_swim3_etable_get_sem((event_t)event))) {
	mysem = org_mklinux_iokit_swim3_etable_new_sem((event_t)event);
    }
    if (!mysem) {
	panic("swim3: Unable to allocate new semaphore in etable!\n");
    }
    if (msec) {
	/* Initialize wait_time */
	wait_time.tv_sec = msec / 1000;
	wait_time.tv_nsec = ((msec % 1000) * 1000000);
	dprintf(DEBUG_WAIT, "Waiting for %d secs, %d nsecs.\n", wait_time.tv_sec,
		wait_time.tv_nsec);

	switch(semaphore_timedwait(mysem, wait_time)) {
	    case KERN_OPERATION_TIMED_OUT:
		org_mklinux_iokit_swim3_etable_free_sem((event_t)event);
		return 0;
	    default:
		org_mklinux_iokit_swim3_etable_free_sem((event_t)event);
		return EVENT_WAKE;
	}
    } else {
	semaphore_wait(mysem);
    }
    org_mklinux_iokit_swim3_etable_free_sem((event_t)event);
    return 0;
#endif
}

void non_bsd_wakeup(void *event)
{
#if (USE_SEMAPHORES == 1)
    semaphore_t mysem;
    if (!(mysem = org_mklinux_iokit_swim3_etable_get_sem((event_t)event))) {
	mysem = org_mklinux_iokit_swim3_etable_new_sem((event_t)event);
    }
    if (!mysem) {
	panic("swim3: Unable to allocate new semaphore in etable!\n");
    }
    semaphore_signal(mysem);
    org_mklinux_iokit_swim3_etable_free_sem((event_t)event);

#else
    thread_wakeup_with_result((event_t) event, EVENT_WAKE);
#error WE_SHUOLD_NOT_BE_HERE

#endif
}

