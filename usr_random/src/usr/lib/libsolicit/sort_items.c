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

#include <strings.h>
#include <solicit.h>

static int compare(char **a, char **b)
{
  return strcmp(*a, *b);
}

/* sort the items in a list */
void sort_items(char *list, int unique)
{
  char *items, **pointer, *i;
  int nitems, n;

  items = (char *)alloca(strlen(list) + 1);
  strcpy(items, list);
  nitems = count_items(items);
  pointer = (char **)alloca(nitems * sizeof(char *));
  for (n = 0, i = items; i; n++) {
    pointer[n] = i;
    if (i = index(i, ':')) *i++ = 0;
  }
  qsort(pointer, nitems, sizeof(char *), compare);
  *list = 0;
  for (n = 0; n < nitems; n++) {
    if (unique) {
      if ((n > 0) && !strcmp(pointer[n], pointer[n - 1])) continue;
    }
    add_item(list, pointer[n]);
  }
}
