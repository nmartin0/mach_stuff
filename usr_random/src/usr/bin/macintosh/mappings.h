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
 *	File: emul/server/mappings.h
 *	Author: David E. Bohman II (CMU macmach)
 */

#define SLOT_NUM_LOW	0x9
#define SLOT_NUM_HIGH	0xe

/*
 * Convert a slot number into a slot
 * base address (slot space)
 */
#define slot_to_ptr(slot)	\
    ((vm_offset_t)(0xf0000000 | ((slot) << 24)))

#define ROM_ADDR 	0x40800000
#define ROM_COMPAT_ADDR	(ROM_ADDR&0x00ffffff)
#define ROM_SIZE 	(512*1024)

#define SLOT_ADDR		slot_to_ptr(slot)
#define SLOT_SIZE		(16*1024*1024)
#define SLOT_COMPAT_ADDR(slot)	((slot)<<20)
#define SLOT_COMPAT_SIZE	(1024*1024)

#define SLOT_SPACE_BASE		SLOT_ADDR(SLOT_NUM_LOW)
#define SLOT_SPACE_EXTENT	((SLOT_NUM_HIGH-SLOT_NUM_LOW+1)*SLOT_SIZE)

#define SLOT_MINOR_SPACE_BASE	SLOT_COMPAT_ADDR(SLOT_NUM_LOW)
#define SLOT_MINOR_SPACE_EXTENT \
	((SLOT_NUM_HIGH-SLOT_NUM_LOW+1)*SLOT_COMPAT_SIZE)
