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
/*
 * HISTORY
 * $Log:	struc-symbol.h,v $
 * Revision 1.2  90/11/08  15:05:39  bww
 * 	Let's always use the name a_out.h to forstall any possible
 * 	recursive include with <a.out.h> when using USE_SYSTEM_HDR.
 * 	[90/11/08  15:03:04  bww]
 * 
 */
/* struct_symbol.h - Internal symbol structure
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

#ifndef		VMS
#include "a_out.h"		/* Needed to define struct nlist. Sigh. */
#else
#include "a_out.h"
#endif

struct symbol			/* our version of an nlist node */
{
  struct nlist	sy_nlist;	/* what we write in .o file (if permitted) */
  long unsigned sy_name_offset;	/* 4-origin position of sy_name in symbols */
				/* part of object file. */
				/* 0 for (nameless) .stabd symbols. */
				/* Not used until write_object_file() time. */
  long int	sy_number;	/* 24 bit symbol number. */
				/* Symbol numbers start at 0 and are */
				/* unsigned. */
  struct symbol * sy_next;	/* forward chain, or NULL */
  struct frag *	sy_frag;	/* NULL or -> frag this symbol attaches to. */
  struct symbol *sy_forward;	/* value is really that of this other symbol */
};

typedef struct symbol symbolS;

#define sy_name		sy_nlist .n_un. n_name
				/* Name field always points to a string. */
				/* 0 means .stabd-like anonymous symbol. */
#define sy_type 	sy_nlist.	n_type
#define sy_other	sy_nlist.	n_other
#define sy_desc		sy_nlist.	n_desc
#define sy_value	sy_nlist.	n_value
				/* Value of symbol is this value + object */
				/* file address of sy_frag. */

typedef unsigned long valueT;	/* The type of n_value. Helps casting. */

/* end: struct_symbol.h */
#ifndef WORKING_DOT_WORD
struct broken_word {
	struct broken_word *next_broken_word;/* One of these strucs per .word x-y */
	fragS	*frag;		/* Which frag its in */
	char	*word_goes_here;/* Where in the frag it is */
	fragS	*dispfrag;	/* where to add the break */
	symbolS	*add;		/* symbol_x */
	symbolS	*sub;		/* - symbol_y */
	long	addnum;		/* + addnum */
	int	added;		/* nasty thing happend yet? */
				/* 1: added and has a long-jump */
				/* 2: added but uses someone elses long-jump */
	struct broken_word *use_jump; /* points to broken_word with a similar
					 long-jump */
};
extern struct broken_word *broken_words;
#endif
