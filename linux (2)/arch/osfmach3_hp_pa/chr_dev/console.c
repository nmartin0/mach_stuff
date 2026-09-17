/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


#include <mach/mach_interface.h>

#include <osfmach3/mach_init.h>
#include <osfmach3/device_utils.h>
#include <osfmach3/console.h>
#include <osfmach3/parent_server.h>
#include <osfmach3/mach3_debug.h>

#include <linux/kernel.h>

boolean_t
osfmach3_con_probe(void)
{
	kern_return_t	kr;

	if (parent_server || osfmach3_use_mach_console) {
		/* no virtual consoles */
		return FALSE;
	}

	return FALSE;
}
