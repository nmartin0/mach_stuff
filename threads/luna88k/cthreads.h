/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * 11-Aug-92  Randall Dean (rwd) at Carnegie-Mellon University
 *	Added multiprocessor defines.
 *
 * $Log:	cthreads.h,v $
 * Revision 2.2  91/08/24  12:34:23  af
 * 	Created by rwd
 * 	[91/07/19  18:45:07  danner]
 * 
 * Revision 2.1.3.1  91/08/19  13:49:35  danner
 * 	Created by rwd
 * 	[91/07/19  18:45:07  danner]
 * 
 * Revision 2.2  90/11/05  14:38:00  rpd
 * 	Created.
 * 	[90/11/01            rwd]
 * 
 */

#ifndef _MACHINE_CTHREADS_H_
#define _MACHINE_CTHREADS_H_

typedef volatile int spin_lock_t;
#define SPIN_LOCK_INITIALIZER 0
#define spin_lock_init(s) *(s)=0
#define spin_lock_locked(s) (*(s) != 0)
#define CTHREAD_STACK_OFFSET 128
#define CTHREAD_WAITER_SPIN_COUNT 300
#define MUTEX_SPIN_COUNT 100
#define LOCK_SPIN_COUNT 100

#endif _MACHINE_CTHREADS_H_
