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