// #undef MACH_DEBUG
// #define MACH_DEBUG 1

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

#ifndef __PORTDEF_H__
#define __PORTDEF_H__
#include "floppyerr.h"
//#include <sys/types.h>
#include <mach/ppc/boolean.h>
void donone(char *,...);
typedef int OSStatus;
typedef long InterruptMemberNumber;
typedef char *LogicalAddress;
typedef char *PhysicalAddress;
typedef long InterruptSetID;	/* it is actually struct opaque in COPLAND */
struct InterruptSetMember {
    InterruptSetID setID;
    InterruptMemberNumber member;
};
typedef struct InterruptSetMember InterruptSetMember;
#define noErr 0
#define true  1
#define false 0
typedef uint_t RegEntryID;
typedef uint_t RegEntryRef;
typedef RegEntryRef BSContainerRef;
typedef char *BytePtr;
typedef unsigned char Byte;
typedef uint_t UInt32;
typedef unsigned char UInt8;
typedef uint_t AreaID;
typedef uint_t KernelProcessID;
typedef uint_t OSType;
typedef boolean_t Boolean;
typedef uint_t ByteCount;
typedef uint_t BSComponentType;
typedef uint_t BSByteCount;
typedef uint_t *BSStorePtr;
typedef uint_t ItemCount;
typedef uint_t BSStoreID;
typedef char *Str255;
typedef char *Str32;
typedef uint_t BSBlockListDescriptorRef;
typedef void *MemListDescriptorRef;
typedef uint_t BSIORequestBlockPtr;
typedef uint_t BSMappingPlugInRef;
typedef uint_t BSPartitioningPlugInRef;
typedef int OptionBits;

struct BSPartitionDescriptor {
    ItemCount entryNum;
    BSByteCount start;
    BSByteCount len;
    BSStoreID destStoreID;
    Boolean isPartitionable;
    Boolean isFilesystem;
    Boolean reserved1;
    Boolean reserved2;
    Str255 name;		/* Max name size is 255 (0 terminated) */
    Str32 typeName;		/* Max type name is 32 (0 terminated) */
};

typedef struct BSPartitionDescriptor *BSPartitionDescriptorPtr;

struct BSStoreMPIComponent {
    BSComponentType componentType;
    BSByteCount startingOffset;
    RegEntryRef sourceNode;	/* Physical components */
    BSStorePtr srcStore;	/* Logical components */
    struct BSPartitionDescriptor partitionInfo;		/* Logical components */
};

typedef struct BSStoreMPIComponent *BSStoreMPIComponentPtr;

enum {
    kDurationMillisecond = 1L,
    kIsrIsComplete = 0,
    kIsrIsNotComplete = -1

};
enum {
    kBSRead = 0, kBSWrite = 1
};


enum {
    kBSMPIDeviceNotSupported = 0,
    kBSMPIDeviceTypeRecognized = 1,
    kBSMPIDeviceMfrRecognized = 2,
    kBSMPIDeviceModelRecognized = 3,
    kBSMPIDeviceMediaRecognized = 4
};
typedef UInt32 BSMPIConfidenceLevel;

/*******************************************************************************

	Mapping Plug-In Level of Confidence values
	
 ******************************************************************************/

enum {
    kBSCPIDeviceNotSupported = 0,
    kBSCPIDeviceTypeRecognized = 1,
    kBSCPIDeviceMfrRecognized = 2,
    kBSCPIDeviceModelRecognized = 3
};
typedef UInt32 BSCPIConfidenceLevel;

/*******************************************************************************

	Plug In Version - this tracks the revision of the plug-in interface and
		should be placed in the version field of the plug-in interface structure
	
 ******************************************************************************/
enum {
    kBSPlugInInterfaceVersion = 0x02011996
};

/*******************************************************************************

	BSIOStatus values
	
 ******************************************************************************/

enum {
    kBSIOCompleted = 1,
    kBSIOContinuing = 2,
    kBSIOFailed = 3,
    kBSIONotStarted = 4
};
typedef UInt32 BSIOStatus;

/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
 
	Types
	
 *******************************************************************************
 *******************************************************************************
 ******************************************************************************/

typedef void *BSContainerPtr;


typedef OSStatus BSIOErrors;

/*******************************************************************************
	BSErrorList:  another structure to manage I/O success/failure.

	BSErrorList data structure allows a Mapping Plug-In to keep track of the
	current state of each transfer request sent to it by the Block Storage client.  If transfers complete
	successfully, a pointer 0 is returned.  If an error occurs, a linked list (?) of these data structures
	is returned specifying the exact state of each transfer request.

	startingBlock:		The initial transfer block of this fragment of the request
	blockCount:			The number of blocks to be transfered including the startingBlock
	status:				A constant representing the final state of the transfer of the blocks:
 								kIONotStarted:				No processing was begun on these blocks
								kIOStarted:					The BS Plug-In began to process the blocks
								kIOToFamily:				The request was sent to the device family
								kIOComplete:				The request completed normally
 								kIOFailed:					The request failed to complete execution
	error:				The exact error, if any
	xferID:				An index representing which request of a multiple-request I/O this is
 	next:				A pointer to the next data structure (could be eliminated in favor of indices)
 ******************************************************************************/
struct BSErrorList {
    BSByteCount startingBlock;
    BSByteCount length;
    UInt32 status;
    BSIOErrors error;
    ItemCount xferID;
    struct BSErrorList *next;
};
typedef struct BSErrorList *BSErrorListPtr;

/*******************************************************************************

	BSStoreMPIInfo - Information about the Store that the Mapping Plug-in can
		provide
		
 ******************************************************************************/
enum {
    kBSFormatFloppyGCR = 0,
    kBSFormatFloppyMFM = 1,
    kBSFormatSCSI = 2,
    kBSFormatATA = 3,
    kBSNotFormatable = 4
};

enum {
    kBSMaxFormats = 8,
    kRegMaxEntryNameLength = 47
};
enum {
    E_Success = 0x00000000,
    E_LoopTermination = 0x08000000,
    E_Underflow = 0x10000000,
    E_Overflow = 0x18000000,
    E_AlreadyExists = 0x20000000,
    E_NotFound = 0x28000000,
    E_AccessViolation = 0x30000000,
    E_Busy = 0x38000000,
    E_VersionMismatch = 0x40000000,
    E_Canceled = 0x48000000,
    E_OutOfResources = 0x50000000,
    E_Timeout = 0x58000000,
    E_ParameterError = 0x60000000,
    E_Fatal = 0x68000000,
    E_Unknown = 0x78000000
};

/*****************************************************************************

	Block Storage Error ID
	
 ****************************************************************************/
enum {
    E_BlockStorageBias = 0x04f00000
};


/*****************************************************************************

	Block Storage Error Categories
	
 ****************************************************************************/
enum {
    E_BSFamilyError = 0x00000000 | E_BlockStorageBias,
    E_BSExpertError = 0x00008000 | E_BlockStorageBias,
    E_BSMappingPlugInError = 0x00010000 | E_BlockStorageBias,
    E_BSPartitioningPlugInError = 0x00018000 | E_BlockStorageBias,
    E_BSContainerPlugInError = 0x00020000 | E_BlockStorageBias,
    E_BSBlockListError = 0x00028000 | E_BlockStorageBias,
    E_BSSuccess = E_Success
};

/*****************************************************************************

	Family Errors
	
 ****************************************************************************/
enum {
    E_BSOutOfResources = 0x00000001 | E_OutOfResources | E_BSFamilyError,
    E_BSStoreInUse = 0x00000001 | E_AccessViolation | E_BSFamilyError,
    E_BSStoreWriteProtected = 0x00000002 | E_AccessViolation | E_BSFamilyError,
    E_BSStoreNotFound = 0x00000001 | E_NotFound | E_BSFamilyError,
    E_BSBadMessage = 0x00000001 | E_ParameterError | E_BSFamilyError,
    E_BSBadConnection = 0x00000002 | E_ParameterError | E_BSFamilyError,
    E_BSTableTooSmall = 0x00000003 | E_ParameterError | E_BSFamilyError,
    E_BSNullParameters = 0x00000004 | E_ParameterError | E_BSFamilyError,
    E_BSParameterError = 0x00000005 | E_ParameterError | E_BSFamilyError,
    E_BSUnimplemented = 0x00000001 | E_VersionMismatch | E_BSFamilyError
};

/*****************************************************************************

	Expert Errors
	
 ****************************************************************************/
enum {
    E_BSEPlugInNotFound = 0x00000001 | E_NotFound | E_BSExpertError,
    E_BSENoPlugInMatch = 0x00000002 | E_NotFound | E_BSExpertError,
    E_BSENoMoreStores = 0x00000001 | E_OutOfResources | E_BSExpertError,
    E_BSEHierarchyTooDeep = 0x00000002 | E_OutOfResources | E_BSExpertError,
    E_BSEOutOfResources = 0x00000003 | E_OutOfResources | E_BSExpertError
};

/*****************************************************************************

	Mapping Plug-in Errors
	
 ****************************************************************************/
enum {
    E_BSMPIOutOfStoreBounds = 0x00000001 | E_ParameterError | E_BSMappingPlugInError,
    E_BSMPITooManyMappings = 0x00000002 | E_ParameterError | E_BSMappingPlugInError,
    E_BSMPIBadMappingParams = 0x00000003 | E_ParameterError | E_BSMappingPlugInError,
    E_BSMPIMappingNotSupported = 0x00000004 | E_ParameterError | E_BSMappingPlugInError,
    E_BSMPIStateNotSupported = 0x00000005 | E_ParameterError | E_BSMappingPlugInError,
    E_BSMPIWriteProtected = 0x00000001 | E_AccessViolation | E_BSMappingPlugInError,
    E_BSMPICannotGoToState = 0x00000002 | E_AccessViolation | E_BSMappingPlugInError,
    E_BSMPIUnitNotResponding = 0x00000001 | E_Fatal | E_BSMappingPlugInError,
    E_BSMPITransferError = 0x00000002 | E_Fatal | E_BSMappingPlugInError,
    E_BSMPIMemoryAccessFault = 0x00000003 | E_Fatal | E_BSMappingPlugInError,
    E_BSMPINoPlugIn = 0x00000004 | E_Fatal | E_BSMappingPlugInError,
    E_BSMPIMediaRemoved = 0x00000005 | E_Fatal | E_BSMappingPlugInError,
    E_BSMPIOutOfResources = 0x00000001 | E_OutOfResources | E_BSMappingPlugInError
};

/*****************************************************************************

	Partitioning Plug-in Errors
	
 ****************************************************************************/

enum {
    E_BSPPIMappingNotSupported = 0x00000001 | E_ParameterError | E_BSPartitioningPlugInError,
    E_BSPPIOverlappingPartition = 0x00000002 | E_ParameterError | E_BSPartitioningPlugInError,
    E_BSPPIOutOfStoreBounds = 0x00000003 | E_ParameterError | E_BSPartitioningPlugInError,
    E_BSPPIPartitionNonExistant = 0x00000004 | E_ParameterError | E_BSPartitioningPlugInError,
    E_BSPPITooManyPartitions = 0x00000005 | E_ParameterError | E_BSPartitioningPlugInError,
    E_BSPPINoPlugIn = 0x00000001 | E_Fatal | E_BSPartitioningPlugInError,
    E_BSPPIOutOfResources = 0x00000002 | E_OutOfResources | E_BSPartitioningPlugInError
};

/*****************************************************************************

	Container Plug-in Errors
	
 ****************************************************************************/

/*****************************************************************************

	Block List Errors
	
 ****************************************************************************/
enum {
    E_BSBLEndOfList = 0x00000001 | E_Underflow | E_BSBlockListError,
    E_BSBLParameterError = 0x00000001 | E_ParameterError | E_BSBlockListError,
    E_BSBLBadBlockList = 0x00000002 | E_ParameterError | E_BSBlockListError,
    E_BSBLBadBlock = 0x00000003 | E_ParameterError | E_BSBlockListError,
    E_BSBLAlreadyFinalized = 0x00000001 | E_AlreadyExists | E_BSBlockListError,
    E_BSBLOutOfResources = 0x00000001 | E_OutOfResources | E_BSBlockListError
};
typedef OSType BSStoreFormatType;
typedef uint_t BSAccessibilityState;
typedef uint_t BSStoreFormatInfo;
struct BSStoreMPIInfo {
    BSAccessibilityState curState;
    BSByteCount storeSize;
    BSByteCount readBlockSize;	/* minimum read size & granularity */
    BSByteCount writeBlockSize;	/* minimum write size & granularity */

    Boolean isEjectable;
    Boolean isWriteable;
    Boolean hasAutoEjectHardware;
    Boolean isFormattable;

    Boolean isPartitionable;
    Boolean isFilesystem;
    Boolean reserved1;
    Boolean reserved2;

    BSStoreFormatInfo curFormat;

    BSStoreFormatInfo possibleFormats[kBSMaxFormats];

    char name[kRegMaxEntryNameLength + 1];
};

typedef struct BSStoreMPIInfo *BSStoreMPIInfoPtr;
typedef ItemCount BSFormatIndex;

struct BSStoreInfo {
    BSStoreID storeID;
    BSByteCount storeSize;
    BSByteCount readBlockSize;	/* minimum read size & granularity */
    BSByteCount writeBlockSize;	/* minimum write size & granularity */
    BSContainerRef container;	/* the container containing this Store */
    ItemCount numChildren;
    ItemCount numParents;

    ItemCount numPartitions;	/* Number of partitions in use */
    ItemCount maxPartitions;	/* Maximum number of partitions possible */

    Boolean isPartitioned;	/* True if a partitioning plug-in is associated with this Store */

    Boolean isEjectable;
    Boolean isBootDevice;
    Boolean isWriteable;
    Boolean hasAutoEjectHardware;
    Boolean isFormattable;

    Boolean isPartitionable;
    Boolean isFilesystem;

    BSMappingPlugInRef mappingPlugIn;
    BSPartitioningPlugInRef partitioningPlugIn;

    BSStoreFormatInfo curFormat;

    BSStoreFormatInfo possibleFormats[kBSMaxFormats];

    Str255 name;		/* Max name size is 255 (0 terminated) */
    Str32 typeName;		/* Max type name is 32 (0 terminated) */
};
enum {
    kBSOffline = 0,
    kBSOnline = 1
};
enum {
    FLOP_ENODEV = 9,
    FLOP_MEM_ERROR = 10
};

#endif
extern OSStatus MemListDescriptorDataCopy(MemListDescriptorRef srcDescriptor,
	     MemListDescriptorRef destDescriptor, ByteCount bytesToCopy);
extern OSStatus MemListDescriptorDataCopyToMemory(MemListDescriptorRef srcDescriptor,
		       LogicalAddress destBuffer, ByteCount bytesToCopy);
extern OSStatus MemListDescriptorDataCopyFromMemory(LogicalAddress srcBuffer,
	     MemListDescriptorRef destDescriptor, ByteCount bytesToCopy);

extern OSStatus MemListDescriptorDataCompare(MemListDescriptorRef descriptor1,
	      MemListDescriptorRef descriptor2, ByteCount bytesToCompare,
					     Boolean * different,
					 ByteCount * firstDifferentByte);
extern OSStatus MemListDescriptorDataCompareWithMemory(
		  MemListDescriptorRef descriptor, LogicalAddress buffer,
			   ByteCount bytesToCompare, Boolean * different,
					 ByteCount * firstDifferentByte);
extern void SynchronizeIO(void);
OSStatus BSMPINotifyFamilyStoreChangedState(void *OSDiskID, int event);
void PrintRegs(void);
