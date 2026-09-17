#include "Types.r"

resource 'DITL' (129, "AlertMsg") {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 41, 88, 384},
		StaticText {
			enabled,
			"^0^1^2^3"
		},
		/* [2] */
		{15, 84, 33, 341},
		StaticText {
			disabled,
			"Mach Operating System Startup Error"
		}
	}
};

resource 'ALRT' (128) {
	{200, 40, 300, 465},
	129,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
	/****** Extra bytes follow... ******/
	/* $"700A"                                               /* p. */
};

type 'mstr' {
	array {
		cstring[64];
	};
};

resource 'mstr' (128, "AlertMsg") {
	{
		"",
		"Out of memory in application zone.",
		"file already open",
		"bad fs magic number",
		"null file name",
		"not a directory",
		"empty directory",
		"Can not find Mach kernel.",
		"no apple partition info",
		"bad apple partition map magic",
		"bad disklabel magic",
		"Can not find a Mach partition on the disk.",
		"SCSIGET error",
		"SCSISELECT error",
		"SCSICMD error",
		"SCSIREAD error",
		"SCSIMSG error",
		"can't read exec header",
		"bad exec header magic",
		"file truncated: ",
		"not enough memory for malloc zone",
		"resource error",
		"bad drive type in disklabel",
		"unknown filesystem type in partition",
		"unknown major device number",
		"Can not find Mach kernel: ",
		"can not open rom driver: ",
		"can not open ram driver: ",
		"can not reset port: ",
		"can not set handshake: ",
		"can not turn DTR off: ",
		"Localtalk is active on the printer port.",
		"No space for kernel.",
		"This version of Mach requires 32-bit mode.",
		"Mach can not run on this Macintosh.",
		""
	}
};

