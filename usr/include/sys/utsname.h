#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#include <features.h>
#include <sys/param.h>

#ifdef __SVR4_I386_ABI_L1__

#define SYS_NMLN 257

struct utsname {
	char sysname[SYS_NMLN];
	char nodename[SYS_NMLN];
	char release[SYS_NMLN];
	char version[SYS_NMLN];
	char machine[SYS_NMLN-65];
	char domainname[65];	/* Kludge to allow linux domain name to work
				   with SVR4 ABI. */
};

#else /* !__SVR4_I386_ABI_L1__ */

struct utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};

#endif /* !__SVR4_I386_ABI_L1__ */


__BEGIN_DECLS

extern int uname __P ((struct utsname * __utsbuf));
extern int __uname __P ((struct utsname * __utsbuf));

__END_DECLS

#endif
