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
 * 
 */
/*
 * MkLinux
 */
/*
 *
 * Support for second HEATHROW (Gatwick)
 *
 * Exactly like interrupt_pci (Grand Central/OHare), except 
 * that it supports 64-bits worth of intererupts. ACK!
 *
 */

#include <mach_kgdb.h>
#include <mach_kdb.h>
#include <debug.h>
#include <mach_mp_debug.h>
#include <mach_debug.h>
#include <kern/misc_protos.h>
#include <kern/assert.h>

#include <kgdb/gdb_defs.h>
#include <kgdb/kgdb_defs.h>	/* For kgdb_printf */

#include <ppc/spl.h>
#include <ppc/proc_reg.h>
#include <ppc/misc_protos.h>
#include <ppc/trap.h>
#include <ppc/exception.h>

extern int		io_size2;		/* IO region 2 */
extern vm_offset_t	io_base_phys2;		/* IO region 2 */
extern vm_offset_t	io_base_virt2;		/* IO region 2 */
extern caddr_t		pci_via_base_phys;
extern caddr_t		pci_viag_base_phys;

vm_offset_t	gatwick_base_phys;
vm_offset_t	gatwick_base_virt;

#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/interrupts.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/POWERMAC/device_tree.h>
#include <ppc/endian.h>

/* PDM interrupt support is very limited, a certain amount of
 * masking can be done using the ICR but we don't do this yet.
 * SPLs are currently all or nothing.
 */


void gatwick_interrupt( int type, void *ssp);
// static void	gatwick_interrupt(int type,
// 			struct ppc_saved_state *ssp,
// 			unsigned int dsisr, unsigned int dar, spl_t old_lvl);
void	gatwick_register_ofint(int ofirq, spl_t level,
				void (*handler)(int, void *));
void pmac_fix_gatwick_interrupts(struct device_node *gw, int irq_base);
spl_t gatwick_spl(spl_t lvl, spl_t old_level);
extern device_node_t *ofw_node_list;

static void gatwick_add_spl(int level, unsigned long mask, unsigned long mask2);

/* Layout of the Grand Central (and derived) interrupt controller */

struct gatwick_int_controller {
	volatile unsigned long	events2;
	volatile unsigned long	mask2;
	volatile unsigned long	clear2;
	volatile unsigned long	levels2;
	volatile unsigned long	events;
	volatile unsigned long	mask;
	volatile unsigned long	clear;
	volatile unsigned long	levels;
} *gatwick_ints;

int machine_has_gatwick = 0;

/* Two identical functions with different names to allow single stepping
 * of pdm_spl whilst in kdb. Define contents in a macro so that it stays
 * identical!
 */

#if 0
#define SPL_FUNCTION(NAME)						\
static spl_t NAME(spl_t lvl, spl_t old_level);				\
spl_t NAME(spl_t lvl, spl_t old_level) {				\
									\
	lvl = lvl & ~SPL_LOWER_MASK;					\
	if (lvl == SPLLO) {						\
		cpu_data[cpu_number()].interrupt_level = lvl;		\
		interrupt_enable();					\
	} else {							\
		interrupt_disable();					\
		cpu_data[cpu_number()].interrupt_level = lvl;		\
	}								\
									\
	/* Make sure that the write completes before we continue */	\
									\
	sync();								\
									\
	return old_level;						\
}
#else
#define SPL_FUNCTION(NAME)						\
static spl_t NAME(spl_t lvl, spl_t old_level);				\
spl_t NAME(spl_t lvl, spl_t old_level) {				\
									\
	lvl = lvl & ~SPL_LOWER_MASK;					\
	if (lvl == SPLLO) {						\
		cpu_data[cpu_number()].interrupt_level = lvl;		\
	} else {							\
		cpu_data[cpu_number()].interrupt_level = lvl;		\
	}								\
									\
	/* Make sure that the write completes before we continue */	\
									\
	sync();								\
									\
	return old_level;						\
}


#endif

#if (SPLLO < SPLHI) || (SPLOFF != 0)
Yikes! The SPL values changed! Gotta fix em!
#endif
#if	MACH_MP_DEBUG
/* when debugging we let clock ticks through at splvm in order to catch
 * any deadlocks, however the rtclock code just checks for deadlock
 * without doing a tick if we are at splvm with MACH_MP_DEBUG
 */
#define	SPL_IRQS_DISABLED	SPLCLOCK
#else	/* MACH_MP_DEBUG */
/* SPLVM is at the same level as SPLCLOCK (and SPLHIGH) normally */
#define SPL_IRQS_DISABLED	SPLVM
#endif	/* MACH_MP_DEBUG */

static unsigned long gatwick_spl_mask[2][SPLLO+1];
static unsigned long gatwick_irq_enable[2] = {0,0}; /* Do we care? */
static unsigned long gatwick_pending_irqs[2] = {0,0};

static unsigned long gatwick_soft_desired_mask[2];
static unsigned long gatwick_soft_current_mask[2];

#if DEBUG
vm_offset_t spl_addr[NCPUS];
extern boolean_t check_spl_inversion;
int gatwick_in_interrupt[NCPUS];
#endif

spl_t gatwick_spl(spl_t ext_lvl, spl_t ext_old_level)
{
	unsigned long events, flags;
	int mycpu;
	int lowering;
	spl_t lvl;
	spl_t old_level;

	lvl=ext_lvl;
	old_level=ext_old_level;

	// interrupt_disable();

	mycpu = cpu_number();

	lowering = lvl & SPL_LOWER_MASK;
	lvl = lvl & ~SPL_LOWER_MASK;

#if DEBUG
	if ((gatwick_in_interrupt[mycpu] != 0) && old_level > SPLHIGH) {
		printf("Gatwick In interrupt with old_level != SPLHIGH, "
			"lvl=%d, old_level=%d\n", lvl, old_level);
		Debugger("BOGUS OLD_LEVEL");
	}		

	/* Only start checking SPL inversions after we have
	 * really started running with all interrupts enabled,
	 * not during kernel setup
	 */
	if (lvl == SPLLO) {
		check_spl_inversion = TRUE;
	}
	check_spl_inversion = 0; // dg -- temp
	if (check_spl_inversion &&
	    gatwick_in_interrupt[mycpu] == 0 &&
	    SPL_CMP_GT(lvl, old_level) && !lowering) {
		printf("Gatwick SPL inversion from %d at 0x%08x to %d at 0x%08x\n",
		       old_level, spl_addr[mycpu],
		       lvl, (unsigned long) __builtin_return_address(0));
		Debugger("SPL INVERSION");
	}
	if (!lowering) {
	    spl_addr[mycpu] = (unsigned long)__builtin_return_address(0);
	}
#endif
	/* Only open up new interrupts if we asked to lower spl level */
	if (!lowering && (lvl > old_level))
		lvl = old_level;

#if DEBUG
	if (gatwick_in_interrupt[mycpu] && lvl > SPLHIGH) {
		Debugger("Gatwick Should be at SPLHIGH whilst in interrupt\n");
	}
#endif
	cpu_data[mycpu].interrupt_level = lvl; sync();

	/*
	 * Try to avoid touching hardware registers. We do not disable
	 * interrupts when calling spl, we simply set a variable
	 * to indicate that they should not occur. The interrupt handler
	 * will then ignore the interrupt and mask it off if it does
	 * occur. This is because the icr control register costs a LOT
	 * to access.
	 */

	if (mycpu == master_cpu) {
		if (lvl > SPL_IRQS_DISABLED) {

			gatwick_soft_desired_mask[0] = gatwick_spl_mask[0][lvl];
			gatwick_soft_desired_mask[1] = gatwick_spl_mask[1][lvl];

			/* Were we not being lazy about the interrupt
			 * masking, we should change the hardware mask here
			 */

			if (old_level < lvl) {
				/* Only checked for missed interrupts
				 * when SPL is being lowered 
				 * (this a g/c bug since an interrupt
				 * will not be reasserted if it was
				 * previously masked)
				 */

				if ((gatwick_soft_current_mask[0] &
				     gatwick_soft_desired_mask[0]) !=
				    gatwick_soft_desired_mask[0] ||
				  (gatwick_soft_current_mask[1] &
				     gatwick_soft_desired_mask[1]) !=
				    gatwick_soft_desired_mask[1] 
					) {
					
					/* an interrupt is disabled that
					 * should now be enabled - fix it
					 */

					gatwick_soft_current_mask[0] =
						gatwick_soft_desired_mask[0];

					gatwick_soft_current_mask[1] =
						gatwick_soft_desired_mask[1];

					gatwick_ints->mask =
						gatwick_soft_current_mask[0]; eieio();
					gatwick_ints->mask2 =
						gatwick_soft_current_mask[1]; eieio();
				}


#if 0 // crashing?
				gatwick_pending_irqs[0] |= gatwick_ints->levels & gatwick_irq_enable[0];
				eieio();

				gatwick_pending_irqs[1] |= gatwick_ints->levels2 & gatwick_irq_enable[1];
				eieio();
#endif

				if (gatwick_pending_irqs[0] &
				    gatwick_soft_desired_mask[0]
				|| gatwick_pending_irqs[1] & gatwick_soft_desired_mask[1]) {
			    
					/* This causes us to
					 * fake a T_INTERRUPT
					 * exception HARDWARE
					 * INTERRUPTS MUST BE
					 * DISABLED when
					 * entering this
					 * routine
					 */
					create_fake_interrupt();
				}
			}
			// interrupt_enable();
		}
	} else {
		/* We are not on the master CPU, can only do OFF/ON */
		if (lvl > SPL_IRQS_DISABLED) {
			// interrupt_enable();
		}
	}
	
	return old_level;
}

#if	MACH_KDB
SPL_FUNCTION(gatwick_db_spl)
#endif	/* MACH_KDB */

static void	gatwick_via1_interrupt (int device, void *ssp);

#define NVIA1_INTERRUPTS 7
struct powermac_interrupt gatwick_via1_interrupts[NVIA1_INTERRUPTS] = {
	{ 0,	0,	-1},			/* Cascade */
	{ 0,	0,	-1},
	{ 0,	0,	-1},
	{ 0,	0, 	-1},
	{ 0,	0, 	-1},
	{ 0,	0,	-1},
	{ 0,	0,	-1}
};

#define NPCI_INTERRUPTS 32

/* This structure is little-endian formatted... */

struct powermac_interrupt  powermac_gatwick_interrupts[NPCI_INTERRUPTS] = {
	{ 0,	0, -1},				/* Bit 88  - */
	{ 0,	0, -1},				/* Bit 89  - */
	{ 0,	0, -1},				/* Bit 90  - */
	{ 0,	0, -1},				/* Bit 91  - */
	{ 0,	0, -1},				/* Bit 92  - */
	{ 0,	0, PMAC_DEV_ATA0},		/* Bit 93  - media_bay */
	{ 0,	0, -1},				/* Bit 94  - */
	{ 0,	0, -1},				/* Bit 95  - */
	{ 0,	0, PMAC_DEV_ESCC_B},		/* Bit 80  - escc B */
	{ 0,	0, -1},				/* Bit 81  - */
	{ 0,	0, -1},				/* Bit 82  - */
	{ 0,	0, PMAC_DEV_FLOPPY2},		/* Bit 83  - SwimIII/Floppy */
	{ 0,	0, -1},				/* Bit 84  - */
	{ 0,	0, -1},				/* Bit 85  - */
	{ 0,	0, -1},				/* Bit 86  - */
	{ 0,	0, -1},				/* Bit 87  - */
	{ 0,	0, -1},				/* Bit 72  - */
	{ 0,	0, -1},				/* Bit 73  - */
	{ 0,	0, -1},				/* Bit 74  - */
	{ 0,	0, -1},				/* Bit 75  - */
	{ 0,	0, -1},				/* Bit 76  - */
	{ 0,	0, -1},				/* Bit 77  - */
	{ 0,	0, -1},				/* Bit 78  - */
	{ 0,	0, PMAC_DEV_ESCC_A},		/* Bit 79  - escc A */
	{ 0,	0, -1},				/* Bit 64  - */
	{ 0,	0, -1},				/* Bit 65  - SWIM III DMA */
	{ 0,	0, -1},				/* Bit 66  - */
	{ 0,	0, -1},				/* Bit 67  - */
	{ 0,	0, -1},				/* Bit 68  - */
	{ 0,	0, -1},				/* Bit 69  - */
	{ 0,	0, -1},				/* Bit 70  - */
	{ 0,	0, -1}				/* Bit 71  - */
};


struct	powermac_interrupt	powermac_gatwick_extended_interrupts[NPCI_INTERRUPTS];
int gatwick_register_int(int device, spl_t level, void (*handler)(int, void *));
void gatwick_interrupt_initialize(void);


/* Reset the hardware interrupt control */
void
gatwick_interrupt_initialize(void)
{
device_node_t	*dev;
int i;

	/* Does second heathrow exist? */
	machine_has_gatwick = 0;

	if ((dev=find_devices("mac-io"))==NULL) {
#if MACH_DEBUG
		printf("HEY!  There's not a Heathrow here!?!\n");
#endif
		return;
	}

	if (dev->next)
		dev=dev->next;
	else return;

#if 0
	else if (dev->child && !strcmp(dev->child->type, "mac-io"))
		dev=dev->child;
	else if (dev->sibling && !strcmp(dev->sibling->type, "mac-io"))
		dev=dev->sibling;
	else {
		dev=dev->allnext;
		while (dev) {
			if (!strcmp(dev->type,"mac-io")) break;
			dev=dev->allnext;
		}
	}
#endif

	if (dev) {
		if (!dev->n_addrs) return; /* can't find it, so don't reset it */
		if (!dev->n_intrs) return; /* can't register it, so don't reset it */

#if MACH_DEBUG
		if (io_size2!=PCI_IO_SIZE) {
			printf("size mismatch: %d %d!\n", io_size2, PCI_IO_SIZE);
			io_size2 = PCI_IO_SIZE;
			}
		printf("%x %x %d\n", io_base_phys2, io_base_virt2, io_size2);

		printf("Resetting Gatwick...");
#endif

		machine_has_gatwick = 1;
		pci_viag_base_phys = (caddr_t)(dev->addrs[0].address);

#if MACH_DEBUG
		printf("assignment...");
#endif
		gatwick_ints = (struct gatwick_int_controller *) POWERMAC_IO2(dev->addrs[0].address + 0x10);

#if MACH_DEBUG
		printf("address: %x, POWERMAC_IO: %x...", dev->addrs[0].address, POWERMAC_IO2(dev->addrs[0].address));
#if 0
		if (dev->addrs[0].address!=0xf4000010)
			gatwick_ints = (struct gatwick_int_controller *) POWERMAC_IO(0xf4000010);
#endif

		printf("mask=0...");
#endif

		gatwick_ints->mask  = 0;	   /* Disable all interrupts */
		eieio();

#if MACH_DEBUG
		printf("clear=ffffffff...");
#endif

		gatwick_ints->clear = 0xffffffff; /* Clear pending interrupts */
		eieio();

#if MACH_DEBUG
		printf("mask=0...");
#endif

		gatwick_ints->mask  = 0;	   /* Disable all interrupts */
		eieio();

#if MACH_DEBUG
		printf("mask2=0...");
#endif

		gatwick_ints->mask2  = 0;	   /* Disable all interrupts */
		eieio();

#if MACH_DEBUG
		printf("clear2=ffffffff...");
#endif

		gatwick_ints->clear2 = 0xffffffff; /* Clear pending interrupts */
		eieio();

#if MACH_DEBUG
		printf("mask2=0...");
#endif

		gatwick_ints->mask2 = 0;	   /* Disable all interrupts */
		eieio();


		/* Now let's start listening for interrupts using the first
		   heathrow's daisy-chain input... */

		// pmac_register_int(PMAC_DEV_CARD7, SPLHIGH, gatwick_interrupt);



		// *(v_u_char *)PCI_VIA1G_PCR	= 0x00; eieio();
		// *(v_u_char *)PCI_VIA1G_IER	= 0x7F; eieio();
		// *(v_u_char *)PCI_VIA1G_IFR	= 0x7F; eieio();



		// for (i=0; i < dev->n_intrs; i++) {
			// pmac_register_ofint(dev->intrs[i], SPLBIO, (void(*)(int,void *))gatwick_interrupt);
			// }

#if MACH_DEBUG
		printf("registering...");
#endif

		pmac_register_ofint(dev->intrs[0], SPLBIO, (void(*)(int,void *))gatwick_interrupt);

		printf("Gatwick interrupt controller at Heathrow interrupt %d.\n",dev->intrs[0]);
		printf("Fixing interrupts.\n");

		pmac_fix_gatwick_interrupts(dev, 64);

#if MACH_DEBUG
		printf("returning.\n");
#endif

		}
	else {
		int count;
		/* only one Heathrow, so go home.... */
#if 0
		count = 0;
		for (dev=ofw_node_list; dev; dev=dev->allnext) {
			printf("device %s: type %s: node %d\n",dev->name, dev->type, count++);
			if (!((count+1)%20)) delay(1500000); // 15 sec.
		}
#endif

		return;
	}

}

/*
 * findbit(n) => b
 *
 * Use the PowerPC cntlzw instruction to quickly locate the position
 * of the most significant bit of those that are set in a word.
 *
 * For non-zero n, compute b such that (n >> b) == 1.
 * [For zero n, there is no solution; return -1]
 *
 * Alternately: max b such that (1<<b & n) != 0.
 */

static __inline__ long
findbit(unsigned long bit)
{
	long	result;

	/*
	 * Given bit, compute result to be number of left shifts
	 * required to set the high bit of bit.
	 */

	__asm__ volatile ("cntlzw	%0,%1"  : "=r" (result) : "r" (bit));

	/*
	 * 0 shifts => bit 31.  
	 * 31 shifts => bit 0.
	 */

	return (31-result);
}

static int
gatwick_find_entry(int device,
	       struct powermac_interrupt **handler,
	       int nentries)
{
	int	i;

	for (i = 0; i < nentries; i++, (*handler)++)
		if ( (*handler)->i_device == device)
			return i;

	*handler = NULL;
	return 0;
}

static void
gatwick_add_spl(int level, unsigned long mask0, unsigned long mask1)
{
	int	i;

	{
	    unsigned long p0 = gatwick_pending_irqs[0];
	    unsigned long p1 = gatwick_pending_irqs[1];

	    if (p0 & mask0 || p1 & mask1)
		panic("gatwick interrupt already pending: mask0=0x%08x p0=0x%08x mask1=0x%08x p1=0x%08x", mask0, p0, mask1, p1);
	}

	/* Add to bag of enabled interrupts */

	gatwick_irq_enable[0] |= mask0;
	gatwick_irq_enable[1] |= mask1;

	/* Enable (mask0,mask1) in spl levels lower than level */

	for (i = SPLLO; i > level; i--) {
		gatwick_spl_mask[0][i] |= mask0;
		gatwick_spl_mask[1][i] |= mask1;
	}
}

/*
 * Register OpenFirmware interrupt 
 */

void gatwick_register_ofint(int ofirq, spl_t level, void (*handler)(int, void *))
{
	int	i;
	unsigned long	beirq;
	unsigned long 	irq;
	struct powermac_interrupt	*p;
	boolean_t	is_extended;

	if (!machine_has_gatwick) {
		printf("WARNING: Attempt to register gatwick (heathrow2) interrupt on\n");
		printf("machine with only one heathrow!  Interrupt %d\n", ofirq);
		return;
		}

	if (ofirq >= 32) {
		is_extended = TRUE;
		p = powermac_gatwick_extended_interrupts;
		beirq = 1 << (ofirq-32);
	} else {
	        is_extended = FALSE;
		p = powermac_gatwick_interrupts;
		beirq = 1 << ofirq;
	}

	irq = byte_reverse_word(beirq);

	/*
	 * ofirq  0.. 7 : beirq 0x000000zz : irq 0xzz000000 : p += 24..31
	 *        8..15 :	0x0000zz00 : 	 0x00zz0000 :      16..23
	 *       16..23 :	0x00zz0000 : 	 0x0000zz00 :       8..15
	 *       24..31 :	0xzz000000 : 	 0x000000zz :       0.. 7
	 *
	 * Note that for irq, zz = (01,02,04,08,10,20,...), not
	 * high-bit to low-bit.
	 */

	/* XXX Locks? */

	p = &p[findbit(irq)];
	if (p->i_handler)
		panic("gatwick_register_ofint: interrupt %d is already taken! handler=0x%08x\n", ofirq, p->i_handler);
	else {
	        unsigned long mask0, mask1;

		/* Install the handler */

		p->i_handler = handler;
		p->i_level = level;

		/* (mask0, mask1) = is_extended? (0, irq) : (irq, 0) */
		mask0 = is_extended ? 0 : irq;
		mask1 = is_extended ? irq : 0;
		gatwick_add_spl(level, mask0, mask1);
	}
}

int
gatwick_register_int(int device, spl_t level, void (*handler)(int, void *))
{
	int	i;
	struct powermac_interrupt	*p;

#if MACH_DEBUG
	printf("gatwick_register_int called...");
#endif

	/* Check primary interrupts */
	p = powermac_gatwick_interrupts;
	i = gatwick_find_entry(device, &p, NPCI_INTERRUPTS);
	if (p) {
		if (p->i_handler) {
			panic("gatwick_register_int: "
			      "Interrupt %d already taken!? ", device);
		} else {
			p->i_handler = handler;
			p->i_level = level;
			gatwick_add_spl(level, 1<<i, 0);
			return 1;
		}
		return 1;
	}

#if MACH_DEBUG
	printf(" not found in main interrupts...");
#endif

	/* Check cascaded interrupts */
	p = gatwick_via1_interrupts;
	i = gatwick_find_entry(device, &p, NVIA1_INTERRUPTS);
	if (p) {
		if (p->i_handler) {
			panic("gatwick_register_int: "
				"Interrupt %d already taken!? ", device);
		} else {
			p->i_handler = handler;
			p->i_level = level;

			*((v_u_char *) PCI_VIA1_IER) |= (1 << i);
			eieio();

			gatwick_add_spl(level, 1<<10, 0);
			return 1;
		}
		return 1;
	}

#if MACH_DEBUG
	printf(" not found in (Gatwick) via1 interrupts...");
#endif

	// panic("gatwick_register_int: Interrupt %d not found", device);
	return 0;
}


// static void
// gatwick_interrupt( int type, struct ppc_saved_state *ssp,
	       // unsigned int dsisr, unsigned int dar, spl_t old_lvl)
void gatwick_interrupt( int type, void *ssp)
{
	unsigned long	irq, irq2, events, events2;
	long		bit;
	struct powermac_interrupt	*handler;
	spl_t	s;
	int     mycpu;

#if MACH_DEBUG
	printf("gatwick_interrupt called.\n");
#endif

	mycpu = cpu_number();

	/* Only master CPU should deal with device interrupts */
	assert(mycpu == master_cpu);

	events = gatwick_ints->events; eieio();
	events2 = gatwick_ints->events2; eieio();

	irq = events | gatwick_pending_irqs[0];
	irq2 = events2 | gatwick_pending_irqs[1];

	/*
	 * Completely ignore interrupts we know nothing about (stray
	 * interrupt).
	 */

	irq &= gatwick_irq_enable[0];
	irq2 &= gatwick_irq_enable[1];

	if (events & ~gatwick_soft_desired_mask[0]
	|| events2 & ~gatwick_soft_desired_mask[1]) {
		/* received interrupt that should have been masked,
		 * so lazily mask it and pretend it did not happen
		 */
	  	gatwick_soft_current_mask[0] = gatwick_soft_desired_mask[0];
		gatwick_ints->mask = gatwick_soft_current_mask[0]; eieio();

	  	gatwick_soft_current_mask[1] = gatwick_soft_desired_mask[1];
		gatwick_ints->mask2 = gatwick_soft_current_mask[1]; eieio();

		/* mark these interrupts as pending */
		gatwick_pending_irqs[0] |= irq;
		gatwick_pending_irqs[1] |= irq2;
	}

	/* Only deal with interrupts we are meant to have */
	irq = irq & gatwick_soft_desired_mask[0];
	irq2 = irq2 & gatwick_soft_desired_mask[1];

	/* Acknowledge interrupts that we have recieved */
	gatwick_ints->clear = events; eieio();
	gatwick_ints->clear2 = events2; eieio();

	/* mark any interrupts that we deal with as no longer pending */
	gatwick_pending_irqs[0] &= ~irq;
	gatwick_pending_irqs[1] &= ~irq2;

#if DEBUG
	gatwick_in_interrupt[mycpu]++;
#endif /* DEBUG */

	while ((bit = findbit(irq)) >= 0) {
		handler = &powermac_gatwick_interrupts[bit];
		irq &= ~(1<<bit);

		if (handler->i_handler) {
			/*s = gatwick_spl(handler->i_level | SPL_LOWER_MASK);*/
			handler->i_handler(handler->i_device, ssp);
			/*splx(s);*/
		} else {
			printf("GATWICK{PCI INT %d}", bit);
		}
	}

	while ((bit = findbit(irq2)) >= 0) {
		handler = &powermac_gatwick_extended_interrupts[bit];
		irq2 &= ~(1<<bit);

		if (handler->i_handler) {
			/*s = gatwick_spl(handler->i_level | SPL_LOWER_MASK);*/
			handler->i_handler(handler->i_device, ssp);
			/*splx(s);*/
		} else {
			printf("{GATWICK INT %d}", bit);
		}
	}
#if DEBUG
	gatwick_in_interrupt[mycpu]--;
#endif /* DEBUG */
}

void
gatwick_via1_interrupt(int device, void *ssp)
{
	register unsigned char intbits;
	long bit;
	struct powermac_interrupt	*handler;
	spl_t		s;

	intbits = via_reg(PCI_VIA1_IFR); eieio();/* get interrupts pending */
	intbits &= via_reg(PCI_VIA1_IER); eieio();	/* only care about enabled */

	if (intbits == 0)
		return;

	via_reg(PCI_VIA1_IFR) = intbits; eieio();

	while ((bit = findbit((unsigned long)intbits)) > 0) {
		intbits &= ~(1<<bit);

		if (bit >= NVIA1_INTERRUPTS)
			continue;

		handler = &gatwick_via1_interrupts[bit];

		if (handler->i_handler) 
				handler->i_handler(handler->i_device, ssp);
	}
}


/* This routine will fix some missing interrupt values in the device tree
 * on the gatwick mac-io controller used by some PowerBooks
 */
void pmac_fix_gatwick_interrupts(struct device_node *gw, int irq_base)
{
	struct device_node *node, *nodeb;
	static unsigned long int_pool[7];


	node = find_devices("escc");
	if (node && node->next && node->next->n_intrs == 0) {
		node->next->n_intrs = 1;
		node->next->intrs = &int_pool[0];
		int_pool[0] = 15+irq_base;
		printf("irq: fixed SCC on second controller (%d)\n",
			int_pool[0]);
	}

	node = find_devices("media-bay");
	if (node && node->next && node->next->n_intrs == 0) {
		node->next->n_intrs = 1;
		node->next->intrs = &int_pool[1];
		int_pool[1] = 29+irq_base;
		printf("irq: fixed media-bay on second controller (%d)\n",
			int_pool[1]);
	}


	node = find_devices("floppy");
	if (node && node->n_intrs == 0) {
		node->n_intrs = 2;
		node->intrs = &int_pool[2];
		int_pool[2] = 19;
		int_pool[3] =  1;
		printf("irq: fixed floppy on first controller (%d,%d)\n",
			int_pool[2], int_pool[3]);
	}
	if (node && node->next && node->next->n_intrs == 0) {
		node->next->n_intrs = 2;
		node->next->intrs = &int_pool[5];
		int_pool[5] = 19+irq_base;
		int_pool[6] =  1+irq_base;
		printf("irq: fixed floppy on second controller (%d,%d)\n",
			int_pool[5], int_pool[6]);
	}

	node = find_devices("ata4");
	if (node && node->n_intrs) {
		printf("irq: fixed ide on second controller (%d -> ",
			node->intrs[0]);
		node->intrs[0] = (node->intrs[0] + irq_base);
		printf("%d)\n", node->intrs[0]);
	} else if (node) {
		printf("irq: fixed ide on second controller (%d -> ",
			node->intrs[0]);
		node->n_intrs = 1;
		node->intrs = &int_pool[4];
		int_pool[4] = 14 + irq_base;
		printf("%d) : guess\n", node->intrs[0]);
	}
}

