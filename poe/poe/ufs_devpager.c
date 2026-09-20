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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	ufs_devpager.c,v $
 * Revision 2.4  92/02/02  13:03:01  rpd
 * 	Removed old IPC vestiges.
 * 	Fixed cthread_fork argument types.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.3  91/12/19  20:29:47  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:21:23  rwd
 * 	Use port_allocate_name call.
 * 	[90/09/03            rwd]
 * 	Convert to new cthread semantics.
 * 	[90/08/16            rwd]
 * 
 * 	For now, wire cthreads.
 * 	[90/07/16            rwd]
 * 
 */
/*
 *	File:	./ufs_devpager.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <ufs_fops.h>
#include <device/device_types.h>
#include <sys/types.h>
#include <sys/stat.h>

extern void ufs_devpager_cthread_busy();
extern void ufs_devpager_cthread_active();

/*
 *  Rename to avoid name conflict with other pagers in same task.
 */
#define	memory_object_init		udev_memory_object_init
#define	memory_object_terminate		udev_memory_object_terminate
#define	memory_object_copy		udev_memory_object_copy
#define	memory_object_data_request	udev_memory_object_data_request
#define	memory_object_data_unlock	udev_memory_object_data_unlock
#define	memory_object_data_write	udev_memory_object_data_write
#define	memory_object_lock_completed	udev_memory_object_lock_completed
#define	memory_object_server		udev_memory_object_server

extern struct udev *ud_by_memory_object();

memory_object_init(memory_object, memory_control, memory_object_name,
		   page_size)
	mach_port_t memory_object;
	mach_port_t memory_control;
	mach_port_t memory_object_name;
	vm_size_t page_size;
{
	struct udev *ud;
	int error;

	if (memory_object_name != MACH_PORT_NULL) {
		mach_port_deallocate(mach_task_self(), memory_object_name);
	}
	if (memory_control == MACH_PORT_NULL) {
		printf("memory_object_init: invalid memory_control\n");
	    	return;
	}
	if (page_size != vm_page_size) {
		printf("memory_object_init: invalid page size\n");
		return;
	}
	ud = ud_by_memory_object(memory_object);
	if (! ud) {
		printf("memory_object_init: invalid memory_object\n");
		return;
	}
	error = memory_object_set_attributes(memory_control, TRUE, TRUE,
					     MEMORY_OBJECT_COPY_DELAY);
	if (error) {
		printf("memory_object_init: set_attributes: %d\n", error);
	}
}

memory_object_data_request(memory_object, memory_control, offset, length,
			   desired_access)
	mach_port_t memory_object, memory_control;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	struct udev *ud;
	int error;
	vm_offset_t data;
	vm_size_t dsize;

	ud = ud_by_memory_object(memory_object);
	if (! ud) {
		printf("data_request: invalid memory_object\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	if ((offset % vm_page_size) != 0) {
		printf("data_request: unaligned offset\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	if (length != vm_page_size) {
		printf("data_request: size not vm_page_size\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	if ((desired_access & VM_PROT_WRITE) != 0) {
		printf("data_request: requesting write!\n");
		memory_object_data_error(memory_control, offset, length,
					 KERN_INVALID_ARGUMENT);
		return;
	}
	ufs_devpager_cthread_busy();
	error = bdev_device_read(ud->ud_special, 0,
				 offset / DEV_BSIZE /* XXX */,
				 vm_page_size, &data, &dsize);
	ufs_devpager_cthread_active();
	if (error) {
		printf("data_request: error %d from device_read\n", error);
		memory_object_data_error(memory_control, offset, length,
					 error);
		return;
	}
#if 0
/*
 * This is alright since it will always pass back a zero-filled full page
 */
	if (dsize != vm_page_size) {
		printf("data_request: dsize=%d, vm_page_size=%d\n",
		       dsize, vm_page_size);
		memory_object_data_error(memory_control, offset, length,
					 error);
		return;
	}
#endif 0
	memory_object_data_provided(memory_control, offset, data, vm_page_size,
				    VM_PROT_WRITE);
	vm_deallocate(mach_task_self(), data, vm_page_size);
}

memory_object_data_unlock(memory_object, memory_control, offset, length,
			  desired_access)
	mach_port_t memory_object;
	mach_port_t memory_control;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	printf("ufs_devpager: data_unlock called!\n");
	memory_object_data_error(memory_control, offset, length,
				 KERN_INVALID_ARGUMENT);
}

memory_object_data_write(memory_object, memory_control, offset, data, length)
	mach_port_t memory_object;
	mach_port_t memory_control;
	vm_offset_t offset;
	char *data;
	int length;
{
	printf("ufs_devpager: data_write called!\n");
}

memory_object_lock_completed(memory_object, memory_control, offset, length)
	mach_port_t memory_object;
	mach_port_t memory_control;
	vm_offset_t offset;
	vm_size_t length;
{
	printf("ufs_devpager: lock_completed called!\n");
}

memory_object_copy(old_memory_object, old_memory_control, offset, length,
		   new_memory_object)
	memory_object_t old_memory_object;
	memory_object_control_t old_memory_control;
	vm_offset_t offset;
	vm_size_t length;
	memory_object_t new_memory_object;
{
	printf("ufs_devpager: copy called!\n");
}

memory_object_terminate(memory_object, memory_control, memory_object_name)
	mach_port_t memory_object;
	mach_port_t memory_control;
	mach_port_t memory_object_name;
{
	printf("ufs_devpager: terminate called...\n");
}

mach_port_t	ufs_devpager_port_set		= MACH_PORT_NULL;
int		ufs_devpager_kthread_min	= 1;
int		ufs_devpager_kthread_max	= 2;
int		ufs_devpager_cthread_min	= 1;
int		ufs_devpager_cthread_current	= 0;
struct mutex	ufs_devpager_cthread_mutex	= MUTEX_INITIALIZER;
extern int	ufs_devpager_loop();

int ufs_devpager_init()
{
	kern_return_t error;

	error = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
		&ufs_devpager_port_set);
	if (error) {
		mach_error("ufs_devpager_init: port_allocate", error);
		panic("ufs_devpager_init");
	}
	cthread_set_kernel_limit(cthread_kernel_limit() + ufs_devpager_kthread_max);
	cthread_detach(cthread_fork((any_t (*)()) ufs_devpager_loop,
				    (any_t) 0));
}

void
ufs_devpager_cthread_busy()
{
	cthread_msg_busy(ufs_devpager_port_set,
			 ufs_devpager_kthread_min,
			 ufs_devpager_kthread_max);
	mutex_lock(&ufs_devpager_cthread_mutex);
	if (--ufs_devpager_cthread_current < ufs_devpager_cthread_min) {
	    mutex_unlock(&ufs_devpager_cthread_mutex);
	    cthread_detach(cthread_fork((any_t (*)()) ufs_devpager_loop,
					(any_t) 0));
	} else {
	    mutex_unlock(&ufs_devpager_cthread_mutex);
	}
}

void
ufs_devpager_cthread_active()
{
	cthread_msg_active(ufs_devpager_port_set,
			   ufs_devpager_kthread_min,
			   ufs_devpager_kthread_max);
	mutex_lock(&ufs_devpager_cthread_mutex);
	++ufs_devpager_cthread_current;
	mutex_unlock(&ufs_devpager_cthread_mutex);
}

int
ufs_devpager_loop()
{
	struct ux_task *ut;

	kern_return_t ret;
	union request_msg {
	    mach_msg_header_t	hdr;
	    mig_reply_header_t	death_pill;
	    char		space[8192];
	} msg_buffer, out;

	char	name[64];

	sprintf(name, "ufs_devpager thread %x", cthread_self());
	cthread_set_name(cthread_self(), name);

	ufs_devpager_cthread_active();
	do {
	    ret = cthread_mach_msg(&msg_buffer, MACH_RCV_MSG,
				   0, sizeof msg_buffer, ufs_devpager_port_set,
				   MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL,
				   ufs_devpager_kthread_min,
				   ufs_devpager_kthread_max);
	    if (ret != MACH_MSG_SUCCESS)
		panic("ufs_devpager_loop: receive", ret);
	    if (!memory_object_server(&msg_buffer, &out))
		{}

	} while (1);
}

ufs_devmap(ud)
	struct udev *ud;
{
	int error;

	error = mach_port_allocate_name(mach_task_self(),
		MACH_PORT_RIGHT_RECEIVE,
		(mach_port_t)ud);
	if (error) {
		return error;
	}
	ud->ud_map_info.mi_pager = (mach_port_t) ud;
	error = mach_port_insert_right(mach_task_self(),
				       ud->ud_map_info.mi_pager,
				       ud->ud_map_info.mi_pager,
				       MACH_MSG_TYPE_MAKE_SEND);
	if (error) {
		return error;
	}
	error = mach_port_move_member(mach_task_self(),
				      ud->ud_map_info.mi_pager,
				      ufs_devpager_port_set);
	if (error) {
		mach_error("ufs_devmap: port_move_member", error);
		return error;
	}
	return KERN_SUCCESS;
}
