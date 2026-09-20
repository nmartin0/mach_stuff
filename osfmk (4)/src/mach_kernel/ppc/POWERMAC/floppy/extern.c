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

#include <types.h>
//#include <sa_mach.h>
//#include <kern/clock.h>
//#include <mach_host.h>
//#include <mach_services/include/mach.h>
#define PZERO 25		//borrowed from fdreg.h
#include "portdef.h"
#include <ppc/proc_reg.h>	/* For isync */
#include <sys/ioctl.h>
#include <kern/spl.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#include "floppypriv.h"
#include "floppysal.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "swimiii.h"
#include "swimiiicommonhal.h"
#include "fdreg.h"
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#include <ppc/POWERMAC/dbdma.h>
#include <fd.h>

#if NFD > 0

#if (!MACH_DEBUG)
#define printf donone
#endif

BSIOStatus BSBlockListDescriptorGetExtent(BSBlockListDescriptorRef blocks,
					  int ignore,
					  BSByteCount * startingByte,
					  ByteCount * blocksExtentLen);

void FlushProcessorCache(uint_t, LogicalAddress, uint_t);
uint_t
CurrentAddressSpaceID(void);
ByteCount
GetLogicalPageSize(void);
OSStatus MapCacheForDMAIO(LogicalAddress logicalBaseAddr,
			  uint_t length,
			  LogicalAddress * logBase,
			  PhysicalAddress * phyBase)
{
    return 0;
}

OSStatus CreateOSHardwareLockResources(spl_t * OSHardwareLockID)
{
    return 0;
}
OSStatus CreateOSEventResources(UInt32 * x)
{
    return 0;
}
OSStatus BSMPINotifyFamilyStoreChangedState(void *OSDiskID, int event)
{
    return 0;
}
/*
   OSStatus
   PostDisketteEvent( int unit, unsigned char   disketteEvent,
   short                        driveIndex )
   {
   return 0;
   }
 */
extern char TrackBuffer[NFD][0xb000];
OSStatus FlushDMAedDataFromCPUCache()
{
    return 0;
}

OSStatus PrepareCPUCacheForDMAWrite()
{
    int unit;

    switch(powermac_info.class) {
	case POWERMAC_CLASS_PCI:
	    for (unit = 0; unit < NFD; unit++)
		flush_dcache((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
	    return 0;
	case POWERMAC_CLASS_PDM:
	    invalidate_cache_for_io((vm_offset_t) (powermac_info.dma_buffer_virt +
					PDM_DMA_BUFFER_FLOPPY_OFFSET), 0xb000, FALSE);
	    break;
	}
    return 0;
}

/*
 * I was worried at one point whether the following shoudl be invalidate_cache_for_io,
 * but I figure it works, so who cares?
 */
OSStatus PrepareCPUCacheForDMARead()
{
    int unit;

    switch(powermac_info.class) {
	case POWERMAC_CLASS_PCI:
	    for (unit = 0; unit < NFD; unit++)
		invalidate_cache_for_io((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
		// invalidate_dcache((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
	    return 0;
	case POWERMAC_CLASS_PDM:
	    invalidate_cache_for_io((vm_offset_t) (powermac_info.dma_buffer_virt +
					     PDM_DMA_BUFFER_FLOPPY_OFFSET),
			      0xb000, FALSE);
	    break;
	}
    return 0;
}

#if 0

OSStatus PrepareCPUCacheForDMAWrite()
{
    flush_dcache((vm_offset_t) TrackBuffer, 0xb000, FALSE);
    return 0;
}
OSStatus PrepareCPUCacheForDMARead()
{
    invalidate_dcache((vm_offset_t) TrackBuffer, 0xb000, FALSE);
    return 0;
}				// should this be invalidate_cache_for_io???

#endif

OSStatus MemListDescriptorDataCopy(MemListDescriptorRef srcDescriptor,
	      MemListDescriptorRef destDescriptor, ByteCount bytesToCopy)
{
    unsigned char *t, *s;
    t = (unsigned char *) destDescriptor;
    s = (unsigned char *) srcDescriptor;
    while (bytesToCopy--)
	*t++ = *s++;
    return 0;
}
OSStatus MemListDescriptorDataCopyToMemory(MemListDescriptorRef srcDescriptor,
			LogicalAddress destBuffer, ByteCount bytesToCopy)
{
    return MemListDescriptorDataCopy(srcDescriptor, (MemListDescriptorRef) destBuffer, bytesToCopy);
}
OSStatus MemListDescriptorDataCopyFromMemory(LogicalAddress srcBuffer,
	      MemListDescriptorRef destDescriptor, ByteCount bytesToCopy)
{
    return MemListDescriptorDataCopy((MemListDescriptorRef) srcBuffer, destDescriptor, bytesToCopy);
}

OSStatus MemListDescriptorDataCompare(MemListDescriptorRef descriptor1,
	      MemListDescriptorRef descriptor2, ByteCount bytesToCompare,
				      Boolean * different,
				      ByteCount * firstDifferentByte)
{
    return 0;
}
OSStatus MemListDescriptorDataCompareWithMemory(
		  MemListDescriptorRef descriptor, LogicalAddress buffer,
			   ByteCount bytesToCompare, Boolean * different,
					  ByteCount * firstDifferentByte)
{
    return 0;
}
void SynchronizeIO()
{
    eieio();
}

OSStatus FloppyTimedSleep(long Milliseconds)
{
    char TimedSleepPtr;

    int MicroSeconds = (int) Milliseconds * 1000;
    //    delay(MicroSeconds);
    if (Milliseconds < 10)
	delay(MicroSeconds);
    else {
	timeout((timeout_fcn_t) wakeup, &TimedSleepPtr, (Milliseconds * HZ + 999) / 1000);
	sleep((char *) &TimedSleepPtr, PZERO);
	//printf( " FTS - Wakeup " );
    }
    return 0;
}

void ExitHardwareLockSection(spl_t x)
{				/*splx(x) */
    return;
}
spl_t EnterHardwareLockSection()
{
    return 0 /*SPL() */ ;
}
void FlushProcessorCache(uint_t a, LogicalAddress b, uint_t c)
{
}
uint_t CurrentAddressSpaceID()
{
    return 0;
}
ByteCount GetLogicalPageSize()
{
    return 4096;
}
OSStatus
SetOSEvent(UInt32 * OSEventID,
	   uint_t eventMask)
{
    printf("SET 0x%x ", eventMask);


    (*OSEventID) |= eventMask;

    // This hack didn't work....
    // (*OSEventID) |= (eventMask | 0x8);

    if (untimeout((timeout_fcn_t) wakeup, (void *) OSEventID)) {
	//(*OSEventID) |= eventMask;
	//    printf("waking up 0x%x event ",eventMask);
	wakeup((char *) OSEventID);
    } else
	printf("TIMEOUT 0x%x ", eventMask);
    return 0;
}

OSStatus
CancelOSEvent(UInt32 * OSEventID,
	      uint_t eventMask)
{
    (*OSEventID) &= (~eventMask);
    return 0;
}

boolean_t
WaitForOSEvent(UInt32 * OSEventID,
	       uint_t eventMask,
	       uint_t timeOut,
	       uint_t * eventMaskResult)
{
    boolean_t eventDidNotTimeOut = 0;
//        printf(" WAITING 0x%x ",eventMask);
    if ((eventMask & (*OSEventID)) != 0) {	/* Event already took place */
	//             printf("RACE!event 0x%x already occured ",eventMask);
	*eventMaskResult = *OSEventID;
	return 1;
    }
//  do
    //{
    timeout((timeout_fcn_t) wakeup, OSEventID, (2 * timeOut * HZ + 999) / 1000);	//naga double 

    sleep((char *) OSEventID, PZERO);
//} while  (((*OSEventID) & eventMask ) == 0 );
    *eventMaskResult = *OSEventID;
    if (((*OSEventID) & eventMask) != 0)
	eventDidNotTimeOut = 1;
/*
   if(eventDidNotTimeOut == 0)  
   {
   unsigned long xx;
   //          GRCSwimIIIRegs *gsPtr = (GRCSwimIIIRegs *)POWERMAC_IO(PCI_FLOPPY_BASE_PHYS);
   //        xx = gsPtr->rInterrupt;
   //        SynchronizeIO();
   //          printf("WAITTIMEOUT ireg=0x%x mask=0x%x",xx,eventMask);
   }
   else printf("OVER 0x%x ",eventMask);
 */
    return eventDidNotTimeOut;
}

BSIOStatus BSBlockListDescriptorGetExtent(BSBlockListDescriptorRef blocks,
					  int ignore,
					  BSByteCount * startingByte,
					  ByteCount * blocksExtentLen)
{
    *startingByte = 0;
    *blocksExtentLen = 40 * 512;
    return E_BSSuccess;
}
void donone(char *a,...)
{
}

#endif // NFD > 0
