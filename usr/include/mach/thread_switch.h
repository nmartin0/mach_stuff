/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: thread_switch.h,v $
 * Revision 1.2.12.2  1995/01/06  19:52:14  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	64bit cleanup; thread_switch prototype; typedefs
 * 	[1994/10/14  03:43:22  dwm]
 *
 * Revision 1.2.12.1  1994/09/23  02:43:39  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:43:23  ezf]
 * 
 * Revision 1.2.10.2  1993/10/12  16:39:05  dwm
 * 	Add SWITCH_OPTION_IDLE [rwd].
 * 	[1993/10/12  16:17:38  dwm]
 * 
 * Revision 1.2.2.2  1993/06/09  02:43:58  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:18:35  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:40:06  devrcs
 * 	Cleanup for ANSI C
 * 	[1993/02/25  16:45:39  sp]
 * 
 * Revision 1.1  1992/09/30  02:32:19  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.4  91/05/14  17:01:33  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:36:45  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:02  mrt]
 * 
 * Revision 2.2  90/06/02  15:00:19  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:51:49  rpd]
 * 
 * 	Merge to X96
 * 	[89/08/02  23:12:52  dlb]
 * 
 * 	Created.
 * 	[89/07/25  19:05:41  dlb]
 * 
 * Revision 2.3  89/10/15  02:06:04  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:41:47  dlb
 * 	Merge.
 * 	[89/09/01  17:57:58  dlb]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */

#ifndef	_MACH_THREAD_SWITCH_H_
#define	_MACH_THREAD_SWITCH_H_

#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/port.h>

/*
 *	Constant definitions for thread_switch trap.
 */

#define	SWITCH_OPTION_NONE	0
#define SWITCH_OPTION_DEPRESS	1
#define SWITCH_OPTION_WAIT	2
#define SWITCH_OPTION_IDLE	3

#define valid_switch_option(opt)	((0 <= (opt)) && ((opt) <= 2))

/* Attempt to context switch */
extern kern_return_t	thread_switch(
				mach_port_t		thread_name,
				int			option,
				mach_msg_timeout_t	option_time);

#endif	/* _MACH_THREAD_SWITCH_H_ */
