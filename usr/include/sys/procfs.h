#ifndef _SYS_PROCFS_H
#define _SYS_PROCFS_H

/* Are we included by gdb/bfd and using the right compiler? */
#if (defined(START_INFERIOR_TRAPS_EXPECTED) || \
	defined(TRAD_CORE_EXTRA_SIZE_ALLOWED)) && \
	defined(__ELF__) && defined(__linux__)

/* We test if we have the ELF patches installed. */
#include <linux/sys.h>

#ifdef sys_quotactl

/* This is strictly for gdb/bfd only. */
#define HAVE_PROCFS

/* We just need it for gdb/bfd. */
#include <linux/elfcore.h>

#else /* sys_quotactl */

/* Oops. Just silence it. */
#undef HAVE_PROCFS

#endif /* sys_quotactl */

/* Don't pollute. */
#undef sys_quotactl

#endif /* __ELF__ && __linux__ */

#endif /* _SYS_PROCFS_H */
