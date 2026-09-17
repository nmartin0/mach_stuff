/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#ifndef	_OSFMACH3_MACH_INIT_H
#define _OSFMACH3_MACH_INIT_H

#include <mach/port.h>

extern	mach_port_t	mach_task_self_;

#define	mach_task_self() mach_task_self_

#endif	/* _OSFMACH3_MACH_INIT_H */
