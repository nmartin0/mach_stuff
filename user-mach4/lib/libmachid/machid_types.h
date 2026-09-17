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
 * $Log: machid_types.h,v $
 * Revision 1.1.1.1  1995/05/04  06:56:38  sclawson
 * New files.
 *
 * Revision 1.1  1994/09/29  22:24:54  mike
 * Initial revision
 *
 * Revision 2.4  93/04/14  11:47:38  mrt
 * 	Added sun4 support.
 * 	[93/01/20            berman]
 * 
 * Revision 2.3  93/04/09  13:48:51  mrt
 * 	64bit cleanup.
 * 	[92/12/02            af]
 * 
 * Revision 2.2  92/01/22  23:12:12  rpd
 * 	Moved to lib/libmachid/.
 * 	[92/01/22            rpd]
 * 
 * 	Moved ancillary definitions to machid_lib.h.
 * 	[92/01/22            rpd]
 * 
 * Revision 2.3  92/01/17  14:26:19  rpd
 * 	Changed vm_region_info_t to vm_region_t.
 * 	Added KERN_INVALID_MEMORY_OBJECT.
 * 	[92/01/02            rpd]
 * 	Conditionalized the definitions of KERN_INVALID_THREAD, etc.
 * 	[91/11/29            rpd]
 * 
 * Revision 2.2  91/08/29  15:05:27  rpd
 * 	Moved to the include directory.
 * 	[91/08/29            rpd]
 * 
 * 	Added mdefault_pager_t, MACH_TYPE_DEFAULT_PAGER,
 * 	KERN_INVALID_DEFAULT_PAGER.
 * 	[91/08/15            rpd]
 * 
 * Revision 2.3  91/03/19  12:31:06  mrt
 * 	Changed to new copyright
 * 
 * Revision 2.2  90/09/12  16:32:03  rpd
 * 	Added mprocessor_set_name_array_t.
 * 	[90/08/07            rpd]
 * 
 * 	Created.
 * 	[90/06/18            rpd]
 * 
 */

#ifndef	_MACHID_TYPES_H_
#define	_MACHID_TYPES_H_

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/task_info.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>

/* define types for machid_types.defs */

typedef unsigned int mach_id_t;
typedef unsigned int mach_type_t;

typedef mach_id_t mhost_t;
typedef mach_id_t mhost_priv_t;

typedef mach_id_t mdefault_pager_t;

typedef mach_id_t mprocessor_t;
typedef mprocessor_t *mprocessor_array_t;

typedef mach_id_t mprocessor_set_t;
typedef mprocessor_set_t *mprocessor_set_array_t;
typedef mach_id_t mprocessor_set_name_t;
typedef mprocessor_set_name_t *mprocessor_set_name_array_t;

typedef mach_id_t mtask_t;
typedef mtask_t *mtask_array_t;

typedef mach_id_t mthread_t;
typedef mthread_t *mthread_array_t;

typedef mach_id_t mobject_t;
typedef mach_id_t mobject_control_t;
typedef mach_id_t mobject_name_t;

typedef struct vm_region {
    vm_offset_t vr_address;
    vm_size_t vr_size;
/*vm_prot_t*/integer_t vr_prot;
/*vm_prot_t*/integer_t vr_max_prot;
/*vm_inherit_t*/integer_t vr_inherit;
/*boolean_t*/integer_t vr_shared;
/*mobject_name_t*/integer_t vr_name;
    vm_offset_t vr_offset;
} vm_region_t;

#ifdef	mips
#include <mach/mips/thread_status.h>

typedef struct mips_thread_state mips_thread_state_t;
#endif

#ifdef	sun3
#include <mach/sun3/thread_status.h>

typedef struct sun_thread_state sun3_thread_state_t;
#endif

#ifdef	sun4
#include <mach/sun4/thread_status.h>

typedef struct sparc_thread_state sparc_thread_state_t;
#endif	sun4

#ifdef	vax
#include <mach/vax/thread_status.h>

typedef struct vax_thread_state vax_thread_state_t;
#endif

#ifdef	i386
#include <mach/i386/thread_status.h>

typedef struct i386_thread_state i386_thread_state_t;
#endif

#ifdef	alpha
#include <mach/alpha/thread_status.h>

typedef struct alpha_thread_state alpha_thread_state_t;
#endif

#ifdef	parisc
#include <mach/parisc/thread_status.h>

typedef struct parisc_thread_state parisc_thread_state_t;
#endif

typedef int unix_pid_t;
typedef char *unix_command_t;

#endif	/* _MACHID_TYPES_H_ */
