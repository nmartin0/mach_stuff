/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: pgtable.h,v $
 * Revision 1.1.2.3  1996/11/18  18:15:54  barbou
 * 	Changed "swapper_pg_dir" to point to the Mach task structure instead of
 * 	the Linux task structure: it's used as the initializer for the
 * 	"mm_mach_task" (formerly "mm_task") field of the "mm_struct" structure.
 * 	[1996/11/18  17:11:23  barbou]
 *
 * Revision 1.1.2.2  1996/09/27  22:59:24  barbou
 * 	Merged up to linux-2.0.21.
 * 	[1996/09/27  22:12:22  barbou]
 * 
 * Revision 1.1.2.1  1996/09/09  16:58:13  barbou
 * 	Added definitions for flush_cache_*() and flush_tlb_*().
 * 	[96/08/21            barbou]
 * 
 * 	Added definition for "swapper_pg_dir".
 * 	[96/08/21            barbou]
 * 
 * 	Added "pgprot" related definitions.
 * 	[96/08/21            barbou]
 * 
 * 	Created.
 * 	[1996/08/21  16:34:47  barbou]
 * 
 * $EndLog$
 */

#ifndef _OSFMACH3_PGTABLE_H
#define _OSFMACH3_PGTABLE_H

/* We don't have any memory cache */
#define flush_cache_all()			do { } while (0)
#define flush_cache_mm(mm)			do { } while (0)
#define flush_cache_range(mm, start, end)	do { } while (0)
#define flush_cache_page(vma, vmaddr)		do { } while (0)
#define flush_page_to_ram(page)			do { } while (0)

#define flush_tlb_all()				do { } while (0)
#define flush_tlb_mm(mm)			do { } while (0)
#define flush_tlb_page(vma, addr)		do { } while (0)
#define flush_tlb_range(mm, start, end)		do { } while (0)
#define local_flush_tlb()			do { } while (0)

#define _PAGE_PRESENT	0x001
#define _PAGE_READ	0x002
#define _PAGE_WRITE	0x004
#define _PAGE_EXEC	0x008
#define _PAGE_COW	0x010

#define _PAGE_TABLE	(_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE)
#define _PAGE_CHG_MASK	(PAGE_MASK)

#define PAGE_NONE	__pgprot(_PAGE_PRESENT)
#define PAGE_SHARED	__pgprot(_PAGE_PRESENT|_PAGE_READ|_PAGE_WRITE)
#define PAGE_COPY	__pgprot(_PAGE_PRESENT|_PAGE_COW)
#define PAGE_READONLY	__pgprot(_PAGE_PRESENT)

/* Do we need to distinguish EXEC permission ? */
#define __P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_COPY
#define __P011	PAGE_COPY
#define __P100	PAGE_READONLY
#define __P101	PAGE_READONLY
#define __P110	PAGE_COPY
#define __P111	PAGE_COPY

#define __S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_SHARED
#define __S011	PAGE_SHARED
#define __S100	PAGE_READONLY
#define __S101	PAGE_READONLY
#define __S110	PAGE_SHARED
#define __S111	PAGE_SHARED

/*
 * We've replaced the "pgd" field with a pointer to the task in "mm_struct".
 */
#define swapper_pg_dir	(&init_osfmach3_task)

#endif /* _OSFMACH3_PGTABLE_H */

