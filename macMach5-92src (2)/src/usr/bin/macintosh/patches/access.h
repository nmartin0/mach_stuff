/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

#include <access_inline.c>

/*
 * Fetch the value in user memory
 * addressed by p having the
 * type t and store it in
 * the variable addressed by px,
 * incrementing p in the process.
 */
#define FETCH_INCR(p, px, t)	\
    fetch_n_byte(((t *)(p))++, (px), sizeof (t))

/*
 * Fetch the value in user memory
 * addressed by p having the
 * type t and store it in the variable
 * addressed by px.
 */
#define FETCH(p, px, t)	\
    fetch_n_byte((p), (px), sizeof (t))

/*
 * Fetch the value in user memory
 * addressed by p of size n and store
 * it in the variable addressed by px.
 */
#define FETCH_N(p, px, n) \
    fetch_n_byte((p), (px), (n))

/*
 * Store the value in the variable
 * addressed by px having the type t
 * in user memory addressed by p.
 */
#define STORE(p, px, t)	\
    store_n_byte((p), (px), sizeof (t))

/*
 * Store the value in the variable
 * addressed by px of size n in
 * user memory addressed by p.
 */
#define STORE_N(p, px, n) \
    store_n_byte((p), (px), (n))
