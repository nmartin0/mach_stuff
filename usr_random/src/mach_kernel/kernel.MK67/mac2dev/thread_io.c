/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	thread_io.c,v $
 * Revision 2.2  91/09/12  16:48:08  bohman
 * 	Created.
 * 	[91/09/11  16:06:16  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2dev/thread_io.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mach/mach_types.h>

#include <kern/sched_prim.h>

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Files.h>

extern void	thread_io_done();

thread_io(func, pb)
register OSErr		(*func)();
register ParmBlkPtr	pb;
{

    pb->ioParam.ioCompletion = thread_io_done;
    (void) (*func)(pb, TRUE);

    while (pb->ioParam.ioResult > 0) {
	assert_wait(&pb->ioParam.ioResult, FALSE);
	thread_block((void (*)()) 0);
    }

    return (pb->ioParam.ioResult);
}

void
thread_io_complete(pb)
register ParmBlkPtr	pb;
{
    thread_wakeup_one(&pb->ioParam.ioResult);
}
