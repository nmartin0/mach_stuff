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

/* The machterm emulator module is organized as:
 *   addstr()            -- append a string, advance pointer
 *   addnum()            -- append a number, advance pointer
 *   termcap()           -- answerback with termcap entry
 *   bbits()             -- compute border bits
 *   border()            -- draw a horizontal border line
 *   cursor()            -- draw the cursor, toggle between on and off
 *   space()             -- draw a space at the specified location
 *   draw()              -- draw a character at the specified location
 *   clear()             -- clear one or more lines
 *   scroll()            -- scroll lines up or down onto specified line
 *   blink()             -- blink the display, a visual bell
 *   _machterm_setup()   -- setup display context
 *   machterm() -- run terminal emulator to display a character
 */

#include "machterm.h"

/* this context is used when a zero pointer is given */
struct context _machterm_default_context;

/* This is the termcap specification for the Mach Terminal Emulator. */
static char machterm_termcap[] =
  "mt|mach:"       /* term = "mach" */
  "do=^J:"         /* cursor down */
  "le=^H:"         /* cursor left */
  "bs:"            /* can backspace with ^H */
  "cd=\\EJ:"       /* clear to end of display */
  "ce=\\EK:"       /* clear to end of line */
  "cl=\\EH\\EJ:"   /* clear screen and home cursor */
  "cm=\\EY%+ %+ :" /* screen-relative cursor motion */
  "nd=\\EC:"       /* non-destrictive space, cursor right */
  "pt:"            /* has hardware tabs */
  "sr=\\EI:"       /* scroll text down */
  "up=\\EA:"       /* move cursor up */
  "ku=\\E[A:"      /* sent by up arrow key */
  "kd=\\E[B:"      /* sent by down arrow key */
  "kr=\\E[C:"      /* sent by right arrow key */
  "kl=\\E[D:"      /* sent by left arrow key */
  "kb=\\177:"      /* sent by backspace key */
  "kD=\\177:"      /* sent by delete key */
  "so=\\Ej:"       /* begin standout mode */
  "se=\\Ek:"       /* end standout mode */
  "us=\\El:"       /* start underscore mode */
  "ue=\\Em:"       /* end underscore mode */
  "as=\\En:"       /* start alternate character set */
  "ae=\\Eo:"       /* end alternate character set */
#ifndef SMALL_MACHTERM
  "md=\\Ep:"       /* turn on bold attribute */
#endif
  "me=\\Eq:"       /* turn off all attributes */
  "bl=^G:"         /* bell */
  "vb=^F:"         /* visable bell */
  "hs:"            /* has extra status line */
  "es:"            /* escape can be used on status line */
  "ts=\\ES%+ :"    /* go to status line, column n */
  "fs=^B:";        /* return from status line */
  /* co */         /* number of columns in a line */
  /* li */         /* number of lines on screen */
  /* ws */         /* number of columns in status line */
  /* co, li and ws are computed by termcap() */

/* The following codes are available in addition to those defined in the
 * termcap entry:
 * ^A  -- enter and clear the status line
 * ^B  -- exit the status line
 * ^C  -- return the termcap entry
 * ^D  -- toggle between white and black
 * ^E% -- set cursor graphics character
 */

/*****************************************************************************/

/* append a string, advance pointer */
static void addstr(register char **p, register char *b)
{
  while (*b) *(*p)++ = *b++;
}

/* append a number, advance pointer */
static void addnum(register char **p, register int n, register int maxw)
{
  char buffer[30];
  register char *b = &buffer[sizeof(buffer) - 1];

  *(b--) = 0;
  do {
    *b-- = '0' + n % 10;
    n /= 10;
  } while ((--maxw > 0) && (n != 0));
  b++;
  addstr(p, b);
}

/* answerback with termcap entry */
static void termcap(register context_t t)
{
  char buffer[sizeof(machterm_termcap) + 23], *p;

  /* if no input write function, do nothing */
  if (!t->ungetc) return;

  /* compose termcap entry */
  *(p = buffer) = 0;
  addstr(&p, machterm_termcap);
  addstr(&p, "co#"); addnum(&p, t->max_x, 3);
  addstr(&p, ":li#"); addnum(&p, t->max_y, 3);
  addstr(&p, ":ws#"); addnum(&p, t->max_x, 3);
  addstr(&p, ":\015");
  *p = 0;

  /* write termcap entry to input stream */
  for (p = buffer; *p; p++) (*t->ungetc)(*p, t->ungetc_argument);

} /* termcap() */

/*****************************************************************************/

/* compute border bits */
static void bbits(register context_t t,
                  char *bits,
                  unsigned int *left,
                  unsigned int *right1,
                  unsigned int *right2,
                  unsigned int *filler)
{
  unsigned int left_bits, right_bits;
  int i, shift;

  /* compute filler bits */
  *filler = (bits[VBORDER] == '0') ? 0x00000000 : 0xFFFFFFFF;

  /* compute left and right border bits */
  left_bits = 0xFFFFFFFF;
  right_bits = *filler;
  for (i = 0; i < VBORDER; i++) {
    left_bits = (left_bits << 1) | ((bits[i] == '0') ? 0 : 1);
    right_bits =
      (right_bits << 1) | ((bits[VBORDER - i - 1] == '0') ? 0 : 1);
  }

  /* compute left border */
  shift = INTBITS - t->lmargin;
  *left = (left_bits << shift) | (*filler >> (INTBITS - shift));

  /* compute right border */
  if ((shift = SHIFT(t, t->rmargin, VBORDER)) < 0) {
    shift = - shift;
    *right1 = (right_bits >> shift) | (*filler << (INTBITS - shift));
    *right2 = (right_bits << (INTBITS - shift)) | (0xFFFFFFFF >> shift);
  }
  else {
    *right1 = (right_bits << shift) | (0xFFFFFFFF >> (INTBITS - shift));
    *right2 = 0;
  }

} /* bbits() */

/* draw a horizontal border line */
static void border(context_t t, int y, char *bits)
{
  unsigned int left, right1, right2, filler;
  register unsigned int *b;
  register int x;

  bbits(t, bits, &left, &right1, &right2, &filler);
  b = &t->buffer[y * t->rowints];
  *b++ = left;
  if (right2) {
    for (x = t->rowsize - 3; x--;) *b++ = filler;
    *b++ = right1;
    *b++ = right2;
  }
  else {
    for (x = t->rowsize - 2; x--;) *b++ = filler;
    *b++ = right1;
  }

} /* border() */

/*****************************************************************************/

/* draw the cursor, toggle between on and off */
static void cursor(register context_t t)
{
  register unsigned int *b, mask1, mask2;
  register unsigned char *bits1;
#ifndef SMALL_MACHTERM
  register unsigned short *bits2;
#endif
  register int shift1, shift2, i, x, y;

  /* get pointer to font bits */
#ifndef SMALL_MACHTERM
  if (t->font_width > CHARBITS)
    bits2 = &((unsigned short *)t->font)[t->cursor * t->font_height];
  else
#endif
    bits1 = &((unsigned char *)t->font)[t->cursor * t->font_height];

  /* convert to pixel coordinates */
  x = t->cursor_x; y = t->cursor_y;
  TOPIXEL(t, x, y);

  /* get pointer to pixel's int */
  b = &t->buffer[INDEX(t, x, y)];

  /* character spans two adjacent int's */
  if ((shift1 = SHIFT(t, x, t->font_width)) < 0) {
    mask1 = t->mask >> (shift1 = -shift1);
    mask2 = t->mask << (shift2 = INTBITS - shift1);

#ifndef SMALL_MACHTERM
    /* font bits contained in two bytes */
    if (t->font_width > CHARBITS) {
      for (i = t->font_height - 1; i; i--, b += t->rowints, bits2++) {
        *b = RCURSOR(*b, mask1, *bits2, shift1);
        b[1] = LCURSOR(b[1], mask2, *bits2, shift2);
      }
      *b++ = RCURSOR(*b, mask1, *bits2, shift1);
      *b = LCURSOR(*b, mask2, *bits2, shift2);
    }

    /* font bits contained in one byte */
    else {
#endif
      for (i = t->font_height - 1; i; i--, b += t->rowints, bits1++) {
        *b = RCURSOR(*b, mask1, *bits1, shift1);
        b[1] = LCURSOR(b[1], mask2, *bits1, shift2);
      }
      *b++ = RCURSOR(*b, mask1, *bits1, shift1);
      *b = LCURSOR(*b, mask2, *bits1, shift2);
#ifndef SMALL_MACHTERM
    }
#endif

  }

  /* character contained in one word */
  else {
    mask1 = t->mask << shift1;

#ifndef SMALL_MACHTERM
    /* font bits contained in two bytes */
    if (t->font_width > CHARBITS) {
      for (i = t->font_height - 1; i; i--, b += t->rowints, bits2++)
        *b = LCURSOR(*b, mask1, *bits2, shift1);
      *b = LCURSOR(*b, mask1, *bits2, shift1);
    }

    /* font bits contained in one byte */
    else {
#endif
      for (i = t->font_height - 1; i; i--, b += t->rowints, bits1++)
        *b = LCURSOR(*b, mask1, *bits1, shift1);
      *b = LCURSOR(*b, mask1, *bits1, shift1);
#ifndef SMALL_MACHTERM
    }
#endif

  }

} /* cursor() */

/*****************************************************************************/

/* draw a space at the specified location */
static void space(register context_t t, register int x, register int y)
{
  register unsigned int *b, space1, space2;
  register int shift, i;
  int normal;

  /* get display or status line mode */
  if (y == t->max_y) normal = t->status_mode & INVERSE;
  else if (t->white_on_black) normal = t->mode & INVERSE;
  else normal = !(t->mode & INVERSE);

  /* convert to pixel coordinates */
  TOPIXEL(t, x, y);

  /* get pointer to pixel's int */
  b = &t->buffer[INDEX(t, x, y)];

  /* character spans two adjacent int's */
  if ((shift = SHIFT(t, x, t->font_width)) < 0) {
    space1 = t->mask >> (-shift);
    space2 = t->mask << (INTBITS + shift);

    /* normal -- black on white space */
    if (normal) {
      for (i = t->font_height - 1; i; i--, b += t->rowints) {
        *b &= ~space1;
        b[1] &= ~space2; 
      }
      *b++ &= ~space1;
      *b &= ~space2;
    }

    /* inverse -- white on black space */
    else {
      for (i = t->font_height - 1; i; i--, b += t->rowints) {
        *b |= space1;
        b[1] |= space2;
      }
      *b++ |= space1;
      *b |= space2;
    }

  }

  /* character contained in one word */
  else {
    space1 = t->mask << shift;

    /* normal -- black on white space */
    if (normal) {
      for (i = t->font_height - 1; i; i--, b += t->rowints) *b &= ~space1;
      *b &= ~space1;
    }

    /* inverse -- white on black space */
    else {
      for (i = t->font_height - 1; i; i--, b += t->rowints) *b |= space1;
      *b |= space1;
    }

  }

} /* space() */

/*****************************************************************************/

/* draw a character */
static void draw(register context_t t, register int x, register int y, char c)
{
  register unsigned int *b, mask1, mask2;
  register unsigned char *bits1;
#ifndef SMALL_MACHTERM
  register unsigned short *bits2;
#endif
  register int shift1, shift2, i;
  unsigned int mode;

  /* get display or status line mode */
  if (y == t->max_y) mode = t->status_mode ^ INVERSE;
  else if (t->white_on_black) mode = t->mode ^ INVERSE;
  else mode = t->mode;

  /* get pointer to font bits */
  if (mode & GRAPHIC) c = TOGRAPHIC(c);
#ifndef SMALL_MACHTERM
  if (t->font_width > CHARBITS) {
    if (mode & BOLD) bits2 = &((unsigned short *)t->bold)[c * t->font_height];
    else bits2 = &((unsigned short *)t->font)[c * t->font_height];
  }
  else {
#endif
#ifndef SMALL_MACHTERM
    if (mode & BOLD) bits1 = &((unsigned char *)t->bold)[c * t->font_height];
#endif
    bits1 = &((unsigned char *)t->font)[c * t->font_height];
#ifndef SMALL_MACHTERM
  }
#endif

  /* convert to pixel coordinates */
  TOPIXEL(t, x, y);

  /* get pointer to pixel's int */
  b = &t->buffer[INDEX(t, x, y)];

  /* character spans two adjacent int's */
  if ((shift1 = SHIFT(t, x, t->font_width)) < 0) {
    mask1 = t->mask >> (shift1 = -shift1);
    mask2 = t->mask << (shift2 = INTBITS - shift1);

    /* display white on black */
    if (mode & INVERSE) {

#ifndef SMALL_MACHTERM
      /* font bits contained in two bytes */
      if (t->font_width > CHARBITS) {
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits2++) {
          *b = RWBCHAR(*b, mask1, *bits2, shift1);
          b[1] = LWBCHAR(b[1], mask2, *bits2, shift2);
        }
        if (mode & UNDERLINE) {
          *b++ = RWBCHAR(*b, mask1, t->mask, shift1);
          *b = LWBCHAR(*b, mask2, t->mask, shift2);
        }
        else {
          *b++ = RWBCHAR(*b, mask1, *bits2, shift1);
          *b = LWBCHAR(*b, mask2, *bits2, shift2);
        }
      }

      /* font bits contained in one byte */
      else {
#endif
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits1++) {
          *b = RWBCHAR(*b, mask1, *bits1, shift1);
          b[1] = LWBCHAR(b[1], mask2, *bits1, shift2);
        }
        if (mode & UNDERLINE) {
          *b++ = RWBCHAR(*b, mask1, t->mask, shift1);
          *b = LWBCHAR(*b, mask2, t->mask, shift2);
        }
        else {
          *b++ = RWBCHAR(*b, mask1, *bits1, shift1);
          *b = LWBCHAR(*b, mask2, *bits1, shift2);
        }
#ifndef SMALL_MACHTERM
      }
#endif
    }

    /* display black on white */
    else {

#ifndef SMALL_MACHTERM
      /* font bits contained in two bytes */
      if (t->font_width > CHARBITS) {
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits2++) {
          *b = RBWCHAR(*b, mask1, *bits2, shift1);
          b[1] = LBWCHAR(b[1], mask2, *bits2, shift2);
        }
        if (mode & UNDERLINE) {
          *b++ = RBWCHAR(*b, mask1, t->mask, shift1);
          *b = LBWCHAR(*b, mask2, t->mask, shift2);
        }
        else {
          *b++ = RBWCHAR(*b, mask1, *bits2, shift1);
          *b = LBWCHAR(*b, mask2, *bits2, shift2);
        }
      }

      /* font bits contained in one byte */
      else {
#endif
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits1++) {
          *b = RBWCHAR(*b, mask1, *bits1, shift1);
          b[1] = LBWCHAR(b[1], mask2, *bits1, shift2);
        }
        if (mode & UNDERLINE) {
          *b++ = RBWCHAR(*b, mask1, t->mask, shift1);
          *b = LBWCHAR(*b, mask2, t->mask, shift2);
        }
        else {
          *b++ = RBWCHAR(*b, mask1, *bits1, shift1);
          *b = LBWCHAR(*b, mask2, *bits1, shift2);
        }
#ifndef SMALL_MACHTERM
      }
#endif
    }
  }

  /* character contained in one word */
  else {
    mask1 = t->mask << shift1;

    /* display white on black */
    if (mode & INVERSE) {

#ifndef SMALL_MACHTERM
      /* font bits contained in two bytes */
      if (t->font_width > CHARBITS) {
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits2++)
          *b = LWBCHAR(*b, mask1, *bits2, shift1);
        if (mode & UNDERLINE) *b = LWBCHAR(*b, mask1, t->mask, shift1);
        else *b = LWBCHAR(*b, mask1, *bits2, shift1);
      }

      /* font bits contained in one byte */
      else {
#endif
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits1++)
          *b = LWBCHAR(*b, mask1, *bits1, shift1);
        if (mode & UNDERLINE) *b = LWBCHAR(*b, mask1, t->mask, shift1);
        else *b = LWBCHAR(*b, mask1, *bits1, shift1);
#ifndef SMALL_MACHTERM
      }
#endif

    }

    /* display black on white */
    else {

#ifndef SMALL_MACHTERM
      /* font bits contained in two bytes */
      if (t->font_width > CHARBITS) {
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits2++)
          *b = LBWCHAR(*b, mask1, *bits2, shift1);
        if (mode & UNDERLINE) *b = LBWCHAR(*b, mask1, t->mask, shift1);
        else *b = LBWCHAR(*b, mask1, *bits2, shift1);
      }

      /* font bits contained in one byte */
      else {
#endif
        for (i = t->font_height - 1; i; i--, b += t->rowints, bits1++)
          *b = LBWCHAR(*b, mask1, *bits1, shift1);
        if (mode & UNDERLINE) *b = LBWCHAR(*b, mask1, t->mask, shift1);
        else *b = LBWCHAR(*b, mask1, *bits1, shift1);
#ifndef SMALL_MACHTERM
      }
#endif

    }

  }

} /* draw() */

/*****************************************************************************/

/* clear one or more lines */
static void clear(register context_t t,
                  register int x,
                  register int y,
                  register int count)
{
  register unsigned int *b, *bb;

  /* clear a partial line */
  if (x) {
    while (x < t->max_x) space(t, x++, y);
    y++;
    if (--count <= 0) return;
  }

  /* if status line, ignore count */
  if (y == t->max_y) {
    b = &t->buffer[STATUSINDEX(t)];
    for (y = t->font_height; y--; b += t->rowints)
      for (bb = b, x = t->rowsize; x--;) *bb++ = -1;
  }

  /* clear specified number of lines */
  else {
    b = &t->buffer[ROWINDEX(t, y)];

    /* white on black display has no border */
    if (t->white_on_black) {
      while (count--)
        for (y = t->font_height; y--; b += t->rowints)
          for (bb = b, x = t->rowsize; x--;) *bb++ = -1;
    }

    /* right border bits split over two ints */
    else if (t->right2) {
      while (count--) for (y = t->font_height; y--; b += t->rowints) {
        bb = b;
        *bb++ = t->left;
        for (x = t->rowsize - 3; x--;) *bb++ = 0;
        *bb++ = t->right1;
        *bb++ = t->right2;
      }
    }

    /* right border bits fit into one int */
    else {
      while (count--) for (y = t->font_height; y--; b += t->rowints) {
        bb = b;
        *bb++ = t->left;
        for (x = t->rowsize - 2; x--;) *bb++ = 0;
        *bb++ = t->right1;
      }
    }

  }

} /* clear() */

/*****************************************************************************/

/* scroll lines up or down onto specified line */
static void scroll(register context_t t, register int y, int down)
{
  register unsigned int *to_row, *to_int, *from_row, *from_int;
  register int count, x;

  /* scroll from above down onto line */
  if (down) {
    to_row = &t->buffer[ROWINDEX(t, y)];
    from_row = &t->buffer[ROWINDEX(t, y - 1)];
    for (count = (y - 1) * t->font_height; count--;) {
      to_int = to_row;
      from_int = from_row;
      for (x = t->rowsize; x--;) *to_int-- = *from_int--;
      from_row -= t->rowints;
      to_row -= t->rowints;
    }
  }

  /* scroll from below up onto line */
  else {
    to_row = &t->buffer[ROWINDEX(t, y)];
    from_row = &t->buffer[ROWINDEX(t, y + 1)];
    for (count = (t->max_y - y - 1) * t->font_height; count--;) {
      to_int = to_row;
      from_int = from_row;
      for (x = t->rowsize; x--;) *to_int++ = *from_int++;
      from_row += t->rowints;
      to_row += t->rowints;
    }
  }

} /* scroll() */

/*****************************************************************************/

/* blink the display, a visual bell */
static void blink(register context_t t)
{
  register unsigned int *b, *bb;
  register int count, x;

  /* make white black and black white */
  b = &t->buffer[ROWINDEX(t, 0)];
  for (count = t->max_y * t->font_height; count--; b += t->rowints)
    for (bb = b, x = t->rowsize; x--;) *bb = ~*bb++;

  /* make black white and white black */
  b = &t->buffer[ROWINDEX(t, 0)];
  for (count = t->max_y * t->font_height; count--; b += t->rowints)
    for (bb = b, x = t->rowsize; x--;) *bb = ~*bb++;

} /* blink() */

/*****************************************************************************/

/* setup display context */
void _machterm_setup(register context_t t,
                     char initial_cursor,
                     int white_on_black)
{
  register unsigned int *b, *bb;
  register int x, y;
  unsigned int filler;

  /* calculate width (in characters) and left margin */
  t->max_x = t->width / t->font_width;
  t->lmargin = (t->width - (t->max_x * t->font_width)) / 2;
  while (t->lmargin < VBORDER) {
    t->max_x--;
    t->lmargin = (t->width - (t->max_x * t->font_width)) / 2;
  }

  /* calculate height (in characters) and top margin */
  t->max_y = t->height / t->font_height;
  t->tmargin = (t->height - (t->max_y * t->font_height)) / 2;
  while (t->tmargin < HBORDER) {
    t->max_y--;
    t->tmargin = (t->height - (t->max_y * t->font_height)) / 2;
  }

  /* status line is y == max_y */
  t->max_y--;

  /* calculate right margin */
  /* this is the first pixel of the right border */
  t->rmargin = t->lmargin + (t->max_x * t->font_width);

  /* calculate rowsize */
  /* this is the number of int's actually displayed */
  t->rowsize = (t->rmargin / INTBITS) + 1;

  /* compute font mask */
  t->mask = (1 << t->font_width) - 1;

  /* make entire display black */
  for (b = t->buffer, y = t->height; y--; b += t->rowints)
    for (bb = b, x = t->rowsize; x--;) *bb++ = -1;

  /* if black on white display, draw borders */
  if (!(t->white_on_black = white_on_black)) {

    /* draw top border */
    border(t, t->tmargin - 3, "11000");
    border(t, t->tmargin - 2, "10011");
    border(t, t->tmargin - 1, "00100");

    /* compute side border bits, clear() draws them */
    bbits(t, "01000", &t->left, &t->right1, &t->right2, &filler);
    clear(t, 0, 0, t->max_y);

    /* draw bottom border */
    border(t, t->max_y * t->font_height + t->tmargin + 0, "00100");
    border(t, t->max_y * t->font_height + t->tmargin + 1, "10011");
    border(t, t->max_y * t->font_height + t->tmargin + 2, "11000");

  }

  /* setup the emulator state */
  t->state = t->mode = t->status = t->status_mode = t->am_count = 0;

  /* draw the initial cursor */
  t->cursor = initial_cursor;
  t->cursor_x = t->cursor_y = 0;
  cursor(t);

} /* _machterm_setup() */

/*****************************************************************************/

/* run terminal emulator to display a character */
#ifdef SMALL_MACHTERM
void small_machterm(machterm_context_t *machterm_context, register char c)
#else
void machterm(machterm_context_t *machterm_context, register char c)
#endif
{
  register context_t t = (context_t)machterm_context;

  /* use default context if zero pointer */
  if (!t) t = &_machterm_default_context;

  /* if margin hit last time, increment the count for possible second time */
  if (t->am_count) t->am_count++;

  /* remove the cursor */
  cursor(t);

  /* a simple state machine */
  switch (t->state) {

    case 0: /* state #0 -- not in escape sequence */
      c &= 127;
      switch (c) {
        case 'A' - '@': /* ^A, enter and clear the status line */
          t->status = 1;
          t->status_mode = 0;
          clear(t, 0, t->max_y, 1);
          break;
        case 'B' - '@': /* ^B, exit the status line */
          t->status = 0;
          break;
        case 'C' - '@': /* ^C, return the termcap entry */
          termcap(t);
          break;
        case 'D' - '@': /* ^D, toggle between white and black */
          _machterm_setup(t, t->cursor, !t->white_on_black);
          break;
        case 'E' - '@': /* ^E, next character will be new cursor */
          t->state = 5;
          break;
        case 'F' - '@': /* ^F, blink */
          blink(t);
          break;
        case 'G' - '@': /* ^G, beep */
          if (t->bell) (*t->bell)();
          else blink(t);
          break;
        case 'H' - '@': /* le, cursor left */
          if (t->status) {
            if (t->status > 1) t->status--;
          }
          else if (t->cursor_x > 0) t->cursor_x--;
          break;
        case 'I' - '@': /* ^I, tab */
          if (t->status) {
            t->status += (8 - ((t->status - 1) % 8));
            if (t->status > t->max_x) t->status = t->max_x;
          }
          else {
            t->cursor_x += (8 - (t->cursor_x % 8));
            if (t->cursor >= t->max_x) t->cursor_x = t->max_x - 1;
          }
          break;
        case 'J' - '@': /* do, down one line */
          if (t->status) t->status = 0;
          else {
            t->cursor_y++;
            if (t->cursor_y >= t->max_y) {
              t->cursor_y = t->max_y - 1;
              scroll(t, 0, 0);
              clear(t, 0, t->max_y - 1, 1);
            }
          }
          break;
        case 'M' - '@': /* ^M, carriage return */
          if (t->status) t->status = 1;
          else t->cursor_x = 0;
          break;
        case '[' - '@': /* begin escape sequence */
          t->state = 1;
          break;
        default: /* paint the character at cursor position, advance cursor */
          if (t->status) {
            if ((c <= ' ') || (c == 127))
              space(t, t->status++ - 1, t->max_y);
            else draw(t, t->status++ - 1, t->max_y, c);
            if (t->status > t->max_x) t->status = 1;
          }
          else {
            if (t->am_count > 1) { /* am, automatic margin */
              t->cursor_x = 0;
              t->cursor_y++;
              if (t->cursor_y >= t->max_y) {
                t->cursor_y = t->max_y - 1;
                scroll(t, 0, 0);
                clear(t, 0, t->max_y - 1, 1);
              }
              t->am_count = 0;
            }
            if ((c <= ' ') || (c == 127))
              space(t, t->cursor_x++, t->cursor_y);
            else draw(t, t->cursor_x++, t->cursor_y, c);
            /* if at margin, leave the cursor there and start the t->am_count */
            if (t->cursor_x >= t->max_x) {
              t->am_count = 1;
              t->cursor_x--;
            }
          }
      }
      break;

    case 1: /* state #1 -- first char of escape sequence */
      switch (c) {
        case 'Y': /* cm, new cursor position follows */
          t->state = 2;
          break;
        case 'S': /* ts, status line column follows */
          t->state = 4;
          break;
        case 'J': /* cd, clear from cursor to end of display */
          t->state = 0;
          if (t->status) clear(t, t->status - 1, t->max_y, 1);
          else clear(t, 0, 0, t->max_y);
          break;
        case 'K': /* ce, clear to end of line */
          t->state = 0;
          if (t->status) clear(t, t->status - 1, t->max_y, 1);
          else clear(t, t->cursor_x, t->cursor_y, 1);
          break;
        case 'H': /* ho, home cursor */
          t->state = 0;
          if (t->status) t->status = 1;
          else t->cursor_x = t->cursor_y = 0;
          break;
        case 'C': /* nd, cursor right */
          t->state = 0;
          if (t->status) {
            t->status++;
            if (t->status > t->max_x) t->status = t->max_x;
          }
          else {
            t->cursor_x++;
            if (t->cursor_x >= t->max_x) t->cursor_x = t->max_x - 1;
          }
          break;
        case 'I': /* sr, scroll text down */
          t->state = 0;
          if (t->status) break;
          t->cursor_y++;
          if (t->cursor_y >= t->max_y) t->cursor_y = t->max_y - 1;
          scroll(t, t->max_y - 1, -1);
          clear(t, 0, 0, 1);
          break;
        case 'A': /* up, cursor up */
          t->state = 0;
          if (t->status) break;
          t->cursor_y--;
          if (t->cursor_y < 0) t->cursor_y = 0;
          break;
        case 'j': /* so, mr, start standout mode */
          t->state = 0;
          if (t->status) t->status_mode |= INVERSE;
          else t->mode |= INVERSE;
          break;
        case 'k': /* se, end standout mode */
          t->state = 0;
          if (t->status) t->status_mode &= ~INVERSE;
          else t->mode &= ~INVERSE;
          break;
        case 'l': /* us, start underline mode */
          t->state = 0;
          if (t->status) t->status_mode |= UNDERLINE;
          else t->mode |= UNDERLINE;
          break;
        case 'm': /* ue, end underline mode */
          t->state = 0;
          if (t->status) t->status_mode &= ~UNDERLINE;
          else t->mode &= ~UNDERLINE;
          break;
        case 'n': /* as, start alternate (graphic) mode */
          t->state = 0;
          if (t->status) t->status_mode |= GRAPHIC;
          else t->mode |= GRAPHIC;
          break;
        case 'o': /* ae, end alternate (graphic) mode */
          t->state = 0;
          if (t->status) t->status_mode &= ~GRAPHIC;
          else t->mode &= ~GRAPHIC;
          break;
#ifndef SMALL_MACHTERM
        case 'p': /* md, start bold mode */
          t->state = 0;
          if (t->status) t->status_mode |= BOLD;
          else t->mode |= BOLD;
          break;
#endif
        case 'q': /* me, end all modes */
          t->state = 0;
          if (t->status) t->status_mode = 0;
          else t->mode = 0;
          break;
        default:
          blink(t);
      }
      break;

    case 2: /* state #2 -- pick up new cursor Y */
      t->new_y = c - ' ';
      t->state = 3;
      break;

    case 3: /* state #3 -- pick up new cursor X, set new cursor position */
      t->new_x = c - ' ';
      t->state = 0;
      if ((t->new_x < 0) || (t->new_x >= t->max_x)) {
        blink(t);
        break;
      }
      if ((t->new_y < 0) || (t->new_y >= t->max_y)) {
        blink(t);
        break;
      }
      t->cursor_x = t->new_x;
      t->cursor_y = t->new_y;
      break;

    case 4: /* state #4 -- pick up status column */
      t->new_x = c - ' ';
      t->state = 0;
      if ((t->new_x < 0) || (t->new_x >= t->max_x)) {
        blink(t);
        t->new_x = 0;
      }
      t->status = t->new_x + 1;
      break;

    case 5: /* state #5 -- set new cursor character */
      t->state = 0;
      if ((c < 'b') || (c > 'h')) {
        blink(t);
        break;
      }
      t->cursor = TOGRAPHIC(c);
      break;

  } /* switch(t->state) */

  /* restore cursor at new position */
  cursor(t);

  /* if automatic margin ready but not done, clear the count */
  if (t->am_count > 1) t->am_count = 0;

} /* machterm() */
