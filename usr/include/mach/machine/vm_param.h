/*
 * @OSF_FREE_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: vm_param.h,v $
 * Revision 1.1.27.3  1997/06/10  14:20:10  bruel
 * 	kernel HP ELF support.
 * 	[97/06/10            bruel]
 *
 * Revision 1.1.27.2  1997/05/20  14:13:40  bruel
 * 	export VM_MAX_KERNEL_LOADED_ADDRESS.
 * 	[97/05/20            bruel]
 * 
 * Revision 1.1.27.1  1997/02/27  09:42:04  bruel
 * 	linux-PA: Changed address of the server to be 0x1fe00000 when kloaded.
 * 	(the elf loader hardcodes the data section at 0x2000000).
 * 	[97/02/27            bruel]
 * 
 * Revision 1.1.18.5  1996/07/31  09:57:01  paire
 * 	Merged with nmk20b7_shared (1.1.24.1)
 * 	[96/06/10            paire]
 * 
 * Revision 1.1.24.1  1996/05/10  16:38:33  ferranti
 * 	Increase KERNEL_STACK_SIZE for NORMA/X_KERNEL usage.
 * 	[1996/05/10  16:30:00  ferranti]
 * 
 * Revision 1.1.18.4  1996/02/02  12:19:44  emcmanus
 * 	Copied from nmk20b5_shared.
 * 	[1996/02/01  16:59:42  emcmanus]
 * 
 * Revision 1.1.21.2  1996/01/17  09:48:36  paire
 * 	CR1814: KERNEL_STACK_SIZE increased to 8Kbytes.
 * 	[96/01/15            paire]
 * 
 * Revision 1.1.21.1  1995/12/30  17:12:49  emcmanus
 * 	Defined hp700_btop, hp700_ptob, machine_btop.
 * 	[1995/12/30  17:04:36  emcmanus]
 * 
 * Revision 1.1.18.3  1995/11/02  14:59:33  bruel
 * 	Protect the page 0.
 * 	To access it with the debugger, set kgdb_validate = FALSE.
 * 	[95/10/12            bruel]
 * 
 * Revision 1.1.18.2  1995/04/19  16:19:20  bruel
 * 	Since page 0 is used (PAGE0 for rtclock) set
 * 	VM_MIN_KERNEL_ADDRESS to 0. Otherwise gdb
 * 	gets confused when failing to access this page.
 * 	[95/01/17            bernadat]
 * 	[95/04/19            bruel]
 * 
 * Revision 1.1.18.1  1995/03/15  17:47:36  bruel
 * 	hppa merge
 * 	[1995/03/15  09:45:39  bruel]
 * 
 * Revision 1.1.4.4  1994/08/29  15:44:39  bruel
 * 	Added UNIX_MAPBASE_DEFAULT.
 * 	[94/08/29            bruel]
 * 
 * Revision 1.1.4.3  1994/06/17  10:04:55  bruel
 * 	push VM_MAX_KERNEL_ADDRESS to the highest.
 * 	[94/06/17            bruel]
 * 
 * Revision 1.1.4.2  1994/06/06  05:54:00  bernadat
 * 	Define stacks sizes for MACH_KERNEL only.
 * 	[94/06/01            bernadat]
 * 
 * Revision 1.1.4.1  1994/05/31  14:57:09  bruel
 * 	increase KERNEL_STACK_SIZE for tgdb task.
 * 	[94/05/31            bruel]
 * 
 * Revision 1.1.2.4  1994/05/06  21:29:17  bruel
 * 	set the KERNEL_STACK_SIZE to 1 page to allow dynamic allocation (see pcb.c)
 * 	[94/05/06            bruel]
 * 
 * Revision 1.1.2.3  1994/05/05  16:55:56  bruel
 * 	added USER_STACK_END.
 * 	     set VM_MAX_ADDRESS to 0xfffff000 for video memory mapping.
 * 	[94/05/04            bruel]
 * 
 * Revision 1.1.2.2  1994/03/16  14:37:21  bruel
 * 	Fixed stack allocation. hp800 -> hp700.
 * 	[94/02/15            bruel]
 * 
 * Revision 1.1.2.1  1994/02/11  13:28:34  bruel
 * 	Created from Utah.
 * 	[93/11/19            bruel]
 * 
 * $EndLog$
 */
/* 
 * Copyright (c) 1990, 1991, 1992, The University of Utah and
 * the Center for Software Science at the University of Utah (CSS).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the Center
 * for Software Science at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSS ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSS DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSS requests users of this software to return to css-dist@cs.utah.edu any
 * improvements that they make and grant CSS redistribution rights.
 *
 * 	Utah $Hdr: vm_param.h 1.10 92/05/22$
 */

#ifndef	_MACH_HP700_VM_PARAM_H_
#define _MACH_HP700_VM_PARAM_H_

#define BYTE_SIZE	8	/* byte size in bits */

#define HP700_PGBYTES	4096	/* bytes per hp700 page */
#define HP700_PGSHIFT	12	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define hp700_btop(x)		(((unsigned)(x)) >> HP700_PGSHIFT)
#define machine_btop(x)		hp700_btop(x)
#define hp700_ptob(x)		(((unsigned)(x)) << HP700_PGSHIFT)

#define VM_MIN_ADDRESS	((vm_offset_t) 0)
#define VM_MAX_ADDRESS	((vm_offset_t) 0xfffff000U)
#define USER_STACK_END  ((vm_offset_t) 0xc0000000U)

#define hp700_round_page(x)	((((unsigned)(x)) + HP700_PGBYTES - 1) & \
					~(HP700_PGBYTES-1))
#define hp700_trunc_page(x)	(((unsigned)(x)) & ~(HP700_PGBYTES-1))

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x1000)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xef000000)

#if	MACH_KERNEL
#include <norma_vm.h>
#include <xkmachkernel.h>

#if	!NORMA_VM && !XKMACHKERNEL
#if	DEBUG
#define KERNEL_STACK_SIZE	(2 * HP700_PGBYTES) /* -g ==> larger frames */
#else	/* MACH_ASSERT */
#define KERNEL_STACK_SIZE	(1 * HP700_PGBYTES)
#endif	/* MACH_ASSERT */
#else	/* !NORMA_VM  && !XKMACHKERNEL */
#define KERNEL_STACK_SIZE	(8 * HP700_PGBYTES)
#endif	/* !NORMA_VM && !XKMACHKERNEL */
#define INTSTACK_SIZE		(5 * HP700_PGBYTES)
#endif	/* MACH_KERNEL */

#ifndef ASSEMBLER
#define VM_MIN_KERNEL_LOADED_ADDRESS	((vm_offset_t)  0x1FE00000U)
#else /* ASSEMBLER */
#define VM_MIN_KERNEL_LOADED_ADDRESS	0x1FE00000
#endif /* ASSEMBLER */

#define VM_MAX_KERNEL_LOADED_ADDRESS   (VM_MIN_KERNEL_LOADED_ADDRESS + (512 * 1024 * 1024))

#endif	/* _HP700_VM_PARAM_H_ */





