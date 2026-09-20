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

/*
 *	File:		rtclock.c
 *	Purpose:	Routines for handling the machine dependent
 *			real-time clock.
 */

#include <cpus.h>
#include <debug.h>
#include <platforms.h>
#include <mach_mp_debug.h>
#include <linux_dev.h>

#include <kern/cpu_number.h>
#include <kern/clock.h>
#include <kern/misc_protos.h>
#include <kern/spl.h>
#include <machine/mach_param.h>	/* HZ */
#include <mach/vm_prot.h>
#include <vm/pmap.h>
#include <vm/vm_kern.h>		/* for kernel_map */
#include <device/dev_hdr.h>	/* for mach_device_t */
#include <device/ds_routines.h>
#include <ppc/misc_protos.h>
#include <ppc/proc_reg.h>
#include <ppc/spl.h>
#include <ppc/POWERMAC/rtclock_entries.h>
#include <ppc/POWERMAC/interrupts.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/device_tree.h>

extern long long read_processor_clock(void);

#if	LINUX_DEV
extern void linux_timer_intr(void);
#endif	/* LINUX_DEV */

/*
 * List of real-time clock dependent routines.
 */
struct clock_ops  rtc_ops = {
	rtc_config,	rtc_init,	rtc_gettime,	rtc_settime,
	rtc_getattr,	rtc_setattr,	rtc_maptime,	rtc_setalrm,
};

/* local data declarations */
tvalspec_t		*RtcTime = (tvalspec_t *)0;
tvalspec_t		*RtcAlrm;
clock_res_t		RtcDelt;

#if	MACH_MP_DEBUG
int	masked_state_cnt[NCPUS];
int	masked_state_max = 10*HZ;
#endif	/* MACH_MP_DEBUG */

/* global data declarations */

rtclock_t rtclock;
boolean_t rtclock_initialised;

int rtclock_intr_clock_ticks;

#define RTC_MAXRES	(NSEC_PER_SEC / HZ)	/* max resolution nsec */
#if	MACH_KPROF
#define	RTC_MINRES	(RTC_MAXRES)		/* min resolution nsec */
#else	/* MACH_KPROF */
#define	RTC_MINRES	(RTC_MAXRES / 20)	/* min resolution nsec */
#endif	/* MACH_KPROF */

#if	DEBUG
struct rtclock_debug {
	int	tick_out_of_sync;
	int	tick_count;
	int	delay;
	int	delay_missed;
	long long processor_clock_now;
} rtclock_debug;
#endif /* DEBUG */

struct rtclock_private_t {
	/* numer,denom is powermac_info.proc_clock_to_nsec_{numer,denom} */
	
	decl_simple_lock_data(,lock)
	int version0, version1;
	/* Write lock for offset_* structures.  Check versions for reading.
	 */

	long long offset_ticks;
	tvalspec_t offset_time;
	/* offset_* is kept close to the current time to allow for processor
	 * clock overflow.
	 *
	 * offset_ticks * numer/denom == offset_time
	 */

	long long rollover_ticks;
	tvalspec_t rollover_time;
	/* rollover_* is the amount of catchup to perform when
	 * read_processor_clock gets ahead of offset_*.
	 *
	 * rollover_ticks * numer/denom == rollover_time.
	 */

	long long rollover_threshold_ticks;
	/* rollover_threshold_ticks determines when rollover should occur.
	 */
};

static struct rtclock_private_t rtclock_private;


/*
 *	Macros to lock/unlock real-time clock device.
 */
#define LOCK_RTC(s)			\
	s = splhigh();			\
	simple_lock(&rtclock.lock);

#define UNLOCK_RTC(s)			\
	simple_unlock(&rtclock.lock);	\
	splx(s);

#define LOCK_RTC_PRIVATE(s) \
	do { s = splhigh(); simple_lock(&rtclock_private.lock); } while (0)
#define UNLOCK_RTC_PRIVATE(s) \
	do { simple_unlock(&rtclock_private.lock); splx(s); } while (0)

/*
 * Configure the real-time clock device. Return success (1)
 * or failure (0).
 */

int
rtc_config(void)
{
	int	RtcFlag;

#if	NCPUS > 1
	if (cpu_number() != master_cpu)
		return(1);
#endif

	/*
	 * We should attempt to test the real-time clock
	 * device here. If it were to fail, we should panic
	 * the system.
	 */
	RtcFlag = /* test device */1;
	return (RtcFlag);
}

/* rtc_init_private(struct rtclock_private_t *p)
 *
 * Initialize *p.
 */

static void
rtc_init_private(
	struct rtclock_private_t *p)  /* OUT */
{
    simple_lock_init(&p->lock, ETAP_MISC_RT_CLOCK_PRIV);
    p->version0 = p->version1 = 0;
    
    /* Start clock relative to now */
    
    p->offset_ticks = read_processor_clock();
    p->offset_time.tv_nsec = 0;
    p->offset_time.tv_sec = 0;

    /* Start rollover at ticks : time == denom : numer */
    
    p->rollover_ticks = powermac_info.proc_clock_to_nsec_denominator;
    p->rollover_time.tv_nsec = powermac_info.proc_clock_to_nsec_numerator;
    p->rollover_time.tv_sec = 0;
    
    while (p->rollover_time.tv_nsec >= NSEC_PER_SEC) {
	p->rollover_time.tv_nsec -= NSEC_PER_SEC;
	p->rollover_time.tv_sec++;
    }
    
    /* Multiply rollover to maximize rollover amount, without causing
     * ticks*numerator to overflow.  To be conservative, keep tv_sec
     * within a reasonable value as well.
     */

    /* This should be true to start, else numer*denom is too large */
    assert(p->rollover_ticks * powermac_info.proc_clock_to_nsec_numerator < 0x1000000000000000LL);
    
    while (p->rollover_time.tv_sec < 8640000 /* 8640000secs = 100days */
	   && p->rollover_ticks * powermac_info.proc_clock_to_nsec_numerator < 0x1000000000000000LL) {
	/* Double the rollover time */
	p->rollover_ticks <<= 1;
	p->rollover_time.tv_nsec <<= 1;
	p->rollover_time.tv_sec <<= 1;
	if (p->rollover_time.tv_nsec > NSEC_PER_SEC) {
	    p->rollover_time.tv_nsec -= NSEC_PER_SEC;
	    p->rollover_time.tv_sec++;
	}
    }

    /* rollover_ticks * numerator < 0x2000 0000 0000 0000
     */
    
    /* Setup threshold.  Give some breathing room between the processor
     * clock and offset_ticks.  [ If two processors' clocks are not
     * synchronized, and the fast one overflows, the slow one may see the
     * offset_ticks > clock.  This may be moot, since unsynchronized clocks
     * would probably cause more fundamental problems ]
     */

    p->rollover_threshold_ticks = p->rollover_ticks << 1;

    /* rollover_threshold_ticks * numerator < 0x4000 0000 0000 0000
     */
}

/*
 * Initialize the real-time clock device. Return success (1)
 * or failure (0). Since the real-time clock is required to
 * provide canonical mapped time, we allocate a page to keep
 * the clock time value. In addition, various variables used
 * to support the clock are initialized.
 */
int
rtc_init(void)
{
	vm_offset_t	*vp;

#if	NCPUS > 1
	if (cpu_number() != master_cpu) {
		if (rtclock_initialised == FALSE) {
			panic("rtc_init on cpu %d, rtc not initialised\n",
			      cpu_number());
		}
		/* Set decrementer and hence our next tick due */
		mtdec(rtclock_intr_clock_ticks);

		return(1);
	}
#endif
	/*
	 * Allocate mapped time page.
	 */
	vp = (vm_offset_t *) &rtclock.mtime;
	if (kmem_alloc_wired(kernel_map, vp, PAGE_SIZE) != KERN_SUCCESS)
		panic("cannot allocate rtclock time page\n");
	bzero((char *)rtclock.mtime, PAGE_SIZE);
	RtcTime = &rtclock.mtime->mtv_time;

	/*
	 * Initialize non-zero clock structure values.
	 */
	simple_lock_init(&rtclock.lock, ETAP_MISC_RT_CLOCK);
	rtclock.intr_nsec = RTC_MAXRES;
	rtclock.intr_freq = (NSEC_PER_SEC / RTC_MAXRES);
	rtclock.intr_hertz = 1;
	rtclock.intr_count = 1;
	RtcDelt = RTC_MAXRES/2;
	
	rtclock_intr_clock_ticks =
		nsec_to_processor_clock_ticks(rtclock.intr_nsec);

	/* Initialize the private stuff */
	rtc_init_private(&rtclock_private);
	
	/* Set decrementer and our next tick due */
	mtdec(rtclock_intr_clock_ticks);

	rtclock_initialised = TRUE;

	return (1);
}

/*
 * Get the clock device time. This routine is responsible
 * for converting the device's machine dependent time value
 * into a canonical tvalspec_t value.
 *
 * SMP configurations - *this currently assumes that the processor
 * clocks will be synchronised*
 *
 * Reads the processor realtime clock, converting into
 * a tvalspec structure. Note that on the 601 processor
 * we have to take into consideration the possibility
 * that the clock doesn't tick at the correct frequency
 */

kern_return_t
rtc_gettime(
	tvalspec_t	*time)	/* OUT */
{
    long long clock;
    int version;

    /* What time is it? */
    clock = read_processor_clock();

    /* Convert clock to time, but do calculation relative to
     * rtclock_private.offset.  do{}while() to repeat calculation
     * if rollover occurs during calculation.
     *
     * Rollover occurs less frequently than 1/day, so the code is repeated
     * very infrequently, and at worst, will be done twice.
     */

    do {
	long long now;

	/* Get version for check later */

	version = rtclock_private.version0;

	/* Determine time relative to offset */
	
	now = clock - rtclock_private.offset_ticks;

	/* Rollover if now is past thresold */

	if (now > rtclock_private.rollover_threshold_ticks) {
	    spl_t s;
	    LOCK_RTC_PRIVATE(s);
	    
	    if (version == rtclock_private.version0) {
		
		/* Got lock and data has not changed */
		
		/* Start update: update version1 */
		
		rtclock_private.version1 = version+1;
		
		/* Add rollover_ticks to offset_ricks
		 * add rollover_time to offset_time
		 */
		
		rtclock_private.offset_ticks += rtclock_private.rollover_ticks;
		ADD_TVALSPEC(&rtclock_private.offset_time,&rtclock_private.rollover_time);
		
		/* End update: update version0 */
		
		rtclock_private.version0 = version+1;

	    } /* else somebody updated offset.  Leave it */
	    
	    UNLOCK_RTC_PRIVATE(s);

	    /* Offset has been changed (by us or another).  Start again */

	    /* version != rtclock_private.version1 */
	    continue;
	}
	
	/* Convert ticks to time */
    
	/*    now <= rollover_threshold_ticks
	 * && rollover_threshold_ticks * numer < 0x4000 0000 0000 0000
	 *
	 * => now * numer <= rollover_threadhold_ticks * numer
	 * => now * numer < 0x4000 0000 0000 0000
	 * => calc will not overflow
	 */
	
	now = ( (now * powermac_info.proc_clock_to_nsec_numerator)
		/ powermac_info.proc_clock_to_nsec_denominator );
	
	time->tv_sec  = (int) (now / NSEC_PER_SEC);
	time->tv_nsec = (int) (now - ( ( (long long) time->tv_sec ) * NSEC_PER_SEC));
	
	/* Add in current offset */
	
	ADD_TVALSPEC(time,&rtclock_private.offset_time);

	/* Repeat if data has changed */
    } while (version != rtclock_private.version1);

    assert(0 <= time->tv_nsec && time->tv_nsec < NSEC_PER_SEC);
    
    return (KERN_SUCCESS);
}

/*
 * Set the clock device time. The alarm clock layer has
 * verified the new_time as a legal tvalspec_t value.
 */
kern_return_t
rtc_settime(
	tvalspec_t	*new_time)
{
	printf("MEB - rtc_settime TODO!!\n");
	return (KERN_SUCCESS);
}

/*
 * Get clock device attributes.
 */
kern_return_t
rtc_getattr(
	clock_flavor_t		flavor,
	clock_attr_t		attr,		/* OUT */
	mach_msg_type_number_t	*count)		/* IN/OUT */
{
	spl_t	s;

	if (*count != 1)
		return (KERN_FAILURE);
	switch (flavor) {

	case CLOCK_GET_TIME_RES:	/* >0 res */
	case CLOCK_MAP_TIME_RES:	/* >0 canonical */
	case CLOCK_ALARM_CURRES:	/* =0 no alarm */
		LOCK_RTC(s);
		*(clock_res_t *) attr = rtclock.intr_nsec;
		UNLOCK_RTC(s);
		break;

	case CLOCK_ALARM_MINRES:
		*(clock_res_t *) attr = RTC_MINRES;
		break;

	case CLOCK_ALARM_MAXRES:
		*(clock_res_t *) attr = RTC_MAXRES;
		break;

	default:
		return (KERN_INVALID_VALUE);
	}
	return (KERN_SUCCESS);
}

/*
 * Set clock device attributes.
 */
kern_return_t
rtc_setattr(
	clock_flavor_t		flavor,
	clock_attr_t		attr,		/* IN */
	mach_msg_type_number_t	count)		/* IN */
{
	spl_t		s;
	int		freq;
	int		adj;
	clock_res_t	new_ires;

	if (count != 1)
		return (KERN_FAILURE);
	switch (flavor) {

	case CLOCK_GET_TIME_RES:
	case CLOCK_MAP_TIME_RES:
	case CLOCK_ALARM_MINRES:
	case CLOCK_ALARM_MAXRES:
		return (KERN_FAILURE);

	case CLOCK_ALARM_CURRES:
		new_ires = *(clock_res_t *) attr;

		/*
		 * The new resolution should be a multiple of the minimum
		 * resolution. If the desired resolution cannot be achieved
		 * to within 0.1%, an error is returned.
		 */
		if (new_ires < RTC_MINRES || new_ires > RTC_MAXRES)
			return (KERN_INVALID_VALUE);
		if (new_ires % RTC_MINRES)
			return (KERN_INVALID_VALUE);
		freq = (NSEC_PER_SEC / new_ires);
		adj = (((RTC_SPEED % freq) * new_ires) / RTC_SPEED);
		if (adj > (new_ires / 1000))
			return (KERN_INVALID_VALUE);

		/*
		 * Record the new alarm resolution which will take effect
		 * on the next HZ aligned clock tick.
		 */
		LOCK_RTC(s);
		if (new_ires  != rtclock.intr_nsec)
			rtclock.new_ires = new_ires;
		UNLOCK_RTC(s);
		return (KERN_SUCCESS);

	default:
		return (KERN_INVALID_VALUE);
	}
}

/*
 * Map the clock device time. The real-time clock is required
 * to map a canonical time value. We cheat here, by using the
 * device subsystem to generate a pager for the clock - this
 * should probably be changed at some point.
 */
kern_return_t
rtc_maptime(
	ipc_port_t	*pager)		/* OUT */
{
	mach_device_t	device;
	vm_size_t	size;

	/*
	 * Open the "rtclock" device. The only interface available
	 * to this device is the d_mmap() routine.
	 */
	device = device_lookup("rtclock");
	if (device == MACH_DEVICE_NULL)
		return (KERN_FAILURE);

	/*
	 * Note: this code does not touch the device state. This should
	 * be ok even though the device can be opened from user mode via
	 * a device_open() request since both the device and the pager
	 * structures should be protected from deletion by reference
	 * counts and since the dev_pager code doesn't depend upon any
	 * internal device state setup via device_open() other than
	 * that established from device_lookup().
	 */
	size = sizeof(mapped_tvalspec_t);
	return (device_pager_setup(device, VM_PROT_READ, 0, size, pager));
}

/*
 * Set next alarm time for the clock device. This call
 * always resets the time to deliver an alarm for the
 * clock.
 */
void
rtc_setalrm(
	tvalspec_t	*alarm_time)
{
	spl_t		s;

	LOCK_RTC(s);
	rtclock.alarm_time = *alarm_time;
	RtcAlrm = &rtclock.alarm_time;
	UNLOCK_RTC(s);
}


/*
 * Reset the clock device. This causes the realtime clock
 * device to reload its mode and count value (frequency).
 */

void
rtclock_reset(void)
{
	return;
}

/*
 * Map the clock device time. This is part of the "cheat"
 * we use to map the time via the device subsystem. This
 * routine is invoked via a vm_map() call by the user.
 */

vm_offset_t
rtclock_map(
	dev_t		dev,
	vm_offset_t	off,
	int		prot)
{
	vm_offset_t	voff;

#ifdef	lint
	dev++; off++;
#endif	/* lint */

	if (prot & VM_PROT_WRITE)
		return (-1);
	voff = pmap_extract(pmap_kernel(), (vm_offset_t) rtclock.mtime);
	return (voff >> PPC_PGSHIFT);
}

/*
 * Real-time clock device interrupt.
 * On master processor it updates the clock time and upcalls
 * into the higher level clock code to deliver alarms.
 */
int
rtclock_intr(int device, struct ppc_saved_state *ssp, spl_t old_spl)
{
	int		mycpu;
	spl_t		s;
	tvalspec_t	clock_time;
	int		now,now2;
	int		i;

	/* We may receive interrupts too early, we must reject them. At
	 * startup, the decrementer interrupts every ~2 seconds without
	 * us doing anything
	 */

	if (rtclock_initialised == FALSE) {
		mtdec(0x7FFFFFFF); /* Max the decrimenter if not init */
		return 0;
	}

	mycpu = cpu_number();

	assert(old_spl > SPLCLOCK);

#if	MACH_MP_DEBUG
	/*
	 * Increments counter of clock ticks handled under a masked state.
	 * Debugger() is called if masked state is kept during 1 sec.
	 * The counter is reset by splx() when ipl mask is set back to SPL0,
	 * and by spl0().
	 */
	if (old_spl != SPLLO) {
	
		if (masked_state_cnt[mycpu]++ >= masked_state_max) {
			int max_save = masked_state_max;

			masked_state_cnt[mycpu] = 0;
			masked_state_max = 0x7fffffff;

			printf("looping at high IPL, usermode=%d pc=0x%x\n",
					USER_MODE(ssp->srr1), ssp->srr0);
			Debugger("");

			masked_state_cnt[mycpu] = 0;
			masked_state_max = max_save;
		}
	} else {
		masked_state_cnt[mycpu] = 0;
	}
	/* we should be masked if we were at splvm. lose a tick. */
	if (old_spl <= SPLVM) {
		LOCK_RTC(s);
		while ((now2 = isync_mfdec()) == now)
			;
		mtdec(now2 + rtclock_intr_clock_ticks);
		UNLOCK_RTC(s);
		return 0;
	}
#endif	/* MACH_MP_DEBUG */

	while ((now = isync_mfdec()) < 0) {

		/*
		 * Wait for the decrementer to change, then jump
		 * in and add decrementer_count to its value
		 * (quickly, before it changes again!)
		 */
		while ((now2 = isync_mfdec()) == now)
			;
		mtdec(now2 + rtclock_intr_clock_ticks);

		/* Some work must be done on master CPU only */
		if (cpu_number() == master_cpu) {

			LOCK_RTC(s);

			/* Update mapped time. Do the update so that
			 * the macro MTS_TO_TS() for reading the
			 * mapped time works (e.g.  update in order:
			 * mtv_csec, mtv_nsec, mtv_sec).
			 */
			rtc_gettime(&clock_time);

			rtclock.mtime->mtv_csec = clock_time.tv_sec;
			rtclock.mtime->mtv_nsec = clock_time.tv_nsec;
			rtclock.mtime->mtv_sec  = clock_time.tv_sec;

			/*
			 * Perform alarm clock processing if needed. The time
			 * passed up is incremented by a half-interrupt tick
			 * to trigger alarms closest to their desired times.
			 * The clock_alarm_intr() routine calls rtc_setalrm()
			 * before returning if later alarms are pending.
			 */

			if (RtcAlrm && (RtcAlrm->tv_sec < RtcTime->tv_sec ||
					(RtcAlrm->tv_sec == RtcTime->tv_sec &&
					 RtcDelt >= RtcAlrm->tv_nsec - 
					 RtcTime->tv_nsec))) {
				clock_time.tv_sec = 0;
				clock_time.tv_nsec = RtcDelt;
				ADD_TVALSPEC (&clock_time, RtcTime);
				RtcAlrm = 0;
				UNLOCK_RTC(s);
				/*
				 * Call clock_alarm_intr() without RTC-lock.
				 * The lock ordering is always CLOCK-lock
				 * before RTC-lock.
				 */
				clock_alarm_intr(REALTIME_CLOCK, &clock_time);
				LOCK_RTC(s);
			}

			/*
			 * On a HZ-tick boundary: return 0 and adjust the clock
			 * alarm resolution (if requested).  Otherwise return a
			 * non-zero value.
			 */
			if ((i = --rtclock.intr_count) == 0) {
				if (rtclock.new_ires) {
					/* adjust rtclock values */
					rtclock.intr_nsec = rtclock.new_ires;
					rtclock.intr_freq = (NSEC_PER_SEC /
							    rtclock.intr_nsec);
					rtclock.intr_hertz =rtclock.intr_freq/
							    HZ;
					RtcDelt = rtclock.intr_nsec / 2;
					rtclock.new_ires = 0;
				
					rtclock_intr_clock_ticks =
						nsec_to_processor_clock_ticks(
						      rtclock.intr_nsec);

					/* reset hardware */
					rtclock_reset();
				}
				rtclock.intr_count = rtclock.intr_hertz;
			}
			UNLOCK_RTC(s);
#if	LINUX_DEV
			/* invoke Linux timer interrupt (jiffy update)
			   only from the main processor on HZ tick */
			if (i == 0)
				linux_timer_intr();
#endif	/* LINUX_DEV */
		}

		/* This is done for all processors */

		hertz_tick(USER_MODE(ssp->srr1), ssp->srr0);

	}

	return i;
}

/*
 * long long read_processor_clock(void)
 *
 * Read the processor's realtime clock, converting into
 * a 64 bit number
 */

long long read_processor_clock(void)
{
	union {
		long long 	time64;
		int		word[2];
	} now;

	if (PROCESSOR_VERSION == PROCESSOR_VERSION_601) {
		unsigned int nsec,sec;
		do {
			sec  = mfrtcu();
			nsec = mfrtcl();
		} while (sec != mfrtcu());
		return (long long) nsec + (NSEC_PER_SEC * (long long)sec);
	} else {
		do {
			now.word[0]  = mftbu();
			now.word[1]  = mftb();
		} while (now.word[0] != mftbu());
		return now.time64;
	}
}

int nsec_to_processor_clock_ticks(int nsec)
{
	long long num;

	if (nsec >= ((unsigned int)0xffffffff /
		     powermac_info.proc_clock_to_nsec_denominator)) {
		
		num = (long long) nsec;

		/* 64 bit division - argh! but we're not called often */
		num = (num * powermac_info.proc_clock_to_nsec_denominator) /
			powermac_info.proc_clock_to_nsec_numerator;

		return (int) num;
	} else {
		/* 32 bit division, slightly faster! */
		return (nsec * powermac_info.proc_clock_to_nsec_denominator) /
			powermac_info.proc_clock_to_nsec_numerator;
	}
}

/*
 * Delay creates a busy-loop of a given number of usecs. Doesn't use
 * interrupts
 *
 * This uses the rtc registers or the time-base registers depending upon
 * processor type.
 */

void delay(int usec)
{
	long ticks;

	ticks = nsec_to_processor_clock_ticks(NSEC_PER_SEC);

	/* longer delays may require multiple calls to tick_delay */
	while (usec > USEC_PER_SEC) {
		tick_delay(ticks);
		usec -= USEC_PER_SEC;
	}
	tick_delay(nsec_to_processor_clock_ticks(usec * NSEC_PER_USEC));
}

/* nsec_delay may overflow for delays over around 2 seconds. */
void tick_delay(int ticks)
{
	long long time, time2;

	time = read_processor_clock();

	/* Add on delay we want */
	
	time += ticks;
		
	/* Busy loop */

	do {
		time2 = read_processor_clock();
	} while (time2 < time);
}
