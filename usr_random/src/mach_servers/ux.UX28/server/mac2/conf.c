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
 * $Log:	conf.c,v $
 * Revision 2.2  90/09/04  17:33:57  bohman
 * 	Created.
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2/conf.c
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)conf.c	7.1 (Berkeley) 6/5/86
 */

#include <vice.h>
#include <pty.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/errno.h>

#include <sys/ioctl.h>
#include <sys/tty.h>

extern int	nulldev();
extern int	nodev();
extern int	seltrue();
extern struct tty *nulltty();

#define bdev_nop		nodev, nodev, nodev

extern int	bdisk_open(), bdisk_close(), bdisk_io();
#define bdisk_ops	bdisk_open, bdisk_close, bdisk_io

struct bdevsw	bdevsw[] =
{
    { "",	0,		bdev_nop },		/*0*/
    { "",	0,		bdev_nop },		/*1*/
    { "",	0,		bdev_nop },		/*2*/
    { "",	0,		bdev_nop },		/*3*/
    { "",	0,		bdev_nop },		/*4*/
    { "",	0,		bdev_nop },		/*5*/
    { "disk",	C_BLOCK(8),	bdisk_ops },		/*6*/
    { "",	0,		bdev_nop },		/*7*/
    { "ramdisk",C_BLOCK(8),	bdisk_ops },		/*8*/
};
int	nblkdev = sizeof (bdevsw) / sizeof (bdevsw[0]);

extern int	char_open(), char_close(), char_read(), char_write();
extern int	char_ioctl(), char_select(), char_port();
#define	char_ops \
	char_open, char_close, char_read, char_write, char_ioctl, \
	char_select, nulldev, nulltty

extern int	bdisk_read(), bdisk_write(), bdisk_ioctl();
#define	cdisk_ops \
	bdisk_open, bdisk_close, bdisk_read, bdisk_write, bdisk_ioctl, \
	seltrue, nulldev, nulltty
	
extern int	tty_open(), tty_close(), tty_read(), tty_write();
extern int	tty_ioctl(), ttselect(), tty_stop();
extern struct tty *tty_find_tty();
#define	tty_ops	\
	tty_open, tty_close, tty_read, tty_write, tty_ioctl, ttselect, \
	tty_stop, tty_find_tty

extern int	cons_open(), cons_write(), cons_ioctl();
#define	console_ops	\
	cons_open, tty_close, tty_read, cons_write, cons_ioctl, ttselect, \
	tty_stop, tty_find_tty

extern int	syopen(), syread(), sywrite(), syioctl(), syselect();
#define	sy_ops \
	syopen,   nulldev,   syread,   sywrite,   syioctl,   syselect, \
	nulldev, nulltty

extern int	logopen(), logclose(), logread(), logioctl(), logselect();
#define log_ops \
	logopen,  logclose,  logread,  nulldev,  logioctl,  logselect, \
	nulldev, nulltty

extern int	mmopen(), mmread(), mmwrite();
#define	mm_ops \
	mmopen,   nulldev,   mmread,   mmwrite,   nodev,     seltrue,  \
	nulldev, nulltty

#if	NPTY > 0
extern int	ptsopen(), ptsclose(), ptsread(), ptswrite();
extern int	ptyioctl(), ptsstop();
extern struct tty *pty_find_tty();
#define	pts_ops \
	ptsopen,  ptsclose,  ptsread,  ptswrite,  ptyioctl,  ttselect, \
	ptsstop, pty_find_tty

extern int	ptcopen(), ptcclose(), ptcread(), ptcwrite();
extern int	ptcselect();
#define	ptc_ops \
	ptcopen,  ptcclose,  ptcread,  ptcwrite,  ptyioctl,  ptcselect, \
	nulldev, nulltty
#endif	NPTY > 0

#if	VICE
extern int	rmtopen(), rmtclose(), rmtread(), rmtwrite(), rmtselect();
#define	rmt_ops \
	rmtopen, rmtclose, rmtread, rmtwrite, nodev, rmtselect, \
	nulldev, nulltty
#endif	VICE

int	video_ioctl();
#define video_ops \
    char_open, char_close, nodev, nodev, video_ioctl, \
    nodev, nodev, nulltty, char_port

int	disk_open(), disk_close(), disk_read(), disk_write(), disk_ioctl();
#define disk_ops \
    disk_open, disk_close, disk_read, disk_write, disk_ioctl, \
    seltrue, nulldev, nulltty

int	sony_ioctl();
#define	sony_ops \
    disk_open, disk_close, disk_read, disk_write, sony_ioctl, \
    seltrue, nulldev, nulltty

#define adb_ops	\
    char_open, char_close, char_read, nodev, char_ioctl, \
    char_select, nulldev, nulltty

#define serial_ops tty_ops

#define	cdev_nop \
	nodev, nodev, nodev, nodev, nodev, nodev, nulldev, nulltty

struct cdevsw	cdevsw[] =
{
    { "console",	0,		console_ops	},	/*0*/
    { "",		0,		cdev_nop	},	/*1*/
    { "",		0,		sy_ops		},	/*2*/
    { "",		0,		mm_ops		},	/*3*/
    { "",		0,		cdev_nop	},	/*4*/
    { "video",		0,		video_ops	},	/*5*/
    { "disk",		C_BLOCK(8),	cdisk_ops	},	/*6*/
    { "macdisk",	C_BLOCK(32),	disk_ops	},	/*7*/
    { "ramdisk",	C_BLOCK(8),	cdisk_ops	},	/*8*/
    { "sony",		0,		sony_ops	},	/*9*/
    { "adb",		0,		adb_ops		},	/*10*/
    { "serial",		0,		serial_ops	},	/*11*/
    { "ramdisk",	C_BLOCK(8),	disk_ops	},	/*12*/
    { "",		0,		cdev_nop	},	/*13*/
    { "",		0,		cdev_nop	},	/*14*/
    { "",		0,		cdev_nop	},	/*15*/
    { "",		0,		cdev_nop	},	/*16*/
    { "",		0,		cdev_nop	},	/*17*/
    { "",		0,		cdev_nop	},	/*18*/
    { "",		0,		cdev_nop	},	/*19*/
#if	NPTY > 0
    { "",		0,		pts_ops		},	/*20*/
    { "",		0,		ptc_ops		},	/*21*/
#else	NPTY > 0
    { "",		0,		cdev_nop	},	/*20*/
    { "",		0,		cdev_nop	},	/*21*/
#endif	NPTY > 0
#if	VICE
    { "",		0,		rmt_ops		},	/*22*/
#else	VICE
    { "",		0,		cdev_nop	},	/*22*/
#endif	VICE
    { "",		0,		log_ops,	},	/*23*/
};
int	nchrdev = sizeof (cdevsw) / sizeof (cdevsw[0]);

dev_t	sydev = makedev(2, 0);	/* device number for indirect tty */

/* Conjure up a name string for funny devices. */
int check_dev(dev_t dev, char str[])
{
  return ENXIO;
}
