/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/* read-only access to scsi disks */

#include <SCSI.h>

#include "param.h"
#include "fs.h"
#include "inode.h"
#include "scsi.h"
#include "string.h"
#include "disklabel.h"
#include "ufs.h"

struct scsiRWcmd {
	char	cmd;
	char	lun:3,
			blkMSB:5;
	char	blk;
	char	blkLSB;
	char	len;
	char	reserved;
};

#define	MACH_PARTITION_TYPE	((char *)getstr(STR_PART_TYPE))

/* read from scsi disk, return < 0 if error */
int scsi_disk_read(io)
register struct iob *io;
{
	long blk;

	blk = io->i_bn;
	switch (major(io->i_dev)) {
		case MAJOR_SCSI:
		{
			int err;
			short stat, mess;
			struct scsiRWcmd cmd;
			SCSIInstr scsiprog[3];

			cmd.cmd = 0x8;
			cmd.lun = scsidrv(io->i_dev);
			cmd.blkMSB = blk >> 16;
			cmd.blk = (blk >> 8) & 255;
			cmd.blkLSB = blk & 255;
			cmd.len = btodb(io->i_cc);
			cmd.reserved = 0;
			scsiprog[0].scOpcode = scInc;
			scsiprog[0].scParam1 = (long)io->i_ma;
			scsiprog[0].scParam2 = 512;
			scsiprog[1].scOpcode = scLoop;
			scsiprog[1].scParam1 = -10;
			scsiprog[1].scParam2 = btodb(io->i_cc);
			scsiprog[2].scOpcode = scStop;
			scsiprog[2].scParam1 = 0;
			scsiprog[2].scParam2 = 0;
			if (err = SCSIGet()) {
				SCSIReset();
				return E_SCSIGET_ERR;
			}
			if (err = SCSISelect(scsictlr(io->i_dev))) {
				SCSIReset();
				return E_SCSISEL_ERR;
			}
			if (err = SCSICmd((Ptr)&cmd, sizeof(cmd))) {
				SCSIReset();
				return E_SCSICMD_ERR;
			}
			if (err = SCSIRead((Ptr)scsiprog)) {
				SCSIReset();
				return E_SCSIREAD_ERR;
			}
			if (err = SCSIComplete(&stat, &mess, 120)) {
				SCSIReset();
				return E_SCSIMSG_ERR;
			}
			break;
		}
		default:
			return E_BAD_DEVICE_MAJ;
			break;
	}
	return 0;
}

/* open partition on scsi disk, return < 0 if error */
int scsi_disk_open(io)
register struct iob *io;
{
	int pmlen, part;
	Partition *dpme;
	Block0 *b0;

	io->i_boff = 0;
	if (major(io->i_dev) == MAJOR_SCSI) {
		io->i_ma = io->i_buf;
		io->i_cc = DEV_BSIZE;
		io->i_bn = 0;
		if (scsi_disk_read(io)) return -1;
		b0 = (Block0 *)io->i_buf;
		if (b0->sbSig != sbSIGWord) return E_NO_PART_INFO;
		io->i_bn = 1;
		if (scsi_disk_read(io)) return -1;
		dpme = (Partition *)io->i_buf;
		if (dpme->pmSig != pMapSIG) return E_BAD_DPME_MAGIC;
		pmlen = dpme->pmMapBlkCnt;
		for (part = 1; part <= pmlen; part++) {
			io->i_bn = part;
			if (scsi_disk_read(io)) return -1;
			if (strcmp(MACH_PARTITION_TYPE, dpme->pmParType) == 0) break;
		}
		if (part > pmlen) return E_NO_MACH_PART;
		io->i_boff = dpme->pmPyPartStart;
	}
	else return E_BAD_DEVICE_MAJ;
	return 0;
}

