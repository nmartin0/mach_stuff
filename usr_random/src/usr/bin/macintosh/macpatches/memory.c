/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

#include "lock.h"
#include "th.h"

extern unsigned char	end[];

unsigned		free_ptr = (unsigned)end;

lock_t			free_ptr_lock;

unsigned
malloc(size)
unsigned	size;
{
    register unsigned	p;

    lock(free_ptr_lock);

    if (size == TH_STACK_SIZE || size == vm_page_size)
	free_ptr = ((free_ptr + (size - 1)) & ~(size - 1));

    p = free_ptr; free_ptr += size;

    unlock(free_ptr_lock);

    return (p);
}
