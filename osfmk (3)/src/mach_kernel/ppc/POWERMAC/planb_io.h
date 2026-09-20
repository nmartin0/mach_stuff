#ifndef _PLANB_IO_H_
#define _PLANB_IO_H_

#ifdef MACH_KERNEL
#include <ppc/POWERMAC/videodev.h>
#else
#include <linux/videodev.h>
#endif

struct planb_info {
	char name[32];
	int vfl_type;
	int vid_type;
	int hardware;
};

/* more or less generic ioctls */
	/* Get misc infos (name, type, hardware) */
#define PLANB_IOC_QUERY	_IOR('v',BASE_VIDIOCPRIVATE + 16,struct planb_info)
	/* Be ready */
#define PLANB_IOC_OPEN	_IOR('v',BASE_VIDIOCPRIVATE + 17,int)
	/* Go idle */
#define PLANB_IOC_CLOSE	_IOR('v',BASE_VIDIOCPRIVATE + 18,int)

struct planb_saa_regs {
	unsigned char addr;
	unsigned char val;
};

struct planb_stat_regs {
	unsigned int ch1_stat;
	unsigned int ch2_stat;
	unsigned char saa_stat0;
	unsigned char saa_stat1;
};

/* private ioctls (hmmm, what to do....) */
	/* Read a saa7196 reg value */
#define PLANBIOCGSAAREGS _IOWR('v', BASE_VIDIOCPRIVATE, struct planb_saa_regs)
	/* Set a saa7196 reg value */
#define PLANBIOCSSAAREGS _IOW('v', BASE_VIDIOCPRIVATE + 1, struct planb_saa_regs)
	/* Read planb status */
#define PLANBIOCGSTAT	_IOR('v', BASE_VIDIOCPRIVATE + 2, struct planb_stat_regs)
#define PLANB_TV_MODE		1
#define PLANB_VTR_MODE		2
	/* Get TV/VTR mode */
#define PLANBIOCGMODE	_IOR('v', BASE_VIDIOCPRIVATE + 3, int)
	/* Set TV/VTR mode */
#define PLANBIOCSMODE	_IOW('v', BASE_VIDIOCPRIVATE + 4, int)


#endif /* _PLANB_IO_H_ */

