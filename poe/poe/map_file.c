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
 * $Log:	map_file.c,v $
 * Revision 2.5  92/02/02  13:02:33  rpd
 * 	Fixed argument types in map_file_data_provided.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.4  91/12/19  20:28:32  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:55:15  rwd
 * 	Enable locking.
 * 	[90/09/11            rwd]
 * 
 * Revision 2.2  90/09/08  00:19:16  rwd
 * 	Include cthread_self in debugging output.
 * 	[90/09/04            rwd]
 * 	First checkin
 * 	[90/08/31  13:42:43  rwd]
 * 
 */
/*
 *	File:	./map_file.c
 *	Author:	Randall W. Dean
 *
 *	Copyright (c) 1990 Randall W. Dean
 */


#include <mach.h>
#include <mach/message.h>
#include <map_info.h>

#define MAP_FILE_MIN_SIZE 64 * 1024

int map_file_debug = 0;
#define PD(flag,format) if (map_file_debug&flag) {printf("[%x]",cthread_self());printf/**/format/**/;} else
/*
 *	1	remap
 *	2	read
 *	4	data_provided
*/

#define	MF_LOCK(mi)	mutex_lock(&((mi)->mi_lock))
#define	MF_UNLOCK(mi)	mutex_unlock(&((mi)->mi_lock))

map_file_lock(mi)
map_info_t	mi;
{
	MF_LOCK(mi);
}

map_file_unlock(mi)
map_info_t	mi;
{
	MF_UNLOCK(mi);
}

map_file_init(mi)
map_info_t	mi;
{
	bzero(mi, sizeof(struct map_info));
}

map_file_remap(mi, offset, size)
map_info_t	mi;
vm_offset_t	offset;
vm_size_t	size;
{
	int old_address = 0;
	int old_map_size = 0;
	kern_return_t error;
	int prot;

	PD(1,("map_file_remap(%x, %x, %x)\n",mi, offset, size));
	if (mi->mi_map_size) {
	    old_address = mi->mi_address;
	    old_map_size = mi->mi_map_size;
	}
	mi->mi_address = 0;
	mi->mi_offset = trunc_page(offset);
	mi->mi_map_size = round_page(offset+size) - mi->mi_offset;
	if (mi->mi_map_size < MAP_FILE_MIN_SIZE)
	    mi->mi_map_size = MAP_FILE_MIN_SIZE;
	prot = VM_PROT_READ;
	if (mi->mi_write) prot |= VM_PROT_WRITE;
	error = vm_map(mach_task_self(), &mi->mi_address, mi->mi_map_size, 0,
		       TRUE, mi->mi_pager, mi->mi_offset, FALSE,
		       prot, prot, VM_INHERIT_NONE);
	if (error) {
	    printf("map_file_remap: vm_map: size = %x, pager = %x, offset = %x, status = %x\n",mi->mi_map_size, mi->mi_pager, mi->mi_offset, error);
	}
	if (old_address) {
	    vm_deallocate(mach_task_self(), old_address, old_map_size);
	}
	return error;
}

map_file_read(mi, buf, offset, size)
map_info_t	mi;
char		*buf;
vm_offset_t	offset;
vm_size_t	size;
{
	int error;

	PD(2,("map_file_read(%x, %x, %x, %x)\n",mi, buf, offset, size));
	MF_LOCK(mi);
	if (offset < mi->mi_offset ||
	    offset + size > mi->mi_offset + mi->mi_map_size) {
	    error = map_file_remap(mi, offset, size);
	    if (error) {
		printf("map_file_read: map_file_remap %d\n",error);
		return error;
	    }
	}
	PD(8,("map_file_read: %x %x %x\n",mi->mi_address, mi->mi_offset, mi->mi_map_size));
	bcopy(mi->mi_address - mi->mi_offset + offset, buf, size);
	MF_UNLOCK(mi);
	return 0;
}

map_file_write(mi, buf, offset, size)
map_info_t	mi;
char		*buf;
vm_offset_t	offset;
vm_size_t	size;
{
	int error;

	PD(16,("map_file_write(%x, %x, %x, %x)\n",mi, buf, offset, size));
	MF_LOCK(mi);
	if (offset < mi->mi_offset ||
	    offset + size > mi->mi_offset + mi->mi_map_size) {
	    error = map_file_remap(mi, offset, size);
	    if (error) {
		printf("map_file_write: map_file_remap %d\n",error);
		return error;
	    }
	}
	PD(32,("map_file_write: %x %x %x\n",mi->mi_address, mi->mi_offset, mi->mi_map_size));
	bcopy(buf, mi->mi_address - mi->mi_offset + offset, size);
	MF_UNLOCK(mi);
	return 0;
}

map_file_data_provided(mi, control, offset, obj_offset, size, prot)
map_info_t	mi;
memory_object_t	control;
vm_offset_t	offset;
vm_offset_t	obj_offset;
vm_size_t	size;
vm_prot_t	prot;
{
	vm_offset_t buffer;
	kern_return_t error;

	PD(4,("map_file_data_provided(%x, %x, %x, %x, %x, %x)\n", mi, control, offset, obj_offset, size, prot));
	error = vm_allocate(mach_task_self(), &buffer, size, TRUE);
	if (error) {
		printf("map_file_data_provided: allocate %d\n",error);
		return error;
	}
	error = map_file_read(mi, buffer, obj_offset, size);
	if (error) {
		printf("map_file_data_provided: map_file_read %d\n",error);
		return error;
	}
	error=memory_object_data_provided(control, offset, buffer, size, prot);
	vm_deallocate(mach_task_self(), buffer, size);
	return error;
}
