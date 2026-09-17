/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie-Mellon University
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: emul/server/pager_mappings.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include "server_defs.h"
#include "pager_defs.h"

#include <device/device_types.h>

#include "mappings.h"

static device_t		kern_device;
static memory_object_t	kern_memory_object;

memory_object_t	low, high;
vm_offset_t	low_offset, high_offset;
vm_size_t	low_size, high_size;

boolean_t	mac32bit;

setup_mappings()
{
    KernelObjects(mach_task_self(), &mac32bit,
		  &low, &low_offset, &low_size,
		  &high, &high_offset, &high_size);

    printf("low %x offset %x size %x\n", low, low_offset, low_size);
    printf("high %x offset %x size %x\n", high, high_offset, high_size);

    map_ram();

    map_patches();

    map_special();
}

map_ram()
{
    vm_address_t	address;
    vm_size_t		size;

    /*
     * Map the low memory with the contents
     * of the kernel low memory object.
     */
    (void) vm_map(task,
		  &low_offset, low_size,	/* addr, size */
		  0, FALSE,			/* addr mask, anywhere */
		  low, 0,			/* obj, offset */
		  FALSE,				/* copy */
		  VM_PROT_READ | VM_PROT_WRITE,	/* cur perm */
		  VM_PROT_READ | VM_PROT_WRITE,	/* max perm */
		  VM_INHERIT_COPY);		/* inheritance */

    /*
     * Map the high memory with the contents
     * of the kernel high memory object.
     */
    (void) vm_map(task,
		  &high_offset, high_size,	/* addr, size */
		  0, FALSE,			/* addr mask, anywhere */
		  high, 0,			/* obj, offset */
		  FALSE,				/* copy */
		  VM_PROT_READ | VM_PROT_WRITE,	/* cur perm */
		  VM_PROT_READ | VM_PROT_WRITE,	/* max perm */
		  VM_INHERIT_COPY);		/* inheritance */

    /*
     * Map the rest zero fill.
     */
    address = low_offset + low_size;
    size = high_offset - (low_offset + low_size);
    (void) vm_map(task,
		  &address, size,		/* addr, size */
		  0, FALSE,			/* addr mask, anywhere */
		  MEMORY_OBJECT_NULL, 0,	/* obj, offset */
		  FALSE,			/* copy */
		  VM_PROT_READ | VM_PROT_WRITE,	/* cur perm */
		  VM_PROT_READ | VM_PROT_WRITE,	/* max perm */
		  VM_INHERIT_COPY);		/* inheritance */
}

map_patches()
{
    register vm_offset_t	offset;
    vm_address_t		address;
    vm_size_t			size;

    if (read(file, &filehdr, sizeof (struct exec)) < 0)
	unix_error_exit("read header");

    if (filehdr.a_entry != EMUL_PATCHES_START)
	other_error_exit("patches entry wrong");

    /*
     * Text region
     */
    offset = EMUL_PATCHES_START;
    pager_new_region(offset,
		     round_page(filehdr.a_text),
		     VM_PROT_READ|VM_PROT_WRITE,
		     sizeof (filehdr),
		     PAGING_OBJ_TYPE_FILE, file,
		     0,
		     PAGING_OBJ_TYPE_NULL, 0);

    pager_map_region(offset,
		     round_page(filehdr.a_text),
		     VM_PROT_READ|VM_PROT_WRITE,
		     offset,
		     VM_INHERIT_NONE);

    /*
     * Data region (and part of BSS XXX)
     */
    offset += round_page(filehdr.a_text);
    pager_new_region(offset,
		     round_page(filehdr.a_data),
		     VM_PROT_READ|VM_PROT_WRITE,
		     sizeof (filehdr) + round_page(filehdr.a_text),
		     PAGING_OBJ_TYPE_FILE, file,
		     0,
		     PAGING_OBJ_TYPE_NULL, 0);

    pager_map_region(offset,
		     round_page(filehdr.a_data),
		     VM_PROT_READ|VM_PROT_WRITE,
		     offset,
		     VM_INHERIT_NONE);

    /*
     * Rest is zero fill
     */
    offset += round_page(filehdr.a_data);
    address = offset;
    size = EMUL_PATCHES_START + EMUL_PATCHES_SIZE - address;
    (void) vm_map(task,
		  &address, size,		/* addr, size */
		  0, FALSE,			/* addr mask, anywhere */
		  MEMORY_OBJECT_NULL, 0,	/* obj, offset */
		  FALSE,			/* copy */
		  VM_PROT_READ | VM_PROT_WRITE,	/* cur perm */
		  VM_PROT_READ | VM_PROT_WRITE,	/* max perm */
		  VM_INHERIT_NONE);		/* inheritance */
}

map_special()
{
    kern_return_t	result;

    result = device_open(master_device_port,
			 D_READ|D_WRITE,
			 "kern",
			 &kern_device);
    if (result != KERN_SUCCESS)
	mach_error_exit("device open kern", result);

    result = device_map(kern_device,
			VM_PROT_READ|VM_PROT_WRITE,
			0,
			0 - vm_page_size,	/* XXX */
			&kern_memory_object,
			0);
    if (result != KERN_SUCCESS)
	mach_error_exit("device map kern", result);

#define pager_special_region(X, prot, max_prot, copy) \
    {							\
	vm_address_t	address;			\
\
	address = X##_ADDR;				\
	result = vm_map(task,				\
			&address,			\
			X##_SIZE,			\
			0, FALSE,			\
			kern_memory_object,		\
			X##_ADDR,			\
			(copy),				\
			(prot), (max_prot),		\
			VM_INHERIT_NONE);		\
	if (result != KERN_SUCCESS)			\
	    mach_error_exit("vm map special", result);	\
    }

    /*
     * Map the NuBus slots in
     * shared.
     */
    {
	register slot;

	for (slot = SLOT_NUM_LOW; slot <= SLOT_NUM_HIGH; slot++) {
	    pager_special_region(SLOT,
				 VM_PROT_READ|VM_PROT_WRITE,
				 VM_PROT_READ|VM_PROT_WRITE,
				 FALSE);
	}
    }

    /*
     * Map in a copy of the
     * ROM (so we can patch it).
     */
    pager_special_region(ROM,
			 VM_PROT_READ|VM_PROT_WRITE,
			 VM_PROT_READ|VM_PROT_WRITE,
			 TRUE);
}
