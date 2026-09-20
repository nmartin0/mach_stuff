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
/* bignum_copy.c - copy a bignum
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

#include "bignum.h"

#ifdef USG
#define bzero(s,n) memset(s,0,n)
#define bcopy(from,to,n) memcpy(to,from,n)
#endif

/*
 *			bignum_copy ()
 *
 * Copy a bignum from in to out.
 * If the output is shorter than the input, copy lower-order littlenums.
 * Return 0 or the number of significant littlenums dropped.
 * Assumes littlenum arrays are densely packed: no unused chars between
 * the littlenums. Uses bcopy() to move littlenums, and wants to
 * know length (in chars) of the input bignum.
 */

/* void */
int
bignum_copy (in, in_length, out, out_length)
     register LITTLENUM_TYPE *	in;
     register int		in_length; /* in sizeof(littlenum)s */
     register LITTLENUM_TYPE *	out;
     register int		out_length; /* in sizeof(littlenum)s */
{
  register int	significant_littlenums_dropped;

  if (out_length < in_length)
    {
      register LITTLENUM_TYPE *	p; /* -> most significant (non-zero) input littlenum. */

      bcopy ((char *)in, (char *)out, out_length << LITTLENUM_SHIFT);
      for (p = in + in_length - 1;   p >= in;   -- p)
	{
	  if (* p) break;
	}
      significant_littlenums_dropped = p - in - in_length + 1;
      if (significant_littlenums_dropped < 0)
	{
	  significant_littlenums_dropped = 0;
	}
    }
  else
    {
      bcopy ((char *)in, (char *)out, in_length << LITTLENUM_SHIFT);
      if (out_length > in_length)
	{
	  bzero ((char *)(out + out_length), (out_length - in_length) << LITTLENUM_SHIFT);
	}
      significant_littlenums_dropped = 0;
    }
  return (significant_littlenums_dropped);
}

/* Allocate a copy of the given VALUE bignum, SIZE littlenums long */
BIGNUM_TYPE
bignum_new(value, size)
  BIGNUM_TYPE value;
  int size;
{
  extern void *xmalloc();
  BIGNUM_TYPE ret;

  if (size < BIGNUM_CPL2_LITTLENUMS)
    ret = (BIGNUM_TYPE) xmalloc(BIGNUM_CPL2_LITTLENUMS * CHARS_PER_LITTLENUM);
  else
    ret = (BIGNUM_TYPE) xmalloc(size * CHARS_PER_LITTLENUM);
  bcopy (value, ret, size * CHARS_PER_LITTLENUM);
  return ret;
}

/* This should probably be in another file */

bignum_negate(bignum, ndigits)
  BIGNUM_TYPE bignum;
{
  unsigned long carry;

  /* floats ? */
  if (ndigits < 0)
    as_warn("Do not know to negate floats! (operator lossage)");
  else {
    carry = 1;
    if (ndigits < BIGNUM_CPL2_LITTLENUMS)
      ndigits = BIGNUM_CPL2_LITTLENUMS;
    bignum_one_complement(bignum, ndigits);
    while (ndigits-- > 0) {
      carry = *bignum + carry;
      *bignum++ = carry & LITTLENUM_MASK; 
      carry = carry >> LITTLENUM_NUMBER_OF_BITS;
    }
  }
}

bignum_one_complement(bignum, ndigits)
  BIGNUM_TYPE bignum;
{

  /* floats ? */
  if (ndigits < 0)
    as_warn("Do not know how to ~ floats! (operator lossage)");
  else {
    if (ndigits < BIGNUM_CPL2_LITTLENUMS)
      ndigits = BIGNUM_CPL2_LITTLENUMS;
    while (ndigits--) {
      *bignum = ~*bignum;
      bignum++;
    }
  }
}

/* end: bignum_copy.c */
