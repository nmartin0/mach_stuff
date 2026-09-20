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

//#include <Types.h>
//#include <Errors.h>
//#include <Memory.h>
//#include <Timing.h>
//#include <Interrupts.h>
//#include <DriverServices.h>
//#include <BlockStorage.h>
//#include <BlockStoragePlugin.h> 

#include <types.h>
#include <kern/task.h>
#include <kern/thread.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppycore.h"
#include "floppysal.h"
#include <fd.h>

#if NFD > 0

/************************************************************************
*
* Global data for driver.
*
************************************************************************/

SonyVarsType SonyVariables[NFD];

void *theDefaultRefCon;
#if 0
InterruptHandler theDefaultHandlerFunction;
InterruptEnabler theDefaultEnableFunction;
InterruptDisabler theDefaultDisableFunction;
InterruptSetMember myISTMember;
IOPreparationTable cachePrepTableID;
Lock theOSLockID;
#endif


/************************************************************************
*
*	FUNCTION : RecordError
*
*	Remembers the last error code for debugging.
*
************************************************************************/

OSStatus
RecordError(OSStatus errorCode)
{

    /*
     * Don't trap unless there really is an error.
     */

    if ((errorCode != noErr) &&
	SonyVariables[0].debuggingEnabled) {

	/*
	 * Now return the error code back to the caller.
	 *
	 * The trick here is; while debugging, un-comment
	 * the debugger trap call. Since ALL error codes
	 * come through here, when you hit this break point
	 * in MacsBug, you can do a "SC", or stack crawl
	 * to see exactly where in the code the error was
	 * detected.
	 */


    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : GetMemory
*
*	Get a block of non-relocatable memory from the System heap. Return a
*	NIL if unable to allocate memory block requested.
*
************************************************************************/
//******************************************************************************
//
//      FUNCTION : MapCacheForDMAIO
//
//******************************************************************************
#if 0
OSStatus
MapCacheForDMAIO(LogicalAddress cacheLogicalBaseAddr,
		 ByteCount cacheByteLength,
		 LogicalAddress * logBase,
		 PhysicalAddress * phyBase)
{

    OSStatus errorCode;
    ByteCount numPages;
    LogicalMappingTablePtr logicalPrepTablePtr;
    PhysicalMappingTablePtr physicalPrepTablePtr;


    // Get the Memory to Logically map the whole address space.
    numPages = ((cacheByteLength + GetLogicalPageSize() - 1) / GetLogicalPageSize()) + 2;

    logicalPrepTablePtr = (LogicalMappingTablePtr) PoolAllocateResident((numPages * sizeof(LogicalAddress)), true);
    physicalPrepTablePtr = (PhysicalMappingTablePtr) PoolAllocateResident((numPages * sizeof(PhysicalAddress)), true);

    // Map the Address Space into Virtual memory visiable to every-one
    cachePrepTableID.options = kIOLogicalRanges | kIOIsInput | kIOIsOutput;
    cachePrepTableID.addressSpace = CurrentAddressSpaceID();
    cachePrepTableID.granularity = 0;
    cachePrepTableID.firstPrepared = 0;
    cachePrepTableID.mappingEntryCount = numPages;
    cachePrepTableID.logicalMapping = logicalPrepTablePtr;
    cachePrepTableID.physicalMapping = physicalPrepTablePtr;
    cachePrepTableID.rangeInfo.range.base = cacheLogicalBaseAddr;
    cachePrepTableID.rangeInfo.range.length = cacheByteLength;

    errorCode = PrepareMemoryForIO(&cachePrepTableID);

    if ((errorCode == noErr) &&
	(cachePrepTableID.lengthPrepared != cacheByteLength)) {

	LLDebugStr((StringPtr) "\pWe're hozed again");

	errorCode = paramErr;

    } else {

	*logBase = cacheLogicalBaseAddr;	// Return the Logical Address of track cache Base

	*phyBase = physicalPrepTablePtr[0];	// Return the Physical Address of track cache Base

	errorCode = SetProcessorCacheMode(CurrentAddressSpaceID(),
					  *logBase,
					  cacheByteLength,
					  kProcessorCacheModeInhibited);

    }

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : PrepareCPUCacheForDMARead
*
*	Make sure that any space that will DMA'ed into is invalidated.
*
************************************************************************/

OSStatus
PrepareCPUCacheForDMARead(void)
{

    OSStatus errorCode;


    errorCode = CheckpointIO(cachePrepTableID.preparationID,
			     kMoreIOTransfers | kNextIOIsInput);

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : PrepareCPUCacheForDMAWrite
*
*	Make sure that any space that will DMA'ed from is invalidated.
*
************************************************************************/

OSStatus
PrepareCPUCacheForDMAWrite(void)
{

    OSStatus errorCode;


    errorCode = CheckpointIO(cachePrepTableID.preparationID,
			     kMoreIOTransfers | kNextIOIsOutput);

    return errorCode;

}

/************************************************************************
*
*	FUNCTION : FlushDMAedDataFromCPUCache
*
*	Make sure that any data that was DMA'ed is flushed to main proccessor
*	memory.
*
************************************************************************/

OSStatus
FlushDMAedDataFromCPUCache(void)
{

    OSStatus errorCode;


    errorCode = CheckpointIO(cachePrepTableID.preparationID,
			     kMoreIOTransfers);

    return errorCode;

}


#endif
/************************************************************************
*
*	FUNCTION : FloppyTimedSleep
*
*	Sleep for the time period specifed in milliseconds.
*
************************************************************************/

#if 0
OSStatus
FloppyTimedSleep(long Milliseconds)
{

    return DelayFor(Milliseconds);

}

/************************************************************************
*
*	FUNCTION : CreateOSHardwareLockResources
*
************************************************************************/

OSStatus
CreateOSHardwareLockResources(void **OSLockID)
{

    *OSLockID = (void *) &theOSLockID;

    return CreateLock(&theOSLockID, 0);

}


/************************************************************************
*
*	FUNCTION : DeleteOSHardwareLockResources
*
************************************************************************/

OSStatus
DeleteOSHardwareLockResources(void *OSLockID)
{

    return DeleteLock((Lock *) OSLockID);

}


/************************************************************************
*
*	FUNCTION : EnterHardwareLockSection
*
************************************************************************/

OSStatus
EnterHardwareLockSection(void *OSLockID)
{

    return BeginLockedSection((Lock *) OSLockID);

}


/************************************************************************
*
*	FUNCTION : ExitHardwareLockSection
*
************************************************************************/

OSStatus
ExitHardwareLockSection(void *OSLockID)
{

    return EndLockedSection((Lock *) OSLockID);

}


/************************************************************************
*
*	FUNCTION : CreateOSEventResources
*
************************************************************************/

OSStatus
CreateOSEventResources(void **OSEventID)
{

    return CreateEventGroup((EventGroupID *) OSEventID);

}


/************************************************************************
*
*	FUNCTION : DeleteOSEventResources
*
************************************************************************/

OSStatus
DeleteOSEventResources(void *OSEventID)
{

    return DeleteEventGroup((EventGroupID) OSEventID);

}


/************************************************************************
*
*	FUNCTION : SetOSEvent
*
************************************************************************/

OSStatus
SetOSEvent(void *OSEventID,
	   UInt32 eventMask)
{

    return SetEvents((EventGroupID) OSEventID,
		     (EventGroupMask) eventMask);

}


/************************************************************************
*
*	FUNCTION : CancelOSEvent
*
************************************************************************/

OSStatus
CancelOSEvent(void *OSEventID,
	      UInt32 eventMask)
{

    return ClearEvents((EventGroupID) OSEventID,
		       (EventGroupMask) eventMask);

}


/************************************************************************
*
*	FUNCTION : WaitForOSEvent
*
************************************************************************/

Boolean
WaitForOSEvent(void *OSEventID,
	       UInt32 eventMask,
	       UInt32 timeOut,
	       UInt32 * eventMaskResult)
{

    Boolean eventDidNotTimeOut = false;


    /*
     * Make sure that the returned events is pre-cleared
     * in case someone is spinning on its value.
     */

    *eventMaskResult = 0;

    /*
     * Wait for the events to occur by the time specified. If WaitForEvents
     * returns an error, its probability a kernelIncompleteErr which means
     * that we timed out.
     */

    WaitForEvents((EventGroupID) OSEventID,
		  (Duration) timeOut,
		  (EventGroupMask) eventMask,
		  kEventFlagAny,
		  (EventGroupMask *) eventMaskResult);

    /*
     * Check to see if the events we're waiting on actually occured,
     * and if so say that we were successful. We just blinding test
     * this because if WaitForEvents didn't return an error, one of
     * our events did occur. If WaitForEvents did return an error but
     * one of our events was still set, the event was probability in
     * a race condition with the timeout. Therefore, we didn't really
     * time out.
     */

    if ((eventMask & *eventMaskResult) != 0)
	eventDidNotTimeOut = true;

    return eventDidNotTimeOut;

}

#endif
/************************************************************************
*
*	FUNCTION : PostDisketteEvent
*
************************************************************************/

OSStatus
PostDisketteEvent(int unit, unsigned char disketteEvent,
		  short driveIndex)
{


    register short errorCode;


    switch (disketteEvent) {

    case DISK_PRESENT:
	errorCode = BSMPINotifyFamilyStoreChangedState(SonyVariables[unit].driveStatus[driveIndex - 1].OSDiskID,
						       kBSOnline);
	break;

    case DISK_AUTO_EJECTING:
	errorCode = BSMPINotifyFamilyStoreChangedState(SonyVariables[unit].driveStatus[driveIndex - 1].OSDiskID,
						       kBSOffline);
	break;

    case NO_DISKETTE:
	errorCode = BSMPINotifyFamilyStoreChangedState(SonyVariables[unit].driveStatus[driveIndex - 1].OSDiskID,
						       kBSOffline);
	break;

    }

    return errorCode;

}
thread_t MediaScanTaskID;

/************************************************************************
*
*	FUNCTION : MediaScanTask
*
************************************************************************/

void
 MediaScanTask(void);
#define kDurationSecond 1000
void MediaScanTask(void)
{

    while (true) {

	/*
	 * Scan for any diskette changes, then reschedule ourselfs to run
	 * again in the next 1/2 second.
	 */

	ScanForDisketteChange();

	FloppyTimedSleep((long) kDurationSecond / 2);

    }

}


/************************************************************************
*
*	FUNCTION : LaunchMediaScanTask
*
************************************************************************/

OSStatus
LaunchMediaScanTask(void)
{

    OSStatus errorCode = noErr;

//      errorCode = CreateTask( (TaskName)'FpSn',
    //                                                      CurrentKernelProcessID(),
    //                                                      (TaskProc)MediaScanTask,
    //                                                      0,
    //                                                      NULL,
    //                                                      0,
    //                                                      NULL,
    //                                                      kNilOptions + kTaskLowDriverPriority + kTaskIsOrphan,
    //                                                      &MediaScanTaskID );
    //      
    MediaScanTaskID = kernel_thread(kernel_task, MediaScanTask, (void *) 0);
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : KillMediaScanTask
*
************************************************************************/


OSStatus
KillMediaScanTask(void)
{

//      TerminateTask( MediaScanTaskID,
    //                                 kTaskOnly,
    //                                 0,
    //                                 noErr );

    //(void)thread_terminate(MediaScanTaskID);
    return noErr;

}


//******************************************************************************
//
//      FUNCTION : RegisterFloppyISR
//
//******************************************************************************

#endif // NFD > 0
