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
 * $Log:	subr_copy.c,v $
 * Revision 2.5  94/03/25  18:23:12  mrt
 * 	Added some comments.
 * 	[94/03/16            mrt]
 * 
 * Revision 2.4  92/02/02  13:02:40  rpd
 * 	Fixed vm_read argument types.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.3  91/12/19  20:29:09  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:20:15  rwd
 * 	First checkin
 * 	[90/08/31  13:55:35  rwd]
 * 
 */
/*
 *	File:	./subr_copy.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <ux_user.h>
#include <errno.h>
#include <setjmp.h>

#define	PRINT_COPIES	0

/* a page descriptor, for multiple vm_reads in copyin_string */
struct pd {
	vm_offset_t	address;
	struct pd	*next;
};

#define	kr_check(s, kr) if (kr) { mach_error(s, kr); return EFAULT; }

/*  copyin(ut, address, size, s_address)
 * 	copies "size" bytes from "address" in task "ut" to
 *	"s_address" in current address space
 */

copyin(ut, address, size, s_address)
	struct ux_task *ut;
	vm_offset_t address;
	vm_size_t size;
	vm_offset_t *s_address;
{
	vm_offset_t data;
	vm_size_t offset, data_count;
	kern_return_t kr;

#if	PRINT_COPIES
printf("<copyin %d\n", size);
#endif
	offset = address - trunc_page(address);
	address -= offset;
	size += offset;
	size = round_page(size);
	kr = vm_read(ut->ut_task, address, size,
		     &data, (unsigned int *) &data_count);
	kr_check("copyin.vm_read", kr);
	*s_address = data + offset;
	return 0;
}

/*  uncopyin(address, size)
 *	vm_deallocates the page(s) in the local address space
 *	that contains the  "size" bytes starting at "address"
 */
uncopyin(address, size)
	vm_offset_t address;
	vm_size_t size;
{
	vm_size_t offset;
	int kr;

	offset = address - trunc_page(address);
	address -= offset;
	size += offset;
	size = round_page(size);
	kr = vm_deallocate(mach_task_self(), address, size);
	if (kr) {
		mach_error("uncopyin.vm_deallocate.", kr);
	}
}

/*
 * copyin_string(ut, address, s_address, sizep)
 *	Copies a string from "adress" in task "ut" address
 *	space to "s_address" in the current address space.
 *	The size of the string must be determined by looking
 *	for the trailing null. The string size is returned in
 *	"* sizep".
 */
copyin_string(ut, address, s_address, sizep)
	struct ux_task *ut;
	vm_offset_t address;
	vm_offset_t *s_address;
	vm_size_t *sizep;
{
	int j;
	vm_size_t offset, start, data_count;
	kern_return_t kr;
	struct pd *pd = 0, *pdhead = 0;

#if	PRINT_COPIES
printf("<copyin_string\n");
#endif
	start = offset = address - trunc_page(address);
	address -= offset;
	*sizep = 0;
	for (;;) {
		if (pd) {
			pd = pd->next = (struct pd *) malloc(sizeof(*pd));
		} else {
			pd = pdhead = (struct pd *) malloc(sizeof(*pd));
		}
		pd->next = 0;
		*sizep += vm_page_size;
		kr = vm_read(ut->ut_task, address, vm_page_size,
			     &pd->address, (unsigned int *) &data_count);
		kr_check("copyin_string.vm_read", kr);
		for (j = start; j < vm_page_size; j++) {
			if (((char *)pd->address)[j] == '\0') {
				goto found_null;
			}
		}
		start = 0;
	}
found_null:
	/* ensure that our pages are contiguous */
	/* xxx should vm_copy if not! */
	for (pd = pdhead; pd && pd->next; pd = pd->next) {
		if (pd->address + vm_page_size != pd->next->address) {
			printf("copy_string: 0x%x+0x%x != 0x%x\n",
				pd->address, vm_page_size, pd->next->address);
			return EFAULT;
		}
	}
	*s_address = pdhead->address + offset;
	return 0;
}

/* copyout(ut, data, address, size)
 *	Copies "size" bytes from "data" in the current address
 *	space to "address" in the "ut" task's addres space.
 */
copyout(ut, data, address, size)
	struct ux_task *ut;
	vm_offset_t data;
	vm_offset_t address;
	vm_size_t size;
{
	kern_return_t kr;
	vm_offset_t tmp_data;
	vm_offset_t trunc_address, trunc_offset;
	vm_size_t count, trunc_size;

#if	PRINT_COPIES
printf(">copyout %d\n", size);
#endif
	/*
	 *  Write beginning partial page, if any.
	 */
	trunc_address = trunc_page(address);
	trunc_offset = address - trunc_address;
	if (trunc_offset) {
		if (size > vm_page_size - trunc_offset) {
			trunc_size = vm_page_size - trunc_offset;
		} else {
			trunc_size = size;
		}
		kr = vm_read(ut->ut_task, trunc_address, vm_page_size,
			&tmp_data, (unsigned int *) &count);
		kr_check("copyout1.vm_read", kr);
		bcopy(data, tmp_data + trunc_offset, trunc_size);
		kr = vm_write(ut->ut_task, trunc_address, tmp_data,
			vm_page_size);
		kr_check("copyout1.vm_write", kr);
		kr = vm_deallocate(mach_task_self(), tmp_data, vm_page_size);
		kr_check("copyout1.vm_deallocate", kr);
		size -= trunc_size;
		data += trunc_size;
		address += trunc_size;
	}

#if 0
	/*
	 * Write any whole middle pages.
	 */
	if (size >= vm_page_size) {
		vm_size_t tsize = size & ~(vm_page_size - 1);
		kr = vm_write(ut->ut_task, address, data, tsize);
		kr_check("copyout2.vm_write", kr);
		size -= tsize;
		data += tsize;
		address += tsize;
	}
#endif

	/*
	 * Write ending partial page, if any.
	 */
	while (size > 0) {
		vm_size_t tsize = (size < vm_page_size ? size : vm_page_size);
		kr = vm_read(ut->ut_task, address, vm_page_size,
			     &tmp_data, (unsigned int *) &count);
		kr_check("copyout3.vm_read", kr);
		bcopy(data, tmp_data, tsize);
		kr = vm_write(ut->ut_task, address, tmp_data, vm_page_size);
		kr_check("copyout3.vm_write", kr);
		kr = vm_deallocate(mach_task_self(), tmp_data, vm_page_size);
		kr_check("copyout3.vm_deallocate", kr);
		size -= tsize;
		data += tsize;
		address += tsize;
	}
	return 0;
}
