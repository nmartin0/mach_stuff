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
 * $Log:	sony.c,v $
 * Revision 2.2  91/09/12  16:47:49  bohman
 * 	Created.
 * 	[91/09/11  16:04:36  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2dev/sony.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mach/mach_types.h>

#include <device/device_types.h>
#include <device/io_req.h>

#include <kern/queue.h>

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Files.h>
#include <mac2os/Disks.h>

#include <mac2dev/sony.h>

/*
 * Macintosh floppy driver
 * for MACH 3.0
 */

#define SONY_BLOCK_SIZE 512

#define SONY_DRIVE_LOW	1
#define SONY_DRIVE_HIGH	3

#define SONY_RefNum	-5

typedef struct sony {
    queue_head_t	pending;
    IOParam		*pb;
    Ptr			buffer;
    int			ref_count;
} *sony_t;

static struct sony	sony_drive;

typedef struct {
    unsigned	     :30,
      		drive:2;
} *sony_id_t;

static io_return_t sonyerror(register OSErr err)
{
  switch (err) {
    case noErr:
      return D_SUCCESS;
    case wPrErr:
      return D_INVALID_OPERATION;
    case offLinErr:
      return D_DEVICE_DOWN;
    case eofErr:
      return D_INVALID_RECNUM;
    case memFullErr:
      return D_NO_MEMORY;
    default:
      return D_IO_ERROR;
  }
}

static OSErr sonyeject(short drive)
{
  register CntrlParam *pb;
  register OSErr err;

  pb = (CntrlParam *)NewPtr(sizeof (CntrlParam));
  if (!pb) return memFullErr;
  pb->ioVRefNum = drive;
  pb->ioCRefNum = SONY_RefNum;
  pb->csCode = 7;
  err = thread_io(PBControl, pb);
  DisposPtr(pb);
  return err;
}

static OSErr sonyformat(short drive, int mode)
{
  register CntrlParam *pb;
  register OSErr err;

  pb = (CntrlParam *)NewPtr(sizeof (CntrlParam));
  if (!pb) return memFullErr;
  pb->ioVRefNum = drive;
  pb->ioCRefNum = SONY_RefNum;
  pb->csCode = 6;
  pb->csParam[0] = 0;
  err = thread_io(PBControl, pb);
  DisposPtr(pb);
  return err;
}

static OSErr sonystatus(short drive, DrvSts *data)
{
  register CntrlParam *pb;
  register OSErr err;

  pb = (CntrlParam *)NewPtr(sizeof (CntrlParam));
  if (!pb) return memFullErr;
  pb->ioVRefNum = drive;
  pb->ioCRefNum = SONY_RefNum;
  pb->csCode = 8;
  err = thread_io(PBStatus, pb);
  if (err == noErr) bcopy(&pb->csParam[0], data, sizeof (DrvSts));
  DisposPtr(pb);
  return err;
}

#ifndef MODE24
static
#endif /* MODE24 */
void sony_complete(void)
{
  register sony_t sony = &sony_drive;
  register io_req_t ior;

  ior = (io_req_t)dequeue_head(&sony->pending);
  if (ior) (void)iodone(ior);
}

static void sony_begin(register io_req_t ior)
{
  register sony_id_t dev = (sony_id_t)&ior->io_unit;
  register sony_t sony = &sony_drive;
  register IOParam *pb;
#ifdef MODE24
  extern void sony_done();
#endif /* MODE24 */

  pb = sony->pb;
#ifdef MODE24
  pb->ioCompletion = sony_done;
#else /* MODE24 */
  pb->ioCompletion = sony_complete;
#endif /* MODE24 */
  pb->ioVRefNum = dev->drive;
  pb->ioRefNum = SONY_RefNum;
  pb->ioBuffer = sony->buffer;
  pb->ioReqCount = ior->io_count;
  pb->ioActCount = 0;
  pb->ioPosMode = fsFromStart;
  pb->ioPosOffset = (ior->io_recnum << 9);
  if (ior->io_op & IO_READ) (void)PBRead(pb, TRUE);
  else {
    bcopy(ior->io_data, sony->buffer, ior->io_count);
    (void)PBWrite(pb, TRUE);
  }
}

static boolean_t sony_read_done(register io_req_t ior)
{
  register sony_t sony = &sony_drive;
  register IOParam *pb = sony->pb;
  register int s;

  if (pb->ioResult == noErr) bcopy(sony->buffer, ior->io_data, pb->ioActCount);
  ior->io_error = sonyerror(pb->ioResult);
  ior->io_residual = ior->io_count - pb->ioActCount;
  (void)ds_read_done(ior);
  s = spl1();
  if (!queue_empty(&sony->pending)) ior = (io_req_t)queue_first(&sony->pending);
  else ior = (io_req_t)0;
  (void)splx(s);
  if (ior) sony_begin(ior);
  return TRUE;
}

static boolean_t sony_write_done(register io_req_t ior)
{
  register sony_t sony = &sony_drive;
  register IOParam *pb = sony->pb;
  register int s;

  ior->io_error = sonyerror(pb->ioResult);
  ior->io_residual = ior->io_count - pb->ioActCount;
  (void)ds_write_done(ior);
  s = spl1();
  if (!queue_empty(&sony->pending)) ior = (io_req_t)queue_first(&sony->pending);
  else ior = (io_req_t)0;
  (void)splx(s);
  if (ior) sony_begin(ior);
  return TRUE;
}

io_return_t sony_open(int number, dev_mode_t mode, io_req_t ior)
{
  register sony_id_t dev = (sony_id_t)&number;
  register sony_t sony;
  DrvSts stats;

  if (dev->drive < SONY_DRIVE_LOW || dev->drive > SONY_DRIVE_HIGH)
    return D_NO_SUCH_DEVICE;
  if (sonystatus(dev->drive, &stats) != noErr) return D_NO_SUCH_DEVICE;
  if (stats.installed != 1 && stats.installed != 0) return D_NO_SUCH_DEVICE;
  sony = &sony_drive;
  if (sony->ref_count == 0) {
    sony->pb = (IOParam *)NewPtr(sizeof(IOParam));
    if (!sony->pb) return D_NO_MEMORY;
    sony->buffer = (Ptr)NewPtr(32*1024);
    if (!sony->buffer) {
      DisposPtr(sony->pb);
      return D_NO_MEMORY;
    }
    queue_init(&sony->pending);
    sony->ref_count++;
  }
  return D_SUCCESS;
}

io_return_t sony_close(int number)
{
  register sony_t sony = &sony_drive;

  if (--sony->ref_count == 0) {
    DisposPtr(sony->pb);
    DisposPtr(sony->buffer);
  }
  return D_SUCCESS;
}

io_return_t sony_rw(int number, register io_req_t ior)
{
  register sony_t sony = &sony_drive;
  register io_return_t result;
  register int s;
  boolean_t wait = FALSE, sony_read_done(), sony_write_done();

  if (ior->io_count % SONY_BLOCK_SIZE) return D_INVALID_SIZE;
  if (ior->io_count > (32 * 1024)) ior->io_count = 32 * 1024;
  if (ior->io_op & IO_READ) {
    result = device_read_alloc(ior, ior->io_count);
    if (result != KERN_SUCCESS) return result;
    ior->io_done = sony_read_done;
  }
  else {
    result = device_write_get(ior, &wait);
    if (result != KERN_SUCCESS) return result;
    ior->io_done = sony_write_done;
  }
  s = spl1();
  if (!queue_empty(&sony->pending)) {
    enqueue_tail(&sony->pending, (queue_entry_t)ior);
    (void)splx(s);
    if (!wait) return D_IO_QUEUED;
    else {
      iowait(ior);
      return D_SUCCESS;
    }
  }
  enqueue_head(&sony->pending, (queue_entry_t)ior);
  (void)splx(s);
  sony_begin(ior);
  if (!wait) return D_IO_QUEUED;
  else {
    iowait(ior);
    return D_SUCCESS;
  }
}

io_return_t sony_getstat(int number,
                         register int flavor,
                         dev_status_t data,
                         unsigned int *count)
{
  register sony_id_t dev = (sony_id_t)&number;
  register sony_t sony = &sony_drive;
  register OSErr err;
  DrvSts stats;

  switch (flavor) {
    case SONY_STATUS:
      err = sonystatus(dev->drive, &stats);
      if (err != noErr)	return sonyerror(err);
      data[0] = *(int *)&stats.writeProt;
      *count = 1;
      break;
    default:
      return D_INVALID_OPERATION;
  }
  return D_SUCCESS;
}

io_return_t sony_setstat(int number,
                         register int flavor,
                         dev_status_t data,
                         unsigned int count)
{
  register sony_id_t dev = (sony_id_t)&number;
  register sony_t sony = &sony_drive;
  register OSErr err;
  register int mode;

  switch (flavor) {
    case SONY_EJECT:
      err = sonyeject(dev->drive);
      if (err != noErr) return sonyerror(err);
      break;
    case SONY_FORMAT:
      if (count == 1) mode = *data;
      else mode = 0;
      err = sonyformat(dev->drive, mode);
      if (err != noErr) return sonyerror(err);
      break;
    default:
      return D_INVALID_OPERATION;
  }
  return D_SUCCESS;
}

io_return_t sony_info(int number, int code, int *data)
{
  switch (code) {
    case D_INFO_BLOCK_SIZE:
      *data = SONY_BLOCK_SIZE;
      break;
    default:
      return D_IO_ERROR;
  }
  return D_SUCCESS;
}
