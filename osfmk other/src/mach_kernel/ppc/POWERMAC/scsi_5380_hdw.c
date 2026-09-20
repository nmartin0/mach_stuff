#define PSC_BASE_PHYS 0x00000001
#define PSC_REGISTER_BASE 0
#define PSC_REGISTER_OFFSET 0

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
 * All of the stuff below was used for the framework, i.e. function
 * names, chunks of stuff.  It's really a hodge-podge, but....
 */

/*
 * MkLinux
 */

/*	$NetBSD: asc.c,v 1.18 1996/03/18 01:39:47 jonathan Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)asc.c	8.3 (Berkeley) 7/3/94
 */

/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 * CMU HISTORY
 * Revision 2.5  91/02/05  17:45:07  mrt
 * 	Added author notices
 * 	[91/02/04  11:18:43  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:17:20  mrt]
 * 
 * Revision 2.4  91/01/08  15:48:24  rpd
 * 	Added continuation argument to thread_block.
 * 	[90/12/27            rpd]
 * 
 * Revision 2.3  90/12/05  23:34:48  af
 * 	Recovered from pmax merge.. and from the destruction of a disk.
 * 	[90/12/03  23:40:40  af]
 * 
 * Revision 2.1.1.1  90/11/01  03:39:09  af
 * 	Created, from the DEC specs:
 * 	"PMAZ-AA TURBOchannel SCSI Module Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990.
 * 	And from the NCR data sheets
 * 	"NCR 53C94, 53C95, 53C96 Advances SCSI Controller"
 * 	[90/09/03            af]
 */

/*
 *	File: scsi_53C94_hdw.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Bottom layer of the SCSI driver: chip-dependent functions
 *
 *	This file contains the code that is specific to the NCR 53C94
 *	SCSI chip (Host Bus Adapter in SCSI parlance): probing, start
 *	operation, and interrupt routine.
 */

/*
 * This layer works based on small simple 'scripts' that are installed
 * at the start of the command and drive the chip to completion.
 * The idea comes from the specs of the NCR 53C700 'script' processor.
 *
 * There are various reasons for this, mainly
 * - Performance: identify the common (successful) path, and follow it;
 *   at interrupt time no code is needed to find the current status
 * - Code size: it should be easy to compact common operations
 * - Adaptability: the code skeleton should adapt to different chips without
 *   terrible complications.
 * - Error handling: and it is easy to modify the actions performed
 *   by the scripts to cope with strange but well identified sequences
 *
 */


#include <psc.h>
#if	NPSC > 0

#include "sbc.c"

#include <platforms.h>

#include <ppc/proc_reg.h> /* For isync */

typedef	unsigned char psc_register_t;
#define PAD(n) u_char n[15];

#define	PSC_PROBE_DYNAMICALLY FALSE
#include <mach_debug.h>


#include <kern/spl.h>
#include <mach/std_types.h>
#include <types.h>
#include <chips/busses.h>
#include <scsi/compat_30.h>

#include <scsi/scsi.h>
#include <scsi/scsi2.h>
#include <scsi/scsi_defs.h>

#include <ppc/POWERMAC/scsi_53C94.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_gestalt.h>
#include <ppc/POWERMAC/device_tree.h>

#define	readback(a)	{ register int foo;  eieio(); foo = (a);}

/* Used only for debugging asserts */
#include <ppc/mem.h>
#include <ppc/pmap.h>
#include <ppc/POWERMAC/interrupts.h>


int	psc_to_scsi_period(int psc, int clk);

static void	psc_intr(int device, void *ssp);

#if 0

/*
 * Internal forward declarations.
 */
static void psc_reset();
static void psc_startcmd();

#if	MACH_DEBUG
#define PSC_DEBUG	1
#define TRACE	1
#endif

#define	PSC_DEBUG 1

#if PSC_DEBUG
int	psc_trace_debug = 0;
#endif

/*
 * Scripts are entries in a state machine table.
 * A script has four parts: a pre-condition, an action, a command to the chip,
 * and an index into psc_scripts for the next state. The first triggers error
 * handling if not satisfied and in our case it is formed by the
 * values of the interrupt register and status register, this
 * basically captures the phase of the bus and the TC and BS
 * bits.  The action part is just a function pointer, and the
 * command is what the 53C94 should be told to do at the end
 * of the action processing.  This command is only issued and the
 * script proceeds if the action routine returns TRUE.
 * See psc_intr() for how and where this is all done.
 */

#include "ncr5380var.h"
// struct ncr5380_softc global_softc;

typedef struct script {
	int		condition;	/* expected state at interrupt time */
	int		(*action)();	/* extra operations */
	int		command;	/* command to the chip */
	struct script	*next;		/* index into psc_scripts for next state */
} script_t;

/* Matching on the condition value */
/* MEB - note mask was 67 to catch Gross Error, but
 * the chip reports a Gross Error in response to a phase
 * change while performing  synchronous transfers.
 */

#define	SCRIPT_MATCH(ir, csr)		((ir) | (((csr) & 0x27) << 8))

typedef struct {
	volatile psc_register_t	psc_tc_lsb;	/* rw: Transfer Counter LSB */
	PAD(pad0)
	volatile psc_register_t	psc_tc_msb;	/* rw: Transfer Counter MSB */
	PAD(pad1)
	volatile psc_register_t	psc_fifo;	/* rw: FIFO top */
	PAD(pad2)
	volatile psc_register_t	psc_cmd;	/* rw: Command */
	PAD(pad3)
	volatile psc_register_t	psc_csr;	/* r:  Status */
	PAD(pad4)
	volatile psc_register_t	psc_intr;	/* r:  Interrupt */
	PAD(pad5)
	volatile psc_register_t	psc_ss;		/* r:  Sequence Step */
	PAD(pad6)
	volatile psc_register_t	psc_flags;	/* r:  FIFO flags + seq step */
	PAD(pad7)
	volatile psc_register_t	psc_cnfg1;	/* rw: Configuration 1 */
	PAD(pad8)
	volatile psc_register_t	psc_ccf;	/* w:  Clock Conv. Factor */
	PAD(pad9)
	volatile psc_register_t	psc_test;	/* w:  Test Mode */
	PAD(pad10)
	volatile psc_register_t	psc_cnfg2;	/* rw: Configuration 2 */
	PAD(pad11)
	volatile psc_register_t	psc_cnfg3;	/* rw: Configuration 3 */
	PAD(pad12)
	volatile psc_register_t psc_cnfg4;	/* rw: Configuration 4 */
	PAD(pad13);
	volatile psc_register_t fas_tc_hi;	/* rw: high register count */
	PAD(pad14);
} sbc_regs;

/* Various Aliases.. */

#define	psc_dbus_id	psc_csr
#define	psc_sel_timo 	psc_intr
#define	psc_syn_p	psc_ss
#define	psc_syn_o	psc_flags

/*
 * DMA Maps.. 
 */


typedef struct dma_softc {
	volatile unsigned char	*base;
	volatile unsigned char	*ctrl;
	volatile unsigned char	*cur_ptr;
} dma_softc_t;

dma_softc_t	dma_softc[NPSC];

#define	u_min(a, b)	(((a) < (b)) ? (a) : (b))

/*
 * State kept for each active SCSI device.
 */

#define	PSC_MAX_SG_SEGS	64

struct sg_segment {
	unsigned long	sg_offset;
	unsigned long	sg_count;
	vm_offset_t	sg_physaddr;
	unsigned char	*sg_vmaddr;
};

typedef struct scsi_state {
	script_t *script;	/* saved script while processing error */
	char	cdb[16];	/* Command block */
	int	statusByte;	/* status byte returned during STATUS_PHASE */
	int	error;		/* errno to pass back to device driver */
	int	dmalen;		/* amount to transfer in this chunk */
	int	total_count;	/* total bytes to transfer */
	u_char	*dma_area;	/* base address to DMA to from */
	int	buflen;		/* total remaining amount of data to transfer */
	int	saved_buflen;	/* Saved buflen for Save Pointers Msg */
	int	saved_sg_index;	/* Saved S/G Index for Save Pointers Msg */
	int	flags;		/* see below */
	int	sg_segment_count;	/* Number of S/G segments */
	struct	sg_segment	sg_list[PSC_MAX_SG_SEGS];
	int	sg_index;	/* Index into sg_list */
	int	device_flags;	/* various device flags */
	int	msg_sent;	/* What message was sent to device */
	int	msglen;		/* number of message bytes to read */
	int	msgcnt;		/* number of message bytes received */
	u_char	msg_out;	/* next MSG_OUT byte to send */
	u_char	msg_in[16];	/* buffer for multibyte messages */
} State;

/* state flags */
#define STATE_DISCONNECTED	0x001	/* currently disconnected from bus */
#define STATE_DMA_RUNNING	0x002	/* data DMA started */
#define STATE_DMA_IN		0x004	/* reading from SCSI device */
#define STATE_DMA_OUT		0x010	/* writing to SCSI device */
#define	STATE_CHAINED_IO	0x020	/* request is a S/G one */
#define STATE_PARITY_ERR	0x080	/* parity error seen */
#define	STATE_DMA_FIFO		0x100	/* using FIFO to transfer data */
#define	STATE_FIFO_XFER		0x200	/* request has to use FIFO */

#endif // 0

/* device flags */
#define	DEV_NO_DISCONNECT	0x001	/* Do not preform disconnect */
#define	DEV_RUN_SYNC		0x002	/* Run synchronous transfers */

#if 0

#define	PSC_NCMD	8

/*
 * State kept for each active SCSI host interface (5380).
 * This is for reference only.  The actual structures are
 * in ncr5380var.h, etc.
 */

struct ncr5380_softc {
	struct mach_device	sc_dev;
	struct		scsipi_link sc_link;

	/* Pointers to 5380 registers.  See ncr5380reg.h */
	volatile u_char *sci_r0;
	volatile u_char *sci_r1;
	volatile u_char *sci_r2;
	volatile u_char *sci_r3;
	volatile u_char *sci_r4;
	volatile u_char *sci_r5;
	volatile u_char *sci_r6;
	volatile u_char *sci_r7;

	/* Functions set from MD code */
	int		(*sc_pio_out) __P((struct ncr5380_softc *,
					   int, int, u_char *));
	int		(*sc_pio_in) __P((struct ncr5380_softc *,
					  int, int, u_char *));
	void		(*sc_dma_alloc) __P((struct ncr5380_softc *));
	void		(*sc_dma_free) __P((struct ncr5380_softc *));

	void		(*sc_dma_setup) __P((struct ncr5380_softc *));
	void		(*sc_dma_start) __P((struct ncr5380_softc *));
	void		(*sc_dma_poll) __P((struct ncr5380_softc *));
	void		(*sc_dma_eop) __P((struct ncr5380_softc *));
	void		(*sc_dma_stop) __P((struct ncr5380_softc *));

	void		(*sc_intr_on) __P((struct ncr5380_softc *));
	void		(*sc_intr_off) __P((struct ncr5380_softc *));

	int		sc_flags;	/* Misc. flags and capabilities */
#define	NCR5380_FORCE_POLLING	1	/* Do not use interrupts. */

	/* Set bits in this to disable disconnect per-target. */
	int 	sc_no_disconnect;

	/* Set bits in this to disable parity for some target. */
	int		sc_parity_disable;

	int 	sc_min_dma_len;	/* Smaller than this is done with PIO */

	/* Begin MI shared data */

	int		sc_state;
#define	NCR_IDLE		   0	/* Ready for new work. */
#define NCR_WORKING 	0x01	/* Some command is in progress. */
#define	NCR_ABORTING	0x02	/* Bailing out */
#define NCR_DOINGDMA	0x04	/* The FIFO data path is active! */
#define NCR_DROP_MSGIN	0x10	/* Discard all msgs (parity err detected) */

	/* The request that has the bus now. */
	struct		sci_req *sc_current;

	/* Active data pointer for current SCSI command. */
	u_char		*sc_dataptr;
	int		sc_datalen;

	/* Begin MI private data */

	/* The number of operations in progress on the bus */
	volatile int	sc_ncmds;

	/* Ring buffer of pending/active requests */
	struct		sci_req sc_ring[SCI_OPENINGS];
	int		sc_rr;		/* Round-robin scan pointer */

	/* Active requests, by target/LUN */
	struct		sci_req *sc_matrix[8][8];

	/* Message stuff */
	int	sc_prevphase;

	u_int	sc_msgpriq;	/* Messages we want to send */
	u_int	sc_msgoutq;	/* Messages sent during last MESSAGE OUT */
	u_int	sc_msgout;	/* Message last transmitted */
#define SEND_DEV_RESET		0x01
#define SEND_PARITY_ERROR	0x02
#define SEND_ABORT		0x04
#define SEND_REJECT		0x08
#define SEND_INIT_DET_ERR	0x10
#define SEND_IDENTIFY  		0x20
#define SEND_SDTR		0x40
#define	SEND_WDTR		0x80
#define NCR_MAX_MSG_LEN 8
	u_char  sc_omess[NCR_MAX_MSG_LEN];
	u_char	*sc_omp;		/* Outgoing message pointer */
	u_char	sc_imess[NCR_MAX_MSG_LEN];
	u_char	*sc_imp;		/* Incoming message pointer */

};


#define	PSC_STATE_IDLE		0	/* idle state */
#define	PSC_STATE_BUSY		1	/* selecting or currently connected */
#define PSC_STATE_TARGET	2	/* currently selected as target */
#define PSC_STATE_RESEL		3	/* currently waiting for reselect */

#endif // 0

// typedef struct ncr5380_softc struct psc_softc;

#define psc_softc sbc_softc
typedef struct psc_softc *psc_softc_t;

struct psc_softc psc_softc_data[NPSC];
psc_softc_t	 psc_softc[NPSC];

boolean_t	psc_probe_dynamically = FALSE;

/* 
 * Various constants.. 
 */

#define	PSC_MAX_OFFSET		7
#define	PSC_MIN_PERIOD_20	5
#define	PSC_MIN_PERIOD_40	4

#define	PSC_NCR_53C94		0	/* Regular 53C94 */
#define	PSC_NCR_53CF94		1	/* Fast-SCSI version of 53C94 */

/*
 * Dma operations.
 */
#define	PSCDMA_READ	1
#define	PSCDMA_WRITE	2

#if 0
/* forward decls of script actions */
void	    psc_start_new_request(psc_softc_t psc);
static int script_nop(psc_softc_t, int, int, int);		/* when nothing needed */
static int psc_end(psc_softc_t, int, int, int);			/* all come to an end */
static int psc_get_status(psc_softc_t, int, int, int);		/* get status from target */
static int psc_dma_in(psc_softc_t, int, int, int);		/* start reading data from target */
static int psc_last_dma_in(psc_softc_t, int, int, int);		/* cleanup after all data is read */
static int psc_dma_out(psc_softc_t, int, int, int);		/* send data to target via dma */
static int psc_last_dma_out(psc_softc_t, int, int, int);		/* cleanup after all data is written */
static void psc_end_read_transfer(psc_softc_t psc, State * state);
static void psc_end_write_transfer(psc_softc_t psc, State * state);
static int psc_sendsync(psc_softc_t, int, int, int);		/* negotiate sync xfer */
static int psc_replysync(psc_softc_t, int, int, int);		/* negotiate sync xfer */
static int psc_msg_in(psc_softc_t, int, int, int);		/* process a message byte */
static int psc_disconnect(psc_softc_t, int, int, int);		/* process an expected disconnect */
static int psc_send_command(psc_softc_t, int, int, int);		/* send the command bytes */
void psc_sync_cache_data(target_info_t *tgt, State *state);
static int psc_clean_fifo(psc_softc_t psc, int status, int ss, int ir);
void	psc_build_sglist(State *st, target_info_t *tgt);
static boolean_t psc_get_io(psc_softc_t psc, State *st, unsigned char **address,
		unsigned long *count, boolean_t has_to_poll);

#endif // 0

static void psc_flush_fifo(psc_softc_t psc);
int psc_go(target_info_t *tgt, int cmd_count, int in_count,
		boolean_t cmd_only);
static void psc_load_fifo(psc_softc_t psc, unsigned char * buf, int count);
int psc_reset_scsibus(psc_softc_t	psc);


/* Define the index into psc_scripts for various state transitions */
#define	SCRIPT_DATA_IN		0
#define	SCRIPT_CONTINUE_IN	2
#define	SCRIPT_DATA_OUT		3
#define	SCRIPT_CONTINUE_OUT	5
#define	SCRIPT_SIMPLE		6
#define	SCRIPT_GET_STATUS	7
#define	SCRIPT_DONE		8
#define	SCRIPT_MSG_IN		9
#define	SCRIPT_REPLY_SYNC	11
#define	SCRIPT_TRY_SYNC		12
#define	SCRIPT_STATE_DISCONNECT	15
#define	SCRIPT_RESEL		16
#define	SCRIPT_RESUME_IN	17
#define	SCRIPT_RESUME_DMA_IN	18
#define	SCRIPT_RESUME_OUT	19
#define	SCRIPT_RESUME_DMA_OUT	20
#define	SCRIPT_RESUME_NO_DATA	21

#define	SCRIPT_NUM(x)	(x ? x - psc_scripts : -1)


/*
 * Autoconfiguration data for config.
 */
void	psc_probe_for_targets(psc_softc_t psc, struct sbc_regs *regs);
int	psc_probe( caddr_t reg, void *ptr);
boolean_t psc_probe_target(target_info_t *tgt, io_req_t ior);
void	psc_alloc_cmdptr(psc_softc_t psc, target_info_t *tgt);

/* Various MACH driver variables.. */
caddr_t	psc_std[NPSC] = { 0 };
struct	bus_device *psc_dinfo[NPSC*8];
struct	bus_ctlr *psc_minfo[NPSC];
struct	bus_driver psc_driver = 
	{ psc_probe, scsi_slave, scsi_attach, (int(*)(void))psc_go,
	  psc_std, "rz", psc_dinfo, "psc", psc_minfo, BUS_INTR_B4_PROBE};


// extern scsi_phoenix_dma_ops_t scsi_amic_ops, scsi_phoenix_dbdma_ops;

int	psc_clock_speed_in_mhz[NPSC] = {
40
#if	NPSC > 1
, 20
#endif /* NPSC > 1*/
};


static void
psc_init_structures()
{
psc_softc[0]->ncr_sc.sci_r0 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(0*PSC_REGISTER_OFFSET);
psc_softc[0]->ncr_sc.sci_r1 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(1*PSC_REGISTER_OFFSET);
psc_softc[0]->ncr_sc.sci_r2 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(2*PSC_REGISTER_OFFSET);
psc_softc[0]->ncr_sc.sci_r3 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(3*PSC_REGISTER_OFFSET);
psc_softc[0]->ncr_sc.sci_r4 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(4*PSC_REGISTER_OFFSET);
psc_softc[0]->ncr_sc.sci_r5 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(5*PSC_REGISTER_OFFSET);
psc_softc[0]->ncr_sc.sci_r6 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(6*PSC_REGISTER_OFFSET);
psc_softc[0]->ncr_sc.sci_r7 = (caddr_t) PSC_BASE_PHYS+PSC_REGISTER_BASE+(7*PSC_REGISTER_OFFSET);

psc_softc[0]->ncr_sc.sc_pio_out=NULL;
psc_softc[0]->ncr_sc.sc_pio_in=NULL;
psc_softc[0]->ncr_sc.sc_dma_alloc=NULL;
psc_softc[0]->ncr_sc.sc_dma_free=NULL;
psc_softc[0]->ncr_sc.sc_dma_setup=NULL;
psc_softc[0]->ncr_sc.sc_dma_start=NULL;
psc_softc[0]->ncr_sc.sc_dma_poll=NULL;
psc_softc[0]->ncr_sc.sc_dma_eop=NULL;
psc_softc[0]->ncr_sc.sc_intr_on=NULL;
psc_softc[0]->ncr_sc.sc_intr_off=NULL;
psc_softc[0]->ncr_sc.sc_flags=0;
psc_softc[0]->ncr_sc.sc_no_disconnect=0;
psc_softc[0]->ncr_sc.sc_parity_disable=0;
psc_softc[0]->ncr_sc.sc_min_dma_len=0;
psc_softc[0]->ncr_sc.sc_state=0;
psc_softc[0]->ncr_sc.sc_current=NULL;
psc_softc[0]->ncr_sc.sc_dataptr=NULL;
psc_softc[0]->ncr_sc.sc_datalen=0;
}


static void
psc_softc_psc_to_softc_ncr(psc_softc_t psc, struct ncr5380_softc *ncr)
{



return;
}


static void
psc_softc_ncr_to_softc_psc(struct ncr5380_softc *ncr, psc_softc_t psc)
{
return;
}


static void
psc_set_flags(unsigned char *env, int flags)
{
	register int unit, i;

	if (strcmp(env, "all") == 0) {
		for (unit = 0; unit < NPSC; unit++)
#if 0
			for (i = 0; i < 7; i++)
				psc_softc_data[unit].st[i].device_flags |= flags;
#else
			psc_softc_data[unit].ncr_sc.sc_flags |= flags;
#endif
	} else {
#if 0
	   {
		while (*env) {
			if (*env >= '0' && *env < '7') {
				for (unit = 0; unit < NPSC; unit++)
					psc_softc_data[unit].st[*env - '0'].device_flags |= flags;
			}
			env++;
		}
	   }
#else
	printf("flag setting not supported for individual targets.\n");
#endif
	}
}

static  void
psc_getconfig(void)
{
	register int unit, i;
	unsigned char	*env;
	extern unsigned char *getenv(char *);


	if ((env = getenv("scsi_nodisconnect")) != NULL)
		psc_set_flags(env, DEV_NO_DISCONNECT);

	if (powermac_info.class == POWERMAC_CLASS_PCI) {
		if ((env = getenv("scsi_sync")) != NULL)
			psc_set_flags(env, DEV_RUN_SYNC);
	}

}

				
int
psc_probe(
	caddr_t		reg,
	void		*ptr)
{
	struct bus_ctlr	*ui	 = (struct bus_ctlr *) ptr;
	int		unit	= ui->unit;
	register psc_softc_t psc = &psc_softc_data[unit];
	register struct sbc_regs *regs;
	target_info_t	*tgt;
	scsi_softc_t	*sc;
	int id, s, i, a;
	device_node_t	*dev = NULL;


	switch (powermac_info.class) {
	case	POWERMAC_CLASS_PDM:
		return 0;
	case	POWERMAC_CLASS_POWERBOOK:
		psc->sc_regs = (struct sbc_regs *) PSC_REGISTER_BASE;
	case	POWERMAC_CLASS_PCI:
		/*
		 * Note - only one 5380!
		 * Is that limitation really necessary?  Is this
		 * probe really necessary?
		 */
		if (unit != 1 || (dev = find_devices("5380")) == NULL) 
			return 0;

		//psc->dma_ops = &scsi_phoenix_dbdma_ops;
		psc->sc_regs = (struct sbc_regs *)
				dev->addrs[0].address;
		break;
	}

	s = splbio();

	simple_lock_init(&psc->lock, ETAP_IO_DEV);
	simple_lock_init(&psc->queue_lock, ETAP_IO_DEV);

	simple_lock(&psc->lock);

	// psc->unit = unit;
	psc_softc[unit] = psc;

	regs = psc->sc_regs = (struct sbc_regs *)
			POWERMAC_IO((vm_offset_t)psc->sc_regs);
	
	// XXX  For now, we have to assume the chip is really there because
	//      I don't have any hardware specs.


#if 0

	/*
	 * Check to see if there is a chip present.. 
	 */

	regs->psc_cmd = PSC_CMD_FLUSH; eieio();
	delay(100);
	regs->psc_fifo = 0x55; eieio();
	regs->psc_fifo = 0xaa; eieio();

	if (((a = regs->psc_flags & PSC_FLAGS_FIFO_CNT)) != 2) {
		simple_unlock(&psc->lock);
		splx(s);
		return	0;	/* Nothing present.. */
	}

	eieio();

	if ((a = regs->psc_fifo) != 0x55) {
		simple_unlock(&psc->lock);
		splx(s);
		return	0;
	}

	eieio();

	if ((a = regs->psc_fifo) != 0xaa) {
		simple_unlock(&psc->lock);
		splx(s);
		return	0;
	}

	psc->max_scsi_offset = PSC_MAX_OFFSET;

	switch (unit) {
	case	1:
		psc->min_period = PSC_MIN_PERIOD_20;
		psc->type = PSC_NCR_53C94;
		psc->max_transfer = PSC_TC_MAX;
		break;

	case	0:
		psc->min_period = PSC_MIN_PERIOD_40;
		psc->type = PSC_NCR_53CF94;
		psc->max_transfer = FSC_TC_MAX;
		break;

	default:
		printf("psc_probe: Unknown SCSI Bus %d??\n", unit);
		simple_unlock(&psc->lock);
		splx(s);
		return 0;
	}

	if (psc->dma_ops)
		psc->dma_ops->dma_init(unit);

	queue_init(&psc->target_queue);

#endif

	sc = scsi_master_alloc(unit, (char *) psc);
	psc->sc = sc;

	// return 0;
#if 0
	// sc->go = (void *) psc_go; // write me!
	// sc->watchdog = psc->ncr_sc; // @@@ ??? @@@
	// sc->probe = psc_probe_target; // write me!
	// psc->wd.reset = ((void (*)(struct watchdog_t *))ncr5380_reset_scsibus);

	sc->max_dma_data = -1;
	sc->max_dma_segs = PSC_MAX_SG_SEGS;

	psc->state = PSC_STATE_IDLE;
	psc->target = -1;

	regs = psc->sc_regs;

	/* preserve our ID for now */
	psc->sc_id = 7; /*regs->psc_cnfg1 & PSC_CNFG1_MY_BUS_ID; eieio();*/
	psc->myidmask = ~(1 << psc->sc_id);

	psc_reset(psc, regs);

	id = psc->sc_id;
	sc->initiator_id = id;

	printf("%s%d: NCR %s %d mhz SCSI ID %d",
		ui->name, unit,
		(psc->type == PSC_NCR_53C94 ? "53C94" : "53CF94"),
		psc->clk, id); 

	tgt = scsi_slave_alloc(sc->masterno, sc->initiator_id, (char *) psc);
	psc_alloc_cmdptr(psc, tgt);
	sccpu_new_initiator(tgt, tgt);
	psc_probe_for_targets(psc, regs);
	psc_getconfig();
	printf("\n");

	switch (unit) {
	case	1:
		if (phoenixdev)
			pmac_register_ofint(phoenixdev->intrs[0], SPLBIO, psc_intr);
		else
			pmac_register_int(PMAC_DEV_SCSI0, SPLBIO, psc_intr);
		break;

	case	0:
		pmac_register_int(PMAC_DEV_SCSI1, SPLBIO, psc_intr);
		break;
	}

#endif

	/*
	 * Check to see if chip is present
	 * not done yet... don't know how
	 */
	if (0) return 0;

	/*
	 * Check to see if any targets
	 * available on 5380.
	 */

	psc_probe_for_targets(psc, regs);

	/*
	 * Register interrupts
	 */

	switch(powermac_info.class) {
		case POWERMAC_CLASS_POWERBOOK:
			pmac_register_int(PMAC_DEV_SCSI0, SPLBIO, psc_intr);
			break;
		case POWERMAC_CLASS_PCI:
			pmac_register_ofint(dev->intrs[0], SPLBIO, psc_intr);
			break;
		default:
			panic("Unknown machine class for NCR5380 scsi driver.\n");
		}

	simple_unlock(&psc->lock);
	splx(s);

	return 1;
}

boolean_t
psc_probe_target(
	target_info_t 		*tgt,
	io_req_t		ior)
{
	psc_softc_t     psc;

	psc = psc_softc[tgt->masterno];

	if (scsi_inquiry(tgt, SCSI_INQ_STD_DATA) == SCSI_RET_DEVICE_DOWN) 
		return FALSE;

	tgt->flags |= TGT_ALIVE|TGT_CHAINED_IO_SUPPORT;

	return TRUE;
}

#if 0 // @@@

void
psc_alloc_cmdptr(psc_softc_t psc, target_info_t *tgt)
{
	tgt->cmd_ptr = psc->cmd_areas[tgt->target_id];
	tgt->dma_ptr = 0;
}

#endif

void
psc_probe_for_targets(psc_softc_t psc, struct sbc_regs *regs)
{
	int	target_id, did_banner = 0;
	register unsigned char	csr, ss, ir, ff;
	register scsi_status_byte_t	status;
	scsi_softc_t		*sc = psc->sc;

#if 0
	if (psc_probe_dynamically) {
		printf("psc%d: will probe targets on demand\n",
			psc->unit);
		return;
	}
#endif

#if 1
	return;
#endif

#if 0

	/*
	 * For all possible targets, see if there is one and allocate
	 * a descriptor for it if it is there.
	 */
	for (target_id = 0; target_id < 8; target_id++) {

		/* except of course ourselves */
		// if (target_id == sc->initiator_id)
		if (target_id == 0)
			continue;

		regs->psc_cmd = PSC_CMD_FLUSH;	/* empty fifo */
		eieio();
		delay(2);

		regs->psc_dbus_id = target_id; eieio();
		regs->psc_sel_timo = psc->timeout; eieio();

		/*
		 * See if the unit is ready.
		 * XXX SHOULD inquiry LUN 0 instead !!!
		 */
		regs->psc_fifo = SCSI_CMD_TEST_UNIT_READY; eieio();
		regs->psc_fifo = 0; eieio();
		regs->psc_fifo = 0; eieio();
		regs->psc_fifo = 0; eieio();
		regs->psc_fifo = 0; eieio();
		regs->psc_fifo = 0; eieio();

		/* select and send it */
		regs->psc_cmd = PSC_CMD_SEL; eieio();

		/* wait for the chip to complete, or timeout */
		csr = psc_wait(regs, PSC_CSR_INT);
		ss = regs->psc_ss; eieio();
		ir = regs->psc_intr; eieio();

		/* empty fifo, there is garbage in it if timeout */
		psc_flush_fifo(psc);

		/*
		 * Check if the select timed out
		 */
		if ((PSC_SS(ss) == 0) && (ir == PSC_INT_DISC))
			/* noone out there */
			continue;

		if (SCSI_PHASE(csr) != SCSI_PHASE_STATUS) {
			printf( " %s%d%s", "ignoring target at ", target_id,
				" cuz it acts weirdo");
			continue;
		}

		printf(",%s%d", did_banner++ ? " " : " target(s) at ",
				target_id);

		regs->psc_cmd = PSC_CMD_I_COMPLETE; eieio();
		csr = psc_wait(regs, PSC_CSR_INT);
		ir = regs->psc_intr; /* ack intr */
		eieio();

		status.bits = regs->psc_fifo; /* empty fifo */
		eieio();
		ff = regs->psc_fifo; eieio();

		if (status.st.scsi_status_code != SCSI_ST_GOOD)
			scsi_error( 0, SCSI_ERR_STATUS, status.bits, 0);

		regs->psc_cmd = PSC_CMD_MSG_ACPT; eieio();
		csr = psc_wait(regs, PSC_CSR_INT);
		ir = regs->psc_intr; /* ack intr */
		eieio();

		/*
		 * Found a target
		 */
		{
			register target_info_t	*tgt;
			tgt = scsi_slave_alloc(sc->masterno, target_id, (char *)psc);
			psc_alloc_cmdptr(psc, tgt);
			tgt->flags |= TGT_CHAINED_IO_SUPPORT;
		}
	}
#endif

}

#if 0

int
psc_to_scsi_period(int	a, int	clk)
{
	/* Note: the SCSI unit is 4ns, hence
		A_P * 1,000,000,000
		-------------------  =  S_P
		    C_Mhz  * 4
	 */
	return a * (250 / clk);
		
}

int
scsi_period_to_psc( int p, int clk, int	min_period)
{
	register int 	ret;

	ret = (p * clk) / 250;

	if (ret < min_period)
		return min_period;

	if ((psc_to_scsi_period(ret,clk)) < p)
		return ret + 1;
	return ret;
}

#endif

/*
 * Start activity on a SCSI device.
 * We maintain information on each device separately since devices can
 * connect/disconnect during an operation.
 */

int /* @@@ */
psc_go(
	target_info_t		*tgt,
	int			cmd_count,
	int			in_count,
	boolean_t		cmd_only)
{
	register psc_softc_t psc;
	// State		*state;
	int s;

#if 0

	psc = (psc_softc_t) tgt->hw_state;

	if (psc->cmd[tgt->target_id]) 
		panic("SCSI Bus %d: Queueing Problem: Target %d busy at start\n", 
			psc->unit, tgt->target_id);

	tgt->transient_state.cmd_count = cmd_count; /* keep it here */

	state = &psc->st[tgt->target_id];

	/*
	 * Setup target state
	 */
	tgt->done = SCSI_RET_IN_PROGRESS;
	state->flags = 0;
	state->buflen = 0;
	state->saved_buflen = 0;
	state->sg_index = 0;
	state->sg_segment_count = 0;

	if ((state->device_flags & DEV_RUN_SYNC) == 0)
		tgt->flags |= TGT_DID_SYNCH;	/* Prevent Sync Transfers*/

	/* 
	 * WHY doesn't the MACH SCSI drivers tell
	 * the SCSI controllers what the I/O direction
	 * is supposed to be?!? ARGHH..
	 */

	switch (tgt->cur_cmd) {
	case SCSI_CMD_INQUIRY:
		if ((tgt->flags & TGT_DID_SYNCH) == 0) {
			state->script = &psc_scripts[SCRIPT_TRY_SYNC];
			tgt->flags |= TGT_TRY_SYNCH;
			state->flags |= STATE_DMA_IN;
			break;
		} 
		/* Fall thru.. */
	case SCSI_CMD_REQUEST_SENSE:
	case SCSI_CMD_MODE_SENSE:
	case SCSI_CMD_RECEIVE_DIAG_RESULTS:
	case SCSI_CMD_READ_CAPACITY:
	case SCSI_CMD_READ_BLOCK_LIMITS:
	case SCSI_CMD_READ_TOC:
	case SCSI_CMD_READ_SUBCH:
	case SCSI_CMD_READ_HEADER:
	case 0xc4:	/* despised: SCSI_CMD_DEC_PLAYBACK_STATUS */
	case 0xdd:	/* despised: SCSI_CMD_NEC_READ_SUBCH_Q */
	case 0xde:	/* despised: SCSI_CMD_NEC_READ_TOC */
	case SCSI_CMD_READ:
	case SCSI_CMD_LONG_READ:
		state->script = &psc_scripts[SCRIPT_DATA_IN];
		state->flags |= STATE_DMA_IN;
		break;

	case SCSI_CMD_WRITE:
	case SCSI_CMD_LONG_WRITE:
		state->script = &psc_scripts[SCRIPT_DATA_OUT];
		state->flags |= STATE_DMA_OUT;
		break;

	case SCSI_CMD_MODE_SELECT:
	case SCSI_CMD_REASSIGN_BLOCKS:
	case SCSI_CMD_FORMAT_UNIT:
	case 0xc9: /* vendor-spec: SCSI_CMD_DEC_PLAYBACK_CONTROL */
		tgt->transient_state.cmd_count = sizeof_scsi_command(tgt->cur_cmd);
		tgt->transient_state.out_count =
			cmd_count - tgt->transient_state.cmd_count;
		state->script = &psc_scripts[SCRIPT_DATA_OUT];
		state->flags |= STATE_DMA_OUT|STATE_FIFO_XFER;
		break;

	case SCSI_CMD_TEST_UNIT_READY:
		/*
		 * Do the synch negotiation here, unless prohibited
		 * or done already
		 */

		if ((tgt->flags & TGT_DID_SYNCH) == 0) {
			state->script = &psc_scripts[SCRIPT_TRY_SYNC];
			tgt->flags |= TGT_TRY_SYNCH;
			break;
		} else
			state->script = &psc_scripts[SCRIPT_SIMPLE];
		break;

	 default:
		state->script = &psc_scripts[SCRIPT_SIMPLE];
		break;
	}

	tgt->transient_state.in_count = in_count;

	/* Figure out where to DMA to/from.. 
	  (Again, WHY?!? does MACH do this?!?)
	*/

	switch (tgt->cur_cmd) {
	case	SCSI_CMD_READ:
	case	SCSI_CMD_LONG_READ:
		state->buflen = in_count;
		goto check_chain;

	case	SCSI_CMD_WRITE:
	case	SCSI_CMD_LONG_WRITE:
		tgt->transient_state.out_count = tgt->ior->io_count;
		state->buflen = tgt->ior->io_count; 
		
check_chain:
		if (tgt->ior->io_op & IO_CHAINED) {
			state->flags |= STATE_CHAINED_IO;
		} else 
			state->dma_area = tgt->ior->io_data;
		break;

	case	SCSI_CMD_MODE_SELECT:
	case	SCSI_CMD_REASSIGN_BLOCKS:
	case	SCSI_CMD_FORMAT_UNIT:
	case	0xc9: /* vendor-spec: SCSI_CMD_DEC_PLAYBACK_CONTROL */
		state->flags |= STATE_FIFO_XFER;
		state->buflen = tgt->transient_state.out_count;
		state->dma_area = (u_char *) tgt->cmd_ptr + sizeof(scsi_command_group_0);
		break;

	case SCSI_CMD_INQUIRY:
	case SCSI_CMD_REQUEST_SENSE:
	case SCSI_CMD_MODE_SENSE:
	case SCSI_CMD_RECEIVE_DIAG_RESULTS:
	case SCSI_CMD_READ_CAPACITY:
	case SCSI_CMD_READ_BLOCK_LIMITS:
	case SCSI_CMD_READ_TOC:
	case SCSI_CMD_READ_SUBCH:
	case SCSI_CMD_READ_HEADER:
		state->flags |= STATE_FIFO_XFER;
		state->buflen = tgt->transient_state.in_count;
		state->dma_area = (u_char *) tgt->cmd_ptr;
		break;

	default:
		state->buflen = 0;
		state->dma_area = 0;
		break;
	}

	/* The following should never happen.. */

	if (psc->dma_ops == NULL) 
		state->flags |= STATE_FIFO_XFER;

	/*
	 * Save off the CDB just in case the CDB and the
	 * area to DMA from/to are in the same cache line.
	 */

	bcopy(tgt->cmd_ptr, state->cdb, tgt->transient_state.cmd_count);

	state->total_count = state->buflen;

	if (state->total_count)
		psc_build_sglist(state, tgt);


	/*
	 * Check if another command is already in progress.
	 * We may have to change this if we allow SCSI devices with
	 * separate LUNs.
	 */

	s = splbio();
	simple_lock(&psc->lock);

	psc->cmd[tgt->target_id] = tgt;

	if (psc->target == -1) 
		psc_startcmd(psc, tgt->target_id);
	else {
		simple_lock(&psc->queue_lock);
		enqueue_tail(&psc->target_queue, (queue_entry_t) tgt);
		simple_unlock(&psc->queue_lock);
	}

	simple_unlock(&psc->lock);
	splx(s);

#endif

}

#if 0

void /* @@@ */
psc_start_new_request(psc_softc_t psc)
{
	target_info_t	*tgt;

	simple_lock(&psc->queue_lock);
	tgt = (target_info_t *) dequeue_head(&psc->target_queue);
	simple_unlock(&psc->queue_lock);

	if (tgt)
		psc_startcmd(psc, tgt->target_id);

}

static void /* @@@ */
psc_reset(psc_softc_t psc, struct sbc_regs * regs)
{

	psc->wd.nactive = 0;

	/*
	 * Reset chip and wait till done
	 */
	regs->psc_cmd = PSC_CMD_RESET;
	eieio(); delay(25);

	/* spec says this is needed after reset */
	regs->psc_cmd = PSC_CMD_NOP;
	eieio(); delay(25);

	/*
	 * Set up various chip parameters
	 */
	regs->psc_ccf = psc->ccf; eieio();
	delay(25);
	regs->psc_sel_timo = psc->timeout; eieio();

	/* restore our ID */

	regs->psc_cnfg1 = psc->sc_id | PSC_CNFG1_SLOW; eieio();

	switch (psc->type) {
	case	PSC_NCR_53C94:
		regs->psc_cnfg2 = PSC_CNFG2_SCSI2; eieio();
		if (powermac_info.class == POWERMAC_CLASS_PCI) {
			psc->nondma_cnfg3 = psc->dma_cnfg3 = PSC_CNFG3_SRB;
		} else {
			psc->nondma_cnfg3 = PSC_CNFG3_SRB;
			psc->dma_cnfg3 = PSC_CNFG3_T8|PSC_CNFG3_ALT_DMA|
					  PSC_CNFG3_SRB;
		}
		break;

	case	PSC_NCR_53CF94:
		regs->psc_cnfg2 = FAS_CNFG2_FEATURES | PSC_CNFG2_SCSI2;eieio();
		psc->nondma_cnfg3 = PSC_CNFG3_SRB | FAS_CNFG3_FASTSCSI | FAS_CNFG3_FASTCLK;
		psc->dma_cnfg3 = PSC_CNFG3_ALT_DMA|PSC_CNFG3_SRB|
				  PSC_CNFG3_T8 | FAS_CNFG3_FASTSCSI |
				  FAS_CNFG3_FASTCLK;
		regs->psc_cnfg4 = FSC_CNFG4_EAN; eieio();
		regs->fas_tc_hi = 0; eieio();
		break;
	}

	regs->psc_cnfg3 = psc->nondma_cnfg3; eieio();

	/* zero anything else */
	PSC_TC_PUT(regs, 0);
	regs->psc_syn_p = psc->min_period; eieio();
	regs->psc_syn_o = 0;	/* async for now */
	eieio();

}

/*
 * Start a SCSI command on a target.
 */
static void /* @@@ */
psc_startcmd(psc_softc_t psc, int target)
{
	register struct sbc_regs *regs;
	register target_info_t *tgt;
	register State *state;
	int len;
	u_char	command;


	regs = psc->sc_regs;

	/*
	 * If a reselection is in progress, it is Ok to ignore it since
	 * the PSC will automatically cancel the command and flush
	 * the FIFO if the PSC is reselected before the command starts.
	 * If we try to use PSC_CMD_DISABLE_SEL, we can hang the system if
	 * a reselect occurs before starting the command.
	 */

	psc->state = PSC_STATE_BUSY;
	psc->target = target;

	/* cache some pointers */
	tgt = psc->cmd[target];
	state = &psc->st[target];
	state->error = SCSI_RET_IN_PROGRESS;

	/*
	 * Init the chip and target state.
	 */
	state->msg_out = SCSI_NOP;
	psc->script = state->script;
	state->script = (script_t *) 0;
	state->dmalen = 0;

	/* preload the FIFO with the message to be sent */
	if (state->device_flags & DEV_NO_DISCONNECT
	|| state->flags & STATE_FIFO_XFER) {
		state->msg_sent = 0;
		regs->psc_fifo = SCSI_IDENTIFY;
	} else {
		state->msg_sent =  SCSI_IFY_ENABLE_DISCONNECT;
		regs->psc_fifo = SCSI_IDENTIFY|SCSI_IFY_ENABLE_DISCONNECT;
	}
		
	eieio();


	if (psc->script != &psc_scripts[SCRIPT_TRY_SYNC])
		/* Don't both sending command if phase is
		 * going to change to Msg-In immediately
		 */
		psc_load_fifo(psc, state->cdb, tgt->transient_state.cmd_count);

	/* initialize the DMA */
	PSC_TC_PUT(regs, 0); readback(regs->psc_cmd);

	regs->psc_dbus_id = target; readback(regs->psc_dbus_id);
	regs->psc_syn_p = tgt->sync_period; readback(regs->psc_syn_p);
	regs->psc_syn_o = tgt->sync_offset; readback(regs->psc_syn_o);

	if (psc->script == &psc_scripts[SCRIPT_TRY_SYNC])
		command = PSC_CMD_SEL_ATN_STOP;
	else
		command = PSC_CMD_SEL_ATN;

	/* Try to avoid reselect collisions */

	if ((regs->psc_csr & (PSC_CSR_INT|SCSI_PHASE_MSG_IN)) !=
	    (PSC_CSR_INT|SCSI_PHASE_MSG_IN))  {
		regs->psc_cmd = command;  
		readback(regs->psc_cmd);

		if (psc->wd.nactive++ == 0)
			psc->wd.watchdog_state = SCSI_WD_ACTIVE;

		/*
		 * Sync the cache here because this is something
		 * that can be done after the request for target
		 * selection has been issued. This helps to
		 * keep the SCSI bus from waiting on the
		 * CPU.
		 * (Possible item to look into, build
		 * S/G list as well here instead of above?
		 * -- Mike Burg 6/10/96)
		 */

		if (powermac_info.io_coherent == FALSE 
		&& state->buflen && !(state->flags & STATE_FIFO_XFER))
			psc_sync_cache_data(tgt, state);

		PSC_TRACE3("start {cmd %x, size %d, script #%d}",
			tgt->cur_cmd, state->buflen, SCRIPT_NUM(psc->script));
	} else 
		PSC_TRACE1("Collision start {cmd %x}", tgt->cur_cmd);

}
#endif

/*
 * Interrupt routine
 *	Take interrupts from the chip
 *
 * Implementation:
 *	Move along the current command's script if
 *	all is well, invoke error handler if not.
 */

void
psc_intr(int device, void *ssp)
{
	int	unit;
	register psc_softc_t psc;
	register struct sbc_regs *regs;
	// register State *state;
	// register script_t *scpt;
	// register int ss, ir, status;
	// register unsigned char cmd_was;


	unit = (device == PMAC_DEV_SCSI0 ? 1 : 0);
	psc = &psc_softc_data[unit];
	regs =  psc->sc_regs;

	simple_lock(&psc->lock);

// sbc_irq_intr(pb_softc);
sbc_irq_intr(psc);

simple_unlock(&psc->lock);
}

#if 0

/*
 * All the many little things that the interrupt
 * routine might switch to.
 */

/* ARGSUSED */
static int
script_nop(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	return (1);
}

static int
psc_clean_fifo(psc_softc_t psc, int status, int ss, int ir)
{
	struct sbc_regs	*regs = psc->sc_regs;
	unsigned char ff;


	PSC_TC_PUT(regs,0);	/* stop dma engine */
	readback(regs->psc_cmd);

	eieio();
	while (regs->psc_flags & PSC_FLAGS_FIFO_CNT) {
		ff = regs->psc_fifo;
		eieio();
	}

	return TRUE;
}

static void
psc_flush_fifo(psc_softc_t psc)
{
	struct sbc_regs	*regs = psc->sc_regs;


	regs->psc_cmd = PSC_CMD_FLUSH; eieio();
	readback(regs->psc_cmd);
	regs->psc_cmd = PSC_CMD_NOP; eieio();
	readback(regs->psc_cmd);
	regs->psc_cmd = PSC_CMD_NOP; eieio();
	readback(regs->psc_cmd);

	return;
}


static int
psc_send_command(psc_softc_t psc, int status, int ss, int ir)
{
	target_info_t	*tgt = psc->cmd[psc->target];
	State		*st = &psc->st[psc->target];


	psc_load_fifo(psc, (unsigned char *) st->cdb, 
			tgt->transient_state.cmd_count);

	return (1);
}

/* ARGSUSED */
static int
psc_get_status(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register struct sbc_regs *regs = psc->sc_regs;
	register int data;
	scsi2_status_byte_t	statusByte;
	State			*st = &psc->st[psc->target];


	/*
	 * Get the last two bytes in the FIFO.
	 */

	data = regs->psc_flags;	eieio();
	
	if ((data & PSC_FLAGS_FIFO_CNT) != 2) {
#ifdef PSC_DEBUG
		printf("psc_get_status: cmdreg %x, fifo cnt %d\n",
		       regs->psc_cmd, data); /* XXX */
		psc_DumpLog("Get Status FIFO problem"); /* XXX */
#endif
		if (data < 2) {
			psc->sc_regs->psc_cmd = PSC_CMD_MSG_ACPT;
			readback(psc->sc_regs->psc_cmd);
			return (0);
		}
		do {
			eieio();
			data = regs->psc_fifo; eieio();
		} while ((regs->psc_flags & PSC_FLAGS_FIFO_CNT) > 2);
	}

	/* save the status byte */
	statusByte.bits = regs->psc_fifo; eieio();

	if (statusByte.st.scsi_status_code != SCSI_ST_GOOD) {
		st->error = (statusByte.st.scsi_status_code == SCSI_ST_BUSY) ?
			SCSI_RET_RETRY : SCSI_RET_NEED_SENSE;
	} else
		st->error = SCSI_RET_SUCCESS;

	data = regs->psc_fifo; eieio();

	/* get the (presumed) command_complete message */
	if (data == SCSI_COMMAND_COMPLETE) {
		PSC_TRACE1("getstatus okay {st %x}", psc->st[psc->target].statusByte);
		return (1);
	}

	PSC_TRACE2("getstatus prob? {st %x, data %x}", psc->st[psc->target].statusByte,
			data);
#ifdef PSC_DEBUG
	printf("psc_get_status: status %x cmd %x\n",
		psc->st[psc->target].statusByte, data);
	psc_DumpLog("Get Status Error");
#endif
	return (0);
}

/* ARGSUSED */
static int
psc_end(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register target_info_t *tgt;
	register State *state;
	register int i, target;

	psc->state = PSC_STATE_IDLE;
	target = psc->target;
	psc->target = -1;
	tgt = psc->cmd[target];
	psc->cmd[target] = (target_info_t *)0;
	state = &psc->st[target];

	PSC_TRACE5("end {bus%d target %d cmd %x err %d resid %d}",
	  psc->unit, target, tgt->cmd_ptr[0], state->error, state->buflen);

	/* look for disconnected devices */
	for (i = 0; i < PSC_NCMD; i++) {
		if (!psc->cmd[i] || !(psc->st[i].flags & STATE_DISCONNECTED))
			continue;
		PSC_TRACE1("end {reconn tgt %d}", i);
		psc->sc_regs->psc_cmd = PSC_CMD_ENABLE_SEL;
		readback(psc->sc_regs->psc_cmd);
		psc->state = PSC_STATE_RESEL;
		psc->script = &psc_scripts[SCRIPT_RESEL];
		break;
	}

	/*
	 * Look for another device that is ready.
	 * May want to keep last one started and increment for fairness
	 * rather than always starting at zero.
	 */

	simple_lock(&psc->queue_lock);
	if (!queue_empty(&psc->target_queue)) {
		simple_unlock(&psc->queue_lock);
		psc_start_new_request(psc);
	} else
		simple_unlock(&psc->queue_lock);

	tgt->done = state->error;

	if (tgt->done == SCSI_RET_IN_PROGRESS)
		tgt->done = SCSI_RET_SUCCESS;

	if (psc->wd.nactive-- == 1)
		psc->wd.watchdog_state = SCSI_WD_INACTIVE;

	/* Note - SCSI driver may call into itself */
	simple_unlock(&psc->lock);

	if (tgt->ior) {
		(*tgt->dev_ops->restart)(tgt, TRUE);
	}

	simple_lock(&psc->lock);

	return (0);
}

static int
psc_dma_in(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register struct sbc_regs *regs = psc->sc_regs;
	register State *state = &psc->st[psc->target];
	unsigned char *address;
	long	len;


	/* check for previous chunk in buffer */
	if (state->flags & STATE_DMA_RUNNING) 
		psc_end_read_transfer(psc, state);

	state->flags |= STATE_DMA_RUNNING;

	regs->psc_cnfg1 = psc->sc_id; eieio();	/* Turn off slow cable mode.. */

	/* setup to start reading the next chunk */
	if (psc_get_io(psc, state, &address, &len, FALSE)) {

		regs->psc_cnfg3 = psc->dma_cnfg3; eieio();
		psc->dma_ops->dma_setup_transfer(psc->unit,
					(vm_offset_t) address, len, TRUE);
		state->dmalen = len;
		PSC_TC_PUT(regs, len);
		/* check for next chunk */
		regs->psc_cmd = PSC_CMD_XFER_INFO | PSC_CMD_DMA;
		readback(regs->psc_cmd);
		psc->dma_ops->dma_start_transfer(psc->unit, TRUE);

	} else {
		regs->psc_cnfg3 = psc->nondma_cnfg3; eieio();
		len = state->dmalen = 1;
		state->flags |= STATE_DMA_FIFO;
		regs->psc_cmd = PSC_CMD_XFER_INFO;
		readback(regs->psc_cmd);
	}

	PSC_TRACE2("read dma {xfer %d/left %d}", len, state->buflen);

	if (len != state->buflen) 
		psc->script = &psc_scripts[SCRIPT_CONTINUE_IN];
	else 
		psc->script = psc->script->next;

	return (0);
}

static void
psc_end_read_transfer(psc_softc_t psc, State * state)
{
	struct sbc_regs *regs = psc->sc_regs;
	unsigned long len, fifo, count;
	unsigned char *address;


	/*
	 * Only count bytes that have been copied to memory.
	 * There may be some bytes in the FIFO if synchonous transfers
	 * are in progress.
	 */
	if ((state->flags & STATE_DMA_FIFO) == 0) {
		PSC_TC_GET(regs, len);
		psc->dma_ops->dma_end(psc->unit, TRUE);
		len = state->dmalen - len;
		state->buflen -= len;

		PSC_TRACE2("read dma end {resid %d, left %d}",
			len, state->buflen);
	}

	regs->psc_cnfg1 = psc->sc_id | PSC_CNFG1_SLOW; eieio();

	regs->psc_cnfg3 = psc->nondma_cnfg3; eieio();

	len = regs->psc_flags & PSC_FLAGS_FIFO_CNT; eieio();

	if (len) {
		PSC_TRACE2("read fifo end {fifo %d, left %d}",
			len, (state->buflen - len));

		while (len-- > 0 && state->buflen > 0) {
			(void) psc_get_io(psc, state, &address, &count, TRUE);

			/* Try to compenstate for cache 
			 * synch problems..
			 */

			if (powermac_info.io_coherent == FALSE
			&& (state->flags & STATE_FIFO_XFER) == 0)
				invalidate_dcache((vm_offset_t) address,
						  1,
						  FALSE);

			*address = regs->psc_fifo; eieio();
			state->buflen--;

			if (powermac_info.io_coherent == FALSE
			&& (state->flags & STATE_FIFO_XFER) == 0)
				flush_dcache((vm_offset_t) address, 1, FALSE);
		}

		if (len)
			psc_flush_fifo(psc);
	}

	state->flags &= ~(STATE_DMA_RUNNING|STATE_DMA_FIFO);

}


/* ARGSUSED */
static int
psc_last_dma_in(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register struct sbc_regs *regs = psc->sc_regs;
	register State *state = &psc->st[psc->target];
	register int len, fifo;


	psc_end_read_transfer(psc, state);

	return (1);
}

static void
psc_end_write_transfer(psc_softc_t psc, State *state)
{
	register struct sbc_regs *regs = psc->sc_regs;
	unsigned long len, fifo;


	/* check to be sure previous chunk was finished */
	if ((state->flags & STATE_DMA_FIFO) == 0) {
		PSC_TC_GET(regs, len);
		psc->dma_ops->dma_end(psc->unit, FALSE);
	} else
		len = 0;

	regs->psc_cnfg1 = psc->sc_id | PSC_CNFG1_SLOW; eieio();

	regs->psc_cnfg3 = psc->nondma_cnfg3; eieio();

	fifo = regs->psc_flags & PSC_FLAGS_FIFO_CNT; eieio();

	PSC_TRACE2("end write {resid %d, buflen %d}",
			fifo+len, state->buflen);

	if (fifo) { 
		psc_flush_fifo(psc);
		len += fifo;
	}

	len = state->dmalen - len;
	state->buflen -= len;

	state->flags &= ~(STATE_DMA_FIFO|STATE_DMA_RUNNING);

}

/* ARGSUSED */
static int
psc_dma_out(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register struct sbc_regs *regs = psc->sc_regs;
	register State *state = &psc->st[psc->target];
	unsigned char *address;
	unsigned long  len, fifo;


	if (state->flags & STATE_DMA_RUNNING) 
		psc_end_write_transfer(psc, state);

	state->flags |= STATE_DMA_RUNNING;

	regs->psc_cnfg1 = psc->sc_id; eieio();	/* Turn off slow cable mode */

	/* check for next chunk */
	if (psc_get_io(psc, state, &address, &len, FALSE)) {
		regs->psc_cnfg3 = psc->dma_cnfg3; eieio();
		psc->dma_ops->dma_setup_transfer(psc->unit,
					(vm_offset_t) address, len, FALSE);
		state->dmalen = len;
		PSC_TC_PUT(regs, len);
		regs->psc_cmd = PSC_CMD_XFER_INFO | PSC_CMD_DMA;
		readback(regs->psc_cmd);
		psc->dma_ops->dma_start_transfer(psc->unit, FALSE);
	} else {
		regs->psc_cnfg3 = psc->nondma_cnfg3; eieio();
		psc_load_fifo(psc, address, len);
		state->dmalen = len;
		state->flags |= STATE_DMA_FIFO;
		regs->psc_cmd = PSC_CMD_XFER_INFO;
		readback(regs->psc_cmd);
	}


	PSC_TRACE2("dma out {xfer %d, left %d}", len, state->buflen);

	if (len != state->buflen) 
		psc->script = &psc_scripts[SCRIPT_CONTINUE_OUT];
	else
		psc->script = psc->script->next;

	return (0);
}

/* ARGSUSED */
static int
psc_last_dma_out(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register State *state = &psc->st[psc->target];


	psc_end_write_transfer(psc, state);

	return (1);
}

/* ARGSUSED */
static int
psc_sendsync(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register struct sbc_regs *regs = psc->sc_regs;
	register State *state = &psc->st[psc->target];


	PSC_TRACE2("sendsync {per %d, off %d}",
		psc->min_period, psc->max_scsi_offset);

	/* send the extended synchronous negotiation message */
	regs->psc_fifo = SCSI_EXTENDED_MESSAGE;
	eieio();
	regs->psc_fifo = 3;
	eieio();
	regs->psc_fifo = SCSI_SYNC_XFER_REQUEST;
	eieio();
	regs->psc_fifo = psc_to_scsi_period(psc->min_period, psc->clk);
	eieio();
	regs->psc_fifo = psc->max_scsi_offset;
	/* state to resume after we see the sync reply message */
	state->script = psc->script + 2;
	state->msglen = 0;
	state->msg_sent = SCSI_SYNC_XFER_REQUEST;

	return (1);
}

/* ARGSUSED */
static int
psc_replysync(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register struct sbc_regs *regs = psc->sc_regs;
	register State *state = &psc->st[psc->target];
	target_info_t		*tgt = psc->cmd[psc->target];


	PSC_TRACE2("replysync {per %d, off %d}",
		tgt->sync_period, tgt->sync_offset);

	/* send synchronous transfer in response to a request */
	regs->psc_fifo = SCSI_EXTENDED_MESSAGE; eieio();
	regs->psc_fifo = 3; eieio();
	regs->psc_fifo = SCSI_SYNC_XFER_REQUEST; eieio();
	regs->psc_fifo = psc_to_scsi_period(tgt->sync_period, psc->clk); eieio();
	regs->psc_fifo = tgt->sync_offset; eieio();
	regs->psc_cmd = PSC_CMD_XFER_INFO;
	readback(regs->psc_cmd);

	state->msg_sent = SCSI_SYNC_XFER_REQUEST;

	/* return to the appropriate script */
	if (!state->script) {
#if	PSC_DEBUG
		psc_DumpLog("Reply Sync Error");
#endif
		panic("SCSI Error - NIL Script for reply sync");
	}
	psc->script = state->script;
	state->script = (script_t *)0;

	return (0);
}

/* ARGSUSED */
static int
psc_msg_in(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register struct sbc_regs *regs = psc->sc_regs;
	register State *state = &psc->st[psc->target];
	target_info_t	*tgt = psc->cmd[psc->target];
	register int msg;
	int i;


	/* read one message byte */
	msg = regs->psc_fifo; eieio();

	PSC_TRACE2("msg_in {msg %x, len %d}",
		msg, state->msglen);

	/* check for multi-byte message */
	if (state->msglen != 0) {
		/* first byte is the message length */
		if (state->msglen < 0) {
			state->msglen = msg;
			return (1);
		}
		if (state->msgcnt >= state->msglen)
			goto abort;
		state->msg_in[state->msgcnt++] = msg;

		/* did we just read the last byte of the message? */
		if (state->msgcnt != state->msglen) {
			return (1);
		}

		/* process an extended message */
		switch (state->msg_in[0]) {
		case SCSI_SYNC_XFER_REQUEST:
			tgt->flags |= TGT_DID_SYNCH;
			tgt->sync_offset = state->msg_in[2];

			/* convert SCSI period to PSC period */
			i = state->msg_in[1];

			tgt->sync_period = scsi_period_to_psc(i, psc->clk, psc->min_period);

			/*
			 * If this is a request, check minimums and
			 * send back an acknowledge.
			 */
			if (!(tgt->flags & TGT_TRY_SYNCH)) {
				regs->psc_cmd = PSC_CMD_SET_ATN;
				readback(regs->psc_cmd);

				if (tgt->sync_period < psc->min_period)
					tgt->sync_period =
						psc->min_period;

				if (tgt->sync_offset > psc->max_scsi_offset)
					tgt->sync_offset = psc->max_scsi_offset;

				/* Is device allowed to run sync? */
				if ((state->device_flags & DEV_RUN_SYNC) == 0)
					tgt->sync_offset = 0;

				psc->script = &psc_scripts[SCRIPT_REPLY_SYNC];
				regs->psc_syn_p = tgt->sync_period;
				readback(regs->psc_syn_p);
				regs->psc_syn_o = tgt->sync_offset;
				readback(regs->psc_syn_o);
				regs->psc_cmd = PSC_CMD_MSG_ACPT;
				readback(regs->psc_cmd);
				return (0);
			}

			regs->psc_syn_p = tgt->sync_period;
			readback(regs->psc_syn_p);
			regs->psc_syn_o = tgt->sync_offset;
			readback(regs->psc_syn_o);
			tgt->flags &= ~TGT_TRY_SYNCH;
			goto done;

		default:
			printf("SCSI Bus %d target %d: rejecting extended message 0x%x\n",
				psc->unit, psc->target,
				state->msg_in[0]);
			goto reject;
		}
	}

	/* process first byte of a message */

	switch (msg) {
	case SCSI_MESSAGE_REJECT:
		switch (state->msg_sent) {
		case	SCSI_SYNC_XFER_REQUEST:
			tgt->flags |= TGT_DID_SYNCH;	/* No sync transfers */
			tgt->flags &= ~TGT_TRY_SYNCH;
			state->device_flags &= ~DEV_RUN_SYNC;
			tgt->sync_offset = 0;
			break;

		case	SCSI_IFY_ENABLE_DISCONNECT:
			state->device_flags &= ~DEV_NO_DISCONNECT;
			break;

		}
		break;

	case SCSI_EXTENDED_MESSAGE: /* read an extended message */
		/* setup to read message length next */
		state->msglen = -1;
		state->msgcnt = 0;
		return (1);

	case SCSI_NOP:
		break;

	case SCSI_SAVE_DATA_POINTER:
		state->saved_buflen = state->buflen;
		state->saved_sg_index  = state->sg_index;
		/* expect another message (Disconnect) */
		return (1);

	case SCSI_RESTORE_POINTERS:
		state->buflen = state->saved_buflen;
		state->sg_index = state->saved_sg_index;
		break;

	case SCSI_DISCONNECT:
		if (state->flags & STATE_DISCONNECTED)
			goto abort;
		state->flags |= STATE_DISCONNECTED;
		regs->psc_cmd = PSC_CMD_MSG_ACPT;
		readback(regs->psc_cmd);
		psc->script = &psc_scripts[SCRIPT_STATE_DISCONNECT];
		return (0);

	default:
		printf(" SCSI Bus %d target %d: rejecting message 0x%x\n",
			psc->unit, psc->target, msg);
	reject:
		/* request a message out before acknowledging this message */
		state->msg_out = SCSI_MESSAGE_REJECT;
		regs->psc_cmd = PSC_CMD_SET_ATN;
		readback(regs->psc_cmd);
	}

done:
	/* return to original script */
	regs->psc_cmd = PSC_CMD_MSG_ACPT;
	readback(regs->psc_cmd);
	if (!state->script) {
	abort:
#if PSC_DEBUG
		psc_DumpLog("Message In Error");
#endif
		panic("SCSI Error - NIL Script in Msg-In");
	}
	psc->script = state->script;
	state->script = (script_t *)0;
	return (0);
}

/* ARGSUSED */
static int
psc_disconnect(psc, status, ss, ir)
	register psc_softc_t psc;
	register int status, ss, ir;
{
	register State *state = &psc->st[psc->target];


	PSC_TRACE("disconnect");
#ifdef DIAGNOSTIC
	if (!(state->flags & STATE_DISCONNECTED)) {
		printf("psc_disconnect: device %d: STATE_DISCONNECTED not set!\n",
			psc->target);
	}
#endif /*DIAGNOSTIC*/
	psc->target = -1;

	psc->state = PSC_STATE_RESEL;

	psc_flush_fifo(psc);

	if (!queue_empty(&psc->target_queue)) {
		psc->sc_regs->psc_cmd = PSC_CMD_ENABLE_SEL;
		readback(psc->sc_regs->psc_cmd);

		psc_start_new_request(psc);
		return 0;
	} else {
		psc->state = PSC_STATE_RESEL;
		return (1);
	}
}

static void 
psc_load_fifo(psc_softc_t psc, unsigned char * buf, int count)
{
	struct sbc_regs	*regs = psc->sc_regs;


	while(count-- > 0) {
		regs->psc_fifo = *buf++; eieio();
	}

	return;
}

int
psc_wait(
	struct sbc_regs	*regs,
	int			until)
{
	int timeo = 1000000;
	unsigned char 	value;


	while ((regs->psc_csr & until) == 0) {
		eieio();
		delay(1);
		if (!timeo--) {
			printf("psc_wait TIMEO with x%x\n", regs->psc_csr);
			break;
		}
	}
	value = regs->psc_csr; eieio();

	return value;
}


/*
 * Watchdog
 *
 * So far I have only seen the chip get stuck in a
 * select/reselect conflict: the reselection did
 * win and the interrupt register showed it but..
 * no interrupt was generated.
 * But we know that some (name withdrawn) disks get
 * stuck in the middle of dma phases...
 */
int
psc_reset_scsibus(
	register psc_softc_t	psc)
{
	register target_info_t		*tgt;
	register struct sbc_regs	*regs;
	register int			ir;

	simple_lock(&psc->lock);

	tgt = psc->cmd[psc->target];
	regs = psc->sc_regs;

#if PSC_DEBUG
	psc_DumpLog("Request Timeout");
	printf("SCSI Timeout - cmd %x csr %x ",
		regs->psc_cmd, regs->psc_csr);
	ir = regs->psc_intr; eieio();
	printf(" ir %x\n", ir);
#endif
		
	regs->psc_cmd = PSC_CMD_BUS_RESET; eieio();
	delay(35);

	simple_unlock(&psc->lock);
}

void 
psc_construct_entry(State *st, vm_offset_t address, long count, long offset)
{
	struct sg_segment *sgp = &st->sg_list[st->sg_segment_count],
			  *prev_sgp;
	vm_offset_t	physaddr;
	long	real_count;
	int	i;


	if ((i = st->sg_segment_count) != 0)
		prev_sgp = &st->sg_list[i-1];
	else
		prev_sgp = NULL;

	for (i = st->sg_segment_count; i < PSC_MAX_SG_SEGS && count > 0;) {
		physaddr = kvtophys(address);

		if (physaddr == 0) {
			printf("psc_construct_entry(0x%x,0x%x,0x%x,0x%x)\n",
			       (unsigned int)st, address, count, offset);
			printf("kvtophys(0x%x)=0x%x\n",address,physaddr);
			printf("real_count=0x%x, i=0x%x\n",real_count,i);
			for (i = 0; i < 10; i++) {
				printf("%d : kvtophys = 0x%x\n",
				       kvtophys(address));
				delay(1);
			}

			panic("SCSI DMA - Zero physical address");
		}

		real_count = 0x1000 - (physaddr & 0xfff);

		real_count = u_min(real_count, count);

		/*
		 * Check to see if this address and count
		 * can be combined with the previous 
		 * S/G element. This helps to reduce the
		 * number of interrupts.
		 */

		if (prev_sgp && (prev_sgp->sg_physaddr+prev_sgp->sg_count) == physaddr) {
			prev_sgp->sg_count += real_count;

			if (prev_sgp->sg_count & 0x7)
				st->flags |= STATE_FIFO_XFER;
		} else {
			if (physaddr & 0x7 || real_count & 0x7)
				st->flags |= STATE_FIFO_XFER;

			sgp->sg_offset = offset;
			sgp->sg_count = real_count;
			sgp->sg_physaddr = physaddr;
			sgp->sg_vmaddr = (unsigned char *) address;
			prev_sgp =  sgp++;
			i++;
		}

		count -= real_count;
		offset += real_count;
		address += real_count;
	}

	if (count)
		panic("SCSI - S/G overflow.");

	st->sg_segment_count = i;

}

void
psc_build_sglist(State *st, target_info_t *tgt)
{
	io_req_t	ior;
	int	i;
	struct sg_segment	*sg = st->sg_list;
	unsigned long offset = 0, count;


	st->sg_segment_count = 0;
	st->sg_index = 0;

	if ((st->flags & STATE_CHAINED_IO) == 0) {
		psc_construct_entry(st, (vm_offset_t) st->dma_area, st->buflen, 0);
		return;
	}

	ior = tgt->ior;

	for (ior = tgt->ior; ior; ior = ior->io_link) {
		count = ior->io_count;

		if (ior->io_link)
			count -= ior->io_link->io_count;

		psc_construct_entry(st, (vm_offset_t) ior->io_data, count, offset);
		offset += count;
	}

}

/*
 * psc_get_io
 *
 * Get the next physical address and count to transfer. There are
 * several things happening here, if the request is to 
 * be polled, or if the address and/or count prevent from
 * doing a DMA transfer, then the address is returned as
 * a virtual address, and the count is limited to 16 bytes
 * or less.
 */

static boolean_t
psc_get_io(psc_softc_t psc, State *st, unsigned char **address,
		unsigned long *count, boolean_t has_to_poll)
{
	long offset, xfer;
	struct sg_segment	*sgp;
	boolean_t		can_dma_transfer = TRUE;


	*count = st->buflen;
	offset = st->total_count - *count;

	sgp = &st->sg_list[st->sg_index];

	if (offset >= sgp->sg_offset
	&& offset < (sgp->sg_offset + sgp->sg_count)) 
		goto done;

	if (offset > sgp->sg_offset) {
		sgp++;
		st->sg_index++;

		if (st->sg_index >= st->sg_segment_count)  {
#if PSC_DEBUG
			psc_DumpLog("SCSI DMA error");
#endif
			panic("SCSI DMA error : sg_index > sg_segment_count");
		}

		if (offset >= sgp->sg_offset
		&& offset < (sgp->sg_offset + sgp->sg_count)) 
			goto done;

#if PSC_DEBUG
		psc_DumpLog("SCSI DMA Error");
#endif
		printf("Current offset %d, s/g item offset %d\n",
			offset, sgp->sg_offset);
		panic("SCSI Error : DMA Offset Error");
	} else {
		printf("New offset is less than current offset??! Current %d, sg %d\n",
			offset, sgp->sg_offset);
			
		panic("SCSI Error: DMA Offset error");
	}

done:
	offset -= sgp->sg_offset;
	xfer = sgp->sg_count - offset;

	/* First, make sure count is within 
	 * the segment, then check to see
	 * if the count is within the
	 * max. xfer count for the chip per
	 * DMA transaction.
	 */
	if (*count > xfer)
		*count = xfer;

	if (*count > psc->max_transfer)
		*count = psc->max_transfer;

	if (has_to_poll || st->flags & STATE_FIFO_XFER) {
		*address = sgp->sg_vmaddr + offset;
		can_dma_transfer = FALSE;
	} else {
		*address = (unsigned char *) sgp->sg_physaddr + offset;

		/* Check to make sure the address and count
		 * can be used to do a DMA transfer.
		 */
		
		if ((unsigned long) *address & 0x7 || *count & 0x7) {
			*address = sgp->sg_vmaddr + offset;
			can_dma_transfer = FALSE;

		}
	}

	/* If this is to be a polled request, then
	 * limit the count to 16 (max size of the fifo)
	 */

	if (can_dma_transfer == FALSE && *count > 16)
		*count = 16;

	PSC_TRACE5("Data {Phys %x Addr %x, S/G Count %d Xfer Count %d, can dma? %d}", sgp->sg_physaddr, *address, sgp->sg_count, *count, can_dma_transfer);


	return can_dma_transfer;
}

void
psc_sync_cache_data(target_info_t *tgt, State *state)
{
	io_req_t ior, next_ior;
	long	 count;

	if (powermac_info.io_coherent)
		return;

	if ((state->flags  & STATE_CHAINED_IO) == 0)  {
		if (state->flags & STATE_DMA_IN) {
			invalidate_cache_for_io((vm_offset_t) state->dma_area,
					state->total_count, FALSE);
		} else {
			flush_dcache((vm_offset_t)state->dma_area,
				     state->total_count,
				     FALSE);
		}
		return;
	}

	for (ior = tgt->ior; ior; ior = next_ior) {
		count = ior->io_count;

		if ((next_ior = ior->io_link) != NULL)
			count -= next_ior->io_count;

		if (state->flags & STATE_DMA_IN) {
			/* Make sure we're cacheline aligned, if we're not
			 * then we need to flush first and last lines, not
			 * invalidate them
			 */
			invalidate_cache_for_io((vm_offset_t) ior->io_data,
					count, FALSE);
		} else {
			flush_dcache((vm_offset_t)ior->io_data,
				     count, FALSE);
		}
	}

}


#ifdef PSC_DEBUG
static int psc_trace_sequence = 0;

static void
psc_print_log(struct psc_log *lp)
{
	if (lp->msg) {
		printf("%d %d:%d ", lp->sequence, lp->unit, lp->target);
		printf(lp->msg, lp->args[0], lp->args[1], lp->args[2],
				lp->args[3], lp->args[4]);
		printf("\n");
	}
}

psc_DumpLog(str)
	char *str;
{
	register struct psc_log *lp;


	printf("*** Dumping SCSI Trace Log. Reason: %s\n", str);

	lp = psc_logp;
	do {
		psc_print_log(lp);
		if (++lp >= &psc_log[NLOG])
			lp = psc_log;
	} while (lp != psc_logp);
	printf("**** Please try to submit the SCSI Trace Log\n");
	printf("**** to mklinux-bugs@globegate.utm.edu. Thank you.\n\n");

}

static void
psc_trace_log(psc_softc_t psc, char *msg, u_long a1, u_long a2,
		u_long a3, u_long a4, u_long a5)
{
	struct psc_log	*lp = psc_logp;


	lp->unit = psc->unit;
	lp->target = psc->target;
	lp->sequence = psc_trace_sequence++;
	lp->msg = msg;
	lp->args[0] = a1;
	lp->args[1] = a2;
	lp->args[2] = a3;
	lp->args[3] = a4;
	lp->args[4] = a5;

	if (psc_trace_debug)
		psc_print_log(lp);

	lp++;
	if (lp >= &psc_log[NLOG])
		lp = psc_log;

	psc_logp = lp;

}

#endif /*PSC_DEBUG*/

#endif // 0 @@@

#endif	/* NPSC > 0 */
