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
 * HISTORY
 * 31-May-92  Alessandro Forin (af) at Carnegie-Mellon University
 *	Adapted for Alpha from GNU BFD
 *
 * $Log:	coff.h,v $
 * 
 */
/*
 *	File: coff.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Structure definitions for COFF headers
 */

/* Internal format of COFF object file data structures, for GNU BFD.
   This file is part of BFD, the Binary File Descriptor library.  */


struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	int		f_timdat;	/* date & time stamp */
	vm_offset_t	f_symptr;	/* file pointer to symtab */
	int		f_nsyms;	/* size of symtable */
	unsigned short	f_opthdr;	/* size of auxilary hdr */
	unsigned short	f_flags;	/* flags */
};

#define  F_EXEC		0000002

#define ALPHAMAGIC	0603

struct scnhdr {
	char		s_name[8];	/* section name */
	vm_offset_t	s_paddr;	/* phys address */
	vm_offset_t	s_vaddr;	/* virtual address */
	vm_offset_t	s_size;		/* size of section */
	vm_offset_t	s_scnptr;	/* fptr to raw data */
	vm_offset_t	s_relptr;	/* fptr to relocation */
	vm_offset_t	s_lnnoptr;	/* fptr to line numbers */
	unsigned short	s_nreloc;	/* no. of relocation entries */
	unsigned short	s_nlnno;	/* no, of line number entries */
	int		s_flags;	/* flags */
};



struct aouthdr {
	short		magic;
	short		vstamp;		/* version stamp */
	int		pad;		/* help for cross compilers */
	vm_offset_t	tsize;		/* text size */
	vm_offset_t	dsize;		/* initialized data size */
	vm_offset_t	bsize;		/* uninitialized data size */
	vm_offset_t	entry;		/* value of "start"*/
	vm_offset_t	text_start;
	vm_offset_t	data_start;
	vm_offset_t	bss_start;
	int		gprmask;	/* general purpose register mask*/
	int		fprmask;	/* FPA register mask */
	vm_offset_t	gp_value;	/* the gp value */
};


#define OMAGIC	0407		/* old impure format */
#define NMAGIC	0410		/* read-only text */
#define ZMAGIC	0413		/* demand load format */

#define	N_BADMAG(a) \
  ((a).magic != OMAGIC && (a).magic != NMAGIC && (a).magic != ZMAGIC)

#define SCNROUND ((long)16)

struct exechdr {
	struct filehdr	f;
	struct aouthdr	a;
};

#define N_TXTOFF(f, a) \
 (((a).magic == ZMAGIC) ? 0 : \
   (sizeof(struct filehdr) + sizeof(struct aouthdr) + \
    (f).f_nscns * sizeof(struct scnhdr)))
