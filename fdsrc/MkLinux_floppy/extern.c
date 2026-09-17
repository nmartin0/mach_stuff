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

#include <sys/types.h>
#include <fd.h>
//#include <sa_mach.h>
//#include <kern/clock.h>
//#include <mach_host.h>
//#include <mach_services/include/mach.h>
// #define PZERO 25		//borrowed from fdreg.h
#include "portdef.h"
#include <ppc/proc_reg.h>	/* For isync */
#include <sys/ioctl.h>
#include <machine/spl.h>
#if 0
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/dev_master.h>
#include <device/ds_routines.h>
#include <device/misc_protos.h>
#endif
#include <kern/lock.h>
#include "floppypriv.h"
#include "floppysal.h"
#include "floppycore.h"
#include "floppyhal.h"
#include "swimiii.h"
#include "swimiiicommonhal.h"
#if 0
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#include <ppc/POWERMAC/dbdma.h>
#endif
#include <sys/systm.h> // for printf
#include "dprintf.h"

#if NFD > 0

#if (!MACH_DEBUG)
// #define printf donone
#else
#if (SERIAL_DEBUG)
#define printf kprintf
#endif
#endif

extern mutex_t *master_floppy_lock;

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

OSStatus
DeleteOSHardwareLockResources(mutex_t *OSLockID)
{
    // mutex_free(OSLockID);

    return 0;
}

// void init_event_table(void);
OSStatus CreateOSHardwareLockResources(mutex_t ** OSLockID)
{
    // *OSLockID = mutex_alloc((etap_event_t)OSLockID);
    *OSLockID = master_floppy_lock;

    // init_event_table();

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
#ifdef MKLINUX_SOURCE
extern char TrackBuffer[NFD][0xb000];
#else
extern char *TrackBuffer[NFD];
#endif
OSStatus FlushDMAedDataFromCPUCache()
{
    return 0;
}

OSStatus PrepareCPUCacheForDMAWrite()
{
    int unit;

#ifdef PDMSUPPORT
    switch(powermac_info.class) {
	case POWERMAC_CLASS_PCI:
#endif
	    for (unit = 0; unit < NFD; unit++) {
		if (!TrackBuffer[unit]) continue;
		flush_dcache((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
	    }
	    return 0;
#ifdef PDMSUPPORT
	case POWERMAC_CLASS_PDM:
	    invalidate_cache_for_io((vm_offset_t) (powermac_info.dma_buffer_virt +
					PDM_DMA_BUFFER_FLOPPY_OFFSET), 0xb000, FALSE);
	    break;
	}
#endif
    return 0;
}

/*
 * I was worried at one point whether the following shoudl be invalidate_cache_for_io,
 * but I figure it works, so who cares?
 */
OSStatus PrepareCPUCacheForDMARead()
{
    int unit;

#ifdef PDMSUPPORT
    switch(powermac_info.class) {
	case POWERMAC_CLASS_PCI:
#endif
	    for (unit = 0; unit < NFD; unit++) {
/*** @@@ DAG VERIFY THIS... MIGHT REALLY NEED THE invalidate_cache_for_io HERE.... @@@ ***/
#ifdef MKLINUX_VERSION
		if (!TrackBuffer[unit]) continue;
		invalidate_cache_for_io((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
		// invalidate_dcache((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
#else
		if (!TrackBuffer[unit]) continue;
		invalidate_cache_for_io((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
		// invalidate_dcache((vm_offset_t) TrackBuffer[unit], 0xb000, FALSE);
#endif
	    }
	    return 0;
#ifdef PDMSUPPORT
	case POWERMAC_CLASS_PDM:
	    invalidate_cache_for_io((vm_offset_t) (powermac_info.dma_buffer_virt +
					     PDM_DMA_BUFFER_FLOPPY_OFFSET),
			      0xb000, FALSE);
	    break;
	}

#endif

// invalidate_cache_v((vm_offset_t)&rxDMACommands[i],
// sizeof(enet_dma_cmd_t));
// or flush_cache_v(addr, size);

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

#if 1
    int MicroSeconds = (int) Milliseconds * 1000;
    //    delay(MicroSeconds);
    if (Milliseconds < 10)
	delay(MicroSeconds);
    else {
#ifdef MKLINUXSOURCE
	timeout((timeout_fcn_t) wakeup, &TimedSleepPtr, (Milliseconds * HZ + 999) / 1000);
	sleep((char *) &TimedSleepPtr, PZERO);
#else
	// IOSleep((char *) &TimedSleepPtr, PZERO);
	/* @@@ BROKEN @@@ */
	// IOSleep(Milliseconds);
	non_bsd_sleep(&TimedSleepPtr, Milliseconds);
#endif
	//printf( " FTS - Wakeup " );
    }

#else
    /* Horrible quick hack */
    int MicroSeconds = (int) Milliseconds * 1000;
    //    delay(MicroSeconds);
    delay(MicroSeconds);
#endif
    return 0;
}

void ExitHardwareLockSection(mutex_t *mutex)
{				/*splx(x) */

    dprintf(DEBUG_VERBOSE, "Releasing lock\n");
    mutex_unlock(mutex);
    dprintf(DEBUG_VERBOSE, "Lock released\n");
    // IOLog("lock released...\n");
    return;
}
void EnterHardwareLockSection(mutex_t *mutex)
{

    dprintf(DEBUG_VERBOSE, "Waiting for lock\n");
    mutex_lock(mutex);
    dprintf(DEBUG_VERBOSE, "Lock taken\n");
    // IOLog("lock taken...\n");
    return /*SPL() */ ;
}
void FlushProcessorCache(uint_t a, LogicalAddress b, uint_t c)
{
// a is address space ID
// b is address
// c is offset???

// not needed, I don't think.

}
uint_t CurrentAddressSpaceID()
{
    return 0;
}
ByteCount GetLogicalPageSize()
{
    // return 4096;
    return PAGE_SIZE;
}
OSStatus
SetOSEvent(UInt32 * OSEventID,
	   uint_t eventMask)
{
    dprintf(DEBUG_VERBOSE, "SET 0x%x ", eventMask);


    (*OSEventID) |= eventMask;

    // This hack didn't work....
    // (*OSEventID) |= (eventMask | 0x8);

// #ifdef NOTYETDAG
#ifdef MKLINUX_CODE
#ifdef SANETIMEOUTROUTINES
    if (untimeout((timeout_fcn_t) wakeup, (void *) OSEventID)) {
	//(*OSEventID) |= eventMask;
	//    dprintf(DEBUG_VERBOSE, "waking up 0x%x event ",eventMask);
	wakeup((char *) OSEventID);
    } else
	dprintf(DEBUG_VERBOSE, "TIMEOUT 0x%x ", eventMask);
#else
    untimeout((timeout_fcn_t) wakeup, (void *) OSEventID);
    wakeup((char *) OSEventID);
#endif
#else
    non_bsd_wakeup((char *) OSEventID);
#endif

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
    // int count = 0;
    boolean_t eventDidNotTimeOut = 0;
    dprintf(DEBUG_VERBOSE, " WAITING 0x%x ",eventMask);
    dprintf(DEBUG_VERBOSE, "In WaitForOSEvent\n");
    if ((eventMask & (*OSEventID)) != 0) {	/* Event already took place */
	             dprintf(DEBUG_VERBOSE, "RACE!event 0x%x already occured ",eventMask);
	*eventMaskResult = *OSEventID;
	return 1;
    }
    /* This is a bad idea to do this do loop. */
     // do
     // {
#ifdef MKLINUX_SOURCE
    timeout((timeout_fcn_t) wakeup, OSEventID, (2 * timeOut * HZ + 999) / 1000);	//naga double 

    sleep((char *) OSEventID, PZERO);
#else
    /* @@@ BROKEN @@@ */
    dprintf(DEBUG_VERBOSE, "Calling non_bsd_sleep\n");
    non_bsd_sleep(OSEventID, timeOut);
    // IOSleep((timeOut * HZ + 999) / 1000000);
#endif
// } while  ((((*OSEventID) & eventMask ) == 0 ) && count++ < 6);
    *eventMaskResult = *OSEventID;
    if (((*OSEventID) & eventMask) != 0)
	eventDidNotTimeOut = 1;
    // if (!(*OSEventID)) eventDidNotTimeOut = 1;
#if 1
   if(eventDidNotTimeOut == 0)  
   {
   // unsigned long xx;
   //          GRCSwimIIIRegs *gsPtr = (GRCSwimIIIRegs *)POWERMAC_IO(PCI_FLOPPY_BASE_PHYS);
   //        xx = gsPtr->rInterrupt;
   //        SynchronizeIO();
   //          dprintf(DEBUG_VERBOSE, "WAITTIMEOUT ireg=0x%x mask=0x%x",xx,eventMask);
	dprintf(DEBUG_VERBOSE, "WAITTIMEOUT\n");
   }
   else dprintf(DEBUG_VERBOSE, "OVER 0x%x ",eventMask);
#endif
    dprintf(DEBUG_VERBOSE, "Left WaitForOSEvent\n");
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
