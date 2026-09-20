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
#include <types.h>
#include <kern/spl.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include <sys/ioctl.h>
#include <kern/kalloc.h>
#include <ppc/pmap.h>
#include "portdef.h"
#include "floppypriv.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "floppysal.h"
#include <ppc/POWERMAC/dbdma.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/device_tree.h>
#include <fd.h>
#include "swimiii.h"

#if NFD > 0

// #define printf donone
/************************************************************************
*
* Global data for driver.
*
************************************************************************/

char TrackBuffer[NFD][0xb000];
extern BSIOStatus BSBlockListDescriptorGetExtent(BSBlockListDescriptorRef blocks,
						 int ignore,
					      BSByteCount * startingByte,
					    ByteCount * blocksExtentLen);
extern
SonyVarsType SonyVariables[];


DriveStatusType *myDriveStatus;

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

static OSStatus
 FloppyPluginCleanup(BSStorePtr theStore);
BSIOStatus
FloppyPluginIO(int unit, BSStorePtr ioStore,
	       BSBlockListDescriptorRef blocks,
	       MemListDescriptorRef memory,
	       BSIORequestBlockPtr parentRequest,
	       OptionBits options,
	       struct BSErrorList **errors);

BSIOStatus
FloppyPluginFlush(BSStorePtr ioStore,
		  BSIORequestBlockPtr parentRequest,
		  struct BSErrorList **errors);

static OSStatus
 FloppyPluginAddComponent(BSStorePtr destStore,
			  struct BSStoreMPIComponent *newComponent,
			  struct BSStoreInfo *storeNewInfo);

OSStatus
FloppyPluginGotoState(BSStorePtr theStore,
		      BSAccessibilityState gotoState);

OSStatus
FloppyPluginEject(BSStorePtr theStore,
		  BSAccessibilityState gotoState);

static OSStatus
 FloppyPluginFormatMedia(BSStorePtr formatStore,
			 BSFormatIndex formatType);

static OSStatus
 FloppyPluginGetInfo(BSStorePtr infoStore,
		     struct BSStoreMPIInfo *info);

static OSStatus
 FloppyPluginioCompletion(BSStorePtr theStore,
			  void *finishedPrivateData,
			  BSErrorListPtr returnedBSErrorList,
			  OSStatus returnedStatus,
			  BSErrorListPtr * errorListPtrPtr);




/************************************************************************
*
*	FUNCTION : FloppyPluginDeviceExamine
*
************************************************************************/

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
    UInt32 i;
    LogicalAddress logCacheBase;
    PhysicalAddress phyCacheBase;
    device_node_t *swims = NULL;


    /*
     * Assume success.
     */

    errorCode = noErr;

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
	printf("Hardcoded addresses: unit %d: %x %x %x %x\n", unit,
	       *deviceRegsLogicalBase,
	       *deviceRegsPhysicalBase, *DMARegsLogicalBase,
	       *DMARegsPhysicalBase);

	for (i = 0, swims = find_devices("floppy"); swims; swims = swims->next, i++)
	    printf("device floppy:%d: %x %x\n", i, swims->addrs[0].address,
		   swims->addrs[1].address);
	for (i = 0, swims = find_devices("swim3"); swims; swims = swims->next, i++)
	    printf("device swim3:%d: %x %x\n", i, swims->addrs[0].address,
		   swims->addrs[1].address);
#endif
	/*
	 * Allocate a track cache buffer out of system memory.
	 */
	*trackCacheLogicalBase = (LogicalAddress) & TrackBuffer[unit];
	if (*trackCacheLogicalBase == NULL) {
	    printf("bsfloppy.c:Unable to create track cache memory ");
	    errorCode = FLOP_MEM_ERROR;
	}
	*trackCachePhysicalBase = (PhysicalAddress) kvtophys((vm_offset_t) * trackCacheLogicalBase);

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

#if MACH_DEBUG
	printf("device floppy%d: %x %x %x %x\n", unit, *deviceRegsLogicalBase,
	       *deviceRegsPhysicalBase, *DMARegsLogicalBase,
	       *DMARegsPhysicalBase);
	// printf("FCR= 0x%x",*(long *)(POWERMAC_IO(powermac_info.io_base_phys +
		// 0x38)));
#endif
	/*
	 * Allocate a track cache buffer out of system memory.
	 */
	*trackCacheLogicalBase = (LogicalAddress) & TrackBuffer[unit];
	if (*trackCacheLogicalBase == NULL) {
	    printf("bsfloppy.c:Unable to create track cache memory ");
	    errorCode = FLOP_MEM_ERROR;
	}
	*trackCachePhysicalBase = (PhysicalAddress) kvtophys((vm_offset_t) * trackCacheLogicalBase);

    }



#if MACH_DEBUG
    printf("trackbuflogic=0x%04x,phys=0x%04x ", *trackCacheLogicalBase, *trackCachePhysicalBase);
    if (powermac_info.class == POWERMAC_CLASS_PDM)
	printf("dma_buffer_phys = 0x%x, dma_buffer_virt = 0x%x\n", powermac_info.dma_buffer_phys,
	    powermac_info.dma_buffer_virt);
#endif

    errorCode = MapCacheForDMAIO(*trackCacheLogicalBase,
				 0x0B000,
				 &logCacheBase,
				 &phyCacheBase);

//                              *trackCacheLogicalBase  = logCacheBase;
    //                              *trackCachePhysicalBase = phyCacheBase;




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

#if MACH_DEBUG
    printf("floppy: hardware addresses: %x %x %x %x\n", deviceRegsLogicalBase,
	deviceRegsPhysicalBase, DMARegsLogicalBase, DMARegsPhysicalBase);
    printf("floppy: track buffer addresses: %x %x\n", trackCacheLogicalBase,
	   trackCachePhysicalBase);
#endif


    /*
     * Initialize the the floppy format tables.
     */

    InitFormatTable();

#if MACH_DEBUG
    printf("floppy: format table initted.\n");
#endif

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
				     &myDriveStatus)) == noErr) {

	/*
	 * Store away the Maxwell system identifier for this disk drive.
	 */

	myDriveStatus->OSDiskID = initStore;

	/*
	 * Startup the task that is needed to watch for media to be
	 * inserted or ejected.
	 */

//              errorCode = LaunchMediaScanTask();

    }
#if MACH_DEBUG
    printf("PluginInit returning.\n");
#endif

    return errorCode;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginCleanup
*
************************************************************************/

static OSStatus
 FloppyPluginCleanup(BSStorePtr theStore)
{

#pragma unused( theStore )

//      DebugStr( (ConstStr255Param)"\pFloppyPluginCleanup: Swim3 plugin match made." );

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
    BSByteCount startingByte;
    ByteCount blocksExtentLen;


//      DebugStr( (ConstStr255Param)"\pFloppyPluginIO: Swim3 plugin match made." );

    /*
     * Make sure we have a valid drive number.
     */
#if MACH_DEBUG
    printf("bsfloppy.c:FloppyPlugin:calling CheckDriveNumber ");
#endif
    if ((IOErrorCode = CheckDriveNumber(unit, 1,
					&DriveStatus)) == noErr) {

	/*
	 * Disable the Diskette Scan task while we're acting on a request.
	 */

	DriveStatus->OSHardwareLockID = EnterHardwareLockSection();

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
#if MACH_DEBUG
	    printf("offset=%d,count=%d ", parentRequest, blocks);
#endif
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

		int i;
	    case prmReadCode:
#if MACH_DEBUG
		printf("bsfloppy.c:PluginIO:calling ReadBlocks ");
#endif
//                                for(i=0;i<0xb000;++i)TrackBuffer[unit][i]='Z';
		IOErrorCode = ReadBlocks(DriveStatus,
					 &transferByteCount);
#if MACH_DEBUG
		printf("bsfloppy.c:PluginIO:ReadBlocks ret=%d,readcount=%ld ", IOErrorCode, transferByteCount);
#endif
		{
		    int i, count = 0;
//      for (i=0;i<0xb000;++i) if(TrackBuffer[unit][i] != 'z')count++;
		    //      printf("Track buffer change=%d ",count);
		}
		break;

	    case prmWriteCode:
#if MACH_DEBUG
		printf("writeprotect=%d ", DriveStatus->writeProt);
#endif
//#define printf donone                                         
#if 0
		// dg deleted  ;-)
		DriveStatus->writeProt = false;		//naga  added
#endif
		if (DriveStatus->writeProt) {
#if 0
		    printf("wrt:call record err ");
#endif
		    printf("Floppy Write: Disk Write Protected!\n");
		    IOErrorCode = RecordError(wPrErr);

		} else {
#if MACH_DEBUG
		    printf("call wrtblks ");
#endif
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
FloppyPluginFlush(BSStorePtr ioStore,
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

    myDriveStatus->OSHardwareLockID = EnterHardwareLockSection();

    /*
     * Flush any stale data in the track cache.
     */

    errorCode = FlushTrackCache(myDriveStatus);

    /*
     * Re-enable the Diskette Scan task.
     */

    ExitHardwareLockSection(myDriveStatus->OSHardwareLockID);

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


	myDriveStatus->OSHardwareLockID = EnterHardwareLockSection();
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
FloppyPluginEject(BSStorePtr theStore,
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


	myDriveStatus->OSHardwareLockID = EnterHardwareLockSection();
	errorCode = EjectDisk(myDriveStatus);

	/*
	 * Re-enable the Diskette Scan task.
	 */

	ExitHardwareLockSection(myDriveStatus->OSHardwareLockID);

    }
    return errorCode;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginFormatMedia
*
************************************************************************/

static OSStatus
 FloppyPluginFormatMedia(BSStorePtr formatStore,
			 BSFormatIndex formatType)
{

#pragma unused( formatStore )
#pragma unused( formatType )

//      DebugStr( (ConstStr255Param)"\pFloppyPluginFormatMedia: Swim3 plugin match made." );

    return E_BSSuccess;

}


/************************************************************************
*
*	FUNCTION : FloppyPluginGetInfo
*
************************************************************************/

static OSStatus
 FloppyPluginGetInfo(BSStorePtr infoStore,
		     struct BSStoreMPIInfo *info)
{

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

    if (myDriveStatus->diskInPlace == NO_DISKETTE) {

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

    } else {

	/*
	 * Disable the Diskette Scan task while we're acting on a request.
	 */


	myDriveStatus->OSHardwareLockID = EnterHardwareLockSection();
	/*
	 * Tell BlockStorage we're online and ready to be used.
	 */

	info->curState = kBSOnline;

	/*
	 * Powering up the drive will go out and characterize the drive and media,
	 * and then set the current format table entry.
	 */

	PowerDriveUp(myDriveStatus);

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

static OSStatus
 FloppyPluginioCompletion(BSStorePtr theStore,
			  void *finishedPrivateData,
			  BSErrorListPtr returnedBSErrorList,
			  OSStatus returnedStatus,
			  BSErrorListPtr * errorListPtrPtr)
{


    return E_BSSuccess;

}
void PrintRegs()
{
    GRCSwimIIIRegs *gsPtr;
    unsigned char er, mo, hs, se, cs, i;

    if (powermac_info.class = POWERMAC_CLASS_PCI)
	gsPtr = (GRCSwimIIIRegs *) POWERMAC_IO(PCI_FLOPPY_BASE_PHYS);
    else
	gsPtr = (GRCSwimIIIRegs *) POWERMAC_IO(PDM_FLOPPY_BASE_PHYS);

    er = gsPtr->rError;
    SynchronizeIO();
    mo = gsPtr->rModewModeZeroes;
    SynchronizeIO();
    hs = gsPtr->rHandshakewModeOnes;
    SynchronizeIO();
    se = gsPtr->rwSetup;
    SynchronizeIO();
    //   printf(" Regs error=0x%x,mode=0x%x,hshake=0x%x,setup=0x%x ",er,mo,hs,se);
    for (i = 0; i < 50; ++i) {

	cs = gsPtr->rCurrentSector;
	SynchronizeIO();
//     printf("cs=0x%x ",cs);
	delay(110000);
    }
}
#endif // NFD > 0
