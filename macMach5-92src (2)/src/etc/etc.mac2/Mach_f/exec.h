/* 
 * Mach Operating System
 * Copyright (c) 1990, 1991 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * ×Mach for MPW 3.1
 *
 * File: exec.h
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef _EXEC_H_
#define _EXEC_H_

/*
 * format of the exec header
 * known by kernel and by user programs
 */
struct exec {
	unsigned short	a_machtype;	/* machine type */
	unsigned short	a_magic;	/* magic number */
	unsigned long	a_text;		/* size of text segment */
	unsigned long	a_data;		/* size of initialized data */
	unsigned long	a_bss;		/* size of uninitialized data */
	unsigned long	a_syms;		/* size of symbol table */
	unsigned long	a_entry;	/* entry point */
	unsigned long	a_trsize;	/* size of text relocation */
	unsigned long	a_drsize;	/* size of data relocation */
};

#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0410		/* read-only text */
#define	ZMAGIC	0413		/* demand load format */

/* machine types */

#define M_68020		2	/* runs only on 68020 */

#define	N_BADMAG(x)		((x).a_magic!=OMAGIC)

#define N_TXTOFF(x)		(sizeof (struct exec))

/*
 * Format of a relocation datum.
 */
struct relocation_info {
	long			r_address;		/* address which is relocated */
	unsigned long	r_symbolnum:24,	/* local symbol ordinal */
					r_pcrel:1, 	/* was relocated pc relative already */
					r_length:2,	/* 0=byte, 1=word, 2=long */
					r_extern:1,	/* does not include value of sym referenced */
					:4;		/* nothing, yet */
};

#endif /* _EXEC_H_ */