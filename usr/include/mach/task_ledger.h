/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: task_ledger.h,v $
 * Revision 1.1.8.2  1995/01/06  19:51:54  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	[1994/10/14  03:43:13  dwm]
 *
 * Revision 1.1.8.1  1994/09/23  02:42:55  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:43:00  ezf]
 * 
 * Revision 1.1.4.3  1993/09/17  21:35:29  robert
 * 	change marker to OSF_FREE_COPYRIGHT
 * 	[1993/09/17  21:28:49  robert]
 * 
 * Revision 1.1.4.2  1993/06/04  15:13:57  jeffc
 * 	CR9193 - MK5.0 merge.
 * 	[1993/05/18  02:38:04  gm]
 * 
 * Revision 3.0.2.2  1993/05/15  15:42:19  jph
 * 	Merge MK5.0: change LEDGER_REAL_ITEMS to be LEDGER_N_ITEMS.
 * 	[1993/05/15  15:21:21  jph]
 * 
 * Revision 3.0  1992/12/31  22:13:53  ede
 * 	Initial revision for OSF/1 R1.3
 * 
 * Revision 1.2  1991/08/15  19:16:53  devrcs
 * 	Ledgers: indices for task_ledger exported routines.
 * 	[91/07/18  11:04:31  dwm]
 * 
 * $EndLog$
 */

/*
 * Definitions for task ledger line items
 */
#define ITEM_THREADS		0	/* number of threads	*/
#define ITEM_TASKS		1	/* number of tasks	*/

#define ITEM_VM	   		2	/* virtual space (bytes)*/

#define LEDGER_N_ITEMS		3	/* Total line items	*/

#define LEDGER_UNLIMITED	0	/* ignored item.maximum	*/
