/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

    /*
     *	opcodes of the call instructions
     */
#define	JSR	((0x4e<<2)+2)
#define BSR	0x61

    /*
     *	register for pc relative addressing
     */
#define	PC	0x7

enum opermodes {
    datareg, addrreg, defer, postinc, predec, displ, indexed,
    shortabsolute, longabsolute, pcrel, pcindexed, immediate
};
typedef enum opermodes	operandenum;

    /*
     *	offset (in bytes) of the code from the entry address of a routine.
     *	(see asgnsamples for use and explanation.)
     */
#define OFFSET_OF_CODE	0 /* there is no mask on a Sun */
#define	UNITS_TO_CODE	(OFFSET_OF_CODE / sizeof(UNIT))
