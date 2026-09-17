/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

static inline
void
fetch_n_byte(p, px, n)
{
    switch (n) {
      case 1:
	*(unsigned char *)px = *(unsigned char *)p; return;

      case 2:
	*(unsigned short *)px = *(unsigned short *)p; return;

      case 4:
	*(unsigned long *)px = *(unsigned long *)p; return;

      default:
	bcopy(p, px, n); return;
    }
}

static inline
void
store_n_byte(p, px, n)
{
    switch (n) {
      case 1:
	*(unsigned char *)p = *(unsigned char *)px; return;

      case 2:
	*(unsigned short *)p = *(unsigned short *)px; return;

      case 4:
	*(unsigned long *)p = *(unsigned long *)px; return;

      default:
	bcopy(px, p, n); return;
    }
}
