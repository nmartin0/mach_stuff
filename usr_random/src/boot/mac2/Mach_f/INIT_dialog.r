/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

resource 'DLOG' (130) {
	{40, 40, 180, 465},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	130,
	""
	/****** Extra bytes follow... ******/
	/* $"0128 0A"                                            /* .(. */
};

resource 'DITL' (130, "INIT dialog") {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{7, 137, 24, 301},
		StaticText {
			disabled,
			"Mach Operating System"
		},
		/* [2] */
		{35, 148, 50, 216},
		StaticText {
			disabled,
			"Startup in"
		},
		/* [3] */
		{35, 217, 50, 227},
		UserItem {
			disabled
		},
		/* [4] */
		{35, 229, 50, 288},
		StaticText {
			disabled,
			"seconds."
		},
		/* [5] */
		{32, 142, 51, 293},
		UserItem {
			disabled
		},
		/* [6] */
		{32, 78, 51, 138},
		Button {
			enabled,
			"Cancel"
		},
		/* [7] */
		{32, 297, 51, 357},
		Button {
			enabled,
			"Pause"
		},
		/* [8] */
		{62, 39, 79, 387},
		Button {
			enabled,
			""
		},
		/* [9] */
		{62, 39, 79, 387},
		UserItem {
			disabled
		},
		/* [10] */
		{88, 39, 105, 387},
		Button {
			enabled,
			""
		},
		/* [11] */
		{88, 39, 105, 387},
		UserItem {
			disabled
		},
		/* [12] */
		{113, 126, 132, 221},
		CheckBox {
			enabled,
			"Single User"
		},
		/* [13] */
		{113, 246, 132, 311},
		CheckBox {
			enabled,
			"Debug"
		},
		/* [14] */
		{12, 19, 44, 51},
		Icon {
			disabled,
			128
		},
	}
};
