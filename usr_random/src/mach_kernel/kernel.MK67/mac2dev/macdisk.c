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
 *
 * This module reads and writes blocks in Apple disk partitions.  A
 * special exception to this is partition #0 which allows access to the
 * entire Apple disk, and thus to the partition map.  Utility software can
 * access the partition map via partition #0.
 *
 * This driver operates on top of the "scsi" layer.  This module handles
 * the Apple disk partition format.  The scsi layer does the actual I/O
 * via calls the Macintosh SCSI Manager.
 *
 * The Apple disk format looks like:
 *   <block zero><partition map><data>
 * The block zero contains a magic number and total disk size information.
 * The partition map contains sizes and offsets into the data for one or
 * more partitions.
 *
 * This module also interprets CD's in the High Sierra or ISO 9660 format.
 * A CD disk looks like:
 *   <format information> .... <apple disk>
 * Access to a CD is transparent.  There is no way for other software to
 * use the disk layer to directly access the CD format information.
 *
 * See Inside Macintosh for documentation on the Apple disk format.
 * For the High Sierra CD format, see:
 *  May 28, 1986 Working Paper for Information Processing,
 *  Volume and File Structure of CD-ROM for Information Interchange
 * For the ISO 9660 CD format, see:
 *  ISO 9660 Volume and File Structure of CD-ROM for Information Interchange
 *
 * This module also provides the "standard" Mach disk interface implemented
 * as a Mach partition on the Apple disk.
 */

#include <mach/mach_types.h>
#include <device/device_types.h>
#include <device/io_req.h>
#include <kern/queue.h>
#include <mac2dev/scsi.h>
#include <mac2dev/macdisk.h>

/* assume one 'disk' per SCSI controller */
struct macdisk macdisk[SCSI_NCTLR];

/* setup hash table during machine initialization */
void macdisk_initialize(void)
{
  register i, j;

  for (i = 0; i < SCSI_NCTLR; i++) {
    macdisk[i].mach_dev = -1;
    for (j = 0; j < MACDISK_HASH_SIZE; j++)
      queue_init(&macdisk[i].macparts[j]);
  }
}

/* lookup open partition information in the hash table */
/* return zero if the partition can not be found */
static macpart_t macpart_lookup(register macdisk_dev_t dev)
{
  register queue_t q = &macdisk[dev->disk].macparts[MACDISK_HASH(dev->part)];
  register queue_entry_t qe = queue_first(q);

  if (dev->part > macdisk[dev->disk].nparts) return 0;
  if (queue_end(q, qe)) return 0;
  if (dev->part == ((macpart_t)qe)->number) return ((macpart_t)qe);
  for (qe = queue_next(qe); !queue_end(q, qe); qe = queue_next(qe))
    if (dev->part == ((macpart_t)qe)->number) return ((macpart_t)qe);
  return 0;
}

/* add open partition information to the hash table */
static void macpart_add(register macdisk_dev_t dev,
                        register macpart_t part)
{
  register macdisk_t disk = &macdisk[dev->disk];

  queue_enter(&disk->macparts[MACDISK_HASH(dev->part)], part, macpart_t, link);
  disk->oparts++;
}

/* remove open partition information from the hash table */
static void macpart_remove(register macdisk_dev_t dev,
                           register macpart_t part)
{
  register macdisk_t disk = &macdisk[dev->disk];

  queue_remove(&disk->macparts[MACDISK_HASH(dev->part)], part, macpart_t, link);
  disk->oparts--;
}

/* Block the thread if the disk is busy. */
/* Return whether we blocked or not. */
boolean_t macdisk_busy(register macdisk_t disk)
{
  if (!disk->busy) {
    disk->busy = TRUE;
    return FALSE;
  }
  assert_wait(&disk->busy, FALSE);
  thread_block((void (*)())0);
  return TRUE;
}

/* make sure that the macintosh disk is ready for access */
io_return_t prepare_disk(macdisk_dev_t dev, io_req_t ior, dev_mode_t *mode)
{
  macdisk_t disk = &macdisk[dev->disk];
  unsigned char block[SCSI_NBBLK];
  Block0 *b0 = (Block0 *)block;
  Partition *p = (Partition *)block;

  if (!disk->ready) {

    disk->offset = 0;

    /* read block zero */
    if (scsi_disk_ready(ior) || scsi_disk_read(ior, disk->offset, 1, block))
      return D_NO_SUCH_DEVICE;

    /* if not an apple disk, maybe it is a CD... */
    if (b0->sbSig != sbSIGWord) {
      /* set disk->offset to start of Apple disk format */
      disk->offset = 0;
      /* read block 0 from a CD */
      if (scsi_disk_read(ior, disk->offset, 1, block)) return D_NO_SUCH_DEVICE;
    }

    /* if mode not specified, give warning and continue */
    if (!*mode) {
      printf("WARNING: disk %d:%d opened with mode == 0\n",
             dev->disk,
             dev->part);
      if (disk->offset) *mode = D_READ;
      else *mode = D_READ | D_WRITE;
    }

    /* assert that disk is read and/or write */
    if (!(*mode & (D_READ | D_WRITE))) return D_IO_ERROR;

    /* assert that a CD is read-only */
    if (disk->offset && (*mode & D_WRITE)) return D_READ_ONLY;

    /* assert that this is an Apple disk */
    if (b0->sbSig != sbSIGWord) return D_NO_SUCH_DEVICE;

    /* note disk length, in blocks */
    disk->length = b0->sbBlkCount;

    /* assert that there is an Apple partition map */
    if (scsi_disk_read(ior, disk->offset + 1, 1, block) ||
        (p->pmSig != pMapSIG)) {
      return D_NO_SUCH_DEVICE;
    }

    /* note partition map size */
    disk->nparts = p->pmMapBlkCnt;

    /* all done, this is a valid Apple disk */
    disk->ready = TRUE;

  } /* if (!disk->ready) */

  return D_SUCCESS;

} /* prepare_disk() */

io_return_t prepare_part(macdisk_dev_t dev,
                         io_req_t ior,
                         char *type,
                         dev_mode_t mode)
{
  macdisk_t disk = &macdisk[dev->disk];
  macpart_t part;
  unsigned char block[SCSI_NBBLK];
  Partition *p = (Partition *)block;

  /* fail if a partition of the specified type is already being accessed */
  if (type &&
      (part = macpart_lookup(dev)) &&
      !strncmp(type, (char *)part->type, sizeof(part->type))) {
    return D_NO_SUCH_DEVICE;
  }

  /* fail if this is the Mach partition and it is already being accessed */
  if (*((int *)dev) == disk->mach_dev) return D_NO_SUCH_DEVICE;

  /* if partiton #0, fake a partition that refers to the entire disk */
  if (!dev->part) {
    if (type) return D_NO_SUCH_DEVICE;
    if (!(part = (macpart_t)kalloc(sizeof(*part)))) return D_NO_MEMORY;
    part->mode = mode;
    part->number = 0;
    part->offset = disk->offset;
    part->length = disk->length;
    part->type[0] = part->name[0] = 0;
  }

  /* if not partition #0, access a 'real' partition */
  /* note that part==1 is the first 'real' partition */
  else {

    /* read partiton map entry */
    if (scsi_disk_read(ior, disk->offset + dev->part, 1, block) ||
        (p->pmSig != pMapSIG))
      return D_NO_SUCH_DEVICE;

    /* if part_type specified, then it must match */
    if (type && strncmp(type, (char *)p->pmPartType, sizeof(p->pmPartType)))
      return D_NO_SUCH_DEVICE;

    /* check read/write status */
    if ((mode & D_WRITE) && !(p->pmPartStatus & 0x20)) return D_READ_ONLY;

    /* set up a copy of the partition map entry */
    if (!(part = (macpart_t)kalloc(sizeof(*part)))) return D_NO_MEMORY;

    part->mode = mode;
    part->number = dev->part;
    part->offset = disk->offset + p->pmPyPartStart;
    part->length = p->pmPartBlkCnt;
    bcopy(p->pmPartName, part->name, sizeof(part->name));
    bcopy(p->pmPartType, part->type, sizeof(part->type));

  } /* not partition #0 */

  /* remember the partiton parameters for this device */
  macpart_add(dev, part);

  return D_SUCCESS;

} /* prepare_part() */

/* open a macintosh disk, sub device is apple partition */
io_return_t macdisk_open(int number, dev_mode_t mode, io_req_t ior_notused)
{
  macdisk_dev_t dev = (macdisk_dev_t)&number;
  macdisk_t disk;
  macpart_t part;
  unsigned char block[SCSI_NBBLK];
  Partition *p = (Partition *)block;
  io_req_t ior;
  io_return_t result;

  /* disk # and SCSI controller # are synonomous */
  if (dev->disk >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  else disk = &macdisk[dev->disk];

  /* if the specified partiton has been opened before, all done */
  if (part = macpart_lookup(dev)) {
    disk->busy = FALSE;
    if (part->mode != mode) return D_IO_ERROR;
    return D_SUCCESS;
  }
  /* otherwise, this partition is being opened for the first time */

  /* only let one thread complete an open per disk at a time */
  while (macdisk_busy(disk));

  io_req_alloc(ior, 0);

  /* "goto done" will set disk->busy to FALSE and release the ior */

  ior->io_device = 0;
  ior->io_unit = dev->disk;

  /* make sure that the macintosh disk is ready */
  if ((result = prepare_disk(dev, ior, &mode)) != D_SUCCESS) goto done;

  /* check that partition number is valid
  if (dev->part > disk->nparts) {
    result = D_NO_SUCH_DEVICE;
    goto done;
  }

  /* prepare the partition */
  result = prepare_part(dev, ior, 0, mode);

done:

  /* all done */
  io_req_free(ior);
  thread_wakeup_one(&disk->busy);
  disk->busy = FALSE;
  return result;

} /* macdisk_open() */

/* close a macintosh disk */
io_return_t macdisk_close(int number)
{
  register macdisk_dev_t dev = (macdisk_dev_t)&number;
  register macdisk_t disk = &macdisk[dev->disk];
  register macpart_t part;

  if (dev->disk >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  if (!(part = macpart_lookup(dev))) return D_NO_SUCH_DEVICE;
  macpart_remove(dev, part);
  kfree(part, sizeof(*part));
  if (!disk->oparts) {
    disk->ready = FALSE;
    disk->nparts = 0;
  }
  else if (disk->oparts < 0) panic("sd close");
  return D_SUCCESS;

} /* macdisk_close() */

/* read from a macintosh disk */
io_return_t macdisk_read(int number, register io_req_t ior)
{
  register macdisk_dev_t dev = (macdisk_dev_t)&number;
  register macpart_t part;
  register kern_return_t result;
  register unsigned int offset, length;

  /* get the partition information for this device */
  if (!(part = macpart_lookup(dev))) return D_NO_SUCH_DEVICE;

  /* make sure mode is correct */
  if (!(part->mode & D_READ)) return D_IO_ERROR;

  /* The SCSI layer's notion of 'unit' is different that ours. */
  ior->io_unit = dev->disk;

  /* Check for bogus arguments. */
  if (ior->io_recnum >= part->length) return D_INVALID_RECNUM;
  if (ior->io_count % MACDISK_REC_SIZE) return D_INVALID_SIZE;
  offset = ior->io_recnum;
  length = (ior->io_count >> MACDISK_REC_SIZE_LOG2);

  /* Adjust the request to fit within the partition, if necessary. */
  if ((offset + length) > part->length) {
    length = part->length - offset;
    ior->io_count = (length << MACDISK_REC_SIZE_LOG2);
  }

  /* Allocate space for the request. */
  result = device_read_alloc(ior, (vm_size_t)ior->io_count);
  if (result != KERN_SUCCESS) return result;

  /* Change offset into an absolute quantity. */
  offset += part->offset;

  /*
   * In 24-bit mode, the SCSI layer uses a fixed-sized intermediate buffer
   * for the data so that the kernel will run on a machine that is not 32
   * bit clean.  We allow arbitrarily large requests to be handled here so
   * that large disk transfers can be handled with a minimum of overhead
   * write RPC calls and vm region creation.  Please note that a thread
   * which is blocked at this time will not allow a stack handoff to occur.
   */
  if (length <= SCSI_MAXBLK) {

    /*
     * Requests which will fit into the SCSI buffer can fully exploit the
     * MACH asynchronous device features, including the stack handoff facility.
     */
    ior->io_recnum = offset;
    return (scsi_op(ior));

  }
  else {
    register io_buf_ptr_t data = ior->io_data;
    register io_req_t r;

    io_req_alloc(r, 0);
    r->io_device = 0;
    r->io_unit = ior->io_unit;
    while (length > SCSI_MAXBLK) {
      if (scsi_disk_read(r, offset, SCSI_MAXBLK, (unsigned char *)data)) {
        ior->io_residual = (length << MACDISK_REC_SIZE_LOG2);
        io_req_free(r);
        return D_SUCCESS;
      }
      length -= SCSI_MAXBLK;
      offset += SCSI_MAXBLK;
      data += SCSI_MAXPHYS;
    }
    if (length > 0)
      if (scsi_disk_read(r, offset, length, (unsigned char *)data)) {
        ior->io_residual = (length << MACDISK_REC_SIZE_LOG2);
        io_req_free(r);
        return D_SUCCESS;
      }
    io_req_free(r);

  }

  return D_SUCCESS;

} /* macdisk_read() */

/* write to a macintosh disk */
io_return_t macdisk_write(int number, register io_req_t ior)
{
  register macdisk_dev_t dev = (macdisk_dev_t)&number;
  register macpart_t part;
  register kern_return_t result;
  register unsigned offset, length;
  boolean_t wait;

  /* get the partition information for this device */
  if (!(part = macpart_lookup(dev))) return D_NO_SUCH_DEVICE;

  /* make sure mode is correct */
  if (!(part->mode & D_WRITE)) return D_READ_ONLY;

  ior->io_unit = dev->disk;
  if (ior->io_recnum >= part->length) return D_INVALID_RECNUM;
  if (ior->io_count % MACDISK_REC_SIZE) return D_INVALID_SIZE;
  offset = ior->io_recnum;
  if (!(ior->io_op & IO_INBAND)) {
    result = device_write_get(ior, &wait);
    if (result != KERN_SUCCESS) return result;
  }
  length = (ior->io_count >> MACDISK_REC_SIZE_LOG2);
  if ((offset + length) > part->length) {
    length = part->length - offset;
    ior->io_count = (length << MACDISK_REC_SIZE_LOG2);
  }
  offset += part->offset;
  if (length <= SCSI_MAXBLK) {

    ior->io_recnum = offset;
    if (!wait) return scsi_op(ior);
    else {
      result = scsi_op(ior);
      if (result == D_IO_QUEUED) scsi_wait(ior);
      return D_SUCCESS;
    }

  }
  else {
    register io_buf_ptr_t data = ior->io_data;
    register io_req_t r;

    io_req_alloc(r, 0);
    r->io_device = 0;
    r->io_unit = ior->io_unit;
    while (length > SCSI_MAXBLK) {
      if (scsi_disk_write(r, offset, SCSI_MAXBLK, (unsigned char *)data)) {
        ior->io_residual = (length << MACDISK_REC_SIZE_LOG2);
        io_req_free(r);
        return D_SUCCESS;
      }
      length -= SCSI_MAXBLK;
      offset += SCSI_MAXBLK;
      data += SCSI_MAXPHYS;
    }
    if (length > 0)
      if (scsi_disk_write(r, offset, length, (unsigned char *)data)) {
        ior->io_residual = (length << MACDISK_REC_SIZE_LOG2);
        io_req_free(r);
        return D_SUCCESS;
      }
    io_req_free(r);

  }

  return D_SUCCESS;

} /* macdisk_write() */

/* get status for a macintosh disk */
io_return_t macdisk_getstat(int number,
                            int flavor,
                            dev_status_t status,
                            unsigned *count)
{
  register macdisk_dev_t dev = (macdisk_dev_t)&number;
  register macpart_t part;
  register macdisk_part_info_t *info;

  /* get the partition information for this device */
  if (!(part = macpart_lookup(dev))) return D_NO_SUCH_DEVICE;

  /* get the specified status */
  switch (flavor) {
    case MACDISK_PART_INFO:
      info = (macdisk_part_info_t *)status;
      info->parts = macdisk[dev->disk].nparts;
      info->length = part->length;
      bcopy(part->name, info->name, sizeof(info->name));
      bcopy(part->type, info->type, sizeof(info->type));
      *count = MACDISK_PART_INFO_COUNT;
      break;
    default:
      return D_INVALID_OPERATION;
  }

  /* all done */
  return D_SUCCESS;

} /* macdisk_getstat() */

/* get information for a macintosh disk */
io_return_t macdisk_info(int number, int code, int *data)
{

  /* get the specified information */
  switch (code) {
    case D_INFO_BLOCK_SIZE:
      *data = MACDISK_REC_SIZE;
      break;
    default:
      return D_IO_ERROR;
  }

  /* all done */
  return D_SUCCESS;

} /* macdisk_info() */
