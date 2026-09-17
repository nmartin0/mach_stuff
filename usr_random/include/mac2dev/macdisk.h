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

/* Macintosh disk driver for MACH 3.0
 *
 * Authors: David E. Bohman II (CMU macmach)
 *          Zonnie L. Williamson (CMU macmach)
 */

#ifndef _MAC2DEV_MACDISK_H_
#define _MAC2DEV_MACDISK_H_

#define MACDISK_PART_INFO 0

typedef struct {
  unsigned parts;         /* num of partitions from map */
  unsigned length;        /* of partition in blocks */
  unsigned char name[32];
  unsigned char type[32];
} macdisk_part_info_t;

#define MACDISK_PART_INFO_COUNT (sizeof(macdisk_part_info_t) / sizeof(int))

#define MACDISK_REC_SIZE      512
#define MACDISK_REC_SIZE_LOG2 9

#ifdef notdef
#define MACH_PARTITION_TYPE "Apple_Mach"
#else
#define MACH_PARTITION_TYPE "Mach_UNIX_BSD4.3"
#endif
#define PAGING_PARTITION_TYPE "Apple_Scratch"

#ifdef KERNEL

#define MACDISK_HASH_SIZE 8
#define MACDISK_HASH(n)	  ((n) & (MACDISK_HASH_SIZE - 1))

/* MACH device number encoding for a macintosh disk */
/* this supports 8 devices with 32 sub-devices */
typedef struct {
  unsigned disk:27,
           part:5;
} *macdisk_dev_t;

/* open disk partition information in the hash table */
typedef struct {
  queue_chain_t link;
  dev_mode_t mode;
  unsigned short number;
  unsigned int offset; /* in blocks */
  unsigned int length; /* in blocks */
  unsigned char name[32];
  unsigned char type[32];
} *macpart_t;

/* a disk device */
typedef struct macdisk {
  boolean_t busy;                   /* drive is currently being opened */
  boolean_t ready;                  /* drive is Apple format */
  unsigned int offset;              /* in blocks */
  unsigned int length;              /* in blocks */
  short nparts;                	    /* number of partitions from map */
  short oparts;                     /* number of open partitions */
  int mach_dev;                     /* macdisk number of Mach partition */
  queue_head_t macparts[MACDISK_HASH_SIZE];
} *macdisk_t;

extern struct macdisk macdisk[];

extern boolean_t macdisk_busy(register macdisk_t disk);
extern io_return_t macdisk_close(int number);
extern io_return_t macdisk_read(int number, register io_req_t ior);
extern io_return_t macdisk_write(int number, register io_req_t ior);
extern io_return_t macdisk_info(int number, int code, int *data);
#endif

#endif /* _MAC2DEV_MACDISK_H_ */
