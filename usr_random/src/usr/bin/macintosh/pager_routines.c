/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: emul/server/pager_routines.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include "server_defs.h"
#include "pager_defs.h"

memory_object_control_t	memory_object_control;
vm_size_t		memory_object_page_size;

kern_return_t
memory_object_init(memory_object_t	        object,
		   memory_object_control_t	control,
		   memory_object_name_t		name,
		   vm_size_t			page_size)
{
#ifdef notdef
    printf("memory_object_init: object %x page_size %d\n", object, page_size);
#endif

    if (object == memory_object) {
	if (page_size != vm_page_size)
	    other_error_exit("bad memory object page size");

	memory_object_control = control;
	memory_object_page_size = page_size;
	(void) memory_object_set_attributes(control,
					    TRUE,
					    FALSE,
					    MEMORY_OBJECT_COPY_DELAY);
    }

    return (KERN_SUCCESS);
}

static inline
void
supply_data(pr, offset, length, perm, written)
paging_region_ptr_t	pr;
vm_offset_t		offset;
vm_size_t		length;
vm_prot_t		perm;
boolean_t		written;
{
    register paging_obj_ptr_t	po;
    vm_offset_t			buffer;
    kern_return_t		result;

    if (written)
	po = &pr->w_paging_obj;
    else
	po = &pr->i_paging_obj;

    switch (po->type) {
      case PAGING_OBJ_TYPE_NULL:
	ASSERT(!written);
	(void) memory_object_data_unavailable(memory_object_control,
					      offset,
					      length);
	break;

      case PAGING_OBJ_TYPE_FILE:
	(void) vm_allocate(mach_task_self(), &buffer, length, TRUE);
	/* XXX check for failure */

	(void) lseek((int)po->data,
		     po->offset + (offset - pr->object_offset),
		     L_SET);
	/* XXX check for failure */

	if (read((int)po->data, buffer, length) < 0)
	    unix_error_exit("supply data read file");

#ifdef NO_DATA_SUPPLY
	(void) memory_object_data_provided(memory_object_control,
					   offset,
					   buffer, length,
					   (~pr->max_permission&VM_PROT_ALL));
	(void) vm_deallocate(mach_task_self(), buffer, length);
#else
	(void) memory_object_data_supply(memory_object_control,
					 offset,
					 buffer, length,
					 TRUE,
					 (~pr->max_permission&VM_PROT_ALL),
					 FALSE,
					 PORT_NULL);
#endif
	break;

      default:
	fprintf(stderr, "unknown object type %x\n", po->type);
	(void) memory_object_data_error(memory_object_control,
					offset,
					length,
					KERN_INVALID_ADDRESS);
    }
}

kern_return_t
memory_object_data_request(memory_object_t		object,
			   memory_object_control_t	control,
			   vm_offset_t			offset,
			   vm_size_t			length,
			   vm_prot_t			permission)
{
    register paging_region_ptr_t	pr;
    register boolean_t			written;
    register unsigned int		page;

#ifdef notdef
    printf("memory_object_data_request: offset %x length %d perm %x\n",
	   offset,
	   length,
	   permission);
#endif

    if (object != memory_object	|| control != memory_object_control)
	return (KERN_SUCCESS);

    if (length > memory_object_page_size) {
	(void) memory_object_data_error(control,
					offset,
					length,
					KERN_MEMORY_ERROR);
    }
    else {
	pr = pager_lookup_region(offset, length);
	if (pr == PAGING_REGION_PTR_NULL)
	    (void) memory_object_data_error(control,
					    offset,
					    length,
					    KERN_INVALID_ADDRESS);
	else {
	    written = page_check_written(pr->w_paging_map,
					 (offset - pr->object_offset)/
					 memory_object_page_size);
	    supply_data(pr,
			offset,
			length,
			permission,
			written);
	}
    }

    return (KERN_SUCCESS);
}

kern_return_t
memory_object_data_write(memory_object_t		object,
			 memory_object_control_t	control,
			 vm_offset_t			offset,
			 pointer_t			data,
			 unsigned int			length)
{
    register paging_region_ptr_t	pr;
    register paging_obj_ptr_t		po;
    register unsigned int		page, n;

#ifdef notdef
    printf("memory_object_data_write: offset %x length %x\n", offset, length);
#endif

    if (object != memory_object || control != memory_object_control)
	return (KERN_SUCCESS);

    pr = pager_lookup_region(offset, length);
    if (pr != PAGING_REGION_PTR_NULL) {
	po = &pr->w_paging_obj;

	ASSERT(po->type == PAGING_OBJ_TYPE_FILE);

	(void) lseek((int)po->data,
		     po->offset + (offset - pr->object_offset),
		     L_SET);

	if (write((int)po->data, data, length) < 0)
	    unix_error_exit("memory_object_data_write: write");

	page = (offset - pr->object_offset)/memory_object_page_size;
	for (n = length; n > 0; page++, n -= memory_object_page_size)
	    page_set_written(pr->w_paging_map, page);
    }
    else
	for (n = 0; n < length; n += memory_object_page_size) {
	    pr = pager_lookup_region(offset + n, memory_object_page_size);
	    if (pr == PAGING_REGION_PTR_NULL)
		other_error_exit("memory_object_data_write: no region");

	    po = &pr->w_paging_obj;

	    ASSERT(po->type == PAGING_OBJ_TYPE_FILE);

	    (void) lseek((int)po->data,
			 po->offset + (offset + n - pr->object_offset),
			 L_SET);

	    if (write((int)po->data, data + n, memory_object_page_size) < 0)
		unix_error_exit("memory_object_data_write: write");

	    page = (offset + n - pr->object_offset)/memory_object_page_size;
	    page_set_written(pr->w_paging_map, page);
	}

    (void) vm_deallocate(mach_task_self(), data, length);

    return (KERN_SUCCESS);
}

kern_return_t
memory_object_data_unlock(memory_object_t		object,
			  memory_object_control_t	control,
			  vm_offset_t			offset,
			  vm_size_t			length,
			  vm_prot_t			permission)
{
    printf("memory_object_data_unlock: offset %x length %d\n", offset, length);

    return (KERN_SUCCESS);
}

kern_return_t
memory_object_lock_completed(memory_object_t		object,
			     memory_object_control_t	control,
			     vm_offset_t		offset,
			     vm_size_t			length)
{
    printf("memory_object_data_lock_completed: offset %x length %d\n", offset, length);

    return (KERN_SUCCESS);
}

kern_return_t
memory_object_copy(memory_object_t		object,
		   memory_object_control_t	control,
		   vm_offset_t			offset,
		   vm_size_t			length,
		   memory_object_t		new_object)
{
    printf("memory_object_data_copy: offset %x length %d\n", offset, length);

    return (KERN_SUCCESS);
}

kern_return_t
memory_object_terminate(memory_object_t		object,
			memory_object_control_t	control,
			memory_object_name_t	name)
{
    printf("memory_object_data_terminate\n");

    return (KERN_SUCCESS);
}

kern_return_t
memory_object_supply_completed(memory_object_t		object,
			       memory_object_control_t	control,
			       vm_offset_t		offset,
			       vm_size_t		length,
			       kern_return_t		result,
			       vm_offset_t		error_offset)
{
    printf("memory_object_supply_completed\n");

    return (KERN_SUCCESS);
}

kern_return_t
memory_object_data_return(memory_object_t		object,
			  memory_object_control_t	control,
			  vm_offset_t			offset,
			  pointer_t			data,
			  unsigned int			length)
{
    printf("memory_object_data_return\n");

    return (KERN_SUCCESS);
}
