/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/* access to MC68851/MC68030 PMMU structures and functions */

/* See "MC68851 Paged Memroy Management Unit User's Manual", first edition
 * Prentice-Hall, 1986
 */

#ifndef _MC68851_H_
#define _MC68851_H_

/* pagesize Field Values */
#if PAGE_SIZE == 4096
#define	PMMU_PAGESIZE	0xC	/* 4k bytes */
#elif PAGE_SIZE == 8192
#define	PMMU_PAGESIZE	0xD	/* 8k bytes */
#endif

/* addrsize Field Value */
#define	PMMU_IS_32b	0x0	/* 32 bit virtual address */

/* Table index maxes */
#define	PMMU_TIA	8
#define	PMMU_TIB	24 - PAGE_SHIFT

typedef struct {
  unsigned long
    enable:1,		/* mapping enable */
    unused:5,
    sre:1,		/* supervisor root enable */
    mbz0:1,
    pagesize:4,		/* page size (bytes) */
    addrsize:4,		/* size of virt addr (bits) */
    tia:4,		/* max tbl indexes/level (bits) */
    tib:4,
    mbz1:8;
} PMMU_TC_reg;

/* Root Pointers */
#define	PMMU_RP_LIMIT	0x8000	/* no limit */
#define PMMU_RP_SG	0x0200  /* shared globally */
#define PMMU_VALID_RP	2	/* valid root pointer */

typedef struct {
    unsigned short	limit;		/* limit reg not used (above) */
    unsigned short	valid;		/* should be PMMU_VALID_RP */
    phys_offset_t	phys;		/* phys address of tree root */
} PMMU_RP_reg;

/* Status register */
#define PMMU_SR_BUSERR	0x8000
#define PMMU_SR_WRPROT	0x0800
#define PMMU_SR_INVALID	0x0400

/* Descriptor Types */
#define	PMMU_INVALID	    0	    /* invalid entry */
#define	PMMU_VALID_PP	    1	    /* valid page pointer */
#define	PMMU_VALID_PTP	    2	    /* valid page table pointer */

/* Page Protection Codes */
#define	PMMU_RW		    0	    /* page is read/write */
#define	PMMU_WP		    1	    /* page is read-only */
 
/* Set the Translation Control register. */
static inline void pmmu_set_tc(PMMU_TC_reg *p)
{
  asm("pmove	%0@,TC" : : "a" (p));
}

/* Set the Supervisor Root Pointer register. */
static inline void pmmu_set_srp(PMMU_RP_reg *p)
{
  asm("pmove	%0@,SRP" : : "a" (p));
}

/* Set the CPU Root Pointer register. */
static inline void pmmu_set_crp(PMMU_RP_reg *p)
{
  asm("pmove	%0@,CRP" : : "a" (p));
}

/*
 * Flush the ATC of entries corresponding
 * to a certain root pointer (68020).
 */
static inline void pmmu_flush_map(PMMU_RP_reg *p)
{
  asm("pflushr	%0@" : : "a" (p));
}

/* Flush all user descriptors from the ATC. */
static inline void pmmu_flush_user_all(void)
{
  asm("pflush	#0,#4");
}

/*
 * Flush the user descriptor mapping
 * the specified address from the ATC.
 */
static inline void pmmu_flush_user(vm_offset_t p)
{
  asm("pflush	#0,#4,%0@" : : "a" (p));
}

/* Flush all supervisor descriptors from the ATC. */
static inline void pmmu_flush_supr_all(void)
{
  asm("pflush #4,#4");
}

/*
 * Flush the supervisor descriptor mapping
 * the specified address from the ATC.
 */
static inline void pmmu_flush_supr(vm_offset_t p)
{
  asm("pflush	#4,#4,%0@" : : "a" (p));
}

/*
 * Flush all supervisor descriptors from the ATC
 * including shared entries (68020).
 */
static inline void pmmu_flush_supr_shared_all(void)
{
  asm("pflushs	#4,#4");
}

/*
 * Flush the supervisor descriptor mapping
 * the specified address from the ATC
 * including shared entries (68020).
 */
static inline void pmmu_flush_supr_shared(vm_offset_t p)
{
  asm("pflushs	#4,#4,%0@" : : "a" (p));
}

/*
 * Load the user descriptor mapping
 * the specified data address into the ATC.
 * Used for page faults on RMW cycles.
 */
static inline void pmmu_load_user(vm_offset_t p)
{
  asm("ploadw	#1,%0@" : : "a" (p));
}

/*
 * Return the PSR register obtained
 * by testing the specified user data address.
 */
static inline unsigned short pmmu_test_user_data(vm_offset_t p)
{
  volatile unsigned short psr;

  asm("ptestr	#1,%0@,#7" : : "a" (p));
  asm("pmove	PSR,%0" : "=m" (psr) : );
  return psr;
}

/*
 * Return the PSR register obtained
 * by testing the specified user text address.
 */
static inline unsigned short pmmu_test_user_text(vm_offset_t p)
{
  volatile unsigned short psr;

  asm("ptestr	#2,%0@,#7" : : "a" (p));
  asm("pmove	PSR,%0" : "=m" (psr) : );
  return psr;
}

#endif /* _MC68851_H_ */

