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
 * 17-Mar-93  Randall Dean (rwd) at Carnegie-Mellon University
 *	Created.
 *
 * $Log$
 */
#include <cthreads.h>
#include <cthread_filter.h>
#include "cthread_internals.h"

/* Available to outside for statistics */

struct cthread_status_struct cthread_status = {
    CTHREADS_VERSION,
    QUEUE_INITIALIZER,
    0,
    0,
    NO_CTHREAD,
    0,
    0,
    0,
    1,
    8192,
    0,
    0,
    0,
    0,
    SPIN_LOCK_INITIALIZER,
    SPIN_LOCK_INITIALIZER,
    QUEUE_INITIALIZER,
    CTHREAD_DLQ_INITIALIZER,
    QUEUE_INITIALIZER,
    PORT_ENTRY_NULL,
    SPIN_LOCK_INITIALIZER };

#ifdef STATISTICS
struct cthread_statistics_struct cthread_stats = {SPIN_LOCK_INITIALIZER,0,0,
						      0,0,0,0,0,0,0,0,0,0,0,
						      0,0,0,0};
#endif STATISTICS

