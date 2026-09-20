/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 *  5-Oct-93  Alessandro (af) at Carnegie-Mellon University
 *	Modified to support Alpha
 *
 * $Log$
 *
 */
/* bignum.h-arbitrary precision integers
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/***********************************************************************\
*									*
*	Arbitrary-precision integer arithmetic.				*
*	For speed, we work in groups of bits, even though this		*
*	complicates algorithms.						*
*	Each group of bits is called a 'littlenum'.			*
*	A bunch of littlenums representing a (possibly large)		*
*	integer is called a 'bignum'.					*
*	Bignums are >= 0.						*
*									*
\***********************************************************************/

#define	LITTLENUM_NUMBER_OF_BITS	(16)
#define	LITTLENUM_RADIX			(1 << LITTLENUM_NUMBER_OF_BITS)
#define	LITTLENUM_MASK			(0xFFFF)
#define LITTLENUM_SHIFT			(1)
#define CHARS_PER_LITTLENUM		(1 << LITTLENUM_SHIFT)
#ifndef BITS_PER_CHAR
#define BITS_PER_CHAR			(8)
#endif

typedef unsigned short int	LITTLENUM_TYPE;
typedef LITTLENUM_TYPE		*BIGNUM_TYPE;

extern BIGNUM_TYPE bignum_new(/* BIGNUM_TYPE val, int nlittle */);

/* JF truncated this to get around a problem with GCC */
#define	LOG_TO_BASE_2_OF_10		(3.3219280948873623478703194294893901758651 )
				/* WARNING: I haven't checked that the trailing digits are correct! */

/* When doing 2-complement operations on bignums.. */

#define	BIGNUM_CPL2_LITTLENUMS	(MACHINE_BITS_PER_LONG/LITTLENUM_NUMBER_OF_BITS)

/* end: bignum.h */
