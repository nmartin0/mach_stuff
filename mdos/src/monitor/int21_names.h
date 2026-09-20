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
 * HISTORY:
 * $Log:	int21_names.h,v $
 * Revision 2.2  91/12/05  16:42:50  grm
 * 	Initial rcs version.
 * 	[91/12/04            grm]
 * 
 *
 * Names of dos and bios functions 
 */

char * names21[] = {
	"Terminate Process",			/* 0x00 */
	"Character Input with Echo",
	"Character Output",
	"Auxiliary Input",
	"Auxiliary Output",
	"Printer Output",
	"Direct Console I/O",
	"Unifiltered Char Input w/o Echo",
	"Character Input w/o Echo",		/* 0x08 */
	"Display String",
	"Buffered Keyboard Input",
	"Check Input Status",
	"Flush In Buf and then Input",
	"Disk Reset",
	"Select Disk",
	"Open File F",
	"Close File F",				/* 0x10 */
	"Find First File F",
	"Find Next File F",
	"Delete File F",
	"Sequential Read F",
	"Sequential Write F",
	"Create File F",
	"Rename File F",
	"Reserved",				/* 0x18 */
	"Get Current Disk",
	"Set DTA Address",
	"Get Default Drive Data",
	"Get Drive Data",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",				/* 0x20 */
	"Random Read F",
	"Random Write F",
	"Get File Size F",
	"Set Relative Record Number F",
	"Set Interrupt Vector",
	"Create New PSP",
	"Random Block Read F",
	"Random Block Write F",			/* 0x28 */
	"Parse Filename",
	"Get Date",
	"Set Date",
	"Get Time",
	"Set Time",
	"Set Verify Flag",
	"Get DTA Address",
	"Get MS-DOS Version Number",		/* 0x30 */
	"Terminate and Stay Resident",
	"Reserved",
	"Get/Set Break Flag, Get Boot Drive",
	"Reserved",
	"Get Interrupt Vector",
	"Get Drive Allocation Information",
	"Reserved",
	"Get/Set Country Information",		/* 0x38 */
	"Create Directory",
	"Delete Dircetory",
	"Set Current Directory",
	"Create File H",
	"Open File H",
	"Close File H",
	"Read File or Device H",
	"Write File or Device H",			/* 0x40 */
	"Delete File H",
	"Set File Pointer H",
	"Get/Set File Attributes",
	"IOCTL",
	"Duplicate Handle",
	"Redirect Handle",
	"Get Current Directory",
	"Allocate Memory Block",		/* 0x48 */
	"Release memory Block",	
	"Resize Memory Block",
	"Execute Program",
	"Terminate Process with Return Code",
	"Get Return Code",
	"Find First File H",
	"Find Next File H",
	"Reserved",				/* 0x50 */
	"Reserved",
	"Reserved",
	"Reserved",
	"Get Verify Flag",
	"Reserved",
	"Rename File",
	"Get/Set File Date and Time H",
	"Get/Set Allocation Strategy",		/* 0x58 */
	"Get Extended Error Information",
	"Create Temporary File H",
	"Create New File H",
	"Lock/Unlock File Region H",
	"Reserved",
	"Get Machine Name, G/S Printer Setup",
	"Device Redirection",
	"Reserved",				/* 0x60 */
	"Reserved",
	"Get PSP Address",
	"Get DBCS Lead Byte Table",
	"Reserved",
	"Get Extended Country Information",
	"Get/Set Code Page",
	"Set Handle Count",
	"Commit File H",				/* 0x68 */
	"Reserved",
	"Reserved",
	"Reserved",
	"Extended Open File H"
	};

char * bios_13_names[] = {
	"BIOS: Disk Reset",			/* 0x00 */
	"BIOS: Get Disk System Status",
	"BIOS: Read Sector",
	"BIOS: Write Sector",
	"BIOS: Verify Sector",
	"BIOS: Format Track",
	"BIOS: Format Bad Track",
	"BIOS: Format Drive",
	"BIOS: Get Drive Parameters",		/* 0x08 */
	"BIOS: Initialize Fixed Disk Chars",
	"BIOS: Read Sector Long",
	"BIOS: Write Sector Long",
	"BIOS: Seek",
	"BIOS: Reset Fixed Disk System",
	"BIOS: Read Sector Buffer",
	"BIOS: Write Sector Buffer",
	"BIOS: Get Drive Status",		/* 0x10 */
	"BIOS: Recalibrate Drive",
	"BIOS: Controller RAM Diagnostic",
	"BIOS: Controller Drive Diagnostic",
	"BIOS: Controller Internal Diagnostic",
	"BIOS: Get Disk Type",
	"BIOS: Get Disk Change Status",
	"BIOS: Set Disk Type",
	"BIOS: Set Media Type for Format",	/* 0x18 */
	"BIOS: Park Heads",
	"BIOS: Format ESDI Drive"
	};

