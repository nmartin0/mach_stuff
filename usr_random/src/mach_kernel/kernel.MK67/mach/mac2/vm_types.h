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
 * $Log:	vm_types.h,v $
 * Revision 2.2  91/09/12  16:53:38  bohman
 * 	Created Mach 3.0 version from 2.5 version.
 * 	[91/09/11  17:17:13  bohman]
 * 
 * Revision 2.2  90/08/30  17:52:27  bohman
 * 	Created.
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mach/mac2/vm_types.h
 */

#ifndef	_MACH_MAC2_VM_TYPES_H_
#define	_MACH_MAC2_VM_TYPES_H_

#ifdef	ASSEMBLER
#else	ASSEMBLER
typedef	unsigned int	vm_offset_t;
typedef	unsigned int	vm_size_t;
#endif	ASSEMBLER

#endif	_MACH_MAC2_VM_TYPES_H_
