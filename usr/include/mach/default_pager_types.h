/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: default_pager_types.h,v $
 * Revision 1.2.12.1  1997/03/27  18:45:35  barbou
 * 	submit adidtional picco changes
 * 	[1996/09/12  22:09:41  robert]
 * 	AXP pool merge.
 * 	[97/02/25            barbou]
 *
 * Revision 1.2.5.3  1995/01/11  19:30:43  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	[1994/11/10  15:34:40  bolinger]
 * 
 * 	Insert OSC1_3 log.
 * 
 * 	BEGIN OSC1_3 HISTORY
 * 
 * 	Revision 1.2.2.6  1994/04/01  18:43:00  jph
 * 	  CR10550 -- Add backing store info interfaces.
 * 	  [1994/04/01  18:40:34  jph]
 * 
 * 	Revision 1.2.2.5  1994/02/07  22:42:06  jph
 * 	  Merged with changes from 1.2.2.4
 * 	  [1994/02/07  22:40:43  jph]
 * 
 * 	  CR10433 -- Added limits for backing store allocation.
 * 	  [1994/02/07  22:30:44  jph]
 * 
 * 	END OSC1_3 HISTORY
 * 	[1994/11/02  20:48:03  bolinger]
 * 
 * Revision 1.2.5.1  1994/09/23  06:57:11  ezf
 * 	change marker to not FREE
 * 	[1994/09/23  06:54:40  ezf]
 * 
 * Revision 1.2.2.3  1993/08/05  17:57:52  gm
 * 	CR9627: Removed deprecated default_pager_filename_t typedef.
 * 	[1993/07/09  19:20:34  gm]
 * 
 * Revision 1.2.2.2  1993/06/09  02:11:18  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  20:42:11  gm]
 * 
 * Revision 1.2  1993/04/19  16:32:53  devrcs
 * 	Fixes for ANSI C
 * 	[1993/02/26  13:30:02  sp]
 * 
 * 	Moved from bootstrap.
 * 	[1993/02/17  13:45:56  bruel]
 * 
 * 	*** empty log message ***
 * 	[1993/02/10  11:00:13  bruel]
 * 
 * 	Created for external default pager.
 * 	[1993/02/09  14:57:19  bruel]
 * 
 * $EndLog$
 */


#ifndef	_MACH_DEFAULT_PAGER_TYPES_H_
#define _MACH_DEFAULT_PAGER_TYPES_H_

/*
 *	Remember to update the mig type definitions
 *	in default_pager_types.defs when adding/removing fields.
 */

typedef struct default_pager_info {
	vm_size_t dpi_total_space;	/* size of backing store */
	vm_size_t dpi_free_space;	/* how much of it is unused */
	vm_size_t dpi_page_size;	/* the pager's vm page size */
} default_pager_info_t;

typedef integer_t *backing_store_info_t;
typedef int	backing_store_flavor_t;

#define BACKING_STORE_BASIC_INFO	1
#define BACKING_STORE_BASIC_INFO_COUNT \
		(sizeof(struct backing_store_basic_info)/sizeof(integer_t))
struct backing_store_basic_info {
	natural_t	pageout_calls;		/* # pageout calls */
	natural_t	pagein_calls;		/* # pagein calls */
	natural_t	pages_in;		/* # pages paged in (total) */
	natural_t	pages_out;		/* # pages paged out (total) */
	natural_t	pages_unavail;		/* # zero-fill pages */
	natural_t	pages_init;		/* # page init requests */
	natural_t	pages_init_writes;	/* # page init writes */

	natural_t	bs_pages_total;		/* # pages (total) */
	natural_t	bs_pages_free;		/* # unallocated pages */
	natural_t	bs_pages_in;		/* # page read requests */
	natural_t	bs_pages_in_fail;	/* # page read errors */
	natural_t	bs_pages_out;		/* # page write requests */
	natural_t	bs_pages_out_fail;	/* # page write errors */

	integer_t	bs_priority;
	integer_t	bs_clsize;
};
typedef struct backing_store_basic_info	*backing_store_basic_info_t;


typedef struct default_pager_object {
	vm_offset_t dpo_object;		/* object managed by the pager */
	vm_size_t dpo_size;		/* backing store used for the object */
} default_pager_object_t;

typedef default_pager_object_t *default_pager_object_array_t;


typedef struct default_pager_page {
	vm_offset_t dpp_offset;		/* offset of the page in its object */
} default_pager_page_t;

typedef default_pager_page_t *default_pager_page_array_t;

#define DEFAULT_PAGER_BACKING_STORE_MAXPRI	4

#endif	/* _MACH_DEFAULT_PAGER_TYPES_H_ */
