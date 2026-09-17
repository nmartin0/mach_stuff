/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

resource 'MENU' (128) {
	128,
	textMenuProc,
	allEnabled,
	enabled,
	"Device",
	{	/* array: 9 elements */
		/* [1] */
		"/dev/sd0a", noIcon, noKey, noMark, plain,
		/* [2] */
		"/dev/sd1a", noIcon, noKey, noMark, plain,
		/* [3] */
		"/dev/sd2a", noIcon, noKey, noMark, plain,
		/* [4] */
		"/dev/sd3a", noIcon, noKey, noMark, plain,
		/* [5] */
		"/dev/sd4a", noIcon, noKey, noMark, plain,
		/* [6] */
		"/dev/sd5a", noIcon, noKey, noMark, plain,
		/* [7] */
		"/dev/sd6a", noIcon, noKey, noMark, plain,
		/* [8] */
		"/dev/ramdisk0a", noIcon, noKey, noMark, plain,
		/* [9] */
		"System Volume", noIcon, noKey, noMark, plain
	}
};
