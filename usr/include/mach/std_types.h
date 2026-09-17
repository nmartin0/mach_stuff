/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: std_types.h,v $
 * Revision 1.2.11.3  1995/01/10  05:17:47  devrcs
 * 	mk6 CR801 - merge up from nmk18b4 to nmk18b7
 * 	* Rev 1.2.11.2  1994/10/19  16:25:54  watkins
 * 	  Use vm_types.h include.
 * 	[1994/12/09  21:14:11  dwm]
 *
 * Revision 1.2.11.1  1994/09/23  02:42:33  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:42:48  ezf]
 * 
 * Revision 1.2.4.3  1993/08/03  19:09:58  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	CR9598: Remove unneeded EXPORT_BOOLEAN definition.
 * 	CR9601: Moved vm_address_t here from mach_types.h header file.
 * 	[1993/08/02  18:33:33  gm]
 * 
 * Revision 1.2.4.2  1993/06/09  02:43:21  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:18:11  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:39:12  devrcs
 * 	ansi C conformance changes
 * 	[1993/02/02  18:54:50  david]
 * 
 * Revision 1.1  1992/09/30  02:32:05  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.4  91/06/25  10:30:50  rpd
 * 	Added ipc/ipc_port.h inside the kernel.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.3  91/05/14  16:59:01  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:35:39  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:57  mrt]
 * 
 * Revision 2.1  89/08/03  16:04:44  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:40:23  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.2  89/01/15  16:31:59  rpd
 * 	Moved from kern/ to mach/.
 * 	[89/01/15  14:34:14  rpd]
 * 
 * Revision 2.2  89/01/12  07:59:07  rpd
 * 	Created.
 * 	[89/01/12  04:15:40  rpd]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
/*
 *	Mach standard external interface type definitions.
 *
 */

#ifndef	STD_TYPES_H_
#define	STD_TYPES_H_

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/vm_types.h>

#ifdef	MACH_KERNEL
#include <ipc/ipc_port.h>
#endif	/* MACH_KERNEL */

#endif	/* STD_TYPES_H_ */
