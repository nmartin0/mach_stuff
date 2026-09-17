type 'Mach' as 'STR ';

type 'mach' {
	unsigned hex integer;
	unsigned hex integer;
};

resource 'Mach' (0, purgeable) {
	"Mach Operating System"
};

resource 'BNDL' (128, purgeable) {
	'Mach',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'ICN#',
		{	/* array IDArray: 2 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		},
		/* [2] */
		'FREF',
		{	/* array IDArray: 2 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		}
	}
};

resource 'FREF' (128, purgeable) {
	'cdev',
	0,
	""
};

resource 'FREF' (129, purgeable) {
	'krnl',
	0,
	""
};

resource 'mach' (-4064, purgeable) {
	0xFFFF,
	0
};

resource 'nrct' (-4064, purgeable) {
	{	/* array RectArray: 1 elements */
		/* [1] */
		{-1, 87, 255, 322}
	}
};
