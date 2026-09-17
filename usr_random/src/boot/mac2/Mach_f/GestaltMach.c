/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <OSUtils.h>
#include <SysEqu.h>

#include "GestaltMach.h"

/* return zero if this Macintosh can run Mach */
int GestaltMach()
{
	SysEnvRec sysenv;
	if (SysEnvirons(1, &sysenv) != noErr) return -1;
	if (sysenv.systemVersion < 0x0607) return -1;
	switch (sysenv.machineType) {
		case envMacII:
		case envMacIIx:
		case envMacIIcx:
		case envMacIIci:
		case envSE30:
		case /* envMacIIfx */ 11:
			break;
		default:
			return -1;
	}
	if (*(long *)MemTop < (8000 * 1024)) return -1;
	/* need to insure that MMU is available */
	return 0;
}