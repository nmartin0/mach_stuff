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
 * Author: Zonnie L. Williamson (CMU macmach)
 *
 * This module provides a "standard" mach disk implemented as a
 * partition in an Apple disk.  This module hooks directly into
 * the macdisk module.
 */

#include <mach/mach_types.h>
#include <device/device_types.h>
#include <device/io_req.h>
#include <kern/queue.h>
#include <mac2dev/scsi.h>
#include <mac2dev/macdisk.h>

/* The following is an implementation of a "standard" Mach disk using
 * an Apple partition of type MACH_PARTITION_TYPE.
 */

/* open the Mach partition on a macintosh disk, no sub device */
io_return_t disk_open(int number, dev_mode_t mode, io_req_t ior_notused)
{
  int _dev;
  macdisk_dev_t dev = (macdisk_dev_t)&_dev;
  macdisk_t disk;
  unsigned char block[SCSI_NBBLK];
  Partition *p = (Partition *)block;
  io_req_t ior;
  io_return_t result;

  dev->disk = number;

  /* disk # and SCSI controller # are synonomous */
  if (number >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  else disk = &macdisk[number];

  /* if the Mach partition has been opened before, all done */
  if (disk->mach_dev != -1) return D_SUCCESS;
  /* otherwise, this partition is being opened for the first time */

  /* only let one thread complete an open per disk at a time */
  while (macdisk_busy(disk));

  io_req_alloc(ior, 0);

  /* "goto done" will set disk->busy to FALSE and release the ior */

  ior->io_device = 0;
  ior->io_unit = number;

  /* make sure that the macintosh disk is ready */
  if ((result = prepare_disk(dev, ior, &mode)) != D_SUCCESS) goto done;

  /* search for Mach partition on macintosh disk */
  for (dev->part = 1; dev->part <= disk->nparts; dev->part++) {
    result = prepare_part(dev, ior, MACH_PARTITION_TYPE, mode);
    if (result == D_SUCCESS) {
      disk->mach_dev = *((int *)dev);
      goto done;
    }
  }

done:
  io_req_free(ior);
  thread_wakeup_one(&disk->busy);
  disk->busy = FALSE;
  return result;

} /* disk_open() */

/* close the Mach partition on a macintosh disk */
io_return_t disk_close(int number)
{
  macdisk_t disk;
  io_return_t result;

  /* disk # and SCSI controller # are synonomous */
  if (number >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  else disk = &macdisk[number];

  /* close the Mach partition */
  result = macdisk_close(disk->mach_dev);
  disk->mach_dev = -1;

  /* all done */
  return result;

} /* disk_close() */

/* read from the Mach partition on a macintosh disk */
io_return_t disk_read(int number, register io_req_t ior)
{
  macdisk_t disk;

  /* disk # and SCSI controller # are synonomous */
  if (number >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  else disk = &macdisk[number];

  /* read from the Mach partition */
  return macdisk_read(disk->mach_dev, ior);

} /* disk_read() */

/* write to a the Mach partition on a macintosh disk */
io_return_t disk_write(int number, register io_req_t ior)
{
  macdisk_t disk;

  /* disk # and SCSI controller # are synonomous */
  if (number >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  else disk = &macdisk[number];

  /* write to the Mach partition */
  return macdisk_write(disk->mach_dev, ior);

} /* disk_write() */

/* get information for the Mach partition on a macintosh disk */
io_return_t disk_info(int number, int code, int *data)
{
  macdisk_t disk;

  /* disk # and SCSI controller # are synonomous */
  if (number >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  else disk = &macdisk[number];

  /* get info for the Mach partition */
  return macdisk_info(disk->mach_dev, code, data);

} /* disk_info() */

