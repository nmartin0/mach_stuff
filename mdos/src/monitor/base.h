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
 * File: $Source: /afs/cs.cmu.edu/project/mach3-rcs/mdos/src/monitor/base.h,v $
 *
 * Purpose:
 *
 * HISTORY: 
 * $Log:	base.h,v $
 * Revision 2.5  92/02/14  17:44:27  grm
 * 	Changed the exit(1) in MACH_CALL to exit_dos().
 * 	[92/02/11            grm]
 * 
 * Revision 2.4  92/02/03  14:24:29  rvb
 * 	Clean Up
 * 
 * Revision 2.3  91/12/05  16:39:31  grm
 * 	New copyright notice.
 * 	[91/05/28  08:26:05  grm]
 * 
 * 	Changes <mach_3.h> to "mach_3.h".  Nitpick.
 * 	[91/05/02  13:28:50  grm]
 * 
 * 	Redirect constant added
 * 	[91/03/26  19:28:46  grm]
 * 
 * 	Added separate debugging macros.
 * 	[90/11/09  21:02:16  grm]
 * 
 * 	added UNCHANGED
 * 	[90/10/04  20:56:47  grm]
 * 
 * 	Fixed up the SETxxx's.
 * 	[90/04/30  15:41:56  grm]
 * 
 * 	Added onoff_t, MASK16 and WORD.
 * 	[90/04/05  21:25:12  grm]
 * 
 * 	Started using as grm.  Moved into v86 branch.
 * 	[90/03/28  18:33:32  grm]
 * 
 * Revision 2.1.1.6  90/03/22  21:40:57  dorr
 * 	no changes.
 * 
 * Revision 2.1.1.5  90/03/19  17:33:06  orr
 * 	Removed device structure and the devices array, put it
 *	in bios.h.
 * 
 * Revision 2.1.1.4  90/03/14  16:58:58  orr
 * 	add device stuff.
 * 
 * Revision 2.1.1.3  90/03/13  15:34:50  orr
 * 	Version that gives an A:> prompt and does a DIR.
 * 	Removed DEBUG.  Added the support for the DebugX's
 * 	run time debugging.
 * 
 * Revision 2.1.1.2  90/03/12  02:16:29  orr
 * 	try it without debug output.
 * 
 * Revision 2.1.1.1  90/03/12  01:17:39  orr
 * 
 * Revision 1.1  90/03/09  11:57:04  orr
 * Initial revision
 * 
 */

#ifndef	_BASE_H
#define	_BASE_H

#include <mach.h>
#include <mach_error.h>
#include <mach/message.h>
#include <mach/exception.h>

#define Debug_Level_0	0
#define Debug_Level_1	1
#define Debug_Level_2	2

extern int us_debug_level;
extern int video_debug_level;
extern int key_debug_level;
extern int disk_debug_level;

#define	Vdebug0(args)						\
	if (video_debug_level > Debug_Level_0)			\
		fprintf args

#define	Vdebug1(args)						\
	if (video_debug_level > Debug_Level_1)			\
		fprintf args

#define	Vdebug2(args)						\
	if (video_debug_level > Debug_Level_2)			\
		fprintf args
#define	Kdebug0(args)						\
	if (key_debug_level > Debug_Level_0)			\
		fprintf args

#define	Kdebug1(args)						\
	if (key_debug_level > Debug_Level_1)			\
		fprintf args

#define	Kdebug2(args)						\
	if (key_debug_level > Debug_Level_2)			\
		fprintf args

#define	Ddebug0(args)						\
	if (disk_debug_level > Debug_Level_0)			\
		fprintf args

#define	Ddebug1(args)						\
	if (disk_debug_level > Debug_Level_1)			\
		fprintf args

#define	Ddebug2(args)						\
	if (disk_debug_level > Debug_Level_2)			\
		fprintf args

#define	Debug0(args)						\
	if (us_debug_level > Debug_Level_0)			\
		fprintf args

#define	Debug1(args)						\
	if (us_debug_level > Debug_Level_1)			\
		fprintf args

#define	Debug2(args)						\
	if (us_debug_level > Debug_Level_2)			\
		fprintf args


/*
 * Space allocator.
 */
#define	Malloc(size)		malloc(size)
#define	New(typ)		(typ *)malloc(sizeof(typ))
#define NewArray(typ,cnt)	(typ *)malloc(sizeof(typ)*(cnt))
#define NewStr(str)		(char *)strcpy(malloc(strlen(str)+1),str)
#define ZeroNew(cnt,typ)	(typ *)calloc(cnt,sizeof(typ))
#define	Free(ptr)		free(ptr)

/*
 * Array operations
 */
#define	Count(arr)		(sizeof(arr)/sizeof(arr[0]))
#define	Lastof(arr)		(Count(arr)-1)
#define	Endof(arr)		(&(arr)[Count(arr)])

#define	Min(a,b) (((a)<(b))?(a):(b))
#define	Max(a,b) (((a)>(b))?(a):(b))

#define MASK8(x)	((x) & 0xff)
#define MASK16(x)	((x) & 0xffff)
#define HIGH(x)		MASK8((unsigned long)(x) >> 8)
#define LOW(x)		MASK8((unsigned long)(x))
#define WORD(x)		MASK16((unsigned long)(x))
#define SETHIGH(x,y) 	(*(x) = (*(x) & ~0xff00) | ((MASK8(y))<<8))
#define SETLOW(x,y) 	(*(x) = (*(x) & ~0xff) | (MASK8(y)))
#define SETWORD(x,y)	(*(x) = (*(x) & ~0xffff) | (MASK16(y)))

#define MACH_CALL(x,y)	{int foo;if((foo=(x))!=KERN_SUCCESS){\
                         mach_error(y,foo);exit_dos();}}

typedef int onoff_t;

#define OFF		0
#define ON		1
#define MAYBE		2
#define UNCHANGED	2
#define	REDIRECT	3

#if	_DEBUG_
#define	private
#else
#define	private static
#endif  _DEBUG_

extern char *malloc();

#endif	_BASE_H

