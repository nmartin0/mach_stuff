/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * pmk1.1
 */

#include <cpus.h>
#include <mach_ldebug.h>
#include <mach/boolean.h>
#include <kern/assert.h>
#include <kern/thread.h>
#include <kern/cpu_data.h>
#include <kern/etap_options.h>
#include <kern/lock.h>
#include <kern/sched_prim.h>
#include <machine/driver_lock.h>

#if NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE

/*
 * hp700_driver_unlock
 *
 * Purpose : Initialize a driver lock
 * Returns : Nothing
 *
 * IN_arg  : dl    ==> Driver lock
 *	     unit  ==> Driver unit
 *	     start ==> Function to call for asynchronous start operation
 *	     intr  ==> Function to call for asynchronous interrupt operation
 *	     timeo ==> Function to call for asynchronous timeout operation
 */
void
hp700_driver_lock_init(
    hp700_driver_lock_t	*dl, 
    int			unit,
    void		(*start)(int),
    void		(*intr)(int),
    void		(*timeo)(int),
    void		(*callb)(int))
{
    simple_lock_init(&dl->dl_lock, ETAP_IO_DEVINS);
    dl->dl_unit = unit;
    dl->dl_count = 0;
    dl->dl_pending = 0;
    dl->dl_thread = current_thread();
    dl->dl_op[HP700_DRIVER_OP_START] = start;
    dl->dl_op[HP700_DRIVER_OP_INTR] = intr;
    dl->dl_op[HP700_DRIVER_OP_TIMEO] = timeo;
    dl->dl_op[HP700_DRIVER_OP_CALLB] = callb;
}

/*
 * hp700_driver_lock
 *
 * Purpose : Try to acquire a driver lock for a driver operation, and
 *		conditionally mark the driver operation as pending
 * Returns : TRUE/FALSE whether the lock is available or busy
 *
 * IN_arg  : dl  ==> Driver lock
 *	     op  ==> Driver operation
 *	     try ==> Don't mark this operation as pending if driver lock busy
 */
boolean_t
hp700_driver_lock(
    hp700_driver_lock_t	*dl, 
    unsigned int 	op,
    boolean_t		try)
{
    thread_t self = current_thread();

    assert(op <= HP700_DRIVER_OP_LAST);
    simple_lock(&dl->dl_lock);
    if (dl->dl_count == 0)
	dl->dl_thread = self;

    else if (dl->dl_thread != self) {
	if (try) {
	    simple_unlock(&dl->dl_lock);
	    return (FALSE);
	}
	if (op != HP700_DRIVER_OP_WAIT) {
	    dl->dl_pending |= (1 << op);
	    simple_unlock(&dl->dl_lock);
	    return (FALSE);
	}
	/*
	 * Wait while the lock is held
	 */
	while (dl->dl_count > 0) {
	    dl->dl_pending |= (1 << HP700_DRIVER_OP_WAIT);
	    assert_wait((event_t)&dl->dl_count, FALSE);
	    simple_unlock(&dl->dl_lock);
	    thread_block((void (*)(void))0);
	    simple_lock(&dl->dl_lock);
	}
	assert(dl->dl_pending == 0);
	dl->dl_thread = self;

    } else
	assert(op != HP700_DRIVER_OP_WAIT);

    dl->dl_count++;
    simple_unlock(&dl->dl_lock);
    return (TRUE);
}

/*
 * hp700_driver_unlock
 *
 * Purpose : Release a driver lock, starting pending driver operations
 * Returns : Nothing
 *
 * IN_arg  : dl ==> Driver lock
 */
void
hp700_driver_unlock(
    hp700_driver_lock_t *dl)
{
    unsigned int i;

    simple_lock(&dl->dl_lock);
    if (dl->dl_count == 1) {
	/*
	 * Scan for asynchronous operation to do only during last unlock
	 */
	while (dl->dl_pending & ~(1 << HP700_DRIVER_OP_WAIT))
	    for (i = 0; i < HP700_DRIVER_OP_LAST; i++)
		if (dl->dl_pending & (1 << i)) {
		    dl->dl_pending &= ~(1 << i);
		    simple_unlock(&dl->dl_lock);
		    (*dl->dl_op[i])(dl->dl_unit);
		    simple_lock(&dl->dl_lock);
		    assert(dl->dl_count == 1);
		}
	/*
	 * No more asynchronous operation to do, so wakeup waiting threads
	 */
	if (dl->dl_pending & (1 << HP700_DRIVER_OP_WAIT)) {
	    dl->dl_pending &= ~(1 << HP700_DRIVER_OP_WAIT);
	    thread_wakeup((event_t)&dl->dl_count);
	}
    } else
	assert(dl->dl_count != 0);
    dl->dl_count--;
    simple_unlock(&dl->dl_lock);
}

#endif /* NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE */
