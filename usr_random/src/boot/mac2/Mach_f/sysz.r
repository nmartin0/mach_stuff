/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

type 'sysz' {
	longint;
};

/* space for system heap */
resource 'sysz' (0) {
	0x00008000
};
