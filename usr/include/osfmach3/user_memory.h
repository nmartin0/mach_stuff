/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: user_memory.h,v $
 * Revision 1.1.2.3  1996/11/18  18:15:56  barbou
 * 	Changed the prototypes for user_memory_flush_task and
 * 	user_memory_flush_area: they deal with Mach tasks not Linux tasks.
 * 	[1996/11/18  17:14:39  barbou]
 *
 * Revision 1.1.2.2  1996/09/11  14:19:21  barbou
 * 	Added prototype for user_memory_get_max_filename.
 * 	[96/09/11            barbou]
 * 
 * Revision 1.1.2.1  1996/09/09  16:58:57  barbou
 * 	Moved task_struct_t type definition to osfmach3/sched.h.
 * 	[96/09/02            barbou]
 * 
 * 	Added prototype for get_phys_addr().
 * 	[96/08/21            barbou]
 * 
 * 	Added prototype for user_memory_verify_area().
 * 	[96/08/21            barbou]
 * 
 * 	Created.
 * 	[1996/08/21  15:12:24  barbou]
 * 
 * $EndLog$
 */

#ifndef	_OSFMACH3_USER_MEMORY_H_
#define _OSFMACH3_USER_MEMORY_H_

#include <mach/mach_types.h>

#include <osfmach3/queue.h>
#include <osfmach3/macro_help.h>

#include <linux/types.h>
#include <linux/sched.h>

typedef struct user_memory *user_memory_t;

extern user_memory_t user_memory_slow_lookup(task_struct_t task,
					     vm_address_t user_addr,
					     vm_size_t size,
					     vm_prot_t prot,
					     vm_address_t *svr_addrp);
extern void user_memory_unlock(user_memory_t);
extern void user_memory_task_init(task_struct_t task);
extern void user_memory_flush_task(osfmach3_mach_task_struct_t mach_task);
extern void user_memory_flush_area(osfmach3_mach_task_struct_t mach_task,
				   vm_address_t start,
				   vm_address_t end);
extern void user_memory_lookup(task_struct_t task,
			       vm_address_t addr,
			       vm_size_t size,
			       vm_prot_t prot,
			       vm_address_t *svr_addrp,
			       user_memory_t *ump);
extern int user_memory_verify_area(int type,
				   const void *addr,
				   unsigned long size);
extern int user_memory_get_max_filename(unsigned long address);
extern unsigned long get_phys_addr(task_struct_t p,
				   unsigned long ptr,
				   user_memory_t *um);

#define user_memory_task_terminate(task)				\
	user_memory_flush_task(task)

#endif	/* _OSFMACH3_USER_MEMORY_H_ */
