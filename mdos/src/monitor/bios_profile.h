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
 * The Mdos profiling header file.
 *
 *
 * HISTORY:
 * $Log:	bios_profile.h,v $
 * Revision 2.2  92/02/14  17:44:43  grm
 * 	Created.
 * 	[92/02/12  16:05:36  grm]
 * 
 *
 */

struct profile_type {
	u_long invoked;
	u_long max_time;
	u_long ave_time;
};

#define PROF_INTS	0x100
#define PROF_AX		0x100

#define DOS21_NAMES	0x6b

#define BIOS13_NAMES	0x1b
