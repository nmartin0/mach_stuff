/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 *	File: scc_8530_hdw.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/91
 *
 *	Hardware-level operations for the SCC Serial Line Driver
 */

/*
 *	3/18/99
 *	ported from darwin to osfmach3 for mklinux
 *	-- Tom Vier <thomassr@erols.com>
 */

#define	NSCC	1	/* Number of serial chips, two ports per chip. */
#if	NSCC > 0

#include <mach_kdb.h>
#include <mach_kgdb.h>

#include <platforms.h>

#include <kgdb/gdb_defs.h>
#include <kgdb/kgdb_defs.h>     /* For kgdb_printf */

#include <kern/spl.h>
#include <kern/cpu_number.h>
#include <mach/std_types.h>
#include <types.h>
#include <sys/syslog.h>
#include <device/io_req.h>
#include <device/tty.h>
#include <chips/busses.h>
#include <ppc/misc_protos.h>
#include <ppc/exception.h>
#include <ppc/io_map_entries.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_performa.h>
#include <ppc/POWERMAC/interrupts.h>
#include <ppc/POWERMAC/dma_funnel.h>
#include <ppc/POWERMAC/device_tree.h>
#include <ppc/POWERMAC/powermac_gestalt.h>
#include <ppc/POWERMAC/scc_8530.h>
#include <ppc/POWERMAC/serial_io.h>

#if	MACH_KDB
#include <machine/db_machdep.h>
#endif	/* MACH_KDB */

#define	kdebug_state()	(1)
#define	scc_delay(x)	{ volatile int _d_; for (_d_ = 0; _d_ < (10000*x); _d_++) ; }

#if	!MACH_KGDB
#define	SCC_DMA_TRANSFERS	1
#else
/* 
 * Don't deal with DMA because of the way KGDB needs to handle
 * the system and serial ports.
 */
#define	SCC_DMA_TRANSFERS	0
#endif
  
struct tty scc_tty[NSCC_LINE];
int scc_carrier[NSCC_LINE] = {1, 1};   	/* 0=ignore carrier,
					   1/2=pay attention */

#define scc_tty_for(chan)	(&scc_tty[chan])
#define scc_dev_no(chan) ((chan)^0x01)
#define scc_chan(dev_no) ((dev_no)^0x01)

int	serial_initted = 0;

static struct scc_byte {
	unsigned char	reg;
	unsigned char	val;
} scc_init_hw[] = {
	9, 0x40,	/* SCC_WR9_RESET_CHA_B */
	4, 0x44,	/* SCC_WR4_CLK_x16| SCC_WR4_1_STOP, */
	3, 0xC0,	/* SCC_WR3_RX_8_BITS| */
	5, 0xE2,	/* SCC_WR5_DTR| SCC_WR5_TX_8_BITS| SCC_WR5_RTS, */
	2, 0x00,
	10, 0x00,	/* SCC_WR10_NRZ| SCC_WR10_8BIT_SYNCH, */
	11, 0x50,	/* SCC_WR11_RCLK_BAUDR| SCC_WR11_XTLK_BAUDR|
			   SCC_WR11_XTLK_RTc_PIN| SCC_WR11_TRcOUT_XTAL, */
	12, 0x01,
	13, 0x00,
	3, 0xC1,	/* SCC_WR3_RX_8_BITS| SCC_WR3_RX_ENABLE, */
	5, 0xEA,	/* SCC_WR5_DTR| SCC_WR5_TX_8_BITS|
			   SCC_WR5_TX_ENABLE| SCC_WR5_RTS, */
	14, 0x01,	/* SCC_WR14_BAUDR_ENABLE, */
	15, 0x00,
	0, 0x10,	/* SCC_RESET_EXT_IP, */
	0, 0x10,	/* SCC_RESET_EXT_IP, */
	1, 0x12,	/* SCC_WR1_RXI_ALL_CHAR|SCC_WR1_TX_IE, */
	9, 0x0A		/* SCC_WR9_MASTER_IE| SCC_WR9_NV, */

};

static int	scc_init_hw_count = sizeof(scc_init_hw)/sizeof(scc_init_hw[0]);

enum scc_error {SCC_ERR_NONE, SCC_ERR_PARITY, SCC_ERR_BREAK, SCC_ERR_OVERRUN};


/*
 * BRG formula is:
 *				ClockFrequency (115200 for Power Mac)
 *	BRGconstant = 	---------------------------  -  2
 *			      BaudRate
 */

#define SERIAL_CLOCK_FREQUENCY (115200*2) /* Power Mac value */
#define	convert_baud_rate(rate)	((((SERIAL_CLOCK_FREQUENCY) + (rate)) / (2 * (rate))) - 2)

#define DEFAULT_SPEED 9600
#define DEFAULT_FLAGS (TF_LITOUT|TF_ECHO)

int	machine_has_pmu(void);
void	scc_attach(struct bus_device *ui );
void	scc_set_modem_control(scc_softc_t scc, boolean_t on);
int	scc_pollc(int unit, boolean_t on);
int	scc_param(struct tty *tp);
int	scc_mctl(struct tty* tp, int bits, int how);
int	scc_cd_scan(void);
void	scc_start(struct tty *tp);
void	scc_intr(int device, struct ppc_saved_state *);
void	scc_input(dev_t dev, int c, enum scc_error err);
void	scc_stop(struct tty *tp, int flags);
void	scc_update_modem(struct tty *tp);
void	scc_waitforempty(struct tty *tp);

struct scc_softc	scc_softc[NSCC];
caddr_t	scc_std[NSCC] = { (caddr_t) 0};

/*
 * Definition of the driver for the auto-configuration program.
 */

struct	bus_device *scc_info[NSCC];

struct	bus_driver scc_driver =
        {scc_probe,
	 0,
	scc_attach,
	 0,
	 scc_std,
	"scc",
	scc_info,
	0,
	0,
	0
};

#define SCC_RR1_ERRS (SCC_RR1_FRAME_ERR|SCC_RR1_RX_OVERRUN|SCC_RR1_PARITY_ERR)
#define SCC_RR3_ALL (SCC_RR3_RX_IP_A|SCC_RR3_TX_IP_A|SCC_RR3_EXT_IP_A|\
                     SCC_RR3_RX_IP_B|SCC_RR3_TX_IP_B|SCC_RR3_EXT_IP_B)

//#define SCC_EXPERIMENTAL

#if	SCC_DMA_TRANSFERS
extern struct scc_dma_ops	scc_amic_ops, scc_db_ops;
#endif

DECL_FUNNEL(, scc_funnel)	/* funnel to serialize the SCC driver */
boolean_t scc_funnel_initted = FALSE;
#define SCC_FUNNEL		scc_funnel
#define SCC_FUNNEL_INITTED	scc_funnel_initted

static __inline__ void
scc_dma_setup_8530(int chan, struct scc_dma_ops *ops)
{
	scc_channel_t sc = &scc_softc[0].schan[chan];

	if (!ops)
		return;
	ops->scc_dma_start_rx(chan);
	sc->wr1 = SCC_WR1_DMA_MODE | SCC_WR1_DMA_RECV_DATA;
	scc_write_reg(sc, SCC_WR1, sc->wr1);
	sc->wr1 |= SCC_WR1_DMA_ENABLE;
	scc_write_reg(sc, SCC_WR1, sc->wr1);
	sc->wr1 |= SCC_WR1_RXI_SPECIAL_O;
	scc_write_reg(sc, SCC_WR1, sc->wr1);
	sc->wr1 &= ~SCC_WR1_TX_IE;
	scc_write_reg(sc, SCC_WR1, sc->wr1);
}

/*
 * Adapt/Probe/Attach functions
 */
boolean_t	scc_uses_modem_control = FALSE;/* patch this with adb */

/* This is called VERY early on in the init and therefore has to have
 * hardcoded addresses of the serial hardware control registers. The
 * serial line may be needed for console and debugging output before
 * anything else takes place
 */

void
initialize_serial()
{
	unsigned char dummy;
	int i, chan, bits;
	scc_softc_t scc = &scc_softc[0];
	static struct bus_device d;

	DECL_FUNNEL_VARS
	if (!SCC_FUNNEL_INITTED) {
		FUNNEL_INIT(&SCC_FUNNEL, master_processor);
		SCC_FUNNEL_INITTED = TRUE;
	}
	FUNNEL_ENTER(&SCC_FUNNEL);

	if (serial_initted) {
		FUNNEL_EXIT(&SCC_FUNNEL);
		return;
	}


	/* originally from darwin */
	// If this machine has PMU then turn on the serial ports.
	if (machine_has_pmu()) { /* darwin uses HasPMU() */
#if 0
	  volatile unsigned long *ohareFeatureCntl;

	  switch (powermac_info.class) {
	  case	POWERMAC_CLASS_PDM:
	  case	POWERMAC_CLASS_POWERBOOK:
	  case	POWERMAC_CLASS_PERFORMA:
	        // ohareFeatureCntl = (void *) (powermac_info.io_base_phys + 0x38);
		ohareFeatureCntl = 0;
	        break;

	  case	POWERMAC_CLASS_PCI:
	        ohareFeatureCntl = (void *) (powermac_info.io_base_phys + 0x38);
	        break;
	  }

	  /* activate serial ports */
	  if (ohareFeatureCntl)
	    {
	      *ohareFeatureCntl &= ~(1 << 24);
	      eieio();
	      *ohareFeatureCntl |= (1 << 17) | ( 1 << 22) | (1 << 23);
	      eieio();
	    }
#endif
	}

	memset ((void*) scc, 0, sizeof(*scc));
	scc->full_modem = TRUE;

	switch (powermac_info.class) {
	case	POWERMAC_CLASS_PDM:
	case	POWERMAC_CLASS_POWERBOOK:
		scc_std[0] = (caddr_t) PDM_SCC_BASE_PHYS;
		scc->schan[SCC_CHANNEL_A].regs = (scc_regmap_t)scc_std[0] + 2;
		scc->schan[SCC_CHANNEL_A].data = (scc_regmap_t)scc_std[0] + 6;
		scc->schan[SCC_CHANNEL_B].regs = (scc_regmap_t)scc_std[0];
		scc->schan[SCC_CHANNEL_B].data = (scc_regmap_t)scc_std[0] + 4;
		break;
	case	POWERMAC_CLASS_PERFORMA:
		scc_std[0] = (caddr_t) PERFORMA_SCC_BASE_PHYS;
		scc->schan[SCC_CHANNEL_A].regs = (scc_regmap_t)scc_std[0] + 2;
		scc->schan[SCC_CHANNEL_A].data = (scc_regmap_t)scc_std[0] + 6;
		scc->schan[SCC_CHANNEL_B].regs = (scc_regmap_t)scc_std[0];
		scc->schan[SCC_CHANNEL_B].data = (scc_regmap_t)scc_std[0] + 4;
		break;
	case	POWERMAC_CLASS_PCI:
	    {
		device_node_t *dev;

		if (!(dev = find_devices("scc")))
		{
		    dev = find_devices("escc-legacy");
		    if (!dev)
			dev = find_devices("escc");
		}

		if (!dev || !dev->n_addrs)
		{
		    /* No device or no addresses */
		    return;
		} else {
		    /* original scc, or escc-legacy, with legitimate
		       OF address properties */
		    scc_std[0] = (caddr_t) dev->addrs[0].address;
		    scc->schan[SCC_CHANNEL_A].regs = (scc_regmap_t)scc_std[0]
						     + 0x20;
		    scc->schan[SCC_CHANNEL_A].data = (scc_regmap_t)scc_std[0]
						     + 0x30;
		    scc->schan[SCC_CHANNEL_B].regs = (scc_regmap_t)scc_std[0];
		    scc->schan[SCC_CHANNEL_B].data = (scc_regmap_t)scc_std[0]
						     + 0x10;
		}
	    }
	    break;
	}

	simple_lock_init(&scc->schan[0].lock, ETAP_IO_TTY);
	simple_lock_init(&scc->schan[1].lock, ETAP_IO_TTY);

	/* reset register pointers on all channels */
	scc_read_reg_zero(&scc->schan[0], dummy);
	scc_read_reg_zero(&scc->schan[1], dummy);
	/* reset Z85C30 */
	scc_write_reg(&scc->schan[SCC_CHANNEL_A], 9, SCC_WR9_HW_RESET);
	scc_delay(100);
	for (chan = 0; chan < NSCC_LINE; chan++) {
	    if (chan == SCC_CHANNEL_A)
		scc_init_hw[0].val = 0x80;
	    for (i = 0; i < scc_init_hw_count; i++) {
		if (scc_init_hw[i].reg == 0xff) {
		    scc_delay(100);
		} else
		    scc_write_reg(&scc->schan[chan], scc_init_hw[i].reg,
				  scc_init_hw[i].val);
	    }
	}

	/* Call probe so we are ready very early for remote gdb and for serial
	   console output if appropriate.  */
	/* d.unit = 0; */
	if (scc_probe(0, (void *) &d)) {
	    for (i = 0; i < NSCC_LINE; i++) {
		scc->schan[i].wr5 = SCC_WR5_DTR | SCC_WR5_RTS;
		scc_param(scc_tty_for(i));
		/* Enable SCC interrupts */
		scc_write_reg(&scc->schan[i],  9, SCC_WR9_NV);
	    }
	}

	serial_initted = TRUE;
	FUNNEL_EXIT(&SCC_FUNNEL);
	return;
}

int
scc_probe(caddr_t  xxx, void *param)
{
	struct bus_device *ui = (struct bus_device *) param;
	device_node_t *dev;
	scc_softc_t     scc = &scc_softc[0];
	register int	val, i;
	register scc_regmap_t	regs;
	spl_t	s;
	DECL_FUNNEL_VARS

	if (!SCC_FUNNEL_INITTED) {
		FUNNEL_INIT(&SCC_FUNNEL, master_processor);
		SCC_FUNNEL_INITTED = TRUE;
		simple_lock_init(&scc->schan[0].lock, ETAP_IO_TTY);
		simple_lock_init(&scc->schan[1].lock, ETAP_IO_TTY);
	}
	FUNNEL_ENTER(&SCC_FUNNEL);

	/* Readjust the I/O address to handling 
	 * new memory mappings.
	 */

	if ((scc->probed_once)) {
		printf("SCC at %x (%x)", scc_std[0],
			POWERMAC_IO(scc_std[0]));
		printf("Default is %x (%x)\n\r", PCI_SCC_BASE_PHYS,
			POWERMAC_IO(PCI_SCC_BASE_PHYS));
	}

	regs = (scc_regmap_t)(scc_std[0] = POWERMAC_IO(scc_std[0]));
	if (!regs) {
		FUNNEL_EXIT(&SCC_FUNNEL);
		return 0;
	}

	if (scc->probed_once++) {
	    /* Second time in means called from system */
	    switch (powermac_info.class) {
		case	POWERMAC_CLASS_PDM:
#if	SCC_DMA_TRANSFERS
		    scc->dma_ops = &scc_amic_ops;
#endif
			/* fall through */
		case	POWERMAC_CLASS_POWERBOOK:
		case	POWERMAC_CLASS_PERFORMA:
		    scc->schan[SCC_CHANNEL_A].regs =
				(scc_regmap_t)scc_std[0] + 2;
		    scc->schan[SCC_CHANNEL_A].data =
				(scc_regmap_t)scc_std[0] + 6;
		    scc->schan[SCC_CHANNEL_B].regs =
				(scc_regmap_t)scc_std[0];
		    scc->schan[SCC_CHANNEL_B].data =
				(scc_regmap_t)scc_std[0] + 4;
		    pmac_register_int(PMAC_DEV_SCC, SPLTTY,
				      (void (*)(int, void *))scc_intr);
		    break;

		case	POWERMAC_CLASS_PCI:
		{
		    device_node_t *dev, *cha, *chb, *ch;

#if	SCC_DMA_TRANSFERS
//		    scc->dma_ops = &scc_db_ops;
#endif
		    dev = find_devices("scc");
		    if (!dev)
			dev = find_devices("escc-legacy");
		    if (!dev)
			dev = find_devices("escc");
		    if (!dev) {
			printf("Didn't find an escc\n");
			goto notfound;
		    }
		    cha = chb = 0;
		    for(ch = dev->child; ch; ch = ch->sibling) {
			if (cha && chb) {
			    printf("Only two channels are supported\n");
			    if (ch->name)
				printf("channel %s not attached\n", ch->name);
			    break;
			}
			if (ch->n_addrs != 3 || ch->n_intrs != 3) {
			    printf("escc device should have three addresses "
				   "and interrupt lines, but got only %d "
				   "address(es) and %d interrupt line(s)\n",
				   ch->n_addrs, ch->n_intrs);
			    continue;
			}
			if (cha)
			    chb = ch;
			else
			    cha = ch;
		    }
		    if (!cha) {
			/* No channels */
			printf("Didn't find any channel\n");
			goto notfound;
		    } else {
			scc->schan[SCC_CHANNEL_A].regs = (scc_regmap_t)
				io_map((unsigned)cha->addrs[0].address, 0x100);
			scc->schan[SCC_CHANNEL_A].data =
				scc->schan[SCC_CHANNEL_A].regs + 0x10;
			pmac_register_ofint(cha->intrs[0], SPLTTY,
					    (void (*)(int, void *)) scc_intr);
			printf("Found an Open Firmware [e]scc device\n");
			if (chb) {
			    scc->schan[SCC_CHANNEL_B].regs = (scc_regmap_t)
				io_map((unsigned)chb->addrs[0].address, 0x100);
			    scc->schan[SCC_CHANNEL_B].data =
				scc->schan[SCC_CHANNEL_B].regs + 0x10;
			    pmac_register_ofint(chb->intrs[0], SPLTTY,
						(void (*)(int,void *))scc_intr);
			    printf("Registering serial interrupts %d and %d\n",
				   cha->intrs[0], chb->intrs[0]);
			} else
			    printf("Registering serial interrupt %d\n",
				   cha->intrs[0]);
		    }
		    scc->schan[SCC_CHANNEL_A].node = cha;
		    scc->schan[SCC_CHANNEL_B].node = chb;
		    break;
		}
		default:
		    printf("unsupported class for serial code\n");
		    goto notfound;
	    }
	    FUNNEL_EXIT(&SCC_FUNNEL);
	    return 1;
notfound:
	    FUNNEL_EXIT(&SCC_FUNNEL);
	    return 0;
	}

	s = spltty();

	for (i = 0; i < NSCC_LINE; i++) {
		register struct tty	*tp;
		tp = scc_tty_for(i);
		simple_lock_init(&tp->t_lock, ETAP_IO_TTY);
		tp->t_addr = (char*)(0x80000000L + (i&1));
		/* Set default values.  These will be overridden on
		   open but are needed if the port will be used
		   independently of the Mach interfaces, e.g., for
		   gdb or for a serial console.  */
		tp->t_ispeed = DEFAULT_SPEED;
		tp->t_ospeed = DEFAULT_SPEED;
		tp->t_flags = DEFAULT_FLAGS;
		scc->schan[i].speed = -1;

		/* do min buffering */
		tp->t_state |= TS_MIN;

		tp->t_dev = scc_dev_no(i);
	}

	splx(s);

	FUNNEL_EXIT(&SCC_FUNNEL);
	return 1;
}

boolean_t scc_timer_started = FALSE;

void
scc_attach( register struct bus_device *ui )
{
	extern int tty_inq_size, tty_outq_size;
	int i;
	struct tty *tp;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

#if	SCC_DMA_TRANSFERS
	if (scc_softc[0].dma_ops) {
		/* DMA Serial can send a lot... ;-) */
		tty_inq_size = 16384;
		tty_outq_size = 16384;
		for (i = 0; i < NSCC_LINE; i++) {
			if (scc_softc[0].dma_ops->scc_dma_init(i))
				continue;
			scc_softc[0].dma_initted |= (1<<i);
		}
	}
#endif

	if (!scc_timer_started) {
		/* do all of them, before we call scc_scan() */
		/* harmless if done already */
		for (i = 0; i < NSCC_LINE; i++)  {
			tp = scc_tty_for(i);
			ttychars(tp);
			/* hack MEB 1/5/96 */
			tp->t_state |= TS_CARR_ON;
			scc_softc[0].modem[i] = 0;
		}

		scc_timer_started = TRUE;
		scc_cd_scan();
	}

	printf("\n sl0: ( alternate console )\n sl1:");

	FUNNEL_EXIT(&SCC_FUNNEL);
	return;
}

/*
 * Would you like to make a phone call ?
 */

void
scc_set_modem_control(scc, on)
	scc_softc_t      scc;
	boolean_t	on;
{
	assert(FUNNEL_IN_USE(&SCC_FUNNEL));
	scc->full_modem = on;
	/* user should do an scc_param() ifchanged */
	assert(FUNNEL_IN_USE(&SCC_FUNNEL));
}

/*
 * Polled I/O (debugger)
 */

int
scc_pollc(int unit, boolean_t on)
{
	scc_softc_t		scc;

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));

	scc = &scc_softc[unit];
	if (on) {
		scc->polling_mode++;
	} else
		scc->polling_mode--;

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));
	return 0;
}

/*
 * Interrupt routine
 */

static __inline__ enum scc_error scc_to_err(scc_channel_t sc)
{
	int stat;
	enum scc_error err = SCC_ERR_NONE;

	scc_read_reg(sc, SCC_RR1, stat);

	if (stat & SCC_RR1_FRAME_ERR)
		err = SCC_ERR_BREAK;
	else if (stat & SCC_RR1_PARITY_ERR)
		err = SCC_ERR_PARITY;
	else if (stat & SCC_RR1_RX_OVERRUN)
		err = SCC_ERR_OVERRUN;
	else
		goto out;

	/* Reset error status */
	scc_write_reg_zero(sc, SCC_RESET_ERROR);
out:
	return err;
}

static __inline__ void scc_restart(struct tty *tp)
{
	int			cc;

	if (tp->t_state & TS_TIMEOUT) 
		return;
	cc = tp->t_outq.c_cc;
	if (cc <= tp->t_lowater)
		tt_write_wakeup(tp);
	if (cc <= 0)
		return;
	tp->t_state |= TS_BUSY;
}

/*
 * Transmission of a character is complete.
 * Return the next character or -1 if none.
 */
static __inline__ int scc_simple_tint(struct tty *tp, boolean_t all_sent)
{
	if ((tp->t_addr == 0) || (tp->t_state & TS_TTSTOP))
		return -1;

	if (all_sent) {
		tp->t_state &= ~TS_BUSY;
		if (tp->t_state & TS_FLUSH)
			tp->t_state &= ~TS_FLUSH;

		scc_restart(tp);
	}

	if (tp->t_outq.c_cc == 0 || (tp->t_state & TS_BUSY) == 0)
		return -1;

	return getc(&tp->t_outq);
}

static __inline__ void do_send_chars(scc_channel_t sc, int chan)
{
	int c, stat;

	scc_read_reg_zero(sc, stat);
	if ((stat & SCC_RR0_TX_EMPTY) == 0) {
		scc_write_reg_zero(sc, SCC_RESET_TX_IP);
		return;
	}
	c = scc_simple_tint(scc_tty_for(chan), FALSE);
	if (c == -1) {
		c = scc_simple_tint(scc_tty_for(chan), TRUE);
		if (c == -1) {
			/* No more characters or need to stop */
			scc_write_reg_zero(sc, SCC_RESET_TX_IP);
			return;
		}
		/* Found more to do & transmitter has been re-enabled */
	}
	scc_write_data(sc, c);
}

static __inline__ void do_receive_chars(scc_channel_t sc, int chan,
				    struct ppc_saved_state *ssp)
{
	int c, stat0;
	enum scc_error err;
	struct tty *tp = scc_tty_for(chan);

	scc_read_reg_zero(sc, stat0);
	while (stat0 & SCC_RR0_RX_AVAIL) {
#if MACH_KGDB
		if (chan == KGDB_PORT) {
			/* 11/10/95 MEB
			 * Drop into the debugger.. scc_getc() will
			 * pick up the character
			 */
			call_kgdb_with_ctx(T_INTERRUPT, 0, ssp);
			return;
		}
#endif
		err = scc_to_err(sc);
		scc_read_data(sc, c);
		simple_lock(&tp->t_lock);
		scc_input(scc_dev_no(chan), c, err);
		simple_unlock(&tp->t_lock);
		scc_read_reg_zero(sc, stat0);
	}
	scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
}

static __inline__ void do_ext_status(scc_channel_t sc, int chan)
{
	int status;
	struct tty *tp = scc_tty_for(chan);

#ifndef SCC_EXPERIMENTAL
	scc_read_reg_zero(sc, status);
#endif
	simple_lock(&tp->t_lock);
#ifndef SCC_EXPERIMENTAL
	if (status & SCC_RR0_BREAK)
		scc_input(scc_dev_no(chan), 0, SCC_ERR_BREAK);
#endif
	scc_update_modem(&scc_tty[chan]);
	simple_unlock(&tp->t_lock);

	scc_write_reg_zero(sc, SCC_RESET_EXT_IP);
}

static __inline__ void do_receive_special(scc_channel_t sc, int chan)
{
	scc_softc_t scc = &scc_softc[0];
	enum scc_error err = scc_to_err(sc);
	struct tty *tp = scc_tty_for(chan);

	/* XXX this is clearly wrong.  No error is reported with DMA. */
#if SCC_DMA_TRANSFERS
	if (scc->dma_initted & (chan<<1)) {
		scc->dma_ops->scc_dma_reset_rx(chan);
		scc->dma_ops->scc_dma_start_rx(chan);
	} else
#endif
	if (err != SCC_ERR_NONE) {
		simple_lock(&tp->t_lock);
		scc_input(scc_dev_no(chan), 0, err);
		simple_unlock(&tp->t_lock);
	}
}

void
scc_intr(int device, struct ppc_saved_state *ssp)
{
	int			chan;
	scc_softc_t		scc = &scc_softc[0];
	scc_channel_t		sc;
	register int		status;
	unsigned char		dummy;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	/* reset register pointer on all channels (should be unnecessary) */
	scc_read_reg_zero(&scc->schan[0], dummy);
	scc_read_reg_zero(&scc->schan[1], dummy);
	/* figure out which interrupt is pending */
	scc_read_reg(&scc->schan[SCC_CHANNEL_A], SCC_RR3, status);
	if (status & 0x3f) {
again:
		if (status & 0x07) {
			chan = SCC_CHANNEL_B;
		} else {
			chan = SCC_CHANNEL_A;
			status = (status >> 3) & 0x07;
		}
		sc = &scc->schan[chan];

		if (status & SCC_RR3_EXT_IP_B)
			do_ext_status(sc, chan);
		if (status & SCC_RR3_TX_IP_B) {
#if	SCC_DMA_TRANSFERS
			if (scc->dma_initted & (1<<chan))
				/* should never reach here */
				assert(0);
			else
#endif
			do_send_chars(sc, chan);
		}
		if (status & SCC_RR3_RX_IP_B) {
#if	SCC_DMA_TRANSFERS
			if (scc->dma_initted & (1<<chan))
				do_receive_special(sc, chan);
			else
#endif
			do_receive_chars(sc, chan, ssp);
		}
		if (status & 0x38) {
			status &= ~0x07;
			goto again;
		}
		scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
	}
	FUNNEL_EXIT(&SCC_FUNNEL);
	return;
}

/*
 * Start output on a line
 */

void
scc_start(tp)
	struct tty *tp;
{
	spl_t			s;
	int			cc;
	int			chan = scc_chan(tp->t_dev), temp;
	scc_softc_t		scc = &scc_softc[0];
	scc_channel_t		sc = &scc->schan[chan];
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	s = spltty();

#if	SCC_DMA_TRANSFERS
	if (scc->dma_initted & (1<<chan)) {
		/* Start up the DMA channel if it was paused */
		if ((tp->t_state & TS_TTSTOP) == 0 && sc->dma_flags & SCC_FLAGS_DMA_PAUSED) {
			scc->dma_ops->scc_dma_continue_tx(chan);
			goto out;
		}
	}
#endif

	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) 
		goto out;

#if	SCC_DMA_TRANSFERS
	if (scc->dma_initted & (1<<chan)) {
 	 	/* Don't worry about low water marks...
	  	 * The DMA operation should be able to pull off most
	  	 * if not all of the TTY output queue
	  	*/
	
		tt_write_wakeup(tp);

		if (tp->t_outq.c_cc <= 0) 
			goto out;

		tp->t_state |= TS_BUSY;

		scc->dma_ops->scc_dma_start_tx(chan, tp);
	} else 
#endif
	{
		cc = tp->t_outq.c_cc;
		if (cc <= tp->t_lowater) {
			tt_write_wakeup(tp);
		}
		if (cc <= 0)
			goto out;
		tp->t_state |= TS_BUSY;

		if (!(sc->wr1 & SCC_WR1_TX_IE)) {
			temp = sc->wr1 | SCC_WR1_TX_IE;
			scc_write_reg(sc, SCC_WR1, temp);
			sc->wr1 = temp;
		}
	
		/* but we need a first char out or no cookie */
		scc_read_reg_zero(sc, temp);
		if (temp & SCC_RR0_TX_EMPTY)
		{
			register char	c;
	
			c = getc(&tp->t_outq);
			scc_write_data(sc, c);
		}
	}
out:
	splx(s);
	FUNNEL_EXIT(&SCC_FUNNEL);
}

#define	u_min(a,b)	((a) < (b) ? (a) : (b))

/*
 * Get a char from a specific SCC line
 * [this is only used for console&screen purposes]
 * must be splhigh since it may be called from another routine
 */

int
scc_getc(int unit, int line, boolean_t wait, boolean_t raw)
{
	scc_channel_t	sc = &scc_softc[0].schan[line];
	unsigned char   c, value;
	int             rcvalue, from_line;
	spl_t		s = spltty();
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	/*
	 * wait till something available
	 *
	 */
again:
	rcvalue = 0;
	while (1) {
		scc_read_reg_zero(sc, value);

		if (value & SCC_RR0_RX_AVAIL)
			break;

		if (!wait) {
			splx(s);
			FUNNEL_EXIT(&SCC_FUNNEL);
			return -1;
		}
	}

	/*
	 * if nothing found return -1
	 */

	scc_read_reg(sc, SCC_RR1, value);
	scc_read_data(sc, c);

	/* osfmach3 */
#if	MACH_KDB
	if (console_is_serial() &&
	    c == ('_' & 0x1f)) {
		/* Drop into the debugger */
		Debugger("Serial Line Request");
		scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
		if (wait) {
			goto again;
		}
		splx(s);
		FUNNEL_EXIT(&SCC_FUNNEL);
		return -1;
	}
#endif	/* MACH_KDB */

	/*
	 * bad chars not ok
	 */
	if (value&(SCC_RR1_ERRS)) {
		scc_write_reg_zero(sc, SCC_RESET_ERROR);

		if (wait) {
			scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
			goto again;
		}
	}

	scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
	splx(s);

	FUNNEL_EXIT(&SCC_FUNNEL);
	return c;
}

/*
 * Put a char on a specific SCC line
 * use splhigh since we might be doing a printf in high spl'd code
 */

int
scc_putc(int unit, int line, int c)
{
	scc_channel_t	sc = &scc_softc[0].schan[line];
	spl_t            s = spltty();
	unsigned char	 value;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	do {
		scc_read_reg_zero(sc, value);
		if (value & SCC_RR0_TX_EMPTY)
			break;
		scc_delay(100);
	} while (1);

	scc_write_data(sc, c);
/* wait for it to swallow the char ? */

	do {
		scc_read_reg_zero(sc, value);
		if (value & SCC_RR0_TX_EMPTY)
			break;
	} while (1);
	scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
	splx(s);

	FUNNEL_EXIT(&SCC_FUNNEL);
	return 0;
}

int
scc_param(struct tty *tp)
{
	unsigned char	value;
	unsigned short	speed_value;
	int		bits, chan;
	spl_t		s;
	scc_channel_t	sc;
	scc_softc_t	scc;

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));

	chan = scc_chan(tp->t_dev);
	scc = &scc_softc[0];
	sc = &scc->schan[chan];

	/* Do a quick check to see if the hardware needs to change */
	if ((sc->flags & (TF_ODDP|TF_EVENP)) == (tp->t_flags & (TF_ODDP|TF_EVENP))
	    && sc->speed == tp->t_ispeed) {
		assert(FUNNEL_IN_USE(&SCC_FUNNEL));
		return 0;
	}

	sc->flags = tp->t_flags;
	sc->speed = tp->t_ispeed;

	s = spltty();

	if (tp->t_ispeed == 0) {
		sc->wr5 &= ~SCC_WR5_DTR;
		scc_write_reg(sc, SCC_WR5, sc->wr5);
		splx(s);

		assert(FUNNEL_IN_USE(&SCC_FUNNEL));
		return 0;
	}
	

#if	SCC_DMA_TRANSFERS
	if (scc->dma_initted & (1<<chan)) 
		scc->dma_ops->scc_dma_reset_rx(chan);
#endif

	value = SCC_WR4_1_STOP;

	/* 
	 * For 115K the clocking divide changes to 64.. to 230K will
	 * start at the normal clock divide 16.
	 *
	 * However, both speeds will pull from a different clocking
	 * source
	 */

	if (tp->t_ispeed == 115200)
		value |= SCC_WR4_CLK_x32;
	else	
		value |= SCC_WR4_CLK_x16 ;

	/* .. and parity */
	if ((tp->t_flags & (TF_ODDP | TF_EVENP)) == TF_EVENP)
		value |= (SCC_WR4_EVEN_PARITY |  SCC_WR4_PARITY_ENABLE);
	else if ((tp->t_flags & (TF_ODDP | TF_EVENP)) == TF_ODDP)
		value |= SCC_WR4_PARITY_ENABLE;

	/* set it now, remember it must be first after reset */
	sc->wr4 = value;

	/* Program Parity, and Stop bits */
	scc_write_reg(sc, SCC_WR4, sc->wr4);

	/* Setup for 8 bits */
	scc_write_reg(sc, SCC_WR3, SCC_WR3_RX_8_BITS);

	// Set DTR, RTS, and transmitter bits/character.
	sc->wr5 = SCC_WR5_TX_8_BITS | SCC_WR5_RTS | SCC_WR5_DTR;

	scc_write_reg(sc, SCC_WR5, sc->wr5);
	scc_write_reg(sc, SCC_WR14, 0);	/* Disable baud rate */
#ifdef SCC_EXPERIMENTAL
	scc_read_reg(sc, SCC_RR15, bits);
	bits |= SCC_WR15_BREAK_IE | SCC_WR15_CTS_IE | SCC_WR15_DCD_IE;
	scc_write_reg(sc, SCC_WR15, bits);
#endif
	/* Setup baud rate 57.6Kbps, 115K, 230K should all yeild
	 * a converted baud rate of zero
	 */
	speed_value = convert_baud_rate(tp->t_ispeed);

	if (speed_value == 0xffff)
		speed_value = 0;

	scc_set_timing_base(sc, speed_value);

	if (tp->t_ispeed == 115200 || tp->t_ispeed == 230400) {
		/* Special case here.. change the clock source*/
		scc_write_reg(sc, SCC_WR11, 0);
		/* Baud rate generator is disabled.. */
	} else {
		scc_write_reg(sc, SCC_WR11, SCC_WR11_RCLK_BAUDR|SCC_WR11_XTLK_BAUDR);
		/* Enable the baud rate generator */
		scc_write_reg(sc, SCC_WR14, SCC_WR14_BAUDR_ENABLE);
	}

	scc_write_reg(sc, SCC_WR3, SCC_WR3_RX_8_BITS|SCC_WR3_RX_ENABLE);

	sc->wr1 = SCC_WR1_RXI_FIRST_CHAR | SCC_WR1_EXT_IE;
	scc_write_reg(sc, SCC_WR1, sc->wr1);
	scc_write_reg_zero(sc, SCC_IE_NEXT_CHAR);

	/* Clear out any pending external or status interrupts */
	scc_write_reg_zero(sc, SCC_RESET_EXT_IP);

	/* Enable SCC interrupts */
	scc_write_reg(sc, SCC_WR9, SCC_WR9_MASTER_IE|SCC_WR9_NV);

	/* Get the current status and save */
	scc_read_reg_zero(sc, bits);
	sc->rr0 = bits;

#if	SCC_DMA_TRANSFERS
	if (scc->dma_initted & (1<<chan))  {
		scc_dma_setup_8530(chan, scc->dma_ops);
	} else
#endif
	{
		sc->wr1 = SCC_WR1_RXI_ALL_CHAR | SCC_WR1_EXT_IE;
		scc_write_reg(sc, SCC_WR1, sc->wr1);
		scc_write_reg_zero(sc, SCC_IE_NEXT_CHAR);
	}

	sc->wr5 |= SCC_WR5_TX_ENABLE;
	scc_write_reg(sc, SCC_WR5, sc->wr5);

	splx(s);

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));
	return 0;

}

void
scc_update_modem(struct tty *tp)
{
	int chan = scc_chan(tp->t_dev);
	scc_softc_t	scc = &scc_softc[0];
	scc_channel_t sc = &scc->schan[chan];
	unsigned char rr0, old_modem;

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));

	old_modem = scc->modem[chan];
	scc->modem[chan] &= ~(TM_CTS|TM_CAR|TM_RNG|TM_DSR);

	scc_read_reg_zero(sc, rr0);

#ifdef SCC_EXPERIMENTAL
	if (rr0 & SCC_RR0_BREAK)
		scc_input(tp->t_dev, 0, SCC_ERR_BREAK);
	scc->modem[chan] |= TM_DSR;
	if(!(rr0 & SCC_RR0_CTS))
		scc->modem[chan] |= TM_CTS;
	if (((rr0 ^ sc->rr0) & SCC_RR0_CTS) && (cpu_number() == master_cpu))
		tty_cts(tp, !(rr0 & SCC_RR0_CTS));
	sc->rr0 = rr0;
#else
	scc->modem[chan] |= TM_DSR|TM_CTS;
#endif
	if (rr0 & SCC_RR0_DCD) {
		scc->modem[chan] |= TM_CAR;
		if ((old_modem & TM_CAR) == 0) {
			/*printf("{DTR-ON %x/%x}", rr0, old_modem);*/
			/*
			 * The trick here is that
			 * the device_open does not hang
			 * waiting for DCD, but a message
			 * is sent to the process 
			 */

			if ((tp->t_state & (TS_ISOPEN|TS_WOPEN))
			&& tp->t_flags & TF_OUT_OF_BAND) {
				/*printf("{NOTIFY}");*/
				tp->t_outofband = TOOB_CARRIER;
				tp->t_outofbandarg = TRUE;
				tty_queue_completion(&tp->t_delayed_read);
			}
		}
	} else if (old_modem & TM_CAR) {
		if (tp->t_state & (TS_ISOPEN|TS_WOPEN)) {
			if (++scc->dcd_timer[chan] >= DCD_TIMEOUT) {
				/*printf("{DTR-OFF %x/%x}", rr0, old_modem);*/
				if (tp->t_flags & TF_OUT_OF_BAND) {
					tp->t_outofband = TOOB_CARRIER;
					tp->t_outofbandarg = FALSE;
					tty_queue_completion(&tp->t_delayed_read);
				} else
					ttymodem(tp, FALSE);
			}
		}
	}

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));
}

/*
 * Modem control functions
 */
int
scc_mctl(struct tty* tty, int bits, int how)
{
	register dev_t dev = tty->t_dev;
	int sccline;
	register int tcr, msr, brk, n_tcr, n_brk;
	int b;
	scc_softc_t      scc;
	int wr5;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	sccline = scc_chan(dev);

	if (bits == TM_HUP) {	/* close line (internal) */
	    bits = TM_DTR | TM_RTS;
	    how = DMBIC;
	}

	scc = &scc_softc[0];
 	wr5 = scc->schan[sccline].wr5;

	if (how == DMGET) {
	    scc_update_modem(tty);
	    FUNNEL_EXIT(&SCC_FUNNEL);
	    return scc->modem[sccline];
	}

	switch (how) {
	case DMSET:
	    b = bits; break;
	case DMBIS:
	    b = scc->modem[sccline] | bits; break;
	case DMBIC:
	    b = scc->modem[sccline] & ~bits; break;
	default:
	    FUNNEL_EXIT(&SCC_FUNNEL);
	    return 0;
	}

	if (scc->modem[sccline] == b) {
	    FUNNEL_EXIT(&SCC_FUNNEL);
	    return b;
	}

	scc->modem[sccline] = b;

	if (bits & TM_BRK) {
	    ttydrain(tty);
	    scc_waitforempty(tty);
	}

	wr5 &= ~(SCC_WR5_SEND_BREAK|SCC_WR5_DTR);

	if (b & TM_BRK)
		wr5 |= SCC_WR5_SEND_BREAK;

	if (b & TM_DTR)
		wr5 |= SCC_WR5_DTR;

	wr5 |= SCC_WR5_RTS;

	scc_write_reg(&scc->schan[sccline], SCC_WR5, wr5);
	scc->schan[sccline].wr5 = wr5;

	FUNNEL_EXIT(&SCC_FUNNEL);
	return scc->modem[sccline];
}

/*
 * Periodically look at the CD signals:
 * they do generate interrupts but we
 * must fake them on channel A.  We might
 * also fake them on channel B.
 */

int
scc_cd_scan(void)
{
	spl_t s = spltty();
	scc_softc_t	scc;
	int		j;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	scc = &scc_softc[0];
	for (j = 0; j < NSCC_LINE; j++) {
		if (scc_tty[j].t_state & (TS_ISOPEN|TS_WOPEN)) {
			simple_lock(&scc_tty[j].t_lock);
			scc_update_modem(&scc_tty[j]);
			simple_unlock(&scc_tty[j].t_lock);
		}
	}
	splx(s);

	timeout((timeout_fcn_t)scc_cd_scan, (void *)0, hz/4);

	FUNNEL_EXIT(&SCC_FUNNEL);
	return 0;
}

#if MACH_KGDB
void no_spl_scc_putc(int chan, char c)
{
	scc_channel_t	sc = &scc_softc[0].schan[chan];
	register unsigned char	value;

	if (!serial_initted)
		initialize_serial();

	do {
		scc_read_reg_zero(sc, value);
		if (value & SCC_RR0_TX_EMPTY)
			break;
		scc_delay(100);
	} while (1);

	scc_write_data(sc, c);
/* wait for it to swallow the char ? */

	do {
		scc_read_reg_zero(sc, value);
		if (value & SCC_RR0_TX_EMPTY)
			break;
	} while (1);
	scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);

	if (c == '\n')
		no_spl_scc_putc(chan, '\r');
 
}

#define	SCC_KGDB_BUFFER	15

int no_spl_scc_getc(int chan, boolean_t timeout)
{
	scc_channel_t	sc = &scc_softc[0].schan[chan];
	unsigned char   c, value, i;
	int             rcvalue, from_line;
	int	timeremaining = timeout ? 10000000 : 0;	/* XXX */
	static unsigned char	buffer[2][SCC_KGDB_BUFFER];
	static int		bufcnt[2], bufidx[2];

	/* This should be rewritten to produce a constant timeout
	   regardless of the processor speed.  */

	if (!serial_initted)
		initialize_serial();

get_char:
	if (bufcnt[chan]) {
		bufcnt[chan] --;
		return ((unsigned int) buffer[chan][bufidx[chan]++]);
	}

	/*
	 * wait till something available
	 *
	 */
	bufidx[chan] = 0;

	for (i = 0; i < SCC_KGDB_BUFFER; i++) {
		rcvalue = 0;

		while (1) {
			scc_read_reg_zero(sc, value);
			if (value & SCC_RR0_RX_AVAIL)
				break;
			if (timeremaining && !--timeremaining) {
				if (i)
					goto get_char;
				else
					return KGDB_GETC_TIMEOUT;
			}
		}

		scc_read_reg(sc, SCC_RR1, value);
		scc_read_data(sc, c);
		buffer[chan][bufcnt[chan]] = c;
		bufcnt[chan]++;

		/*
	 	 * bad chars not ok
	 	 */


		if (value&SCC_RR1_ERRS) {
			scc_write_reg_zero(sc, SCC_RESET_ERROR);
			scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
			bufcnt[chan] = 0;
			return KGDB_GETC_BAD_CHAR;
		}

		scc_write_reg_zero(sc, SCC_RESET_HIGHEST_IUS);
		
		for (timeremaining = 0; timeremaining < 1000; timeremaining++) {
			scc_read_reg_zero(sc, value);

			if (value & SCC_RR0_RX_AVAIL)
				continue;
		}

		if (timeout == FALSE)
			break;

	}


	goto get_char;
}
#endif	/* MACH_KGDB */

/*
 * Open routine
 */

io_return_t
scc_open(
	dev_t		dev,
	dev_mode_t	flag,
	io_req_t	ior)
{
	register struct tty	*tp;
	spl_t			s;
	scc_softc_t		scc;
	int			chan;
	int			forcedcarrier;
	io_return_t		result;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	if (dev >= NSCC * NSCC_LINE) {
		FUNNEL_EXIT(&SCC_FUNNEL);
		return D_NO_SUCH_DEVICE;
	}

	chan = scc_chan(dev);
	tp = scc_tty_for(chan);
	scc = &scc_softc[0];

	/* But was it there at probe time */
	if (tp->t_addr == 0) {
		FUNNEL_EXIT(&SCC_FUNNEL);
		return D_NO_SUCH_DEVICE;
	}

	s = spltty();
	simple_lock(&tp->t_lock);

	if (!(tp->t_state & (TS_ISOPEN|TS_WOPEN))) {
		tp->t_dev	= dev;
		tp->t_start	= scc_start;
		tp->t_stop	= scc_stop;
		tp->t_mctl	= scc_mctl;
		tp->t_getstat	= scc_get_status;
		tp->t_setstat	= scc_set_status;
		scc->modem[chan] = 0;	/* No assumptions on things.. */
		if (tp->t_ispeed == 0) {
			tp->t_ispeed = DEFAULT_SPEED;
			tp->t_ospeed = DEFAULT_SPEED;
			tp->t_flags = DEFAULT_FLAGS;
		}

		scc->schan[chan].speed = -1;	/* Force reset */
		scc->schan[chan].wr5 |= SCC_WR5_DTR;
		scc_param(tp);
	}

	scc_update_modem(tp);

	tp->t_state |= TS_CARR_ON;	/* Always.. */

	simple_unlock(&tp->t_lock);
	splx(s);
	result = char_open(dev, tp, flag, ior);

	if (tp->t_flags & CRTSCTS) {
		simple_lock(&tp->t_lock);
		if (!(scc->modem[chan] & TM_CTS)) 
			tp->t_state |= TS_TTSTOP;
		simple_unlock(&tp->t_lock);
	}

	FUNNEL_EXIT(&SCC_FUNNEL);
	return result;
}

/*
 * Close routine
 */
void
scc_close(
	dev_t	dev)
{
	register struct tty	*tp;
	spl_t			s;
	scc_softc_t		scc = &scc_softc[0];
	int			chan = scc_chan(dev);
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	tp = scc_tty_for(chan);
	s = spltty();
	simple_lock(&tp->t_lock);

	ttstart(tp);
	ttydrain(tp);
	scc_waitforempty(tp);

	/* Disable Receiver.. */
	scc_write_reg(&scc->schan[chan], SCC_WR3, 0);
#if SCC_DMA_TRANSFERS
	if (scc->dma_initted & (chan <<1))
		scc->dma_ops->scc_dma_reset_rx(chan);
#endif

	ttyclose(tp);
	if (tp->t_state & TS_HUPCLS) {
		scc->schan[chan].wr5 &= ~(SCC_WR5_DTR);
		scc_write_reg(&scc->schan[chan], SCC_WR5, scc->schan[chan].wr5);
		scc->modem[chan] &= ~(TM_DTR|TM_RTS);
	}
	tp->t_state &= ~TS_ISOPEN;
#if 0
	if (scc_carrier[chan] == 2)  /* turn off temporary true carrier */
		scc_carrier[chan] = 0;
#endif
	simple_unlock(&tp->t_lock);
	splx(s);

	FUNNEL_EXIT(&SCC_FUNNEL);
}

io_return_t
scc_read(
	dev_t		dev,
	io_req_t	ior)
{
	io_return_t	retval;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);
	retval = char_read(scc_tty_for(scc_chan(dev)), ior);
	FUNNEL_EXIT(&SCC_FUNNEL);

	return retval;
}

io_return_t
scc_write(
	dev_t		dev,
	io_req_t	ior)
{
	io_return_t	retval;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);
	retval = char_write(scc_tty_for(scc_chan(dev)), ior);
	FUNNEL_EXIT(&SCC_FUNNEL);

	return retval;
}

/*
 * Stop output on a line.
 */
void
scc_stop(
	struct tty	*tp,
	int		flags)
{
	int chan;
	scc_softc_t scc;
	scc_channel_t sc;
	spl_t s;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	chan = scc_chan(tp->t_dev);
	scc = &scc_softc[0];
	sc = &scc->schan[chan];

	s = spltty();

	if (tp->t_state & TS_BUSY) {
#if SCC_DMA_TRANSFERS
		if ((scc->dma_initted & (chan <<1)) &&
		   (sc->dma_flags & SCC_FLAGS_DMA_TX_BUSY)) {
			scc->dma_ops->scc_dma_pause_tx(chan);
		} else
#endif
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
	}

	splx(s);
	/*printf("{STOP %x}", flags);*/

	FUNNEL_EXIT(&SCC_FUNNEL);
}

/*
 * Abnormal close
 */
boolean_t
scc_portdeath(
	dev_t		dev,
	ipc_port_t	port)
{
	boolean_t	retval;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);
	retval = tty_portdeath(scc_tty_for(scc_chan(dev)), port);
	FUNNEL_EXIT(&SCC_FUNNEL);

	return retval;
}

/*
 * Get/Set status rotuines
 */
io_return_t
scc_get_status(
	dev_t			dev,
	dev_flavor_t		flavor,
	dev_status_t		data,
	mach_msg_type_number_t	*status_count)
{
	register struct tty *tp;
	io_return_t	retval;
	int		chan = scc_chan(dev);
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	tp = scc_tty_for(chan);
	switch (flavor) {
	case TTY_MODEM:
		simple_lock(&tp->t_lock);
		scc_update_modem(tp);
		simple_unlock(&tp->t_lock);
		*data = scc_softc[0].modem[chan];
		*status_count = 1;
		FUNNEL_EXIT(&SCC_FUNNEL);
		return (D_SUCCESS);
	default:
		retval = tty_get_status(tp, flavor, data, status_count);
		FUNNEL_EXIT(&SCC_FUNNEL);
		return retval;
	}
}

io_return_t
scc_set_status(
	dev_t			dev,
	dev_flavor_t		flavor,
	dev_status_t		data,
	mach_msg_type_number_t	status_count)
{
	register struct tty *tp;
	spl_t s;
	io_return_t result = D_SUCCESS;
	scc_softc_t scc;
	int chan;
	DECL_FUNNEL_VARS

	FUNNEL_ENTER(&SCC_FUNNEL);

	scc = &scc_softc[0];
	chan = scc_chan(dev);

	tp = scc_tty_for(scc_chan(dev));
	s = spltty();
	simple_lock(&tp->t_lock);

	switch (flavor) {
	case TTY_MODEM:
		(void) scc_mctl(tp, *data, DMSET);
		break;

	case TTY_NMODEM:
#if 0
//#ifdef SCC_EXPERIMENTAL
		switch (*data) {
			case 0:   /* turn on soft carrier */
			case 1:   /* turn off soft carrier permanently */
			case 2:   /* turn off soft carrier temporarily */
				scc_carrier[chan] = *data;
				break;
			default:  /* ignore */
				break;
		}
#endif
		break;

	case TTY_SET_BREAK:
		(void) scc_mctl(tp, TM_BRK, DMBIS);
		break;

	case TTY_CLEAR_BREAK:
		(void) scc_mctl(tp, TM_BRK, DMBIC);
		break;

	default:
		simple_unlock(&tp->t_lock);
		splx(s);
		result = tty_set_status(tp, flavor, data, status_count);
		s = spltty();
		simple_lock(&tp->t_lock);
		if (result == D_SUCCESS &&
		    (flavor== TTY_STATUS_NEW || flavor == TTY_STATUS_COMPAT)) {
			result = scc_param(tp);

			if (tp->t_flags & CRTSCTS) {
				if (scc->modem[chan] & TM_CTS) {
					tp->t_state &= ~TS_TTSTOP;
					ttstart(tp);
				} else
					tp->t_state |= TS_TTSTOP;
			}
		}
		break;
	}

	simple_unlock(&tp->t_lock);
	splx(s);

	FUNNEL_EXIT(&SCC_FUNNEL);
	return result;
}

void
scc_waitforempty(struct tty *tp)
{
	int chan = scc_chan(tp->t_dev);
	scc_softc_t scc = &scc_softc[0];
	int rr0;

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));

	while (1) {
		scc_read_reg_zero(&scc->schan[chan], rr0);
		if (rr0 & SCC_RR0_TX_EMPTY)
			break;
		assert_wait(0, TRUE);
		thread_set_timeout(1);
		simple_unlock(&tp->t_lock);
		thread_block((void (*)(void)) 0);
		reset_timeout_check(&current_thread()->timer);
		simple_lock(&tp->t_lock);
	}

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));
}

/*
 * Send along a character on a tty.  If we were waiting for
 * this char to complete the open procedure do so; check
 * for errors; if all is well proceed to ttyinput().
 */

void
scc_input(dev_t dev, int c, enum scc_error err)
{
	register struct tty *tp;

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));

	tp = scc_tty_for(scc_chan(dev));
	if ((tp->t_state & TS_ISOPEN) == 0) {
		if (tp->t_state & TS_INIT)
			tt_open_wakeup(tp);
		assert(FUNNEL_IN_USE(&SCC_FUNNEL));
		return;
	}
	switch (err) {
	case SCC_ERR_NONE:
		ttyinput(c, tp);
		break;
	case SCC_ERR_OVERRUN:
		/*log(LOG_WARNING, "sl%d: silo overflow\n", dev);*/
		/* Currently the Mach interface doesn't define an out-of-band
		   event that we could use to signal this error to the user
		   task that has this device open.  */
		break;
	case SCC_ERR_PARITY:
		ttyinputbadparity(c, tp);
		break;
	case SCC_ERR_BREAK:
		ttybreak(c, tp);
		break;
	}

	assert(FUNNEL_IN_USE(&SCC_FUNNEL));
}

/* modem port is 1, printer port is 0 */
#define LINE	1

#if got_console_now
int
kmtrygetc()
{
	return scc_getc(0 /* ignored */, LINE, 0 /*no_wait*/, 0);
}

int
cngetc()
{
	return scc_getc(0 /* ignored */, LINE, 1 /*wait*/, 0);
}

int
cnputc(char c)
{
	int a;

	a= scc_putc(0 /* ignored */, LINE, c);
	if (c == '\n')
		a = cnputc('\r');
	return a;
}
#endif /* got_console_now */
#endif	/* NSCC > 0 */
