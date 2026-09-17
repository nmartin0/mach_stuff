/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

#include <mach.h>

#include <lock.h>
#include <th.h>

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
