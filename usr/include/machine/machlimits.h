/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: machlimits.h,v $
 * Revision 1.1.2.1  1996/09/17  16:31:53  bruel
 * 	moved from OSF1 server.
 * 	[96/09/17            bruel]
 *
 * Revision 1.1.9.2  1995/08/21  19:25:37  devrcs
 * 	ri-osc CR1556 - fix warning in libsa_mach by fixing decls of
 * 	unsigned constants to be unsigned, e.g.: 0x12345678U.
 * 	HP/PA change made to parallel i860 and i386.
 * 	[1995/08/12  09:47:25  emcmanus]
 * 
 * Revision 1.1.9.1  1995/03/20  17:42:35  emcmanus
 * 	CR1149: Merged HP/PA version into mainline.
 * 	[1995/03/20  17:35:05  emcmanus]
 * 
 * Revision 1.1.5.1  1995/02/14  14:46:28  bruel
 * 	hppa merge
 * 	[1995/02/13  14:49:13  bruel]
 * 
 * Revision 1.1.2.3  1994/03/17  17:57:54  jose
 * 	Creted from i386/machlimits.h
 * 	[94/03/15            jose]
 * 
 * Revision 3.0.2.2  1993/06/03  03:09:47  gm
 * 	Moved from /kernel to /osf1_server.
 * 	[1993/06/03  00:45:39  gm]
 * 
 * Revision 3.0  1992/12/31  22:00:17  ede
 * 	Initial revision for OSF/1 R1.3
 * 
 * Revision 1.8.2.2  1992/04/16  17:43:19  lehotsky
 * 	#6303 - need parentheses
 * 	[1992/04/14  18:14:11  lehotsky]
 * 
 * Revision 1.8  1991/08/15  19:15:59  devrcs
 * 	Added IBM's modifications for NLS subsystem support.
 * 	[91/03/25  15:42:24  kathyg]
 * 
 * Revision 1.7  91/03/23  17:08:06  devrcs
 * 	ANSI says the macros must be suitable arguments for #IF directives.
 * 	This means that type-casts aren't permitted
 * 	[91/03/07  13:38:13  lehotsky]
 * 
 * 	Kernels won't build cuz they don't like the 'signed' keyword
 * 	[91/03/05  13:46:40  lehotsky]
 * 
 * 	Added cast to (signed int) to force INT_MIN and LONG_MIN to
 * 	be signed values.
 * 	[91/03/01  13:26:41  lehotsky]
 * 
 * Revision 1.6  90/10/31  13:56:39  devrcs
 * 	Moved CLOCKS_PER_SEC to machtime.h.
 * 	[90/10/06  11:57:35  rabin]
 * 
 * Revision 1.5  90/10/07  13:42:20  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  09:36:38  gm]
 * 
 * Revision 1.4  90/09/13  11:44:42  devrcs
 * 	Fix "min values" to be signed, to keep VSX happy.
 * 	[90/08/29  20:44:39  lehotsky]
 * 
 * 	Change CLOCKS_PER_SEC to match that defined in param.c.
 * 	[90/08/21  10:27:11  brezak]
 * 
 * Revision 1.3  90/07/17  11:31:15  devrcs
 * 	Updates for SS4
 * 	[90/06/29  09:24:27  kevins]
 * 
 * Revision 1.2  90/04/27  19:07:19  devrcs
 * 	First placed in i386
 * 	[90/04/18  17:13:04  jd]
 * 
 * Revision 1.3  90/03/13  20:48:40  mbrown
 * 	AIX merge first cut.
 * 	[90/02/13  16:15:08  tom]
 * 
 * Revision 1.2  90/01/03  11:57:15  gm
 * 	Fixes for first snapshot.
 * 	[90/01/03  09:31:58  gm]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)machlimits.h	7.1 (Berkeley) 2/15/89
 */
#ifndef _MACH_MACHLIMITS_H_
#define _MACH_MACHLIMITS_H_

#define	CHAR_BIT	8		/* number of bits in a char */

#define	SCHAR_MAX	127		/* max value for a signed char */
#define	SCHAR_MIN	(-128)		/* min value for a signed char */

#define	UCHAR_MAX	255U		/* max value for an unsigned char */
#define	CHAR_MAX	127		/* max value for a char */
#define	CHAR_MIN	(-128)		/* min value for a char */

#define	USHRT_MAX	65535U		/* max value for an unsigned short */
#define	SHRT_MAX	32767		/* max value for a short */
#define	SHRT_MIN	(-32768)	/* min value for a short */

#define	UINT_MAX	0xFFFFFFFFU	/* max value for an unsigned int */
#define	INT_MAX		2147483647	/* max value for an int */
#define	INT_MIN		(-2147483647-1)	/* min value for an int */

#define	ULONG_MAX	UINT_MAX	/* max value for an unsigned long */
#define	LONG_MAX	INT_MAX		/* max value for a long */
#define	LONG_MIN	INT_MIN		/* min value for a long */

/* Must be at least two, for internationalization (NLS/KJI) */
#define MB_LEN_MAX	4		/* multibyte characters */

#endif /* _MACH_MACHLIMITS_H_ */
