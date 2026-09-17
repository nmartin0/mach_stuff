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
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: emul/server/catch_exception.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include "server_defs.h"

#include <mach/exception.h>

kern_return_t
catch_exception_raise(port_t	exception_port,
		      port_t	thread,
		      port_t	task_,
		      int	exception,
		      int	code,
		      int	subcode)
{
    kern_return_t	result;

    if (task_ == task)
	switch (exception) {
	  case EXC_BAD_ACCESS:
	  case EXC_BAD_INSTRUCTION:
	  case EXC_BREAKPOINT:
	    debugger(thread);
	    return (KERN_SUCCESS);
	}
    else {
	fprintf(stderr, "catch exception: wrong task\n");
	return (KERN_FAILURE);
    }

    fprintf(stderr, "unhandled exception %x code %x subcode %x\n",
	    exception,
	    code,
	    subcode);
    debugger(thread);

    return (KERN_FAILURE);
}
