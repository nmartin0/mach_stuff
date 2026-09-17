/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#ifndef _ASM_OSFMACH3_MACHINE_SIGCONTEXT_H
#define _ASM_OSFMACH3_MACHINE_SIGCONTEXT_H

#include <mach/thread_status.h>

struct sigcontext_struct {
	struct hp700_thread_state sig_state;
	struct hp700_float_state  sig_fstate;
	unsigned long oldmask;
};

#endif	/* _ASM_OSFMACH3_MACHINE_SIGCONTEXT_H */
