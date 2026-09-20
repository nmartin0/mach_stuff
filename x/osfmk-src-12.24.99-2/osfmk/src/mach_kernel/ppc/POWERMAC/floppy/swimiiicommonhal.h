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
*	File:		SwimIIICommonHAL.h
*
*	Contains:	Floppy driver HAL routines for SWIMIII on all machines.
************************************************************************/

//#include <DriverSupport.h>

#include "floppypriv.h"
#include "floppysal.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "swimiii.h"


/************************************************************************
*
*	Local constants for this module.
*
************************************************************************/

#define DMA_READ	true
#define DMA_WRITE	false


/************************************************************************
*
*	The following group of the subroutines may only be referenced
*	internally to this module. Therfore, their function prototypes
*	appear only in this module.
*
************************************************************************/

OSStatus
SleepUntilReady(long PreScanTimeout);

void
 SwimIIISmallWait(unsigned short microseconds);

boolean_t
SwimIIITimeOut(unsigned long *milliseconds);

OSStatus
WaitForEvent(unsigned long milliseconds,
	     unsigned char actionBitMask,
	     unsigned char interruptMask);

void
 ByteMove(void *srcPtr,
	  void *destPtr,
	  long byteCount);

void
 FormatGCRCacheSWIMIIIData(DriveStatusType * DriveStatus);

void
 FormatMFMCacheSWIMIIIData(DriveStatusType * DriveStatus);

void
 SwimIIIAddrSignal(unsigned char stat_cmd);

boolean_t
SwimIIISenseSignal(unsigned char status);

void
 SwimIIISetSignal(unsigned char command);

void
 SwimIIIHeadSelect(short HeadNumber);

void
 SwimIIIDiskSelect(DriveStatusType * DriveStatus);

OSStatus
SwimIIIStepDrive(short StepCount);

void
 SwimIIIDisableRWMode(void);

void
 SwimIIISetReadMode(void);

void
 SwimIIISetWriteMode(void);

OSStatus
ResetDMAChannel(void);

OSStatus
StopDMAChannel(void);

OSStatus
StartDMAChannel(void *DMAAddress,
		unsigned long DMACount,
		unsigned short direction);
