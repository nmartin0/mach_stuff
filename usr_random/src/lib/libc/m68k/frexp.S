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

#include <mac2/SYS.h>

/*
 * double frexp(val, eptr)
 * returns: x s.t. val = x * (2 ** n), with n stored in *eptr
 */
ENTRY(frexp)
	fmoved		sp@(4),fp1
	fgetmanx	fp1,fp0
	fgetexpx	fp1
	fmovel		fp1,d0
	addql		#1,d0
	movel		sp@(12),a0
	movel		d0,a0@
	fdivl		#2,fp0
	fmoved		fp0,sp@-
	movel		sp@+,d0
	movel		sp@+,d1
	rts
