/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
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
