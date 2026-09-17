#ifndef _PLANB_H_
#define _PLANB_H_

#ifdef __KERNEL__

struct planb {
	struct	video_device video_dev;

	int	user;
	int lock;
	struct wait_queue *lockq;
};

#endif /* __KERNEL__ */

#endif /* _PLANB_H_ */
