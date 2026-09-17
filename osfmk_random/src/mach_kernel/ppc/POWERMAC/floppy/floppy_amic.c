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
 * Based on scsi_amic.c, SCSI DMA support, from MkLinux DR3
 *
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
 * 
 */
/*
 * MkLinux
 */

#include <fd.h>

#if     NFD > 0
#include <platforms.h>
#include <ppc/proc_reg.h> /* For isync */
#include <mach_debug.h>
#include <kern/spl.h>
#include <mach/std_types.h>
#include <types.h>
#include <chips/busses.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/floppy/floppy_amic.h>

#include "portdef.h"
#include "floppypriv.h"

#if 0
// Included in floppy_amic.h

void	floppy_amic_init(void);
void	floppy_amic_setup(vm_offset_t address, unsigned int count);
void	floppy_amic_start(boolean_t isread);
void	floppy_amic_end(void);

floppy_curio_dma_ops_t	floppy_amic_ops = {
	floppy_amic_init,
	floppy_amic_setup,
	floppy_amic_start,
	floppy_amic_end
};

struct dma_softc {
	volatile unsigned char	*base;
	volatile unsigned char	*ctrl;
} floppy_amic_softc[NFD];

typedef struct dma_softc dma_softc_t;

#endif

extern
SonyVarsType SonyVariables[];

void
floppy_amic_init()
{
	int unit = 0;
	dma_softc_t	*dmap = &floppy_amic_softc[unit];

	// printf("floppy_amic_init ");

	/* @@@ TODO DG @@@ */
	dmap->base = (v_u_char *) POWERMAC_IO(PDM_FLOPPY_AMIC_BASE_PHYS+0x0);
	dmap->ctrl = (v_u_char *) POWERMAC_IO(PDM_FLOPPY_AMIC_BASE_PHYS+0x8); // 4? 5? 9?
	dmap->bytecount_high = (v_u_char *) POWERMAC_IO(PDM_FLOPPY_AMIC_BASE_PHYS+0x4);
	dmap->bytecount_low  = (v_u_char *) POWERMAC_IO(PDM_FLOPPY_AMIC_BASE_PHYS+0x5);

	// 0x032081 == 0x032060 + 0x21
	//

#if 0
	/* Rather sorry hack */
	SonyVariables[0].driveStatus[0].transferAddress.physicalAddress = (char *)
		((dmap->base[0] << 24) + (dmap->base[1] << 16) + (dmap->base[2] << 8)
		 + dmap->base[3]);
	SonyVariables[0].driveStatus[0].transferAddress.physicalAddress =
		POWERMAC_IO(SonyVariables[0].driveStatus[0].transferAddress.logicalAddress);
#endif
}


void
floppy_amic_end()
{
	int unit = 0;
	dma_softc_t	*dmap = &floppy_amic_softc[unit];

	// printf("floppy_amic_end ");

	/*
	 * Flush out the DMA, and kill it.
	 */

	eieio();
#if 0
	// No such thing as a floppy DMA flush
	if ((*dmap->ctrl & MASK8(PDM_FLOPPY_DMA_CTRL_DIR)) == 0) {
		*dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_CTRL_FLUSH);
		do {
			eieio();
		} while (*dmap->ctrl & MASK8(PDM_FLOPPY_DMA_CTRL_FLUSH));
	}
#endif

#if 0
	// maybe this will work better?  Dunno.
	*dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_CTRL_RESET); eieio();

	while (*dmap->ctrl & MASK8(PDM_FLOPPY_DMA_CTRL_RUN))
		eieio();
	isync();

	// floppy_reset_offset();
#else
	*dmap->ctrl &= ~MASK8(PDM_FLOPPY_DMA_CTRL_RUN); eieio();

	while (*dmap->ctrl & MASK8(PDM_FLOPPY_DMA_CTRL_RUN))
		eieio();
#endif
}

void
floppy_amic_setup(vm_offset_t addr, unsigned int count)
{
	int unit = 0;
	dma_softc_t	*dmap = &floppy_amic_softc[unit];
	
	// printf("floppy_amic_setup ");

	*dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_CTRL_RESET); eieio();

	while (*dmap->ctrl & MASK8(PDM_FLOPPY_DMA_CTRL_RUN))
		eieio();
	isync();
	// floppy_reset_offset();

	if (addr & 7) 
		panic("FLOPPY DMA - non-aligned address");

	// printf("Address: 0x%x ", addr);
	// printf("Count: %d ", count);

	if (addr) {
		dmap->base[0] = (unsigned char)((addr >> 24) & 0xff); // will fail - RO
		dmap->base[1] = (unsigned char)((addr >> 16) & 0xff); // will fail - RO
		dmap->base[2] = (unsigned char)((addr >>  8) & 0xff);
		dmap->base[3] = (unsigned char)((addr) & 0xff);
	}

	eieio();

	if (count) {
		*dmap->bytecount_high = ((count >> 8) & 0xff);
		*dmap->bytecount_low  = (count & 0xff);
		eieio();
	}

	*dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_INTERRUPT_ENABLE); eieio();

	return;
}

void
floppy_amic_start(boolean_t isread)
{
	int unit = 0;
	dma_softc_t	*dmap = &floppy_amic_softc[unit];
	
	// printf("floppy_amic_start ");

	if (isread) {
		*dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_CTRL_RUN); eieio();
		*dmap->ctrl &= ~MASK8(PDM_FLOPPY_DMA_CTRL_DIR);
		}
	else
		*dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_CTRL_DIR)|MASK8(PDM_FLOPPY_DMA_CTRL_RUN);
	eieio();

	// printf("done ");

	return;
}

void
floppy_amic_ack()
{
	int unit = 0;
	dma_softc_t	*dmap = &floppy_amic_softc[unit];

	if (dmap->ctrl) {
		// *dmap->ctrl &= ~MASK8(PDM_FLOPPY_DMA_INTERRUPT); eieio();
		*dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_INTERRUPT); eieio();
		// *dmap->ctrl |= MASK8(PDM_FLOPPY_DMA_CTRL_RESET); eieio();
	}
	return;
}

#endif 	// NFD > 0
