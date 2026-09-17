type 'conf' {
	integer; /* LoadDev1 */
	integer; /* RootDev1 */
	integer; /* LoadDev2 */
	integer; /* RootDev2 */
	integer; /* TimeOffset */
	integer; /* Flags */
	longint; /* KernelSpace */
	cstring[256];
	cstring[256];
};

resource 'conf' (128) {
	1,
	1,
	9,
	8,
	13,
	0,
	0x00010000,
	"/mach_kernel",
	":Control Panels:×Mach:"
};
