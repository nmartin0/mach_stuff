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

/* interactive solicitation library, Zonnie L. Williamson, CMU MacMach 1992 */

#ifndef _SOLICIT_H_
#define _SOLICIT_H_

/* count the items in a list */
extern int count_items(char *list);

/* add an item to a list */
extern void add_item(char *list, char *item);

/* select the nth item in a list, return non-zero if error */
extern int select_item(char *list, char *item, int n);

/* search for a target in a list, return non-zero if not found */
extern int search_items(char *list, char *target);

/* sort the items in a list */
extern void sort_items(char *list, int unique);

/* solicit text */
extern char *solicit_text(char *prompt, char *default_text, char *help);

/* solicit password */
extern char *solicit_password(char *prompt, char *help);

/* select one or more items from a list */
extern char *solicit_one_or_more_items(char *list,
                                       char *prompt,
                                       char *help,
                                       int more);

/* select a single item from a list */
#define solicit_item(list, prompt, help) \
  solicit_one_or_more_items((list), (prompt), (help), 0)

/* select one or more items from a list */
#define solicit_items(list, prompt, help) \
  solicit_one_or_more_items((list), (prompt), (help), 1)

/* ask question, return non-zero if response is "yes" */
extern int solicit_yesno(char *question);

#endif /* _SOLICIT_H_ */
