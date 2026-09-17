/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#define MAGIC0 0x1F
#define MAGIC1 0x9D
#define BITS 16
#define HSIZE 69001 /* 95% occupancy */
#define BIT_MASK 0x1f
#define BLOCK_MASK 0x80
#define INIT_BITS 9 /* initial number of bits/code */
#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)
#define FIRST 257 /* first free entry */
#define	CLEAR 256 /* table clear output code */

typedef long (*decompress_reader_t)(char *, char *, long);

typedef struct decompress_state {
  int n_bits;
  int maxbits;
  long maxcode;
  long maxmaxcode;
  int block_compress;
  long free_ent;
  int clear_flg;
  unsigned char rmask[9];
  char *x;
  decompress_reader_t reader;
  int offset;
  int size;
  unsigned char buf[BITS];
  long *htab /*[HSIZE]*/;
  unsigned short *tab_prefix/*[HSIZE]*/;
  int state;
  unsigned char *stackp;
  int finchar;
  long oldcode, incode;
} *decompress_state_t;

extern void setup_decompress(decompress_state_t s,
	long *htab,
	unsigned short *tab_prefix,
	char *x,
	decompress_reader_t reader);
extern long decompress_read(decompress_state_t s, char *buffer, long count);
