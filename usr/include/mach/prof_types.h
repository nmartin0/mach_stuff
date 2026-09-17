/*
 * @OSF_FREE_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: prof_types.h,v $
 * Revision 1.2.7.2  1995/01/26  22:15:46  ezf
 * 	corrected CR
 * 	[1995/01/26  21:16:02  ezf]
 *
 * Revision 1.2.3.2  1993/06/09  02:43:16  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:18:04  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:39:03  devrcs
 * 	ansi C conformance changes
 * 	[1993/02/02  18:54:26  david]
 * 
 * Revision 1.1  1992/09/30  02:32:04  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 */

#ifndef	_MACH_PROF_TYPES_H
#define	_MACH_PROF_TYPES_H

#define	SAMPLE_MAX	256	/* Max array size */
typedef unsigned	sample_array_t[SAMPLE_MAX];

#endif	/* _MACH_PROF_TYPES_H */
