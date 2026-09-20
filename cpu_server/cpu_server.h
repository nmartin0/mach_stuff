/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	cpu_server.h,v $
 * Revision 2.3  91/02/14  14:32:50  mrt
 * 	Changed to new Mach copyright
 * 
 * Revision 2.2  90/01/24  23:33:39  mrt
 * 	Changed include of sys/error.h to mach/error.h
 * 	[90/01/22            mrt]
 * 
 * 	Created
 * 	[90/01/18            dlb]
 * 
 */
/*
 *	File: 	cpu_server.h
 *	Author:	David Black, Carnegie Mellon University
 *	Date:	Jan 1990
 *
 */

#ifndef	_SERVERS_CPU_SERVER_
#define	_SERVERS_CPU_SERVER_

#include <servers/cpu_request.h>
/*
 *	Definitions needed by users of the cpu_server.
 */

typedef port_t		cpu_request_t;

/*
 *	Options for cpu_request activate.
 */

#define CPU_OPTION_NONE			0
#define CPU_OPTION_DESTROY		0x1
#define CPU_OPTION_REPEAT		0x2

#define CPU_VALID_OPTIONS		0x3


/*
 *	Notification types
 */

#define CPU_NOTIFY_START	1
#define CPU_NOTIFY_END		2

/*
 *	Errors for cpu server.
 */

#include <mach/error.h>

#define CPU_SUCCESS		(ERR_SUCCESS)
#define CPU_INVALID_ARGUMENT	(err_server|err_sub(4)|1)
#define CPU_LIMIT_EXCEEDED	(err_server|err_sub(4)|2)
#define CPU_REQUEST_ACTIVE	(err_server|err_sub(4)|3)

#endif	_SERVERS_CPU_SERVER_
