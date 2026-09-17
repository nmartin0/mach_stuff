/*
 * @OSF_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: rthreads.h,v $
 * Revision 1.1.5.1  1996/02/05  17:36:35  emcmanus
 * 	Copied from nmk20b5_shared.
 * 	[1996/02/05  17:23:23  emcmanus]
 *
 * Revision 1.1.3.1  1996/01/02  13:33:45  bruel
 * 	first revision.
 * 	[96/01/02            bruel]
 * 
 * $EndLog$
 */

#ifndef _MACHINE_RTHREADS_H_
#define _MACHINE_RTHREADS_H_

#ifndef RTHREADS
#define RTHREADS 1
#endif	/*RTHREADS*/

#include <mach/machine/thread_status.h>
#include <mach/boolean.h>

#define RTHREAD_STACK_OFFSET 128

extern int rthread_sp(void);

#define STATE_FLAVOR HP700_THREAD_STATE
#define STATE_COUNT HP700_THREAD_STATE_COUNT
typedef struct hp700_thread_state thread_state;
#define STATE_STACK(state) ((state)->r30)

#endif /* _MACHINE_RTHREADS_H_ */
