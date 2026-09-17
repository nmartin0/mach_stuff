#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <features.h>
#include <sys/types.h>
#include <linux/mman.h>

#ifndef MAP_ANON
#define MAP_ANON	MAP_ANONYMOUS	   /* idem */
#endif
#define MAP_FILE	0x00	   /* The 'normal' way: mapped from file */

__BEGIN_DECLS

extern caddr_t mmap __P((caddr_t __addr, size_t __len,
		int __prot, int __flags, int __fd, off_t __off));
extern int munmap __P((caddr_t __addr, size_t __len));
extern int mprotect __P ((caddr_t __addr, size_t __len, int __prot));

extern int msync __P((caddr_t __addr, size_t __len, int __flags));

extern int mlock __P((caddr_t __addr, size_t __len));
extern int munlock __P((caddr_t __addr, size_t __len));

extern int mlockall __P((int __flags));
extern int munlockall __P((void));

__END_DECLS

#endif /* _SYS_MMAN_H */
