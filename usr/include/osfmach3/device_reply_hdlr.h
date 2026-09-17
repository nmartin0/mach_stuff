/*
 * @OSF_FREE_FREE_COPYRIGHT@
 * 
 */
/*
 * HISTORY
 * $Log: device_reply_hdlr.h,v $
 * Revision 1.1.2.1  1996/09/09  16:57:18  barbou
 * 	Created.
 * 	[1996/08/22  16:34:41  barbou]
 *
 * $EndLog$
 */

/*
 * Handler for device read and write replies.
 */
#ifndef	_OSFMACH3_DEVICE_REPLY_HDLR_H_
#define	_OSFMACH3_DEVICE_REPLY_HDLR_H_

#define	DEV_REPLY_PORT_ALIAS	0
#define	DEV_REPLY_MESSAGE	0

#include <mach/kern_return.h>
#include <device/device.h>

#if	! DEV_REPLY_MESSAGE
extern mach_port_t	device_reply_queue;
#endif

typedef kern_return_t	(*dev_reply_t)(char *object,
				       kern_return_t return_code,
				       char *data,
				       unsigned int data_count);

enum dev_reply_select {
	DR_READ,
	DR_WRITE
};
typedef enum dev_reply_select	dr_select_t;

extern void		device_reply_register(mach_port_t *portp,
					      char *object,
					      dev_reply_t read_reply,
					      dev_reply_t write_reply);
extern boolean_t	device_reply_lookup(mach_port_t port,
					    dr_select_t which,
					    char **object,
					    dev_reply_t *func);
extern void		device_reply_deregister(mach_port_t port);

extern void		device_reply_hdlr(void);

#endif	/* _OSFMACH3_DEVICE_REPLY_HDLR_H_ */
