/*
 *
 *
 *
 */

#define AWACS_DEBUG
#undef  AWACS_DEBUG

#include <linux/sched.h>
#include <linux/wait.h>
#include "awacs_types.h"

static unsigned int awacs_set_bits(awacs_devc_t *devc, unsigned int bits);
static int awacs_set_speed(awacs_devc_t *devc, int speed);
static int awacs_set_channels(awacs_devc_t *devc, int channels);
static dev_reply_t awacs_write_reply(awacs_devc_t *devc, kern_return_t rc, char *data, unsigned int count);
static dev_reply_t awacs_read_reply(awacs_devc_t *devc, kern_return_t rc, char *data, unsigned int count);
static int awacs_mach_open(awacs_devc_t *devc, int mode);
static void awacs_halt_xfer(awacs_devc_t *devc, int fOutput);

#define AWACS_BUFSIZE (64*1024)
static unsigned short unpacked_buffer[AWACS_BUFSIZE];
static int buf_index;
static int dev_open = FALSE;

extern void awacs_busy(int state);

#ifndef TRUE
#define FALSE  0
#define TRUE   1
#endif

#define SUPPORT_LEGACY_APPS

#ifdef SUPPORT_LEGACY_APPS
#define SOUND_MIXER_LEGACY_APPS SOUND_MIXER_PRIVATE1
static int awacs_legacy_apps = 0;
#endif

static __inline__ short
SWAP(short x)
{
	return ((x&0xFF00)>>8) | ((x&0x00FF)<<8);
}

static void __awacs_wait(awacs_devc_t *devc)
{
	struct wait_queue wait = { current, NULL };

	add_wait_queue(&devc->lockq, &wait);
repeat:
	current->state = TASK_UNINTERRUPTIBLE;
	if (devc->lock) {
		schedule();
		goto repeat;
	}
	remove_wait_queue(&devc->lockq, &wait);
}

static __inline__ void awacs_wait(awacs_devc_t *devc)
{
	if(devc->lock)
		__awacs_wait(devc);
}

static __inline__ void awacs_lock(awacs_devc_t *devc)
{
	if(devc->lock)
		__awacs_wait(devc);
	devc->lock = 1;
}

static __inline__ void awacs_unlock(awacs_devc_t *devc)
{
	devc->lock = 0;
	wake_up(&devc->lockq);
}

/*
 *
 *
 */
int 
awacs_audio_open(int dev, int mode)
{
	awacs_devc_t        *devc;

	if (dev_open) {
		return (-EBUSY);
	} 
	dev_open = TRUE;
	awacs_busy(TRUE);
	devc = awacs_dev_to_devc(dev);
#ifdef AWACS_DEBUG
	printk("awacs_audio_open(%s) - %s %d\n\r", devc->mach_dev_name,  __FILE__, __LINE__);
#endif
	if (!devc->device_port) {
		awacs_mach_open(devc, mode);
	}
      
#ifdef AWACS_DEBUG
	printk("awacs device_port = %08x - awacs reply port = %08x\n\r", devc->device_port, devc->reply_port); 
#endif
	buf_index = 0;
	return 1;
}
/*
 *
 *
 */
void  
awacs_audio_close(int dev)
{
	awacs_devc_t       *devc;
#ifdef AWACS_DEBUG
	printk("awacs_audio_close() - %s %d\n\r", __FILE__, __LINE__);
#endif

	awacs_busy(FALSE);
	devc = awacs_dev_to_devc(dev);
 
	device_close(devc->device_port);
	devc->device_port = 0;
	device_reply_deregister(devc->reply_port);
	devc->reply_port = 0;
	dev_open = FALSE;
}

/*
 *
 *
 */

caddr_t awacs_output_8(unsigned long buf, int channels, int *count, int mask)
{
	unsigned char *buf8;
	unsigned short *buf16;
	int i;

	buf8 = (unsigned char *)buf;
	if (channels == 1) {
		if ((buf_index + (*count*2)) >= AWACS_BUFSIZE) buf_index = 0;
		buf16 = (unsigned short *)&unpacked_buffer[buf_index];
		buf_index += *count*2;
		for (i = 0;  i < *count;  i++) {
			buf16[(2*i)+0] = buf16[(2*i)+1] = (buf8[i] ^ mask) << 8;
		}
		*count *= 4;
	} else {
		if ((buf_index + *count) >= AWACS_BUFSIZE) buf_index = 0;
		buf16 = (unsigned short *)&unpacked_buffer[buf_index];
		buf_index += *count;
		for (i = 0;  i < *count;  i++) {
			buf16[i] = (buf8[i] ^ mask) << 8;
		}
		*count *= 2;
	}
	return (caddr_t)buf16;
}

caddr_t awacs_output_w16(unsigned long buf, int channels, int *count, int mask)
{
	unsigned short *buf16, *orig_buf16;
	int i;

	if (channels == 1) {
		if ((buf_index + *count) >= AWACS_BUFSIZE) buf_index = 0;
		buf16 = (unsigned short *)&unpacked_buffer[buf_index];
		buf_index += *count;
		orig_buf16 = (unsigned short *)buf;
		for (i = 0;  i < *count;  i++)
			buf16[(2*i)+0] = buf16[(2*i)+1] = SWAP(orig_buf16[i] ^ mask);
		*count *= 2;
	} else {
		buf16 = (unsigned short *)buf;
		for (i = 0;  i < *count/2;  i++)
			buf16[i] = SWAP(buf16[i] ^ mask);
	}
	return (caddr_t)buf16;
}

caddr_t awacs_output_n16(unsigned long buf, int channels, int *count, int mask)
{
	unsigned short *buf16, *orig_buf16;
	int i;

	if (channels == 1) {
		if ((buf_index + *count) >= AWACS_BUFSIZE) buf_index = 0;
		buf16 = (unsigned short *)&unpacked_buffer[buf_index];
		buf_index += *count;
		orig_buf16 = (unsigned short *)buf;
		for (i = 0;  i < *count;  i++)
			buf16[(2*i)+0] = buf16[(2*i)+1] = orig_buf16[i] ^ mask;
		*count *= 2;
	} else {
		buf16 = (unsigned short *)buf;
		if (mask)
			for (i = 0;  i < *count/2;  i++)
				buf16[i] = buf16[i] ^ mask;
	}
	return (caddr_t)buf16;
}

void  
awacs_output_block(int dev, unsigned long buf, int count, int intrflag, int dma_restart)
{
	kern_return_t	kr = KERN_SUCCESS;
	awacs_devc_t	*devc;
	caddr_t		sound_buf = 0;

	devc = awacs_dev_to_devc(dev);

	awacs_lock(devc);
	if (devc->awacs_output) {
		sound_buf = devc->awacs_output(buf, devc->audio_channels,
					       &count, devc->audio_mask);
	} else {
		awacs_unlock(devc);
		return;
	}
	awacs_unlock(devc);

#ifdef AWACS_DEBUG  
	printk("awacs_output_block(%08x/%08x[%d],%d,%d) - %s %d\n\r",
	       (int) buf, (int)sound_buf, buf_index, count, intrflag, __FILE__, __LINE__);
#endif

	kr = serv_device_write_async((mach_port_t)            devc->device_port,
				     (mach_port_t)            devc->reply_port,
				     (dev_mode_t)             D_WRITE,
				     (recnum_t)               0,
				     (caddr_t)                sound_buf,
				     (mach_msg_type_number_t) count,
				     (boolean_t)              FALSE );


	if (kr != KERN_SUCCESS) {
		printk("serv_device_write_async() - rc = %08x - %s %d\n\r", kr, __FILE__, __LINE__);
	} else {
		devc->output_bufs_active++;
	}
}

/*
 *
 *
 */
static 
dev_reply_t awacs_write_reply(awacs_devc_t *devc, kern_return_t rc, char *data, unsigned int count)
{
	int               event;

#ifdef AWACS_DEBUG
	printk("awacs_write_reply(%08x,%d) - %s %d\n\r", (int) data, count, __FILE__, __LINE__); 
#endif
	if (devc->output_active) {
		event = (--devc->output_bufs_active) ? 0 : 1; 
		DMAbuf_outputintr(devc->dev, event);
	}
	return KERN_SUCCESS;
}

/*
 *
 *
 */
void 
awacs_input_block(int dev, unsigned long buf, int count, int intrflag, int dma_restart)
{
	kern_return_t                kr = KERN_SUCCESS;
	awacs_devc_t                 *devc;

	devc = awacs_dev_to_devc(dev);
#ifdef AWACS_DEBUG  
	printk("awacs_input_block(%08x,%d,%d) - %s %d\n\r",(int) buf, count, intrflag, __FILE__, __LINE__);
#endif
	kr = serv_device_read_async((mach_port_t)            devc->device_port,
				    (mach_port_t)            devc->reply_port,
				    (dev_mode_t)             D_READ,
				    (recnum_t)               0,
				    (caddr_t)                buf,
				    (mach_msg_type_number_t) count,
				    (boolean_t)              FALSE);

	if (kr != KERN_SUCCESS) {
		printk("serv_device_read_async() - rc = %08x - %s %d\n\r", kr, __FILE__, __LINE__);
	}
}

/*
 *
 *
 */
static 
dev_reply_t awacs_read_reply(awacs_devc_t *devc, kern_return_t rc, char *data, unsigned int count)
{
#ifdef AWACS_DEBUG
	printk("awacs_read_reply(%08x,%d) - %s %d\n\r", (int) data, (int) count, __FILE__, __LINE__); 
#endif 
	if (devc->input_active) {
		DMAbuf_inputintr(devc->dev); 
	}
	return KERN_SUCCESS;
}

/*
 *
 *
 */		 
int  awacs_audio_ioctl(int dev, unsigned int cmd, caddr_t arg, int local)
{
	awacs_devc_t        *devc;
	unsigned int        fmt;

	devc = awacs_dev_to_devc(dev);
#ifdef AWACS_DEBUG  
	printk("awacs_audio_ioctl(%d) - %s %d\n\r", cmd, __FILE__, __LINE__);
#endif  
	switch (cmd) {
	case SOUND_PCM_WRITE_RATE:
		if (local)
			return awacs_set_speed (devc, (int) arg);
		return snd_ioctl_return ((int *) arg, awacs_set_speed (devc, get_user ((int *) arg)));
		break;

	case SOUND_PCM_READ_RATE:
		if (local)
			return devc->audio_speed;
		return snd_ioctl_return ((int *) arg, devc->audio_speed);
		break;

	case SNDCTL_DSP_STEREO:
		if (local)
			return awacs_set_channels (devc, (int) arg + 1) - 1;
		return snd_ioctl_return ((int *) arg, awacs_set_channels (devc, get_user ((int *) arg) + 1) - 1);
		break;
 
	case SOUND_PCM_WRITE_CHANNELS:
		if (local)
			return awacs_set_channels (devc, (int) arg);
		return snd_ioctl_return ((int *) arg, awacs_set_channels (devc, get_user ((int *) arg)));
		break;

	case SOUND_PCM_READ_CHANNELS:
		if (local)
			return devc->audio_channels;
		return snd_ioctl_return ((int *) arg, devc->audio_channels);
		break;

	case SNDCTL_DSP_SETFMT:
		if (local)
			return awacs_set_bits (devc, (int) arg);
		return snd_ioctl_return ((int *) arg, awacs_set_bits (devc, get_user ((int *) arg)));
		break;

	case SNDCTL_DSP_GETFMTS:
		fmt = AFMT_U8 | AFMT_S16_BE | AFMT_S16_LE | AFMT_S8 | AFMT_U16_LE | AFMT_U16_BE;
		if (local)
			return fmt;
		return snd_ioctl_return ((int *) arg, fmt);

	case SOUND_PCM_READ_BITS:
		if (local)
			return devc->audio_fmt;
		return snd_ioctl_return ((int *) arg, devc->audio_fmt);

	case SOUND_PCM_WRITE_FILTER:	/* NOT POSSIBLE */
		return snd_ioctl_return ((int *) arg, -(EINVAL));
		break;

	case SOUND_PCM_READ_FILTER:
		return snd_ioctl_return ((int *) arg, -(EINVAL));
		break;

	}
	return -(EINVAL);

}

/*
 *
 *
 */
int   
awacs_mixer_ioctl(int dev, unsigned int cmd, caddr_t arg)
{
	awacs_devc_t        *devc;
	int                 count;
	int                 vol;
	int                 mask;
	int                 imask;
	int                 left;
	int                 right;
	dev_flavor_t        devcmd;
	kern_return_t       kr;
  
  
	devc = awacs_mixer_to_devc(dev);

	if (!devc->device_port) {
		awacs_mach_open(devc, D_READ | D_WRITE);
	}
#ifdef AWACS_DEBUG
	printk("awacs_mixer_ioctl() - port - %08x - %s %d\n\r", devc->device_port, __FILE__, __LINE__);
#endif
	if (_IOC_DIR(cmd) == _IOC_READ) {
		switch(cmd & 0xff) {
		case SOUND_MIXER_RECSRC:
			count = 1;
			kr = device_get_status(devc->device_port,
					       AWACS_DEVCMD_RECSRC,
					       &mask,
					       &count );
			if (kr != D_SUCCESS) {
				printk("awacs_mixer_ioctl() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
				return -EIO;
			}

			return snd_ioctl_return((int *)arg, mask);

		case SOUND_MIXER_DEVMASK:
			return snd_ioctl_return((int *)arg, (SOUND_MASK_VOLUME	|
							     SOUND_MASK_MIC	|
							     SOUND_MASK_CD	|
							     SOUND_MASK_SPEAKER	|
							     SOUND_MASK_LINE	|
							     SOUND_MASK_RECLEV	|
							     SOUND_MASK_ALTPCM	|
							     SOUND_MASK_IMIX	));
		case SOUND_MIXER_RECMASK:
			return snd_ioctl_return((int *)arg, (SOUND_MASK_MIC |
							     SOUND_MASK_CD  |
							     SOUND_MASK_LINE));

		case SOUND_MIXER_CAPS:
			return snd_ioctl_return((int *)arg, 0);

		case SOUND_MIXER_STEREODEVS:
			return snd_ioctl_return((int *)arg, (SOUND_MASK_VOLUME |
							     SOUND_MASK_SPEAKER|
							     SOUND_MASK_RECLEV ));
		case SOUND_MIXER_ALTPCM:
		case SOUND_MIXER_VOLUME:
			devcmd = AWACS_DEVCMD_OUTPUTVOL;
			goto     GetVol;

		case SOUND_MIXER_MIC:
			devcmd = AWACS_DEVCMD_MIC;
			goto     GetVol;

		case SOUND_MIXER_SPEAKER:
			devcmd = AWACS_DEVCMD_SPEAKER;
			goto     GetVol;

		case SOUND_MIXER_RECLEV:
			devcmd = AWACS_DEVCMD_INPUTVOL;

		GetVol:
			count  = 1;
			kr = device_get_status(devc->device_port,
					       devcmd,
					       &vol,
					       &count);

			if (kr != D_SUCCESS) {
				printk("awacs_mixer_ioctl() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
				return -EIO;
			}

			return snd_ioctl_return((int *)arg, vol);

		case SOUND_MIXER_IMIX:
			imask = SOUND_MASK_IMIX;
			goto     GetDev;

		case SOUND_MIXER_CD:
			imask = SOUND_MASK_CD;
			goto     GetDev;

		case SOUND_MIXER_LINE:
			imask = SOUND_MASK_LINE;

		GetDev:
			count  = 1;
			kr = device_get_status(devc->device_port,
					       AWACS_DEVCMD_DEV,
					       &mask,
					       &count);

			if (kr != D_SUCCESS) {
				printk("awacs_mixer_ioctl() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
			}

			vol = (mask & imask)? 100: 0;
			return snd_ioctl_return((int *)arg, vol);
		}
	} else {
		switch (cmd & 0xff) {
		case SOUND_MIXER_RECSRC:
			mask = get_user((int *) arg);
			mask &= (SOUND_MASK_MIC | SOUND_MASK_CD |
				 SOUND_MASK_LINE);

			if (mask == 0) {
				mask = SOUND_MASK_MIC;
			}
			kr = device_set_status(devc->device_port,
					       AWACS_DEVCMD_RECSRC,
					       &mask,
					       1     );

			if (kr != D_SUCCESS) {
				printk("awacs_mixer_ioctl() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
				return -EIO;
			}
			return snd_ioctl_return((int *)arg, mask);

		case SOUND_MIXER_CD:
			imask = SOUND_MASK_CD;
			goto     SetDev;

		case SOUND_MIXER_IMIX:
			imask = SOUND_MASK_IMIX;
			goto     SetDev;

		case SOUND_MIXER_LINE:
			imask = SOUND_MASK_LINE;

		SetDev:
			vol = get_user((int *) arg);
			vol &= 0xff;
			/* Use the MSB to indicate if the device should be
			   on or off. */
			if (vol >= 50)
				imask |= 1 << 31;

			kr = device_set_status(devc->device_port,
					       AWACS_DEVCMD_DEV,
					       &imask,
					       1    );

			if (kr != D_SUCCESS) {
				printk("awacs_mixer_ioctl() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
				return -EIO;
			}

			goto GetDev;

		case SOUND_MIXER_SPEAKER:
			devcmd = AWACS_DEVCMD_SPEAKER;
			goto     SetVol;

		case SOUND_MIXER_MIC:
			devcmd = AWACS_DEVCMD_MIC;
			goto     SetVol;

		case SOUND_MIXER_RECLEV:
			devcmd = AWACS_DEVCMD_INPUTVOL;
			goto     SetVol;
	  
		case SOUND_MIXER_ALTPCM:
		case SOUND_MIXER_VOLUME:
			devcmd = AWACS_DEVCMD_OUTPUTVOL;
			goto     SetVol;

		SetVol:
			vol = get_user((int *) arg);

			left  = vol & 0xff;         if (left  > 100) left  = 100;
			right = (vol >> 8) & 0xff;  if (right > 100) right = 100; 

			vol = left | (right << 8);

			kr = device_set_status(devc->device_port,
					       devcmd,
					       &vol,
					       1    );

			if (kr != D_SUCCESS) {
				printk("awacs_mixer_ioctl() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
				return -EIO;
			}

			goto GetVol;
		}
	}
#ifdef SUPPORT_LEGACY_APPS
	if (cmd == SOUND_MIXER_LEGACY_APPS) {
		awacs_legacy_apps = get_user((int *) arg);
		return snd_ioctl_return((int *)arg, awacs_legacy_apps);
	}
#endif
	return -(EINVAL);
}

/*
 *
 *
 */
static unsigned int 
awacs_set_bits(awacs_devc_t *devc, unsigned int fmt)
{
	kern_return_t         kr;
	int                   count;
	int                   byteswap = 0;
	int                   result = 0;
	int                   size = 8;
	unsigned int          mask = 0x8000;

	switch (fmt) {
	case AFMT_QUERY:
		goto out;
	case AFMT_S8:
		mask = 0;
		break;
	case AFMT_S16_LE:
		byteswap = 1;
		/* fall through */
	case AFMT_S16_BE:
		size = 16;
		mask = 0;
		break;
	case AFMT_U16_LE:
		byteswap = 1;
		mask = 0x80;
		/* fall through */
	case AFMT_U16_BE:
		size = 16;
		break;
	default:
		printk("awacs: unknown format 0x%X\n", fmt);
		fmt = AFMT_U8;
		/* fall through */
	case AFMT_U8:
		mask = 0x80;
		break;
	}
#ifdef SUPPORT_LEGACY_APPS
	if ((size == 16) && awacs_legacy_apps)
		byteswap ^= 1;
#endif
	kr = device_set_status(devc->device_port,
			       AWACS_DEVCMD_SWAP,
			       &byteswap,
			       1        );

	if (kr != D_SUCCESS) {
		printk("awacs_set_bits() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__); 
		goto out;
	} 
	count = 1;
	kr = device_get_status(devc->device_port,
			       AWACS_DEVCMD_SWAP,
			       &result,
			       &count);
	if (kr != D_SUCCESS) {
		printk("awacs_set_bits() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__); 
	}
	awacs_lock(devc);
	if (size == 16) {
		if ((byteswap) && (result != byteswap))
			devc->awacs_output = awacs_output_w16;
		else
			devc->awacs_output = awacs_output_n16;
	} else
		devc->awacs_output = awacs_output_8;
	devc->audio_size = size;
	devc->audio_mask = mask;
	devc->audio_fmt = fmt;
	awacs_unlock(devc);
out:
	return devc->audio_fmt;
}

/*
 *
 *
 */
static int 
awacs_set_speed(awacs_devc_t *devc, int speed)
{
	kern_return_t         kr;
	int                   count;

	if (speed != -1) {

		kr = device_set_status(devc->device_port,
				       AWACS_DEVCMD_RATE,
				       &speed,
				       1        );

		if (kr != D_SUCCESS) {
			printk("awacs_set_speed() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
			goto out;
		}
	}
 
	count = 1;
	kr = device_get_status(devc->device_port,
			       AWACS_DEVCMD_RATE,
			       &speed,
			       &count);
	if (kr != D_SUCCESS) {
		printk("awacs_set_speed() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__); 
	}

	awacs_lock(devc);
	devc->audio_speed = speed;
	awacs_unlock(devc);
out:
	return devc->audio_speed;
}

/*
 *
 *
 */
static int 
awacs_set_channels(awacs_devc_t *devc, int channels)
{
	if (channels != 0) {
		awacs_lock(devc);
		devc->audio_channels = channels;
		awacs_unlock(devc);
	}
	return devc->audio_channels;
}

/*
 *
 *
 */
int 
awacs_audio_prepare_for_input(int dev, int bufsize, int nbufs)
{
	awacs_devc_t                 *devc;

#ifdef AWACS_DEBUG 
	printk("awacs_audio_prepare_for_input(%d,%d) - %s %d\n\r", bufsize, nbufs, __FILE__, __LINE__);
#endif

	devc = awacs_dev_to_devc(dev);
	devc->input_active = 1;
	return 1;
}

/*
 *
 *
 */
int 
awacs_audio_prepare_for_output(int dev, int bufsize, int nbufs)
{
	awacs_devc_t                 *devc;

#ifdef AWACS_DEBUG
	printk("awacs_audio_prepare_for_output(%d,%d) - %s %d\n\r", bufsize, nbufs,__FILE__, __LINE__);
#endif

	devc = awacs_dev_to_devc(dev);
	devc->output_active = 1;
	return 1;
}

/*
 *
 *
 */
void 
awacs_audio_reset(int dev)
{
	awacs_devc_t                 *devc;

#ifdef AWACS_DEBUG
	printk("awacs_audio_reset() - %s %d\n\r", __FILE__, __LINE__);
#endif

	devc = awacs_dev_to_devc(dev);
	devc->input_active = devc->output_active = 0;
	awacs_halt_xfer(devc, 0);
	awacs_halt_xfer(devc, 1);
}

/*
 *
 *
 *
 */
void 
awacs_audio_halt_input(int dev)
{
	awacs_devc_t                 *devc;

#ifdef AWACS_DEBUG
	printk("awacs_audio_halt_input() - %s %d\n\r", __FILE__, __LINE__);
#endif

	devc = awacs_dev_to_devc(dev);
	devc->input_active = 0;
	awacs_halt_xfer(devc, 0);
}

/*
 *
 *
 *
 */
void 
awacs_audio_halt_output(int dev)
{
	awacs_devc_t                 *devc;

#ifdef AWACS_DEBUG
	printk("awacs_audio_halt_output() - %s %d\n\r", __FILE__, __LINE__);
#endif

	devc = awacs_dev_to_devc(dev);
	devc->output_active = 0;
	awacs_halt_xfer(devc, 0);
}



/*
 *
 *
 */
static 
void awacs_halt_xfer(awacs_devc_t *devc, int fOutput)
{
	kern_return_t       kr;

	kr = device_set_status(devc->device_port,
			       AWACS_DEVCMD_HALT,
			       &fOutput,
			       1        );
	if (kr != D_SUCCESS) {
		printk("awacs_halt_xfer() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__); 
	}
}

/*
 *
 *
 */
void 
awacs_audio_trigger(int dev, int bits) 
{
#ifdef AWACS_DEBUG
  printk("awacs_audio_trigger(%d) - %s %d\n\r", bits, __FILE__, __LINE__);
#endif
}

/*
 *
 *
 */
int  
awacs_audio_set_speed(int dev, int speed)
{
	awacs_devc_t          *devc;

#ifdef AWACS_DEBUG
	printk("awacs_audio_set_speed(%d) - %s %d\n\r", speed, __FILE__, __LINE__);
#endif

	devc = awacs_dev_to_devc(dev);

	if (speed == -1) {
		return devc->audio_speed;
	}

	return awacs_set_speed(devc, speed);
  
}

/*
 *
 *
 */
unsigned int 
awacs_audio_set_bits(int dev, unsigned int bits)
{
	awacs_devc_t          *devc;
#ifdef AWACS_DEBUG
	printk("awacs_audio_set_bits(%d) - %s %d\n\r", bits, __FILE__, __LINE__);
#endif
	devc = awacs_dev_to_devc(dev);
	return awacs_set_bits(devc, bits);
}

/*
 *
 *
 */
short 
awacs_audio_set_channels(int dev, short channels)
{
	awacs_devc_t          *devc;
#ifdef AWACS_DEBUG
	printk("awacs_set_channels(%d) - %s %d\n\r", channels,__FILE__, __LINE__);
#endif
	devc = awacs_dev_to_devc(dev);
	return awacs_set_channels(devc, channels);
}


static int 
awacs_mach_open(awacs_devc_t *devc, int mode)
{
	kern_return_t          kr;

	kr = device_open(device_server_port,
			 MACH_PORT_NULL,
			 D_READ | D_WRITE,
			 server_security_token,
			 devc->mach_dev_name,
			 &devc->device_port);

	if (kr != KERN_SUCCESS) {
		printk("awacs_mach_open() - rc = %d - %s %d\n\r", kr, __FILE__, __LINE__);
	}
  
	device_reply_register(&devc->reply_port,
			      (char *) devc, 
			      (dev_reply_t) awacs_read_reply,
			      (dev_reply_t) awacs_write_reply);
 
	return kr;
} 


