/*
 * Copyright (c) 1992 Carnegie Mellon University
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
 * The Mdos condition header file.
 *
 * HISTORY:
 * $Log:	bios_cond.h,v $
 * Revision 2.2  92/07/01  14:24:35  grm
 * 	Ifdef out idle code.
 * 	[92/07/01            grm]
 * 	Created.
 * 	[92/06/30  13:46:39  grm]
 * 
 *
 */
#ifdef	IDLE_WORK_IN_PROGRESS
typedef struct condition {
	int	lock;
	char *	name;
} * condition_t;
#endif	/* IDLE_WORK_IN_PROGRESS */
