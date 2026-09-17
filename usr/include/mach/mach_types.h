/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: mach_types.h,v $
 * Revision 1.3.25.6  1995/02/24  14:48:24  alanl
 * 	Merged with DIPC2_SHARED.
 * 	[1995/01/04  14:10:43  alanl]
 *
 * Revision 1.3.33.2  1994/10/14  03:50:57  dwm
 * 	mk6 CR668 - 1.3b26 merge
 * 	ledgers, security, 64bit cleanup
 * 	[1994/10/14  03:42:51  dwm]
 * 
 * Revision 1.3.25.4  1994/09/23  02:39:31  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:41:22  ezf]
 * 
 * Revision 1.3.25.3  1994/08/16  16:22:21  joe
 * 	Added synchronizer user level port definitions
 * 	[1994/08/16  16:18:18  joe]
 * 
 * Revision 1.3.25.2  1994/08/07  20:49:41  bolinger
 * 	Merge up to colo_shared (post-b7, pre-MP).
 * 	[1994/08/07  18:35:24  bolinger]
 * 
 * Revision 1.3.25.1  1994/06/13  20:49:49  dlb
 * 	Merge MK6 and NMK17
 * 	[1994/06/13  20:48:03  dlb]
 * 
 * Revision 1.3.18.4  1994/07/08  20:09:01  dwm
 * 	mk6 CR227 - Add vm_behavior.
 * 	[1994/07/08  20:02:23  dwm]
 * 
 * Revision 1.3.18.3  1994/03/17  22:39:03  dwm
 * 	The infamous name change:  thread_activation + thread_shuttle = thread.
 * 	[1994/03/17  21:28:39  dwm]
 * 
 * Revision 1.3.18.2  1994/01/21  01:23:13  condict
 * 	Declare user_subsystem_t as char *.
 * 	[1994/01/21  01:21:31  condict]
 * 
 * Revision 1.3.18.1  1994/01/12  17:56:35  dwm
 * 	Coloc: initial restructuring to follow Utah model.
 * 	add act types, some backward compat. defines
 * 	[1994/01/12  17:30:41  dwm]
 * 
 * Revision 1.3.4.4  1993/10/05  22:23:10  watkins
 * 	Merge forward.
 * 	[1993/10/05  22:04:36  watkins]
 * 
 * Revision 1.3.11.2  1993/09/28  19:42:30  watkins
 * 	Added ledger_port_t. New include vm_region.h
 * 	Brezak's ledger version will supercede.
 * 
 * Revision 1.3.4.3  1993/08/03  19:09:56  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	CR9600: Change to export new *_port_* typedefs.
 * 	CR9601: Moved vm_address_t to std_types.h header file.
 * 	[1993/08/02  18:13:48  gm]
 * 
 * Revision 1.3.4.2  1993/06/09  02:41:48  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:17:09  jeffc]
 * 
 * Revision 1.3  1993/04/19  16:35:37  devrcs
 * 	Added include of mach/clock_types.h.
 * 	[1993/01/28            jat]
 * 
 * 	vm_sync changes
 * 	[1993/03/03  12:34:51  david]
 * 
 * 	ansi C conformance changes
 * 	[1993/02/02  18:53:20  david]
 * 
 * Revision 1.2  1992/12/07  21:29:23  robert
 * 	integrate any changes below for 14.0 (branch from 13.16 base)
 * 
 * 	Joseph Barrera (jsb) at Carnegie-Mellon University 05-Aug-92
 * 	Added inline_existence_map_t.
 * 	[1992/12/06  20:25:39  robert]
 * 
 * Revision 1.1  1992/09/30  02:31:20  robert
 * 	Initial revision
 * 
 * Revision 1.3.30.1  1994/08/04  02:28:16  mmp
 * 	xmm_obj.h moved from norma/ to xmm/
 * 	[1994/08/03  20:38:57  mmp]
 * 
 * Revision 1.3.22.2  1994/04/14  15:06:28  bernadat
 * 	Removed host_paging_t.
 * 	[94/04/13            bernadat]
 * 
 * Revision 1.3.22.1  1994/03/07  16:41:35  paire
 * 	Added <norma/xmm_user.h> import for prototypes.
 * 	[94/02/15            paire]
 * 
 * Revision 1.3.4.4  1993/10/05  22:23:10  watkins
 * 	Merge forward.
 * 	[1993/10/05  22:04:36  watkins]
 * 
 * Revision 1.3.11.2  1993/09/28  19:42:30  watkins
 * 	Added ledger_port_t. New include vm_region.h
 * 	Brezak's ledger version will supercede.
 * 
 * Revision 1.3.4.3  1993/08/03  19:09:56  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	CR9600: Change to export new *_port_* typedefs.
 * 	CR9601: Moved vm_address_t to std_types.h header file.
 * 	[1993/08/02  18:13:48  gm]
 * 
 * Revision 1.3.4.2  1993/06/09  02:41:48  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:17:09  jeffc]
 * 
 * Revision 1.3  1993/04/19  16:35:37  devrcs
 * 	Added include of mach/clock_types.h.
 * 	[1993/01/28            jat]
 * 
 * 	vm_sync changes
 * 	[1993/03/03  12:34:51  david]
 * 
 * 	ansi C conformance changes
 * 	[1993/02/02  18:53:20  david]
 * 
 * Revision 1.2  1992/12/07  21:29:23  robert
 * 	integrate any changes below for 14.0 (branch from 13.16 base)
 * 
 * 	Joseph Barrera (jsb) at Carnegie-Mellon University 05-Aug-92
 * 	Added inline_existence_map_t.
 * 	[1992/12/06  20:25:39  robert]
 * 
 * Revision 1.1  1992/09/30  02:31:20  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.8.2.3  92/09/15  17:23:39  jeffreyh
 * 	Added mach/prof_types.h
 * 	[92/07/24            bernadat]
 * 
 * Revision 2.8.2.2  92/06/24  18:05:31  jeffreyh
 * 	Added host_paging_t for NORMA_IPC
 * 	[92/06/17            jeffreyh]
 * 
 * Revision 2.8.2.1  92/02/21  11:24:01  jsb
 * 	NORMA_VM: define mach_xmm_obj_t and xmm_kobj_lookup().
 * 	[92/02/10  08:47:29  jsb]
 * 
 * Revision 2.8  91/06/25  10:30:20  rpd
 * 	Added KERNEL-compilation includes for *_array_t types.
 * 	[91/05/23            rpd]
 * 
 * Revision 2.7  91/06/06  17:08:07  jsb
 * 	Added emulation_vector_t for new get/set emulation vector calls.
 * 	[91/05/24  17:46:31  jsb]
 * 
 * Revision 2.6  91/05/14  16:55:17  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:33:43  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:18:38  mrt]
 * 
 * Revision 2.4  90/08/07  18:00:30  rpd
 * 	Added processor_set_name_array_t.
 * 	Removed vm_region_t, vm_region_array_t.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.3  90/06/02  14:58:42  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:33:59  rpd]
 * 
 * Revision 2.2  90/01/22  23:05:48  af
 * 	Added inclusion of vm_attributes.
 * 	[89/12/09            af]
 * 
 * 	Moved KERNEL type definitions into kern/mach_types_kernel.h, so
 * 	that changing them will not affect user programs.
 * 	[89/04/06            dbg]
 * 
 * 	Removed io_buf_t, io_buf_ptr_t in favor of device interface.
 * 	Removed include of ipc_netport.h.  Removed vm_page_data_t
 * 	(obsolete).
 * 	[89/01/14            dbg]
 * 
 * Revision 2.1  89/08/03  16:02:27  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:38:04  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.2  89/01/15  16:30:50  rpd
 * 	Moved from kern/ to mach/.
 * 	[89/01/15  14:35:53  rpd]
 * 
 * Revision 2.10  89/01/12  11:15:18  rpd
 * 	Removed pointer_t declaration; it belongs in std_types.h.
 * 
 * Revision 2.9  89/01/12  07:57:53  rpd
 * 	Moved basic stuff to std_types.h.  Removed debugging definitions.
 * 	Moved io_buf definitions to device_types.h.
 * 	[89/01/12  04:51:54  rpd]
 * 
 * Revision 2.8  89/01/04  13:37:34  rpd
 * 	Include <kern/fpa_counters.h>, for fpa_counters_t.
 * 	[89/01/01  15:03:52  rpd]
 * 
 * Revision 2.7  88/09/25  22:15:28  rpd
 * 	Changed sys/callout.h to kern/callout_statistics.h.
 * 	[88/09/09  14:00:19  rpd]
 * 	
 * 	Changed includes to the new style.
 * 	Added include of sys/callout.h.
 * 	[88/09/09  04:47:42  rpd]
 * 
 * Revision 2.6  88/08/06  18:22:34  rpd
 * Changed sys/mach_ipc_netport.h to kern/ipc_netport.h.
 * 
 * Revision 2.5  88/07/21  00:36:06  rpd
 * Added include of ipc_statistics.h.
 * 
 * Revision 2.4  88/07/17  19:33:20  mwyoung
 * *** empty log message ***
 * 
 * 29-Jun-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Use new <mach/memory_object.h>.
 *
 *  9-Apr-88  Daniel Julin (dpj) at Carnegie-Mellon University
 *	Changed mach_ipc_vmtp.h to mach_ipc_netport.h.
 *
 *  1-Mar-88  Mary Thompson (mrt) at Carnegie Mellon
 *	Added a conditional on _MACH_INIT_ before the include
 *	of mach_init.h so that the kernel make of mach_user_internal
 *	would not include mach_init.h
 *
 * 18-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Added includes of task_info, thread_info, task_special_ports,
 *	thread_special_ports for new interfaces.
 *
 * 12-Dec-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Reduced old history.
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
 *	File:	mach/mach_types.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1986
 *
 *	Mach external interface definitions.
 *
 */

#ifndef	_MACH_MACH_TYPES_H_
#define _MACH_MACH_TYPES_H_

#include <mach/host_info.h>
#include <mach/machine.h>
#include <mach/machine/vm_types.h>
#include <mach/memory_object.h>
#include <mach/port.h>
#include <mach/processor_info.h>
#include <mach/task_info.h>
#include <mach/task_special_ports.h>
#include <mach/thread_info.h>
#include <mach/thread_special_ports.h>
#include <mach/thread_status.h>
#include <mach/time_value.h>
#include <mach/clock_types.h>
#include <mach/vm_attributes.h>
#include <mach/vm_inherit.h>
#include <mach/vm_behavior.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>
#include <mach/vm_sync.h>
#include <mach/vm_region.h>
#include <mach/prof_types.h>

#ifdef	MACH_KERNEL
#include <kern/task.h>		/* for task_port_array_t */
#include <kern/thread.h>	/* for thread_port_array_t */
#include <kern/processor.h>	/* for processor_array_t,
				       processor_set_array_t,
				       processor_set_name_array_t */
#include <kern/syscall_emulation.h>
				/* for emulation_vector_t */
#include <kern/ledger.h>	/* for ledger_t */

#include <norma_vm.h>
#if	NORMA_VM
#include <xmm/xmm_obj.h>
typedef char		*inline_existence_map_t;
#endif	/* NORMA_VM */

#else	/* MACH_KERNEL */
typedef mach_port_t		task_port_t;
typedef	task_port_t		*task_port_array_t;
typedef mach_port_t		thread_port_t;
typedef	thread_port_t		*thread_port_array_t;
typedef mach_port_t		host_name_port_t;
typedef mach_port_t		processor_set_control_port_t;
typedef mach_port_t		*processor_port_array_t;
typedef mach_port_t		processor_set_name_port_t;
typedef mach_port_t		*processor_set_name_port_array_t;
typedef vm_offset_t		*emulation_vector_t;
typedef	mach_port_t		thread_act_t;
typedef	thread_act_t		*thread_act_array_t;
typedef mach_port_t		thread_act_port_t;
typedef	thread_act_port_t	*thread_act_port_array_t;
typedef mach_port_t		lock_set_port_t;
typedef mach_port_t		semaphore_port_t;
typedef mach_port_t		security_port_t;
typedef integer_t		ledger_item_t;
#endif	/* MACH_KERNEL */

typedef mach_port_t		*ledger_port_array_t;
typedef mach_port_t    		ledger_port_t;
typedef char			*user_subsystem_t;

/*
 *	Backwards compatibility, for those programs written
 *	before mach/{std,mach}_types.{defs,h} were set up.
 */
#include <mach/std_types.h>

#endif	/* _MACH_MACH_TYPES_H_ */
