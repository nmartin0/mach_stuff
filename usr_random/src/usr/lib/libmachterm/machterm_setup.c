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

/* setup the font */
/* use internal font if none specified */
/* pick font size if none specified */
static int font(register context_t t,
                char *font_bits,
                char *bold_bits,
                int font_height,
                int font_width)
{
  int width;

  /* if height or width not specified, select internal font */
  if (!font_height || !font_width) {
    font_bits = 0;
#ifndef SMALL_MACHTERM
    bold_bits = 0;
    width = t->width - VBORDER * 2;
    if ((width / 9) >= 80) {
      font_height = 15;
      font_width = 9;
    }
    else if ((width / 7) >= 80) {
      font_height = 13;
      font_width = 7;
    }
    else {
#endif
      font_height = 13;
      font_width = 6;
#ifndef SMALL_MACHTERM
    }
#endif
  }

  /* if font bits not specified, use internal fonts */
  if (!font_bits) {
    if (font_height == 13) {
      if (font_width == 6) {
        font_bits = (char *)_machterm_6x13_font;
#ifndef SMALL_MACHTERM
        bold_bits = (char *)_machterm_6x13_bold;
#endif
      }
#ifndef SMALL_MACHTERM
      else if (font_width == 7) {
        font_bits = (char *)_machterm_7x13_font;
        bold_bits = (char *)_machterm_7x13_bold;
      }
#endif
    }
#ifndef SMALL_MACHTERM
    else if (font_height == 15) {
      if (font_width == 9) {
        font_bits = (char *)_machterm_9x15_font;
        bold_bits = (char *)_machterm_9x15_bold;
      }
    }
#endif
  }

  /* assert that a font has been setup */
  if (!(t->font = font_bits)) return -6;
  if ((t->font_height = font_height) <= 0) return -7;
  if ((t->font_width = font_width) <= 0) return -8;
#ifdef SMALL_MACHTERM
  if (bold_bits) return -8;
  if ((t->font_width = font_width) > CHARBITS) return -8;
#endif

  /* if bold font not specified, use normal font as bold font */
  t->bold = bold_bits ? bold_bits : font_bits;

  /* all done, no error */
  return 0;

} /* font() */

/* prepare a terminal context */
/* return non-zero if error */
#ifdef SMALL_MACHTERM
int small_machterm_setup(machterm_context_t *machterm_context,
                         char *display_buffer,
                         int display_height,
                         int display_width,
                         int display_rowsize,
                         char *font_bits,
                         char *bold_bits,
                         int font_height,
                         int font_width,
                         char initial_cursor,
                         int white_on_black,
                         void (*ungetc)(char, char *),
                         char *ungetc_argument,
                         void (*bell)(void))
#else
int machterm_setup(machterm_context_t *machterm_context,
                   char *display_buffer,
                   int display_height,
                   int display_width,
                   int display_rowsize,
                   char *font_bits,
                   char *bold_bits,
                   int font_height,
                   int font_width,
                   char initial_cursor,
                   int white_on_black,
                   void (*ungetc)(char, char *),
                   char *ungetc_argument,
                   void (*bell)(void))
#endif
{
  register context_t t = (context_t)machterm_context;
  int result;

  /* use default context if zero pointer */
  if (!t) t = &_machterm_default_context;

  /* set display height and width */
  if ((t->height = display_height) <= 0) return -9;
  if ((t->width = display_width) <= 0) return -10;

  /* set the font */
  result = font(t, font_bits, bold_bits, font_height, font_width);
  if (result) return result;

  /* set display buffer */
  result = machterm_buffer(machterm_context, display_buffer, display_rowsize);
  if (result) return result;

  /* set input write function and its argument */
  t->ungetc = ungetc;
  t->ungetc_argument = ungetc_argument;

  /* set bell function */
  t->bell = bell;
  
  /* setup the rest of the terminal context */
  _machterm_setup(t, TOGRAPHIC(initial_cursor), white_on_black);

  /* assert that the display is at least 24x80 */
  if (t->max_x < 80) return -11;
  if (t->max_y < 24) return -12;

  /* all done, no error */
  return 0;

} /* machterm_setup() */

