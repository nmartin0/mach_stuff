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

#ifndef _MACHINE_DRIVER_LOCK_H_
#define	_MACHINE_DRIVER_LOCK_H_

#include <cpus.h>
#include <mach_ldebug.h>
#include <mach/boolean.h>
#include <kern/etap_options.h>
#include <kern/lock.h>

#if NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE

#include <kern/thread.h>

#define	HP700_DRIVER_OP_START	0 /* Wants to register a pending start */
#define	HP700_DRIVER_OP_INTR	1 /* Wants to register a pending interrupt */
#define	HP700_DRIVER_OP_TIMEO	2 /* Wants to register a pending timeout */
#define HP700_DRIVER_OP_CALLB	3 /* Wants to register a pending callback */
#define	HP700_DRIVER_OP_LAST	4 /* Last pending operation */

#define	HP700_DRIVER_OP_WAIT	HP700_DRIVER_OP_LAST
				  /* Wait for exclusive use of the driver */

typedef struct hp700_driver_lock {
	decl_simple_lock_data(, dl_lock)	/* Structure protection */
	unsigned int		dl_pending;	/* Pending driver ops */
	void			(*dl_op[HP700_DRIVER_OP_LAST])(int);	
						/* Driver op routines */
	int			dl_unit;	/* Driver op argument */
	thread_t		dl_thread;	/* Thread holding the lock */
	unsigned int		dl_count;	/* Recursive lock count */
} hp700_driver_lock_t;

#define	hp700_driver_lock_decl(attr, name)	attr hp700_driver_lock_t name;

extern void		hp700_driver_lock_init(hp700_driver_lock_t *, 
					       int,
					       void (*)(int),
					       void (*)(int),
					       void (*)(int),
					       void (*)(int));

extern boolean_t	hp700_driver_lock(hp700_driver_lock_t *, 
					  unsigned int,
					  boolean_t);

extern void		hp700_driver_unlock(hp700_driver_lock_t *);

#else /* NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE */

#define	hp700_driver_lock_decl(attr, name)				\
					decl_simple_lock_data(attr, name)
#define	hp700_driver_lock_init(addr, unit, start, intr, timeo, callb)	\
					simple_lock_init((addr), ETAP_IO_DEVINS)
#define	hp700_driver_lock(lock, op, try)				\
					simple_lock_try(lock)
#define	hp700_driver_unlock(lock)					\
					simple_unlock(lock)

#endif /* NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE */

#define	hp700_driver_lock_state()

#endif /* _MACHINE_DRIVER_LOCK_H_ */
