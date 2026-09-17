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
 *	File: emul/server/pager_support.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include "server_defs.h"
#include "pager_defs.h"

addressing_region_t	*paging_data;

pager_setup()
{
    register i;

    paging_data =
	(addressing_region_t *)calloc(ADDRESSING_REGION_NUMB,
				      sizeof (addressing_region_t));
    if (paging_data == 0)
	other_error_exit("calloc paging_data");

    for (i = 0; i < ADDRESSING_REGION_NUMB; i++)
	queue_init(&paging_data[i].regions);
}

paging_region_ptr_t
pager_lookup_region(offset, length)
register vm_offset_t	offset;
register vm_size_t	length;
{
    register queue_t	q = &paging_data[ADDRESSING_REGION(offset)].regions;
    register queue_entry_t		qe;
    register paging_region_ptr_t	pr;

    if (queue_empty(q))
	return (PAGING_REGION_PTR_NULL);

    for (qe = queue_first(q); !queue_end(q, qe); qe = queue_next(qe)) {
	pr = (paging_region_ptr_t)qe;
	if (pr->object_offset <= offset &&
	    (pr->object_offset + pr->length) >= (offset + length))
	    return (pr);
    }

    return (PAGING_REGION_PTR_NULL);
}

void
pager_new_region(offset, length, permission,
		 i_paging_offset, i_type, i_data,
		 w_paging_offset, w_type, w_data)
register vm_offset_t	offset;
register vm_size_t	length;
vm_prot_t		permission;
vm_offset_t		i_paging_offset;
int			i_type;
unsigned long		i_data;
vm_offset_t		w_paging_offset;
int			w_type;
unsigned long		w_data;
{
    register queue_t	q = &paging_data[ADDRESSING_REGION(offset)].regions;
    register queue_entry_t		qe;
    register paging_region_ptr_t	prn, pr;
    register vm_offset_t		nxt_rgn_offset;
    kern_return_t			result;

    nxt_rgn_offset = round_region(offset + 1);
    if (nxt_rgn_offset < (offset + length)) {
	(void) pager_new_region(nxt_rgn_offset,
				length - (nxt_rgn_offset - offset),
				permission,
				i_paging_offset + (nxt_rgn_offset - offset),
				i_type, i_data,
				w_paging_offset + (nxt_rgn_offset - offset),
				w_type, w_data);
	length = nxt_rgn_offset - offset;
    }

    printf("pager_new_region: offset %x length %x (%x to %x)\n",
	   offset, length, offset, offset + length);

    prn = (paging_region_ptr_t)calloc(1, sizeof (paging_region_t));
    if (prn == PAGING_REGION_PTR_NULL)
	other_error_exit("calloc paging_region failed");

    prn->object_offset = offset = trunc_page(offset);
    prn->length = length = round_page(length);
    prn->max_permission = permission;
    prn->i_paging_obj.offset = i_paging_offset;
    prn->i_paging_obj.type = i_type;
    prn->i_paging_obj.data = i_data;
    prn->w_paging_obj.offset = w_paging_offset;
    prn->w_paging_obj.type = w_type;
    prn->w_paging_obj.data = w_data;
    if (w_type != PAGING_OBJ_TYPE_NULL) {
	prn->w_paging_map =
	    (bitmap_ptr_t)calloc(roundup((length / vm_page_size), 8) / 8,
				 sizeof (unsigned char));

	if (prn->w_paging_map == BITMAP_PTR_NULL)
	    other_error_exit("calloc paging_region available bitmap");
    }

    if (queue_empty(q)) {
	q->next = q->prev = &prn->link;
	prn->link.next = q;
	prn->link.prev = q;
    }
    else {
	for (qe = queue_first(q); !queue_end(q, qe); qe = queue_next(qe)) {
	    pr = (paging_region_ptr_t)qe;
	    if (pr->object_offset > offset &&
		pr->object_offset >= (offset + length))
		break;
	    else if (pr->object_offset < offset &&
		     (pr->object_offset + pr->length) <= offset)
		continue;
	    else
		other_error_exit("pager_map_region already mapped");
	}

	pr = (paging_region_ptr_t)qe;
	prn->link.prev = pr->link.prev;
	pr->link.prev->next = &prn->link;
	pr->link.prev = &prn->link;
	prn->link.next = &pr->link;
    }
}

void
pager_map_region(offset, length, permission, address, inheritance)
vm_offset_t	offset;
vm_size_t	length;
vm_prot_t	permission;
vm_offset_t	address;
vm_inherit_t	inheritance;
{
    register paging_region_ptr_t	pr;
    register vm_offset_t		nxt_rgn_offset;
    kern_return_t			result;

    nxt_rgn_offset = round_region(offset + 1);
    if (nxt_rgn_offset < (offset + length)) {
	(void) pager_map_region(nxt_rgn_offset,
				length - (nxt_rgn_offset - offset),
				permission,
				address + (nxt_rgn_offset - offset),
				inheritance);
	length = nxt_rgn_offset - offset;
    }

    pr = pager_lookup_region(offset, length);
    if (pr == PAGING_REGION_PTR_NULL)
	other_error_exit("pager_map_region no region");

    result = vm_map(task,
		    &address,
		    length,
		    0, FALSE,
		    memory_object,
		    pr->object_offset,
		    (pr->w_paging_obj.type == PAGING_OBJ_TYPE_NULL)?
		    TRUE: FALSE,
		    permission, pr->max_permission,
		    inheritance);

    if (result != KERN_SUCCESS)
	mach_error_exit("vm_map region", result);
}
