/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


#include <linux/autoconf.h>

#include <linux/major.h>
#include <linux/tty.h>

#include <mach/mach_interface.h>

#include <osfmach3/server_thread.h>
#include <osfmach3/uniproc.h>
#include <osfmach3/mach_init.h>
#include <osfmach3/device_utils.h>
#include <osfmach3/console.h>
#include <osfmach3/parent_server.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/char_dev.h>
#include <linux/mm.h>
#include <linux/kernel.h>

#include <hp_pa/hilioctl.h>

/* Note: HIL_BUF_SIZE must be a power of 2 */
#define HIL_BUF_SIZE	2048
struct hil_queue_struct {
	unsigned long	head;
	unsigned long	tail;
	struct wait_queue	*proc_list;
	unsigned char		buf[HIL_BUF_SIZE];
};
struct hil_queue_struct hil_queue[16];

#define hil_queue_empty(hqueue) (hqueue->head == hqueue->tail)

unsigned int get_from_hil_queue(struct hil_queue_struct *hqueue)
{
	unsigned int result;

	result = hqueue->buf[hqueue->tail];
	hqueue->tail = (hqueue->tail + 1) & (HIL_BUF_SIZE-1);
	return result;
}

extern void chrdev_get_reference(kdev_t);
extern void chrdev_release_reference(kdev_t);

#define cdev_port_register(dev, port) \
	dev_to_object_register((dev), S_IFCHR, (char *) (port))
#define cdev_port_lookup(dev) \
	(mach_port_t) dev_to_object_lookup((dev), S_IFCHR)
#define cdev_port_deregister(dev) \
	dev_to_object_deregister((dev), S_IFCHR)

int hil_ioctl(
	struct inode	*inode,
	struct file	*file,
	unsigned int    cmd,
	unsigned long   arg)
{
	unsigned char describe[32];
	int size;
	kern_return_t	kr = 0;
	mach_port_t	device_port;

	device_port = cdev_port_lookup(inode->i_rdev);

	switch (cmd) {
	case HILID:
	case HILA1:
	case HILA2:
	case HILA3:
		size = 32 / sizeof(int);

		if(verify_area(VERIFY_WRITE, (long *) arg, 32)) {
			return -EIO;
		}

		kr = device_get_status(device_port,
				       cmd,
				       (dev_status_t) describe,
				       &size);
		if (kr != D_SUCCESS) {
			return -EIO;
		}
		
		memcpy_tofs((void*)arg, describe, 32);
		
		break;

	case HILER1:
	case HILER2:
		size = 1;

		kr = device_set_status(device_port,
				       cmd,
				       (dev_status_t) describe,
				       size);
		if (kr != D_SUCCESS) {
			return -EIO;
		}

		break;

	case EFTSBP:
		size = 1;
		
		if(verify_area(VERIFY_READ, (long *) arg, 4)) {
			return -EIO;
		}

		memcpy_fromfs(describe, (void *)arg, 4);
		
		kr = device_set_status(device_port,
				       cmd,
				       (dev_status_t) describe,
				       size);
		if (kr != D_SUCCESS) {
			return -EIO;
		}

		break;

	default:
		printk("hil_ioctl command 0x%x not implemented\n", cmd);

	}
	return 0;
}

void *
hil_input_thread(void *hil_handle)
{
	struct server_thread_priv_data	priv_data;
	kern_return_t			kr;
	io_buf_ptr_inband_t		inbuf;	/* 128 chars */
	mach_msg_type_number_t 		count;
	int				head, maxhead;
	int 				i;
	int dev = (int)hil_handle;
	int minor, major;
	mach_port_t	device_port;
	struct hil_queue_struct *hqueue;

	cthread_set_name(cthread_self(), "hil thread");
	server_thread_set_priv_data(cthread_self(), &priv_data);

	device_port = cdev_port_lookup(dev);
	major = MAJOR(dev);
	minor = MINOR(dev);
	hqueue = &hil_queue[minor];

	uniproc_enter();

	for (;;) {
		count = sizeof inbuf;
		server_thread_blocking(FALSE);
		kr = device_read_inband(device_port, 0, 0,
					sizeof inbuf, inbuf, &count);
		server_thread_unblocking(FALSE);
		if (kr != D_SUCCESS) {
			if (kr == D_DEVICE_DOWN ||
			    kr == MACH_SEND_INVALID_DEST ||
			    kr == MACH_SEND_INTERRUPTED ||
			    kr == MACH_RCV_INTERRUPTED) {
				/* device has been closed */
				uniproc_exit();
				cthread_detach(cthread_self());
				cthread_exit((void *) 0);
			}
			MACH3_DEBUG(0, kr,
				    ("hil_input_thread: device_read_inband"));
			panic("hil_input_thread: input error");
		}

		head = hqueue->head;
		maxhead = (hqueue->tail-1) & (HIL_BUF_SIZE-1);
		for (i = 0; i < count; i++) {
			hqueue->buf[head] = inbuf[i];
			if (head != maxhead) {
				head++;
				head &= HIL_BUF_SIZE-1;
			}
		}
		hqueue->head = head;
		wake_up_interruptible(&hqueue->proc_list);
	}
	/*NOTREACHED*/

}

int hil_open(
	struct inode	*inode,
	struct file	*file)
{
	int dev;
	unsigned int	major, minor;
	char		name[16];
	mach_port_t	device_port;
	kern_return_t kr;
	struct hil_queue_struct *hqueue;

	dev = inode->i_rdev;
	major = MAJOR(dev);
	minor = MINOR(dev);

	sprintf(name, "hil%d", minor);

	device_port = cdev_port_lookup(dev);
	if (device_port != MACH_PORT_NULL) {
		chrdev_get_reference(dev);
		return 0;
	}

	kr = device_open(device_server_port,
			 MACH_PORT_NULL,
			 D_READ|D_WRITE,
			 server_security_token, name, &device_port);
	if (kr != KERN_SUCCESS) {
		if (kr != D_NO_SUCH_DEVICE) {
			MACH3_DEBUG(1, kr,
				    ("hil_open(0x%x): "
				     "device_open(\"%s\")",
				     inode->i_rdev, name));
		}
		return -ENODEV;
	}
	
	chrdev_get_reference(dev);
	cdev_port_register(dev, device_port);

	hqueue = &hil_queue[minor];
	hqueue->head = hqueue->tail = 0;
	hqueue->proc_list = NULL;
	server_thread_start(hil_input_thread, (void *) dev);

	return 0;
}

void hil_close(
	struct inode	*inode,
	struct file	*file)
{
	if (cdev_port_lookup(inode->i_rdev) != MACH_PORT_NULL) 
		chrdev_release_reference(inode->i_rdev);
}

int hil_read(
	      struct inode *inode,
	      struct file *file,
	      char *buffer,
	      int count)
{
	struct hil_queue_struct *hqueue;
	mach_port_t	device_port;
	int dev;
	struct wait_queue wait = { current, NULL };
	int i = count;
	int minor, major;
	unsigned char c;

	dev = inode->i_rdev;
	device_port = cdev_port_lookup(dev);
	major = MAJOR(dev);
	minor = MINOR(dev);
	hqueue = &hil_queue[minor];

	if (hil_queue_empty(hqueue)) {
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		add_wait_queue(&hqueue->proc_list, &wait);
	    repeat:
		current->state = TASK_INTERRUPTIBLE;
		if (hil_queue_empty(hqueue) &&
		    !(current->signal & ~current->blocked)) {
			schedule();
			goto repeat;
		}
		current->state = TASK_RUNNING;
		remove_wait_queue(&hqueue->proc_list, &wait);
	}
	while (i > 0 && !hil_queue_empty(hqueue)) {
		c = get_from_hil_queue(hqueue);
		put_fs_byte(c, buffer++);
		i--;
	}
	if (count - i) {
		inode->i_atime = CURRENT_TIME;
		return count - i;
	}
	if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
	return 0;
}

int hil_select(struct inode *inode,
	       struct file *file,
	       int sel_type,
	       select_table *wait)
{
	struct hil_queue_struct *hqueue;
	mach_port_t	device_port;
	int dev;
	int major, minor;

	dev = inode->i_rdev;
	device_port = cdev_port_lookup(dev);
	major = MAJOR(dev);
	minor = MINOR(dev);
	hqueue = &hil_queue[minor];

	if (sel_type != SEL_IN)
		return 0;
	if(! hil_queue_empty(hqueue)) {
		return 1;
	}

	select_wait(&hqueue->proc_list, wait);
	return 0;
}

static struct file_operations hil_fops =
{
	NULL,		/* lseek */
	hil_read,	/* read */
	NULL,	        /* write */
	NULL,		/* readdir */
	hil_select,	/* select */
	hil_ioctl,      /* ioctl */
	NULL,           /* mmap */
	hil_open,       /* open */
	hil_close,	/* close */
};

int hil_init(void)
{
	if (register_chrdev(HIL_MAJOR, "rhil", &hil_fops)) {
		printk("hil: unable to get major %d\n", HIL_MAJOR);
		return -EIO;
	}
	return 0;
}

