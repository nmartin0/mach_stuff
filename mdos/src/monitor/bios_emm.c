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
 *	Mach EMM Memory Manager
 *
 * HISTORY: 
 * $Log:	bios_emm.c,v $
 * Revision 2.4  92/03/02  15:47:09  grm
 * 	Minor changes to work with MK69.
 * 	[92/02/20            grm]
 * 
 * Revision 2.3  92/02/03  14:24:36  rvb
 * 	Clean Up
 * 
 * Revision 2.2  91/12/05  16:40:08  grm
 * 	Put bios_emm_init back.
 * 	[91/06/28  17:56:59  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  14:43:21  grm]
 * 
 * 	Filled in 3.2 support.  Made more robust.
 * 	Put in error support.
 * 	[91/04/30  13:25:28  grm]
 * 
 * 	created
 * 	[91/02/01  13:24:12  grm]
 * 
 */

#include "base.h"
#include "bios.h"

#include <sys/file.h>
#include <sys/ioctl.h>


#define	GET_MANAGER_STATUS	0x40		/* V3.0 */
#define	GET_PAGE_FRAME_SEGMENT	0x41		/* V3.0 */
#define	GET_PAGE_COUNTS		0x42		/* V3.0 */
#define	GET_HANDLE_AND_ALLOCATE	0x43		/* V3.0 */
#define MAP_UNMAP		0x44		/* V3.0 */
#define	DEALLOCATE_HANDLE	0x45		/* V3.0 */
#define	GET_EMM_VERSION		0x46		/* V3.0 */
#define	SAVE_PAGE_MAP		0x47		/* V3.0 */
#define RESTORE_PAGE_MAP	0x48		/* V3.0 */
#define	RESERVED_1		0x49		/* V3.0 */
#define	RESERVED_2		0x4a		/* V3.0 */
#define	GET_HANDLE_COUNT	0x4b		/* V3.0 */
#define GET_PAGES_OWNED		0x4c		/* V3.0 */
#define	GET_PAGES_FOR_ALL	0x4d		/* V3.0 */
#define	PAGE_MAP_REGISTERS	0x4e		/* V3.2 */
#define	GET_REGISTERS		0x0
#define SET_REGISTERS		0x1
#define	GET_AND_SET_REGISTERS	0x2
#define GET_SIZE_FOR_PAGE_MAP	0x3

#define	EMM_BASE_ADDRESS 0xd0000
#define	EMM_SEGMENT 	 0xd000

#define	MAX_HANDLES	256
#define MAX_EMM		64
#define	EMM_PAGE_SIZE	(16*1024)
#define EMM_MAX_PHYS	4
#define NULL_HANDLE	-1
#define	NULL_PAGE	0xffff

/* Support EMM version 3.2 */
#define EMM_VERS	0x32

/* EMM errors */
#define EMM_NO_ERR	0x0
#define EMM_INV_HAN	0x83
#define EMM_FUNC_NOSUP	0x84
#define EMM_OUT_OF_HAN	0x85
#define EMM_OUT_OF_PHYS	0x87
#define EMM_OUT_OF_LOG	0x88
#define EMM_ZERO_PAGES	0x89
#define EMM_LOG_OUT_RAN	0x8a
#define EMM_ILL_PHYS	0x8b

#define EMM_ERROR -1
u_char emm_error;

int handle_total = 0;
int emm_allocated = 0;
int emm_total = MAX_EMM;

struct emm_record {
	int handle;
	int logical_page;
} emm_map[EMM_MAX_PHYS];

struct handle_record {
	int numpages;
	mach_port_t object;
	int saved_mappings[EMM_MAX_PHYS];
} handle_info[MAX_HANDLES];

void bios_emm_init()
{
	int i;

	for (i=0; i < EMM_MAX_PHYS; i++) {
		emm_map[i].handle = NULL_HANDLE;
	}
}

int allocate_handle(pages_needed)
	int pages_needed;
{
	int i, j;
	if (handle_total >= MAX_HANDLES) {
		emm_error = EMM_OUT_OF_HAN;
		return (EMM_ERROR);
	}
	if (pages_needed > (emm_total-emm_allocated)) {
		emm_error = EMM_OUT_OF_PHYS;
		return (EMM_ERROR);
	}
	for (i = 0; i < MAX_HANDLES; i++) {
		if (handle_info[i].numpages == 0) {
			handle_info[i].object = 
				new_memory_object(pages_needed*EMM_PAGE_SIZE);
			handle_info[i].numpages = pages_needed;
			handle_total++;
			emm_allocated += pages_needed;
			for (j = 0; j < EMM_MAX_PHYS; j++) {
				handle_info[i].saved_mappings[j] = NULL_PAGE;
			}
			return(i);
		}
	}
	emm_error = EMM_OUT_OF_HAN;
	return (EMM_ERROR);
}

boolean_t deallocate_handle(handle)
	int handle;
{
	int numpages, i;
	mach_port_t object;
	if ((handle < 0) || (handle >= MAX_HANDLES)) return (FALSE);
	for (i = 0; i < EMM_MAX_PHYS; i++) {
		if (emm_map[i].handle == handle)
			emm_map[i].handle = NULL_HANDLE;		
	}
	numpages = handle_info[handle].numpages;
	object = handle_info[handle].object;
	destroy_memory_object(object);
	handle_info[handle].numpages = 0;
	handle_info[handle].object = MACH_PORT_NULL;
	handle_total--;
	emm_allocated -= numpages;
	return (TRUE);
}

boolean_t unmap_page(physical_page)
	int physical_page;
{
	if ((physical_page < 0) || (physical_page >= EMM_MAX_PHYS)) 
		return (FALSE);
	if (emm_map[physical_page].handle < 0) return (FALSE);
	MACH_CALL((vm_deallocate(mach_task_self(),
				 EMM_BASE_ADDRESS+(physical_page*EMM_PAGE_SIZE),
				 EMM_PAGE_SIZE)), "unmap");
	emm_map[physical_page].handle = NULL_HANDLE;
	emm_map[physical_page].logical_page = NULL_PAGE;
	return(TRUE);
}

boolean_t map_page(handle, physical_page, logical_page)
	int handle;
	int physical_page;
	int logical_page;
{
	if ((physical_page < 0) || (physical_page >= EMM_MAX_PHYS)) 
		return (FALSE);
	if ((emm_map[physical_page].handle == handle) &&
	    (emm_map[physical_page].logical_page == logical_page)) {
		return(TRUE);
	}
	memory_object_remap(
			    handle_info[handle].object,
			    logical_page*EMM_PAGE_SIZE,
			    EMM_BASE_ADDRESS+(physical_page*EMM_PAGE_SIZE),
			    EMM_PAGE_SIZE
			    );
	emm_map[physical_page].handle = handle;
	emm_map[physical_page].logical_page = logical_page;
	return(TRUE);
}

int handle_pages(handle)
	int handle;
{
	return (handle_info[handle].numpages);
}

int save_handle_state(handle)
	int handle;
{	
	int i;
	for (i = 0; i < EMM_MAX_PHYS; i++) {
		if (emm_map[i].handle == handle) {
			handle_info[handle].saved_mappings[i] = 
				emm_map[i].logical_page;
		} else {
			handle_info[handle].saved_mappings[i] = NULL_PAGE;
		}
	}
}

int restore_handle_state(handle)
	int handle;
{
	int i;
	for (i = 0; i < EMM_MAX_PHYS; i++) {
		int saved_mapping;
		saved_mapping = handle_info[handle].saved_mappings[i];
		if (saved_mapping != NULL_PAGE) {
			map_page(handle, i, saved_mapping);
		}
	}
}

void test_handle(handle, numpages)
	int handle;
{
	int i;

	for (i = 0; i < numpages; i++) {
		map_page(handle, 0, i);
		*(int *) EMM_BASE_ADDRESS = i;
	}
	for (i = 0; i < numpages; i++) {
		map_page(handle, 0, i);
		if ((*(int *) EMM_BASE_ADDRESS) != i) {
			printf("bios_emm: test_handle, Mapping failure: %d, %d\n",
			       i, (*(int *)EMM_BASE_ADDRESS));
		}
	}
}

boolean_t bios_emm_fn(state)
	state_t *state;
{
	switch (HIGH(state->eax)) {
	    case GET_MANAGER_STATUS: {		/* 0x40 */
		    Kdebug1((dbg_fd,"bios_emm: Get Manager Status\n"));

		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    break;
	    }
	    case GET_PAGE_FRAME_SEGMENT: {	/* 0x41 */
		    Kdebug1((dbg_fd,"bios_emm: Get Page Frame Segment\n"));

		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    SETWORD(&(state->ebx), EMM_SEGMENT);
		    break;
	    }
	    case GET_PAGE_COUNTS: {		/* 0x42 */
		    u_short left = emm_total - emm_allocated;

		    Kdebug1((dbg_fd,"bios_emm: Get Page Counts left=0x%x\n",
			     left));

		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    SETWORD(&(state->ebx), left);
		    SETWORD(&(state->edx), emm_total);
		    break;
	    }
	    case GET_HANDLE_AND_ALLOCATE: {	/* 0x43 */
		    int pages_needed = WORD(state->ebx);
		    int handle;

		    Kdebug1((dbg_fd,"bios_emm: Get Handle and Allocate pages = 0x%x\n",
			     pages_needed));

		    if (pages_needed == 0) {
			    SETHIGH(&(state->eax), EMM_ZERO_PAGES);
			    return(UNCHANGED);
		    }

		    if ((handle = allocate_handle(pages_needed)) == EMM_ERROR) {
			    SETHIGH(&(state->eax), emm_error);
			    return(UNCHANGED);
		    }

		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    SETWORD(&(state->edx), handle);

		    Kdebug1((dbg_fd,"bios_emm: handle = 0x%x\n",handle));

		    break;
	    }
	    case MAP_UNMAP: {			/* 0x44 */
		    int physical_page = LOW(state->eax);
		    int logical_page = WORD(state->ebx);
		    int handle = WORD(state->edx);

		    Kdebug1((dbg_fd,"bios_emm: Map Unmap, phys = 0x%x log = 0x%x han = 0x%x\n",
			     physical_page, logical_page, handle));

		    if ((handle < 0) || (handle > MAX_HANDLES) ||
			(handle_info[handle].numpages == 0)) {
			    SETHIGH(&(state->eax), EMM_INV_HAN);
			    return(UNCHANGED);
		    }

		    if ((physical_page < 0) || (physical_page > EMM_MAX_PHYS)) {
			    SETHIGH(&(state->eax), EMM_ILL_PHYS);
			    return(UNCHANGED);
		    }
		    
		    if (logical_page == 0xffff) {
			    unmap_page(physical_page);
		    } else {
			    if(logical_page >= handle_info[handle].numpages) {
				    SETHIGH(&(state->eax), EMM_LOG_OUT_RAN);
				    return(UNCHANGED);
			    }
			    
			    map_page(handle, physical_page, logical_page);
		    }
		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    break;			
	    }
	    case DEALLOCATE_HANDLE: {		/* 0x45 */
		    int handle = WORD(state->edx);

		    Kdebug1((dbg_fd,"bios_emm: Deallocate Handle, han-0x%x\n",
			     handle));

		    if ((handle < 0) || (handle > MAX_HANDLES) ||
			(handle_info[handle].numpages == 0)) {
			    SETHIGH(&(state->eax), EMM_INV_HAN);
			    return(UNCHANGED);
		    }

		    deallocate_handle(handle);
		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    break;
	    }
	    case GET_EMM_VERSION: {		/* 0x46 */
		    Kdebug1((dbg_fd,"bios_emm: Get EMM version\n"));

		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    SETLOW(&(state->eax), EMM_VERS);
		    break;
	    }
	    case SAVE_PAGE_MAP: {		/* 0x47 */
		    int handle = WORD(state->edx);

		    Kdebug1((dbg_fd,"bios_emm: Save Page Map, han-0x%x\n",
			     handle));

		    if ((handle < 0) || (handle > MAX_HANDLES) ||
			(handle_info[handle].numpages == 0)) {
			    SETHIGH(&(state->eax), EMM_INV_HAN);
			    return(UNCHANGED);
		    }

		    save_handle_state(handle);
		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    break;
	    }
	    case RESTORE_PAGE_MAP: {		/* 0x48 */
		    int handle = WORD(state->edx);

		    Kdebug1((dbg_fd,"bios_emm: Restore Page Map, han-0x%x\n",
			     handle));

		    if ((handle < 0) || (handle > MAX_HANDLES) ||
			(handle_info[handle].numpages == 0)) {
			    SETHIGH(&(state->eax), EMM_INV_HAN);
			    return(UNCHANGED);
		    }

		    restore_handle_state(handle);
		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    break;
	    }
	    case GET_HANDLE_COUNT: {		/* 0x4b */
		    Kdebug1((dbg_fd,"bios_emm: Get Handle Count\n"));

		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    SETWORD(&(state->ebx), handle_total);

		    Kdebug1((dbg_fd,"bios_emm: handle_total = 0x%x\n",
			     handle_total));

		    break;
	    }
	    case GET_PAGES_OWNED: {		/* 0x4c */
		    int handle = WORD(state->edx);
		    u_short pages;

		    Kdebug1((dbg_fd,"bios_emm: Get Pages Owned, han-0x%x\n",
			     handle));

		    if ((handle < 0) || (handle > MAX_HANDLES) ||
			(handle_info[handle].numpages == 0)) {
			    SETHIGH(&(state->eax), EMM_INV_HAN);
			    return(UNCHANGED);
		    }

		    pages = handle_pages(handle);
		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    SETWORD(&(state->ebx), pages);

		    Kdebug1((dbg_fd,"bios_emm: pages owned - 0x%x\n",
			     pages));

		    break;
	    }
	    case GET_PAGES_FOR_ALL: {		/* 0x4d */
		    int i;
		    int * ptr = (int *) Addr(state, es, edi);

		    Kdebug1((dbg_fd,"bios_emm: Get Pages For all\n"));

		    for (i = 0; i < MAX_HANDLES; i++) {
			    if (handle_info[i].numpages > 0) {
				    *ptr = i; ptr++;
				    *ptr = handle_pages(i); ptr++;
			    }
		    }
		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    SETWORD(&(state->ebx), handle_total);

		    Kdebug1((dbg_fd,"bios_emm: total pages = 0x%x\n",
			     handle_total));

		    break;
	    }
	    case PAGE_MAP_REGISTERS: {		/* 0x4e */
		    Kdebug1((dbg_fd,"bios_emm: Page Map Registers Function.\n"));

		    switch (LOW(state->eax)) {
			case GET_REGISTERS: {
				int i;
				int * ptr = (int *) Addr(state, es, edi);

				Kdebug1((dbg_fd,"bios_emm: Get Registers\n"));

				for (i = 0; i < EMM_MAX_PHYS; i++) {
					*ptr = emm_map[i].handle; ptr++;
					*ptr = emm_map[i].logical_page; ptr++;
					Kdebug1((dbg_fd,"phy %d h %x lp %x\n",
						 i, emm_map[i].handle,
						 emm_map[i].logical_page));
				}

				break;
			}
			case SET_REGISTERS: {
				int i;
				int * ptr = (int *) Addr(state, ds, esi);
				int handle;
				int logical_page;

				Kdebug1((dbg_fd,"bios_emm: Set Registers\n"));

				for(i = 0; i < EMM_MAX_PHYS; i++) {
					handle = *ptr; ptr++;
					logical_page = *ptr; ptr++;
					map_page(handle, i, logical_page);
					Kdebug1((dbg_fd,"phy %d h %x lp %x\n",
						 i, handle, logical_page));
				}

				break;
			}
			case GET_AND_SET_REGISTERS: {
				int i;
				int * ptr;
				int handle;
				int logical_page;

				Kdebug1((dbg_fd,"bios_emm: Get and Set Registers\n"));

				ptr = (int *) Addr(state, es, edi);

				for (i = 0; i < EMM_MAX_PHYS; i++) {
					*ptr = emm_map[i].handle; ptr++;
					*ptr = emm_map[i].logical_page; ptr++;
				}

				ptr = (int *) Addr(state, ds, esi);

				for(i = 0; i < EMM_MAX_PHYS; i++) {
					handle = *ptr; ptr++;
					logical_page = *ptr; ptr++;
					map_page(handle, i, logical_page);
				}

				break;
			}
			case GET_SIZE_FOR_PAGE_MAP: {
				Kdebug1((dbg_fd,"bios_emm: Get size for page map\n"));

				SETHIGH(&(state->eax), 0);
				SETLOW(&(state->eax), sizeof(int) * 8);
				return(UNCHANGED);
			}
			default: {
				Kdebug1((dbg_fd,"bios_emm: Page Map Regs unknwn fn\n"));

				SETHIGH(&(state->eax), EMM_FUNC_NOSUP);
				return(UNCHANGED);
			}
		    }

		    SETHIGH(&(state->eax), EMM_NO_ERR);
		    break;
	    }
	    default: {
		    Kdebug1((dbg_fd,"bios_emm: EMM function not supported 0x%x\n",
			     WORD(state->eax)));

		    SETHIGH(&(state->eax), EMM_FUNC_NOSUP);
		    break;
	    }
	}		
	return(UNCHANGED);
}
