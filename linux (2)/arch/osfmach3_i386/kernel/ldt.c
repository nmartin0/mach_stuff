/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <linux/kernel.h>
#include <linux/errno.h>

asmlinkage int sys_modify_ldt(int func, void *ptr, unsigned long bytecount)
{
	printk("sys_modify_ldt not implemented\n");
	return -ENOSYS;
}
