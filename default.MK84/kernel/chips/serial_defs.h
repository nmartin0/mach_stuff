/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	serial_defs.h,v $
 * Revision 2.4  93/11/17  16:14:36  dbg
 * 	Added ANSI function prototypes.
 * 	[93/10/13            dbg]
 * 
 * Revision 2.3  93/03/26  17:58:27  mrt
 * 	Rid of dev_t.
 * 	[93/03/19            af]
 * 
 * Revision 2.2  91/08/24  11:53:23  af
 * 	Created.
 * 	[91/07/07            af]
 * 
 */
/*
 *	File: serial_defs.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/91
 *
 *	Generic console driver for serial-line based consoles.
 */

#ifndef	_CHIPS_SERIAL_DEFS_H_
#define	_CHIPS_SERIAL_DEFS_H_

/*
 * Common defs
 */

#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <chips/busses.h>
#include <device/tty.h>

extern boolean_t
    (*console_probe)(
	vm_offset_t	addr,
	struct bus_device *d);

extern void
    (*console_param)(
	struct tty	*tp,
	int		line);

extern void
    (*console_start)(
	struct tty	*tp);

extern void
    (*console_putc)(
	int	unit,
	int	line,
	int	ch);

extern int
    (*console_getc)(
	int	unit,
	int	line,
	boolean_t wait,
	boolean_t raw);

extern void
    (*console_pollc)(
	int	unit,
	boolean_t poll);

extern int
    (*console_mctl)(
	int	dev,
	int	bits,
	int	how);

extern void
    (*console_softCAR)(
	int	unit,
	int	line,
	boolean_t on);

extern int	cngetc(void);
extern int	cnmaygetc(void);
extern void	cnputc(int);
extern void	cnpollc(boolean_t);
extern void	rcputc(int);

extern struct tty	*console_tty[];
extern int rcline, cnline;
extern int	console;

/* Simple one-char-at-a-time scheme */
extern int
cons_simple_tint(
	int	ttyno,
	boolean_t all_sent);

extern void
cons_simple_rint(
	int	ttyno,
	int	line,
	int	c,
	int	err);

#define	CONS_ERR_PARITY		0x1000
#define	CONS_ERR_BREAK		0x2000
#define	CONS_ERR_OVERRUN	0x4000

#endif	/* _CHIPS_SERIAL_DEFS_H_ */

