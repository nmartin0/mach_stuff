/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/* Macintosh Video driver for MACH 3.0
 *
 * Authors: David E. Bohman II (CMU macmach)
 *          Zonnie L. Williamson (CMU macmach)
 *
 * Opening the console video device will disable kernel printf's.  When
 * The console video device is closed, another console_printf_init() is
 * done.
 */

#include <mach/mach_types.h>

#include <device/errno.h>

#include <vm/vm_kern.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#undef NULL
#include <mac2os/Types.h>
#include <mac2os/Errors.h>
#include <mac2os/Slots.h>
#include <mac2os/Video.h>
#include <mac2os/ROMDefs.h>
#include <mac2os/Files.h>

#include <mac2/slots.h>

#include <mac2dev/video.h>

#define	NVIDEO 6

#define VIDEO_F_SETUP  0x00000001
#define VIDEO_F_OPEN   0x00000002
#define VIDEO_F_STATUS 0x00000004

typedef struct video {
  short video_RefNum;
  unsigned short video_slot;
  unsigned short video_id;
  vm_offset_t video_devbase;
  vm_size_t video_devsize;
  unsigned long	video_flags;
  union {
    VDPageInfo vpi;
    VDEntryRecord ver;
  } video_stat;
} *video_t;

/* global screen structures indexed by minor dev */
static struct video video[NVIDEO];
static int nvideo;

/* Retrieve the video parameter block for the specified display mode. */
/* Returns non-zero if error. */
static boolean_t video_get_params(video_t vp, unsigned char mode, VPBlockPtr v)
{
  register SpBlockPtr sp = 0;
  register int error;

  if (error = !(sp = (SpBlockPtr)NewPtr(sizeof(SpBlock)))) goto done;

  sp->spSlot = vp->video_slot;
  sp->spID = vp->video_id;
  sp->spExtDev = 0;
  if (error = SRsrcInfo(sp)) goto done;
  sp->spID = mode;
  if (error = SFindStruct(sp)) goto done;
  sp->spID = mVidParams;
  if (error = SGetBlock(sp)) goto done;
  bcopy(sp->spResult, v, sizeof(VPBlock));
  DisposPtr(sp->spResult);

  /* all done */
done:
  if (sp) DisposPtr(sp);
  return error ? FALSE : TRUE;

} /* video_get_params() */

/* Do MacOS driver calls necessary to initialize a video device.
 * Doing a real cscReset seems to be a bad thing to do.
 * With the IIci builtin video it seems to do totally the wrong thing.
 * This code simulates it according to the spec in the book:
 *   "Designing Cards and Drivers for the Macintosh II and Macintosh SE".
 * Returns FALSE if error.
 */
static boolean_t video_cscReset(register video_t vp)
{
  CntrlParam *params = 0;
  VDPgInfoPtr vpi = 0;
  VDEntRecPtr ver = 0;
  ColorSpec *color = 0;
  register int error;

  if (error = !(vp->video_flags & VIDEO_F_SETUP)) goto done;

  if (error = !(params = (CntrlParam *)NewPtr(sizeof(CntrlParam)))) goto done;
  if (error = !(vpi = (VDPgInfoPtr)NewPtr(sizeof(VDPageInfo)))) goto done;
  if (error = !(ver = (VDEntRecPtr)NewPtr(sizeof(VDEntryRecord)))) goto done;
  if (error = !(color = (ColorSpec *)NewPtr(2 * sizeof(ColorSpec)))) goto done;

  /* set device to oneBitMode */
  vpi->csMode = oneBitMode;
  vpi->csPage = 0;
  params->ioCRefNum = vp->video_RefNum;
  params->csCode = cscSetMode;
  *(Ptr *)params->csParam = (Ptr)vpi;
  if (error = PBControl(params, FALSE)) goto done;

  /* set screen to 50% gray */
  vpi->csPage = 0;
  params->ioCRefNum = vp->video_RefNum;
  params->csCode = cscGrayPage;
  *(Ptr *)params->csParam = (Ptr)vpi;
  if (error = PBControl(params, FALSE)) goto done;

  /*
   * Load the CLUT with black & white pixel values.
   * The invocation code is different here; the SetEntries call is
   * allowed to return an error for devices that have a read-only CLUT.
   */
  color[0].value = 0;
  color[0].rgb.red = color[0].rgb.green = color[0].rgb.blue = 0xffff;
  color[1].value = 1;
  color[1].rgb.red = color[1].rgb.green = color[1].rgb.blue = 0x0000;
  ver->csTable = color;
  ver->csStart = 0;
  ver->csCount = (2 - 1);
  params->ioCRefNum = vp->video_RefNum;
  params->csCode = cscSetEntries;
  *(Ptr *)params->csParam = (Ptr)ver;
  (void)PBControl(params, FALSE);

  /* all done */
done:
  if (params) DisposPtr(params);
  if (vpi) DisposPtr(vpi);
  if (ver) DisposPtr(ver);
  if (color) DisposPtr(color);
  return error ? FALSE : TRUE;

} /* video_cscReset() */

/* open a video device */
io_return_t video_open(register int dev)
{
  register struct video	*vp;
  extern boolean_t printf_enable;

  if (dev >= NVIDEO) return D_NO_SUCH_DEVICE;
  vp = &video[dev];

  /* if the video_config() failed, this video device is not available */
  if (!(vp->video_flags & VIDEO_F_SETUP)) return D_NO_SUCH_DEVICE;

  /* do an initial reset */
  if (!video_cscReset(vp)) return D_NO_SUCH_DEVICE;

  /* this video device is now */
  if (!(vp->video_flags & VIDEO_F_OPEN)) vp->video_flags |= VIDEO_F_OPEN;

  /* disable kernel printf's while the console display is in use */
  if (vp == &video[0]) printf_enable = FALSE;

  /* all done */
  return D_SUCCESS;

} /* video_open() */

/* close a video device */
io_return_t video_close(register int dev)
{
  register video_t vp = &video[dev];
  extern console_printf_init();

  /* re-enable kernel printf's when console display is not in use */
  if (vp == &video[0]) console_printf_init();
  else (void)video_cscReset(vp);
  vp->video_flags &= ~(VIDEO_F_OPEN | VIDEO_F_STATUS);

  /* all done */
  return D_SUCCESS;

} /* video_close() */

/* map a video device */
unsigned int video_mmap(int dev, register vm_offset_t offset, int prot)
{
  register video_t vp = &video[dev];
    
  if (slot_ptr_to_offset(vp->video_devbase+offset) >= (16*1024*1024))
    return -1;

  return (mac2_btop(pmap_extract(kernel_pmap, vp->video_devbase + offset)));

} /* video_mmap() */

#define VOID	0
#define IN	1
#define OUT	2	
#define Ctrl_Call(code, len, dir)		\
MACRO_BEGIN					\
    Ptr		p;				\
    if ((len) > 0) {				\
	p = (Ptr)NewPtr(len);			\
	if (!p) {				\
	    error = ENOMEM;			\
	    break;				\
	}					\
	*(Ptr *)params->csParam = p;		\
	if ((dir) & IN) bcopy(data, p, (len));	\
    }						\
    params->ioCRefNum = vp->video_RefNum;	\
    params->csCode = (code);			\
    if (PBControl(params, FALSE) != noErr)	\
	error = EIO;				\
    else if ((dir) & OUT) bcopy(p, data, (len));\
    if ((len) > 0) DisposPtr(p);		\
MACRO_END

#define Stat_Call(code, len, dir)		\
MACRO_BEGIN					\
    Ptr		p;				\
						\
    if ((len) > 0) {				\
	p = (Ptr)NewPtr(len);			\
	if (!p) {				\
	    error = ENOMEM;			\
	    break;				\
	}					\
	*(Ptr *)params->csParam = p;		\
	if ((dir) & IN) bcopy(data, p, (len));	\
    }						\
    params->ioCRefNum = vp->video_RefNum;	\
    params->csCode = (code);			\
    if (PBStatus(params, FALSE) != noErr)	\
	error = EIO;				\
    else if ((dir) & OUT) bcopy(p, data, (len));\
    if ((len) > 0) DisposPtr(p);		\
MACRO_END

#define size_to_count(size) (((size) + 3) >> 2)

/* getstat for a video device */
video_getstat(int dev, int flavor, int *data, unsigned int *count)
{
  register video_t vp = &video[dev];
  register error = 0;
  CntrlParam *params;

  params = (CntrlParam *)NewPtr(sizeof(CntrlParam));
  if (!params) return ENOMEM;
  switch (flavor) {
    case VIDEO_CTRL_Init:
	{
	    register VDPgInfoPtr v = (VDPgInfoPtr)data;

	    (void)video_cscReset(vp);
	    Stat_Call(cscGetMode, sizeof(*v), OUT);
	    *count = size_to_count(sizeof(*v));
	    v->csBaseAddr = (Ptr)slot_ptr_to_offset(v->csBaseAddr);
	}
	break;

    case VIDEO_STAT_GetMode:
    case VIDEO_CTRL_SetMode:
	{
	    register VDPgInfoPtr v = (VDPgInfoPtr)data;

	    Stat_Call(cscGetMode, sizeof(*v), OUT);
	    *count = size_to_count(sizeof(*v));
	    v->csBaseAddr = (Ptr)slot_ptr_to_offset(v->csBaseAddr);
	}
	break;

    case VIDEO_STAT_GetEntries:
	{
	    register VDEntRecPtr v = (VDEntRecPtr)data;
	    register Ptr p;
	    register size;

	    if ((vp->video_flags & VIDEO_F_STATUS) == 0) {
		error = EINVAL;
		break;
	    }
	    vp->video_flags &= ~VIDEO_F_STATUS;
	    bcopy(&vp->video_stat.ver, data, sizeof(VDEntryRecord));
	    size = (v->csCount + 1) * sizeof(ColorSpec);
	    p = (Ptr)NewPtr(size);
	    if (!p) {
		error = ENOMEM;
		break;
	    }
	    v->csTable = (ColorSpec *)p;
	    Stat_Call(cscGetEntries, sizeof(*v), IN);
	    if (!error) {
		bcopy(p, data, size);
		*count = size_to_count(size);
	    }
	    DisposPtr(p);
	}
	break;

    case VIDEO_STAT_GetPages:
	if ((vp->video_flags & VIDEO_F_STATUS) == 0) {
	    error = EINVAL;
	    break;
	}
	vp->video_flags &= ~VIDEO_F_STATUS;
	bcopy(&vp->video_stat.vpi, data, sizeof(VDPageInfo));
	*count = size_to_count(sizeof(VDPageInfo));
	Stat_Call(cscGetPages, sizeof(VDPageInfo), IN | OUT);
	break;

    case VIDEO_STAT_GetBaseAddr:
	{
	    register VDPgInfoPtr v = (VDPgInfoPtr)data;

	    if ((vp->video_flags&VIDEO_F_STATUS) == 0) {
		error = EINVAL;
		break;
	    }
	    vp->video_flags &= ~VIDEO_F_STATUS;

	    bcopy(&vp->video_stat.vpi, data, sizeof(VDPageInfo));

	    *count = size_to_count(sizeof(*v));
	    Stat_Call(cscGetBaseAddr, sizeof(*v), IN | OUT);
	    v->csBaseAddr = (Ptr)slot_ptr_to_offset(v->csBaseAddr);
	}
	break;

    case VIDEO_STAT_GetGray:
	*count = size_to_count(sizeof(VDPageInfo));
	Stat_Call(cscGetGray, sizeof(VDPageInfo), OUT);
	break;

	/*
	 * Return the size of the video device.  This is probably not
	 * useful because the value stored on most cards is incorrect.
	 */
    case VIDEO_SIZE:
	bcopy(&vp->video_devsize, data, sizeof(vm_size_t));
	*count = size_to_count(sizeof(vm_size_t));
	break;

	/*
	 * Return the video parameter block for the mode that the card is
	 * currently in.
	 */
    case VIDEO_PARAMS:
	{
	    VDPgInfoPtr	vpi = (VDPgInfoPtr)NewPtr(sizeof(VDPageInfo));

	    if (vpi) {
		params->ioCRefNum = vp->video_RefNum;
		params->csCode = cscGetMode;
		*(Ptr *)params->csParam = (Ptr)vpi;
		if (PBStatus(params, FALSE) != noErr) error = EIO;
		else if (!video_get_params(vp, vpi->csMode, (VPBlockPtr)data))
                  error = EIO;
		else *count = size_to_count(sizeof(VPBlock));
	    }
	    else error = ENOMEM;
	    if (vpi) DisposPtr(vpi);
	}
	break;

    default:
	error = EINVAL;
	break;

  }

  DisposPtr(params);
  return error;

} /* video_getstat() */

/* setstat for a video device */
video_setstat(int dev, int flavor, int *data, unsigned int count)
{
  register video_t vp = &video[dev];
  register int error = 0;
  CntrlParam *params;

  params = (CntrlParam *)NewPtr(sizeof(CntrlParam));
  if (!params) return ENOMEM;
  switch (flavor) {
    case VIDEO_CTRL_KillIO:
	Ctrl_Call(cscKillIO, 0, VOID);
	break;

    case VIDEO_CTRL_SetMode:
	Ctrl_Call(cscSetMode, sizeof(VDPageInfo), IN);
	break;

    case VIDEO_CTRL_SetEntries:
	{
	    register VDEntRecPtr v = (VDEntRecPtr)data;
	    register Ptr p;
	    register size;

	    if (v->csStart == -1) {
		error = EINVAL;
		break;
	    }
	    size = (v->csCount + 1) * sizeof(ColorSpec);
	    if ((sizeof(*v) + size) > (count << 2)) {
		error = EINVAL;
		break;
	    }
	    p = (Ptr)NewPtr(size);
	    if (!p) {
		error = ENOMEM;
		break;
	    }
	    bcopy((Ptr)(v + 1), p, size);
	    v->csTable = (ColorSpec *)p;
	    Ctrl_Call(cscSetEntries, sizeof(*v), IN);
	    DisposPtr(p);
	}
	break;

    case VIDEO_CTRL_GrayScreen:
	Ctrl_Call(cscGrayPage, sizeof(VDPageInfo), IN);
	break;

    case VIDEO_CTRL_SetGray:
	Ctrl_Call(cscSetGray, sizeof(VDPageInfo), IN);
	break;

    case VIDEO_STAT_GetEntries:
	{
	    register VDEntRecPtr v = (VDEntRecPtr)data;
	    register size;

	    if (v->csStart == -1) {
		error = EINVAL;
		break;
	    }
	    size = sizeof(*v) + (v->csCount + 1) * sizeof(ColorSpec);
	    if (size_to_count(size) > DEV_STATUS_MAX) {
		error = EINVAL;
		break;
	    }
	    if ((vp->video_flags&VIDEO_F_STATUS) == 0
		&& (count << 2) >= sizeof(VDEntryRecord)) {
		bcopy(data, &vp->video_stat.ver, sizeof(VDEntryRecord));
		vp->video_flags |= VIDEO_F_STATUS;
	    }
	    else error = EINVAL;
	}
	break;

    case VIDEO_STAT_GetPages:
    case VIDEO_STAT_GetBaseAddr:
	if ((vp->video_flags&VIDEO_F_STATUS) == 0
	    && (count << 2) >= sizeof(VDEntryRecord)) {
	    bcopy(data, &vp->video_stat.vpi, sizeof(VDPageInfo));
	    vp->video_flags |= VIDEO_F_STATUS;
	}
	else error = EINVAL;
	break;
	
    default:
	error = EINVAL;
	break;

  }

  DisposPtr(params);
  return error;

} /* video_setstat() */

#undef size_to_count
#undef Stat_Call
#undef Ctrl_Call
#undef OUT
#undef IN
#undef VOID

/* setup a video device */
boolean_t video_config(unsigned char slot, unsigned char id)
{
  SpBlock sp;
  register video_t vp;
  register unit;
  VPBlock v;

  /* search for video device information for this (slot,id) */
  for (unit = 0; unit < nvideo; unit++)
    if (video[unit].video_slot == slot && video[unit].video_id == id) break;
  vp = &video[unit];

  /* if first time seen, reset the video device and get the parameters */
  if (unit == nvideo) {

    if (nvideo >= NVIDEO) return FALSE;

    vp->video_slot = slot;
    vp->video_id = id;

    /* determine base address and size of video device */
    sp.spSlot = slot;
    sp.spID = id;
    sp.spExtDev = 0;
    if (SFindDevBase(&sp)) return FALSE;
    vp->video_devbase = sp.spResult;
    if (SRsrcInfo(&sp)) return FALSE;
    if ((vp->video_devbase & 0xf0000000) == 0xf0000000) sp.spID = minorLength;
    else sp.spID = majorLength;
    if (SReadLong(&sp)) return FALSE;
    vp->video_devsize = sp.spResult;

    /* determine I/O reference number of video device */
    sp.spSlot = slot;
    sp.spID = id;
    sp.spExtDev = 0;
    if (SRsrcInfo(&sp)) return FALSE;
    vp->video_RefNum = sp.spRefNum;

    /* determine if this video device is slot mappable */
    if ((slot < SLOT_NUM_LOW) || (slot > SLOT_NUM_HIGH)) return FALSE;
    slot_to_slotdata_ptr(slot)->SFlags |= SLOT_MAPPABLE;

    /* set flag to indicate that this device has been setup properly */
    vp->video_flags = VIDEO_F_SETUP;

    /* reset to gray */
    if (!video_cscReset(vp)) {
      vp->video_flags = 0;
      return FALSE;
    }

    nvideo++;

  }

  /* get the parameters */
  if (!video_get_params(vp, oneBitMode, &v)) return FALSE;

  /* display parameters for each video device available */
  printf("video sRsrc: slot %x id %d (%d,%d,%d,%d) assigned to unit %d%s\n",
         slot, id,
         v.vpBounds.top, v.vpBounds.left,
         v.vpBounds.bottom, v.vpBounds.right,
         unit, unit ? "" : " (console)");

  /* all done */
  return TRUE;

} /* video_config() */

/* Reset the video display device  used as the system console. */
/* Returns non-zero if error. */
boolean_t console_video_init(vm_offset_t *devbase, VPBlockPtr *v)
{
  SpBlock sp;
  register video_t vp = &video[0];
  struct {
    unsigned char sdSlot;
    unsigned char sdSResource;
  } vd;
  struct {
    unsigned short len;
    unsigned short addr;
  } arg;
  static VPBlock console_vpb;

  /* if a console display has already been found, then just reset it */
  if (vp->video_flags & VIDEO_F_SETUP) {
    if (!video_cscReset(vp)) return FALSE;
    goto done;
  }

  /* otherwise, first try the display used for the Happy Macintosh */
  arg.addr = 0x80;
  arg.len = sizeof(vd);
  if (!ReadXPRam(&vd, arg) && vd.sdSlot) {
    sp.spSlot = vd.sdSlot;
    sp.spID = vd.sdSResource;
    sp.spExtDev = 0;
    if (!SRsrcInfo(&sp)) {
      if (sp.spCategory == catDisplay && sp.spCType == typeVideo) {
        if (video_config(sp.spSlot, sp.spID)) goto done;
      }
    }
  }

  /* otherwise find the display with lowest slot and id number */
  sp.spSlot = 0;
  sp.spID = sp.spExtDev = 0;
  if (SNextsRsrc(&sp)) return FALSE;
  else for (;;) {
    if (sp.spCategory == catDisplay && sp.spCType == typeVideo) {
      if (video_config(sp.spSlot, sp.spID)) goto done;
    }
    if (SNextsRsrc(&sp)) return FALSE;
  }

  /* all done, return the base address and a pointer to the video parameters */
done:
  if (!video_get_params(vp, oneBitMode, &console_vpb)) return FALSE;
  *v = &console_vpb;
  *devbase = vp->video_devbase;
  return TRUE;

} /* console_video_reset() */
