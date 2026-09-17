/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

#include <mach/thread_switch.h>

#include "lock.h"
#include "sched.h"

inline
boolean_t
lock_try(l)
register lock_t	l;
{
    register boolean_t	result;

    asm volatile("tas %1@; bne 0f; moveq #1,%0; bra 1f; 0: moveq #0,%0; 1:"
		 : "=d" (result) : "a" (l));

    return (result);
}

void
lock(l)
register lock_t	l;
{
    int		x;

    while (!lock_try(l))
	swtch_pri(0);

    l[1] = (unsigned)&x;
}

void
unlock(l)
register lock_t	l;
{
    *l = 0;
}
