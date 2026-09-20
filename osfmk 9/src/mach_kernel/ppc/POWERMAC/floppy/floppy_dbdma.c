/*
 * Portions Copyright (c) 1999 The MkLinux Project.
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
 *	This product includes software developed by The MkLinux Project
 *	and its contributors.
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
 */
/*
 * floppy_dbdma.c - extra functions for dbdma floppy support
 * written by David A. Gatwood.
 *
 * based in part on code written by Apple Computer
 *
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
#include <ppc/proc_reg.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/powermac_pdm.h>
#include <ppc/POWERMAC/dbdma.h>
#include "floppy_dbdma.h"

#include <fd.h>

#if NFD > 0

#define	DBDMA_FLOPPY		0x1

#if (!MACH_DEBUG)
#define printf donone
#endif

extern void FlushProcessorCache(uint_t, LogicalAddress, uint_t);

/* floppy_dbdma_setup - builds a CCL */
void floppy_dbdma_setup(dbdma_regmap_t *channelConnection,
			     boolean_t isread,
			     PhysicalAddress addressPtr,
			     LogicalAddress cclPtr,
			     ByteCount transferCount)
{
    UInt32 op;
    UInt32 thisTime;
    UInt32 byteCount = transferCount;
    UInt8 *bufferAddr = (UInt8 *) addressPtr;
    // static dbdma_command_t desc;
    dbdma_command_t *DescPtr = (dbdma_command_t *)cclPtr; // &desc;
//	(dbdma_command_t *)(channelConnection->d_cmdptrlo);
//	                    + (channelConnection->d_cmdptrhi << 32));

    // Stop any running channel commands.
    floppy_dbdma_set_chan_ctrl(channelConnection,
		      DBDMA_CLEAR_CNTRL(DBDMA_CNTRL_RUN) |
		      DBDMA_CLEAR_CNTRL(DBDMA_CNTRL_PAUSE) |
		      DBDMA_CLEAR_CNTRL(DBDMA_CNTRL_DEAD));

    op = isread ? DBDMA_CMD_IN_MORE : DBDMA_CMD_OUT_MORE;

    // build the CCL
    while (byteCount) {
	thisTime = (byteCount < 0xFFF0) ? byteCount : 0xFFF0;
	byteCount -= thisTime;

	DBDMA_BUILD(DescPtr, op, 0, thisTime, (long)(bufferAddr),
		(byteCount) ? DBDMA_INT_NEVER : DBDMA_INT_ALWAYS,
	        DBDMA_BRANCH_NEVER, DBDMA_WAIT_NEVER);
	// MakeCCDescriptor(DescPtr, op | thisTime, (long) bufferAddr);
	bufferAddr += thisTime;
	// printf("DescPtr=0x%x,decriptors=0x%x,0x%x,0x%x,0x%x ", (long) DescPtr, ((long *) DescPtr)[0], ((long *) DescPtr)[1], ((long *) DescPtr)[2], ((long *) DescPtr)[3]);
	DescPtr += 1;
    }

    // MakeCCDescriptor(DescPtr, STOP_CMD, 0);

    /* We don't want the stop to interrupt, do we??? --------------\/ */
    DBDMA_BUILD(DescPtr, DBDMA_CMD_STOP, 0, thisTime, (long)(bufferAddr),
		DBDMA_INT_NEVER, DBDMA_BRANCH_NEVER, DBDMA_WAIT_NEVER);
    DescPtr += 1;

#if 0
    FlushProcessorCache(CurrentAddressSpaceID(),
			privChannel->cclStaticLogicalAddr,
	((UInt32) DescPtr - (UInt32) privChannel->cclStaticLogicalAddr));
#endif
}


void floppy_dbdma_prep(dbdma_regmap_t *regs, char *comptr)
{
    // regs->d_cmdptrlo = (unsigned long)(comptr); eieio();
    DBDMA_ST4_ENDIAN((&regs->d_cmdptrlo), (unsigned long)(comptr)); eieio();
}


extern unsigned long ccCommandsLogicalAddr[];
void PrintDMA()
{
    dbdma_regmap_t *rg;
    unsigned long DescPtr = (unsigned long) ccCommandsLogicalAddr;

    if (powermac_info.class = POWERMAC_CLASS_PCI)
	rg = (dbdma_regmap_t *) (PCI_DMA_BASE_PHYS + (DBDMA_FLOPPY << 8));
    else return;

    printf("PtrLo=0x%04x,ccntl=0x%04x,cstat=0x%04x ", rg->d_cmdptrlo, rg->d_control, rg->d_status);
    printf("decriptors=0x%x,0x%x,0x%x,0x%x ", ((long *) DescPtr)[0], ((long *) DescPtr)[1], ((long *) DescPtr)[2], ((long *) DescPtr)[3]);
}

#endif // (NFD > 0)

