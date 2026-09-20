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
 *
 * Purpose:
 *	Mach XMS Memory Manager
 *
 * HISTORY: 
 * $Log:	bios_xms.c,v $
 * Revision 2.7  92/07/01  14:24:58  grm
 * 	Added some debugging code.
 * 	[92/06/30  13:43:58  grm]
 * 
 * Revision 2.6  92/04/14  13:20:32  grm
 * 	Changed code for use with the mmap call.  Now saves and restores
 * 	the first page via bcopy's if the bsd boolean flag is set
 * 	(running on UX BSD Server and not the OSF/1 Single Server)
 * 	[92/04/06            grm]
 * 	Mach3 Version.  Copies the first page of iopl device memory
 * 	before A20 remap and then copies it back.  This is the No Runtime
 * 	Files version.
 * 	[92/03/27            grm]
 * 
 * Revision 2.5  92/02/14  17:44:51  grm
 * 	Merged in the OSF Single Server changes.  Added vm_regions calls
 * 	for debugging.
 * 	[92/02/11            grm]
 * 
 * Revision 2.4  92/02/03  14:24:58  rvb
 * 	Clean Up
 * 
 * Revision 2.3  91/12/06  15:29:14  grm
 * 	Replaced the absolute mon_space code with the relative dosres
 * 	code.
 * 	[91/12/06            grm]
 * 
 * Revision 2.2  91/12/05  16:41:59  grm
 * 	Allocate 0x2000 bytes of space for mon_space.
 * 	[91/12/04            grm]
 * 	Created.
 * 	[91/06/28  18:51:47  grm]
 * 
 */

#include "base.h"
#include "bios.h"
#include "bios_misc.h"

#include <sys/file.h>
#include <sys/ioctl.h>
#include <mach/message.h>

/* Support XMS version 2.0 */
#define XMS_VERSION	0x20
#define XMM_VERSION	0x20

#define	EMS_GET_EXTENDED_MEMORY_SIZE	0x88
#define EMS_MOVE_EXTENDED_MEMORY_BLOCK	0x87

/* EMM errors */

#define gdt_address(gdt, indx) \
	(char *)((*((u_long *)(gdt+(8*indx+2))))&0xffffff)

#define gdt_length(gdt, indx) \
	(*((u_short *)(gdt+(8*indx))))

#define gdt_access(gdt, indx) \
	((*((u_long *)(gdt+(8*indx+2))))>>24)

#define	EMS_SRC_INDX	0x2
#define EMS_DST_INDX	0x3

boolean_t A20_On_Flag = FALSE;

#define IN_WRAP_AROUND(addr) ((u_long)addr>=0x100000&&(u_long)addr<=0x10ffff)

mach_port_t wrap_around_obj;
mach_port_t ems_obj;

#define XMS_BASE 0x100000
#define XMS_SIZE 0x200000
#define XMS_WRAP_SIZE 0x10000
#define XMS_HANDLES 128
#define XMS_NO_HANDLE -1

#define UMB_BASE 0xc0000
#define UMB_SIZE 0x40000
#define UMBS (UMB_SIZE/16)
#define UMB_PAGE 4096
#define UMB_NULL -1

#define IN_EMM_SPACE(addr) (addr >= 0xd0000 && addr <= 0xe0000)

struct umb_record {
	vm_address_t	addr;
	vm_size_t	size;
	boolean_t	in_use;
	boolean_t	free;
} umbs[UMBS];

boolean_t
umb_memory_empty(addr, size) 
vm_address_t addr;
int size;
{
	int i;
	int * memory = (int *) addr;
	for (i = 0; i < (size/4); i++) {
		if ((memory[i] != 0xffffffff) && (memory[i] != 0)) {
			return FALSE;
		}
	}
	Debug0((dbg_fd,"Found free UMB region: %x\n",addr));
	return (TRUE);
}

int
umb_setup()
{
	int i;
	int umb;
	vm_address_t addr;

	for (i = 0; i < UMBS; i++) {
		umbs[i].in_use = FALSE;
	}

	umb = umb_find_unused();

	umbs[umb].in_use = TRUE;
	umbs[umb].free = FALSE;
	umbs[umb].addr = 0xd0000;
	umbs[umb].size = 0x10000;

	for (addr = UMB_BASE; addr < (vm_address_t)(UMB_BASE+UMB_SIZE);
	     addr += UMB_PAGE) {
		if (IN_EMM_SPACE(addr)) continue;
		if (umb_memory_empty(addr, UMB_PAGE)) {
			if ((umbs[umb].addr + umbs[umb].size) == addr) {
				umbs[umb].size += UMB_PAGE;
			} else {
				umb = umb_find_unused();
				umbs[umb].in_use = TRUE;
				umbs[umb].free = TRUE;
				umbs[umb].addr = addr;
				umbs[umb].size = UMB_PAGE;
				Debug0((dbg_fd,"New UMB region: %x\n",addr));
			}
		}
	}

	for (i = 0; i < UMBS; i++) {
		if (umbs[i].in_use && umbs[i].free) {
			vm_address_t addr = umbs[i].addr;
			vm_size_t size = umbs[i].size;
			MACH_CALL((vm_deallocate(mach_task_self(), 		
				  (vm_address_t)addr,
			          (vm_size_t)size)), "vm_deallocate");
			MACH_CALL((vm_allocate(mach_task_self(), &addr, 
			          (vm_size_t)size, 
				  FALSE)), 
				  "vm_allocate of umb block.");
		}
	}
}

int 
umb_find_unused()
{
	int i;
	for (i = 0; i < UMBS; i++) {
		if (!umbs[i].in_use) return (i);
	}
	return (UMB_NULL);
}

int 
umb_find(segbase)
vm_address_t segbase;
{
	int i;
	vm_address_t addr = segbase*16;
	for (i = 0; i < UMBS; i++) {
		if (umbs[i].in_use &&
		    ((addr >= umbs[i].addr) && 
		     (addr <= (umbs[i].addr + umbs[i].size)))) {
			return (i);
		}
	}
	return (UMB_NULL);
}

vm_address_t
umb_allocate(size)
int size;
{
	int i;
	for (i = 0; i < UMBS ; i++) {
		if (umbs[i].in_use && umbs[i].free) {
			if (umbs[i].size >= size) {
				int new_umb = umb_find_unused();
				if (new_umb != UMB_NULL) {
					umbs[new_umb].in_use = TRUE;
					umbs[new_umb].free   = TRUE;
					umbs[new_umb].addr = 
						umbs[i].addr + size;
					umbs[new_umb].size = 
						umbs[i].size - size;
					umbs[i].size = size;
					umbs[i].free = FALSE;
					return (umbs[i].addr);
				}
			}
		}
	}
	return ((vm_address_t)0);
}

int
umb_free(segbase)
int segbase;
{
	int umb = umb_find(segbase);
	if (umb != UMB_NULL) 
		umbs[umb].free = TRUE;
	return (0);
}

int
umb_query()
{
	int i;
	int largest = 0;
	for (i = 0; i < UMBS; i++) {
		if (umbs[i].in_use && umbs[i].free) {
			if (umbs[i].size > largest) largest = umbs[i].size;
		}
	}
	return (largest);
}

int total_extended_memory_size = XMS_SIZE/1024;		/* in K bytes */

#define CHECK_ADDRESS(addr) ((u_long)addr>=0&&(u_long)addr<=(XMS_BASE+XMS_SIZE))

#define check_handle(handle) 				\
{							\
	if (handle < 0 || handle >= XMS_HANDLES) {	\
		Debug0((dbg_fd, "Handle error: %x\n",	\
			handle));			\
		return (XMS_NO_HANDLE);			\
	}						\
	if (!handles[handle].in_use) {			\
		Debug0((dbg_fd, "Handle error: %x\n",	\
			handle));			\
		return (XMS_NO_HANDLE);			\
	}						\
}

struct handle_record {
	vm_address_t	addr;
	vm_size_t	size;
	boolean_t	in_use;
	boolean_t	free;
	int		lock_cnt;
} handles[XMS_HANDLES];

int handles_available = XMS_HANDLES - 2;

int
xms_check_handle(handle)
int handle;
{
	check_handle(handle);
	return(0);
}

int 
xms_find_unused_handle()
{
	static last_handle = 0;
	int i;

	for (i = last_handle; i < XMS_HANDLES; i++) {
		if (!handles[i].in_use) return (i);
	}

	for (i = 0; i < last_handle; i++) {
		if (!handles[i].in_use) return (i);
	}
	
	return (XMS_NO_HANDLE);
}

int
xms_query()
{
	int i;
	int largest = 0;
	for (i = 0; i < XMS_HANDLES; i++) {
		if (handles[i].in_use && handles[i].free) {
			if (handles[i].size > largest) 
				largest = handles[i].size;
		}
	}
	return (largest);
}

int
xms_allocate(kbytes)
int kbytes;
{
	int i;
	for (i = 0; i < XMS_HANDLES; i++) {
		if (handles[i].in_use && handles[i].free) {
			if ((handles[i].size/1024) >= kbytes) {
				int new_handle = xms_find_unused_handle();
				if (new_handle != XMS_NO_HANDLE) {
					handles[new_handle].in_use = TRUE;
					handles[new_handle].free   = TRUE;
					handles[new_handle].lock_cnt = 0;
					handles[new_handle].addr = 
						handles[i].addr+(kbytes*1024);
					handles[new_handle].size = 
						handles[i].size-(kbytes*1024);
					handles[i].size = (kbytes*1024);
					handles[i].free = FALSE;
					handles_available--;
					return (i);
				}
			}
		}
	}
	return (XMS_NO_HANDLE);
}

int
xms_free(handle)
int handle;
{
	check_handle(handle);
	handles[handle].free = TRUE;
	handles_available++;
	return (0);
}

int
xms_move(src_handle, src_offset, dst_handle, dst_offset, count)
int src_handle;
vm_address_t src_offset;
int dst_handle;
vm_address_t dst_offset;
int count;
{
	u_char * src;
	u_char * dst;
	boolean_t was_on;
	check_handle(src_handle);
	check_handle(dst_handle);

	Debug0((dbg_fd, "xms_move: %x %x, %x %x\n", src_handle, dst_handle, 
						     src_offset, dst_offset));

	if (src_handle == 0) {
		src = (u_char *)
			Addr_8086((src_offset >> 16), (src_offset & 0xffff));
	} else {
		src_offset = src_offset;
		src = (u_char *) (handles[src_handle].addr + src_offset);
		if (handles[src_handle].size < src_offset) {
			Debug0((dbg_fd,"Offset too large: %x\n",src_offset));
			return(XMS_NO_HANDLE);
		}
	}


	if (dst_handle == 0) {
		dst = (u_char *)
			Addr_8086((dst_offset >> 16), (dst_offset & 0xffff));
	} else {
		dst_offset = dst_offset;
		dst = (u_char *) (handles[dst_handle].addr + dst_offset);
		if (handles[dst_handle].size < dst_offset) {
			Debug0((dbg_fd,"Offset too large: %x\n",dst_offset));
			return(XMS_NO_HANDLE);
		}
	}

	Debug0((dbg_fd, "xms_move: %x -> %x (%x)\n", src, dst, count));

	if ((!CHECK_ADDRESS(src)) || (!CHECK_ADDRESS(dst))) {
		Debug0((dbg_fd,"Address error in move: %x -> %x\n",
				src, dst));
		return(XMS_NO_HANDLE);
	}

	if (IN_WRAP_AROUND(src) || IN_WRAP_AROUND(dst)) {
		was_on = A20_on();
	}

	if (us_debug_level) {
		u_long * sl = (u_long *)src;
		fprintf(dbg_fd, "src: %x %x %x %x\n",
			sl[0], sl[1], sl[2], sl[3]);
		fprintf(dbg_fd, "src: %x %x %x %x\n",
			sl[4], sl[5], sl[6], sl[7]);
	}

	bcopy(src, dst, count);

	if (IN_WRAP_AROUND(src) || IN_WRAP_AROUND(dst)) {
		if (!was_on) A20_off();
	}

	return (0);
}

int
xms_lock(handle)
int handle;
{
	check_handle(handle);
	handles[handle].lock_cnt++;
	return (0);
}

int
xms_unlock(handle)
int handle;
{
	check_handle(handle);
	handles[handle].lock_cnt--;
	return (0);
}

int 
xms_resize(handle, kbytes)
int handle;
int kbytes;
{
	check_handle(handle);
	return (0);
}

u_char pseudo_vdisk_partition[0x20] = {
	0x00, 0x00, 0x00, 0x56, 0x44, 0x49, 0x53, 0x4b,
	0x33, 0x2e, 0x33, 0x80, 0x00, 0x01, 0x01, 0x00,
	0x01, 0x40, 0x00, 0x00, 0x02, 0xfe, 0x06, 0x00,
	0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x44 };

u_char pseudo_vdisk_driver[0x30] = {
	0x00, 0x00, 0xe7, 0x19, 0x00, 0x08, 0xa9, 0x00,
	0xd4, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x56, 0x44, 0x49, 0x53, 0x4b, 0x20,
	0x20, 0x56, 0x33, 0x2e, 0x33, 0x28, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x60, 0x86, 0x09, 0x00, 0x00, 0x11, 0xe0 };

void
A20_init()
{
	int i;
	vm_address_t addr, tmpaddr;
	vm_address_t first_page;
	extern int mon_space;
	extern boolean_t bsd;
	kern_return_t rc;
	mach_port_t iopl_device_port;

	umb_setup();

	mon_space = (int) umb_allocate(RESERVE_SPACE);
	addr = (vm_address_t) mon_space;
	MACH_CALL((vm_deallocate(mach_task_self(), (vm_address_t)addr,
			      (vm_size_t)RESERVE_SPACE)), "vm_deallocate");
	MACH_CALL((vm_allocate(mach_task_self(), &addr, (vm_size_t)RESERVE_SPACE, 
		FALSE)), "vm_allocate of mon usable address space.");
	if (us_debug_level > 0) {
		fprintf(dbg_fd,"vm_allocate of mon usable address space.\n");
		vm_regions();
	}

	addr = (vm_address_t) XMS_BASE;
	rc = vm_allocate(mach_task_self(), &addr, 
			(vm_size_t)XMS_SIZE, 
			FALSE);
	if (rc != KERN_SUCCESS) {
		fprintf(dbg_fd,"vm_allocate of xms address memory\n");
		vm_regions();
		exit_dos();
	}
	if (us_debug_level > 0) {
		fprintf(dbg_fd,"vm_allocate of xms address memory\n");
		vm_regions();
	}

	wrap_around_obj = new_memory_object(XMS_WRAP_SIZE);

	if (bsd) {
		/*
		 * Save the contents of the first page
		 */
		
		rc = vm_allocate(mach_task_self(),&addr,vm_page_size, TRUE);
		if (rc != KERN_SUCCESS) {
			mach_error("Couldn't allocate temp space.",rc);
			exit(-1);
		}
		
		first_page = (vm_address_t)0;
		
		bcopy(first_page, addr, vm_page_size);
		
		/*
		 * get rid of the low mem area and bring in fresh zero filled
		 * memory.
		 */
		
		tmpaddr = (vm_address_t)0;
		rc = vm_deallocate(mach_task_self(), tmpaddr, 0xa0000);
		if (rc != KERN_SUCCESS) {
			mach_error("Couldn't deallocate main memory.",rc);
			exit(-1);
		}
		rc = vm_allocate(mach_task_self(), &tmpaddr, 0xa0000, FALSE);
		if (rc != KERN_SUCCESS) {
			mach_error("Couldn't reallocate main memory.",rc);
			exit(-1);
		}
	}

	/*
	 * Finish remapping the wrap around area.
	 */

	memory_object_remap(wrap_around_obj, 0, 0, XMS_WRAP_SIZE);
	ems_obj = new_memory_object(XMS_WRAP_SIZE);
	memory_object_remap(wrap_around_obj, 0, XMS_BASE, XMS_WRAP_SIZE);

	if (bsd) {
		/*
		 * Recopy the contents of the first page.
		 */
		
		bcopy(addr, first_page, vm_page_size);
		
		rc = vm_deallocate(mach_task_self(), addr, vm_page_size);
		if (rc != KERN_SUCCESS) {
			mach_error("Couldn't deallocate temp space.", rc);
			exit(-1);
		}
	}

	for (i = 0; i < XMS_HANDLES; i++) {
		handles[i].in_use = FALSE;		
		handles[i].lock_cnt = 0;
	}
	
	handles[0].addr = XMS_BASE;
	handles[0].size = XMS_WRAP_SIZE;
	handles[0].in_use = TRUE;
	handles[0].free = FALSE;
	
	handles[1].addr = XMS_BASE+XMS_WRAP_SIZE;
	handles[1].size = XMS_SIZE-XMS_WRAP_SIZE;
	handles[1].in_use = TRUE;
	handles[1].free = TRUE;
}

boolean_t 
A20_on() 
{
	boolean_t was_on = A20_On_Flag;
	if (!A20_On_Flag)
		memory_object_remap(ems_obj, 0, XMS_BASE, XMS_WRAP_SIZE);
	A20_On_Flag = TRUE;
	return (was_on);
}

boolean_t
A20_off() 
{
	boolean_t was_on = A20_On_Flag;
	if (A20_On_Flag)
		memory_object_remap(wrap_around_obj,0,XMS_BASE,XMS_WRAP_SIZE);
	A20_On_Flag = FALSE;
	return (was_on);
}

void
xms_setup()
{
	static done = FALSE;
	vm_address_t addr;
	boolean_t was_on;

	if (done) return;
	done = TRUE;

	/* return; */
	was_on = A20_on();

	Debug0((dbg_fd,"About to bcopy: %x, %x\n",
			pseudo_vdisk_partition, XMS_BASE));

	bcopy(pseudo_vdisk_partition, XMS_BASE, 0x20);

#ifdef NOVEMBER_91
	addr = mon_space;

	Debug0((dbg_fd,"About to bcopy, addr = %x\n",addr));

	bcopy(pseudo_vdisk_driver, addr, 0x30);
#endif NOVEMBER_91

	addr = (vm_address_t)(0x19*4);

	Debug0((dbg_fd,"About to store, addr = %x\n",addr));

#ifdef NOVEMBER_91
	*(u_long *)addr = (mon_space<<12)+0x0008;
#else
	*(u_long *)addr = Abs2Segoff(VDISK_HEADER+0x8);
#endif NOVEMBER_91

	Debug0((dbg_fd,"We stored = %x\n",*(u_long *)addr));

	if (!was_on) A20_off();
}

boolean_t bios_ems_fn(state)
	state_t *state;
{
	switch (HIGH(state->eax)) {
		case EMS_GET_EXTENDED_MEMORY_SIZE:
			xms_setup();
			SETWORD(&(state->eax),total_extended_memory_size);
			return (TRUE);
		case EMS_MOVE_EXTENDED_MEMORY_BLOCK: {
			u_char * gdt;
			int count;
			char * src;
			char * dst;
			boolean_t was_on;

			count = WORD(state->ecx)*2;
			gdt = (u_char *)Addr(state, es, esi);
			src = gdt_address(gdt, EMS_SRC_INDX);
			dst = gdt_address(gdt, EMS_DST_INDX);
			Debug0((dbg_fd, "EMS_MOVE: %x, %x\n", src, dst));
			if (IN_WRAP_AROUND(src) || IN_WRAP_AROUND(dst)) {
				was_on = A20_on();
			}
			bcopy(src, dst, count);
			if (IN_WRAP_AROUND(src) || IN_WRAP_AROUND(dst)) {
				if (!was_on) A20_off();
			}
			return (TRUE);
		}
		default:
		    return (FALSE);
	}
}

#define XMS_DRIVER_PRESENT		0x00
#define	XMS_DRIVER_ADDRESS		0x10

#define XMS_DRIVER_SEGMENT		Segment(XMS_DRIVER)
#define	XMS_DRIVER_OFFSET		Offset(XMS_DRIVER)

boolean_t bios_xms_driver(state)
	state_t *state;
{
	switch (LOW(state->eax)) {
		case XMS_DRIVER_PRESENT:
		    SETLOW(&(state->eax), 0x80);
		    return (TRUE); 
		case XMS_DRIVER_ADDRESS:
		    SETWORD(&(state->es), XMS_DRIVER_SEGMENT);
		    SETWORD(&(state->ebx), XMS_DRIVER_OFFSET);
		    return (TRUE);
		default:
		    return (FALSE);
	}
}

#define	 XMS_GET_VERSION		0x00
#define	 XMS_ALLOCATE_HIGH_MEMORY	0x01
#define	 XMS_FREE_HIGH_MEMORY		0x02
#define	 XMS_GLOBAL_ENABLE_A20		0x03
#define	 XMS_GLOBAL_DISABLE_A20		0x04
#define	 XMS_LOCAL_ENABLE_A20		0x05
#define	 XMS_LOCAL_DISABLE_A20		0x06
#define	 XMS_QUERY_A20			0x07
#define	 XMS_QUERY_FREE_EXTENDED_MEMORY	0x08
#define	 XMS_ALLOCATE_EXTENDED_MEMORY	0x09
#define	 XMS_FREE_EXTENDED_MEMORY	0x0a
#define	 XMS_MOVE_EXTENDED_MEMORY_BLOCK	0x0b
#define	 XMS_LOCK_EXTENDED_MEMORY_BLOCK	0x0c
#define	 XMS_UNLOCK_EXTENDED_MEMORY_BLOCK 0x0d
#define	 XMS_GET_EMB_HANDLE_INFORMATION	0x0e
#define	 XMS_RESIZE_EXTENDED_MEMORY_BLOCK 0x0f
#define	 XMS_ALLOCATE_UMB		0x10
#define	 XMS_DEALLOCATE_UMB		0x11

boolean_t	high_memory_in_use = FALSE;

#define	HIGH_MEMORY_IN_USE		0x92
#define	HIGH_MEMORY_NOT_ALLOCATED	0x93
#define XMS_OUT_OF_SPACE		0xa0
#define XMS_INVALID_HANDLE		0xa2

boolean_t bios_xms_fn(state)
	state_t *state;
{
	int ret;
	boolean_t retval;

	if (us_debug_level) {
		u_short * sp;
		u_short cs, eip;
		sp = (u_short *)Addr(state, ss, uesp);
		eip = *sp++; cs = *sp;
		Debug0((dbg_fd, "Entering bios_xms_fn, old_eip = %x, %x\n",
					cs, eip));

		fflush(dbg_fd);
	}

	switch (HIGH(state->eax)) {
		case XMS_GET_VERSION:
		    SETWORD(&(state->eax), XMS_VERSION);
		    SETWORD(&(state->ebx), XMS_VERSION);
		    SETWORD(&(state->edx), 1);
		    retval = TRUE;
		    break;

		case XMS_ALLOCATE_HIGH_MEMORY:
		    if (high_memory_in_use) {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), HIGH_MEMORY_IN_USE);
			retval = FALSE;
		    } else {
			SETWORD(&(state->eax), 1);
			high_memory_in_use = TRUE;
			retval = TRUE;
		    }
		    break;

		case XMS_FREE_HIGH_MEMORY:
		    if (high_memory_in_use) {
			SETWORD(&(state->eax), 1);
			high_memory_in_use = TRUE;
			retval = TRUE;
		    } else {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), HIGH_MEMORY_NOT_ALLOCATED);
			retval = FALSE;
		    }
		    break;

		case XMS_GLOBAL_ENABLE_A20:
		case XMS_LOCAL_ENABLE_A20:
		    SETWORD(&(state->eax), 1);
		    A20_on();
		    retval = TRUE;
		    break;

		case XMS_GLOBAL_DISABLE_A20:
		case XMS_LOCAL_DISABLE_A20:
		    SETWORD(&(state->eax), 1);
		    A20_off();
		    retval = TRUE;
		    break;

		case XMS_QUERY_A20:
		    if (A20_On_Flag) {
		    	SETWORD(&(state->eax), 1);
		    } else {
		    	SETWORD(&(state->eax), 0);
		    	SETLOW(&(state->ebx), 0);
		    }
		    Debug0((dbg_fd, "Query A20: %x\n",WORD(state->eax)));
		    retval = TRUE;
		    break;

		case XMS_QUERY_FREE_EXTENDED_MEMORY:
		    SETWORD(&(state->eax), xms_query()/1024);
		    SETWORD(&(state->edx), total_extended_memory_size);
		    retval = TRUE;
		    break;

		case XMS_ALLOCATE_EXTENDED_MEMORY:
		    Debug0((dbg_fd, "Allocate extended memory: %x\n",
				     WORD(state->edx)));
		    ret = xms_allocate(WORD(state->edx));
		    Debug0((dbg_fd, "Allocate extended memory returns: %x\n",
				     ret));
		    if (ret == XMS_NO_HANDLE) {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), XMS_OUT_OF_SPACE);
			retval = FALSE;
		    } else {
			SETWORD(&(state->eax), 1);
			SETWORD(&(state->edx), ret);
			retval = TRUE;
		    }
		    break;

		case XMS_FREE_EXTENDED_MEMORY:
		    Debug0((dbg_fd, "Free extended memory: %x\n",
				     WORD(state->edx)));
		    ret = xms_free(WORD(state->edx));
		    if (ret == XMS_NO_HANDLE) {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), XMS_INVALID_HANDLE);
			retval = FALSE;
		    } else {
			SETWORD(&(state->eax), 1);
			retval = TRUE;
		    }
		    break;

		case XMS_MOVE_EXTENDED_MEMORY_BLOCK:
	        {
		    char * ptr;
		    ptr = (char *) Addr(state,ds,esi);
		    ret = xms_move(*(u_short *)(ptr+4),
				   *(u_long  *)(ptr+6),
				   *(u_short *)(ptr+10),
				   *(u_long  *)(ptr+12),
				   *(u_long  *)(ptr));
							
		    if (ret == XMS_NO_HANDLE) {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), XMS_INVALID_HANDLE);
			retval = FALSE;
		    } else {
			SETWORD(&(state->eax), 1);
			retval = TRUE;
		    }
		}
		    break;

		case XMS_LOCK_EXTENDED_MEMORY_BLOCK:
		{
		    int handle = WORD(state->edx);
		    ret = xms_lock(WORD(state->edx));
		    if (ret == XMS_NO_HANDLE) {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), XMS_INVALID_HANDLE);
			retval = FALSE;
		    } else {
			SETWORD(&(state->eax), 1);
			SETWORD(&(state->edx), handles[handle].addr >> 16);
			SETWORD(&(state->ebx), handles[handle].addr & 0xffff);
			retval = TRUE;
		    }
	        }
		    break;

		case XMS_UNLOCK_EXTENDED_MEMORY_BLOCK:
		    ret = xms_free(WORD(state->edx));
		    if (ret == XMS_NO_HANDLE) {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), XMS_INVALID_HANDLE);
			retval = FALSE;
		    } else {
			SETWORD(&(state->eax), 1);
			retval = TRUE;
		    }
		    break;

		case XMS_GET_EMB_HANDLE_INFORMATION:
		{
		    int handle = WORD(state->edx);
		    ret = xms_check_handle(handle);
		    if (ret == XMS_NO_HANDLE) {
			SETWORD(&(state->eax), 0);
			SETLOW(&(state->ebx), XMS_INVALID_HANDLE);
			retval = FALSE;
		    } else {
			SETWORD(&(state->eax), 1);
			SETHIGH(&(state->ebx), handles[handle].lock_cnt);
			SETLOW(&(state->ebx), handles_available);
			SETWORD(&(state->edx), handles[handle].size/1024);
			retval = TRUE;
		    }
	        }
		    break;

		case XMS_ALLOCATE_UMB:
		{
			int size = WORD(state->edx)*16;
			vm_address_t addr = umb_allocate(size);
		    	Debug0((dbg_fd, "Allocate UMB memory: %x\n",
					     WORD(state->edx)));
			if (addr == (vm_address_t)0) {
				SETWORD(&(state->eax),0);
				SETLOW(&(state->ebx),0xb0);
				SETWORD(&(state->edx),umb_query()>>4);
			} else {
				SETWORD(&(state->eax),1);
				SETWORD(&(state->ebx),addr>>4);
				SETWORD(&(state->edx),size>>4);
			}
		    	Debug0((dbg_fd, "umb_allocated: %x, %x\n",
				     WORD(state->ebx), WORD(state->edx)));
			retval = UNCHANGED;
			break;
		}

		case XMS_DEALLOCATE_UMB:
		{
			umb_free(WORD(state->edx));
			SETWORD(&(state->eax),1);
			retval = UNCHANGED;
			break;
		}

		case XMS_RESIZE_EXTENDED_MEMORY_BLOCK:
		default:
		    SETWORD(&(state->eax), 0);
		    SETLOW(&(state->ebx), 0x80);
		    retval = FALSE;
		    break;
	    }
	Debug0((dbg_fd,"bios_xms retval = %s\n", ((retval == TRUE) ? "True" :
						   ((retval == FALSE) ? "False" :
						    "Unchanged")) ));
	return(retval);
}
