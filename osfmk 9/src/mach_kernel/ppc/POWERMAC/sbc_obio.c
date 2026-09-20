/*	$NetBSD: sbc_obio.c,v 1.8 1998/05/02 16:45:31 scottr Exp $	*/

/*
 * Copyright (C) 1996,1997 Scott Reynolds.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifdef __netbsd__

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/user.h>

#include <dev/scsipi/scsi_all.h>
#include <dev/scsipi/scsipi_all.h>
#include <dev/scsipi/scsipi_debug.h>
#include <dev/scsipi/scsiconf.h>

#include <dev/ic/ncr5380reg.h>
#include <dev/ic/ncr5380var.h>

#include <machine/cpu.h>
#include <machine/viareg.h>

#include <mac68k/dev/sbcreg.h>
#include <mac68k/dev/sbcvar.h>

#define NPSC 0

#else
#include "psc.h"
#endif


#if NPSC > 0

#include <platforms.h>
#include <ppc/proc_reg.h> /* For isync */

typedef unsigned char asc_register_t;
#define PAD(n) u_char n[15];

#define ASC_PROBE_DYNAMICALLY FALSE

#include <kern/spl.h>
#include <mach/std_types.h>
#include <types.h>
#include <chips/busses.h>
#include <scsi/compat_30.h>

// #include <scsi/scsi.h>
// #include <scsi/scsi2.h>
// #include <scsi/scsi_defs.h>

// #include <ppc/POWERMAC/scsi_53C94.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_gestalt.h>
#include <ppc/POWERMAC/device_tree.h>

#include <kern/assert.h>
#include <kern/cpu_number.h>
#include <vm/vm_kern.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/io_map_entries.h>
#include <ppc/POWERMAC/pci.h>
#include <ppc/POWERMAC/pcireg.h>
#include <ppc/POWERMAC/pcivar.h>

#include "bsdtypes.h"
#include "cdefs.h"
#include "queue.h"

// #include <sys/types.h>
// #include <sys/param.h>
// #include <sys/systm.h>
// #include <sys/kernel.h>
// #include <sys/errno.h>
// #include <sys/device.h>
// #include <sys/buf.h>
// #include <sys/proc.h>
// #include <sys/user.h>

#include "scsipi/scsi_all.h"
#include "scsipi/scsipi_all.h"
#include "scsipi/scsipi_debug.h"
#include "scsipi/scsiconf.h"

#include "ncr5380reg.h"
#include "ncr5380var.h"

// #include <machine/cpu.h>
// #include <machine/viareg.h>

#include "sbcreg.h"
#include "sbcvar.h"

#endif // __netbsd__

/*
 * From Guide to the Macintosh Family Hardware, pp. 137-143
 * These are offsets from PSC_BASE_PHYS (see pmap_bootstrap.c)
 */
#define	SBC_REG_OFS		0x10000
#define	SBC_DMA_OFS		0x12000
#define	SBC_HSK_OFS		0x06000

#define	SBC_DMA_OFS_PB500	0x06000

#define	SBC_REG_OFS_IIFX	0x08000		/* Just guessing... */
#define	SBC_DMA_OFS_IIFX	0x0c000
#define	SBC_HSK_OFS_IIFX	0x0e000

#define	SBC_REG_OFS_DUO2	0x00000
#define	SBC_DMA_OFS_DUO2	0x02000
#define	SBC_HSK_OFS_DUO2	0x04000

#define PSC_REG_OFS		0x10000
#define PSC_DMA_OFS		0x12000
#define PSC_HSF_OSF		0x06000

static int	sbc_obio_match __P((mach_device_t, struct cfdata *, void *));
static void	sbc_obio_attach __P((mach_device_t, struct device *, void *));

void	sbc_intr_enable __P((struct ncr5380_softc *));
void	sbc_intr_disable __P((struct ncr5380_softc *));
void	sbc_obio_clrintr __P((struct ncr5380_softc *));

struct cfattach sbc_obio_ca = {
	sizeof(struct sbc_softc), sbc_obio_match, sbc_obio_attach
};


struct sbc_softc self;


/*
 * Autoconfiguration data for config.
 */
void	psc_probe_for_targets(asc_softc_t asc, asc_curio_regmap_t *regs);
int	psc_probe( caddr_t reg, void *ptr);
boolean_t psc_probe_target(target_info_t *tgt, io_req_t ior);
void	psc_alloc_cmdptr(asc_softc_t asc, target_info_t *tgt);


/* Various MACH driver variables.. */
caddr_t	psc_std[NPSC] = { 0 };
struct	bus_device *psc_dinfo[NPSC*8];
struct	bus_ctlr *psc_minfo[NPSC];
struct	bus_driver psc_driver = 
        { psc_probe, scsi_slave, scsi_attach, (int(*)(void))psc_go,
	  psc_std, "rz", psc_dinfo, "psc", psc_minfo, BUS_INTR_B4_PROBE};


#if 0
caddr_t	mesh_std[NMESH] = { 0 };
struct	bus_device *mesh_dinfo[NMESH*8];
struct	bus_ctlr *mesh_minfo[NMESH];
struct	bus_driver mesh_driver = 
        { mesh_probe, scsi_slave, scsi_attach, (int(*)(void))mesh_go,
	  mesh_std, "rz", mesh_dinfo, "mesh", mesh_minfo, BUS_INTR_B4_PROBE};
#endif



#ifdef __netbsd__
// static int
// sbc_obio_match(parent, cf, args)
//	struct device *parent;
//	struct cfdata *cf;
//	void *args;
static int sbc_obio_match()
{
	switch (current_mac_model->machineid) {
	case MACH_MACIIFX:	/* Note: the IIfx isn't (yet) supported. */
/*
		if (cf->cf_unit == 0)
			return 1;
*/
		break;
	case MACH_MACPB210:
	case MACH_MACPB230:
	case MACH_MACPB250:
	case MACH_MACPB270:
	case MACH_MACPB280:
	case MACH_MACPB280C:
		if (cf->cf_unit == 1)
			return 1;
		/*FALLTHROUGH*/
	default:
		if (cf->cf_unit == 0 && mac68k_machine.scsi80)
			return 1;
	}
	return 0;
}
#else
// MkLinux

int psc_probe( caddr_t reg, void *ptr)
{
switch(powermac_info.class) {
	case POWERMAC_CLASS_POWERBOOK:
		break;	
	default:
		return 0;
	}

psc_attach();
psc_probe_for_targets();
psc_get_config();

return 1; // (powermac_info.class==POWERMAC_CLASS_POWERBOOK);
}

void psc_probe_for_targets()
{


}

#endif

//static void
//sbc_obio_attach(parent, self, args)
//	struct device *parent, *self;
//	void *args;

static void
psc_attach(void *args)
{
	struct sbc_softc *sc = (struct sbc_softc *) self;
	struct ncr5380_softc *ncr_sc = (struct ncr5380_softc *) sc;
	char bits[64];
	// extern vm_offset_t PSC_BASE_PHYS;
	int s;

	/* Pull in the options flags. */ 
	sc->sc_options = ((ncr_sc->sc_dev.dv_cfdata->cf_flags | sbc_options)
	    & SBC_OPTIONS_MASK);

	/*
	 * Set up offsets to 5380 registers and GLUE I/O space, and turn
	 * off options we know we can't support on certain models.
	 */

#if 0
	switch (current_mac_model->machineid) {
	case MACH_MACIIFX:	/* Note: the IIfx isn't (yet) supported. */
		sc->sc_regs = (struct sbc_regs *)(PSC_BASE_PHYS + SBC_REG_OFS_IIFX);
		sc->sc_drq_addr = (vm_offset_t)(PSC_BASE_PHYS + SBC_HSK_OFS_IIFX);
		sc->sc_nodrq_addr = (vm_offset_t)(PSC_BASE_PHYS + SBC_DMA_OFS_IIFX);
		sc->sc_options &= ~(SBC_INTR | SBC_RESELECT);
		break;
	case MACH_MACPB500:
		sc->sc_regs = (struct sbc_regs *)(PSC_BASE_PHYS + SBC_REG_OFS);
		sc->sc_drq_addr = (vm_offset_t)(PSC_BASE_PHYS + SBC_HSK_OFS); /*??*/
		sc->sc_nodrq_addr = (vm_offset_t)(PSC_BASE_PHYS + SBC_DMA_OFS_PB500);
		sc->sc_options &= ~(SBC_INTR | SBC_RESELECT);
		break;
	case MACH_MACPB210:
	case MACH_MACPB230:
	case MACH_MACPB250:
	case MACH_MACPB270:
	case MACH_MACPB280:
	case MACH_MACPB280C:
		if (ncr_sc->sc_dev.dv_unit == 1) {
			sc->sc_regs = (struct sbc_regs *)(0xfee00000 + SBC_REG_OFS_DUO2);
			sc->sc_drq_addr = (vm_offset_t)(0xfee00000 + SBC_HSK_OFS_DUO2);
			sc->sc_nodrq_addr = (vm_offset_t)(0xfee00000 + SBC_DMA_OFS_DUO2);
			break;
		}
		/*FALLTHROUGH*/
	default:
		sc->sc_regs = (struct sbc_regs *)(PSC_BASE_PHYS + PSC_REG_OFS);
		sc->sc_drq_addr = (vm_offset_t)(PSC_BASE_PHYS + PSC_HSK_OFS);
		sc->sc_nodrq_addr = (vm_offset_t)(PSC_BASE_PHYS + PSC_DMA_OFS);
		break;
	}
#endif
	sc->sc_regs = (struct sbc_regs *)(PSC_BASE_PHYS + PSC_REG_OFFSET);
	sc->sc_drq_addr = (vm_offset_t)(PSC_BASE_PHYS + PSC_HSK_OFFSET);
	sc->sc_nodrq_addr = (vm_offset_t)(PSC_BASE_PHYS + PSC_DMA_OFFSET);

#if 1 // PDMA turned on
	sc->sc_options &= ~(SBC_INTR | SBC_RESELECT);
#else // PDMA turned off
	// sc->sc_options &= ~(SBC_INTR | SBC_PDMA | SBC_RESELECT);
#endif // 1

	/*
	 * Fill in the prototype scsi_link.
	 */
	ncr_sc->sc_link.scsipi_scsi.channel = SCSI_CHANNEL_ONLY_ONE;
	ncr_sc->sc_link.adapter_softc = sc;
	ncr_sc->sc_link.scsipi_scsi.adapter_target = 7;
	ncr_sc->sc_link.adapter = &sbc_ops;
	ncr_sc->sc_link.device = &sbc_dev;
	ncr_sc->sc_link.type = BUS_SCSI;

	/*
	 * Initialize fields used by the MI code
	 */
	ncr_sc->sci_r0 = &sc->sc_regs->sci_pr0.sci_reg;
	ncr_sc->sci_r1 = &sc->sc_regs->sci_pr1.sci_reg;
	ncr_sc->sci_r2 = &sc->sc_regs->sci_pr2.sci_reg;
	ncr_sc->sci_r3 = &sc->sc_regs->sci_pr3.sci_reg;
	ncr_sc->sci_r4 = &sc->sc_regs->sci_pr4.sci_reg;
	ncr_sc->sci_r5 = &sc->sc_regs->sci_pr5.sci_reg;
	ncr_sc->sci_r6 = &sc->sc_regs->sci_pr6.sci_reg;
	ncr_sc->sci_r7 = &sc->sc_regs->sci_pr7.sci_reg;

	/*
	 * MD function pointers used by the MI code.
	 */
	if (sc->sc_options & SBC_PDMA) {
		ncr_sc->sc_pio_out   = sbc_pdma_out;
		ncr_sc->sc_pio_in    = sbc_pdma_in;
	} else {
		ncr_sc->sc_pio_out   = ncr5380_pio_out;
		ncr_sc->sc_pio_in    = ncr5380_pio_in;
	}
	ncr_sc->sc_dma_alloc = NULL;
	ncr_sc->sc_dma_free  = NULL;
	ncr_sc->sc_dma_poll  = NULL;
	ncr_sc->sc_intr_on   = NULL;
	ncr_sc->sc_intr_off  = NULL;
	ncr_sc->sc_dma_setup = NULL;
	ncr_sc->sc_dma_start = NULL;
	ncr_sc->sc_dma_eop   = NULL;
	ncr_sc->sc_dma_stop  = NULL;
	ncr_sc->sc_flags = 0;
	ncr_sc->sc_min_dma_len = MIN_DMA_LEN;

	s=splhigh();

	if (sc->sc_options & SBC_INTR) {
		ncr_sc->sc_dma_alloc = sbc_dma_alloc;
		ncr_sc->sc_dma_free  = sbc_dma_free;
		ncr_sc->sc_dma_poll  = sbc_dma_poll;
		ncr_sc->sc_dma_setup = sbc_dma_setup;
		ncr_sc->sc_dma_start = sbc_dma_start;
		ncr_sc->sc_dma_eop   = sbc_dma_eop;
		ncr_sc->sc_dma_stop  = sbc_dma_stop;
		
		pmac_register_int(PMAC_DEV_SCSI0, SPLBIO, psc_intr);
//		via2_register_irq(VIA2_SCSIDRQ, sbc_drq_intr, ncr_sc);
	}

//	via2_register_irq(VIA2_SCSIIRQ, sbc_irq_intr, ncr_sc);

	pmac_register_int(PMAC_DEV_SCSI0, SPLBIO, psc_intr);
	sc->sc_clrintr = sbc_obio_clrintr;

	if ((sc->sc_options & SBC_RESELECT) == 0)
		ncr_sc->sc_no_disconnect = 0xff;

	if (sc->sc_options)
		printf(": options=%s", bitmask_snprintf(sc->sc_options,
		    SBC_OPTIONS_BITS, bits, sizeof(bits)));
	printf("\n");

	if (sc->sc_options & (SBC_INTR|SBC_RESELECT)) {
		/* Enable SCSI interrupts through VIA2 */
		sbc_intr_enable(ncr_sc);
	}

#ifdef SBC_DEBUG
	if (sbc_debug)
		printf("%s: softc=%p regs=%p\n", ncr_sc->sc_dev.dv_xname,
		    sc, sc->sc_regs);
	ncr_sc->sc_link.flags |= sbc_link_flags;
#endif

	/*
	 *  Initialize the SCSI controller itself.
	 */
	ncr5380_init(ncr_sc);
	ncr5380_reset_scsibus(ncr_sc);
	config_found(self, &(ncr_sc->sc_link), scsiprint);
}

/*
 * Interrupt support routines.
 */
void
sbc_intr_enable(ncr_sc)
	struct ncr5380_softc *ncr_sc;
{
	struct sbc_softc *sc = (struct sbc_softc *)ncr_sc;
	int s, flags;

	flags = V2IF_SCSIIRQ;
	if (sc->sc_options & SBC_INTR)
		flags |= V2IF_SCSIDRQ;

	s = splhigh();
	if (VIA2 == VIA2OFF)
		via2_reg(vIER) = 0x80 | flags;
	else
		via2_reg(rIER) = 0x80 | flags;
	splx(s);
}

void
sbc_intr_disable(ncr_sc)
	struct ncr5380_softc *ncr_sc;
{
	struct sbc_softc *sc = (struct sbc_softc *)ncr_sc;
	int s, flags;

	flags = V2IF_SCSIIRQ;
	if (sc->sc_options & SBC_INTR)
		flags |= V2IF_SCSIDRQ;

	s = splhigh();
	if (VIA2 == VIA2OFF)
		via2_reg(vIER) = flags;
	else
		via2_reg(rIER) = flags;
	splx(s);
}

void
sbc_obio_clrintr(ncr_sc)
	struct ncr5380_softc *ncr_sc;
{
	struct sbc_softc *sc = (struct sbc_softc *)ncr_sc;
	int flags;

	flags = V2IF_SCSIIRQ;
	if (sc->sc_options & SBC_INTR)
		flags |= V2IF_SCSIDRQ;

	if (VIA2 == VIA2OFF)
		via2_reg(vIFR) = 0x80 | flags;
	else
		via2_reg(rIFR) = 0x80 | flags;
}


psc_intr(int a, void *b)
{

sbc_irq_intr(self);
sbc_drq_intr(self);
handle_intr(self);



}
