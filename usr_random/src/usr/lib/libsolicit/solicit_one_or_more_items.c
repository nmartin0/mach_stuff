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

/* solicit one or more items */
/* malloc's result */
char *solicit_one_or_more_items(char *list, char *prompt, char *help, int more)
{
  char *items, item[1024], buffer[1024], output[1024], *b1, *b2, *tmp;
  int nitems, i, n;

  items = (char *)alloca(strlen(list) + 1);
  strcpy(items, list);

  nitems = count_items(items);

  if (!nitems) return 0;
  if (nitems == 1) {
    strcpy(output, items);
    goto done;
  }

  sort_items(items, 1);

  if (nitems == 1) strcpy(output, items);
  else while (1) {

    printf("%s\n", prompt);

    for (i = 0; i < nitems; i++) {
      select_item(items, buffer, i);
      printf("%2d: %s\n", i + 1, buffer);
    }

    if (!more) printf("Enter item number: ");
    else printf("Enter item number(s): ");

    if (!gets(buffer)) return 0;
    if (!*buffer || !strcmp(buffer, "?")) printf("%s\n", help);
    else if (*buffer == 27) return 0;

    for (n = 0, *output = 0, b2 = buffer; b2; n++) {
      for (b1 = b2; *b1 && ((*b1 == ' ') || (*b1 == ',')); b1++);
      if (!*b1) break;
      for (b2 = b1; *b2 && !((*b2 == ' ') || (*b2 == ',')); b2++);
      if (*b2) *b2++ = 0;
      if (!select_item(items, item, atoi(b1) - 1)) add_item(output, item);
      else {
        n = 0;
        break;
      }
    }

    if (n == 0) printf("Please enter %s between 1 and %d.\n",
                       more ? "numbers" : "a number",
                       nitems);
    else if (!more && (n > 1))
      printf("Please enter a single selection.\n");
    else break;

  }

  sort_items(output, 1);

done:

  tmp = (char *)malloc(strlen(output) + 1);
  strcpy(tmp, output);

  return tmp;

} /* select_one_or_more_items() */

