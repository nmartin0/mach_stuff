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

/************************************************************************
*
*	File:		BSFloppy.c
*
*	Contains:	BlockStorage interface routines for the Floppy driver.
*
************************************************************************/

//#include <Types.h>
//#include <DriverSupport.h>
//#include <DriverFamilyMatching.h>
//#include <BlockStorage.h>
//#include <BlockStoragePlugin.h> 

//#include <PoolsPriv.h> 
#include <fd.h>
#include <sys/types.h>
#include <machine/spl.h>
#if 0
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include <sys/ioctl.h>
#include <kern/kalloc.h>
#include <ppc/pmap.h>
#endif
#include <kern/lock.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "floppysal.h"
#if 0
#include <ppc/POWERMAC/dbdma.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/device_tree.h>
#endif
#include "swimiii.h"
#include <sys/systm.h> // for printf
#include "dprintf.h"

#if NFD > 0

// #if (!MACH_DEBUG)
// #define printf donone
// #else
// #if (SERIAL_DEBUG)
// #undef dprintf
// #define dprintf kprintf
// #define printf kprintf
// #endif
// #endif
/************************************************************************
*
* Global data for driver.
*
************************************************************************/

extern caddr_t org_mklinux_swim3_vaddr[2];
extern caddr_t org_mklinux_swim3_dmvaddr[2];
#if 0
char stupidbuffer[0xfffff];
char stupidbuffer2[0xfffff];
char stupidbuffer3[0xfffff];
char stupidbuffer4[0xfffff];
#endif
#ifdef KERNEL_BUILTIN
// char TrackBuffer[NFD][0xb000];
#else
PhysicalAddress TrackBufferPhys[NFD]={NULL
#if NFD > 1
    , NULL
#endif
};
char *TrackBuffer[NFD]={NULL
#if NFD > 1
    , NULL
#endif
};
#endif

extern BSIOStatus BSBlockListDescriptorGetExtent(BSBlockListDescriptorRef blocks,
						 int ignore,
					      BSByteCount * startingByte,
					    ByteCount * blocksExtentLen);
extern
SonyVarsType SonyVariables[];


// DriveStatusType *myDriveStatus;
DriveStatusType *driveStatus[NFD];

/************************************************************************
*
* Static functions in this driver.
*
************************************************************************/

OSStatus
GetFloppyHardwareAddresses(int unit,
			   RegEntryID * deviceEntryID,
			   LogicalAddress * deviceRegsLogicalBase,
			   PhysicalAddress * deviceRegsPhysicalBase,
			   LogicalAddress * DMARegsLogicalBase,
			   PhysicalAddress * DMARegsPhysicalBase,
			   LogicalAddress * trackCacheLogicalBase,
			   PhysicalAddress * trackCachePhysicalBase);

OSStatus
FloppyPluginInit(int unit, BSStorePtr initStore);

#if 0
static OSStatus
 FloppyPluginCleanup(int unit);
#endif
BSIOStatus
FloppyPluginIO(int unit, BSStorePtr ioStore,
	       BSBlockListDescriptorRef blocks,
	       MemListDescriptorRef memory,
	       BSIORequestBlockPtr parentRequest,
	       OptionBits options,
	       struct BSErrorList **errors);

BSIOStatus
FloppyPluginFlush(int unit,
		  BSIORequestBlockPtr parentRequest,
		  struct BSErrorList **errors);

#if 0
static OSStatus
 FloppyPluginAddComponent(BSStorePtr destStore,
			  struct BSStoreMPIComponent *newComponent,
			  struct BSStoreInfo *storeNewInfo);
#endif

OSStatus
FloppyPluginGotoState(BSStorePtr theStore,
		      BSAccessibilityState gotoState);

OSStatus
FloppyPluginEject(int unit,
		  BSAccessibilityState gotoState);

OSStatus
 FloppyPluginFormatMedia(int unit, BSStorePtr formatStore,
			 BSFormatIndex formatType);

OSStatus
 FloppyPluginGetInfo(int unit,
		     struct BSStoreMPIInfo *info);

#if 0
static OSStatus
 FloppyPluginioCompletion(BSStorePtr theStore,
			  void *finishedPrivateData,
			  BSErrorListPtr returnedBSErrorList,
			  OSStatus returnedStatus,
			  BSErrorListPtr * errorListPtrPtr);
#endif




/************************************************************************
*
*	FUNCTION : FloppyPluginDeviceExamine
*
************************************************************************/

#if 0
static OSStatus
 FloppyPluginDeviceExamine(BSStorePtr deviceID,
			   BSMPIConfidenceLevel * certainty)
{

#pragma unused( deviceID )

    OSStatus errorCode;


//      DebugStr( (ConstStr255Param)"\pFloppyPluginDeviceExamine: Swim3 plugin match made." );

    /*
     * Assume success.
     */

    errorCode = E_BSSuccess;

    *certainty = kBSMPIDeviceModelRecognized;

    return errorCode;

}
#endif

#if 0
void blowcache()
{
int i;
char k;

    for (i=0; i<0xfffff; i++) {
	k=stupidbuffer[i];
	stupidbuffer[i] = stupidbuffer2[i];
	stupidbuffer2[i] = stupidbuffer3[i];
	stupidbuffer3[i] = stupidbuffer4[i];
	stupidbuffer4[i] =k;
    }

}
#endif

//******************************************************************************
//
//      FUNCTION : GetFloppyHardwareAddresses
//
//******************************************************************************

typedef struct RegPropertyRec {
    UInt32 addressOffset;
    UInt32 addressLength;
} RegPropertyRec;

OSStatus
GetFloppyHardwareAddresses(int unit,
			   RegEntryID * deviceEntryID,
			   LogicalAddress * deviceRegsLogicalBase,
			   PhysicalAddress * deviceRegsPhysicalBase,
			   LogicalAddress * DMARegsLogicalBase,
			   PhysicalAddress * DMARegsPhysicalBase,
			   LogicalAddress * trackCacheLogicalBase,
			   PhysicalAddress * trackCachePhysicalBase)
{

    OSStatus errorCode;
#if 0
    UInt32 i;
    LogicalAddress logCacheBase;
    PhysicalAddress phyCacheBase;
#endif
#if 0
    device_node_t *swims = NULL;


    /*
     * Assume success.
     */

#endif
    errorCode = noErr;
#if 0

    switch (powermac_info.class) {
    case POWERMAC_CLASS_PERFORMA:
    case POWERMAC_CLASS_POWERBOOK:
	/* UNSUPPORTED! */
	*deviceRegsLogicalBase = 0;
	*deviceRegsPhysicalBase = 0;

	*DMARegsPhysicalBase = 0;
	*DMARegsLogicalBase = 0;
	break;
    case POWERMAC_CLASS_PDM:
	*deviceRegsLogicalBase = (LogicalAddress) POWERMAC_IO(PDM_FLOPPY_BASE_PHYS);
	*deviceRegsPhysicalBase = (PhysicalAddress) PDM_FLOPPY_BASE_PHYS;

	*DMARegsPhysicalBase = (PhysicalAddress) (PDM_FLOPPY_AMIC_BASE_PHYS);
	*DMARegsLogicalBase = POWERMAC_IO(*deviceRegsPhysicalBase);


	/*
	 * Allocate a track cache buffer out of system memory.
	 */

	// *trackCacheLogicalBase  = (LogicalAddress)  (powermac_info.dma_buffer_virt +
	// 	powermac_info.dma_buffer_size + PDM_DMA_BUFFER_FLOPPY_OFFSET);
	*trackCacheLogicalBase  = (LogicalAddress)  (powermac_info.dma_buffer_virt + PDM_DMA_BUFFER_FLOPPY_OFFSET);
	*trackCachePhysicalBase = (PhysicalAddress) (powermac_info.dma_buffer_phys + PDM_DMA_BUFFER_FLOPPY_OFFSET);
	// *trackCachePhysicalBase = (PhysicalAddress) kvtophys((vm_offset_t) *trackCacheLogicalBase);

	break;

    default:			/* nothing should fall into this catch-all... */

	*deviceRegsLogicalBase = (LogicalAddress) POWERMAC_IO(PCI_FLOPPY_BASE_PHYS);
	*deviceRegsPhysicalBase = (PhysicalAddress) PCI_FLOPPY_BASE_PHYS;

	*DMARegsLogicalBase = (LogicalAddress) DBDMA_REGMAP(DBDMA_FLOPPY);
	*DMARegsPhysicalBase = (PhysicalAddress) ((PCI_DMA_BASE_PHYS) + (DBDMA_FLOPPY << 8));

#if 0
	dprintf(DEBUG_VERBOSE, "Hardcoded addresses: unit %d: %x %x %x %x\n", unit,
	       *deviceRegsLogicalBase,
	       *deviceRegsPhysicalBase, *DMARegsLogicalBase,
	       *DMARegsPhysicalBase);

	for (i = 0, swims = find_devices("floppy"); swims; swims = swims->next, i++)
	    dprintf(DEBUG_VERBOSE, "device floppy:%d: %x %x\n", i, swims->addrs[0].address,
		   swims->addrs[1].address);
	for (i = 0, swims = find_devices("swim3"); swims; swims = swims->next, i++)
	    dprintf(DEBUG_VERBOSE, "device swim3:%d: %x %x\n", i, swims->addrs[0].address,
		   swims->addrs[1].address);
#endif
	/*
	 * Allocate a track cache buffer out of system memory.
	 */
#ifdef KERNEL_BUILTIN
	*trackCacheLogicalBase = (LogicalAddress) & TrackBuffer[unit];
#else
	*trackCacheLogicalBase = (LogicalAddress) TrackBuffer[unit];
#endif
	if (*trackCacheLogicalBase == NULL) {
	    printf("bsfloppy.c:Unable to create track cache memory ");
	    errorCode = FLOP_MEM_ERROR;
	}
	*trackCachePhysicalBase = (PhysicalAddress) TrackBufferPhys[unit];
		// kvtophys((vm_offset_t) * trackCacheLogicalBase);

	break;

    case POWERMAC_CLASS_PCI:
	for (i = 0, swims = find_devices("floppy"); i < unit && swims; i++, swims = swims->next);

	if (!swims)
	    for (swims = find_devices("swim3"); i < unit && swims; i++, swims = swims->next);

	if (!swims) {
	    errorCode = FLOP_ENODEV;
	    return errorCode;
	}
	*deviceRegsPhysicalBase = (PhysicalAddress) swims->addrs[0].address;
	*DMARegsPhysicalBase = (PhysicalAddress) (swims->addrs[1].address);
	//*DMARegsPhysicalBase    = (PhysicalAddress)((PCI_DMA_BASE_PHYS) + (swims->addrs[1].address << 8));
	if (!unit) {
	    *deviceRegsLogicalBase = (LogicalAddress) POWERMAC_IO(swims->addrs[0].address);
	    *DMARegsLogicalBase = (LogicalAddress) DBDMA_REGMAP((swims->addrs[1].address -
						PCI_DMA_BASE_PHYS) >> 8);
	} else {
	    *deviceRegsLogicalBase = (LogicalAddress) POWERMAC_IO2(swims->addrs[0].address);
	    *DMARegsLogicalBase = (LogicalAddress) DBDMA_REGMAP2((swims->addrs[1].address -
					     (PCI_DMA_BASE_PHYS2)) >> 8);
	}

#endif // 0
	/*
	 * Allocate a track cache buffer out of system memory.
	 */
#ifdef KERNEL_BUILTIN
	*trackCacheLogicalBase = (LogicalAddress) & TrackBuffer[unit];
#else
	*trackCacheLogicalBase = (LogicalAddress) TrackBuffer[unit];
#endif
	if (*trackCacheLogicalBase == NULL) {
	    printf("bsfloppy.c:Unable to create track cache memory ");
	    errorCode = FLOP_MEM_ERROR;
	}
	// *trackCachePhysicalBase = (PhysicalAddress) kvtophys((vm_offset_t) * trackCacheLogicalBase);
	*trackCachePhysicalBase = (PhysicalAddress) TrackBufferPhys[unit];
		// kvtophys((vm_offset_t) * trackCacheLogicalBase);

#if 0
    }
#endif


    *deviceRegsPhysicalBase = (PhysicalAddress) org_mklinux_swim3_vaddr[unit];
    *deviceRegsLogicalBase = (LogicalAddress) org_mklinux_swim3_vaddr[unit];
    *DMARegsLogicalBase = (LogicalAddress) org_mklinux_swim3_dmvaddr[unit];
    *DMARegsPhysicalBase = (LogicalAddress) org_mklinux_swim3_dmvaddr[unit];


	dprintf(DEBUG_VERBOSE, "device floppy%d: %x %x %x %x\n", unit,
	        (unsigned int)*deviceRegsLogicalBase,
	        (unsigned int)*deviceRegsPhysicalBase,
	        (unsigned int)*DMARegsLogicalBase,
	        (unsigned int)*DMARegsPhysicalBase);
	// printf("FCR= 0x%x",*(int *)(POWERMAC_IO(powermac_info.io_base_phys +
		// 0x38)));

    // IOLog("trackbuflogic=0x%04x,phys=0x%04x ", *trackCacheLogicalBase, *trackCachePhysicalBase);
    dprintf(DEBUG_VERBOSE, "trackbuflogic=0x%04x,phys=0x%04x ",
	    (unsigned int)*trackCacheLogicalBase,
	    (unsigned int)*trackCachePhysicalBase);
#ifdef PDMSUPPORT
    if (powermac_info.class == POWERMAC_CLASS_PDM)
	dprintf(DEBUG_VERBOSE, "dma_buffer_phys = 0x%x, dma_buffer_virt = 0x%x\n", powermac_info.dma_buffer_phys,
	    powermac_info.dma_buffer_virt);
#endif // PDMSUPPORT

#if 0
    errorCode = MapCacheForDMAIO(*trackCacheLogicalBase,
				 0x0B000,
				 &logCacheBase,
				 &phyCacheBase);

//                              *trackCacheLogicalBase  = logCacheBase;
    //                              *trackCachePhysicalBase = phyCacheBase;




#endif
    return errorCode;
}


OSStatus FloppyPluginReinit(int unit, BSStorePtr initStore)
{
    OSStatus errorCode;
    LogicalAddress deviceRegsLogicalBase;
    PhysicalAddress deviceRegsPhysicalBase;
    LogicalAddress DMARegsLogicalBase;
    PhysicalAddress DMARegsPhysicalBase;
    LogicalAddress trackCacheLogicalBase;
    PhysicalAddress trackCachePhysicalBase;
    struct BSStoreMPIComponent componentInfo;

        errorCode = GetFloppyHardwareAddresses(unit,
                               (RegEntryID *) & componentInfo.sourceNode,
                                               &deviceRegsLogicalBase,
                                               &deviceRegsPhysicalBase,
                                               &DMARegsLogicalBase,
                                               &DMARegsPhysicalBase,
                                               &trackCacheLogicalBase,
                                               &trackCachePhysicalBase);
        if (errorCode != E_BSSuccess) {
                dprintf((DEBUG_VERBOSE | DEBUG_GENERAL),
			"GetFloppyHardwareAddress call failed.\n");
        } else {
                if ((errorCode = InitializeDrive(unit, 1,
                                     deviceRegsLogicalBase,
                                     DMARegsLogicalBase,
                                     DMARegsLogicalBase,
                                     trackCacheLogicalBase,
                                     trackCachePhysicalBase,
                                     0x0B000,
                                     &driveStatus[unit])) == noErr) {
                        driveStatus[unit]->OSDiskID = initStore;
                        driveStatus[unit]->unit = unit;
                }
        }
	return errorCode;
}

/************************************************************************
*
*	FUNCTION : FloppyPluginInit
*
************************************************************************/

OSStatus
FloppyPluginInit(int unit, BSStorePtr initStore)
{

    OSStatus errorCode;
    LogicalAddress deviceRegsLogicalBase;
    PhysicalAddress deviceRegsPhysicalBase;
    LogicalAddress DMARegsLogicalBase;
    PhysicalAddress DMARegsPhysicalBase;
    LogicalAddress trackCacheLogicalBase;
    PhysicalAddress trackCachePhysicalBase;
    struct BSStoreMPIComponent componentInfo;
    int i;

    dprintf(DEBUG_VERBOSE, "FloppyPluginInit called for unit %d\n", unit);
    
    // for (i=0; i<NFD; i++) {
    i = unit;
        TrackBuffer[i] = (char *)IOMallocContiguous(0xb000, PAGE_SIZE,
	    &TrackBufferPhys[i]);
	org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "IOMallocContiguous\n");
    // }


//      DebugStr( (ConstStr255Param)"\pFloppyPluginInit: Swim3 plugin match made." );

    /*
     * Assume success.
     */

    errorCode = E_BSSuccess;

//      BSStoreGetComponent( initStore,
//    0,
//                                               &componentInfo );

    /*
     * Get the hardware addresses for the SWIM3.
     */
	errorCode = GetFloppyHardwareAddresses(unit,
			       (RegEntryID *) & componentInfo.sourceNode,
					       &deviceRegsLogicalBase,
					       &deviceRegsPhysicalBase,
					       &DMARegsLogicalBase,
					       &DMARegsPhysicalBase,
					       &trackCacheLogicalBase,
					       &trackCachePhysicalBase);

	if (errorCode != E_BSSuccess) return errorCode;

    /*
     * Register our ISR with the system.
     */

//      RegisterFloppyISR( (RegEntryID *)&componentInfo.sourceNode,
    //HALISRHandler );

    dprintf(DEBUG_VERBOSE, "floppy: hardware addresses: %lx %lx %lx %lx\n", (unsigned long)deviceRegsLogicalBase,
	(unsigned long)deviceRegsPhysicalBase, (unsigned long)DMARegsLogicalBase, (unsigned long)DMARegsPhysicalBase);
    dprintf(DEBUG_VERBOSE, "floppy: track buffer addresses: %lx %lx\n", (unsigned long)trackCacheLogicalBase,
	   (unsigned long)trackCachePhysicalBase);


    /*
     * Initialize the the floppy format tables.
     */

    InitFormatTable();

    dprintf(DEBUG_VERBOSE, "floppy: format table initted.\n");

    /*
     * Now check to see which drives are installed, and setup
     * its variables as needed.
     */
    if ((errorCode = InitializeDrive(unit, 1,
				     deviceRegsLogicalBase,
				     DMARegsLogicalBase,
				     DMARegsLogicalBase,
				     trackCacheLogicalBase,
				     trackCachePhysicalBase,
				     0x0B000,
				     &driveStatus[unit])) == noErr) {

	/*
	 * Store away the Maxwell system identifier for this disk drive.
	 */

	driveStatus[unit]->OSDiskID = initStore;
	driveStatus[unit]->unit = unit;
	driveStatus[unit]->isFormatted = -1;

	/*
	 * Startup the task that is needed to watch for media to be
	 * inserted or ejected.
	 */

#ifndef MKLINUX_CODE
/* @@@ Maybe MkLinux didn't do this? @@@ */
//              errorCode = LaunchMediaScanTask();
#endif

    }
    dprintf(DEBUG_VERBOSE, "PluginInit returning.\n");

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginCleanup
*
************************************************************************/

OSStatus FloppyPluginCleanup(int unit)
{

#pragma unused( theStore )

//      DebugStr( (ConstStr255Param)"\pFloppyPluginCleanup: Swim3 plugin match made." );

    DeleteOSHardwareLockResources(driveStatus[unit]->OSHardwareLockID);

    return E_BSSuccess;

}

/************************************************************************
*
*	FUNCTION : FloppyPluginIO
*
************************************************************************/

BSIOStatus
FloppyPluginIO(int unit, BSStorePtr ioStore,
	       BSBlockListDescriptorRef blocks,
	       MemListDescriptorRef memory,
	       BSIORequestBlockPtr parentRequest,
	       OptionBits options,
	       struct BSErrorList ** errors)
{


    BSIOStatus BSErrorCode;
    OSStatus IOErrorCode = noErr;
    DriveStatusType *DriveStatus;
    long transferByteCount = 0;
#if 0
    BSByteCount startingByte;
    ByteCount blocksExtentLen;
#endif


//      DebugStr( (ConstStr255Param)"\pFloppyPluginIO: Swim3 plugin match made." );

    BSErrorCode = noErr;
    /*
     * Make sure we have a valid drive number.
     */
    dprintf(DEBUG_VERBOSE, "bsfloppy.c:FloppyPlugin:calling CheckDriveNumber ");
    if ((IOErrorCode = CheckDriveNumber(unit, 1,
					&DriveStatus)) == noErr) {

	/*
	 * Disable the Diskette Scan task while we're acting on a request.
	 */

	EnterHardwareLockSection(DriveStatus->OSHardwareLockID);
	SWIMIIIFixGlobals(DriveStatus);

	/*
	 * Say which kind of DRVR call we're executing.
	 */

	DriveStatus->DRVROperation = DRVRPrimeCC;

	/*
	 * Decipher what operation we're going to do, and execute
	 * the function.
	 */

	if (options == kBSRead)
	    DriveStatus->currentCommand = prmReadCode;

	if (options == kBSWrite)
	    DriveStatus->currentCommand = prmWriteCode;

	/*
	 * Decide if this is a regular request, or a read/write and verify.
	 */

	DriveStatus->commandModifier = 0;

	/*
	 * Get the absolute starting block number to read as long as there
	 * are no errors.
	 */

/* naga commented to avoid infinite loop */
//              while( ( IOErrorCode == noErr ) &&
	//                         ( BSErrorCode = BSBlockListDescriptorGetExtent( blocks,
	//                                                                                                                         0,
	//                                                                                                                         &startingByte,
	//                                                                                                                         &blocksExtentLen ) ) == E_BSSuccess )
	//                          BSBlockListDescriptorGetExtent( blocks,
	//                                                                                                                         0,
	//                                                                                                                         &startingByte,
	//                                                                                                                         &blocksExtentLen ) ;
	{
		// int i, count = 0;
	    dprintf(DEBUG_VERBOSE, "offset=%d,count=%d ", parentRequest, blocks);
	    DriveStatus->firstBlockNumber = parentRequest / 512;	//startingByte / 512;

	    DriveStatus->blockCount = blocks / 512;	// blocksExtentLen / 512;

	    /*
	     * Get the scatter/gather list of the clients buffer.
	     */

	    DriveStatus->transferSGList = memory;

	    /*
	     * Call the requested operation.
	     */

	    switch (DriveStatus->currentCommand) {
	    case prmReadCode:
		dprintf(DEBUG_VERBOSE, "bsfloppy.c:PluginIO:calling ReadBlocks ");
		/* DANGER, WILL ROBINSON!  Enabling this debug code will
		   cause massive failures.  The ReadBlocks() function
		   shortcuts the actual reading of the disk if a block
		   is already stored in cache.  The change to the
		   TrackBuffer will always be zero except for the first
		   read from the track, which will cause lots of really
		   bizarre errors if this code is used beyond the
		   very early stages of DMA testing!  --DAG */
                                // for(i=0;i<0xb000;++i)TrackBuffer[unit][i]='Z';
		// blowcache();
		IOErrorCode = ReadBlocks(DriveStatus,
					 &transferByteCount);
		// blowcache();
		dprintf(DEBUG_VERBOSE, "bsfloppy.c:PluginIO:ReadBlocks ret=%d,readcount=%ld ", IOErrorCode, transferByteCount);
		{
      // for (i=0;i<0xb000;++i) if(TrackBuffer[unit][i] != 'Z')count++;
		          // printf("Track buffer change=%d ",count);
		}
		break;

	    case prmWriteCode:
		dprintf(DEBUG_VERBOSE, "writeprotect=%d ", DriveStatus->writeProt);
//#define printf donone                                         
#if 0
		// dg deleted  ;-)
		DriveStatus->writeProt = false;		//naga  added
#endif
		if (DriveStatus->writeProt) {
#if 0
		    dprintf(DEBUG_VERBOSE, "wrt:call record err ");
#endif
		    dprintf(DEBUG_VERBOSE, "Floppy Write: Disk Write Protected!\n");
		    IOErrorCode = RecordError(wPrErr);

		} else {
		    dprintf(DEBUG_VERBOSE, "call wrtblks ");
		    IOErrorCode = WriteBlocks(DriveStatus,
					      &transferByteCount);

		}

		break;

	    }

	}

	/*
	 * Did we successfully get to the end of the requested block list,
	 * or did we terminate early?
	 */

	if ((BSErrorCode == E_BSSuccess) ||
	    (BSErrorCode == E_BSBLEndOfList)) {

	    /*
	     * Did all of the IO requests successfully complete?
	     */

	    if (IOErrorCode == noErr) {

		/*
		 * Return a Block Storage style return code to say that we
		 * completed the IO request successfully.
		 */

		BSErrorCode = kBSIOCompleted;

	    } else {

		/*
		 * Return our error back to BlockStorage.
		 */

		BSErrorCode = IOErrorCode;

	    }

	}
	/*
	 * Re-enable the Diskette Scan task.
	 */

	ExitHardwareLockSection(DriveStatus->OSHardwareLockID);

    }
    *ioStore = transferByteCount;	//naga copy to Unix  

    return BSErrorCode;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginFlush
*
************************************************************************/

BSIOStatus
FloppyPluginFlush(int unit,
		  BSIORequestBlockPtr parentRequest,
		  struct BSErrorList ** errors)
{

    OSStatus errorCode;

#pragma unused( ioStore )
#pragma unused( parentRequest )
#pragma unused( errors )


//      DebugStr( (ConstStr255Param)"\pFloppyPluginFlush: Swim3 plugin match made." );

    /*
     * Disable the Diskette Scan task while we're acting on a request.
     */

    EnterHardwareLockSection(driveStatus[unit]->OSHardwareLockID);
    SWIMIIIFixGlobals(driveStatus[unit]);

    /*
     * Flush any stale data in the track cache.
     */

    errorCode = FlushTrackCache(driveStatus[unit]);

    /*
     * Re-enable the Diskette Scan task.
     */

    ExitHardwareLockSection(driveStatus[unit]->OSHardwareLockID);

    if (errorCode == noErr) {

	/*
	 * Return a Block Storage style return code to say that we
	 * completed the IO request successfully.
	 */

	errorCode = kBSIOCompleted;

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginAddComponent
*
************************************************************************/

#if 0
static OSStatus
 FloppyPluginAddComponent(BSStorePtr destStore,
			  struct BSStoreMPIComponent *newComponent,
			  struct BSStoreInfo *storeNewInfo)
{

#pragma unused( destStore )
#pragma unused( newComponent )
#pragma unused( storeNewInfo )

//      DebugStr( (ConstStr255Param)"\pFloppyPluginAddComponent: Swim3 plugin match made." );

    return E_BSSuccess;

}
#endif


/************************************************************************
*
*	FUNCTION : FloppyPluginGotoState
*
************************************************************************/

OSStatus
FloppyPluginGotoState(BSStorePtr theStore,
		      BSAccessibilityState gotoState)
{

#pragma unused( theStore )

    OSStatus errorCode;


//      DebugStr( (ConstStr255Param)"\pFloppyPluginGotoState: Swim3 plugin match made." );

    errorCode = 0;

    if ((gotoState == kBSOnline) ||
	(gotoState == kBSOffline)) {

	/*
	 * Disable the Diskette Scan task while we're acting on a request.
	 */

#if 0				// annoying eject on unmount....


	EnterHardwareLockSection(myDriveStatus->OSHardwareLockID);
	SWIMIIIFixGlobals(myDriveStatus);
	errorCode = EjectDisk(myDriveStatus);

	/*
	 * Re-enable the Diskette Scan task.
	 */

	ExitHardwareLockSection(myDriveStatus->OSHardwareLockID);

#endif

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginEject
*
************************************************************************/

OSStatus
FloppyPluginEject(int unit,
		  BSAccessibilityState gotoState)
{

#pragma unused( theStore )

    OSStatus errorCode;


//      DebugStr( (ConstStr255Param)"\pFloppyPluginGotoState: Swim3 plugin match made." );

    errorCode = 0;

    if ((gotoState == kBSOnline) ||
	(gotoState == kBSOffline)) {

	/*
	 * Disable the Diskette Scan task while we're acting on a request.
	 */


	EnterHardwareLockSection(driveStatus[unit]->OSHardwareLockID);
	SWIMIIIFixGlobals(driveStatus[unit]);
	errorCode = EjectDisk(driveStatus[unit]);

	/*
	 * Re-enable the Diskette Scan task.
	 */

	ExitHardwareLockSection(driveStatus[unit]->OSHardwareLockID);

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginFormatMedia
*
************************************************************************/

OSStatus
 FloppyPluginFormatMedia(int unit, BSStorePtr formatStore,
			 BSFormatIndex formatType)
{
    BSIOStatus BSErrorCode;
    OSStatus IOErrorCode = noErr;
    DriveStatusType *DriveStatus;

    BSErrorCode = noErr;
    /*
     * Make sure we have a valid drive number.
     */
    dprintf(DEBUG_VERBOSE, "bsfloppy.c:FloppyPluginFormatMedia:calling CheckDriveNumber ");
    if ((IOErrorCode = CheckDriveNumber(unit, 1,
                                        &DriveStatus)) == noErr) {

        /*
         * Disable the Diskette Scan task while we're acting on a request.
         */

        EnterHardwareLockSection(DriveStatus->OSHardwareLockID);
        SWIMIIIFixGlobals(DriveStatus);

        /*
         * Say which kind of DRVR call we're executing.
         */
	DriveStatus->DRVROperation = DRVRControlCC;
	DriveStatus->currentCommand = formatCC;
	DriveStatus->commandModifier = 0;

	// if (RecalDrive(DriveStatus) != noErr) {
		// printf("Warning: Recalibrate failed.\n");
	// }

	if (DriveStatus->writeProt) {
		IOErrorCode = RecordError(wPrErr);
	} else {
		IOErrorCode = FormatDisk(DriveStatus, formatType);
	}

       	if (IOErrorCode == noErr) {
	    BSErrorCode = kBSIOCompleted;
	} else {
	    BSErrorCode = IOErrorCode;
	}

	ExitHardwareLockSection(DriveStatus->OSHardwareLockID);

    } else {
	BSErrorCode = kBSIONotStarted;
    }
    return BSErrorCode;
}


/************************************************************************
*
*	FUNCTION : FloppyPluginGetInfo
*
************************************************************************/

/* Was static.  making public */
OSStatus
 FloppyPluginGetInfo(int unit,
		     struct BSStoreMPIInfo *info)
{
    /* Override this global variable locally */
    DriveStatusType *myDriveStatus;
    short errorCode; 

    myDriveStatus = driveStatus[unit];

#pragma unused( infoStore )
#pragma unused( info )

//      DebugStr( (ConstStr255Param)"\pFloppyPluginGetInfo: Swim3 plugin match made." );

    /*
     * Tell Block Storage what to call us in the Name Registry.
     */

    strcpy(info->name, "Floppy-0-0");

    info->isEjectable = true;
    info->hasAutoEjectHardware = true;

    /*
     * Is there a diskette in the drive?
     */

    if (myDriveStatus->installed == DRV_NOT_INSTALLED ||
	myDriveStatus->diskInPlace == NO_DISKETTE) {

	info->curState = kBSOffline;

	/*
	 * No diskette in drive, so we don't know.
	 */

	info->storeSize = 0;
	info->readBlockSize = 0;
	info->writeBlockSize = 0;
	info->isWriteable = false;
	info->isFormattable = false;
	info->isPartitionable = false;
	info->isFilesystem = false;
	info->isFormatted = false;

    } else {

	/*
	 * Disable the Diskette Scan task while we're acting on a request.
	 */


	EnterHardwareLockSection(myDriveStatus->OSHardwareLockID);
	SWIMIIIFixGlobals(myDriveStatus);
	/*
	 * Tell BlockStorage we're online and ready to be used.
	 */

	info->curState = kBSOnline;

	/*
	 * Powering up the drive will go out and characterize the drive and media,
	 * and then set the current format table entry.
	 */

	/*
	 * Set the drive's status to be a read operation so that powering
	 * up the drive doesn't wait an unnecessarily long time.
	 */
	myDriveStatus->DRVROperation = DRVRPrimeCC;
	myDriveStatus->currentCommand = prmStatCode;

	if ((errorCode = PowerDriveUp(myDriveStatus)) == noNybErr) {
	    /* What we have here is a failure to communicate. */
	    info->isFormatted = false;
	    myDriveStatus->isFormatted = false;
	} else {
	    info->isFormatted = myDriveStatus->isFormatted;
	}

	/*
	 * Return the stats on the injected diskette.
	 */

	info->storeSize = myDriveStatus->diskFormat.drvBlockMax * myDriveStatus->diskFormat.logicalSectorSize;
	info->readBlockSize = myDriveStatus->diskFormat.logicalSectorSize;
	info->writeBlockSize = myDriveStatus->diskFormat.logicalSectorSize;

	/*
	 * Signal the file system to mount the diskette and
	 * a hint as to how.
	 */

	info->isFilesystem = true;
	info->isPartitionable = false;

	/*
	 * We're finished with the request, so schedule the drive
	 * to be powered down in 6 VBL scan times (3 seconds).
	 */

	PowerDriveDown(myDriveStatus,
		       6);

	/*
	 * See if the diskette is write protected.
	 */

	if (myDriveStatus->writeProt) {

	    info->isWriteable = false;
	    info->isFormattable = false;

	} else {

	    info->isWriteable = true;
	    info->isFormattable = true;

	}

	/*
	 * Re-enable the Diskette Scan task.
	 */

	ExitHardwareLockSection(myDriveStatus->OSHardwareLockID);
    }

    return E_BSSuccess;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginioCompletion
*
************************************************************************/

#if 0
static OSStatus
 FloppyPluginioCompletion(BSStorePtr theStore,
			  void *finishedPrivateData,
			  BSErrorListPtr returnedBSErrorList,
			  OSStatus returnedStatus,
			  BSErrorListPtr * errorListPtrPtr)
{


    return E_BSSuccess;

}
#endif
void PrintRegs(int unit)
{
#ifdef DO_PRINT_REGS
    GRCSwimIIIRegs *gsPtr;
    unsigned char er, mo, hs, se, ns;
#ifdef REALLYFREAKINGSLOWANDNOTTHATUSEFUL
    unsigned char cs, i;
#endif // REALLYFREAKINGSLOWANDNOTTHATUSEFUL
/* This is useful only for debugging, as it will ensure that
   no transaction ever completes by siphoning off the interrupt. */
// #define WILLCAUSEFAILURES
#ifdef WILLCAUSEFAILURES
    unsigned char intr;
#endif // WILLCAUSEFAILURES

#if 0
    if (powermac_info.class = POWERMAC_CLASS_PCI)
	gsPtr = (GRCSwimIIIRegs *) POWERMAC_IO(PCI_FLOPPY_BASE_PHYS);
    else
	gsPtr = (GRCSwimIIIRegs *) POWERMAC_IO(PDM_FLOPPY_BASE_PHYS);
#else

    gsPtr = (GRCSwimIIIRegs *)org_mklinux_swim3_vaddr[unit];
#endif
    dprintf(DEBUG_REGS, "Regs for SWIM3 Unit %d\n", unit);
    if (!gsPtr) {
	dprintf(DEBUG_REGS, "No Device Available.\n");
	return;
    }

    er = gsPtr->rError;
    SynchronizeIO(); delay(10);
    mo = gsPtr->rModewModeZeroes;
    SynchronizeIO(); delay(10);
    hs = gsPtr->rHandshakewModeOnes;
    SynchronizeIO(); delay(10);
    se = gsPtr->rwSetup;
    SynchronizeIO(); delay(10);
    ns = gsPtr->rwSectorCount;
    SynchronizeIO(); delay(10);
#ifdef WILLCAUSEFAILURES
    intr = gsPtr->rInterrupt;
    dprintf(DEBUG_REGS, " FD INTR STATUS %d\n", intr);
#endif // WILLCAUSEFAILURES
       dprintf(DEBUG_REGS, " Regs error=0x%x,mode=0x%x,hshake=0x%x,setup=0x%x,nsect=0x%x ",er,mo,hs,se,ns);
#ifdef REALLYFREAKINGSLOWANDNOTTHATUSEFUL
    for (i = 0; i < 50; ++i) {

	cs = gsPtr->rCurrentSector;
	SynchronizeIO(); delay(10);
     dprintf(DEBUG_REGS, "cs=0x%x ",cs);
	FloppyTimedSleep(110);
    }
#endif // REALLYFREAKINGSLOWANDNOTTHATUSEFUL
#endif // DO_PRINT_REGS
}
#endif // NFD > 0
