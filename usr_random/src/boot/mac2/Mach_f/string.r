/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

type 'cstr' {
	array {
		cstring[64];
	};
};

resource 'cstr' (129, "string") {
	{
		".",
		"Mach_UNIX_BSD4.3",
		" ",
		"\pAbout to start kernel...",
		""
	}
};
