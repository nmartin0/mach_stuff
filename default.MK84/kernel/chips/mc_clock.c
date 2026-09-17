/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	mc_clock.c,v $
 * Revision 2.16  93/11/17  16:12:25  dbg
 * 	Added autoconfiguration structures for PS2.
 * 	[93/09/09            dbg]
 * 	Fixed setting CLOCK_RESOLUTION: mc_setstat was passing an
 * 	uninitialized variable to log2.  Changed mc_setstat to
 * 	pick the available resolution closest to the one requested,
 * 	not just the next higher power of 2.
 * 	[93/08/30            dbg]
 * 
 * 	Probe routine now sets the clock address for all machines.
 * 	[93/07/15            dbg]
 * 
 * 	Century is in a different place on the PS2.
 * 	[93/07/13            dbg]
 * 
 * 	Update clock->check_seconds when reading clock at initialization.
 * 	[93/06/18            dbg]
 * 
 * 	Use routines from device/clock_dev.c.
 * 	[93/06/02            dbg]
 * 
 * 	Changed mmap routines to return physical address instead of
 * 	physical page number.
 * 	[93/05/24            dbg]
 * 
 * 	Adapted for new Mach clocks and timers, based on Stefan Savage's
 * 	code.  Generalized to be usable for i386 machines, also.
 * 	[93/05/21            dbg]
 * 
 * Revision 2.15  93/05/17  15:11:48  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 	[93/05/17            rvb]
 * 
 * Revision 2.14  93/05/15  19:37:12  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.13  93/05/10  20:08:19  rvb
 * 	Fixed types. Define what is exported and what not.
 * 	[93/05/06  09:57:45  af]
 * 
 * Revision 2.12  93/02/05  08:05:43  danner
 * 	Heavy cosmetic changes to make it look like a device after all.
 * 	Flamingo uses it too.
 * 	[93/02/04  01:30:53  af]
 * 
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.11  92/02/19  16:45:57  elf
 * 	Wait for uip before stopping the chip, too.
 * 	[92/01/22            af]
 * 
 * Revision 2.10  91/08/24  11:52:28  af
 * 	Fixed spl munging, for 3min.
 * 	[91/08/02  01:49:09  af]
 * 
 * Revision 2.9  91/06/25  20:54:18  rpd
 * 	Tweaks to make gcc happy.
 * 
 * Revision 2.8  91/06/19  11:53:54  rvb
 * 	mips->DECSTATION; vax->VAXSTATION
 * 	[91/06/12  14:01:54  rvb]
 * 
 * 	File moved here from mips/PMAX since it tries to be generic;
 * 	it is used on the PMAX and could be used on the Vax3100.
 * 	It would need a few ifdef vax & ifdef mips for the latter.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.7  91/05/14  17:24:22  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/14  14:34:41  mrt
 * 	Factored out delay() function, and made it box-indep.
 * 	Added accurate_config_delay() which calls delay()'s
 * 	configuration code.  Modified ackrtclock() to
 * 	invoke it on first call.
 * 	Tell the user what the CPU clock speed loks like, 
 * 	distinguish between DS3100 and DS2100 based on clock speed.
 * 	[91/02/12  13:03:16  af]
 * 
 * Revision 2.5  91/02/05  17:42:35  mrt
 * 	Added author notices
 * 	[91/02/04  11:15:06  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:13:45  mrt]
 * 
 * Revision 2.4  90/12/05  23:32:39  af
 * 
 * 
 * Revision 2.3  90/12/05  20:47:31  af
 * 	Made file conditional.
 * 	[90/12/03  23:28:03  af]
 * 
 * Revision 2.2  90/08/27  22:06:05  dbg
 * 	Initialized cpu_speed to a non-zero value.  Apparently,
 * 	we need delays much earlier than we can possibly know
 * 	what this machine looks like.  I know this will bite back.
 * 	[90/08/21            af]
 * 	Made apparent who is resetting the clock back to 1972.
 * 	[90/08/20  10:30:19  af]
 * 
 * 	Created, from Motorola's MC146818 specs.
 * 	[90/08/17            af]
 * 
 */
/*
 *	File:	mc_clock.c
 *	Author: Alessandro Forin
 *	Date:	8/90
 *
 *	Driver for the MC146818 Clock
 */

#include <mc.h>
#if	NMC > 0
#include <platforms.h>

#include <mach/boolean.h>
#include <mach/time_spec.h>
#include <mach/machine/vm_types.h>

#include <kern/clock.h>
#include <kern/kern_io.h>

#ifdef	PS2
#include <i386ps2/bus.h>		/* PS2 autoconf */
#else
#include <chips/busses.h>		/* standard autoconf */
#endif
#include <device/device_types.h>
#include <device/clock_status.h>
#include <device/clock_dev.h>

#include <machine/machspl.h>

#define	private	static
#define	public

/*
 *	Real-Time Clock plus non-volatile RAM (MC146818)
 */

/*
 *	Registers are accessed by number.
 */
#define	MC_SECOND		0x00
#define	MC_ALARM_SECOND		0x01
#define	MC_MINUTE		0x02
#define	MC_ALARM_MINUTE		0x03
#define	MC_HOUR			0x04
#define	MC_ALARM_HOUR		0x05
#define	MC_DAY_OF_WEEK		0x06	/* range 1..7  */
#define	MC_DAY_OF_MONTH		0x07	/* range 1..31 */
#define	MC_MONTH		0x08
#define	MC_YEAR			0x09
#define	MC_REG_A		0x0a
#define	MC_REG_B		0x0b
#define	MC_REG_C		0x0c
#define	MC_REG_D		0x0d

/*
 * Register A defines (read/write)
 */

#define	MC_REG_A_RS	0x0f		/* Interrupt rate (and SQwave) select */
#define	MC_REG_A_DV	0x70		/* Divider select */
#define	MC_REG_A_UIP	0x80		/* Update In Progress (read-only bit) */

/* Time base configuration */
#define	MC_BASE_4_MHz	0x00
#define	MC_BASE_1_MHz	0x10
#define	MC_BASE_32_KHz	0x20
#define	MC_BASE_NONE	0x60		/* actually, both of these reset */
#define	MC_BASE_RESET	0x70

/* Interrupt rate table */
#define	MC_RATE_NONE	0x0		/* disabled */
#define	MC_RATE_1	0x1		/* 256Hz if MC_BASE_32_KHz,
					   else 32768Hz */
#define	MC_RATE_2	0x2		/* 128Hz if MC_BASE_32_KHz,
					   else 16384Hz */
#define	MC_RATE_8192_Hz	0x3		/* Tpi: 122.070 usecs */
#define	MC_RATE_4096_Hz	0x4		/* Tpi: 244.141 usecs */
#define	MC_RATE_2048_Hz	0x5		/* Tpi: 488.281 usecs */
#define	MC_RATE_1024_Hz	0x6		/* Tpi: 976.562 usecs */
#define	MC_RATE_512_Hz	0x7		/* Tpi: 1.953125 ms */
#define	MC_RATE_256_Hz	0x8		/* Tpi: 3.90625 ms */
#define	MC_RATE_128_Hz	0x9		/* Tpi: 7.8125 ms */
#define	MC_RATE_64_Hz	0xa		/* Tpi: 15.625 ms */
#define	MC_RATE_32_Hz	0xb		/* Tpi: 31.25 ms */
#define	MC_RATE_16_Hz	0xc		/* Tpi: 62.5 ms */
#define	MC_RATE_8_Hz	0xd		/* Tpi: 125 ms */
#define	MC_RATE_4_Hz	0xe		/* Tpi: 250 ms */
#define	MC_RATE_2_Hz	0xf		/* Tpi: 500 ms */

/* Update cycle time */
#define MC_UPD_4_MHz	 248		/* usecs */
#define MC_UPD_1_MHz	 248		/* usecs */
#define MC_UPD_32_KHz	1984		/* usecs */
#define MC_UPD_MINIMUM	 244		/* usecs, guaranteed if UIP=0 */

/*
 * Register B defines (read/write)
 */

#define MC_REG_B_DSE	0x01		/* obsolete (Daylight Savings Enable) */
#define MC_REG_B_24HM	0x02		/* 24/12 Hour Mode */
#define MC_REG_B_DM	0x04		/* Data Mode, 1=Binary 0=BCD */
#define MC_REG_B_SQWE	0x08		/* Square Wave Enable */
#define MC_REG_B_UIE	0x10		/* Update-ended Interrupt Enable */
#define MC_REG_B_AIE	0x20		/* Alarm Interrupt Enable */
#define MC_REG_B_PIE	0x40		/* Periodic Interrupt Enable */
#define MC_REG_B_SET	0x80		/* Set NVram info, e.g.
					   update time or ..*/
#define MC_REG_B_STOP	MC_REG_B_SET	/* Stop updating the timing info */

/*
 * Register C defines (read-only)
 */

#define MC_REG_C_ZEROES	0x0f		/* Reads as zero bits  */
#define MC_REG_C_UF	0x10		/* Update-ended interrupt flag */
#define MC_REG_C_AF	0x20		/* Alarm interrupt flag */
#define MC_REG_C_PF	0x40		/* Periodic interrupt flag */
#define MC_REG_C_IRQF	0x80		/* Interrupt request flag */

/*
 * Register D defines (read-only)
 */

#define MC_REG_D_ZEROES	0x7f		/* Reads as zero bits  */
#define MC_REG_D_VRT	0x80		/* Valid RAM and Time */

/*
 *	Different machines map the chip registers in different
 *	ways.
 */

#ifdef	DECSTATION
#include <mips/mips_cpu.h>

#define	MC_DEFAULT_ADDRESS	PHYS_TO_K1SEG(0x1d000000)
#define	MC_DOES_DELAYS		1

/*
 * Both the Pmax and the 3max implementations of the chip map
 * bytes of the chip's RAM to 32 bit words (low byte).
 * For convenience, we redefine here the chip's RAM layout
 * making padding explicit. 
 */

typedef struct {
	struct {
		volatile unsigned char	real_reg;
		char			pad[3];
	} regs[64];
} * mc_clock_t;

#define	mc_read(clock, reg)	(clock->regs[reg].real_reg)
#define	mc_write(clock, reg, data) \
				(clock->regs[reg].real_reg = (data))

#endif	/* DECSTATION */

#ifdef	FLAMINGO
#define	MC_DEFAULT_ADDRESS	0L

/*
 *	Clock is not padded
 */
typedef struct {
	volatile unsigned char	regs[64];
} * mc_clock_t;

#define	mc_read(clock, reg)	(clock->regs[reg])
#define	mc_write(clock, reg, data) \
				(clock->regs[reg] = (data))

#endif	/* FLAMINGO */

#if	defined(AT386) || defined(PS2)

#include <i386/pio.h>
#define	wbflush()

/*
 *	The RTC is accessed through IO ports.
 */
#define	MC_ADDR		0x70		/* high bit is NMI enable! */
#define	MC_DATA		0x71

/*
 *	Delay is needed between outb and inb.
 */
#ifdef	__GNUC__
#define	spin()	asm volatile(" jmp 0f; 0: ")
#endif

#define	mc_read(clock, reg)	\
	({ outb(MC_ADDR, reg); spin(); inb(MC_DATA);})

#define	mc_write(clock, reg, data) \
	({ outb(MC_ADDR, reg); spin(); outb(MC_DATA, data); (void)0;})

/*
 *	Need typedef anyway.
 */
typedef volatile unsigned char* mc_clock_t;

#define	MC_DEFAULT_ADDRESS	MC_ADDR

/*
 *	We keep time in BCD
 */
#define	MC_CLOCK_BCD	1

/*
 *	And there is a place to put the century (!)
 */
#ifdef	AT386
#define	MC_CENTURY	0x32
#endif
#ifdef	PS2
#define	MC_CENTURY	0x37
#endif

#endif	/* AT386 || PS2 */

/*
 *	We configure the clock to run in 24 hour mode,
 *	and to deliver periodic interrupts that we
 *	use as clock interrupts.
 *	If some other OS wants time to be kept in BCD,
 *	we do that to.
 *	But we keep time in UTC, not local time.
 */

#ifdef	MC_CLOCK_BCD
#define	MC_REG_B_CONFIGURE	(MC_REG_B_24HM)
#else
#define	MC_REG_B_CONFIGURE	(MC_REG_B_24HM | MC_REG_B_DM)
#endif

/*
 *	Minimum and maximum resolution for clock.
 */
#define MC_CLOCK_DEFAULT_RESOLUTION	(NANOSEC_PER_SEC/128)
#define MC_CLOCK_MAX_RESOLUTION		(16000000)
#define MC_CLOCK_MIN_RESOLUTION		(125000)

/*
 *	We have only one per machine.
 */
mach_clock_data_t		mc_clock0;

mach_clock_t			mc_clock[] = {&mc_clock0};

void	mc_clock_setresolution(mach_clock_t);		/* forward */
void	mc_clock_write(mach_clock_t, time_spec_t);
void	mc_clock_enable_interrupts(mach_clock_t);
time_spec_t mc_clock_read(mach_clock_t);
void	mc_wait_for_uip(mc_clock_t clock);

struct clock_ops mc_clock_ops = {
	mc_clock_setresolution,
	mc_clock_write,
	mc_clock_enable_interrupts
};

#ifdef	PS2
/*
 *	PS2 autoconf
 */
int	mc_probe(), mc_attach();
extern	mc_intr();

char *	mc_std[] = { (char *) 0x70, 0 };

struct i386_dev	*mc_info[NMC];
struct i386_driver mcdriver =
	/* probe slave attach    dname  dinfo mname minfo */
	{ mc_probe, 0, mc_attach, "mc", mc_info, 0, 0 };
int (*mcintrs[])() = {mc_intr, 0};

#else	/* PS2 */
/*
 *	Standard Configuration
 */
boolean_t mc_probe(
	vm_offset_t		addr,
	struct bus_device 	*dev);
void mc_attach(
	struct bus_device	*dev);

static vm_offset_t		mc_std[] = { MC_DEFAULT_ADDRESS, 0 };
static struct bus_device	*mc_info[NMC];

struct bus_driver		mc_driver = {
	mc_probe, 0, mc_attach, 0, mc_std, "mc",
	mc_info, 0, 0, 0};

#endif	/* PS2 */

#if	defined(AT386) || defined(PS2)
extern mc_sysintr();		/* MC system clock interrupt */
#endif

/*
 *	Where is the chip`s RAM mapped?
 */
mc_clock_t	rt_clock = (mc_clock_t) MC_DEFAULT_ADDRESS;

/*
 *	status
 */
int		mc_interrupt_enabled = 0;	/* OR with reg B */
boolean_t	mc_new_century = FALSE;		/* patch after Dec 31, 1999 */

/*
 *	Probe to see whether the device exists.
 */
#ifdef	PS2
int mc_probe(addr, dev)
	vm_offset_t	addr;
	struct i386_dev *dev;
#else
boolean_t mc_probe(
	vm_offset_t		addr,
	struct bus_device 	*dev)
#endif
{
#ifdef	PS2
	int	unit = dev->dev_unit;
#else
	int	unit = dev->unit;
#endif

	if (unit < 0 || unit >= NMC) {
	    printf("mc%d out of range\n", unit);
	    return FALSE;
	}

	/*
	 *	We assume that we always have exactly one
	 *	clock chip.
	 *
	 *	Set the clock`s address.
	 */
	rt_clock = (mc_clock_t) addr;
	return TRUE;
}

/*
 *	Attach the clock device:
 *	set up the clock structure and read
 *	the current time from the hardware.
 */
#ifdef	PS2
mc_attach(dev)
	struct i386_dev *dev;
#else
void mc_attach(
	struct bus_device	*dev)
#endif
{
#ifdef	PS2
	int		unit = dev->dev_unit;
#else
	int		unit = dev->unit;
#endif
	mach_clock_t	clock = mc_clock[unit];
	time_spec_t	cur_time;

	printf(": MC146818 (or the like) Time-of-Year chip");

	clock_init(clock, &mc_clock_ops);

	clock->resolution = MC_CLOCK_DEFAULT_RESOLUTION;	/* @128 hz */
	mc_clock_setresolution(clock);

	printf(", resolution = %u nsecs", clock->resolution);

	if (sys_clock == 0) {
	    /*
	     *	If no system clock already, use this as system clock.
	     */
	    sys_clock = &mc_clock0;
#if	defined(AT386) || defined(PS2)
#ifdef	PS2
	    mcintrs[0] = mc_sysintr;
#else
	    dev->intr = mc_sysintr;	/* use system clock interrupt */
#endif
	    dev_change_indirect("clock_priv", "mc", unit);
					/* set generic clock device */
	    set_clock_unpriv();		/* and 'unprivileged' clock */
#endif
	}

#ifdef	AT386
	take_dev_irq(mc_info[0]);	/* XXX */
#endif
#ifdef	PS2
	take_dev_irq(dev);
#endif

	/*
	 *	Read the current time from the chip
	 */
	cur_time = mc_clock_read(clock);
	if (!time_spec_valid(cur_time)) {
	    /*
	     *	Battery dead - no time
	     */
	    printf("\n*** Dead Battery *** Time Unknown ***");
	    clock->check_seconds = 0;
	    clock->time.nanoseconds = 0;
	    clock->time.seconds = 0;
	}
	else {
	    clock->check_seconds = cur_time.seconds;
	    clock->time.nanoseconds = cur_time.nanoseconds;
	    clock->time.seconds = cur_time.seconds;

	    /*
	     *	If the year is less than 1990, someone
	     *	has clobbered the clock.
	     */
	    if (clock->time.seconds < (60*60*24*365) * (1990 - 1970))
		printf("\n*** The PROM has clobbered the clock ***");
	}
}

/*
 *	Enable interrupts from MC clock chip
 */
void mc_clock_enable_interrupts(
	mach_clock_t	clock)
{
	mc_interrupt_enabled = MC_REG_B_PIE;
	mc_clock_setresolution(clock);	/* turns on interrupts */
}

io_return_t mc_open(
	int	dev)
{
	if (dev < 0 || dev >= NMC)
	    return D_NO_SUCH_DEVICE;

	return clock_open(mc_clock[dev]);
}

io_return_t mc_close(
	int	dev)
{
	return D_SUCCESS;
}

io_return_t mc_getstat(
	int		dev,
	int	       	flavor,
	dev_status_t	stat,
	natural_t	*count)
{
	return clock_getstat(mc_clock[dev], flavor, stat, count);
}

io_return_t mc_setstat(
	int		dev,
	int		flavor,
	dev_status_t	stat,
	natural_t	count)
{
	mach_clock_t	clock = mc_clock[dev];
	unsigned int	res_request;
	unsigned int	res_ceiling, res_floor, res_closest;
	int		diff_ceiling, diff_floor, diff_closest;
	spl_t		s;

	switch (flavor) {
	    case CLOCK_RESOLUTION:
	    {
		clock_resolution_t	request;

		if (count < CLOCK_RESOLUTION_COUNT)
		    return D_INVALID_SIZE;

		/*
		 * Clock can only interrupt at frequencies of
		 * 2**n hz, 1 <= n <= 13.
		 */

		request = (clock_resolution_t)stat;
		res_request = request->resolution;

		if (res_request < MC_CLOCK_MIN_RESOLUTION ||
		    res_request > MC_CLOCK_MAX_RESOLUTION)
		{
		    return D_INVALID_SIZE;
		}
			
		/*
		 *	Find the first available resolution
		 *	(NANOSEC_PER_SEC / 2**n) smaller than
		 *	the requested resolution.  We already
		 *	checked that resolution is above the
		 *	hardware`s minimum.
		 */
		for (res_floor = NANOSEC_PER_SEC;
		     res_floor >= res_request;
		     res_floor >>= 1)
		    continue;

		/*
		 *	Calculate the next possible resolution
		 *	above the request.
		 */
		res_ceiling = res_floor << 1;

		/*
		 *	Find the difference between the requested
		 *	resolution and the available resolutions
		 *	that bracket it, and pick the smaller one.
		 */

		diff_floor = (int) res_floor - (int) res_request;
		if (diff_floor < 0)
		    diff_floor = -diff_floor;

		diff_ceiling = (int) res_ceiling - (int) res_request;
		if (diff_ceiling < 0)
		    diff_ceiling = -diff_floor;

		if (diff_floor < diff_ceiling) {
		    res_closest = res_floor;
		    diff_closest = diff_floor;
		}
		else {
		    res_closest = res_ceiling;
		    diff_closest = diff_ceiling;
		}

		/*
		 *	If the closer value is still outside
		 *	the tolerance ('skew'), reject the request.
		 */
		if (diff_closest > request->skew)
		    return D_INVALID_SIZE;

		/*
		 *	Now we have the new resolution to set.
		 *	Save it to take effect at the next
		 *	clock interrupt.
		 */
		s = splsched();
		clock_queue_lock(clock);

		clock->new_resolution = res_closest;
		clock->new_skew = diff_closest;

		clock_queue_unlock(clock);
		splx(s);

		return D_SUCCESS;
	    }

	    default:
		return clock_setstat(clock, flavor, stat, count);
	}
}


vm_offset_t mc_mmap(
	int		dev,
	vm_offset_t	off,
	vm_prot_t	prot)
{
	return clock_map_page(mc_clock[dev], off, prot);
}

io_return_t mc_devinfo(
	int		dev,
	int		flavor,
	mach_clock_t	*info)
{
	if (flavor == D_INFO_CLOCK) {
	    *info = mc_clock[dev];
	    return D_SUCCESS;
	}
	else {
	    return D_INVALID_OPERATION;
	}
}

void mc_clock_setresolution(
	mach_clock_t	clock)
{
	int		index, temp;
	unsigned int	resolution, res;
	
	/*
	 *	Convert the resolution to a rate index
	 *	for the chip.
	 */
	resolution = clock->resolution;
	for (res = NANOSEC_PER_SEC/2, index = MC_RATE_2_Hz;
	     res > resolution && index > MC_RATE_8192_Hz;
	     res >>=1, index--)
	    continue;


	/*
	 * Stop updates while we fix it 
	 */
	while (mc_read(rt_clock, MC_REG_A) & MC_REG_A_UIP)
	    delay(MC_UPD_MINIMUM >> 2);
	mc_write(rt_clock, MC_REG_B, MC_REG_B_STOP);
	wbflush();

	/*
	 * Ack any pending interrupts 
	 */
	temp = mc_read(rt_clock, MC_REG_C);

	/*
	 * Reset the frequency divider, in case we are changing it. 
	 */
	mc_write(rt_clock, MC_REG_A, MC_BASE_RESET);

	/*
	 * Spec says the VRT bit can be validated, but does not say how. I
	 * assume it is via reading the register. 
	 */
	temp = mc_read(rt_clock, MC_REG_D);

	/*
	 * Reconfigure the chip and get it started again 
	 */
	mc_write(rt_clock, MC_REG_A,
			   MC_BASE_32_KHz | index);
	mc_write(rt_clock, MC_REG_B,
			   MC_REG_B_CONFIGURE | mc_interrupt_enabled);
}

#ifdef	MC_CLOCK_BCD
int bcd_to_int(int bcd)
{
	return (bcd >> 4) * 10 + (bcd & 0xf);
}

int int_to_bcd(int intval)
{
	return ((intval / 10) << 4) + (intval % 10);
}
#endif

/*
 *	Read time from clock chip.  If battery was dead,
 *	return an invalid time_spec.
 */
time_spec_t mc_clock_read(
	mach_clock_t	clock)
{
	unsigned int years, months, days, hours, minutes, seconds;
#ifdef	MC_CENTURY
	unsigned int century;
#endif
	time_spec_t   clock_time;
	spl_t	s;

	/*
	 *	Check the battery.
	 */
	if ((mc_read(rt_clock, MC_REG_D) && MC_REG_D_VRT) == 0) {
	    /*
	     *	No time.
	     */
	    clock_time.seconds = 0;
	    clock_time.nanoseconds = NANOSEC_PER_SEC + 1; /* invalid */
	    return clock_time;
	}

	/*
	 *	Wait for update to finish
	 */
	s = splhigh();

	while (mc_read(rt_clock, MC_REG_A) & MC_REG_A_UIP)
	    delay(MC_UPD_MINIMUM >> 2);

	/*
	 *	Grab the date and time
	 */
#ifdef	MC_CENTURY
	century = mc_read(rt_clock, MC_CENTURY);
#endif
	years	= mc_read(rt_clock, MC_YEAR);
	months	= mc_read(rt_clock, MC_MONTH);
	days	= mc_read(rt_clock, MC_DAY_OF_MONTH);
	hours	= mc_read(rt_clock, MC_HOUR);
	minutes	= mc_read(rt_clock, MC_MINUTE);
	seconds	= mc_read(rt_clock, MC_SECOND);

	splx(s);

#ifdef	MC_CLOCK_BCD
	/*
	 *	Convert the numbers from BCD to binary.
	 */
#ifdef	MC_CENTURY
	century = bcd_to_int(century);
#endif
	years	= bcd_to_int(years);
	months	= bcd_to_int(months);
	days	= bcd_to_int(days);
	hours	= bcd_to_int(hours);
	minutes	= bcd_to_int(minutes);
	seconds	= bcd_to_int(seconds);
#endif

	/*
	 *	Convert to seconds from 1970.
	 */
#ifdef	MC_CENTURY
	years += century * 100;
#else
	if (years < 70 || mc_new_century)
	    years += 2000;	/* it`s coming soon... */
	else
	    years += 1900;
#endif

	clock_time.seconds = ymd_to_seconds(
		years, months, days, hours, minutes, seconds);
	clock_time.nanoseconds = 0;

	return clock_time;
}

/*
 *	Set real-time-clock-chip from supplied time.
 *	Clock has already been updated.
 */
void mc_clock_write(
	mach_clock_t	clock,
	time_spec_t	new_time)
{
	unsigned int years, months, days, hours, minutes, seconds;
	unsigned int day_of_week, temp;
#ifdef	MC_CENTURY
	unsigned int century;
#endif
	spl_t	s;

	/*
	 *	Update hardware, to nearest second
	 */
	seconds_to_ymd(new_time.seconds,
		&years, &months, &days, &hours, &minutes, &seconds,
		&day_of_week);

#ifdef	MC_CENTURY
	/*
	 *	Keep track of century
	 */
	century = years / 100;
#endif
	years %= 100;

	/*
	 * Check for "hot dates" (chip bugs)
	 */
	if (days >= 28 && days <= 30 &&
	    hours == 23 && minutes == 59 &&
	    seconds >= 58)
		seconds = 57;

#if	MC_CLOCK_BCD
	/*
	 *	DOS keeps the time in BCD.  Convert numbers from decimal.
	 */
#ifdef	MC_CENTURY
	century = int_to_bcd(century);
#endif
	years	= int_to_bcd(years);
	months	= int_to_bcd(months);
	days	= int_to_bcd(days);
	hours	= int_to_bcd(hours);
	minutes	= int_to_bcd(minutes);
	seconds	= int_to_bcd(seconds);
#endif

	/*
	 *	Stop updates while we change the time
	 */
	s = splhigh();

	while (mc_read(rt_clock, MC_REG_A) & MC_REG_A_UIP)
	    delay(MC_UPD_MINIMUM >> 2);

	mc_write(rt_clock, MC_REG_B, MC_REG_B_STOP);

	/*
	 *	Ack any pending interrupts
	 */
	temp = mc_read(rt_clock, MC_REG_C);

	/*
	 *	Update the time
	 */
#ifdef	MC_CENTURY
	mc_write(rt_clock, MC_CENTURY, century);
#endif
	mc_write(rt_clock, MC_YEAR, years);
	mc_write(rt_clock, MC_MONTH, months);
	mc_write(rt_clock, MC_DAY_OF_MONTH, days);
	mc_write(rt_clock, MC_HOUR, hours);
	mc_write(rt_clock, MC_MINUTE, minutes);
	mc_write(rt_clock, MC_SECOND, seconds);

	mc_write(rt_clock, MC_DAY_OF_WEEK, day_of_week+1); /* 1..7 */

	/*
	 *	Restart the chip
	 */
	mc_write(rt_clock, MC_REG_B, MC_REG_B_CONFIGURE | mc_interrupt_enabled);

	splx(s);
}

/****************************************************************/

#if	defined(DECSTATION) || defined(FLAMINGO)
/*
 *	Interrupt routines
 */

#if	MC_DOES_DELAYS

#ifdef	DECSTATION

#include <kern/machine.h>
#define	MC_DELAY_PMAX	8
#define	MC_DELAY_3MAX	12

#endif

/*
 * Timed delays
 */

static int		config_step = 0;	/* configure when clock
						   interrupts are first
						   enabled */
static volatile int	had_intr;

extern unsigned int cpu_speed;

void
config_delay(unsigned int speed)
{
	/*
	 * This is just an initial estimate, later on with the clock
	 * running we'll tune it more accurately.
	 */
	cpu_speed = speed;
}

void accurate_config_delay(
	spl_t		spllevel)
{
	register unsigned int	i;
	register spl_t		s;
	int			inner_loop_count;
	int			clock_ticks_per_second;

#ifdef	mips
	/* find "spllevel - 1" */
	s = spllevel | ((spllevel >> 1) & SR_INT_MASK);
	splx(s);
#else
#endif

	/* wait till we have an interrupt pending */
	had_intr = 0;
	while (!had_intr)
		continue;

	had_intr = 0;
	i = delay_timing_function(1, &had_intr, &inner_loop_count);

	splx(spllevel);

	/* split calculation to avoid overflow */
	clock_ticks_per_second = (NANOSEC_PER_SEC / mc_clock0.resolution);
	i *= clock_ticks_per_second;

	cpu_speed = i / (inner_loop_count * 1000000);

	/* roundup clock speed */
	i /= 100000;
	if ((i % 10) >= 5)
		i += 5;
	printf("Estimating CPU clock at %d Mhz\n", i / 10);
	if (isa_pmax() && cpu_speed != MC_DELAY_PMAX) {
		printf("%s\n", "This machine looks like a DEC 2100");
		machine_slot[cpu_number()].cpu_subtype = CPU_SUBTYPE_MIPS_R2000;
	}
}

int mc_intr(
	spl_t	spllevel)
{
	/*
	 * Interrupt flags are read-to-clear.
	 */
	if (config_step > 2)
		return (mc_read(rt_clock,MC_REG_C) & MC_REG_C_IRQF);
	had_intr = (mc_read(rt_clock,MC_REG_C) & MC_REG_C_IRQF) ? 1 : 0;
	if (config_step++ == 0)
		accurate_config_delay(spllevel);
	return had_intr;
}

#else	/* MC_DOES_DELAYS */

int mc_intr(spl_t	spllevel)
{
	return mc_read(rt_clock, MC_REG_C);	/* clear intr */
}

#endif	/* MC_DOES_DELAYS */


#endif	/* defined(DECSTATION) || defined(FLAMINGO) */

#endif	/* NMC > 0 */
