/*
 * Copyright (c) 1998-2001 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <IOKit/IOLib.h>
#include <IOKit/IOMemoryCursor.h>

#define CPLUSPLUS
typedef long unsigned int uint_t;
#include "floppy.h"
#include "portdef.h"

// kIOMessageMediaStateHasChanged
// call message(kIOMessageMediaStateHasChanged); when state has changed.

extern "C" {
#include <pexpert/pexpert.h>	//This is for debugging purposes ONLY

io_return_t fdsetstat(dev_t dev,
	     dev_flavor_t flavor,
	     dev_status_t data,
	     natural_t count);
             
void ScanForDisketteChange(void);

BSIOStatus
FloppyPluginIO(int unit, BSStorePtr ioStore,
               BSBlockListDescriptorRef blocks,
               MemListDescriptorRef memory,
               BSIORequestBlockPtr parentRequest,
               OptionBits options,
               struct BSErrorList **errors);
OSStatus FloppyPluginGetInfo(int unit,
                     struct BSStoreMPIInfo *info);
BSIOStatus FloppyPluginFormatMedia(int unit, BSStorePtr formatStore,
                         BSFormatIndex formatType);
OSStatus FloppyPluginReinit(int unit, BSStorePtr initStore);
OSStatus FloppyPluginFlush(int unit, BSIORequestBlockPtr parentRequest,
                  struct BSErrorList **errors);
OSStatus FloppyPluginEject(int unit, BSAccessibilityState gotoState);

int non_bsd_sleep(void *event, unsigned long usec);

void donone(char *,...);
char *onepage = NULL;
int org_mklinux_iokit_swim3_busyflag = 0;
extern int printf(char *a, ...);
#include "MkLinux_floppy/dprintf.h"
}
extern int org_mklinux_iokit_swim3_objcount;

// #undef MACH_DEBUG
// #define MACH_DEBUG 0
// #if (SERIAL_DEBUG)
// #define IOLog kprintf
// #endif
// #endif

// in floppy.cpp
// virtual IOReturn doAsyncReadWrite(IOMemoryDescriptor *buffer,
//                         UInt32 block,UInt32 nblks,
//                         IOStorageCompletion completion) = 0;

IOReturn org_mklinux_iokit_swim3_driver::doEjectMedia(void)
{
    if(locked)
        return kIOReturnBusy;

    if (FloppyPluginFlush(this->busdev->unit, 0, 0) != kBSIOCompleted)
	return kIOReturnError;

    if (FloppyPluginEject(this->busdev->unit, kBSOffline) != noErr)
	return kIOReturnError;

    return kIOReturnSuccess;
}

IOReturn org_mklinux_iokit_swim3_driver::doFormatMedia(UInt64 byteCapacity)
{
    int index = 0;
    struct BSStoreMPIInfo info;
    int nblks;

    // IOLog("doFormatMedia point 1\n");
    if (FloppyPluginGetInfo(this->busdev->unit, &info) != E_BSSuccess) {
	return kIOReturnError;
    }

    // IOLog("doFormatMedia point 2\n");
    nblks = info.storeSize / 512; // get number of blocks in media
    switch (nblks) {
        case    0:                      // no disk inserted
	    dIOLog(DEBUG_GENERAL, "Invalid format.\n");
            return kIOReturnError;
        case  800:                      // 400k GCR
	    /* Allow 400k to fall down into 720k/800k to deal with
	       badly formatted disks. */
	    if (byteCapacity == (800 * 512)) {
		index = 1;
		break;
	    }
        case 1600:                      // 800k GCR
        case 1440:                      // 720k MFM
	    if (byteCapacity == (1600 * 512)) {
		index = 2;
		break;
	    } else if (byteCapacity == (1440 * 512)) {
		index = 3;
		break;
	    }
	    dIOLog(DEBUG_GENERAL, "Invalid format.\n");
	    return kIOReturnError;
        case 2880:
	    if (byteCapacity == (2880 * 512)) {
		index = 1;
		break;
	    }
	    dIOLog(DEBUG_GENERAL, "Invalid format.\n");
	    return kIOReturnError;
        case 3360:
        case 5760:
        default:
		/* These wacko cases would need a lot of help.... */
		dIOLog(DEBUG_GENERAL, "Invalid format.\n");
		return kIOReturnError;
    }

    // IOLog("doFormatMedia point 3\n");
    IOLockLock(this->fdFormatLock);
    if (FloppyPluginFormatMedia(this->busdev->unit, NULL, index) != kBSIOCompleted) {
        dIOLog(DEBUG_GENERAL, "doFormatMedia error\n");
	IOLockUnlock(this->fdFormatLock);
	return kIOReturnError;
    }
    // IOLog("doFormatMedia success\n");

    /* Need to notify block size changed here. */
    IOLockUnlock(this->fdFormatLock);
    return kIOReturnSuccess;
}

UInt32 org_mklinux_iokit_swim3_driver::doGetFormatCapacities(UInt64 * capacities,
                        UInt32 capacitiesMaxCount)
{
    struct BSStoreMPIInfo info;
#if 0
    BSStorePtr infoStore;
#endif
    UInt64 *ptr = capacities;
    int rem = capacitiesMaxCount;
    int count = 0;
    int nblks;

    /* If they don't give us any space, we're hosed, so bail. */
    if (!ptr) return 0;
    if (!rem) return 0;

    if (FloppyPluginGetInfo(this->busdev->unit, &info) != E_BSSuccess)
	return 0;

    nblks = info.storeSize / 512; // get number of blocks in media
    switch (nblks) {
	case    0:			// no disk inserted
	    return 0;
	case  800:			// 400k GCR
	    dIOLog(DEBUG_GENERAL, "swim3: capacity 400k (poss. 720/800k)\n");
	    *capacities = 800;
	    capacities++; count++;
	    if (!(--rem)) return count;
	case 1600:			// 800k GCR
	    dIOLog(DEBUG_GENERAL, "swim3: capacity 800/720k\n");
	    *capacities = 1600;
	    capacities++; count++;
	    if (!(--rem)) return count;
	    *capacities = 1440;
	    capacities++; count++;
	    return count;
	case 1440:			// 720k MFM
	    dIOLog(DEBUG_GENERAL, "swim3: capacity 720/800k\n");
	    *capacities = 1440;
	    capacities++;
	    if (!(--rem)) return 1;
	    *capacities = 1600;
	    capacities++;
	    if (!(--rem)) return 2;
	    *capacities = 800;
	    return 3;
	case 2880:
	    dIOLog(DEBUG_GENERAL, "swim3: capacity 1440k\n");
	case 3360:
	case 5760:
	    dIOLog(DEBUG_GENERAL, "fd: Oversized capacity may not work.\n");
	    *capacities = info.storeSize;
	    return 1;
	default:
	    dIOLog(DEBUG_GENERAL, "fd: Wacko capacity %d blocks\n", nblks);
	    *capacities = info.storeSize;
	    return 1;
    }
    /* We should never get here. */
    return 0;
}

IOReturn org_mklinux_iokit_swim3_driver::doLockUnlockMedia(bool doLock)
{
    locked = doLock;
    return kIOReturnSuccess;
}

/* We're running into a panic when IOMalloc/IOFree are used repeatedly
   a few hundred (thousand) times.  Thus far, the upper layers of the
   OS have yet to request any data more than a single block in length,
   despite the fact that we say that we can read the entire disk in
   a single transaction.  (This is, in fact, a lie.  We can read an
   entire track in a single transaction, though, and can do a whole
   disk read a heck of a lot faster inside the driver than the I/O
   Kit can do it by calling in once per track....)

   The IOMalloc and IOFree functions are basically thin shims to
   kalloc and kfree for requests under a page.  After that, they
   use kmem_alloc and kmem_free.  So the new version of the code
   uses kmem_free for requests above a page, and uses a static
   buffer for requests up to a page in length.
 */

IOReturn org_mklinux_iokit_swim3_driver::doSyncReadWrite(IOMemoryDescriptor *buffer,
                        UInt32 block,UInt32 nblks)
{
return doReadWrite(buffer, block, nblks, 0);
}

IOReturn org_mklinux_iokit_swim3_driver::doReadWrite(IOMemoryDescriptor *buffer, UInt32 block, UInt32 nblks, int is_async)
{
    uint_t Result;
    BSIOStatus retval = kBSIOCompleted;
    OptionBits direction;
    char *localbuf;
    IOReturn myIOReturn;
    int retrycount;
    unsigned int blocks_per_page = PAGE_SIZE / 512;

#ifdef IOKITTEST
    dIOLog(DEBUG_GENERAL, "Returning I/O Error immediately.\n");
    return kIOReturnIOError;
#endif

/* Use this option to cause any disk inserted to immediately ask you
   if you want to format it.  This is handy if you have a disk that
   is really, really hosed, like some of the coasters I made while
   testing format support.  -DAG- :-) */

// #define FORCEFORMAT

#ifdef FORCEFORMAT
    return kIOReturnIOError;
#endif
    /* I assume the higher levels of the I/O Kit handle checking write protection */

    dIOLog(DEBUG_GENERAL, "swim3: Entered doReadWrite.\n");
    if (!buffer) return kIOReturnNoMemory;
    direction = (buffer->getDirection() == kIODirectionIn ?
        kBSRead : kBSWrite);

    dIOLog(DEBUG_GENERAL, "swim3: direction is %s\n", ((direction == kBSRead) ? "kBSRead" :
	"kBSWrite"));
    dIOLog(DEBUG_RW, "swim3: %s of block%s %ld", ((direction==kBSRead) ? "read" : "write"), (nblks == 1 ? "" : "s"), block);
    if (nblks > 1) {
	dIOLog(DEBUG_RW, "-%ld", block + nblks - 1);
    }
    dIOLog(DEBUG_RW, "\n");
// #define LOG_EACH_ACTION
#ifdef SLOW_LOG_EACH_ACTION
    kprintf("swim3: %s of block%s %d", ((direction==kBSRead) ? "read" : "write"), (nblks == 1 ? "" : "s"), block);
    if (nblks > 1) {
	kprintf("-%d", block + nblks - 1);
    }
    kprintf("\n");
#endif // SLOW_LOG_EACH_ACTION
    
    dIOLog(DEBUG_GENERAL, "swim3: allocating buffer.\n");
    if (nblks <= blocks_per_page) {
	localbuf = onepage;
    } else {
	localbuf = (char *)IOMalloc((nblks * 512) + PAGE_SIZE);
	org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "IOMalloc\n");
	if (!localbuf)
	{
	    IOLog("swim3: Out of MEMORY!\n");
	    return kIOReturnNoMemory;
	}
    }

    dIOLog(DEBUG_GENERAL, "swim3: possibly waiting for busy flag....\n");
    while (org_mklinux_iokit_swim3_busyflag == 1) {
	dIOLog(DEBUG_GENERAL, "swim3: yup.  We're waiting.\n");
        non_bsd_sleep((char *) &org_mklinux_iokit_swim3_busyflag, 10);
    }
    dIOLog(DEBUG_GENERAL, "swim3: done waiting.\n");
    org_mklinux_iokit_swim3_busyflag = 1;

    if (direction == kBSWrite) {
	/* Copy the data to kernel memory for writing */
	if (buffer->readBytes(0, (void *)localbuf, nblks * 512) !=
            (nblks * 512)) {
		IOLog("Write copy failed.  Returning kIOReturnVMError\n");
		if (nblks > blocks_per_page) {
		    IOFree(localbuf, (nblks * 512) + PAGE_SIZE);
		    org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "IOFree\n");
		}
		org_mklinux_iokit_swim3_busyflag = 0;
		return kIOReturnVMError;
	}
	dIOLog(DEBUG_GENERAL, "Doing write.\n");
    }
    
    dIOLog(DEBUG_GENERAL, "swim3: calling FloppyPluginIO.\n");

    for (retrycount = 0; retrycount < 5; retrycount++) {
      retval = FloppyPluginIO(this->busdev->unit, &Result, (nblks * 512),
        (MemListDescriptorRef)localbuf, block * 512,
        direction, 0);
      if (retval == kBSIOCompleted) break;
      dIOLog(DEBUG_GENERAL, "Retrying request.\n");
    }

    dIOLog(DEBUG_GENERAL, "swim3: checking return values and freeing.\n");

    if (retval == kBSIOCompleted) {
      if (Result == (nblks * 512)) {
	if (direction == kBSWrite) {
                myIOReturn=kIOReturnSuccess;
        } else if (buffer->writeBytes(0, (const void *)localbuf, Result) ==
            (Result)) {
                myIOReturn=kIOReturnSuccess;
        } else {
		/* copyout to user buffer failed */
		myIOReturn=kIOReturnVMError;
	}
      } else {
	/* Partial read or write */
	myIOReturn=kIOReturnUnderrun;
      }
    } else if ((OSStatus)retval == (OSStatus)(BSIOStatus)tk0BadErr ||
	       (block == 0)) {
	/* Unformatted disk */
	myIOReturn=kIOReturnUnformattedMedia;
    } else {
	/* Some error in the actual read/write operation */
	myIOReturn=kIOReturnIOError;
    }
    if (nblks > blocks_per_page) {
	IOFree(localbuf, (nblks * 512) + PAGE_SIZE);
	org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "IOFree\n");
    }
    org_mklinux_iokit_swim3_busyflag = 0;
    dIOLog(DEBUG_GENERAL, "swim3: for block %ld-%ld, returning %s (FloppyPluginIO returned %ld/%lx).\n",
	block, block+nblks-1,
	myIOReturn == kIOReturnSuccess ?
	"kIOReturnSuccess" : myIOReturn == kIOReturnVMError? "kIOReturnVMError"
	: myIOReturn == kIOReturnUnderrun ? "kIOReturnUnderrun" :
	myIOReturn == kIOReturnUnformattedMedia ? "kIOReturnUnformattedMedia" :
	"kIOReturnIOError", (long)retval, (unsigned long)retval);
    // printf("swim3: for block %d-%d, returning %s (FloppyPluginIO returned %d/%x).\n",
	// block, block+nblks-1,
	// myIOReturn == kIOReturnSuccess ?
	// "kIOReturnSuccess" : myIOReturn == kIOReturnVMError? "kIOReturnVMError"
	// : myIOReturn == kIOReturnUnderrun ? "kIOReturnUnderrun" :
	// "kIOReturnIOError", (int)retval, (unsigned int)retval);
    return myIOReturn;
}

// virtual IOReturn doSynchronizeCache(void) = 0;
// virtual char * getAdditionalDeviceInfoString(void) = 0;
// virtual char * getProductString(void) = 0;
// virtual char * getRevisionString(void) = 0;
// virtual char * getVendorString(void) = 0;
// virtual bool init(OSDictionary * properties);

IOReturn org_mklinux_iokit_swim3_driver::reportBlockSize(UInt64 *blockSize)
{
    struct BSStoreMPIInfo info;
#if 0
    BSStorePtr infoStore;
#endif
    OSStatus ret;

#ifdef IOKITTEST
    *blockSize = 512;
    dIOLog(DEBUG_GENERAL, "Returning blocksize of 512.\n");
    return kIOReturnSuccess;
#endif

    ret=FloppyPluginGetInfo(this->busdev->unit, &info);

    *blockSize=info.writeBlockSize;

    if (*blockSize != 0) {
#if 0
      /* Try a test read of block 0 to see if disk is formatted. */
      while (org_mklinux_iokit_swim3_busyflag == 1) {
        dIOLog(DEBUG_GENERAL, "swim3: yup.  We're waiting.\n");
        non_bsd_sleep((char *) &org_mklinux_iokit_swim3_busyflag, 10);
      }
      dIOLog(DEBUG_GENERAL, "swim3: done waiting.\n");
      org_mklinux_iokit_swim3_busyflag = 1;

      retval = FloppyPluginIO(this->busdev->unit, &Result, 512,
        (MemListDescriptorRef)formatbuf, 0,
        kBSRead, 0);
      if (retval != kBSIOCompleted) *blockSize = 0;
      org_mklinux_iokit_swim3_busyflag = 0;
#else
    if ((info.curState == kBSOnline) && !info.isFormatted) {
	dIOLog(DEBUG_GENERAL, "Probably unformatted media.\n");
    } else if (info.curState == kBSOnline) {
	// dIOLog(DEBUG_GENERAL, "Setting formatted to 1.\n");
    } else {
	// dIOLog(DEBUG_GENERAL, "Setting formatted to -1.\n");
    }
#endif
    }

    if (ret == E_BSSuccess)
        return kIOReturnSuccess;
    else
        return kIOReturnError;
}

IOReturn org_mklinux_iokit_swim3_driver::reportEjectability(bool *isEjectable)
{
    *isEjectable=1;
    return kIOReturnSuccess;
}

IOReturn org_mklinux_iokit_swim3_driver::reportLockability(bool *isLockable)
{
    *isLockable=1;
    return kIOReturnSuccess;
}

IOReturn org_mklinux_iokit_swim3_driver::reportMaxReadTransfer(UInt64 blockSize,UInt64 *max)
{
    /* It's the same for floppies, so why screw with two functions? */
    return reportMaxWriteTransfer(blockSize, max);
}

IOReturn org_mklinux_iokit_swim3_driver::reportMaxValidBlock(UInt64 *maxBlock)
{
    struct BSStoreMPIInfo info;
#if 0
    BSStorePtr infoStore;
#endif
    OSStatus ret;
#ifdef IOKITTEST
    dIOLog(DEBUG_GENERAL, "Returning maxBlock of 1600\n");
    *maxBlock=1600;
    return kIOReturnSuccess;
#endif

    ret=FloppyPluginGetInfo(this->busdev->unit, &info);

    *maxBlock=info.storeSize / info.writeBlockSize;

    if (ret == E_BSSuccess)
        return kIOReturnSuccess;
    else
        return kIOReturnError;
}


IOReturn org_mklinux_iokit_swim3_driver::reportMaxWriteTransfer(UInt64 blockSize,UInt64 *max)
{

    *max=MAX_PHYS / blockSize;

    return kIOReturnSuccess;
}

IOReturn org_mklinux_iokit_swim3_driver::reportMediaState(bool *mediaPresent,bool *changedState)
{
    struct BSStoreMPIInfo info;
#if 0
    BSStorePtr infoStore;
#endif
    OSStatus ret;
    static int cs = 1;

#ifdef IOKITTEST
    *mediaPresent = true;
    if (cs) {
	*changedState = true;
	cs = 0;
    } else {
	*changedState = false;
    }
    dIOLog(DEBUG_GENERAL, "CS: %s, MP: %s\n", *changedState ? "true" : "false",
	*mediaPresent? "true" : "false");
    return kIOReturnSuccess;
#endif

    IOLockLock(this->fdFormatLock);
    FloppyPluginReinit(this->busdev->unit, (BSStorePtr) NULL);
    
    dIOLog(DEBUG_GENERAL, "Polling swim3 %d\n", this->busdev->unit);
    ScanForDisketteChange();
    ret=FloppyPluginGetInfo(this->busdev->unit, &info);
    
    if (info.curState == kBSOnline)
        *mediaPresent = true;
    else
        *mediaPresent = false;

    if (media_present != *mediaPresent)
        *changedState = true;
    else
        *changedState = false;

    media_present = *mediaPresent;

    dIOLog(DEBUG_GENERAL, "swim3 %d: mediaPresent = %s, changedState = %s, ret = %d\n",
	this->busdev->unit,
        (*mediaPresent == true) ? "true" : "false",
        (*changedState == true) ? "true" : "false", (int)ret);

    if (*changedState) {
	// dIOLog(DEBUG_GENERAL, "Setting formatted to -1.\n");
    }

    IOLockUnlock(this->fdFormatLock);
    if (ret == E_BSSuccess)
        return kIOReturnSuccess;
    else
        return kIOReturnError;
}

IOReturn org_mklinux_iokit_swim3_driver::reportPollRequirements(bool *pollRequired,
                    bool *pollIsExpensive)
{
*pollRequired=true;
*pollIsExpensive=false;
return kIOReturnSuccess;
}

IOReturn org_mklinux_iokit_swim3_driver::reportRemovability(bool *isRemovable)
{
*isRemovable=true;
return kIOReturnSuccess;
}

IOReturn org_mklinux_iokit_swim3_driver::reportWriteProtection(bool *isWriteProtected)
{
    struct BSStoreMPIInfo info;
#if 0
    BSStorePtr infoStore;
#endif
    OSStatus ret;
#ifdef IOKITTEST
    *isWriteProtected = false;
    return kIOReturnSuccess;
#endif

    ret=FloppyPluginGetInfo(this->busdev->unit, &info);

    if (info.curState == kBSOffline) {
	return kIOReturnOffline; // or kIOReturnNotReady
    }

    if (!info.isWriteable)
        *isWriteProtected = true;
    else
        *isWriteProtected = false;

    if (ret == E_BSSuccess) {
	dIOLog(DEBUG_GENERAL, "swim3: reportWriteProtection succeeded with status %d\n",
		*isWriteProtected);
        return kIOReturnSuccess;
    } else {
	dIOLog(DEBUG_GENERAL, "swim3: reportWriteProtection failed with status %d (error %d)\n",
		*isWriteProtected, (int)ret);
        return kIOReturnError;
    }
}
