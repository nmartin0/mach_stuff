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

#include <solicit.h>

/* ask question, return non-zero if response is "yes" */
int solicit_yesno(char *question)
{
  char buffer[128];

  while (1) {
    printf("%s ", question);
    *buffer = 0;
    if (gets(buffer)) {
      if (!strcmp(buffer, "Y")) return 1;
      if (!strcmp(buffer, "y")) return 1;
      if (!strcmp(buffer, "YES")) return 1;
      if (!strcmp(buffer, "yes")) return 1;
      if (!strcmp(buffer, "N")) return 0;
      if (!strcmp(buffer, "n")) return 0;
      if (!strcmp(buffer, "NO")) return 0;
      if (!strcmp(buffer, "no")) return 0;
    }
    printf("Answer yes or no.\n");
  }
}

