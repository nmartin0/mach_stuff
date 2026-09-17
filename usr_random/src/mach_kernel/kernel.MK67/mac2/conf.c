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
 * Revision 2.2  91/09/12  16:39:29  bohman
 * 	Created.
 * 	[91/09/11  14:26:30  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2/conf.c
 */

#include <device/conf.h>

extern int	block_io_mmap();

extern int	console_open(), console_close(), console_read();
extern int	console_write(), console_getstat(), console_setstat();
extern int	console_portdeath();
#define console_name	"console"

extern int	adb_open(), adb_close(), adb_read();
extern int	adb_getstat(), adb_setstat();
#define adb_name	"adb"

extern int	macdisk_open(), macdisk_close(), macdisk_read(), macdisk_write();
extern int	macdisk_getstat(), macdisk_info();
#define macdisk_name	"macdisk"

extern int	disk_open(), disk_close(), disk_read(), disk_write(), disk_info();
#define disk_name	"disk"

extern int	timeopen(), timeclose(), timeread(), timemmap();
#define timename	"time"

extern int	ramdisk_open(), ramdisk_close(), ramdisk_read();
extern int	ramdisk_write(), ramdisk_info();
#define ramdisk_name	"ramdisk"

extern int	enopen(), enoutput();
extern int	ensetinput(), engetstat(), ensetstat();
#define enname		"en"

extern int	video_open(), video_close(), video_mmap();
extern int	video_getstat(), video_setstat();
#define video_name	"video"

extern int	sony_open(), sony_close(), sony_rw();
extern int	sony_getstat(), sony_setstat(), sony_info();
#define sony_name	"sony"

extern int	kernmmap(), kerngetstat(), kernsetstat();
#define kernname	"kern"

extern int	serial_open(), serial_close(), serial_read(), serial_write();
extern int	serial_getstat(), serial_setstat(), serial_portdeath();
#define serial_name	"serial"

/* List of devices - console must be at slot 0 */
struct dev_ops	dev_name_list[] =
{
    /*
        name,		open,		close,		read,
        write,		getstat,	setstat,	mmap,
        async_in,	reset,		port_death,	subdev,
	dev_info,
    */
    { /* unit 0 */
	console_name,	console_open,	console_close,	console_read,
	console_write,	console_getstat,console_setstat, 0,
	nodev,		nulldev,	console_portdeath, 0,
	nodev,
    },
    { /* unit 1 */
	adb_name,	adb_open,	adb_close,	adb_read,
	nodev,		adb_getstat,	adb_setstat,	0,
	nodev,		nulldev,	nulldev,	0,
	nodev,
    },
    { /* unit 2 */
	disk_name,	disk_open,	disk_close,	disk_read,
 	disk_write,	nulldev		,nulldev,	block_io_mmap,
	nodev,		nulldev,	nulldev,	0,
	disk_info,
    },
    { /* unit 3 */
	macdisk_name,	macdisk_open,	macdisk_close,	macdisk_read,
 	macdisk_write,	macdisk_getstat,nulldev,	block_io_mmap,
	nodev,		nulldev,	nulldev,	32,
	macdisk_info,
    },
    { /* unit 4 */
	timename,	timeopen,	timeclose,	timeread,
	nulldev,	nulldev,	nulldev,	timemmap,
	nodev,		nulldev,	nulldev,	0,
	nodev,
    },
    { /* unit 5 */
	ramdisk_name,	ramdisk_open,	ramdisk_close,	ramdisk_read,
	ramdisk_write,	nulldev,	nulldev,	block_io_mmap,
	nodev,		nulldev,	nulldev,	0,
	ramdisk_info,
    },
    { /* unit 6 */
	enname,		enopen,		nulldev,	nodev,
	enoutput,	engetstat,	ensetstat,	0,
	ensetinput,	nulldev,	nulldev,	0,
	nodev,
    },
    { /* unit 7 */
	video_name,	video_open,	video_close,	nodev,
	nodev,		video_getstat,	video_setstat,	video_mmap,
	nodev,		nulldev,	nulldev,	0,
	nodev,
    },
    { /* unit 8 */
	sony_name,	sony_open,	sony_close,	sony_rw,
	sony_rw,	sony_getstat,	sony_setstat,	block_io_mmap,
	nodev,		nulldev,	nulldev,	0,
	sony_info,
    },
    { /* unit 9 */
	kernname,	nulldev,	nulldev,	nodev,
	nodev,		kerngetstat,	kernsetstat,	kernmmap,
	nodev,		nulldev,	nulldev,	0,
	nodev,
    },
    { /* unit 10 */
	serial_name,	serial_open,	serial_close,	serial_read,
	serial_write,	serial_getstat,	serial_setstat,	nodev,
	nodev,		nulldev,	serial_portdeath,0,
	nodev,
    },
#ifdef notdef
    { /* unit 11 */
	mhdname,	mhdopen,	nulldev,	mhdread,
	mhdwrite,	nulldev,	nulldev,	block_io_mmap,
	nodev,		nulldev,	nulldev,	32,
	nodev,
    },
#endif
};

int dev_name_count = sizeof(dev_name_list) / sizeof(dev_name_list[0]);

/* Indirect list. */
struct dev_indirect dev_indirect_list[] = {
    /* access console by "console" instead of "console0" */
    {
	"console",	&dev_name_list[0],		0,
    },
};

int dev_indirect_count = sizeof(dev_indirect_list) / sizeof(dev_indirect_list[0]);
