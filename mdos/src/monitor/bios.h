/*
 * Copyright (c) 1991 Carnegie Mellon University
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
 *
 * Purpose:
 *	V86 BIOS emulation
 *
 * HISTORY: 
 * $Log:	bios.h,v $
 * Revision 2.7  92/07/01  14:24:25  grm
 * 	Ifdef out idle code.
 * 	[92/07/01  14:18:28  grm]
 * 
 * 	Added idle locking macros.
 * 	[92/06/30  13:40:06  grm]
 * 
 * 	Removed the EFL_ALLCC (jvt@kampi.hut.fi).
 * 	[92/06/03            grm]
 * 
 * Revision 2.6  92/03/02  15:47:04  grm
 * 	Removed the curses.h include, and added a sys/types.h include.
 * 	[92/02/20            grm]
 * 
 * Revision 2.5  92/02/03  14:24:31  rvb
 * 	Clean Up
 * 
 * Revision 2.4  91/12/06  15:27:51  grm
 * 	Added Segment, Offset, and Abs2Sogoff macros to convert absolute
 * 	addresses into seg:off addresses.
 * 	[91/12/06            grm]
 * 
 * Revision 2.3  91/12/05  16:39:37  grm
 * 	Modified for dbg's new kernel support.
 * 	[91/06/14  11:39:36  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  08:30:06  grm]
 * 
 * 	Removed the trace support.
 * 	[91/05/02  13:30:05  grm]
 * 
 * 	Ch ch ch changes....
 * 	[91/03/26  19:29:20  grm]
 * 
 * 	Added media info.  lock_cs and unlock_cs added.
 * 	[90/11/28  19:07:45  grm]
 * 
 * 	Added start_sec to devices.  Changed the vga
 * 	structure's header info.
 * 	[90/11/09  21:03:31  grm]
 * 
 * 	Various additions.
 * 	[90/10/04  20:57:54  grm]
 * 
 * 	Fixed port_ok.  Added dd_fd and dd_stream, removed dosdevice.
 * 	[90/04/30  15:42:41  grm]
 * 
 * 	Added PORT_foo marcos and dbg_fd.
 * 	[90/04/17  22:44:08  grm]
 * 
 * 	Added trace and vga consts.  Put extern iopl_fd.
 * 	[90/04/05  21:26:06  grm]
 * 
 * 	Started editing as grm.  Using v86 branch.
 * 	[90/03/28  18:34:20  grm]
 * 
 * Revision 2.1.1.5  90/03/22  21:41:53  dorr
 * 	Added media information to the device structure.
 * 	Added vga externs and Fprintf
 * 
 * Revision 2.1.1.4  90/03/19  17:34:21  orr
 * 	Moved the device structure from base.h to this file.
 * 	Added d_state constants.
 * 
 * Revision 2.1.1.3  90/03/13  15:35:19  orr
 * 	Version that gives an A:> prompt and does a DIR.
 * 	No major changes.
 * 
 * 
 * Revision 2.1.1.2  90/03/12  02:17:06  orr
 * 	fix addr so that it masks off high word of segment.
 * 	turn off debug output.
 * 
 * 
 * Revision 1.1  90/03/09  12:00:42  orr
 * Initial revision
 * 
 */

#ifndef	_bios_
#define	_bios_

#include <stdio.h>
#include <mach/thread_status.h>
#include <machine/psl.h>
#include <i386/pio.h>
#include <sys/types.h>

#define VGA_OFF			0
#define VGA_ENABLED		1
#define VGA_DISABLED		2

extern onoff_t vga_state;
extern onoff_t curses_state;

#define DEVICE_NOT_PRESENT		0
#define DEVICE_NOT_INITIALIZED		1
#define DEVICE_INITIALIZED		2
#define DEVICE_INITIALIZATION_FAILED	3

#define	MAX_DEVICES	256

#define	EFL_SAFE	(EFL_CF|EFL_PF|EFL_AF|EFL_ZF|EFL_SF|EFL_DF|EFL_OF|EFL_RF|EFL_IF)
#define	EFL_TSAFE	(EFL_TF|EFL_CF|EFL_PF|EFL_AF|EFL_ZF|EFL_SF|EFL_DF|EFL_OF|EFL_RF|EFL_IF)

#define LEAVE_EIP_ALONE 4

typedef struct device {
	u_int d_state;
	char * d_path;
	int d_fd;

	boolean_t d_absability;

	/* info from the boot block */
	u_char media;		/* media type */
	u_char bps[2];		/* blocks per sector */
	u_char spa;		/* sectors per allocation */
	u_short sectrk;		/* sectors per track */
	u_short heads;		/* heads per track */
	u_char drive;		/* drive number */
	u_short sects;		/* total sectors */
	u_long SECTS;		/* total number of secotrs */

	/* hard disk partition info */
	u_char start_head;
	u_short start_cylsec;	/* starting cylinder and sec */
	u_char partition_type;
	u_char end_head;
	u_short end_cylsec;	/* ending cylinder and sec */
	u_long start_sec_rel;	/* starting sec w/r to beginnng of disk */
	u_long partition_len;	/* partition length */
} * device_t;

typedef	struct {
	u_short	offset;
	u_short	selector;
} * idt_t;

extern thread_t v86_thread;
extern idt_t idt;
extern struct device devices[MAX_DEVICES];
extern struct sgttyb old_tty_state;

extern struct vga_state ega_vga_old_state;

typedef struct i386_thread_state	state_t;

#define Addr_8086(x,y)	(( ((x) & 0xffff) << 4) + ((y) & 0xffff))
#define Addr(s,x,y)	Addr_8086(((s)->x), ((s)->y))

#define Segment(x)	(((x) & 0xff000)>>4)
#define	Offset(x)	((x) & 0xfff)
#define Abs2Segoff(x)	( (((x) & 0xffff0)<<12) | ((x) & 0xf) )

#define Fprintf(args)	reset_tty();fprintf args;set_tty()

#define MAX_IO_PORTS	128
extern u_char io_ports[MAX_IO_PORTS];
#define PORT_ENABLE(x)	io_ports[(x)/8] |= (u_char) (1 << ((x) % 8))
#define PORT_DISABLE(x)	io_ports[(x)/8] &= (u_char)!(1 << ((x) % 8))
#define PORT_OK(x)	(io_ports[(x)/8] & (1 << ((x) % 8)))

extern FILE * dbg_fd;
extern int dd_fd;
extern FILE * dd_stream;
extern int iopl_fd;
extern boolean_t interrupt_bit;

extern	boolean_t cs_switch_needed;
extern int cs_switch_turns;
extern int cs_switch_count;
extern int mon_space;

#define	PENDING_EIP (*((int *)mon_space))

/*
 * Mutex objects.
 */
typedef struct mutex {
	int lock;
	char *name;
} *mutex_t;
extern mutex_t cs_lock;

#define	lock_cs() 					\
	{						\
		while (! mutex_try_lock(cs_lock)) { 	\
			cs_switch_needed = TRUE;	\
			swtch_pri(0);			\
			cs_switch_count++;		\
			cs_switch_needed = FALSE;	\
		}					\
	}

#define	unlock_cs() 					\
	{						\
		mutex_unlock(cs_lock);			\
		if (cs_switch_needed) {			\
			swtch_pri(255);			\
		}					\
	}

#define	lock_mouse() 					\
	{						\
		while (! mutex_try_lock(mouse_lock)) { 	\
			cs_switch_needed = TRUE;	\
			swtch_pri(0);			\
			cs_switch_count++;		\
			cs_switch_needed = FALSE;	\
		}					\
	}

#define	unlock_mouse() 					\
	{						\
		mutex_unlock(mouse_lock);		\
		if (cs_switch_needed) {			\
			swtch_pri(255);			\
		}					\
	}

#ifdef	IDLE_WORK_IN_PROGRESS
#define	lock_idle() 					\
	{						\
		while (! mutex_try_lock(idle_lock)) { 	\
			cs_switch_needed = TRUE;	\
			swtch_pri(0);			\
			cs_switch_count++;		\
			cs_switch_needed = FALSE;	\
		}					\
	}

#define	unlock_idle() 					\
	{						\
		mutex_unlock(idle_lock);		\
		if (cs_switch_needed) {			\
			swtch_pri(255);			\
		}					\
	}
#endif	/* IDLE_WORK_IN_PROGRESS */
#endif	_bios_
