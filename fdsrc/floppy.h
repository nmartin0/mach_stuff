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

#include <IOKit/IOService.h>
#include <IOKit/platform/AppleMacIODevice.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/IOInterruptEventSource.h>
extern "C" {
#define MACH_KERNEL
#include "bus_device.h"
}
#define REAL_ASYNC

/* Use this if you need to turn this into a stub driver for testing. */
/* #define IOKITTEST */

#define REVISIONSTRING "0.7 (2002-04-01)"

struct swim3_async_args {
        IOMemoryDescriptor *buffer;
        UInt32 block;   
        UInt32 nblks;
        IOStorageCompletion completion;
};

typedef struct swim3_async_args swim3_async_args_t;

class org_mklinux_iokit_swim3_driver : public IOService
{
OSDeclareDefaultStructors(org_mklinux_iokit_swim3_driver)
private:
    IOWorkLoop *              workLoop;
    IOInterruptEventSource *  interruptSource;
    IOInterruptEventSource *  DMAInterruptSource;
    org_mklinux_iokit_swim3_driver  *swim3Instance;
    swim3_async_args_t AsyncArgs;
    IOLock *AsyncLock;
    IOLock *fdFormatLock;
    bool locked;

    void handleInterrupt( IOInterruptEventSource *src, int /*count*/ );
    void handleDMAInterrupt( IOInterruptEventSource *src, int /*count*/ );

public:	  

        virtual bool createWorkLoop();
        virtual IOWorkLoop* getWorkLoop() const;
        AppleMacIODevice *DAG_Rules;
        struct bus_device *busdev;
        bool	media_present;
        caddr_t vaddr, paddr;
        caddr_t dma_vaddr, dma_paddr;
	virtual bool init(OSDictionary *dictionary = 0);
	virtual void free(void);	
	virtual IOService *probe(IOService *provider, SInt32 *score);
        virtual IOService * instantiateNub ( void );
        virtual bool createNub ( IOService * provider );		
	virtual bool start(IOService *provider);	
	virtual void stop(IOService *provider);
        virtual char *		getVendorString ( void );
        virtual char *		getProductString ( void );
        virtual char *		getRevisionString ( void );
        virtual char *		getAdditionalDeviceInfoString ( void );
        virtual IOReturn doAsyncReadWrite(IOMemoryDescriptor *buffer,
                                UInt32 block,UInt32 nblks,
                                IOStorageCompletion completion);
        virtual IOReturn doEjectMedia(void);
        virtual IOReturn doFormatMedia(UInt64 byteCapacity);
	virtual UInt32 doGetFormatCapacities(UInt64 * capacities,
                                UInt32 capacitiesMaxCount);
        virtual IOReturn doLockUnlockMedia(bool doLock);
        virtual IOReturn doReadWrite(IOMemoryDescriptor *buffer,
                                UInt32 block,UInt32 nblks, int is_async);
        virtual IOReturn doSyncReadWrite(IOMemoryDescriptor *buffer,
                                UInt32 block,UInt32 nblks);
        virtual IOReturn doSynchronizeCache(void);
        virtual IOReturn reportBlockSize(UInt64 *blockSize);
        virtual IOReturn reportEjectability(bool *isEjectable);
        virtual IOReturn reportLockability(bool *isLockable);
        virtual IOReturn reportMaxReadTransfer(UInt64 blockSize,UInt64 *max);
        virtual IOReturn reportMaxValidBlock(UInt64 *maxBlock);
        virtual IOReturn reportMaxWriteTransfer(UInt64 blockSize,UInt64 *max);
        virtual IOReturn reportMediaState(bool *mediaPresent,bool *changedState);
        virtual IOReturn reportPollRequirements(bool *pollRequired,
                                bool *pollIsExpensive);
        virtual IOReturn reportRemovability(bool *isRemovable);
        virtual IOReturn reportWriteProtection(bool *isWriteProtected);
#ifdef REAL_ASYNC
	void doAsyncWrapper();
#else
	void doAsyncWrapper(swim3_async_args_t args);
#endif
        virtual IOReturn	setProperties ( OSObject * properties );

};

class org_mklinux_iokit_swim3_device : public IOBlockStorageDevice
{

        OSDeclareDefaultStructors ( org_mklinux_iokit_swim3_device )
    
protected:
    
        org_mklinux_iokit_swim3_driver *     fProvider;
    
    	virtual bool		attach ( IOService * provider );
	virtual void		detach ( IOService * provider );
        
public:

        virtual IOReturn	setProperties ( OSObject * properties );
	
	virtual IOReturn	doAsyncReadWrite ( 	IOMemoryDescriptor *	buffer,
											UInt32					block,
											UInt32					nblks,
											IOStorageCompletion		completion );

	virtual IOReturn	doSyncReadWrite ( 	IOMemoryDescriptor *	buffer,
											UInt32					block,
											UInt32					nblks );
	
    virtual IOReturn	doEjectMedia ( void );
	
    virtual IOReturn	doFormatMedia ( UInt64 byteCapacity );
	
    virtual UInt32		doGetFormatCapacities ( UInt64 *	capacities,
    											UInt32		capacitiesMaxCount ) const;
	
    virtual IOReturn	doLockUnlockMedia ( bool doLock );
	
    virtual IOReturn	doSynchronizeCache ( void );
        
    virtual char *		getVendorString ( void );
    
    virtual char *		getProductString ( void );
    
    virtual char *		getRevisionString ( void );
    
    virtual char *		getAdditionalDeviceInfoString ( void );
    
    virtual IOReturn	reportBlockSize ( UInt64 * blockSize );
    
    virtual IOReturn	reportEjectability ( bool * isEjectable );
    
    virtual IOReturn	reportLockability ( bool * isLockable );
    
    virtual IOReturn	reportMediaState ( bool * mediaPresent, bool * changed );
    
    virtual IOReturn	reportPollRequirements ( 	bool * pollIsRequired,
    												bool * pollIsExpensive );
    
    virtual IOReturn	reportMaxReadTransfer ( UInt64 blockSize, UInt64 * max );
    
    virtual IOReturn	reportMaxValidBlock ( UInt64 * maxBlock );
    
    virtual IOReturn	reportMaxWriteTransfer ( UInt64 blockSize, UInt64 * max );
    
    virtual IOReturn	reportRemovability ( bool * isRemovable );
    
    virtual IOReturn	reportWriteProtection ( bool * isWriteProtected );
    
};


