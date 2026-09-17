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
 * $Log:	scsi.c,v $
 * Revision 2.2  91/09/12  16:47:09  bohman
 * 	Created.
 * 	[91/09/11  15:35:57  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2dev/scsi.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mach/mach_types.h>

#include <device/device_types.h>
#include <device/io_req.h>

#include <mac2/act.h>

#include <mac2dev/scsi.h>

#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/SCSI.h>

#define SCSI_TSTUNITRDY	0x00
#define SCSI_READ	0x08
#define SCSI_WRITE	0x0a

#define scsiMSBlk(n)	(((n) >> 16) & 0x1f)
#define scsiBlk(n)	(((n) >> 8) & 0xff)
#define scsiLSBlk(n)	((n) & 0xff)

#define scsiMAXNBlk	256

#define scsiNBlk(n)	(((n) < scsiMAXNBlk)? (n): 0)

#define SCSI_CMDLEN	6	/* group 0 commands only */

typedef union {
    unsigned char	op;
    struct {
	unsigned char	op;
	unsigned char	lun:3,
			msblk:5;
	unsigned char	blk;
	unsigned char	lsblk;
	unsigned char	nblk;
    } rw;
    unsigned char	xxx[SCSI_CMDLEN];
} SCSICmdBlk;

#define SCSI_X_READ	0x01
#define SCSI_X_WRITE	0x02
#define SCSI_X_BLIND	0x04

#define scsiBranch(n)	((n) * sizeof (SCSITransferInstr))

#define SCSI_STAT_REQ	0x20

/*
 * Keep a queue of pending requests, as well as the current one.
 */
static queue_head_t	scsi_queue;
static io_req_t		scsi_cur;

/*
 * The macII hardware does support interrupt driven disk I/O, however the
 * current (7.0) MacOS code does not support the use of the hardware in this
 * manner.  We fake it by asynchronously polling the REQ line to begin data
 * transfers.  The 'softint' activity facility is used for this purpose.
 * Activities in this queue are executed whenever a normal return to user
 * mode is taken (via rte_user) or on a return from an interrupt (via rte_intr)
 * when the previous context is at IPL0.
 */
static struct act	*scsi_act;
#define SCSI_ACT_LIST	0

static int		scsi_mode;

static			scsi_cmd(), scsi_transfer();

/*
 * Called during machine initialization.
 */
void scsi_initialize()
{
  extern struct actlist	actsoft;
  static void scsi_intr();

  scsi_act = makeact(scsi_intr, SR_IPL0, 1);
  if (!scsi_act) panic("scsi_initialize");
  addact(SCSI_ACT_LIST, scsi_act, &actsoft);
  queue_init(&scsi_queue);
}

/*
 * Meant to be called by a client layer when a thread needs to block inside
 * the kernel until a request is completed.
 */
io_return_t scsi_wait(register io_req_t ior)
{
  register int s;

  s = splbio();
  while ((ior->io_op & IO_DONE) == 0) {
    assert_wait((int)ior, FALSE);
    thread_block((void (*)()) 0);
  }
  splx(s);
  return ior->io_error;
}

/*
 * Called by a client layer to perform a SCSI operation.  Currently, this
 * module only supports disk operations, however it would be relatively
 * straightforward to generalize it for other types of SCSI devices.
 */
io_return_t scsi_op(register io_req_t ior)
{
  register s = splbio();

  if (scsi_cur != 0 || !queue_empty(&scsi_queue)) {
    enqueue_tail(&scsi_queue, (queue_entry_t)ior);
    splx(s);
    return D_IO_QUEUED;
  }
  scsi_cur = ior;
  splx(s);
  return scsi_cmd(ior);
}

#define IODONE(result)		\
MACRO_BEGIN			\
  register	s = splbio();	\
  scsi_cur = 0;			\
  splx(s);			\
  return (result);		\
MACRO_END

#define scsiCmdSetup(x) 	\
MACRO_BEGIN			\
  cmd = (SCSICmdBlk) { 0 };	\
  cmd.op = (x);			\
MACRO_END

/*
 * Send a SCSI command to a target.  This routine may return D_IO_QUEUED if
 * there is a data phase for the command (scsi_transfer() gets called) and the
 * target does not assert REQ right after the command is sent.
 */
static io_return_t scsi_cmd(register io_req_t ior)
{
  static unsigned short stat, msg;
  static SCSICmdBlk cmd;

  switch (ior->io_op & (IO_READ | IO_WRITE | IO_OPEN)) {
    case IO_OPEN:
      scsiCmdSetup(SCSI_TSTUNITRDY);
      scsi_mode = 0;
      break;
    case IO_READ:
      scsiCmdSetup(SCSI_READ);
      scsi_mode = SCSI_X_READ | SCSI_X_BLIND;
      cmd.rw.msblk = scsiMSBlk(ior->io_recnum);
      cmd.rw.blk = scsiBlk(ior->io_recnum);
      cmd.rw.lsblk = scsiLSBlk(ior->io_recnum);
      cmd.rw.nblk = scsiNBlk(ior->io_count >> SCSI_NBBLKLOG2);
      break;
    case IO_WRITE:
      scsiCmdSetup(SCSI_WRITE);
      scsi_mode = SCSI_X_WRITE | SCSI_X_BLIND;
      cmd.rw.msblk = scsiMSBlk(ior->io_recnum);
      cmd.rw.blk = scsiBlk(ior->io_recnum);
      cmd.rw.lsblk = scsiLSBlk(ior->io_recnum);
      cmd.rw.nblk = scsiNBlk(ior->io_count >> SCSI_NBBLKLOG2);
      break;
    default:
      panic("scsi_cmd");
  }
  if (SCSIGet()) {
    (void)SCSIReset();
    IODONE(D_IO_ERROR);
  }
  if (SCSISelect(ior->io_unit)) IODONE(D_NO_SUCH_DEVICE);
  if (SCSICmd(&cmd, SCSI_CMDLEN)) {
    (void)SCSIReset();
    IODONE(D_IO_ERROR);
  }
  if (scsi_mode) return scsi_transfer(ior);
  else if (SCSIComplete(&stat, &msg, 120)) {
    (void)SCSIReset();
    IODONE(D_IO_ERROR);
  }
  IODONE(D_SUCCESS);
}
#undef scsiCmdSetup

/*
 * This can be patched to FALSE to make all SCSI operations be synchronous.
 */
boolean_t async_scsi = TRUE;

/*
 * Perform a SCSI data transfer phase for a SCSI command.
 */
static io_return_t scsi_transfer(register io_req_t ior)
{
  OSErr result;
  static SCSITransferInstr prog[3];
  static unsigned short	stat, msg;
#ifdef MODE24
  static unsigned char buffer[SCSI_MAXPHYS];
  extern boolean_t mac32bit;
#endif /* MODE24 */

  if (async_scsi) if ((SCSIStat() & SCSI_STAT_REQ) == 0) {
    runact(SCSI_ACT_LIST, scsi_act, 0, 0);
    return D_IO_QUEUED;
  }
  if (ior->io_count > SCSI_MAXPHYS) panic("scsi_transfer");
  if (scsi_mode & SCSI_X_BLIND) {
    prog[0] =
      (SCSITransferInstr)
        { scInc,
#ifdef MODE24
          mac32bit ? (unsigned)ior->io_data : (unsigned)buffer,
#else /* MODE24 */
          (unsigned)ior->io_data,
#endif /* MODE24 */
          SCSI_NBBLK
        };
    prog[1] =
      (SCSITransferInstr)
        { scLoop, scsiBranch(-1), (ior->io_count >> SCSI_NBBLKLOG2) };
    prog[2] =
      (SCSITransferInstr)
        { scStop };
  }
  else {
    prog[0] =
      (SCSITransferInstr)
        { scNoInc,
#ifdef MODE24
          mac32bit ? (unsigned)ior->io_data : (unsigned)buffer,
#else /* MODE24 */
          (unsigned)ior->io_data,
#endif /* MODE24 */
          ior->io_count
        };
    prog[1] =
      (SCSITransferInstr)
        { scStop };
  }
  if (scsi_mode & SCSI_X_WRITE) {
#ifdef MODE24
    if (!mac32bit) bcopy(ior->io_data, buffer, ior->io_count);
#endif /* MODE24 */
    result = (scsi_mode & SCSI_X_BLIND) ? SCSIWBlind(prog) : SCSIWrite(prog);
  }
  else {
    result = (scsi_mode & SCSI_X_BLIND) ? SCSIRBlind(prog) : SCSIRead(prog);
#ifdef MODE24
    if (!mac32bit) bcopy(buffer, ior->io_data, ior->io_count);
#endif /* MODE24 */
  }
  if (result) {
    (void)SCSIReset();
    IODONE(D_IO_ERROR);
  }
  if (SCSIComplete(&stat, &msg, 120)) {
    (void)SCSIReset();
    IODONE(D_IO_ERROR);
  }
  IODONE(D_SUCCESS);
}
#undef IODONE

/*
 * The 'interrupt' routine.  First it attempts to finish up the currently
 * active command, and then performs the next one.
 */
static void scsi_intr()
{
  register io_req_t ior;
  register io_return_t result;

  ior = scsi_cur;
  if (!ior) panic("scsi_intr");
  result = scsi_transfer(ior);
  if (result == D_IO_QUEUED) return;
  if (ior->io_error = result) ior->io_op |= IO_ERROR;
  iodone(ior);
  while (scsi_cur = (io_req_t)dequeue_head(&scsi_queue)) {
    ior = scsi_cur;
    result = scsi_cmd(ior);
    if (result == D_IO_QUEUED) break;
    if (ior->io_error = result) ior->io_op |= IO_ERROR;
    iodone(ior);
  }
}

/* do a scsi disk ready test, returns non-zero if the disk is not ready */
io_return_t scsi_disk_ready(register io_req_t ior)
{
  io_return_t result;

  ior->io_op = IO_OPEN | IO_WANTED;
  ior->io_mode = 0;
  ior->io_alloc_size = 0;
  result = scsi_op(ior);
  if (result == D_IO_QUEUED) result = scsi_wait(ior);
  return result;

} /* scsi_disk_ready() */

/* do a scsi disk read operation, returns non-zero if error */
io_return_t scsi_disk_read(register io_req_t ior,
                           unsigned int recnum,
                           unsigned int count,
                           unsigned char *buffer)
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

} /* scsi_disk_read() */

/* do a scsi disk write operation, returns non-zero if error */
io_return_t scsi_disk_write(register io_req_t ior,
                            unsigned int recnum,
                            unsigned int count,
                            unsigned char *buffer)
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

} /* scsi_disk_write() */

