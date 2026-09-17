/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	scsi.h,v $
 * Revision 2.2  91/09/12  16:47:19  bohman
 * 	Created.
 * 	[91/09/11  16:00:42  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2dev/scsi.h
 *	Author: David E. Bohman II (CMU macmach)
 */

#ifndef _MAC2DEV_SCSI_H_
#define _MAC2DEV_SCSI_H_

#include <mac2os/Types.h>
#include <mac2os/SCSI.h>

#define SCSI_NBBLK	512
#define SCSI_NBBLKLOG2	9

#define SCSI_NCTLR	7

#define SCSI_MAXPHYS	(32 * 1024)

#define SCSI_MAXBLK (SCSI_MAXPHYS >> SCSI_NBBLKLOG2)

#ifdef KERNEL

io_return_t scsi_disk_ready(io_req_t ior);

io_return_t scsi_disk_read(io_req_t ior,
                           unsigned int recnum,
                           unsigned int count,
                           unsigned char *buffer);

io_return_t scsi_disk_write(io_req_t ior,
                            unsigned int recnum,
                            unsigned int count,
                            unsigned char *buffer);

#endif /* KERNEL */

#endif /* _MAC2DEV_SCSI_H_ */
