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

/*
   File:                DBDMASupport.c

   Contains:    Hardware interface functions for the Apple DB DMA fucntionality
 */


//#include <MemAllocatorsPriv.h>
//#include "PCI.h"                                              // EndianSwap32Bit Used in MakeCCDescriptor
//#include "DriverSupport.h"

// Private types.
#include <types.h>
#include <sys/ioctl.h>
#include <kern/spl.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include <ppc/io_map_entries.h>
#include <sys/ioctl.h>
#include <kern/kalloc.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/pmap.h>
#include "portdef.h"
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#include "dbdma_maxwell.h"

#include <fd.h>

#if NFD > 0
#ifdef OLD_DBDMA

#define	DBDMA_FLOPPY		0x1
struct PrivDBDMAChannel {
    KernelProcessID owningProcessID;
    DBDMAChannelRegisters *DBDMAPtr;
    UInt32 state;
    AreaID cclAreaID;
    LogicalAddress cclPtr;
    LogicalAddress cclStaticLogicalAddr;
    PhysicalAddress cclPhysicalAddr;
    DBDMADescriptor *cclControlCommand0;
    DBDMADescriptor *cclControlCommand1;
    PhysicalAddress cclOutputCommand0;
    PhysicalAddress cclOutputCommand1;
};
typedef struct PrivDBDMAChannel PrivDBDMAChannel, *PrivDBDMAChannelPtr;
PrivDBDMAChannel PrivDBDMAChannelArea;
enum {
    stateReadInProgress = 0x00000001,
    stateWriteInProgress = 0x00000002,
    stateSetForRead = 0x00000004,
    stateSetForWrite = 0x00000008
};

#if (!MACH_DEBUG)
#define printf donone
#endif

extern void FlushProcessorCache(uint_t, LogicalAddress, uint_t);
extern uint_t
 CurrentAddressSpaceID(void);
extern ByteCount
 GetLogicalPageSize(void);
/*******************************************************************************
**
**	Function Name:	StartDBDMA
**
**	Purpose:		Start a DMA operation
**
**	Inputs:			-> channel to start
**
**	Outputs:		none
**
**	Notes:			Reset anything wrong with the channel status and start it running.
**
*/
void StartDBDMA(DBDMAChannelConnectionPtr channelConnection)
{
    PrivDBDMAChannelPtr privChannel = (PrivDBDMAChannelPtr) channelConnection;
    DBDMAChannelRegisters *DBDMAPtr = privChannel->DBDMAPtr;


    privChannel->state |= ((privChannel->state & stateSetForRead) != 0) ?
	stateReadInProgress : stateWriteInProgress;
    SetChannelControl(DBDMAPtr, kdbdmaSetRun);
}

/*******************************************************************************
**
**	Function Name:	StopDBDMA
**
**	Purpose:		Stop an DB DMA read or write operation.	
**
**	Inputs:			-> channel to stop
**
**	Outputs:		none
**
**	Notes:			Stop any running channel commands.
**
*/
void StopDBDMA(DBDMAChannelConnectionPtr channelConnection)
{
    PrivDBDMAChannelPtr privChannel = (PrivDBDMAChannelPtr) channelConnection;
    DBDMAChannelRegisters *DBDMAPtr = privChannel->DBDMAPtr;


    privChannel->state &= ((privChannel->state & stateSetForRead) != 0) ?
	~stateReadInProgress : ~stateWriteInProgress;
    SetChannelControl(DBDMAPtr, (kdbdmaClrRun | kdbdmaClrPause | kdbdmaClrDead));

}

/*******************************************************************************
**
**	Function Name:	PrepDBDMA
**
**	Purpose:		Makes this channel ready to start by pointing the dbdma
**					hardware at this channel's CCL.
**
**	Inputs:			-> channel to prepare
**
**	Outputs:		none
**
**	Notes:			none.
**
*/
void PrepDBDMA(DBDMAChannelConnectionPtr channelConnection)
{
    PrivDBDMAChannelPtr privChannel = (PrivDBDMAChannelPtr) channelConnection;
    DBDMAChannelRegisters *DBDMAPtr = privChannel->DBDMAPtr;


    // Point the DMA hardware at the new CCL.
    switch(powermac_info.class) {
	case POWERMAC_CLASS_PCI:
	    SetCommandPtr(DBDMAPtr, (UInt32) (privChannel->cclPhysicalAddr));
	    break;
	default:
	    printf("PrepDBDMA called for unsupported machine Class!\n");
	    // SetPDMCommandPtr((unsigned long *)(DBDMAPtr+2), (unsigned short)((unsigned long)(privChannel->cclPhysicalAddr) & 0xFFFF));
	    break;
    }

    printf("chnl lo=0x%04x ", DBDMAPtr->commandPtrLo);
}

/*******************************************************************************
**
**	Function Name:	ResetDBDMA
**
**	Purpose:		Reset the DB DMA engine.
**
**	Inputs:			-> channel to reset
**
**	Outputs:		none
**
**	Notes:			Stop any running channel commands.
**
*/
void ResetDBDMA(DBDMAChannelConnectionPtr channelConnection)
{
    PrivDBDMAChannelPtr privChannel = (PrivDBDMAChannelPtr) channelConnection;
    DBDMAChannelRegisters *DBDMAPtr = privChannel->DBDMAPtr;


    SetChannelControl(DBDMAPtr, (kdbdmaClrRun | kdbdmaClrPause | kdbdmaClrDead));

    privChannel->state = 0;

    switch(powermac_info.class) {
	case POWERMAC_CLASS_PCI:
	    SetCommandPtr(DBDMAPtr, 0);
	    break;
	default:
	    printf("ResetDBDMA called for unsupported machine Class!\n");
	    // SetPDMCommandPtr((unsigned long *)(DBDMAPtr+2), (unsigned short)((unsigned long)(privChannel->cclPhysicalAddr) & 0xFFFF));
	    break;
    }
}

/*******************************************************************************
**
**	Function Name:	SetDBDMAPhysicalAddress
**
**	Purpose:		Get ready to start a DMA transfer.
**
**	Inputs:			-> channel to set, buffer and len, read/write indicator
**
**	Outputs:		none
**
**	Notes:			builds a CCL
**
*/
void SetDBDMAPhysicalAddress(DBDMAChannelConnectionPtr channelConnection,
			     Boolean isReadTransfer,
			     PhysicalAddress addressPtr,
			     ByteCount transferCount)
{
    UInt32 op;
    UInt32 thisTime;
    UInt32 byteCount = transferCount;
    UInt8 *bufferAddr = (UInt8 *) addressPtr;
    PrivDBDMAChannelPtr privChannel = (PrivDBDMAChannelPtr) channelConnection;
    DBDMAChannelRegisters *DBDMAPtr = privChannel->DBDMAPtr;
    DBDMADescriptor *DescPtr = (DBDMADescriptor *) privChannel->cclStaticLogicalAddr;

    // Stop any running channel commands.
    SetChannelControl(DBDMAPtr, kdbdmaClrRun |
		      kdbdmaClrPause |
		      kdbdmaClrDead);

    op = isReadTransfer ? INPUT_MORE : OUTPUT_MORE;
    privChannel->state = isReadTransfer ? stateSetForRead : stateSetForWrite;

    // build the CCL
    while (byteCount) {
	thisTime = (byteCount < 0xFFF0) ? byteCount : 0xFFF0;
	byteCount -= thisTime;

	// Is this the last ccl command?
	// If so, have it generate an interrupt.
	if (byteCount == 0)
	    op |= kIntAlways;

	MakeCCDescriptor(DescPtr, op | thisTime, (long) bufferAddr);
	bufferAddr += thisTime;
	printf("DescPtr=0x%x,decriptors=0x%x,0x%x,0x%x,0x%x ", (long) DescPtr, ((long *) DescPtr)[0], ((long *) DescPtr)[1], ((long *) DescPtr)[2], ((long *) DescPtr)[3]);
	DescPtr += 1;
    }

    MakeCCDescriptor(DescPtr, STOP_CMD, 0);
    DescPtr += 1;

    FlushProcessorCache(CurrentAddressSpaceID(),
			privChannel->cclStaticLogicalAddr,
	((UInt32) DescPtr - (UInt32) privChannel->cclStaticLogicalAddr));
}


/*******************************************************************************
**
**	Function Name:	OpenDBDMAChannel
**
**	Purpose:		Allocate runtime resource to manage a channel in the DBDMA chip.
**
**	Inputs:			ID of kernel process to own area created for channel commands
**					Ptr to DBDMAChannelRegisters (the Channel to INIT) 
**					Addresses of output vars
**
**	Outputs:		ptr to channel command bufer  (caller's address space)
**
**	Notes:			All plug-ins using dbdma can make use of this code.
**					We use create area to make a single page-aligned global area
**					to build channel commands.
*/
OSStatus
OpenDBDMAChannel(DBDMAChannelRegisters * DBDMAPtr,
		 DBDMAChannelConnectionPtr * channelConnection,
		 UInt32 cclNum,
		 LogicalAddress * logicalAddr,
		 PhysicalAddress * physicalAddr)
{
    PrivDBDMAChannelPtr privChannel;
    DBDMADescriptor *DescPtr;
    LogicalAddress logicalMap[1];
    PhysicalAddress physicalMap[1];
    OSStatus status = noErr;
    int i;
    ByteCount areaLength, pageSize = GetLogicalPageSize();
    //KernelProcessID                       owningProcessID = CurrentKernelProcessID();

    // Allocate memory for a channel connection control block.
    //      status = MemDbgNewFixedClear(   "IOSS",
    //                                                                      "DBDM",
    //                                                                      xResidentSysAllocator,
    //                                                                      sizeof(PrivDBDMAChannel),
    //                                                                      (void **)channelConnection      );
    *channelConnection = &PrivDBDMAChannelArea;
    privChannel = (PrivDBDMAChannelPtr) * channelConnection;

    // Determine the amount of space for the CCL's
    if (cclNum <= 0)
	return paramErr;
    areaLength = ((cclNum * sizeof(DBDMADescriptor)) + (pageSize - 1)) & -pageSize;

    // Create a global area attached to supplied kernel process ID.
    //      status = CreateArea (   owningProcessID,
    //                                                      kNoBackingObjectID,
    //                                                      NULL, 
    //                                                      areaLength,
    //                                                      kMemoryExcluded,
    //                                                      kMemoryReadWrite,
    //                                                      1, 
    //                                                      kGlobalArea | kPhysicallyContiguousArea | kResidentArea,
    //                                                      &privChannel->cclPtr,
    //                                                      &privChannel->cclAreaID);

    switch (powermac_info.class) {
        case POWERMAC_CLASS_PCI:

	    privChannel->cclStaticLogicalAddr = (LogicalAddress) io_map(0, areaLength);
	    if (privChannel->cclStaticLogicalAddr == NULL) {
	        printf("dbdmasupport.c:Unable to create DBDMA CCLs memory\n");
	        status = FLOP_MEM_ERROR;
	    }
	    privChannel->cclPhysicalAddr = (PhysicalAddress) kvtophys((vm_offset_t) privChannel->cclStaticLogicalAddr);
	    break;
        case POWERMAC_CLASS_PDM:
        case POWERMAC_CLASS_POWERBOOK:
        case POWERMAC_CLASS_PERFORMA:
        default:
	    printf("ResetDBDMA called for unsupported machine Class!\n");
	    // privChannel->cclStaticLogicalAddr = (LogicalAddress) (powermac_info.dma_buffer_virt + PDM_DMA_BUFFER_FLOPPY_CCL_OFFSET);
	    // privChannel->cclPhysicalAddr = (PhysicalAddress) kvtophys((vm_offset_t) privChannel->cclStaticLogicalAddr);
        }
    printf("cclphys=0x%04x,ccllogical=0x%04x ", privChannel->cclPhysicalAddr, privChannel->cclStaticLogicalAddr);

    /* fill the buffer with stop commands */
    DescPtr = (DBDMADescriptorPtr) privChannel->cclStaticLogicalAddr;
    for (i = 0; i < (GetLogicalPageSize() / sizeof(struct DBDMADescriptor)); i++) {
	MakeCCDescriptor(DescPtr, STOP_CMD, 0);
	DescPtr += 1;
    }

    if (status != noErr)
	CloseDBDMAChannel(*channelConnection);
    else {
//              privChannel -> owningProcessID  = owningProcessID;
	privChannel->DBDMAPtr = DBDMAPtr;
	ResetDBDMA(*channelConnection);
    }
    printf("dbdmasupport.c:returning from OpenDbdma ");

    return status;
}

/*******************************************************************************
**
**	Function Name:	CloseDBDMAChannel
**
**	Purpose:		DeAllocate runtime resources associated with a DMDMA channel.
**
**	Inputs:			-> channel
**
**	Outputs:		none
**
**	Notes:			Command Area is deleted and connection memory is deallocated.
*/
void CloseDBDMAChannel(DBDMAChannelConnectionPtr channel)
{
    PrivDBDMAChannelPtr DBDMAchannel = (PrivDBDMAChannelPtr) channel;

//      if (DBDMAchannel -> cclAreaID != 0)
    //              DeleteArea      (DBDMAchannel -> cclAreaID);
    //kfree (DBDMAchannel);
}


void PrintDMA()
{
    DBDMAChannelRegisters *rg;
    unsigned long DescPtr = (unsigned long) PrivDBDMAChannelArea.cclStaticLogicalAddr;

    if (powermac_info.class = POWERMAC_CLASS_PCI)
	rg = (DBDMAChannelRegisters *) (PCI_DMA_BASE_PHYS + (DBDMA_FLOPPY << 8));
    else
	rg = (DBDMAChannelRegisters *) (POWERMAC_IO(PDM_FLOPPY_AMIC_BASE_PHYS));

    printf("PtrLo=0x%04x,ccntl=0x%04x,cstat=0x%04x ", rg->commandPtrLo, rg->channelControl, rg->channelStatus);
    printf("decriptors=0x%x,0x%x,0x%x,0x%x ", ((long *) DescPtr)[0], ((long *) DescPtr)[1], ((long *) DescPtr)[2], ((long *) DescPtr)[3]);
}
#endif // OLD_DBDMA
#endif // NFD > 0
