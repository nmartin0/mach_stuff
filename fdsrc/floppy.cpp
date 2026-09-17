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
#include <IOKit/storage/IOStorage.h>

#define CPLUSPLUS
typedef long unsigned int uint_t;
#include "floppy.h"
#include "portdef.h"

extern "C" {
#include <pexpert/pexpert.h>	//This is for debugging purposes ONLY

OSStatus FloppyPluginInit(int unit, BSStorePtr initStore);
BSIOStatus FloppyPluginFlush(int unit,
                                    BSIORequestBlockPtr parentRequest,
                                    struct BSErrorList **errors);
OSStatus FloppyPluginCleanup(int unit);

int non_bsd_sleep(void *event, unsigned long msec);
void non_bsd_wakeup(void *event);
extern caddr_t TrackBuffer[2];
void donone(char *,...);
#define CURRENT_CODE 1
#if CURRENT_CODE
extern char *onepage;
#endif
void org_mklinux_iokit_swim3_init_event_table(void);
mutex_t *master_floppy_lock = NULL;
mutex_t *org_mklinux_iokit_swim3_startlock = NULL;
extern int printf(char *a, ...);
extern semaphore_t org_mklinux_iokit_swim3_etable_new_sem(event_t event);
extern void org_mklinux_iokit_swim3_etable_free_sem(event_t event);
extern void org_mklinux_iokit_swim3_dispose_event_table(void);
#include "MkLinux_floppy/dprintf.h"
int org_mklinux_iokit_swim3_debug = 0;
int org_mklinux_iokit_swim3_objcount = 0;
}

caddr_t org_mklinux_swim3_vaddr[2];
caddr_t org_mklinux_swim3_dmvaddr[2];
caddr_t org_mklinux_swim3_device[2];
caddr_t org_mklinux_swim3_driver[2];
// caddr_t org_mklinux_iokit_swim3_cmdbase[2];
void (*org_mklinux_iokit_swim3_funcptr)(org_mklinux_iokit_swim3_driver *,
	long unsigned int);

// #undef MACH_DEBUG
// #define MACH_DEBUG 0
// #if (SERIAL_DEBUG)
// #define IOLog kprintf
// #endif
// #endif

// Define my superclass
#define super IOService

// REQUIRED! This macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.  Do NOT use super as the
// second parameter.  You must use the literal name of the superclass.
OSDefineMetaClassAndStructors(org_mklinux_iokit_swim3_driver, IOService)

// static void handleInterrupt(org_mklinux_iokit_swim3_driver *self);
// static void handleDMAInterrupt(org_mklinux_iokit_swim3_driver *self);
// static void org_mklinux_iokit_swim3_driver_NotifyMediaChange(
	// org_mklinux_iokit_swim3_driver *self, BSAccessibilityState status);

// struct bus_device *

/* TODO: Implement the wrappers or equivalents for:

    fdclose()		(do equivalent at driver unload or call as wrapper)
    m765io()		(low-level read/write I/O stuff)
    fdread()		(calls block_io, which calls physio which calls fdstrategy and others)
    fdwrite()		(calls block_io, which calls physio which calls fdstrategy and others)
    fd_format() ??? (does this even work?)
*/

bool org_mklinux_iokit_swim3_driver::init(OSDictionary *dict)
{
	bool res = super::init(dict);
	OSNumber *debugProp = OSDynamicCast(OSNumber,
		dict->getObject("SWIM3 Debug"));

	dIOLog(DEBUG_GENERAL, "SWIM3 Initializing\n");
        media_present=false;
	org_mklinux_iokit_swim3_objcount = 0; dprintf(DEBUG_ALLOC, "objcount reset\n");
	if (!org_mklinux_iokit_swim3_startlock) {
	    org_mklinux_iokit_swim3_startlock = mutex_alloc(0);
	    org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "mutex_alloc\n");
	}

	if (debugProp) {
		org_mklinux_iokit_swim3_debug = debugProp->unsigned32BitValue();
	} else {
		org_mklinux_iokit_swim3_debug = 0;
	}

	return res;
}

void org_mklinux_iokit_swim3_driver::free(void)
{
	dIOLog(DEBUG_GENERAL, "SWIM3 Freeing\n");	
	super::free();
}

IOService *org_mklinux_iokit_swim3_driver::probe(IOService *provider, SInt32
*score)
{
	dIOLog(DEBUG_GENERAL, "SWIM3 Probing\n");

        if (super::probe(provider, score) == NULL)
            return NULL;

        // In all the other cases we handle it:
        return this;
}

bool org_mklinux_iokit_swim3_driver::start(IOService *provider)
{
	bool res = super::start(provider);
        struct bus_device *bd;
        IOMemoryMap * fdMap;
        IOMemoryMap * DMAfdMap;
        io_return_t error;
	UInt32 Channel;

	IOLockLock(org_mklinux_iokit_swim3_startlock);

        swim3Instance = this;

	locked = false;

        IOWorkLoop  *myWorkLoop     = getWorkLoop();

	dIOLog(DEBUG_GENERAL, "Starting\n");
	// printf("This is a printf.\n");
	// kprintf("This is a kprintf.\n");

        if (!res) {
            IOLog("Failed: super::start(provider).\n");
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }

	if (!master_floppy_lock) {
		master_floppy_lock = mutex_alloc(0);
		org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "mutex_alloc\n");
	}
	org_mklinux_iokit_swim3_init_event_table();

	// AsyncLock = IOLockAlloc();
	org_mklinux_iokit_swim3_etable_new_sem(&AsyncLock);
	non_bsd_wakeup((void *)&AsyncLock);

	fdFormatLock = IOLockAlloc();

        DAG_Rules=OSDynamicCast(AppleMacIODevice, provider);
        
        if (!DAG_Rules) {
            IOLog("Failed: OSDynamicCast\n"); 
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }

        if( 0 == (fdMap = provider->mapDeviceMemoryWithIndex( 0 )) ) {
            IOLog("%s: no fd memory\n", getName());
            kprintf("Start is bailing\n");
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }
        this->vaddr = (char *)fdMap->getVirtualAddress();
        this->paddr = (char *)fdMap->getPhysicalAddress();

	kprintf("Floppy device 0x%x (0x%x phys)\n", (unsigned int)this->vaddr,
	    (unsigned int)this->paddr);
	dIOLog(DEBUG_GENERAL, "Floppy device 0x%x (0x%x phys)\n", (unsigned int)this->vaddr,
	    (unsigned int)this->paddr);

	/* We have to allocate the bus_device so we can store the unit
	   information, which we need in order to map the right DMA
	   channel.... */
	bd = (struct bus_device *)IOMalloc(sizeof(*bd));
	org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "IOMalloc\n");
	if (!bd) {
	    kprintf("Out of memory error -- malloc failed.\n");
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
	    return false;
	}
        this->busdev = bd;

#if CURRENT_CODE
	if (!onepage)
	    onepage = (char *)IOMalloc(PAGE_SIZE * 2);
	    org_mklinux_iokit_swim3_objcount++; dprintf(DEBUG_ALLOC, "IOMalloc\n");
	if (!onepage) {
	    kprintf("Out of memory error -- malloc of onepage failed.\n");
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
	    return false;
	}
#endif

        /* Need to have a unique identifier.... */
        bd->unit=(this->paddr == (char *)0xf3015000) ? 0 : 1;

	if (bd->unit) Channel = 0x8000 + 0x01000000;
	else Channel = 0x8000;

	dIOLog(DEBUG_GENERAL, "Unit: %d\n", bd->unit);
	kprintf("Unit: %d\n", bd->unit);

#if 0
	if (bd->unit) {
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
	    return false;
	}
#endif
        
        if( 0 == (DMAfdMap = provider->mapDeviceMemoryWithIndex( 1 )) ) {
            IOLog("%s: no fd memory\n", getName());
            kprintf("Start is bailing\n");
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }
        this->dma_vaddr = (char *)(((UInt32)DMAfdMap->getVirtualAddress()));
	    // + Channel);
        this->dma_paddr = (char *)(((UInt32)DMAfdMap->getPhysicalAddress()));
	    // + Channel);

        org_mklinux_swim3_vaddr[bd->unit] = this->vaddr;
        org_mklinux_swim3_dmvaddr[bd->unit] = this->dma_vaddr;
        org_mklinux_swim3_device[bd->unit] = NULL;
        org_mklinux_swim3_driver[bd->unit] = (caddr_t)this;
	// org_mklinux_iokit_swim3_funcptr =
		 // &org_mklinux_iokit_swim3_driver_NotifyMediaChange;

	// org_mklinux_iokit_swim3_cmdbase[bd->unit] = dbdma_alloc(1);
	// if (!org_mklinux_iokit_swim3_cmdbase[bd->unit]) {
		// kprintf("Bailing: no memory for channel command structure.\n");
		// return false;
	// }

	kprintf("Set org_mklinux_swim3_vaddr[%d] to %d]\n",
		bd->unit, org_mklinux_swim3_vaddr[bd->unit]);

        /* For now, only one device per floppy controller.  We're not dealing with 68k.... */
        bd->ctlr=0;

        /* Replace with virtual address of hardware. Note: original code assumed this was physical address.
           Fake it out with mapping functions that return the original value when asked to map phys->virt. */
        bd->address=this->vaddr;
        
        /* Now do the equivalent of the open call. */
#ifndef IOKITTEST
        error = FloppyPluginInit(bd->unit, (BSStorePtr) NULL);
        if (error != E_BSSuccess) {
	    IOLog("FloppyPluginInit failed!\n");
	    kprintf("FloppyPluginInit failed!\n");
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return NULL;
        }
#endif

        // Now call:
        // void fdattach(bd);
        // as soon as fd.c compiles.

#if 0
        /* Enable interrupts and install our handler */
        provider->registerInterrupt(0, this, (IOInterruptAction) &handleInterrupt, 0);
        provider->enableInterrupt(0);
        provider->registerInterrupt(1, this, (IOInterruptAction) &handleDMAInterrupt, 0);
        provider->enableInterrupt(1);
#endif

        createWorkLoop();
        myWorkLoop = getWorkLoop();
        
        if( myWorkLoop == NULL)
        {
            IOLog( "org_mklinux_iokit_swim3_driver::start: Couldn't allocate workLoop event source\n" );
            
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }
        
        interruptSource = IOInterruptEventSource::interruptEventSource(
                        (OSObject*)this,
                        (IOInterruptEventAction)&org_mklinux_iokit_swim3_driver::handleInterrupt,
                        (IOService*)provider,
                        (int)0 );
                        
        if ( interruptSource == NULL )
        {   IOLog( "org_mklinux_iokit_swim3_driver::start: Couldn't allocate Interrupt event source\n" );

	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }
        
        if ( myWorkLoop->addEventSource( interruptSource ) != kIOReturnSuccess )
        {   IOLog( "org_mklinux_iokit_swim3_driver::start - Couldn't add Interrupt event source\n" );
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        } 
                        
        DMAInterruptSource = IOInterruptEventSource::interruptEventSource(
                        (OSObject*)this,
                        (IOInterruptEventAction)&org_mklinux_iokit_swim3_driver::handleDMAInterrupt,
                        (IOService*)provider,
                        (int)1 );
                        
        if ( DMAInterruptSource == NULL )
        {   
            IOLog( "org_mklinux_iokit_swim3_driver::start: Couldn't allocate Interrupt event source\n" );
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }
        
        if ( myWorkLoop->addEventSource( DMAInterruptSource ) != kIOReturnSuccess )
        {
            IOLog( "org_mklinux_iokit_swim3_driver::start - Couldn't add Interrupt event source\n" );
	    IOLockUnlock(org_mklinux_iokit_swim3_startlock);
            return false;
        }
        
        myWorkLoop->enableAllInterrupts();

        registerService();
	IOLockUnlock(org_mklinux_iokit_swim3_startlock);
	// return res;
        
        return ( createNub ( provider ) );
}

IOService *
org_mklinux_iokit_swim3_driver::instantiateNub ( void )
{

        dIOLog(DEBUG_GENERAL, "org_mklinux_iokit_swim3_driver::instantiateNub entering.\n");

        IOService * nub = new org_mklinux_iokit_swim3_device;

        dIOLog(DEBUG_GENERAL, "org_mklinux_iokit_swim3_driver::instantiateNub exiting nub = %p.\n", nub);
        org_mklinux_swim3_device[this->busdev->unit] = NULL;

        return nub;

}


bool
org_mklinux_iokit_swim3_driver::createNub ( IOService * provider )
{

        IOService *             nub = NULL;

        dIOLog(DEBUG_GENERAL,  "org_mklinux_iokit_swim3_driver::createNub entering.\n" );

        // Instantiate a generic hard disk nub so a generic driver
        // can match above us.
        nub = instantiateNub ( );
        if ( nub == NULL )
        {

                dIOLog(DEBUG_GENERAL, "org_mklinux_iokit_swim3_driver::createNub instantiateNub() failed, returning false\n");
                return false;

        }

        nub->init ( );

        if ( !nub->attach ( this ) )
        {

                // panic since the nub can't attach
                panic("org_mklinux_iokit_swim3_driver::createNub() unable to attach nub" );
                return false;

        }

        nub->registerService ( );

        // The IORegistry retains the nub, so we can release our refcount
        // because we don't need it any more.
        nub->release ( );

        dIOLog(DEBUG_GENERAL, "org_mklinux_iokit_swim3_driver::createNub exiting true.\n");

        return true;

}

void org_mklinux_iokit_swim3_driver::stop(IOService *provider)
{
	dIOLog(DEBUG_GENERAL, "Stopping\n");	
	super::stop(provider);
        
        /* @@@@ disable the interrupt and unregister! @@@ */
        if ( getWorkLoop() )
                 getWorkLoop()->disableAllInterrupts();
                 
        if(interruptSource)
                interruptSource->release();
                
        if(DMAInterruptSource)
                DMAInterruptSource->release();
                 
        if ( workLoop ) {
            workLoop->release(); workLoop = 0;
        }

	org_mklinux_iokit_swim3_dispose_event_table();
	if (org_mklinux_iokit_swim3_startlock) {
		mutex_free(org_mklinux_iokit_swim3_startlock);
		org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "mutex_free\n");
	}
	if (master_floppy_lock) {
		mutex_free(master_floppy_lock);
		org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "mutex_free\n");
	}
#ifndef IOKITTEST
	FloppyPluginCleanup(busdev->unit);
#endif
	if (TrackBuffer[busdev->unit])
		IOFreeContiguous(TrackBuffer[busdev->unit], 0xb000);
		org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "IOFreeContiguous\n");
#if CURRENT_CODE
	IOFree(onepage, PAGE_SIZE * 2);
	org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "IOFree\n");
#endif
	IOFree(busdev, sizeof(*busdev));
	org_mklinux_iokit_swim3_objcount--; dprintf(DEBUG_ALLOC, "IOFree\n");

	printf("Leaked buffers: %d\n", org_mklinux_iokit_swim3_objcount);
	
	// IOLockFree(AsyncLock);
}

#if 0
static void org_mklinux_iokit_swim3_driver_NotifyMediaChange(
	org_mklinux_iokit_swim3_driver *self, BSAccessibilityState status)
{
	dIOLog(DEBUG_GENERAL, "NotifyMediaChange: enter (0x%x)\n", (unsigned int)self);

	/* Do something here */

	switch (status) {
	    case kBSOnline:
		dIOLog(DEBUG_GENERAL, "New state: kBSOnline\n");
		// self->IOMediaState=kIOMediaStateOnline;
		// self->mediaStateHasChanged((IOMediaState)kIOMediaStateOnline);

		/* @@@ Create IOMedia Nub? @@@ */
		break;
	    case kBSOffline:
		dIOLog(DEBUG_GENERAL, "New state: kBSOffline\n");
		// self->IOMediaState=kIOMediaStateOffline;
		// self->mediaStateHasChanged((IOMediaState)kIOMediaStateOffline);

		/* @@@ Create IOMedia Nub? @@@ */
		break;
	}

	dIOLog(DEBUG_GENERAL, "NotifyMediaChange: exit\n");
}
#endif

void org_mklinux_iokit_swim3_driver::handleInterrupt( IOInterruptEventSource *src, int /*count*/ )
{

    /* ssp argument is never used anyway, so pass ourselves.... */
    HALISRHandler(swim3Instance->busdev->unit, swim3Instance);

}

void org_mklinux_iokit_swim3_driver::handleDMAInterrupt( IOInterruptEventSource *src, int /*count*/ )
{

/* ssp argument is never used anyway, so pass ourselves.... */
    HALISR_DMA(swim3Instance->busdev->unit, swim3Instance);

}

char *
org_mklinux_iokit_swim3_driver::getVendorString ( void )
{
        return "Apple Computer, Inc.\n";
}

char *
org_mklinux_iokit_swim3_driver::getProductString ( void )
{
    return "Internal Floppy Drive\n";
}
    
char *
org_mklinux_iokit_swim3_driver::getRevisionString ( void )
{
     return REVISIONSTRING;
}

char *
org_mklinux_iokit_swim3_driver::getAdditionalDeviceInfoString ( void )
{
    return "Driver Copyright 2002, The MkLinux Project.  Portions Copyright Apple Computer, Inc.  Mac OS X Integration by David A. Gatwood and Louis Gerbarg.\n";
}

#ifdef REAL_ASYNC
void org_mklinux_iokit_swim3_driver::doAsyncWrapper()
{
    swim3_async_args_t args;
    IOReturn retval;

    dIOLog(DEBUG_GENERAL, "swim3: entered doAsyncWrapper\n");
    memcpy(&args, &AsyncArgs, sizeof(swim3_async_args_t));

    dIOLog(DEBUG_GENERAL, "swim3: unlocking AsyncLock\n");
    // IOLockUnlock(AsyncLock);
    non_bsd_wakeup((void *)&AsyncLock);

#else
void org_mklinux_iokit_swim3_driver::doAsyncWrapper(swim3_async_args_t args)
{
    IOReturn retval;

    dIOLog(DEBUG_GENERAL, "swim3: entered doAsyncWrapper\n");
#endif

    dIOLog(DEBUG_GENERAL, "swim3: calling doSyncReadWrite asynchronously\n");
    retval = doSyncReadWrite(args.buffer, args.block, args.nblks);

    dIOLog(DEBUG_GENERAL, "swim3: calling completion routine for block %ld-%ld with status %d\n", args.block, (args.block+args.nblks-1), retval);
    IOStorage::complete(args.completion, retval, (retval ? 0 : (args.nblks * 512)));
    dIOLog(DEBUG_GENERAL, "swim3: done calling completion routine.\n");
#if 1
}
#else
}
#endif

IOReturn
org_mklinux_iokit_swim3_driver::doAsyncReadWrite(IOMemoryDescriptor *buffer,
                                UInt32 block,UInt32 nblks,
                                IOStorageCompletion completion)
{
#ifndef REAL_ASYNC
    // swim3_async_args_t args;
    IOReturn retval;
#endif
    int retry=0;
    void *tret;
	
	dIOLog(DEBUG_GENERAL, "swim3:doAsyncReadWrite @@@@@@@@@@@@ START @@@@@@@@@@@@\n");

	dIOLog(DEBUG_GENERAL, "swim3: taking AsyncLock\n");
	// IOLockLock(AsyncLock);
#ifdef REAL_ASYNC
	non_bsd_sleep((void *)&AsyncLock, 0);
	AsyncArgs.buffer = buffer;
	AsyncArgs.block = block;
	AsyncArgs.nblks = nblks;
	AsyncArgs.completion = completion;
#endif

#ifdef REAL_ASYNC
	dIOLog(DEBUG_GENERAL, "swim3: creating thread\n");
	while (retry++ < 5) {
	    if (tret=(void *)IOCreateThread((void *)(void *)&org_mklinux_iokit_swim3_driver::doAsyncWrapper, (void *)this)) {
		dIOLog(DEBUG_GENERAL, "swim3: AsyncReadWrite returning success\n");
		dIOLog(DEBUG_GENERAL, "swim3:doAsyncReadWrite @@@@@@@@@@@@ DONE @@@@@@@@@@@@\n");
		return kIOReturnSuccess;
	    }
	    IOLog("swim3: IOCreateThread failed with error 0x%x\n", tret);
	}
	dIOLog(DEBUG_GENERAL, "swim3: AsyncReadWrite returning failure\n");
	dIOLog(DEBUG_GENERAL, "swim3:doAsyncReadWrite @@@@@@@@@@@@ DONE @@@@@@@@@@@@\n");
	return kIOReturnIOError;
#else // REAL_ASYNC

    dIOLog(DEBUG_GENERAL, "swim3: calling doSyncReadWrite\n");
    retval = doReadWrite(buffer, block, nblks, 1);
    // doSyncReadWrite(buffer, block, nblks);

    dIOLog(DEBUG_GENERAL, "swim3: calling completion routine for block %d-%d with status %d\n", block, (block+nblks-1), retval);
    if (!retval) {
      IOStorage::complete(completion, retval, (retval ? 0 : (nblks * 512)));
	dIOLog(DEBUG_GENERAL, "swim3: returning success.\n");
	dIOLog(DEBUG_GENERAL, "swim3:doAsyncReadWrite @@@@@@@@@@@@ DONE @@@@@@@@@@@@\n");
    }
    return kIOReturnSuccess;
#endif // REAL_ASYNC
}


bool org_mklinux_iokit_swim3_driver::createWorkLoop()
{
    workLoop = IOWorkLoop::workLoop();     

    return ( workLoop != 0 );
}/* end createWorkLoop */


        /* Override IOService::getWorkLoop() method to return our workloop.     */

IOWorkLoop* org_mklinux_iokit_swim3_driver::getWorkLoop() const
{
    return workLoop;
}/* end getWorkLoop */

//---------------------------------------------------------------------------
// attach to provider.

#undef super
#define super IOBlockStorageDevice
OSDefineMetaClassAndStructors ( org_mklinux_iokit_swim3_device, IOBlockStorageDevice );
 

bool
org_mklinux_iokit_swim3_device::attach ( IOService * provider )
{

#if 0
	OSDictionary *	dictionary = OSDictionary::withCapacity ( 1 );
#endif
        
	if ( !super::attach ( provider ) )
		return false;
	
	fProvider = OSDynamicCast ( org_mklinux_iokit_swim3_driver, provider );
	if ( fProvider == NULL )
	{
		
		IOLog ( "org_mklinux_iokit_swim3_device: attach; wrong provider type!\n" );
		return false;
		
	}
	
#if 0
        
	if ( dictionary != NULL )
	{
		
		dictionary->setObject ( kIOPropertyPhysicalInterconnectTypeKey, fProvider->getProperty ( kIOPropertyPhysicalInterconnectTypeKey ) );
		dictionary->setObject ( kIOPropertyPhysicalInterconnectLocationKey, fProvider->getProperty ( kIOPropertyPhysicalInterconnectLocationKey ) );
		
		setProperty ( kIOPropertyProtocolCharacteristicsKey, dictionary );
		
		dictionary->release ( );
		
	} else {
		IOLog("Null Dictionary!\n");
	}
	
	dictionary = OSDictionary::withCapacity ( 3 );
	if ( dictionary != NULL )
	{
		
		char *		string;
		OSString *	theString;
		
		string = getVendorString ( );
		
		theString = OSString::withCString ( string );
		if ( theString != NULL )
		{
			dictionary->setObject ( kIOPropertyVendorNameKey, theString );
			theString->release ( );
		}

		string = getProductString ( );

		theString = OSString::withCString ( string );
		if ( theString != NULL )
		{
			dictionary->setObject ( kIOPropertyProductNameKey, theString );
			theString->release ( );
		}

		string = getRevisionString ( );

		theString = OSString::withCString ( string );
		if ( theString != NULL )
		{
			dictionary->setObject ( kIOPropertyProductRevisionLevelKey, theString );
			theString->release ( );
		}

		string = "true";

		theString = OSString::withCString ( string );
		if ( theString != NULL )
		{
			dictionary->setObject ( kIOMediaWritableKey, theString );
			theString->release ( );
		}
		
		setProperty ( kIOPropertyDeviceCharacteristicsKey, dictionary );
		
		dictionary->release ( );
		
	} else {
		IOLog("Null Dictionary!\n");
	}

#endif

	return true;
	
}



//---------------------------------------------------------------------------
// flush the cache.

IOReturn
org_mklinux_iokit_swim3_driver::doSynchronizeCache ( void )
{
#ifndef IOKITTEST
    FloppyPluginFlush(this->busdev->unit, 0, 0);     /*Flush */
#endif
    return kIOReturnSuccess;
}


//---------------------------------------------------------------------------
// detach from provider.

void
org_mklinux_iokit_swim3_device::detach ( IOService * provider )
{
	
	if ( fProvider == provider )
		fProvider = 0;
	
	super::detach ( provider );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_driver::setProperties ( OSObject * properties )
{

#if 0
	OSDictionary *	dict 		= NULL;
	OSNumber *		number 		= NULL;
	IOReturn		status 		= kIOReturnSuccess;
	UInt32			features	= 0;
#endif
	OSDictionary *dict = OSDynamicCast(OSDictionary, properties);
	OSNumber *debugProp = NULL;

	IOLog( "org_mklinux_iokit_swim3_device::setProperties called\n");

	if (dict) {
		debugProp = OSDynamicCast(OSNumber,
			dict->getObject("SWIM3 Debug"));
	}

	if (debugProp) {
		org_mklinux_iokit_swim3_debug = debugProp->unsigned32BitValue();
	}
	
#if 0
        
	number = ( OSNumber * ) fProvider->getProperty ( kIOATASupportedFeaturesKey );
	features = number->unsigned32BitValue ( );
	
	if ( ( features & kIOATAFeatureAdvancedPowerManagement ) == 0 )
	{
		
		IOLog( "org_mklinux_iokit_swim3_device::setProperties called on unsupported drive\n" );
		return kIOReturnUnsupported;
		
	}
	
	dict = OSDynamicCast ( OSDictionary, properties );
	if ( dict != NULL )
	{
		number = OSDynamicCast ( OSNumber, dict->getObject ( "APM Level" ) );
	}
	
	if ( number != NULL )
	{
				
		UInt8	apmLevel = number->unsigned8BitValue ( );
		
		if ( ( apmLevel == 0 ) || ( apmLevel == 0xFF ) )
		{
			return kIOReturnBadArgument;
		}

		IOLog( "apmLevel = %d\n", apmLevel );
		
		status = fProvider->setAdvancedPowerManagementLevel ( apmLevel, true );
		
	}
	
	else
		status = kIOReturnBadArgument;
	
	IOLog( "org_mklinux_iokit_swim3_device::leave setProperties\n" );
	
        return status;
        
#endif 

	return kIOReturnSuccess;
        // return kIOReturnUnsupported;
}



IOReturn
org_mklinux_iokit_swim3_device::setProperties ( OSObject * properties )
{

#if 0
	OSDictionary *	dict 		= NULL;
	OSNumber *		number 		= NULL;
	IOReturn		status 		= kIOReturnSuccess;
	UInt32			features	= 0;
#endif
	OSDictionary *dict = OSDynamicCast(OSDictionary, properties);
	OSNumber *debugProp = NULL;

	IOLog( "org_mklinux_iokit_swim3_device::setProperties called\n");

	if (dict) {
		debugProp = OSDynamicCast(OSNumber,
			dict->getObject("SWIM3 Debug"));
	}

	if (debugProp) {
		org_mklinux_iokit_swim3_debug = debugProp->unsigned32BitValue();
	}
	
#if 0
        
	number = ( OSNumber * ) fProvider->getProperty ( kIOATASupportedFeaturesKey );
	features = number->unsigned32BitValue ( );
	
	if ( ( features & kIOATAFeatureAdvancedPowerManagement ) == 0 )
	{
		
		IOLog( "org_mklinux_iokit_swim3_device::setProperties called on unsupported drive\n" );
		return kIOReturnUnsupported;
		
	}
	
	dict = OSDynamicCast ( OSDictionary, properties );
	if ( dict != NULL )
	{
		number = OSDynamicCast ( OSNumber, dict->getObject ( "APM Level" ) );
	}
	
	if ( number != NULL )
	{
				
		UInt8	apmLevel = number->unsigned8BitValue ( );
		
		if ( ( apmLevel == 0 ) || ( apmLevel == 0xFF ) )
		{
			return kIOReturnBadArgument;
		}

		IOLog( "apmLevel = %d\n", apmLevel );
		
		status = fProvider->setAdvancedPowerManagementLevel ( apmLevel, true );
		
	}
	
	else
		status = kIOReturnBadArgument;
	
	IOLog( "org_mklinux_iokit_swim3_device::leave setProperties\n" );
	
        return status;
        
#endif 

	return kIOReturnSuccess;
        // return kIOReturnUnsupported;
}




//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::doAsyncReadWrite (
											IOMemoryDescriptor *	buffer,
											UInt32					block,
											UInt32					nblks,
											IOStorageCompletion		completion )
{
	
	// Block incoming I/O if we have been terminated
	if ( isInactive ( ) != false )
	{
		return kIOReturnNotAttached;
	}
	
//	fProvider->checkPowerState ( );
	return fProvider->doAsyncReadWrite ( buffer, block, nblks, completion );
	
}


//---------------------------------------------------------------------------
//

IOReturn
org_mklinux_iokit_swim3_device::doSyncReadWrite (	IOMemoryDescriptor * buffer,
											UInt32 block, UInt32 nblks )
{
	
	// Block incoming I/O if we have been terminated
	if ( isInactive ( ) != false )
	{
		return kIOReturnNotAttached;
	}
	
//	fProvider->checkPowerState ( );
	return fProvider->doSyncReadWrite ( buffer, block, nblks );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::doEjectMedia ( void )
{
	
	return fProvider->doEjectMedia ( );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::doFormatMedia ( UInt64 byteCapacity )
{
	
	return fProvider->doFormatMedia ( byteCapacity );
	
}


//---------------------------------------------------------------------------
// 

UInt32
org_mklinux_iokit_swim3_device::doGetFormatCapacities ( UInt64 * capacities,
												 UInt32   capacitiesMaxCount ) const
{
	
	return fProvider->doGetFormatCapacities ( capacities, capacitiesMaxCount );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::doLockUnlockMedia ( bool doLock )
{
	
	return fProvider->doLockUnlockMedia ( doLock );
	
}


//---------------------------------------------------------------------------
//

IOReturn
org_mklinux_iokit_swim3_device::doSynchronizeCache ( void )
{
	
	// Block incoming I/O if we have been terminated
	if ( isInactive ( ) != false )
	{
		return kIOReturnNotAttached;
	}
	
//	fProvider->checkPowerState ( );
	return fProvider->doSynchronizeCache ( );
	
}

//---------------------------------------------------------------------------
//

char *
org_mklinux_iokit_swim3_device::getVendorString ( void )
{

        return fProvider->getVendorString ( );

}


//---------------------------------------------------------------------------
//

char *
org_mklinux_iokit_swim3_device::getProductString ( void )
{

        return fProvider->getProductString ( );

}


//---------------------------------------------------------------------------
//

char *
org_mklinux_iokit_swim3_device::getRevisionString ( void )
{

        return fProvider->getRevisionString ( );

}


//---------------------------------------------------------------------------
//

char *
org_mklinux_iokit_swim3_device::getAdditionalDeviceInfoString ( void )
{
       
        return fProvider->getAdditionalDeviceInfoString ( );

}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportBlockSize ( UInt64 * blockSize )
{
	
	return fProvider->reportBlockSize ( blockSize );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportEjectability ( bool * isEjectable )
{
	
	return fProvider->reportEjectability ( isEjectable );

}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportLockability ( bool * isLockable )
{
	
	return fProvider->reportLockability ( isLockable );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportPollRequirements ( 	bool * pollIsRequired,
													bool * pollIsExpensive )
{
	
	return fProvider->reportPollRequirements ( pollIsRequired, pollIsExpensive );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportMaxReadTransfer ( UInt64 blockSize,
													UInt64 * max )
{
	
	return fProvider->reportMaxReadTransfer ( blockSize, max );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportMaxValidBlock ( UInt64 * maxBlock )
{

	return fProvider->reportMaxValidBlock ( maxBlock );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportMaxWriteTransfer ( UInt64	blockSize,
												  UInt64 *	max )
{
	
	return fProvider->reportMaxWriteTransfer ( blockSize, max );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportMediaState ( bool * mediaPresent,
											bool * changed )	
{
	
	return fProvider->reportMediaState ( mediaPresent, changed );
	
}


//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportRemovability ( bool * isRemovable )
{
	
	return fProvider->reportRemovability ( isRemovable );
	
}



//---------------------------------------------------------------------------
// 

IOReturn
org_mklinux_iokit_swim3_device::reportWriteProtection ( bool * isWriteProtected )
{
	
	return fProvider->reportWriteProtection ( isWriteProtected );
	
}

