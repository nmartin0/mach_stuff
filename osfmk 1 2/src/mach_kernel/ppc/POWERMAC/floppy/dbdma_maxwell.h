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


// This structure defines the standard set of DB-DMA channel registers.

struct DBDMAChannelRegisters {
    unsigned long channelControl;
    unsigned long channelStatus;
    unsigned long commandPtrHi;	// implementation optional

    unsigned long commandPtrLo;
    unsigned long interruptSelect;	// implementation optional

    unsigned long branchSelect;	// implementation optional

    unsigned long waitSelect;	// implementation optional

    unsigned long transferModes;	// implementation optional

    unsigned long data2PtrHi;	// implementation optional

    unsigned long data2PtrLo;	// implementation optional

    unsigned long reserved1;
    unsigned long addressHi;	// implementation optional

    unsigned long reserved2[4];
    unsigned long unimplemented[16];

    // This structure must remain fully padded to 256 bytes.
    unsigned long undefined[32];
};
typedef struct DBDMAChannelRegisters DBDMAChannelRegisters;


// These constants define the DB-DMA channel control words and status flags.

//enum DBDMAChannelControlCommands {
#define 	kdbdmaSetRun      0x80008000
#define 	kdbdmaClrRun	  0x80000000
#define 	kdbdmaSetPause	  0x40004000
#define 	kdbdmaClrPause	  0x40000000
#define 	kdbdmaSetFlush	  0x20002000
#define 	kdbdmaSetWake	  0x10001000
#define 	kdbdmaClrDead	  0x08000000

#define 	kdbdmaSetS7		  0x00800080
#define 	kdbdmaClrS7		  0x00800000
#define 	kdbdmaSetS6		  0x00400040
#define 	kdbdmaClrS6		  0x00400000
#define 	kdbdmaSetS5		  0x00200020
#define 	kdbdmaClrS5		  0x00200000
#define 	kdbdmaSetS4		  0x00100010
#define 	kdbdmaClrS4		  0x00100000
#define 	kdbdmaSetS3		  0x00080008
#define 	kdbdmaClrS3		  0x00080000
#define 	kdbdmaSetS2		  0x00040004
#define 	kdbdmaClrS2		  0x00040000
#define 	kdbdmaSetS1		  0x00020002
#define 	kdbdmaClrS1		  0x00020000
#define 	kdbdmaSetS0		  0x00010001
#define 	kdbdmaClrS0		  0x00010000

#define 	kdbdmaClrAll	  0xFFFF0000
//};

//enum DBDMAChannelStatusMasks {
#define 	kdbdmaRun		  0x00008000
#define 	kdbdmaPause		  0x00004000
#define 	kdbdmaFlush		  0x00002000
#define 	kdbdmaWake		  0x00001000
#define 	kdbdmaDead		  0x00000800
#define 	kdbdmaActive	  0x00000400
#define 	kdbdmaBt		  0x00000100

#define 	kdbdmaS7		  0x00000080
#define 	kdbdmaS6		  0x00000040
#define 	kdbdmaS5		  0x00000020
#define 	kdbdmaS4		  0x00000010
#define 	kdbdmaS3		  0x00000008
#define 	kdbdmaS2		  0x00000004
#define 	kdbdmaS1		  0x00000002
#define 	kdbdmaS0		  0x00000001
//};


// This structure defines the DB-DMA channel command descriptor.

// *** WARNING: Endian-ness issues must be considered when performing load/store! ***
// ***                  DB-DMA specifies memory organization as quadlets so it is not correct
// ***                  to think of either the operation or result field as two 16-bit fields.
// ***                  This would have undesirable effects on the byte ordering within their
// ***                  respective quadlets. Use the accessor macros provided below.

struct DBDMADescriptor {
    unsigned long operation;	// cmd || key || i || b || w || reqCount

    unsigned long address;
    unsigned long cmdDep;
    unsigned long result;	// xferStatus || resCount

};
typedef struct DBDMADescriptor DBDMADescriptor;
typedef DBDMADescriptor *DBDMADescriptorPtr;


// These constants define the DB-DMA channel command operations and modifiers.

//enum DBDMACCCommands {
	// Command.cmd operations
#define	OUTPUT_MORE			 0x00000000
#define	OUTPUT_LAST			 0x10000000
#define	INPUT_MORE			 0x20000000
#define	INPUT_LAST			 0x30000000
#define	STORE_QUAD			 0x40000000
#define	LOAD_QUAD			 0x50000000
#define	NOP_CMD				 0x60000000
#define	STOP_CMD			 0x70000000
#define	kdbdmaCmdMask		 0xF0000000
//};

enum DBDMACCCommandModifiers {
    // Command.key modifiers (choose one for INPUT, OUTPUT, LOAD, and STORE)
    KEY_STREAM0 = 0x00000000,	// default modifier
     KEY_STREAM1 = 0x01000000,
    KEY_STREAM2 = 0x02000000,
    KEY_STREAM3 = 0x03000000,
    KEY_REGS = 0x05000000,
    KEY_SYSTEM = 0x06000000,
    KEY_DEVICE = 0x07000000,
    kdbdmaKeyMask = 0x07000000,

    // Command.i modifiers (choose one for INPUT, OUTPUT, LOAD, STORE, and NOP)
    kIntNever = 0x00000000,	// default modifier
     kIntIfTrue = 0x00100000,
    kIntIfFalse = 0x00200000,
    kIntAlways = 0x00300000,
    kdbdmaIMask = 0x00300000,

    // Command.b modifiers (choose one for INPUT, OUTPUT, and NOP)
    kBranchNever = 0x00000000,	// default modifier
     kBranchIfTrue = 0x00040000,
    kBranchIfFalse = 0x00080000,
    kBranchAlways = 0x000C0000,
    kdbdmaBMask = 0x000C0000,

    // Command.w modifiers (choose one for INPUT, OUTPUT, LOAD, STORE, and NOP)
    kWaitNever = 0x00000000,	// default modifier
     kWaitIfTrue = 0x00010000,
    kWaitIfFalse = 0x00020000,
    kWaitAlways = 0x00030000,
    kdbdmaWMask = 0x00030000

    // operation masks
};

#define kdbdmaCommandMask		  0xFFFF0000
#define 	kdbdmaReqCountMask		  0x0000FFFF


// These constants define the DB-DMA channel command results.

enum DBDMACCXferStatuses {
    kXferStatusRun = kdbdmaRun << 16,
    kXferStatusPause = kdbdmaPause << 16,
    kXferStatusFlush = kdbdmaFlush << 16,
    kXferStatusWake = kdbdmaWake << 16,
    kXferStatusDead = kdbdmaDead << 16,
    kXferStatusActive = kdbdmaActive << 16,

    kXferStatusBt = kdbdmaBt << 16,

    kXferStatusS7 = kdbdmaS7 << 16,
    kXferStatusS6 = kdbdmaS6 << 16,
    kXferStatusS5 = kdbdmaS5 << 16,
    kXferStatusS4 = kdbdmaS4 << 16,
    kXferStatusS3 = kdbdmaS3 << 16,
    kXferStatusS2 = kdbdmaS2 << 16,
    kXferStatusS1 = kdbdmaS1 << 16,
    kXferStatusS0 = kdbdmaS0 << 16,

    // result masks
    kdbdmaResCountMask = 0x0000FFFF,
    kdbdmaXferStatusMask = kdbdmaResCountMask << 16
};

typedef void *DBDMAChannelConnectionPtr;

///////////////////////////////////////////////////////////////////////////////////
//
// Channel Connections
//
extern OSStatus OpenDBDMAChannel(DBDMAChannelRegisters * DBDMAPtr,
			   DBDMAChannelConnectionPtr * channelConnection,
				 uint_t cclNum,
				 LogicalAddress * logicalAddr,
				 PhysicalAddress * physicalAddr);


extern void CloseDBDMAChannel(DBDMAChannelConnectionPtr channelConnection);

///////////////////////////////////////////////////////////////////////////////////
//
// Client Buffer Assignment
//
extern void SetDBDMAPhysicalAddress(
			     DBDMAChannelConnectionPtr channelConnection,
				       boolean_t isReadTransfer,
				       PhysicalAddress addressPtr,
				       uint_t transferCount);

///////////////////////////////////////////////////////////////////////////////////
//
// Channel Control Operations
//
extern void PrintDMA(void);
extern void StartDBDMA(DBDMAChannelConnectionPtr channelConnection);

extern void StopDBDMA(DBDMAChannelConnectionPtr channelConnection);

extern void ResetDBDMA(DBDMAChannelConnectionPtr channelConnection);

extern void PrepDBDMA(DBDMAChannelConnectionPtr channelConnection);

#define	DBDMA_SET_CNTRL(x)	( ((x) | (x) << 16) )

#define	DBDMA_BUILD(d, cmd, key, count, address, interrupt, wait, branch) {\
		DBDMA_ST4_ENDIAN(&d->d_cmd_count, \
				((cmd) << 28) | ((key) << 24) |\
				((interrupt) << 20) |\
				((branch) << 18) | ((wait) << 16) | \
				(count)); \
		DBDMA_ST4_ENDIAN(&d->d_address, address); \
		(d)->d_status_resid = 0; \
		(d)->d_cmddep = 0; \
	}

static __inline__ void dbdma_st4_endian(volatile unsigned long *a, unsigned long x)
{
    __asm__ volatile
     ("stwbrx %0,0,%1"::"r" (x), "r"(a):"memory");

    return;
}

static __inline__ unsigned long dbdma_ld4_endian(volatile unsigned long *a)
{
    unsigned long swap;

    __asm__ volatile
     ("lwbrx %0,0,%1":"=r" (swap):"r"(a));

    return swap;
}

#define	DBDMA_LD4_ENDIAN(a) 	dbdma_ld4_endian(a)
#define	DBDMA_ST4_ENDIAN(a, x) 	dbdma_st4_endian(a, x)
#define SetChannelControl(a,x)  dbdma_st4_endian(&(a->channelControl),x)
#define SetCommandPtr(a,x)      dbdma_st4_endian(&(a->commandPtrLo),x)
static __inline__ void SetPDMCommandPtr(volatile unsigned long *a, unsigned short x)	{
	__asm__ volatile
	 ("sthbrx %0,0,%1"::"r" (x), "r"(a):"memory");

	return;
	}
#define MakeCCDescriptor(d,op,addr) {\
		DBDMA_ST4_ENDIAN(&d->operation,(op)); \
		DBDMA_ST4_ENDIAN(&d->address, addr); \
		(d)->result = 0; \
		(d)->cmdDep = 0; \
	}
