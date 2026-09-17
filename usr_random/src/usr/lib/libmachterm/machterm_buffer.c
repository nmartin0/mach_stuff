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

#include "machterm.h"

/* switch to a new display buffer */
/* return non-zero if error */
int machterm_buffer(machterm_context_t *machterm_context,
                    char *display_buffer,
                    int display_rowsize)
{
  register context_t t = (context_t)machterm_context;
  register unsigned int *to_row, *to_int, *from_row, *from_int;
  register int x, y, rowints;

  /* use default context if zero pointer */
  if (!t) t = &_machterm_default_context;

  if (!display_buffer) return -2;

  /* assert that new rows are accessable as (int *) */
  if (display_rowsize % INTBYTES) return -3;

  /* assert that new buffer is (int *) */
  if ((int)display_buffer % INTBYTES) return -4;

  /* assert that new rowsize is sufficient */
  rowints = display_rowsize / INTBYTES;
  if (rowints * INTBITS < t->width) return -5;

  /* if old buffer exists, copy its contents into the new buffer */
  from_row = t->buffer;
  to_row = (unsigned int *)display_buffer;
  if (from_row) for (y = t->height; y--;) {
    to_int = to_row;
    from_int = from_row;
    for (x = t->rowsize; x--;) *to_int++ = *from_int++;
    to_row += rowints;
    from_row += t->rowints;
  }

  /* set new buffer and rowsize */
  t->buffer = (unsigned int *)display_buffer;
  t->rowints = rowints;

  /* all done, no error */
  return 0;

} /* machterm_buffer() */

