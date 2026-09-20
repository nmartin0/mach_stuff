/*
 * Copyright (c) 1991 Carnegie Mellon University
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
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	pager.c,v $
 * Revision 2.7  92/04/14  13:20:40  grm
 * 	Bob's changes so that linkage with libmach.a will work.
 * 	[92/03/27            grm]
 * 
 * Revision 2.6  92/03/02  15:47:37  grm
 * 	Changed for use with MK69.  The FOO_MSG_SIZE_MAX hack was added.
 * 	[92/02/20            grm]
 * 
 * Revision 2.5  92/02/14  17:44:54  grm
 * 	Remplaced the exit(0)'s with exit_dos() again!
 * 	[92/02/11            grm]
 * 
 * Revision 2.4  92/02/03  14:25:19  rvb
 * 	Clean Up
 * 
 * Revision 2.3  92/02/02  23:02:45  rvb
 * 	Replaced the exit(0)'s with exit_dos()'s.
 * 	[92/01/27            grm]
 * 
 * Revision 2.2  91/12/05  16:43:06  grm
 * 	Added some debugging code.
 * 	[91/12/04            grm]
 * 	New Copyright
 * 	[91/05/28  15:22:13  grm]
 * 
 * 	Fixed new_memory_object.  Added some debugging
 * 	output.  Graceful exits added.
 * 	[91/04/30  13:56:26  grm]
 * 
 * 	rcs created.
 * 	[91/02/01  13:32:30  grm]
 * 
 */

#include "base.h"
#include "bios.h"
#include <mach.h>
#include <mach/message.h>

/*
 * XXX This is a BIG hack.  This should be changed.
 */
#define	FOO_MSG_SIZE_MAX	8192

typedef struct max_msg {
    mach_msg_header_t header;
    char data[FOO_MSG_SIZE_MAX - sizeof(mach_msg_header_t)];
} max_msg_t;

max_msg_t in_msg;
max_msg_t out_msg;

int pager_initialized = FALSE;
mach_port_t memory_object_mux = MACH_PORT_NULL;

extern int exit_index;

void
pager()
{
    kern_return_t status;

    for (;;)	{
        in_msg.header.msgh_local_port = memory_object_mux;
        in_msg.header.msgh_size = sizeof(in_msg);	
	Debug0((dbg_fd, "About to call msg_receive in pager.\n"));
	status = mach_msg(&in_msg.header, MACH_RCV_MSG, 0,
			  (&in_msg.header)->msgh_size,
			  (&in_msg.header)->msgh_local_port, 0, MACH_PORT_NULL);
	if (status != MACH_MSG_SUCCESS) continue;
	Debug0((dbg_fd, "About to call memory object server.\n"));
	if (us_debug_level) fflush(dbg_fd);
	if (!memory_object_server(&in_msg.header, &out_msg.header)) {	
		printf("memory object server confused\n");
		exit_index = 11;
		exit_dos();
	}
	Debug0((dbg_fd, "Called memory object server.\n"));
	if (us_debug_level) fflush(dbg_fd);
    }
}

#define MAX_OBJECTS 256

struct object_record {
	mach_port_t memory_object;
	mach_port_t memory_control;
	mach_port_t memory_name;
	vm_address_t backing_addr;
	vm_size_t    backing_size;
} object_table[MAX_OBJECTS];

void
object_table_init()
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		object_table[i].memory_object = MACH_PORT_NULL;
		object_table[i].memory_control = MACH_PORT_NULL;
		object_table[i].memory_name = MACH_PORT_NULL;
		object_table[i].backing_addr = 0;
		object_table[i].backing_size = 0;
	}
}

int
object_table_lookup(memory_object)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (object_table[i].memory_object == memory_object) {
			return (i);
		}
	}
	return (-1);
}

mach_port_t
object_control_lookup(memory_object)
    mach_port_t memory_object;
{
	int index;
	index = object_table_lookup(memory_object);
	if (index == -1) return (MACH_PORT_NULL);
	return (object_table[index].memory_control);
}


boolean_t
object_table_add(memory_object, memory_control, memory_name, addr, size)
    mach_port_t memory_object;
    mach_port_t memory_control;
    mach_port_t memory_name;
    vm_address_t addr;
    vm_size_t size;
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (object_table[i].memory_object == MACH_PORT_NULL) {
			object_table[i].memory_object = memory_object;
			object_table[i].memory_control = memory_control;
			object_table[i].memory_name = memory_name;
			object_table[i].backing_addr = addr;
			object_table[i].backing_size = size;
			return(TRUE);
		}
	}
	return(FALSE);
}

void
object_table_remove(memory_object)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (object_table[i].memory_object == memory_object) {
			object_table[i].memory_object = MACH_PORT_NULL;
			object_table[i].memory_control = MACH_PORT_NULL;
			object_table[i].memory_name = MACH_PORT_NULL;
			if (object_table[i].backing_addr) {
				MACH_CALL((vm_deallocate(mach_task_self(),
					object_table[i].backing_addr,
					object_table[i].backing_size)),
					"Deallocating backing memory");
			}
			object_table[i].backing_addr = 0;
			object_table[i].backing_size = 0;
			return;
		}
	}
}

void
pager_initialize()
{
    kern_return_t status;
    thread_t pager_thread;
    int i;
    if (pager_initialized == FALSE)  {
	MACH_CALL((mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &memory_object_mux)),
		  "Initializing mux port.");
	pager_initialized = TRUE;
	object_table_init();
        create_a_thread(&pager_thread, pager);
        thread_resume(pager_thread);
        millisecond_wait(1000);
    }
}
    
/* Server side of memory object creation */
mach_port_t
new_memory_object(size)
vm_size_t size;
{
    mach_port_t memory_object;
    mach_port_t tmp1,tmp2;
    vm_offset_t memory_region;
    kern_return_t ret;

    if (pager_initialized == FALSE) pager_initialize();


    MACH_CALL((mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &memory_object)),
	      "Allocating new memory object.");

    mach_port_extract_right(mach_task_self(), memory_object, 
				MACH_MSG_TYPE_MAKE_SEND, &tmp1, &tmp2);

    MACH_CALL((mach_port_move_member(mach_task_self(), memory_object, memory_object_mux)),
	     "create_paged_object: could not add memory object to mux");
    object_table_add(memory_object, MACH_PORT_NULL, MACH_PORT_NULL, 0, size);
    memory_region = 0;
    ret = vm_map(mach_task_self(), &memory_region, size,
		    0, TRUE, memory_object, 0, FALSE,
		    VM_PROT_ALL, VM_PROT_ALL, VM_INHERIT_SHARE);

    if (ret != KERN_SUCCESS) {
	    fprintf (dbg_fd,"pager: EXITING!!! new_memory_object size = 0x%x err = 0x%x\n",
		     size, ret);
	    exit_index = 17;
	    exit_dos();
    }

    return memory_object;
}

void
memory_object_remap(memory_object, offset, addr, size)
mach_port_t memory_object;
vm_offset_t offset;
vm_address_t addr;
vm_size_t size;
{
	kern_return_t ret;
	
        Vdebug2((dbg_fd, "Pager remap %x, %x, %x\n", addr, offset, size));
	MACH_CALL((vm_deallocate(mach_task_self(), addr, size)),"remap");
	ret = vm_map(mach_task_self(), &addr, size, 0, FALSE, 
			  memory_object, offset, FALSE, 
			  VM_PROT_ALL, VM_PROT_ALL, VM_INHERIT_SHARE);

        Vdebug2((dbg_fd, "Pager remap returns %x\n", ret));
	if (ret != KERN_SUCCESS) {
		fprintf(dbg_fd,"pager: EXITING!!! memory_object_remap size = 0x%x\n",
			size);
		exit_index = 18;
		exit_dos();
	}
}
	
void
destroy_memory_object(memory_object)
    mach_port_t memory_object;
{
    kern_return_t status;
    object_table_remove(memory_object);	
    status = mach_port_move_member(mach_task_self(), memory_object, MACH_PORT_NULL);
}

kern_return_t
memory_object_init(memory_object, memory_control, memory_object_name,
		   page_size)
    mach_port_t memory_object;
    mach_port_t memory_control;
    mach_port_t memory_object_name;
    vm_size_t page_size;
{
    int index;
    if (page_size != vm_page_size) {
	printf("memory_object_init: invalid page size %d (v. %d)\n",
	        page_size, vm_page_size);
	exit_index = 19;
	exit_dos();
    }
    memory_object_set_attributes(memory_control, TRUE, TRUE,
			         MEMORY_OBJECT_COPY_NONE);

    index = object_table_lookup(memory_object);
    if (index == -1) {
	printf("object table lookup failure: fatal error!\n");
	exit_index = 19;
	exit_dos();
    }
    object_table[index].memory_control = memory_control;
    object_table[index].memory_name = memory_object_name;

    Vdebug2((dbg_fd, "Pager init %x, %x\n", memory_object, memory_control));

    return KERN_SUCCESS;
}

kern_return_t
memory_object_copy(old_memory_object, old_memory_control, offset, length,
	 new_memory_object)
    memory_object_t old_memory_object;
    memory_object_control_t old_memory_control;
    vm_offset_t offset;
    vm_size_t length;
    memory_object_t new_memory_object;
{
    Vdebug2((dbg_fd, "Pager copy %x, %x\n",
				old_memory_object, old_memory_control));
    return KERN_SUCCESS;
}

kern_return_t
memory_object_terminate(memory_object, memory_control, memory_object_name)
    mach_port_t memory_object;
    mach_port_t memory_control;
    mach_port_t memory_object_name;
{
    Vdebug2((dbg_fd, "Pager terminate %x, %x\n", 
					memory_object, memory_control));
    mach_port_deallocate(mach_task_self(), memory_control);
    mach_port_deallocate(mach_task_self(), memory_object_name);
    destroy_memory_object(memory_object);
    return KERN_SUCCESS;
}

kern_return_t
memory_object_data_request(memory_object, memory_control, offset, length,
			   desired_access)
    mach_port_t memory_object, memory_control;
    vm_offset_t offset;
    vm_size_t length;
    vm_prot_t desired_access;
{
    int index;
    vm_address_t addr;

    index = object_table_lookup(memory_object);

    Vdebug2((dbg_fd, "Pager data request %x, %x, %x\n",
	 			    memory_object, offset, length));

    if ((index == -1) || ((addr = object_table[index].backing_addr) == 0)) {
	    MACH_CALL((memory_object_data_unavailable(memory_control, 
					offset, length)),"Data unavailable");
    } else {
	    MACH_CALL((memory_object_data_provided(memory_control,
			offset, addr + offset, length, VM_PROT_NONE)),
			"Data provided");
    }

    return KERN_SUCCESS;
}

kern_return_t
memory_object_data_unlock(memory_object, memory_control, offset, length,
			  desired_access)
    mach_port_t memory_object, memory_control;
    vm_offset_t offset;
    vm_size_t length;
    vm_prot_t desired_access;
{
    Vdebug2((dbg_fd, "Pager unlock %x, %x, %x\n",
	  				    memory_object, offset, length));
    return KERN_FAILURE;    
}

kern_return_t
memory_object_data_write(memory_object, memory_control, offset, data, length)
    mach_port_t memory_object;
    mach_port_t memory_control;
    vm_offset_t offset;
    char *data;
    int length;
{
    int index;
    vm_address_t addr;
    index = object_table_lookup(memory_object);
    Vdebug2((dbg_fd, "Pager data write %x, %x, %x\n",
	  				    memory_object, offset, length));
    if (index == -1) return (KERN_FAILURE);
    if (((addr = object_table[index].backing_addr) == 0)) {
	    addr = 0;
	    MACH_CALL((vm_allocate(mach_task_self(), &addr, 
				object_table[index].backing_size, TRUE)), 
				"vm_allocate of backing store.");
	    object_table[index].backing_addr = addr;
    }
    bcopy(data, addr+offset, length);
    return KERN_SUCCESS;
}

/*
 * memory_object_lock_completed: called when kernel has finished
 * performing the last memory_object_lock_request operation on this
 * memory object.
 */

kern_return_t
memory_object_lock_completed(memory_object, memory_control, offset, length)
    mach_port_t memory_object;
    mach_port_t memory_control;
    vm_offset_t offset;
    vm_size_t length;
{
    Vdebug2((dbg_fd, "Pager lock completed %x, %x, %x\n",
	  				    memory_object, offset, length));
    return KERN_SUCCESS;    
}

/*
 *	Indicate that a previous memory_object_data_supply has been
 *	completed.  Note that this call is made on whatever
 *	port is specified in the memory_object_data_supply; that port
 *	need not be the memory object port itself.
 *
 *	The result parameter indicates what happened during the supply.
 *	If it is not KERN_SUCCESS, then error_offset identifies the
 *	first offset at which a problem occurred.  The pagein operation
 *	stopped at this point.  Note that the only failures reported
 *	by this mechanism are KERN_MEMORY_PRESENT.  All other failures
 *	(invalid argument, error on pagein of supplied data in manager's
 *	address space) cause the entire operation to fail.
 *
 *	XXX Check what actually happens in latter case!
 *
 *	[No reply expected.]
 */

kern_return_t
memory_object_supply_completed(memory_object, memory_control, offset, length, result, error_offset)
    mach_port_t memory_object;
    mach_port_t memory_control;
    vm_offset_t	offset;
    vm_size_t	length;
    kern_return_t	result;
    vm_offset_t	error_offset;
{
    Vdebug2((dbg_fd, "Pager supply completed %x, %x, %x, %x, %x, %x\n",
	  		memory_object, memory_control, offset, length,
			result, error_offset));
    return KERN_SUCCESS;    
}

/*
 *	Return data to manager.  This call is used in place of data_write
 *	for objects initialized by object_ready instead of set_attributes.
 *	This call indicates whether the returned data is dirty and whether
 *	the kernel kept a copy.  Precious data remains precious if the
 *	kernel keeps a copy.  The indication that the kernel kept a copy
 *	is only a hint if the data is not precious; the cleaned copy may
 *	be discarded without further notifying the manager.
 *
 *	[Reply should be vm_deallocate to release the data.]
 */
kern_return_t
memory_object_data_return(memory_object, memory_control, offset, data, dirty, kernel_copy)
    mach_port_t memory_object;
    mach_port_t memory_control;
    vm_offset_t	offset;
    pointer_t	data;
    boolean_t	dirty;
    boolean_t	kernel_copy;
{
    Vdebug2((dbg_fd, "Pager data return %x %x %x %x %x %x\n",
    			memory_object, memory_control, offset, data,
			dirty, kernel_copy));
    return KERN_SUCCESS;    
}

/*
 * XXX	Warning:  This routine does NOT contain a memory_object_control_t
 * XXX	because the memory_object_change_attributes call may cause
 * XXX  memory object termination (by uncaching the object).  This would
 * XXX  yield an invalid port.
 */
kern_return_t
memory_object_change_completed(memory_object, may_cache, copy_strategy)
    mach_port_t				 memory_object;
    boolean_t				may_cache;
    memory_object_copy_strategy_t	copy_strategy;
{
    Vdebug2((dbg_fd, "Pager change completed %x, %x, %x\n",
	  			    memory_object, may_cache, copy_strategy));
    return KERN_SUCCESS;    
}
