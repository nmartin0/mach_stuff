/*
 * MkLinux Public Source License
 * v1.1
 *
 * Portions Copyright (c) 1999-2002 The MkLinux Project.
 *              All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice and disclaimer
 * appear in all copies of the software, derivative works or modified
 * versions, and any portions thereof, and that these notices also
 * appear in supporting documentation, and provided that all advertising
 * materials mentioning features or use of this software displays the
 * following acknowledgement:
 *
 *      This product includes software developed by The MkLinux Project
 *      and its contributors.
 *
 * Neither the name of The MkLinux Project nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND THE MKLINUX PROJECT HEREBY DISCLAIMS ALL
 * SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT."
 * 
 * IN NO EVENT SHALL THE MKLINUX PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN ACTION OF CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF OR IN CONNECTION WITH THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Software whose source code is freely available, including those licensed
 * under the GNU General Public License (GPL) are exempt from all advertising
 * requirements.  Software collections are not exempt unless they include
 * non MPSL versions of this code as allowed below.
 *
 * License is hereby granted to freely redistribute this code as part of
 * another software product under the terms of the GPL or any other OSI-
 * certified open source license agreement provided that this copyright notice
 * remain intact, and provided that all copies of the source code contain, in
 * some reasonable, human-readable form, information telling where any party
 * may obtain a copy of this source code that is not burdened by the GPL or
 * other license's potentially more restrictive licensing terms.
 */
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

#include <fd.h>
#include <sys/types.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/lock.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppycore.h"
#include "floppysal.h"
#include <kern/wait_queue.h>

#if NFD > 0

#if 1
/* Darwin's kernel doesn't export this in a header file.... */
kern_return_t
thread_terminate(
        register thread_act_t   thr_act);
/* Ouch.  We need a thread_t... */
/* Ouch.  This wasn't correct under MkLinux, either, technically.... */
    wait_queue_t scan_terminate_queue;
#endif

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

volatile int MediaScanTaskContinue = true;
extern caddr_t org_mklinux_swim3_device[2];
extern caddr_t org_mklinux_swim3_driver[2];
// extern void (*org_mklinux_iokit_swim3_funcptr)(void *,  BSAccessibilityState);

// void org_mklinux_iokit_swim3_driver_NotifyMediaChange(void *driverclassptr,
	// int newstatus);

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
*	FUNCTION : PostDisketteEvent
*
************************************************************************/

OSStatus
PostDisketteEvent(int unit, unsigned char disketteEvent,
		  short driveIndex)
{


    register short errorCode;

    errorCode=featureUnsupported;

    switch (disketteEvent) {

    case DISK_PRESENT:
	errorCode = BSMPINotifyFamilyStoreChangedState(SonyVariables[unit].driveStatus[driveIndex - 1].OSDiskID,
						       kBSOnline);
	// (org_mklinux_iokit_swim3_funcptr)(
	    // org_mklinux_swim3_driver[unit], kBSOnline);
	break;

    case DISK_AUTO_EJECTING:
	errorCode = BSMPINotifyFamilyStoreChangedState(SonyVariables[unit].driveStatus[driveIndex - 1].OSDiskID,
						       kBSOffline);
	// (org_mklinux_iokit_swim3_funcptr)(
	    // org_mklinux_swim3_driver[unit], kBSOffline);
	break;

    case NO_DISKETTE:
	errorCode = BSMPINotifyFamilyStoreChangedState(SonyVariables[unit].driveStatus[driveIndex - 1].OSDiskID,
						       kBSOffline);
	// (org_mklinux_iokit_swim3_funcptr)(
	    // org_mklinux_swim3_driver[unit], kBSOffline);
	break;

    }

    return errorCode;

}

#if 0

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

    scan_terminate_queue=wait_queue_alloc(SYNC_POLICY_FIFO);
    org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "wait_queue_alloc\n");
    while (MediaScanTaskContinue == true) {

	/*
	 * Scan for any diskette changes, then reschedule ourselfs to run
	 * again in the next 1/2 second.
	 */

	ScanForDisketteChange();

	/* @@@ */

	FloppyTimedSleep((long) kDurationSecond / 2);
        
        // if (wait_queue_wakeup_all(scan_terminate_queue,
            // (event_t)&MediaScanTaskID, 0) == KERN_SUCCESS) {
            // /* We're supposed to die now.... */
            // break;
        // }

    }
    non_bsd_wakeup(&MediaScanTaskID);
    MediaScanTaskContinue = true;
    thread_terminate(MediaScanTaskID); /* this thread commits suicide */
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
#ifdef MKLINUX_VERSION
    MediaScanTaskID = kernel_thread(kernel_task, MediaScanTask, (void *) 0);
#else
    MediaScanTaskID = kernel_thread(kernel_task, MediaScanTask);
#endif
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

    /* Assume that if we're interrupted the other thread is probably
       dead, so don't bother checking return values.  If we're wrong,
       we'll panic.  It's a hack, but the alternative is worse.  :-) */
    // wait_queue_assert_wait(scan_terminate_queue,
        // (event_t)&MediaScanTaskID, FALSE);

    MediaScanTaskContinue = false;

    while (!MediaScanTaskContinue) {
	/* Max wait 5 seconds, but if we get legitimately woken, 
	   we're leaving, fair and square. :-) */
	if (non_bsd_sleep(&MediaScanTaskID, 5000) == EVENT_WAKE) break;
    }

    /* Just to avoid possible race with thread termination when the
       stack is being torn down.  Not likely, but worth a delay to be
       safe.  Use a different address as the event to avoid the risk of
       being woken up immediately. */
    non_bsd_sleep((void *)&MediaScanTaskContinue, 1000); // one second

    // (void)thread_terminate(MediaScanTaskID);

    return noErr;

}

#endif

//******************************************************************************
//
//      FUNCTION : RegisterFloppyISR
//
//******************************************************************************

#endif // NFD > 0
