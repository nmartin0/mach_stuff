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
 * $Log:	setroot.c,v $
 * Revision 2.2  91/09/12  16:43:42  bohman
 * 	Created.
 * 	[91/09/11  15:02:41  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2/setroot.c
 */

char *root_name;

get_root_device()
{
    extern int bootdev; /* from locore.s */
    int minor;
    int major;

    minor = (bootdev >> 5) & 0x7;
    major = (bootdev >> 8) & 0xFF;

    if (major == 0) {
      root_name = "disk0a";
      root_name[4] = '0' + minor;
    }
    else if (major == 8) {
      root_name = "ramdisk0a";
      root_name[7] = '0' + minor;
    }
    else root_name = "   ";
}
