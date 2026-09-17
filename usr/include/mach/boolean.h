/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: boolean.h,v $
 * Revision 1.2.6.1  1994/09/23  02:34:07  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:39:00  ezf]
 *
 * Revision 1.2.2.3  1993/08/03  18:22:11  gm
 * 	CR9598: Remove unneeded EXPORT_BOOLEAN and KERNEL ifdefs.  Move
 * 	the code inside the include protection and remove the boolean_t
 * 	casts from TRUE and FALSE.
 * 	[1993/08/02  17:49:29  gm]
 * 
 * Revision 1.2.2.2  1993/06/09  02:39:27  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:15:31  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:31:43  devrcs
 * 	ansi C conformance changes
 * 	[1993/02/02  18:52:46  david]
 * 
 * Revision 1.1  1992/09/30  02:30:33  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.3  91/05/14  16:51:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:31:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:16:36  mrt]
 * 
 * Revision 2.1  89/08/03  15:59:35  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:12:08  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:51:34  mwyoung
 * Relocated from sys/boolean.h
 * 
 * Revision 2.2  88/08/24  02:23:06  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:09:46  mwyoung]
 * 
 *
 * 18-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Header file fixup, purge history.
 *
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
/*
 *	File:	mach/boolean.h
 *
 *	Boolean data type.
 *
 */

#ifndef	BOOLEAN_H_
#define	BOOLEAN_H_

/*
 *	Pick up "boolean_t" type definition
 */

#ifndef	ASSEMBLER
#include <mach/machine/boolean.h>
#endif	/* ASSEMBLER */

/*
 *	Define TRUE and FALSE, only if they haven't been before,
 *	and not if they're explicitly refused.
 */

#ifndef	NOBOOL

#ifndef	TRUE
#define TRUE	1
#endif	/* TRUE */

#ifndef	FALSE
#define FALSE	0
#endif	/* FALSE */

#endif	/* !defined(NOBOOL) */

#endif	/* BOOLEAN_H_ */
