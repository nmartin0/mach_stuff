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
 */

#ifndef _MAC2DEV_VIDEO_H_
#define _MAC2DEV_VIDEO_H_

#include <mac2os/Types.h>
#include <mac2os/Video.h>

/* Video Ioctl calls. */

/* MAC driver control calls */
#define VIDEO_CTRL_Init		_IOR('V', 0, VDPageInfo)
#define VIDEO_CTRL_KillIO	_IO('V', 1)
#define VIDEO_CTRL_SetMode	_IOWR('V', 2, VDPageInfo)
#define VIDEO_CTRL_SetEntries	_IOW('V', 3, VDEntryRecord)
#define VIDEO_CTRL_SetGamma	_IOW('V', 4, Ptr)
#define VIDEO_CTRL_GrayScreen	_IOW('V', 5, VDPageInfo)
#define VIDEO_CTRL_SetGray	_IOW('V', 6, VDPageInfo)

/* MAC driver status calls */
#define VIDEO_STAT_GetMode	_IOR('V', 7, VDPageInfo)
#define VIDEO_STAT_GetEntries	_IOWR('V', 8, VDEntryRecord)
#define VIDEO_STAT_GetPages	_IOWR('V', 9, VDPageInfo)
#define VIDEO_STAT_GetBaseAddr	_IOWR('V', 10, VDPageInfo)
#define VIDEO_STAT_GetGray	_IOR('V', 11, VDPageInfo)

/* Return size of a video device. (don't use this) */
#define VIDEO_SIZE		_IOR('V', 20, vm_size_t)

/* Return video parameters based on current device mode. */
#define VIDEO_PARAMS		_IOR('V', 22, VPBlock)

#endif /* _MAC2DEV_VIDEO_H_ */
