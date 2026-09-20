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


#ifndef __FLOPPYSAL_H__
#define __FLOPPYSAL_H__


//#include <Types.h>
//#include <Disks.h>
//#include <NameRegistry.h>
#include "portdef.h"

/************************************************************************
*
* Function prototypes for routines in FloppySAL.c
*
************************************************************************/

OSStatus
RecordError(OSStatus errorCode);

OSStatus
GetMemory(long RequestSize,
	  Ptr * memoryPtr);

OSStatus
MapCacheForDMAIO(LogicalAddress logicalBaseAddr,
		 uint_t length,
		 LogicalAddress * logBase,
		 PhysicalAddress * phyBase);

OSStatus
PrepareCPUCacheForDMARead(void);

OSStatus
PrepareCPUCacheForDMAWrite(void);

OSStatus
FlushDMAedDataFromCPUCache(void);

OSStatus
FloppyTimedSleep(long Milliseconds);

OSStatus CreateOSHardwareLockResources(spl_t * OSLockID);

OSStatus
DeleteOSHardwareLockResources(void *OSLockID);

spl_t
EnterHardwareLockSection(void);

void
 ExitHardwareLockSection(spl_t);

OSStatus
CreateOSEventResources(UInt32 * OSEventID);

OSStatus
DeleteOSEventResources(void *OSEventID);

OSStatus
SetOSEvent(UInt32 * OSEventID,
	   uint_t eventMask);

OSStatus
CancelOSEvent(UInt32 * OSEventID,
	      uint_t eventMask);

boolean_t
WaitForOSEvent(UInt32 * OSEventID,
	       uint_t eventMask,
	       uint_t timeOut,
	       uint_t * eventMaskResult);

OSStatus
PostDisketteEvent(int unit, unsigned char disketteEvent,
		  short driveIndex);

OSStatus
LaunchMediaScanTask(void);

OSStatus
KillMediaScanTask(void);

OSStatus
RegisterFloppyISR(RegEntryID * deviceEntryID,
		  void *ISRHandlerPtr);

#endif				// if already included.
