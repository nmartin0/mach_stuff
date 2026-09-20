/*
 * Copyright (c) 1991 Carnegie Mellon University
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
/*
 *
 * Purpose:
 *	V86 BIOS Disk emulation
 *
 * HISTORY: 
 * $Log:	bios_disk.h,v $
 * Revision 2.3  92/02/02  23:02:16  rvb
 * 	Added Huge file system type as a valid System Type.
 * 	[92/01/27            grm]
 * 
 * Revision 2.2  91/12/05  16:40:03  grm
 * 	New Copyright
 * 	[91/05/28  14:38:52  grm]
 * 
 * 	Created.
 * 	[90/12/08  20:50:29  grm]
 * 
 */
#include "base.h"
#include "bios.h"
#include <sys/file.h>
#include <sys/ioctl.h>
#include <errno.h>

/* 	int 13 bios disk operations	*/

#define DISK_RESET		0x00
#define DISK_STATUS		0x01
#define DISK_READ_SECTOR	0x02
#define DISK_WRITE_SECTOR	0x03
#define DISK_VERIFY_SECTOR	0x04
#define DISK_FORMAT_TRACK	0x05
#define DISK_FORMAT_BAD_TRACK	0x06
#define DISK_FORMAT_DRIVE	0x07
#define DISK_DRIVE_PARAMETERS	0x08
#define DISK_INIT_FIXED_DISK	0x09
#define DISK_READ_SECTOR_LONG	0x0a
#define DISK_WRITE_SECTOR_LONG	0x0b
#define DISK_SEEK		0x0c
#define DISK_RESET_FIXED_DISK	0x0d
#define DISK_GET_TYPE		0x15
#define DISK_CHANGE_LINE	0x16

#define MICRO_FLOPPY_0		0xf0
#define FIXED_DISK		0xf8
#define BIG_FLOPPY		0xf9

#define	FD_5_L			0x01
#define	FD_5_H			0x02
#define	FD_3_L			0x03
#define	FD_3_H			0x04

#define DRIVE_NOT_PRESENT	0x00
#define DRIVE_FLOPPY_NO_CNGLN	0x01
#define DRIVE_FLOPPY_WITH_CNGLN	0x02
#define DRIVE_FIXED		0x03

#define ERR_NONE		0x00
#define	ERR_INVALID_CMD		0x01
#define	ERR_NO_ADDR_MARK	0x02
#define	ERR_WRITE_PROT		0x03
#define	ERR_SECTOR_NOT_FOUND	0x04
#define	ERR_RESET_FAILED	0x05
#define	ERR_FLOPPY_DISK_REMOVED	0x06
#define	ERR_BAD_PARM_TABLE	0x07
#define	ERR_DMA_OVERRUN		0x08
#define	ERR_DMA_CROSSED_64k	0x09
#define	ERR_BAD_SECTOR_FLAG	0x0a
#define	ERR_BAD_TRACK_FLAG	0x0b

#define DOS_SYS1		0x01
#define DOS_SYS2		0x04
#define DOS_SYS3		0x06
#define BOOTABLE		0x80

#define SEC_LENGTH		0x200

#undef ZERO
#define ZERO			0

struct boot_sector {
/*00*/	unsigned char	ex;
/*01*/	unsigned char	xx;
/*02*/	unsigned char	x90;
/*03*/	unsigned char	oem[8];			/* oem id */
/*0b*/	unsigned char	bps[2];			/* blocks per sector */
/*0d*/	unsigned char	spa;			/* sectors per allocation */
/*0e*/	unsigned char	rsvds[2];		/* reserved sectors */
/*10*/	unsigned char	fats;			/* Number of FAT's */
/*11*/	unsigned char	rtents[2];		/* root entries */
/*13*/	unsigned char	sects[2];		/* total sectors */
/*15*/	unsigned char	media;			/* media descriptor byte */
/*16*/	unsigned short	secfat;			/* sectors for fat */
/*18*/	unsigned short sectrk;			/* sectors per track */
/*1a*/	unsigned short heads;			/* heads per track */
/*1c*/	unsigned long	hidden;			/* hidden sectors */
/*20*/	unsigned long	SECTS;			/* total number of sectors */
/*24*/	unsigned char	drive;			/* drive number */
/*25*/	unsigned char	rc;			/* reserved */
/*26*/	unsigned char	x29;			/* extended boot signature record */
/*27*/	unsigned char	vid[4];
/*2b*/	unsigned char	label[11];
/*36*/	unsigned char	rd[8];
/*3e*/	unsigned char	boot[1];
};

struct fat12 {
	unsigned int	 low : 12,
		high : 12;
};

struct fat16 {
	unsigned int	 low : 16,
		high : 16;
};

#define	CL12_MASK 0x0ff0
#define	CL16_MASK 0xfff0

struct dir_entry {
/*00*/	unsigned char	name[8];
/*08*/	unsigned char	ext[3];
/*0b*/	unsigned char	attr;
/*0c*/	unsigned char	ra[10];
/*16*/	unsigned short	time;
/*18*/	unsigned short	date;
/*1a*/	unsigned short	cluster;
/*1c*/	unsigned long	size;
};

#define	F_END	0
#define	F_ERASE	0xe5
#define F_E5	0x05
#define F_DOT	0x2e

#define A_RO	0x01
#define A_HID	0x02
#define A_SYS	0x04
#define A_VOL	0x08
#define A_DIR	0x10
#define A_ARCH	0x20

#define A_A "\8\6ARCH\5DIR\4VOL\3SYS\2HID\1RO"

struct f_time {
	unsigned int	sec2 : 5,
		 min : 6,
		hour : 5;
};

struct f_date {
	unsigned int	 day : 5,
		 mon : 4,
		year : 7;
};
#define GF_DATE(x,f) ((struct f_date *) &x)->f
#define GF_TIME(x,f) ((struct f_time *) &x)->f		

struct dir_entry *lookup();

#define NO_DISK_PRESENT	51
#define DRIVE_ERROR	52

