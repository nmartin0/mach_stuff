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
 * $Log:	vm_param.h,v $
 * Revision 2.2  91/09/12  16:53:29  bohman
 * 	Created Mach 3.0 version from 2.5 version.
 * 	[91/09/11  17:16:38  bohman]
 * 
 * Revision 2.2  90/08/30  17:52:05  bohman
 * 	Created.
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mach/mac2/vm_param.h
 *	Author: David E. Bohman II (CMU macmach)
 */

#ifndef _MACH_MAC2_VM_PARAM_H_
#define _MACH_MAC2_VM_PARAM_H_

#include <mac2/mach_param.h>

#define BYTE_SIZE	8	/* byte size in bits */

#define PAGE_SIZE_FIXED

#ifdef SMALLPAGE
#define PAGE_SIZE  4096 /* bytes per page */
#define PAGE_SHIFT 12   /* number of bits to shift for pages */
#else
#define PAGE_SIZE  8192 /* bytes per page */
#define PAGE_SHIFT 13   /* number of bits to shift for pages */
#endif

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define	mac2_btop(x)		(((unsigned)(x)) >> PAGE_SHIFT)
#define	mac2_ptob(x)		(((unsigned)(x)) << PAGE_SHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define mac2_round_page(x)	((((unsigned)(x)) + PAGE_SIZE - 1) & \
					~(PAGE_SIZE-1))
#define mac2_trunc_page(x)	(((unsigned)(x)) & ~(PAGE_SIZE-1))

/*
 * Macintosh II implementation
 * uses separate user and
 * kernel address spaces.
 *
 */
#define	VM_MIN_ADDRESS	((vm_offset_t) 0)
#define	VM_MAX_ADDRESS	((vm_offset_t) 0xffffffff)

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x0)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xffffffff)

#define	KERNEL_STACK_SIZE	PAGE_SIZE

#define INTSTACK_SIZE		PAGE_SIZE

/*
 *	Conversion between mac2 pages and VM pages
 */

#define trunc_mac2_to_vm(p)	(atop(trunc_page(mac2_ptob(p))))
#define round_mac2_to_vm(p)	(atop(round_page(mac2_ptob(p))))
#define vm_to_mac2(p)		(mac2_btop(ptoa(p)))

#endif	_MACH_MAC2_VM_PARAM_H_
