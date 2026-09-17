/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	pmap.h,v $
 * Revision 2.2  91/09/12  16:42:41  bohman
 * 	Created Mach 3.0 version from 2.5 version.
 * 	[91/09/11  14:57:46  bohman]
 * 
 * Revision 2.2  90/09/04  17:29:11  bohman
 * 	Created.
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2/pmap.h
 *	Author: David E. Bohman II (CMU macmach)
 */

/*
 * Machine-dependent structures
 * for the physical map module.
 */

#ifndef	_MAC2_PMAP_H_
#define	_MAC2_PMAP_H_

#include <mach/mac2/vm_param.h>
#include <mach/vm_statistics.h>

#include <kern/lock.h>

typedef	unsigned long	phys_offset_t;

/* Access various fields in virtual offsets. */
union vm_offset_fields {

  vm_offset_t full_offset;

  struct {
    vm_offset_t
         index:8,
        offset:24;
  } ptptbl;
#define ptptbl_index	ptptbl.index
#define ptptbl_offset	ptptbl.offset

  struct {
    vm_offset_t
              :8,
         index:24 - PAGE_SHIFT,
              :PAGE_SHIFT;
  } pptbl;
#define pptbl_index	pptbl.index

  struct {
    vm_offset_t
         index:32 - PAGE_SHIFT,
        offset:PAGE_SHIFT;
  } pg;
#define pg_index	pg.index
#define pg_offset	pg.offset

};

/*
 * Descriptors
 */
struct page_tbl_ptr {
    unsigned long
	    pfn:19,		    /* page frame number of page table */
	    mbz:11,
	    valid:2;
};

typedef	struct page_tbl_ptr ptp_t;
#define	PTP_NULL    ((ptp_t *) 0)

struct page_ptr {
    unsigned long
	    pfn:19,		    /* page frame number of page */
	    unused0:4,
	    wired:1,		    /* page is wired */
	    mbz0:1,
	    ci:1,		    /* cache inhibit */
	    mbz1:1,
	    mod:1,		    /* page modified */
	    ref:1,		    /* page referenced */
	    prot:1,		    /* page protection */
	    valid:2;
};

typedef	struct page_ptr	pp_t;
#define	PP_NULL ((pp_t *)0)

typedef	unsigned long cpu_set;	/* set of CPUs - must be <= 32 */

struct pmap {
    phys_offset_t	root;	/* phys addr of translation tree root */
    struct pt_root_page *	/* pt root page descriptor */
			ptrt;
#ifdef MODE24
    phys_offset_t		/* phys addr of alternate root */
			alt_root;
    struct pt_root_page *	/* pt root page descriptor */
			alt_ptrt;
#endif /* MODE24 */
    int				/* pmap reference count */
			ref_count;
#ifdef MODE24
    unsigned long	flags;
#define PMAP_F_24BIT 0x00000001	/* task is using folded address space */
#endif /* MODE24 */
    simple_lock_data_t	lock;	/* lock on pmap */
    struct pmap_statistics	/* map statistics */
			stats;
    cpu_set			/* bitmap of cpus using pmap */
			cpus_using;
};

typedef struct pmap	*pmap_t;
#define	PMAP_NULL	((pmap_t) 0)

inline pp_t *pmap_pp(pmap_t pmap, vm_offset_t va);

/*
 *	Macros for speed.
 */
#ifdef MODE24
#define PMAP_ACTIVATE(pmap_, thread, cpu)	\
{									\
    if ((pmap_) != kernel_pmap) {					\
	pmap_t		pmap;						\
	PMMU_RP_reg	rp;						\
\
	pmap = (pmap_);							\
									\
	if (pmap->flags&PMAP_F_24BIT)					\
	    rp.phys = pmap->alt_root;					\
	else								\
	    rp.phys = pmap->root;					\
\
	rp.limit = PMMU_RP_LIMIT;					\
	rp.valid = PMMU_VALID_RP;					\
	pmmu_set_crp(&rp);						\
\
	flush_cpu_caches();						\
\
	pmap->cpus_using = TRUE;					\
    }									\
}
#else /* MODE24 */
#define PMAP_ACTIVATE(pmap_, thread, cpu)	\
{									\
    if ((pmap_) != kernel_pmap) {					\
	pmap_t		pmap;						\
	PMMU_RP_reg	rp;						\
\
	pmap = (pmap_);							\
									\
	rp.phys = pmap->root;					\
\
	rp.limit = PMMU_RP_LIMIT;					\
	rp.valid = PMMU_VALID_RP;					\
	pmmu_set_crp(&rp);						\
\
	flush_cpu_caches();						\
\
	pmap->cpus_using = TRUE;					\
    }									\
}
#endif /* MODE24 */

#define PMAP_DEACTIVATE(pmap_, thread, cpu)	\
{									\
    if ((pmap_) != kernel_pmap) {					\
	pmap_t		pmap;						\
\
	pmap = (pmap_);							\
	pmap->cpus_using = FALSE;					\
    }									\
}

#define PMAP_CONTEXT(pmap_, thread)

#define	pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define	pmap_phys_address(frame)	((vm_offset_t) (mac2_ptob(frame)))
#define pmap_phys_to_frame(phys)	((int) (mac2_btop(phys)))

#include <mac2/mc68851.h>

#endif	_MAC2_PMAP_H_
