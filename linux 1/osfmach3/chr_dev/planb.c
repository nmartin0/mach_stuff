#include <linux/config.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/malloc.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/wrapper.h>
#include <linux/tqueue.h>
#include <linux/videodev.h>

#ifdef	CONFIG_OSFMACH3
#include <osfmach3/server_thread.h>
#include <osfmach3/uniproc.h>
#include <osfmach3/device_utils.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/mach_init.h>
#include <ppc/planb_io.h>
#endif

#include "planb.h"

//#define DEBUG(x...) printk(KERN_DEBUG ## x)  /* Debug driver */	
#define DEBUG(x...) 		/* Don't debug driver */	
//#define IDEBUG(x...) printk(KERN_DEBUG ## x) /* Debug interrupt related part */
#define IDEBUG(x...) 		/* Don't debug interrupt related part */

#if LINUX_VERSION_CODE < 0x020100
#define mdelay(x) udelay((x)*1000)
#define signal_pending(current)  (current->signal & ~current->blocked)
#define sigfillset(set)

static inline int time_before(unsigned long a, unsigned long b)
{
	return((long)((a) - (b)) < 0L);
}

static inline unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
        memcpy_tofs(to,from,n);
	return 0;
}

static inline unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
        memcpy_fromfs(to,from,n);
	return 0;
}
#endif

#ifdef CONFIG_OSFMACH3
static mach_port_t	planb_port = MACH_PORT_NULL;
#endif

static char		*mach_planb_name = "planb0";

static struct planb planbs;

/* ------------------ PlanB Exported Functions ------------------ */
static long planb_write(struct video_device *, const char *, unsigned long, int);
static long planb_read(struct video_device *, char *, unsigned long, int);
static int planb_open(struct video_device *, int);
static void planb_close(struct video_device *);
static int planb_ioctl(struct video_device *, unsigned int, void *);
static int planb_init_done(struct video_device *);
static int planb_mmap(struct video_device *, const char *, unsigned long);
int init_planbs(struct video_init *);

/* ------------------ PlanB Internal Functions ------------------ */
static void release_planb(void);
static int init_planb(struct planb *);

/******************************/
/* misc. supporting functions */
/******************************/

static void __planb_wait(struct planb *pb) {
	struct wait_queue wait = { current, NULL };

	add_wait_queue(&pb->lockq, &wait);
repeat:
	current->state = TASK_UNINTERRUPTIBLE;
	if (pb->lock) {
		schedule();
		goto repeat;
	}
	remove_wait_queue(&pb->lockq, &wait);
	current->state = TASK_RUNNING;
}

static inline void planb_wait(struct planb *pb) {
	DEBUG("PlanB: planb_wait\n");
	if(pb->lock)
		__planb_wait(pb);
}

static inline void planb_lock(struct planb *pb) {
	DEBUG("PlanB: planb_lock\n");
	if(pb->lock)
		__planb_wait(pb);
	pb->lock = 1;
}

static inline void planb_unlock(struct planb *pb) {
	DEBUG("PlanB: planb_unlock\n");
	pb->lock = 0;
	wake_up(&pb->lockq);
}

/*******************************
 * Device Operations functions *
 *******************************/

static int planb_open(struct video_device *dev, int mode)
{
	struct planb *pb = (struct planb *)dev;
	int count, dummy;
	kern_return_t kr;

#ifdef CONFIG_OSFMACH3
	if(pb->user == 0) {
		DEBUG("PlanB: IOCTL PLANB_IOC_OPEN\n");

		ASSERT(planb_port == MACH_PORT_NULL);
		count = 1;
		kr = device_get_status(planb_port, PLANB_IOC_OPEN, &dummy,
									&count);
		if (kr != D_SUCCESS) {
			DEBUG("PlanB: Mach side failed with %d\n", kr);
			if (kr == D_NO_MEMORY)
				return -ENOMEM;
			return	-EIO;
		}
	}
#endif
	pb->user++;

	DEBUG("PlanB: device opened\n");

	MOD_INC_USE_COUNT;
	return 0;   
}

static void planb_close(struct video_device *dev)
{
	struct planb *pb = (struct planb *)dev;
	int count, dummy;
	kern_return_t kr;

	if(pb->user < 1) /* ??? */
		return;
	planb_lock(pb);
#ifdef CONFIG_OSFMACH3
	if(pb->user == 1) {
		DEBUG("PlanB: IOCTL PLANB_IOC_CLOSE\n");

		count = sizeof(int) / sizeof(int);
		kr = device_get_status(planb_port, PLANB_IOC_CLOSE, &dummy,
									&count);
		if(kr != D_SUCCESS)
			printk(KERN_WARNING "PlanB: failed to close\n");
	}
#endif
	pb->user--;
	planb_unlock(pb);

	DEBUG("PlanB: device closed\n");

	MOD_DEC_USE_COUNT;  
}

static long planb_read(struct video_device *v, char *buf, unsigned long count,
				int nonblock)
{
	DEBUG("planb: read request\n");
	return -EINVAL;
}

static long planb_write(struct video_device *v, const char *buf,
				unsigned long count, int nonblock)
{
	DEBUG("planb: write request\n");
	return -EINVAL;
}

static int planb_ioctl(struct video_device *dev, unsigned int cmd, void *arg)
{
	struct planb *pb=(struct planb *)dev;
	int count;
	kern_return_t kr;
	int d_get = 0, d_set = 0;
	int *var, var_size = 0;

	switch (cmd)
	{
#ifdef CONFIG_OSFMACH3
		case VIDIOCGCAP:
		{
			DEBUG("PlanB: IOCTL VIDIOCGCAP\n");
			var_size = sizeof(struct video_capability);
			d_get = 1;
			break;
		}
		case VIDIOCSFBUF:
		{
			DEBUG("PlanB: IOCTL VIDIOCSFBUF\n");
#if LINUX_VERSION_CODE >= 0x020100
			if(!capable(CAP_SYS_ADMIN))
#else
			if(!suser())
#endif
				return -EPERM;
			var_size = sizeof(struct video_buffer);
			d_set = 1;
			break;
		}
		case VIDIOCGFBUF:
		{
			DEBUG("PlanB: IOCTL VIDIOCGFBUF\n");
			var_size = sizeof(struct video_buffer);
			d_get = 1;
			break;
		}
		case VIDIOCCAPTURE:
		{
			var_size = sizeof(int);
			d_set = 1;
			break;
		}
		case VIDIOCGCHAN:
		{
			DEBUG("PlanB: IOCTL VIDIOCGCHAN\n");
			var_size = sizeof(struct video_channel);
			d_set = 1;
			d_get = 1;
			break;
		}
		case VIDIOCSCHAN:
		{
			DEBUG("PlanB: IOCTL VIDIOCSCHAN\n");
			var_size = sizeof(struct video_channel);
			d_set = 1;
			break;
		}
		case VIDIOCGPICT:
		{
			DEBUG("PlanB: IOCTL VIDIOCGPICT\n");
			var_size = sizeof(struct video_picture);
			d_get = 1;
			break;
		}
		case VIDIOCSPICT:
		{
			DEBUG("PlanB: IOCTL VIDIOCSPICT\n");
			var_size = sizeof(struct video_picture);
			d_set = 1;
			break;
		}
		case VIDIOCSWIN:
		{
			struct video_window	vw;
			unsigned char		*cptr;
			int 			i;

			DEBUG("PlanB: IOCTL VIDIOCSWIN\n");

			if(copy_from_user(&vw,arg,sizeof(vw)))
				return -EFAULT;

			planb_lock(pb);
			count = sizeof(vw);
			if(vw.clipcount > 0) {
				count += sizeof(struct video_clip)*vw.clipcount;
				cptr = (unsigned char *)kmalloc(sizeof(vw)
					+vw.clipcount*sizeof(struct video_clip),
								GFP_KERNEL);
			} else {
				cptr = (unsigned char *)kmalloc(sizeof(vw),
								GFP_KERNEL);
			}
			if(!(cptr)) {
				planb_unlock(pb);
				return -ENOMEM;
			}
			memcpy((void*)cptr, &vw, sizeof(vw));
			for(i = 0; i < vw.clipcount; i++) {
				copy_from_user((void*)cptr + sizeof(vw)
					+ sizeof(struct video_clip) * i,
					vw.clips + i,sizeof(struct video_clip));
			}
			if(!(count)) {
				kfree(cptr);
				planb_unlock(pb);
				return -EINVAL;
			}
			count /= sizeof(int);
			kr = device_set_status(planb_port, VIDIOCSWIN,
							(int *)cptr, count);
			if (kr != D_SUCCESS) {
				kfree(cptr);
				planb_unlock(pb);
				return	-EIO;
			}
			kfree(cptr);
			planb_unlock(pb);
			return 0;
		}
		case VIDIOCGWIN:
		{
			DEBUG("PlanB: IOCTL VIDIOCGWIN\n");
			var_size = sizeof(struct video_window);
			d_get = 1;
			break;
		}
		case PLANBIOCGSAAREGS:
		{
			DEBUG("PlanB: IOCTL PLANBIOCGSAAREGS\n");
			var_size = sizeof(struct planb_saa_regs);
			d_set = 1;
			d_get = 1;
			break;
		}
		case PLANBIOCSSAAREGS:
		{
			DEBUG("PlanB: IOCTL PLANBIOCSSAAREGS\n");
			var_size = sizeof(struct planb_saa_regs);
			d_set = 1;
			break;
		}
		case PLANBIOCGSTAT:
		{
			DEBUG("PlanB: IOCTL PLANBIOCGSTAT\n");
			var_size = sizeof(struct planb_stat_regs);
			d_get = 1;
			break;
		}
		case PLANBIOCSMODE:
		{
			DEBUG("PlanB: IOCTL PLANBIOCSMODE\n");
			var_size = sizeof(int);
			d_set = 1;
			break;
		}
		case PLANBIOCGMODE:
		{
			DEBUG("PlanB: IOCTL PLANBIOCGMODE\n");
			var_size = sizeof(int);
			d_get = 1;
			break;
		}
#endif /* CONFIG_OSFMACH3 */
		default:
		{
			DEBUG("PlanB: Unimplemented IOCTL\n");
			return -ENOIOCTLCMD;
		}
	}
#ifdef CONFIG_OSFMACH3
	if(d_set) {
		var = (int *)kmalloc(var_size, GFP_KERNEL);
		if(!(var)) {
			return -ENOMEM;
		}
		if (copy_from_user(var, arg, var_size))
			return -EFAULT;
		count = var_size / sizeof(int);
		planb_lock(pb);
		kr = device_set_status(planb_port, cmd, var, count);
		if (kr != D_SUCCESS) {
			planb_unlock(pb);
			kfree(var);
			return	-EIO;
		}
		planb_unlock(pb);
		kfree(var);
	}
	if(d_get) {
		var = (int *)kmalloc(var_size, GFP_KERNEL);
		if(!(var)) {
			return -ENOMEM;
		}
		count = var_size / sizeof(int);
		kr = device_get_status(planb_port, cmd, var, &count);
		if (kr != D_SUCCESS) {
			DEBUG("PlanB: device_get_status() failed: %d\n", kr);
			kfree(var);
			return	-EIO;
		}
		if (copy_to_user(arg,(void*)var,var_size)) {
			kfree(var);
			return -EFAULT;
		}
		kfree(var);
	}
#endif /* CONFIG_OSFMACH3 */
	return 0;
}

static int planb_mmap(struct video_device *dev, const char *adr,
							unsigned long size)
{
        return -EINVAL;
}

/* This gets called upon device registration */
/* we could do some init here */
static int planb_init_done(struct video_device *dev)
{
	return 0;
}

static struct video_device planb_template=
{
	"",
	0,
	0,
	planb_open,
	planb_close,
	planb_read,
	planb_write,
#if LINUX_VERSION_CODE >= 0x020100
	NULL,	/* poll */
#endif
	planb_ioctl,
	planb_mmap,	/* mmap? */
	planb_init_done,
	NULL,	/* pointer to private data */
	0,
	0
};

static int init_planb(struct planb *pb)
{
	struct planb_info info;
	int count;
	kern_return_t	kr;

#ifdef	CONFIG_OSFMACH3
	ASSERT(planb_port == MACH_PORT_NULL);

	kr = device_open(device_server_port,
			 MACH_PORT_NULL,
			 D_READ | D_WRITE,
			 server_security_token,
			 mach_planb_name,
			 &planb_port);

	if (kr != D_SUCCESS) {
		if (kr != D_NO_SUCH_DEVICE) {
			MACH3_DEBUG(1, kr, ("init_planb(): "
						"device_open(\"planb0\")"));
		}
		return -ENODEV;
	}
#endif

	/* Now add the template and register the device unit. */
	memcpy(&pb->video_dev,&planb_template,sizeof(planb_template));

#ifdef CONFIG_OSFMACH3

	DEBUG("PlanB: IOCTL PLANB_IOC_QUERY\n");

	count = sizeof(struct planb_info) / sizeof(int);
	kr = device_get_status(planb_port, PLANB_IOC_QUERY,
						(int *)&info, (int *)&count);
	if (kr != D_SUCCESS)
		return	-EIO;
	strncpy(pb->video_dev.name, info.name, 32);
	pb->video_dev.type = info.vid_type;
	pb->video_dev.hardware = info.hardware;
	DEBUG("PlanB: device %s opened\n", info.name);
#endif
	if(video_register_device(&pb->video_dev, info.vfl_type)<0)
		return 1;

	return 0;
}

static void release_planb(void)
{
	struct planb *pb;
	kern_return_t kr;

	pb=&planbs;

#ifdef	CONFIG_OSFMACH3
	kr = device_close(planb_port);
	if (kr != D_SUCCESS) {
		MACH3_DEBUG(1, kr, ("release_planb(): device_close(0x%x)",
								planb_port));
		panic("release_planb: device_close failed");
	}
	kr = mach_port_destroy(mach_task_self(), planb_port);
	if (kr != D_SUCCESS) {
		MACH3_DEBUG(1, kr, ("release_planb(): "
					"mach_port_destroy(0x%x)", planb_port));
		panic("release_planb: mach_port_destroy failed");
	}
	planb_port = MACH_PORT_NULL;
#endif
}

#ifdef MODULE

int init_module(void)
{
#else
int init_planbs(struct video_init *unused)
{
#endif
	int err;

	if ((err = init_planb(&planbs)) != 0) {
		switch (err) {
		case 1:
			printk(KERN_ERR "PlanB: error registering device"
							" with v4l\n");
			err = -EIO;
			/* fall through */
		case -EIO:
			release_planb();
			break;
		}
		return err;
	}
	printk(KERN_INFO "PlanB: device registered with v4l\n");

	return 0;
}

#ifdef MODULE

void cleanup_module(void)
{
	video_unregister_device(&planbs[0].video_dev);
	release_planb();
}

#endif
