/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/* Module for character and block I/O to disk devices with disklabel's.
 * This interfaces to generic Mach disk devices.
 *
 * By Zonnie L. Williamson, CMU MacMach 1992
 *
 * bdisk_init    -- initial setup for disk I/O module
 * bdisk_open    -- open a disk device, uses bdevsw parameters
 * bdisk_close   -- close a disk device
 * bdisk_ioctl   -- ioctl's for a disk device
 * bdisk_read    -- read from a disk device, uses bdevsw parameters
 * bdisk_write   -- write to a disk device, uses bdevsw parameters
 * bdisk_io      -- strategy routine for disk device block I/O
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/systm.h>
#include <sys/zalloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/disklabel.h>
#include <uxkern/import_mach.h>
#include <uxkern/device_reply_hdlr.h>
#include <uxkern/device_utils.h>
#undef KERNEL
#include <device/device.h>
#include <mach_init.h>

typedef struct bdisk {
  mach_port_t disk_port;      /* single port for all partitions */
  int flag;                   /* how the disk device was opened */
  int reference_count;        /* if non-zero, some partition is still open */
  int wlabel;                 /* if non-zero, allow LABELSECTOR write */
  struct disklabel disklabel; /* the disklabel */
} *bdisk_t;

static zone_t disk_zone;

#define	bdisk_hash_enter(dev, port) \
		dev_number_hash_enter(XDEV_BLOCK(dev), (char *)(port))
#define	bdisk_hash_remove(dev)	\
		dev_number_hash_remove(XDEV_BLOCK(dev))
#define	bdisk_hash_lookup(dev)	\
		((bdisk_t)dev_number_hash_lookup(XDEV_BLOCK(dev)))

/* initial setup for disk I/O module */
void bdisk_init()
{
  int i;

  /* must be wired because inode_pager can use these structures */
  disk_zone = zinit((vm_size_t)sizeof(struct bdisk),
                    (vm_size_t)sizeof(struct bdisk) * 4096,
	            vm_page_size,
	            FALSE,
	            "bdisk disklabel");
}

/* close a disk device */
int bdisk_close(dev_t dev, int flag)
{
  bdisk_t d;
  int error;

  if (!(d = bdisk_hash_lookup(dev))) panic("bdisk_close: no disk");
  if (d->disk_port == MACH_PORT_NULL) panic("bdisk_close: null port");
  if (--d->reference_count > 0) return 0;
  bdisk_hash_remove(dev);
  error = dev_error_to_errno(device_close(d->disk_port));
  (void)mach_port_deallocate(mach_task_self(), d->disk_port);
  d->disk_port = MACH_PORT_NULL;
  zfree(disk_zone, (vm_offset_t)d);
  return error;
}

/* open a disk device */
int bdisk_open(dev_t dev, int flag)
{
  bdisk_t d;
  char name[128];
  int error;
  kern_return_t rc;
  io_buf_ptr_t data;
  mach_msg_type_number_t count;

  if (d = bdisk_hash_lookup(dev)) {
    if (d->disk_port == MACH_PORT_NULL) panic("bdisk_open: null port");
    if (d->reference_count <= 0) panic("bdisk_open: bad reference count");
    if (!(d->flag & FWRITE) && (flag & FWRITE)) return EROFS;
    d->reference_count++;
    return 0;
  }
  if ((major(dev) < 0) || (major(dev) >= nblkdev)) return ENXIO;
  strcpy(name, bdevsw[major(dev)].d_name);
  itoa(minor(dev)/C_BLOCK_GET(bdevsw[major(dev)].d_flags), &name[strlen(name)]);
  d = (bdisk_t)zalloc(disk_zone);
  d->reference_count = 1;
  d->flag = FREAD | FWRITE;
  rc = device_open(device_server_port,
                   D_READ | D_WRITE,
                   name,
                   &d->disk_port);
  if (rc == D_READ_ONLY) {
    d->flag = FREAD;
    rc = device_open(device_server_port,
                     D_READ,
                     name,
                     &d->disk_port);
  }
  if (error = dev_error_to_errno(rc)) {
    zfree(disk_zone, (vm_offset_t)d);
    return error;
  }
  bdisk_hash_enter(dev, d);
  if (!(d->flag & FWRITE) && (flag & FWRITE)) {
    (void)bdisk_close(dev, 0);
    return EROFS;
  }
  rc = device_read(d->disk_port,
                   0,
                   LABELSECTOR,
                   DEV_BSIZE,
                   &data,
                   &count);
  if (error = dev_error_to_errno(rc)) {
    (void)bdisk_close(dev, 0);
    return error;
  }
  if (count < (LABELOFFSET + sizeof(struct disklabel))) {
    (void)bdisk_close(dev, 0);
    return ENXIO;
  }
  bcopy(&data[LABELOFFSET], &d->disklabel, sizeof(struct disklabel));
  (void)vm_deallocate(mach_task_self(), (vm_address_t)data, count);
  return 0;
}

/* check disklabel, return non-zero error code if error */
static int check_disklabel(struct disklabel *lp)
{
  u_short *start, *end, sum;

  /* verify magic numbers */
  if (lp->d_magic != DISKMAGIC) return EINVAL;
  if (lp->d_magic2 != DISKMAGIC) return EINVAL;

  /* verify checksum */
#ifdef notdef
  sum = 0;
  start = (u_short *)lp;
  end = (u_short *)&lp->d_partitions[lp->d_npartitions];
  while (start < end) sum ^= *start++;
  if (sum != 0) return EINVAL;
#endif

  return 0;

} /* check_disklabel() */

/* ioctl's for a disk device */
int bdisk_ioctl(dev_t dev, int cmd, caddr_t data, int flag)
{
  bdisk_t d;
  struct disklabel *dl;
  int error;
  kern_return_t rc;
  int count;
  io_buf_ptr_t tmp_data;
  mach_msg_type_number_t tmp_count;

  if (!(d = bdisk_hash_lookup(dev))) panic("bdisk_ioctl: no disk");
  if (d->disk_port == MACH_PORT_NULL) panic("bdisk_ioctl: null port");
  dl = &d->disklabel;
  switch (cmd) {
    case DIOCGDINFO:
      bcopy(dl, data, sizeof(struct disklabel));
      break;
    case DIOCSDINFO:
      if (!(flag & FWRITE)) return EBADF;
      if (error = check_disklabel((struct disklabel *)data)) return error;
      bcopy(data, dl, sizeof(*dl));
      break;
    case DIOCWLABEL:
      if (!(flag & FWRITE)) return EBADF;
      d->wlabel = *(int *)data;
      break;
    case DIOCWDINFO:
      if (!(flag & FWRITE)) return EBADF;
      if (error = check_disklabel((struct disklabel *)data)) return error;
      rc = device_read(d->disk_port,
                       0,
                       LABELSECTOR,
                       DEV_BSIZE,
                       &tmp_data,
                       &tmp_count);
      if (error = dev_error_to_errno(rc)) return error;
      if (tmp_count < (LABELOFFSET + sizeof(struct disklabel))) {
        (void)vm_deallocate(mach_task_self(), (vm_address_t)tmp_data, tmp_count);
        return ENXIO;
      }
      bcopy(data, &tmp_data[LABELOFFSET], sizeof(struct disklabel));
      rc = device_write(d->disk_port,
                        0,
                        LABELSECTOR,
                        tmp_data,
                        tmp_count,
                        &count);
      (void)vm_deallocate(mach_task_self(), (vm_address_t)tmp_data, tmp_count);
      if (error = dev_error_to_errno(rc)) return error;
      if (count < (LABELOFFSET + sizeof(struct disklabel))) return ENXIO;
      bcopy(data, dl, sizeof(struct disklabel));
      break;
    default:
      return ENOTTY;
      break;
  }
  return 0;
}

/* read from a disk device */
int bdisk_read(dev_t dev, struct uio *uio)
{
  bdisk_t d;
  struct disklabel *dl;
  struct partition *p;
  struct iovec *iov;
  int error;
  kern_return_t rc;
  unsigned int count;
  recnum_t recnum;
  io_buf_ptr_t data;

  if (!(d = bdisk_hash_lookup(dev))) panic("bdisk_read: no disk");
  if (d->disk_port == MACH_PORT_NULL) panic("bdisk_read: null port");
  dl = &d->disklabel;
  p = &dl->d_partitions[minor(dev) % C_BLOCK_GET(bdevsw[major(dev)].d_flags)];
  if ((dl->d_magic != DISKMAGIC) || (dl->d_magic2 != DISKMAGIC)) return ENXIO;
  while (uio->uio_iovcnt > 0) {
    iov = uio->uio_iov;
    if (iov->iov_len == 0) {
      uio->uio_iovcnt--;
      uio->uio_iov++;
      continue;
    }
    if (useracc(iov->iov_base, (u_int)iov->iov_len, 0) == 0) return EFAULT;
    if ((uio->uio_offset + iov->iov_len) > (p->p_size * dl->d_secsize))
      return EINVAL;
    recnum = (uio->uio_offset / dl->d_secsize) + p->p_offset;
    rc = device_read(d->disk_port,
		     0,
		     recnum,
		     iov->iov_len,
		     &data,
		     &count);
    if (rc != D_SUCCESS) return dev_error_to_errno(rc);
    /* moveout() deallocates data buffer allocated by device_read() */
    (void)moveout(data, iov->iov_base, count);
    iov->iov_base += count;
    iov->iov_len -= count;
    uio->uio_resid -= count;
    uio->uio_offset += count;
  }
  return 0;
}

/* write to a disk device */
int bdisk_write(dev_t dev, struct uio *uio)
{
  bdisk_t d;
  struct disklabel *dl;
  struct partition *p;
  struct iovec *iov;
  kern_return_t rc;
  vm_offset_t kern_addr;
  vm_size_t kern_size;
  vm_size_t count;
  recnum_t recnum;

  if (!(d = bdisk_hash_lookup(dev))) panic("bdisk_write: no disk");
  if (d->disk_port == MACH_PORT_NULL) panic("bdisk_write: null port");
  dl = &d->disklabel;
  p = &dl->d_partitions[minor(dev) % C_BLOCK_GET(bdevsw[major(dev)].d_flags)];
  if ((dl->d_magic != DISKMAGIC) || (dl->d_magic2 != DISKMAGIC)) return ENXIO;
  while (uio->uio_iovcnt > 0) {
    iov = uio->uio_iov;
    if (iov->iov_len == 0) {
      uio->uio_iovcnt--;
      uio->uio_iov++;
      continue;
    }
    if ((uio->uio_offset + iov->iov_len) > (p->p_size * dl->d_secsize))
      return EINVAL;
    recnum = (uio->uio_offset / dl->d_secsize) + p->p_offset;
    if ((recnum <= LABELSECTOR) &&
        ((recnum + iov->iov_len / dl->d_secsize) >= LABELSECTOR) &&
        !d->wlabel)
      return EROFS;
    kern_size = iov->iov_len;
    (void)vm_allocate(mach_task_self(), &kern_addr, kern_size, TRUE);
    if (copyin(iov->iov_base, kern_addr, (u_int)iov->iov_len)) {
      (void)vm_deallocate(mach_task_self(), (vm_address_t)kern_addr, kern_size);
      return EFAULT;
    }
    rc = device_write(d->disk_port,
		      0,
		      recnum,
		      (io_buf_ptr_t)kern_addr,
		      iov->iov_len,
		      &count);
    (void)vm_deallocate(mach_task_self(), (vm_address_t)kern_addr, kern_size);
    if (rc != D_SUCCESS) return dev_error_to_errno(rc);
    iov->iov_base += count;
    iov->iov_len -= count;
    uio->uio_resid -= count;
    uio->uio_offset += count;
  }
  return 0;
}

/* strategy routine for disk device block I/O */
void bdisk_io(struct buf *bp)
{
  bdisk_t d;
  struct disklabel *dl;
  struct partition *p;
  kern_return_t rc;
  vm_size_t count;
  recnum_t recnum;

  if (!(d = bdisk_hash_lookup(bp->b_dev))) panic("bdisk_io: no disk");
  if (d->disk_port == MACH_PORT_NULL) panic("bdisk_io: null port");
  if (!(d->flag & FWRITE) && (!(bp->b_flags & B_READ))) {
    bp->b_flags |= B_ERROR;
    bp->b_error = EROFS;
    biodone(bp);
    return;
  }
  dl = &d->disklabel;
  p = &dl->d_partitions[minor(bp->b_dev) %
                        C_BLOCK_GET(bdevsw[major(bp->b_dev)].d_flags)];
  if ((dl->d_magic != DISKMAGIC) || (dl->d_magic2 != DISKMAGIC)) {
    bp->b_flags |= B_ERROR;
    bp->b_error = ENXIO;
    biodone(bp);
    return;
  }
  if (((bp->b_blkno * dl->d_secsize) + bp->b_bcount) >
      (p->p_size * dl->d_secsize)) {
    bp->b_flags |= B_ERROR;
    bp->b_error = EINVAL;
    biodone(bp);
    return;
  }
  recnum = bp->b_blkno + p->p_offset;
  count = bp->b_bcount;
  if (!(bp->b_flags & B_READ) && (recnum <= LABELSECTOR) &&
      ((recnum + count / dl->d_secsize) >= LABELSECTOR) &&
      !d->wlabel) {
    bp->b_flags |= B_ERROR;
    bp->b_error = EROFS;
    biodone(bp);
    return;
  }
  if (bp->b_flags & B_READ) {
    rc = device_read_request(d->disk_port,
                             bp->b_reply_port,
                             0,
                             recnum,
                             count);
    if (rc != D_SUCCESS) panic("bdisk_io: read request ", rc);
  }
  else {
    rc = device_write_request(d->disk_port,
                              bp->b_reply_port,
                              0,
                              recnum,
                              bp->b_un.b_addr,
                              count);
    if (rc != D_SUCCESS) panic("bdisk_io: write request ", rc);
  }
}
