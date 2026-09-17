/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: console.h,v $
 * Revision 1.1.2.3  1997/01/21  19:07:30  barbou
 * 	Added external declaration of osfmach3_video_physaddr.
 * 	[1997/01/21  18:39:19  barbou]
 *
 * Revision 1.1.2.2  1996/09/11  14:19:17  barbou
 * 	Added declaration of osfmach3_use_mach_console.
 * 	[96/09/11            barbou]
 * 
 * Revision 1.1.2.1  1996/09/09  16:57:14  barbou
 * 	Added declaration of osfmach3_video_memory_object.
 * 	[96/08/26            barbou]
 * 
 * 	Added definitions for cnputc() and co.
 * 	[96/08/26            barbou]
 * 
 * 	Fixed copyright marker.
 * 	[96/08/21            barbou]
 * 
 * 	Created.
 * 	[1996/08/09  13:59:21  barbou]
 * 
 * $EndLog$
 */

#ifndef	_OSFMACH3_CONSOLE_H
#define _OSFMACH3_CONSOLE_H

#include <mach/mach_types.h>

extern mach_port_t	osfmach3_console_port;
extern mach_port_t	osfmach3_keyboard_port;
extern mach_port_t	osfmach3_video_port;
extern vm_address_t	osfmach3_video_physaddr;
extern vm_address_t	osfmach3_video_map_base;
extern vm_size_t	osfmach3_video_map_size;
extern unsigned long	osfmach3_video_offset;
extern memory_object_t	osfmach3_video_memory_object;
extern int		osfmach3_use_mach_console;

extern boolean_t	osfmach3_con_probe(void);
extern unsigned long	osfmach3_con_init(unsigned long kmem_start);
extern void		osfmach3_launch_console_read_thread(void *tty_handle);

#define GETS_MAX 128	/* Max # of chars gets() will put in its buffer */
extern int		cnputc(int c);
extern void		cnflush(void);
extern int		cngetc(void);
extern char		*gets(char buf[GETS_MAX]);

#endif	/* _OSFMACH3_CONSOLE_H */
