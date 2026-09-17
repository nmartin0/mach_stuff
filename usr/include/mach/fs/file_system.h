/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: file_system.h,v $
 * Revision 1.1.16.1  1996/09/17  17:31:08  bruel
 * 	use standalone includes only
 * 	[1996/09/17  14:31:14  bruel]
 *
 * Revision 1.1.3.3  1996/07/31  06:37:07  paire
 * 	Merged with nmk20b7_shared (1.1.14.1)
 * 	[96/05/31            paire]
 * 
 * Revision 1.1.14.1  1996/04/12  06:43:16  paire
 * 	Modified typedef of fs_private_t from char * to void *.
 * 	Moved global definitions into externs.h.
 * 	[96/01/30            paire]
 * 
 * Revision 1.1.3.2  1995/11/02  14:27:37  bruel
 * 	Added device structure to include
 * 	device_port and record_size.
 * 	[95/09/07            bernadat]
 * 	[95/09/29            bruel]
 * 
 * Revision 1.1.3.1  1995/08/21  19:24:40  devrcs
 * 	Initial revision
 * 	[95/03/21            bernadat]
 * 
 * $EndLog$
 */

/*
 * File system independent interfaces definitions
 */

#ifndef _FS_FILE_SYSTEM_H_
#define _FS_FILE_SYSTEM_H_

#include <string.h>
#include <mach.h>
#include <device/device_types.h>
#include <mach/boot_info.h>

typedef struct fs_ops 	*fs_ops_t;
typedef void 		*fs_private_t;

struct device {
	mach_port_t	dev_port;	/* port to device */
	unsigned int	rec_size;	/* record size */
};

/*
 * In-core open file.
 */
struct file {
	fs_ops_t	f_ops;
	fs_private_t	f_private;	/* file system dependent */
	struct device	f_dev;		/* device */
};

struct fs_ops {
	int		(*open_file)(struct device *,
				     const char *,
				     fs_private_t *);
	void		(*close_file)(fs_private_t);
    	int 		(*read_file)(fs_private_t,
				     vm_offset_t,
				     vm_offset_t,
				     vm_size_t);
    	size_t 		(*file_size)(fs_private_t);
	boolean_t 	(*file_is_directory)(fs_private_t);
	boolean_t 	(*file_is_executable)(fs_private_t);
};

/*
 * Error codes for file system errors.
 */
#define	FS_NOT_DIRECTORY	5000	/* not a directory */
#define	FS_NO_ENTRY		5001	/* name not found */
#define	FS_NAME_TOO_LONG	5002	/* name too long */
#define	FS_SYMLINK_LOOP		5003	/* symbolic link loop */
#define	FS_INVALID_FS		5004	/* bad file system */
#define	FS_NOT_IN_FILE		5005	/* offset not in file */
#define	FS_INVALID_PARAMETER	5006	/* bad parameter to a routine */

extern int open_file(mach_port_t, const char *, struct file *);
extern void close_file(struct file *);
extern int read_file(struct file *, vm_offset_t, vm_offset_t, vm_size_t);
extern size_t file_size(struct file *);
extern boolean_t file_is_directory(struct file *);
extern boolean_t file_is_executable(struct file *);

#endif /* _FS_FILE_SYSTEM_H_ */
