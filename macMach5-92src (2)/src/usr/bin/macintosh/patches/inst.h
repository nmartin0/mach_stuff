/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

/*
 * Format of
 * status register
 */
struct status_reg {
    union {
	struct {
	    unsigned short t1:1,
			   t0:1,
			    s:1,
			    m:1,
			     :1,
			  ipl:3,
			     :3,
			    x:1,
			    n:1,
			    z:1,
			    v:1,
			    c:1;
	} bits;
	struct {
	    unsigned char system;
	    unsigned char user;
	} bytes;
    } sr_un;
};
#define sr_t1 sr_un.bits.t1
#define sr_t0 sr_un.bits.t0
#define sr_s sr_un.bits.s
#define sr_m sr_un.bits.m
#define sr_ipl sr_un.bits.ipl
#define sr_x sr_un.bits.x
#define sr_n sr_un.bits.n
#define sr_z sr_un.bits.z
#define sr_v sr_un.bits.v
#define sr_c sr_un.bits.c
#define sr_sys sr_un.bytes.system
#define sr_user sr_un.bytes.user
#define sr_cc sr_user

/*
 * Generic operation
 * sizes
 */
#define OPSIZE_BYTE		0x0
#define OPSIZE_WORD		0x1
#define OPSIZE_LONG		0x2
#define OPSIZE_DBLONG		0x3
#define OPSIZE_NONE		0x4

/*
 * Instruction extension words
 */
struct exten_brief {
    unsigned short ext_iregtyp:1,
  		      ext_ireg:3,
		     ext_isize:1,
		     ext_scale:2,
		      ext_full:1,
		      ext_disp:8;
};

struct exten_full {
    unsigned short ext_iregtyp:1,
		      ext_ireg:3,
		     ext_isize:1,
		     ext_scale:2,
		      ext_full:1,
			ext_bs:1,
			ext_is:1,
		    ext_bdsize:2,
			      :1,
		      ext_isel:3;
};

/*
 * Thread data for instruction
 * emulation routines.
 */
typedef struct {
    thread_t			thread;
    thread_state_regs_t		*regs;
    unsigned			regs_count;
    thread_state_frame_t	*frame;
    unsigned			frame_count;
    thread_state_fpframe_t	*fp_frame;
    unsigned			fp_frame_count;
} inst_state_t;

/*
 * Memory Cell
 * allows access to
 * shorts and bytes.
 */
struct cell {
    union {
	struct {
	    unsigned char msbyte;
	    unsigned char msmbyte;
	    unsigned char lsmbyte;
	    unsigned char lsbyte;
	} bytes;
	struct {
	    unsigned short hiword;
	    unsigned short loword;
	} shortwords;
	unsigned char byte;
	unsigned short word;
	unsigned long longword;
    } un;
};

#define c_lsbyte un.bytes.lsbyte
#define c_msmbyte un.bytes.msmbyte
#define c_lsmbyte un.bytes.lsmbyte
#define c_lsbyte un.bytes.lsbyte
#define c_hiword un.shortwords.hiword
#define c_loword un.shortwords.loword
#define c_byte un.byte
#define c_word un.word
#define c_longword un.longword

/*
 * Instruction emulation
 * result codes.
 */
#define INST_ERROR	0
#define INST_DONE	1
#define INST_RETRY	2
