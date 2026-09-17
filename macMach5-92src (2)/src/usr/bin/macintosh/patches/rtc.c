/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

#include <mach.h>

#include <device/device_types.h>

#include <emul.h>

extern mach_port_t	master_device_port;

static device_t		time_device, kern_device;

unsigned		*time;

/*
 * RTC Time and
 * Parameter RAM Traps
 */

initRTC()
{
    memory_object_t	pager;

    (void) device_open(master_device_port,
		       D_READ|D_WRITE,
		       "time",
		       &time_device);

    (void) device_map(time_device,
		      VM_PROT_READ,
		      0, vm_page_size,
		      &pager, 0);

    (unsigned)time = malloc(vm_page_size);

    (void) vm_deallocate(mach_task_self(), time, vm_page_size);

    (void) vm_map(mach_task_self(),
		  &time, vm_page_size,
		  0, FALSE,
		  pager,
		  0, FALSE,
		  VM_PROT_READ, VM_PROT_READ,
		  VM_INHERIT_NONE);

    (void) mach_port_deallocate(mach_task_self(), pager);

    (void) device_open(master_device_port,
		       D_READ|D_WRITE,
		       "kern",
		       &kern_device);
}

/*
 * The delta (in seconds) between MacOS T(0) and U**X T(0)
 */
#define T0_DELTA (((365*(1970-1904))+((1970-1904)/4)+1)*24*60*60)

ReadDateTime(regs)
os_reg_t	regs;
{
    register unsigned	t;

    t = *time + T0_DELTA /* - (tz.tz_minuteswest*60) */;

    *(unsigned *)0x20c = t;
    *(unsigned *)(regs.a_0) = t;

    regs.d_0 = 0;
}

SetDateTime(regs)
os_reg_t	regs;
{
    *(unsigned *)0x20c = regs.d_0;

    regs.d_0 = 0;
}

typedef struct {
    unsigned char valid;
    unsigned char aTalkA;
    unsigned char aTalkB;
    unsigned char config;
    unsigned short portA;
    unsigned short portB;
    unsigned long alarm;
    unsigned short font;
    unsigned short kbdPrint;
    unsigned short volClik;
    unsigned short misc;
} SysParamType;

SysParamType	DefParam
    = { 0xa8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x6300, 0x388, 0x4c };

#define SysParam	((SysParamType *)0x1f8)
#define GetParam	((SysParamType *)0x1e4)

int	pram = -1;

InitUtil(regs)
os_reg_t	regs;
{
    register boolean_t		failed = FALSE;
#ifdef notdef
    struct timeval		time;
    struct timezone		tz;

    if (pram < 0)
	pram = open("/tmp/mac_pram", O_RDWR | O_CREAT, 0600);

    if (pram < 0)
	failed = TRUE;
    else {
	(void) lseek(pram, 0, L_SET);

	if (read(pram,
		 GetParam,
		 sizeof (SysParamType)) != sizeof (SysParamType))
	    failed = TRUE;
	else if (GetParam->valid != DefParam.valid)
	    failed = TRUE;
    }

    if (failed) {
	bcopy(&DefParam, GetParam, sizeof (SysParamType));
	regs.d_0 = -88;	/* prInitErr */

	(void) lseek(pram, 0, L_SET);

	(void) write(pram, GetParam, sizeof (SysParamType));
    }

    bcopy(GetParam, SysParam, sizeof (SysParamType));

    (void) gettimeofday(&time, &tz);

    *(unsigned long *)0x20c = time.tv_sec + T0_DELTA - (tz.tz_minuteswest*60);
#endif
    asm("trap #15");

    if (!failed)
	regs.d_0 = 0;
}

WriteParam(regs)
os_reg_t	regs;
{
    regs.d_0 = 0;
}

ReadXPRam(regs)
os_reg_t	regs;
{
    union {
	unsigned long	arg;
	struct {
	    unsigned short	len;
	    unsigned short	addr;
	} y;
    } x;
    unsigned char	data[256];
    unsigned		count;
    io_return_t		result;

    x.arg = regs.d_0;

    if (x.y.len > sizeof (data))
	x.y.len = sizeof (data);

    count = (x.y.len + sizeof (int) - 1) / sizeof (int);
    result = device_get_status(kern_device,
			       0x80000000 | x.arg,
			       data, &count);

    if (result == D_SUCCESS)
	bcopy(data, regs.a_0, x.y.len);
    else
	/* XXX */;

    regs.d_0 = 0;
}

WriteXPRam(regs)
os_reg_t	regs;
{
#ifdef notdef
    union {
	unsigned long	arg;
	struct {
	    unsigned short	len;
	    unsigned short	addr;
	} y;
    } x;
    unsigned char	data[256];
    unsigned		count;
    io_return_t		result;

    x.arg = regs.d_0;

    if (x.y.len > sizeof (data))
	x.y.len = sizeof (data);

    bcopy(regs.a_0, data, x.y.len);

    count = (x.y.len + sizeof (int) - 1) / sizeof (int);
    result = device_set_status(kern_device,
			       0x80000000 | x.arg,
			       data, count);

    if (result != D_SUCCESS)
	/* XXX */;
#endif

    regs.d_0 = 0;
}
