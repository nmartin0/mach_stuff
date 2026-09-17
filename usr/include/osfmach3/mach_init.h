/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: mach_init.h,v $
 * Revision 1.1.2.1  1996/09/09  16:57:48  barbou
 * 	Created.
 * 	[1996/08/21  18:48:29  barbou]
 *
 * $EndLog$
 */

#ifndef	_OSFMACH3_MACH_INIT_H
#define _OSFMACH3_MACH_INIT_H

#include <mach/port.h>

extern	mach_port_t	mach_task_self_;

#define	mach_task_self() mach_task_self_

#endif	/* _OSFMACH3_MACH_INIT_H */
