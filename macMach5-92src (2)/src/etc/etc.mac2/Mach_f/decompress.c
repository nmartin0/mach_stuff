/* BSD compatible file decompression */

#undef MAIN

/* This decompression code is based on the Berkeley compress utility code
 * which carries the following copyright:
 *
 * Copyright (c) 1985, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James A. Woods, derived from original work by Spencer Thomas
 * and Joseph Orost.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "decompress.h"

void setup_decompress(decompress_state_t s,
	long *htab,
	unsigned short *tab_prefix,
	char *x,
	decompress_reader_t reader)
{
  s->x = x;
  s->reader = reader;
  s->offset = 0;
  s->size = 0;
  s->state = 0;
  if ((s->size = (*reader)(x, (char *)s->buf, 3)) == 3) {
    if ((s->buf[0] == MAGIC0) && (s->buf[1] == MAGIC1)) {
      s->maxbits = s->buf[2];
      s->block_compress = s->maxbits & BLOCK_MASK;
      s->maxbits &= BIT_MASK;
      s->maxmaxcode = 1 << s->maxbits;
      if (s->maxbits <= BITS) s->state = 1;
      s->maxbits = BITS;
      s->maxmaxcode = 1 << BITS;
      s->block_compress = BLOCK_MASK;
      s->free_ent = 0;
      s->clear_flg = 0;
      s->rmask[0] = 0x00;
      s->rmask[1] = 0x01;
      s->rmask[2] = 0x03;
      s->rmask[3] = 0x07;
      s->rmask[4] = 0x0F;
      s->rmask[5] = 0x1F;
      s->rmask[6] = 0x3F;
      s->rmask[7] = 0x7F;
      s->rmask[9] = 0xFF;
      s->size = 0;
	  s->htab = htab;
	  s->tab_prefix = tab_prefix;
    }
  }
}

/* Read the next input code, return -1 if end of file */
static long getcode(decompress_state_t s) {
  register long code;
  register int r_off, bits;
  register unsigned char *bp = s->buf;

  if ((s->clear_flg > 0) || (s->offset >= s->size) || (s->free_ent > s->maxcode)) {
    /* If the next entry will be too big for the current code
     * size, then we must increase the size.  This implies reading
     * a new buffer full, too.
     */
    if (s->free_ent > s->maxcode) {
      s->n_bits++;
      if (s->n_bits == s->maxbits) s->maxcode = s->maxmaxcode;
      else s->maxcode = MAXCODE(s->n_bits);
    }
    if (s->clear_flg > 0) {
      s->maxcode = MAXCODE(s->n_bits = INIT_BITS);
      s->clear_flg = 0;
    }
    if ((s->size = s->reader(s->x, (char *)s->buf, s->n_bits)) <= 0) {
      code = -1;
      goto done;
    }
    s->offset = 0;
    /* Round size down to integral number of codes */
    s->size = (s->size << 3) - (s->n_bits - 1);
  }
  r_off = s->offset;
  bits = s->n_bits;
  /* Get to the first byte. */
  bp += (r_off >> 3);
  r_off &= 7;
  /* Get first part (low order bits) */
  code = *bp++ >> r_off;
  bits -= 8 - r_off;
  r_off = 8 - r_off; /* now, offset into code word */
  /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
  if (bits >= 8) {
    code |= *bp++ << r_off;
    r_off += 8;
    bits -= 8;
  }
  /* high order bits. */
  code |= (*bp & s->rmask[bits]) << r_off;
  s->offset += s->n_bits;
done:
  return code;
}

#define tab_prefixof(i)	s->tab_prefix[i]
#define tab_suffixof(i)	((unsigned char *)(s->htab))[i]
#define de_stack ((unsigned char *)&tab_suffixof(1 << BITS))

static long decompress(decompress_state_t s, char *buffer, long count)
{
  long n;
  long code;

  n = 0;
  if (s->state == 1) {
    /* Initialize the first 256 entries in the table. */
    s->maxcode = MAXCODE(s->n_bits = INIT_BITS);
    for (code = 255; code >= 0; code--) {
      tab_prefixof(code) = 0;
      tab_suffixof(code) = (unsigned char)code;
    }
    s->free_ent = ((s->block_compress) ? FIRST : 256 );
    s->finchar = s->oldcode = getcode(s);
    if(s->oldcode == -1) return 0;
    *buffer = (char)s->finchar; /* first code must be 8 bits = char */
    n = 1;
    s->stackp = de_stack;
    s->state = 2;
  }
loop:
  if (s->state == 2) {
    if ((code = getcode(s)) == -1) return n;
    if ((code == CLEAR) && s->block_compress) {
      for (code = 255; code >= 0; code--) tab_prefixof(code) = 0;
      s->clear_flg = 1;
      s->free_ent = FIRST - 1;
      if ((code = getcode(s)) == -1) return n;
    }
    s->incode = code;
    /* Special case for KwKwK string. */
    if (code >= s->free_ent ) {
      *s->stackp++ = s->finchar;
      code = s->oldcode;
    }
    /* Generate output characters in reverse order */
    while (code >= 256) {
      *s->stackp++ = tab_suffixof(code);
      code = tab_prefixof(code);
    }
    *s->stackp++ = s->finchar = tab_suffixof(code);
  }
  s->state = 3;
  /* And put them out in forward order */
  if (s->stackp > de_stack) do {
    buffer[n++] = *--(s->stackp);
    if (n == count) return n;
  } while ( s->stackp > de_stack );
  /* Generate the new entry. */
  if ((code = s->free_ent) < s->maxmaxcode) {
    tab_prefixof(code) = (unsigned short)s->oldcode;
    tab_suffixof(code) = s->finchar;
    s->free_ent = code + 1;
  } 
  /* Remember previous code. */
  s->oldcode = s->incode;
  s->state = 2;
  goto loop;
}

long decompress_read(decompress_state_t s, char *buffer, long count)
{
  long n, more, i;

  if (!s->state) {
    if (s->size && (s->offset < s->size)) {
      n = s->size - s->offset;
      if (count < n) n = count;
	  for (i = 0; i < n; i++) buffer[i] = s->buf[s->offset + i];
      s->offset += n;
      if ((more = count - n) > 0) {
        more = (*s->reader)(s->x, &buffer[n], more);
        if (more < 0) return more;
        n += more;
      }
    }
    else n = (*s->reader)(s->x, buffer, count);
  }
  else n = decompress(s, buffer, count);
  return n;
}

#ifdef MAIN

extern int read(int, char *, int);
extern int write(int, char *, int);

/* decompress stdin to stdout */
main()
{
  struct decompress_state s;
  long htab[HSIZE];
  unsigned char tab_prefix[HSIZE];
  char buffer[512];
  int n;

  setup_decompress(&s, htab, tab_prefix, (char *)0, (decompress_reader_t)read);
  while ((n = decompress_read(&s, buffer, 512)) > 0) write(1, buffer, n);
}

#endif
