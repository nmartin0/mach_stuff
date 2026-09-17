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

/* solicit text */
char *solicit_text(char *prompt, char *default_text, char *help)
{
  char buffer[1024], *tmp;

  while (1) {
    if (!default_text) printf("%s ", prompt);
    else printf("%s [%s] ", prompt, default_text);
    if (!gets(buffer)) return 0;
    if (!*buffer && default_text) strcpy(buffer, default_text);
    if (!*buffer || !strcmp(buffer, "?")) printf("%s\n", help);
    else if (*buffer == 27) return 0;
    else break;
  }
  tmp = (char *)malloc(strlen(buffer) + 1);
  strcpy(tmp, buffer);
  return tmp;
}

