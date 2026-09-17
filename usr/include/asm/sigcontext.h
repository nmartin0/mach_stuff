/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: sigcontext.h,v $
 * Revision 1.1.2.1  1997/02/27  11:10:53  bruel
 * 	First revision
 * 	[1997/02/27  11:01:15  bruel]
 *
 * $EndLog$
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
