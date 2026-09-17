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
 *	File: macintosh/pager_defs.h
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mac2/queue.h>

typedef unsigned char *bitmap_ptr_t;
#define BITMAP_PTR_NULL	((bitmap_ptr_t)0)

typedef struct {
    vm_offset_t		offset;
    int			type;
#define PAGING_OBJ_TYPE_NULL	0
#define PAGING_OBJ_TYPE_FILE	1
    unsigned long	data;
} paging_obj_t, *paging_obj_ptr_t;

typedef struct {
    queue_chain_t	link;
    vm_offset_t		object_offset;
    vm_size_t		length;
    vm_prot_t		max_permission;
    paging_obj_t	i_paging_obj;
    paging_obj_t	w_paging_obj;
    bitmap_ptr_t	w_paging_map;
} paging_region_t, *paging_region_ptr_t;
#define PAGING_REGION_PTR_NULL	((paging_region_ptr_t)0)

typedef struct {
    queue_head_t	regions;
} addressing_region_t;

#define ADDRESSING_REGION_NUMB	256

#define ADDRESSING_REGION_SIZE	(1 << 24)

#define ADDRESSING_REGION(a)	((int)((a) >> 24))

#define round_region(x)	(((vm_offset_t)(x)			\
			  + (ADDRESSING_REGION_SIZE - 1))	\
			 & ~(ADDRESSING_REGION_SIZE - 1))

extern addressing_region_t	*paging_data;

#include "pager_bitmap_inline.c"

extern paging_region_ptr_t	pager_lookup_region(vm_offset_t, vm_size_t);
extern void			pager_new_region(vm_offset_t, vm_size_t,
						 vm_prot_t,
						 vm_offset_t,
						 int, unsigned long,
						 vm_offset_t,
						 int, unsigned long);
extern void			pager_map_region(vm_offset_t, vm_size_t,
						 vm_prot_t,
						 vm_offset_t,
						 vm_inherit_t);
