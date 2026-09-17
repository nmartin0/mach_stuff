/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mach.h>

kern_return_t
access_fault(thread, code, subcode)
register mach_port_t	thread;
int			code;
register unsigned	subcode;
{
    register kern_return_t	result;

    if (code == KERN_INVALID_ADDRESS) {
	if (subcode >= 0x50f00000 && subcode < 0x51000000)
	    result = handle_fault(thread);
	else
	    result = emulate_fault(thread);
    }
    else
	result = KERN_FAILURE;
    
    return (result);
}

kern_return_t
handle_fault(thread)
register mach_port_t	thread;
{
    register kern_return_t	result = KERN_FAILURE;
    thread_state_frame_t	frame;
    unsigned			frame_count;

    frame_count = THREAD_STATE_FRAME_COUNT;
    (void) thread_get_state(thread,
			    THREAD_STATE_FRAME,
			    (thread_state_t)&frame,
                            &frame_count);
    
    switch (frame.f_normal.f_fmt) {
      case STKFMT_SHORT_BUSERR:
	if (frame.f_short_buserr.f_dfault) {
	    if (frame.f_long_buserr.f_fault == 0x50f00000) {
		frame.f_short_buserr.f_dfault = 0;
		
		result = KERN_SUCCESS;
	    }
	}
	break;
	
      case STKFMT_LONG_BUSERR:
	if (frame.f_long_buserr.f_dfault) {
	    if (frame.f_long_buserr.f_fault == 0x50f00000) {
		if (frame.f_long_buserr.f_rw == BUSERR_READ)
		    frame.f_long_buserr.f_dib = 0xffffffff;
		frame.f_long_buserr.f_dfault = 0;
		
		result = KERN_SUCCESS;
	    }
	}
	break;
    }

    if (result == KERN_SUCCESS)
	(void) thread_set_state(thread,
				THREAD_STATE_FRAME,
				(thread_state_t)&frame,
                                frame_count);

    return (result);
}

kern_return_t
emulate_fault(thread)
register mach_port_t	thread;
{
    register kern_return_t	result = KERN_FAILURE;
    thread_state_frame_t	frame;
    thread_state_regs_t		regs;
    unsigned			frame_count, regs_count;

    frame_count = THREAD_STATE_FRAME_COUNT;
    (void) thread_get_state(thread,
			    THREAD_STATE_FRAME,
			    (thread_state_t)&frame,
                            &frame_count);

    regs_count = THREAD_STATE_REGS_COUNT;
    (void) thread_get_state(thread,
			    THREAD_STATE_REGS,
			    (thread_state_t)&regs,
                            &regs_count);

    switch (frame.f_normal.f_fmt) {
      case STKFMT_SHORT_BUSERR:
	regs.r_sp -= SHORT_BUSERR_EXCEPTION_FRAME_SIZE;
	bcopy(&frame, regs.r_sp, SHORT_BUSERR_EXCEPTION_FRAME_SIZE);
	result = KERN_SUCCESS;
	break;

      case STKFMT_LONG_BUSERR:
	regs.r_sp -= LONG_BUSERR_EXCEPTION_FRAME_SIZE;
	bcopy(&frame, regs.r_sp, LONG_BUSERR_EXCEPTION_FRAME_SIZE);
	result = KERN_SUCCESS;
	break;
    }

    if (result == KERN_SUCCESS) {
	frame.f_normal.f_sr = 0;
	frame.f_normal.f_pc = *(unsigned *)frame.f_normal.f_vector;
	frame.f_normal.f_fmt = STKFMT_NORMAL;

	(void) thread_set_state(thread,
				THREAD_STATE_FRAME,
				(thread_state_t)&frame,
				NORMAL_EXCEPTION_FRAME_SIZE / sizeof (int));

	(void) thread_set_state(thread,
			       THREAD_STATE_REGS,
			       (thread_state_t)&regs,
                               regs_count);
    }

    return (result);
}
