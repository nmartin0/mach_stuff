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

/* Macintosh SCSI disk driver for MACH 3.0
 *
 * Authors: David E. Bohman II (CMU macmach)
 *          Zonnie L. Williamson (CMU macmach)
 *
 * The sdisk module reads and writes blocks in apple disk partitions.  A
 * special exception to this is partition #0 which allows access to the
 * entire apple disk, and thus to the partition map.  Utility software can
 * access the partition map via partition #0.
 *
 * This disk driver operates on top of the "mscsi" layer.  This module
 * worries about the Apple disk partition format.  The mscsi layer does
 * the actual SCSI I/O via calls the Macintosh SCSI Manager.
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
 * use the sdisk layer to directly access the CD format information.
 *
 * See Inside Macintosh for documentation on the Apple disk format.
 * For the High Sierra CD format, see:
 *  May 28, 1986 Working Paper for Information Processing,
 *  Volume and File Structure of CD-ROM for Information Interchange
 * For the ISO 9660 CD format, see:
 *  ISO 9660 Volume and File Structure of CD-ROM for Information Interchange
 */

#include <mach/mach_types.h>

#include <device/device_types.h>
#include <device/io_req.h>

#include <kern/queue.h>

#include <mac2os/Types.h>
#include <mac2os/SCSI.h>

#include <mac2dev/sdisk.h>
#include <mac2dev/mscsi.h>

#define HASH_SIZE 8
#define HASH(n)	  ((n) & (HASH_SIZE - 1))

/* open disk partition information in the hash table */
typedef struct {
  queue_chain_t link;
  dev_mode_t mode;
  unsigned short number;
  unsigned int offset; /* in blocks */
  unsigned int length; /* in blocks */
  unsigned char name[32];
  unsigned char type[32];
} *part_t;

/* a disk device */
typedef struct disk {
  boolean_t busy;                /* drive is currently being opened */
  boolean_t ready;               /* drive is Apple format */
  unsigned int offset;           /* in blocks */
  unsigned int length;           /* in blocks */
  short nparts;                	 /* number of partitions from map */
  short oparts;                	 /* number of open partitions */
  queue_head_t parts[HASH_SIZE]; /* hash table of partition information */
} *disk_t;

/* assume one 'disk' per SCSI controller */
static struct disk sdisk[SCSI_NCTLR];

/* MACH device number encoding for an sdisk */
typedef struct {
  unsigned disk:3,
               :24,
           part:5;
} *sdisk_t;

/* setup hash table during machine initialization */
void sdisk_initialize(void)
{
  register i, j;

  for (i = 0; i < SCSI_NCTLR; i++)
    for (j = 0; j < HASH_SIZE; j++)
      queue_init(&sdisk[i].parts[j]);
}

/* lookup open partition information in hash table */
/* return zero if partition not found */
static inline part_t part_lookup(register sdisk_t dev)
{
  register queue_t q = &sdisk[dev->disk].parts[HASH(dev->part)];
  register queue_entry_t qe = queue_first(q);

  if (dev->part > sdisk[dev->disk].nparts) return 0;
  if (queue_end(q, qe)) return 0;
  if (dev->part == ((part_t)qe)->number) return ((part_t)qe);
  for (qe = queue_next(qe); !queue_end(q, qe); qe = queue_next(qe))
    if (dev->part == ((part_t)qe)->number) return ((part_t)qe);
  return 0;
}

/* add open partition information to hash table */
static inline void part_add(register sdisk_t dev, register part_t part)
{
  register disk_t disk = &sdisk[dev->disk];

  queue_enter(&disk->parts[HASH(dev->part)], part, part_t, link);
  disk->oparts++;
}

/* remove open partition information from hash table */
static inline void part_remove(register sdisk_t dev, register part_t part)
{
  register disk_t disk = &sdisk[dev->disk];

  queue_remove(&disk->parts[HASH(dev->part)], part, part_t, link);
  disk->oparts--;
}

/* do SCSI disk ready test -- interface to mscsi module */
/* return non-zero if disk NOT ready */
static inline io_return_t disk_ready(register io_req_t ior)
{
  io_return_t result;

  ior->io_op = IO_OPEN | IO_WANTED;
  ior->io_mode = 0;
  ior->io_alloc_size = 0;
  result = scsi_op(ior);
  if (result == D_IO_QUEUED) result = scsi_wait(ior);
  return result;

} /* disk_ready() */

/* do SCSI disk read operation -- interface to mscsi module */
/* return non-zero if error */
static inline io_return_t disk_read(ior, recnum, count, buffer)
register io_req_t ior;
unsigned recnum;
unsigned count;
unsigned char buffer[];
{
  io_return_t result;

  ior->io_op = IO_READ | IO_WANTED;
  ior->io_mode = 0;
  ior->io_recnum = recnum;
  ior->io_data = (io_buf_ptr_t)(buffer);
  ior->io_count = count << SCSI_NBBLKLOG2;
  ior->io_alloc_size = 0;
  result = scsi_op(ior);
  if (result == D_IO_QUEUED) result = scsi_wait(ior);
  return result;

} /* disk_read() */

/* do SCSI disk write operation -- interface to mscsi module */
/* return non-zero if error */
static inline io_return_t disk_write(ior, recnum, count, buffer)
register io_req_t ior;
unsigned recnum;
unsigned count;
unsigned char buffer[];
{
  io_return_t result;

  ior->io_op = IO_WRITE | IO_WANTED;
  ior->io_mode = 0;
  ior->io_recnum = recnum;
  ior->io_data = (io_buf_ptr_t)(buffer);
  ior->io_count = count << SCSI_NBBLKLOG2;
  ior->io_alloc_size = 0;
  result = scsi_op(ior);
  if (result == D_IO_QUEUED) result = scsi_wait(ior);
  return result;

} /* disk_write() */

/* Block the thread if the disk is busy. */
/* Return whether we blocked or not. */
static inline boolean_t disk_busy(register disk_t disk)
{
  if (!disk->busy) {
    disk->busy = TRUE;
    return FALSE;
  }
  assert_wait(&disk->busy, FALSE);
  thread_block((void (*)()) 0);
  return TRUE;
}

/* open an sdisk device */
io_return_t sd_open(int number, dev_mode_t mode, io_req_t ior_notused)
{
  sdisk_t dev = (sdisk_t)&number;
  disk_t disk = &sdisk[dev->disk];
  part_t part;
  unsigned char block[SCSI_NBBLK];
  Block0 *b0 = (Block0 *)block;
  Partition *p = (Partition *)block;
  io_req_t ior;
  io_return_t result;

  /* disk # and SCSI controller # are synonomous */
  if (dev->disk >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;

  /* only let one thread complete an open per disk at a time */
  while (disk_busy(disk));

  /* if the specified partiton has been opened before, all done */
  if (part = part_lookup(dev)) {
    disk->busy = FALSE;
    if (part->mode != mode) return D_IO_ERROR;
    return D_SUCCESS;
  }

  /* otherwise, this partition is being opened for the first time */

  io_req_alloc(ior, 0);
  /* "goto done" will set disk->busy to FALSE and release the ior */
  ior->io_device = 0;
  ior->io_unit = dev->disk;

  /* if this disk is being opened for the first time... */
  if (!disk->ready) {

    disk->offset = 0;

    /* read block zero */
    if (disk_ready(ior) || disk_read(ior, disk->offset, 1, block)) {
      result = D_NO_SUCH_DEVICE;
      goto done;
    }

    /* if not an apple disk, maybe it is a CD... */
    if (b0->sbSig != sbSIGWord) {
      /* set disk->offset to start of Apple disk format */
      disk->offset = 0;
      /* read block 0 from a CD */
      if (disk_read(ior, disk->offset, 1, block)) {
        result = D_NO_SUCH_DEVICE;
        goto done;
      }
    }

    /* if mode not specified, give warning and continue */
    if (!mode) {
      printf("WARNING: sdisk %d:%d opened with mode == 0\n",
             dev->disk,
             dev->part);
      if (disk->offset) mode = D_READ;
      else mode = D_READ | D_WRITE;
    }

    /* assert that disk is read and/or write */
    if (!(mode & (D_READ | D_WRITE))) {
      result = D_IO_ERROR;
      goto done;
    }

    /* assert that a CD is read-only */
    if (disk->offset && (mode & D_WRITE)) {
      result = D_READ_ONLY;
      goto done;
    }

    /* assert that this is an Apple disk */
    if (b0->sbSig != sbSIGWord) {
      result = D_NO_SUCH_DEVICE;
      goto done;
    }

    /* note disk length, in blocks */
    disk->length = b0->sbBlkCount;

    /* assert that there is an Apple partition map */
    if (disk_read(ior, disk->offset + 1, 1, block) ||
        (p->pmSig != pMapSIG)) {
      result = D_NO_SUCH_DEVICE;
      goto done;
    }

    /* note partition map size */
    disk->nparts = p->pmMapBlkCnt;

    /* all done, this is a valid Apple disk */
    disk->ready = TRUE;

  } /* if (!disk->ready) */

  /* check that partition number is valid
  if (dev->part > disk->nparts) {
    result = D_NO_SUCH_DEVICE;
    goto done;
  }

  /* if partiton #0, fake a partition that refers to the entire disk */
  if (!dev->part) {
    if (!(part = (part_t)kalloc(sizeof (*part)))) {
      result = D_NO_MEMORY;
      goto done;
    }
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
    if (disk_read(ior, disk->offset + dev->part, 1, block) ||
        (p->pmSig != pMapSIG)) {
      result = D_NO_SUCH_DEVICE;
      goto done;
    }

    /* set up a copy of the partition map entry */
    if (!(part = (part_t)kalloc(sizeof (*part)))) {
      result = D_NO_MEMORY;
      goto done;
    }
    part->mode = mode;
    part->number = dev->part;
    part->offset = disk->offset + p->pmPyPartStart;
    part->length = p->pmPartBlkCnt;
    bcopy(p->pmPartName, part->name, sizeof (part->name));
    bcopy(p->pmPartType, part->type, sizeof (part->type));

  } /* not partition #0 */

  /* remember the partiton parameters for this device */
  part_add(dev, part);

  result = D_SUCCESS;

done:

  /* all done */
  io_req_free(ior);
  thread_wakeup_one(&disk->busy);
  disk->busy = FALSE;
  return result;

} /* sd_open() */

/* close an sdisk device */
io_return_t sd_close(int number)
{
  register sdisk_t dev = (sdisk_t)&number;
  register disk_t disk = &sdisk[dev->disk];
  register part_t	part;

  if (dev->disk >= SCSI_NCTLR) return D_NO_SUCH_DEVICE;
  if (!(part = part_lookup(dev))) return D_NO_SUCH_DEVICE;
  part_remove(dev, part);
  kfree(part, sizeof (*part));
  if (!disk->oparts) {
    disk->ready = FALSE;
    disk->nparts = 0;
  }
  else if (disk->oparts < 0) panic("sd close");
  return D_SUCCESS;

} /* sd_close() */

#define SCSI_MAXBLK (SCSI_MAXPHYS >> SCSI_NBBLKLOG2)

/* read from an sdisk device */
io_return_t sd_read(int number, register io_req_t ior)
{
  register sdisk_t dev = (sdisk_t)&number;
  register part_t part;
  register kern_return_t result;
  register unsigned offset, length;

  /* get the partition information for this device */
  if (!(part = part_lookup(dev))) return D_NO_SUCH_DEVICE;

  /* make sure mode is correct */
  if (!(part->mode & D_READ)) return D_IO_ERROR;

  /* The SCSI layer's notion of 'unit' is different that ours. */
  ior->io_unit = dev->disk;

  /* Check for bogus arguments. */
  if (ior->io_recnum >= part->length) return D_INVALID_RECNUM;
  if (ior->io_count % SD_REC_SIZE) return D_INVALID_SIZE;
  offset = ior->io_recnum;
  length = (ior->io_count >> SD_REC_SIZE_LOG2);

  /* Adjust the request to fit within the partition, if necessary. */
  if ((offset + length) > part->length) {
    length = part->length - offset;
    ior->io_count = (length << SD_REC_SIZE_LOG2);
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
  if (length > SCSI_MAXBLK) {
    register io_buf_ptr_t data = ior->io_data;
    register io_req_t r;

    io_req_alloc(r, 0);
    r->io_device = 0;
    r->io_unit = ior->io_unit;
    while (length > SCSI_MAXBLK) {
      if (disk_read(r, offset, SCSI_MAXBLK, data)) {
        ior->io_residual = (length << SD_REC_SIZE_LOG2);
        io_req_free(r);
        return D_SUCCESS;
      }
      length -= SCSI_MAXBLK;
      offset += SCSI_MAXBLK;
      data += SCSI_MAXPHYS;
    }
    if (length > 0) if (disk_read(r, offset, length, data)) {
      ior->io_residual = (length << SD_REC_SIZE_LOG2);
      io_req_free(r);
      return D_SUCCESS;
    }
    io_req_free(r);
  }
  else {
    /*
     * Requests which will fit into the SCSI buffer can fully exploit the
     * MACH asynchronous device features, including the stack handoff facility.
     */
    ior->io_recnum = offset;
    return (scsi_op(ior));
  }
  return D_SUCCESS;

} /* sd_read() */

/* write to an sdisk device */
io_return_t sd_write(int number, register io_req_t ior)
{
  register sdisk_t dev = (sdisk_t)&number;
  register part_t part;
  register kern_return_t result;
  register unsigned offset, length;
  boolean_t wait;

  /* get the partition information for this device */
  if (!(part = part_lookup(dev))) return D_NO_SUCH_DEVICE;

  /* make sure mode is correct */
  if (!(part->mode & D_WRITE)) return D_READ_ONLY;

  ior->io_unit = dev->disk;
  if (ior->io_recnum >= part->length) return D_INVALID_RECNUM;
  if (ior->io_count % SD_REC_SIZE) return D_INVALID_SIZE;
  offset = ior->io_recnum;
  if (!(ior->io_op & IO_INBAND)) {
    result = device_write_get(ior, &wait);
    if (result != KERN_SUCCESS) return result;
  }
  length = (ior->io_count >> SD_REC_SIZE_LOG2);
  if ((offset + length) > part->length) {
    length = part->length - offset;
    ior->io_count = (length << SD_REC_SIZE_LOG2);
  }
  offset += part->offset;
  if (length > SCSI_MAXBLK) {
    register io_buf_ptr_t data = ior->io_data;
    register io_req_t r;

    io_req_alloc(r, 0);
    r->io_device = 0;
    r->io_unit = ior->io_unit;
    while (length > SCSI_MAXBLK) {
      if (disk_write(r, offset, SCSI_MAXBLK, data)) {
        ior->io_residual = (length << SD_REC_SIZE_LOG2);
        io_req_free(r);
        return D_SUCCESS;
      }
      length -= SCSI_MAXBLK;
      offset += SCSI_MAXBLK;
      data += SCSI_MAXPHYS;
    }
    if (length > 0) if (disk_write(r, offset, length, data)) {
      ior->io_residual = (length << SD_REC_SIZE_LOG2);
      io_req_free(r);
      return D_SUCCESS;
    }
    io_req_free(r);
  }
  else {
    ior->io_recnum = offset;
    if (!wait) return scsi_op(ior);
    else {
      result = scsi_op(ior);
      if (result == D_IO_QUEUED) scsi_wait(ior);
      return D_SUCCESS;
    }
  }
  return D_SUCCESS;

} /* sd_write() */

/* get status for an sdisk device */
io_return_t sd_getstat(int number, int flavor, dev_status_t status, unsigned *count)
{
  register sdisk_t dev = (sdisk_t)&number;
  register part_t part;
  register sd_part_info_t *info;

  /* get the partition information for this device */
  if (!(part = part_lookup(dev))) return D_NO_SUCH_DEVICE;

  /* get the specified status */
  switch (flavor) {
    case SD_PART_INFO:
      info = (sd_part_info_t *)status;
      info->parts = sdisk[dev->disk].nparts;
      info->length = part->length;
      bcopy(part->name, info->name, sizeof (info->name));
      bcopy(part->type, info->type, sizeof (info->type));
      *count = SD_PART_INFO_COUNT;
      break;
    default:
      return D_INVALID_OPERATION;
  }

  /* all done */
  return D_SUCCESS;

} /* sd_getstat() */

/* get information for an sdisk device */
io_return_t sd_info(int number, int code, int *data)
{

  /* get the specified information */
  switch (code) {
    case D_INFO_BLOCK_SIZE:
      *data = SD_REC_SIZE;
      break;
    default:
      return D_IO_ERROR;
  }

  /* all done */
  return D_SUCCESS;

} /* sd_info() */
