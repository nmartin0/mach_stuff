/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * Copyright 1991-1998 by Apple Computer, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */

/************************************************************************
*
*	File:		FloppyHAL.h
*
************************************************************************/

#ifndef __FLOPPYHAL_H__
#define __FLOPPYHAL_H__
//#include <Interrupts.h>

#include "portdef.h"
#include "floppypriv.h"
/************************************************************************
*
* Function prototypes for abstract routines used in the Sony HAL.
*
************************************************************************/

OSStatus
HALReset(DriveStatusType * DriveStatus,
	 LogicalAddress floppyControllerBaseAddr,
	 LogicalAddress DMAReadControllerBaseAddr,
	 LogicalAddress DMAWriteControllerBaseAddr);

void
 HALISRHandler(int device,
	       void *ssp);
void
 HALISR_DMA(int device,
	    void *ssp);

OSStatus
HALSetFormatMode(DriveStatusType * DriveStatus);

OSStatus
HALPowerUpDrive(DriveStatusType * DriveStatus);

void
 HALPowerDownDrive(DriveStatusType * DriveStatus);

OSStatus
HALEjectDiskette(DriveStatusType * DriveStatus);

boolean_t
HALGetDriveType(DriveStatusType * DriveStatus);

OSStatus
HALSeekDrive(DriveStatusType * DriveStatus);

OSStatus
HALRecalDrive(DriveStatusType * DriveStatus);

boolean_t
HALDiskettePresence(DriveStatusType * DriveStatus);

void
 HALGetMediaType(DriveStatusType * DriveStatus);

OSStatus
HALGetNextAddressID(DriveStatusType * DriveStatus);

OSStatus
HALReadSector(DriveStatusType * DriveStatus);

OSStatus
HALWriteSector(DriveStatusType * DriveStatus);

OSStatus
HALFormatTrack(DriveStatusType * DriveStatus);

#endif				// if already included
