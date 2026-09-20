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
 *	Mach DOS com driver support
 *
 * HISTORY: 
 * $Log:	bios_com.c,v $
 * Revision 2.6  92/07/01  14:24:28  grm
 * 	Removed kd.h inclusion.
 * 	[92/06/03            grm]
 * 
 * Revision 2.5  92/03/02  15:47:06  grm
 * 	Moved the interrupt vector table indices into bios_misc.h.
 * 	[92/02/27            grm]
 * 
 * Revision 2.4  92/02/02  23:02:10  rvb
 * 	Changed kd.h include location for first alpha release.
 * 	[92/02/02  14:50:48  grm]
 * 
 * Revision 2.3  91/12/06  15:27:57  grm
 * 	Removed the absolute mon_space code and replaced with the new
 * 	relocatable dosres code.  Rewrote some of the modem code.
 * 	[91/12/06            grm]
 * 
 * Revision 2.2  91/12/05  16:39:43  grm
 * 	Minor changes.  Added a check_interrupt.
 * 	[91/08/09  19:38:12  grm]
 * 
 * 	Modified for use with dbg's new kernel support.
 * 	In the middle of adding the ptty support.
 * 	[91/06/14  11:45:13  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  08:31:43  grm]
 * 
 * 	Mouse support in Windows 3.0 now.
 * 	[91/03/26  19:14:16  grm]
 * 
 * 	Created.
 * 	[91/02/01  13:22:21  grm]
 * 
 */

#include "base.h"
#include "bios.h"
#include "bios_misc.h"

#include <sys/file.h>
#include <sys/ioctl.h>

#define COM1_IVEC	0x0c
#define COM2_IVEC	0x0b

#define com1_int_defined() ((*((u_long *)0x30)) != (Abs2Segoff(IRET_LOCATION)))

int start_com_int = 0;

int		com_interrupt_eip = 0;
static int	com_int_enabled = 0;
boolean_t 	need_com = FALSE;
disable_com_int()
{
	com_int_enabled = 1;
	Debug1((dbg_fd, "disable_com_int\n"));
}

#define	MOUSE_MSG_SIZE 5
#define	COM_BUFFER_SIZE 5
char com_buffer[COM_BUFFER_SIZE] = {0};
int com_head = 0;
int com_tail = 0;
boolean_t com_empty = TRUE;
boolean_t com_full  = FALSE;

void
com_enqueue(ch)
char ch;
{
	int new_tail;
	if (com_full) {
		Debug1((dbg_fd, "Com buffer overflow.\n"));
		return;
	}
	com_buffer[com_tail] = ch;
	com_empty = FALSE;
	Debug1((dbg_fd, "com_enqueue:%x,%d,%d\n",ch&0xff,com_head, com_tail));

	new_tail = com_tail + 1;
	if (new_tail >= COM_BUFFER_SIZE) new_tail = 0;
	com_tail = new_tail;

	if (com_tail == com_head) com_full = TRUE;
	need_com = TRUE;
}

boolean_t com_mouse_full()
{
	int space_left;

	if (com_empty) return (FALSE);
	if (com_full) return (TRUE);

	if (com_head > com_tail) {
		space_left = com_head - com_tail;
	} else {
		space_left = COM_BUFFER_SIZE - (com_tail - com_head);
	}

	if (space_left >= MOUSE_MSG_SIZE) return FALSE;
	return TRUE;
}

char com_dequeue()
{
	char ch;
	if (com_empty) {
		need_com = FALSE;
		return (NULL);
	}
	ch = com_buffer[com_head];
	com_head++;
	if (com_head >= COM_BUFFER_SIZE) com_head = 0;
	if (com_head == com_tail) {
		need_com = FALSE;
		com_empty = TRUE;
	}
	com_full = FALSE;
	Debug1((dbg_fd, "com_dequeue:%x,%d,%d\n",ch&0xff,com_head, com_tail));
	return(ch);
}

com_mouse_event(buttons, dx, dy)
int	buttons;
int	dx;
int 	dy;
{
	static lastdx = 0, lastdy = 0;
	int b = 0;
	char chdx, chdy;
	Debug1((dbg_fd, "COM mouse event: %x, %d, %d\n",
			buttons, dx, dy));
	b |= (buttons & 0x1 ? 0x4 : 0);
	b |= (buttons & 0x2 ? 0x1 : 0);
	b |= (buttons & 0x4 ? 0x2 : 0);

	if (com_mouse_full()) {
		Debug1((dbg_fd, "COM mouse overflow.\n"));
		lastdx += dx;
		lastdy += dy;
		if (lastdx > 127) lastdx = 127;
		if (lastdx < -126) lastdx = -126;
		if (lastdy > 127) lastdy = 127;
		if (lastdy < -126) lastdy = -126;
		return;
	} else if ((lastdx != 0) || (lastdy != 0)) {
		dx += lastdx;
		dy += lastdy;
		if (dx > 127)  dx = 127;
		if (dx < -126) dx = -126;
		if (dy > 127)  dy = 127;
		if (dy < -126) dy = -126;
	}
	lastdx = 0;
	lastdy = 0;

	com_enqueue(0x80 | b);
	dx = -dx;
	dy = -dy;
	chdx = dx;
	chdy = dy;
	com_enqueue(chdx);
	com_enqueue(chdy);
	com_enqueue(0);
	com_enqueue(0);

	if (com1_int_defined()) {
	        queue_interrupt(COM1_INT_VEC, 5, FALSE);
		check_interrupt(FALSE);
	}
}

/*
 * Com support to do mouse stuff
 */
u_int do_com1_port(inout, port, val, byte_p, state)
	boolean_t inout;
	u_int port;
	u_int val;
	boolean_t byte_p;
	state_t * state;
{
	static boolean_t dlab = FALSE;
	static u_char tx_buffer = 0;	/* 3f8 dlab = 0 write */
	static u_char rx_buffer = 0;	/* 3f8 dlab = 0 read */
	static u_char divl_lsb = 0x0;	/* 3f8 dlab = 1 */
	static u_char divl_msb = 0x5;	/* 3f9 dlab = 1 */
	static u_char inten_reg = 0;	/* 3f9 dlab = 0 */
	static u_char intid_reg;	/* 3fa */
	static u_char lnctl_reg = 0x3;	/* 3fb */
	static u_char mdctl_reg = 0xb;	/* 3fc */
	static u_char lnsts_reg = 0x60;	/* 3fd */
	static u_char mdsts_reg;	/* 3fe */

	static u_char last_write = 0;
	
	u_int ret = 0;

	Debug0((dbg_fd,"COM called from 0x%x\n", Addr(state, cs,eip)));
        

	if(inout) {
		Debug0((dbg_fd,"\rinb 0x%x = 0x%x\n", port, inb(port)));
	}else{
		Debug0((dbg_fd,"\routb 0x%x <-- 0x%x\n", port, val));
	}

	switch(port) {
	    case 0x3f8: {
		    if (dlab) {
			ret = 0x60;
		    }else{
			    if(inout) {
				    /* in from com */
				    Debug0((dbg_fd,"\rbios_com: read of inbuf\n"));				    
				    ret = com_dequeue();
				    lnsts_reg = 0x60;
				    return(ret);
			    }else{
				    /* out to com */
				    last_write = val;
				    if (val == 0x73) {
					    Debug0((dbg_fd,"\rbios_com: wt of obuf\n"));
					    com_dequeue();
					    com_enqueue(0x8f);
					    start_com_int++;
					    /* com_intr(TRUE); */
				    }
				    lnsts_reg = 0x60;
			    }
		    }
		    break;
	    }
	    case 0x3f9: {
		    if (dlab) {
			ret = 0x0;
			dlab = FALSE;
		    }else{
			ret = 0x0;
		    }
		    break;
 	    }
	    case 0x3fa: {
		    break;
	    }
	    case 0x3fb: {
		    if (inout) {
			    ret = lnctl_reg;
		    }else{
			    lnctl_reg = (u_char)val;
			    dlab = (lnctl_reg & 0x80);
			    lnsts_reg = 0x60;
		    }
		    break;
	    }
	    case 0x3fc: {
		    ret = 0x83;
		    break;
	    }
	    case 0x3fd: {
	            ret = lnsts_reg;
		    lnsts_reg = 0x1;
		    break;
	    }
	    case 0x3fe: {
		    ret = 0x1;
		    break;
	    }
	    default: {
		    fprintf(dbg_fd,"\rbios_misc: bad com port 0x%x\n",port);
		    break;
	    }
	}

	return(ret);
}

/*
 * Virtual Modem's code and vars
 *
 */

struct mutex com_lock_data = { 0, 0};
mutex_t com_lock = &com_lock_data;

#define	lock_com() 					\
	{						\
		while (! mutex_try_lock(com_lock)) { 	\
			cs_switch_needed = TRUE;	\
			swtch_pri(0);			\
			cs_switch_count++;		\
			cs_switch_needed = FALSE;	\
		}					\
	}

#define	unlock_com() 					\
	{						\
		cs_switch_turns++;			\
		if (cs_switch_turns > 10) 		\
			cs_switch_turns = 10;		\
		mutex_unlock(com_lock);			\
		if (cs_switch_needed || 		\
			(cs_switch_turns==0)) {		\
			swtch_pri(255);			\
		}					\
	}

boolean_t vmodem_in_isr = FALSE;

#define IN	TRUE

#define RECLNSTS_INT	0x08
#define RECDTARDY_INT	0x04
#define TXREMPTY_INT	0x02
#define MDMSTS_INT	0x01
int vm_int_map = MDMSTS_INT;

u_char int_array[6] = {1,4,4,4,4,-1};
int int_index = 0;

/* Interrupt Enable Register */
#define	IEN_DATA_AVAIL	1
#define	IEN_TX_EMPTY	2
#define IEN_REC_LN_STS	4
#define	IEN_MDM_STS	8

/* Interrupt Flags Register */
#define	IFL_NO_PENDING	1
#define IFL_MDM_STS	0
#define IFL_TX_EMPTY	2
#define IFL_DATA_AVAIL	4
#define	IFL_REC_LN_STS	6

/* Modem Control Register */
#define	MCR_DTR		1
#define	MCR_RTS		2
#define MCR_DSINT	8
#define MCR_LOOP	16

/* Modem Status Register */
#define MSR_DCTS	1
#define MSR_DDSR	2
#define	MSR_TERI	4
#define MSR_DDCD	8
#define MSR_CTS		0x10
#define MSR_DSR		0x20
#define MSR_RI		0x40
#define MSR_DCD		0x80
#define MSR_ANY_CHANGED	(MSR_DCTS | MSR_DDSR | MSR_TERI | MSR_DDCD)

/* Line Control Register */
#define LCR_WL0		0x01
#define LCR_WL1		0x02
#define	LCR_STB		0x04
#define LCR_PEN		0x08
#define LCR_EPS		0x10
#define LCR_STICK_PAR	0x20
#define LCR_SET_BRK	0x40
#define LCR_DLAB	0x80

/* Line Status Register */
#define	LSR_DR		0x01
#define	LSR_OVERRUN_ER 	0x02
#define LSR_PARITY_ER	0x04
#define LSR_FRAMING_ER	0x08
#define LSR_BREAK_INTR	0x10
#define	LSR_THRE	0x20
#define LSR_TSRE	0x40

/* Com port 2's ports */
boolean_t vm_dlab = FALSE;
u_char vm_txb = 0;	/* 2f8 vm_dlab = 0 write */
u_char vm_rxb = 0;	/* 2f8 vm_dlab = 0 read */
u_char vm_dlsb = 0x0;	/* 2f8 vm_dlab = 1 */
u_char vm_dmsb = 0x5;	/* 2f9 vm_dlab = 1 */
u_char vm_ien = 0;	/* 2f9 vm_dlab = 0 */	/* No interrupts enabled */
u_char vm_iid = 0x1;	/* 2fa */		/* No interrupt pending	*/
u_char vm_lcr = 0x2;	/* 2fb */		/* 7 bpc no par 1 stop	*/
u_char vm_mcr = 0x1;	/* 2fc */		/* DTR = 1 int enabled	*/
u_char vm_lsr = 0x60;	/* 2fd */		/* TEMT = 1 THRE = 1	*/
u_char vm_msr = 0xb0;	/* 2fe */	/* DCD = 1 RI = 0 DSR = 1 CTS = 1 */

void cause_vmodem_intr(main_thread)
	boolean_t main_thread;
{
	queue_interrupt(COM2_INT_VEC, 1, main_thread);
	check_interrupt(FALSE);
}

u_char vmodem_read()
{
	static u_char ch = 'a';
	u_char ret;
	/* read from pty's master side */
	/* XXX just a hack for right now. */
	ret = ch++;
	if (ch > 'z') ch = 'a';
	return(ret);
}

void vmodem_write(ch)
	u_char ch;
{
	/* dump it into the pty's master side */
}

void set_vm_iid()
{
	if (vm_int_map & RECLNSTS_INT) {
		vm_iid = 0x6;
	}else if (vm_int_map & RECDTARDY_INT) {
		vm_iid = 0x4;
	}else if (vm_int_map & TXREMPTY_INT) {
		vm_iid = 0x2;
	}else if (vm_int_map & MDMSTS_INT) {
		vm_iid = 0x0;
	}else{
		vm_iid = 0x1;
	}
	Debug0((dbg_fd,"bios_com: vmodem set_vm_iid vm_int_map = 0x%x vm_iid = 0x%x\n",
		vm_int_map, vm_iid));
}

/* state machine */
#define NO_CONTACT	0
#define GAVE_DCD	3
#define GAVE_DTR	1
#define GAVE_CTS	2
#define GAVE_CHAR	4

int vmodem_state = NO_CONTACT;

void possible_intr()
{

	switch (vmodem_state) {
	    case NO_CONTACT:
		Debug0((dbg_fd,"bios_com: poss_intr NO_CONTACT "));
		if ((vm_ien & IEN_MDM_STS) &&
		    (vm_msr & MSR_ANY_CHANGED)) {
			vm_iid = IFL_MDM_STS;
			Debug0((dbg_fd,"interrupting.\n"));
			cause_vmodem_intr(FALSE);
		}else{
			Debug0((dbg_fd,"not interrupting.\n"));
			/* XXX Hack alert!! bwoop bwoop bwoop  */
			if (vm_ien == 0xf) {
#ifndef DATA_CARRIER_INTERUPTS
				vm_iid = IFL_MDM_STS;
				vm_msr = MSR_DCD|MSR_DDCD;
				vmodem_state = GAVE_DCD;
#endif
#ifdef BREAK_STRAT
				vm_iid = IFL_DATA_AVAIL;
				vm_lsr |= LSR_DR;
				vmodem_state = GAVE_CHAR;
#endif				
				Debug0((dbg_fd,"bios_com: vm_iid forcing intr\n"));
				cause_vmodem_intr(FALSE);
			}
		}
		break;
	    case GAVE_CTS:
		if (vm_ien & IEN_TX_EMPTY)
			Debug0((dbg_fd,"bios_com: forcing tx empty intr\n"));
			cause_vmodem_intr(FALSE);
		break;
	    default:
		Debug0((dbg_fd,"bios_com: poss_intr default.\n"));
		break;
	}
}


u_int do_com2_port(inout, port, val, byte_p, state)
	boolean_t inout;	/* true => in */
	u_int port;
	u_int val;
	boolean_t byte_p;
	state_t * state;
{
	static u_char last_write = 0;

	u_int ret = 0;

	lock_com();

	if (inout == IN) {
		Debug0((dbg_fd,"bios_com: COM2 - in%c 0x%x cs:ip = 0x%4.4x:%4.4x\n",
			(byte_p ? 'b' : 'w'), port, (u_short)state->cs, 
			(u_short)state->eip));
	}else{
		Debug0((dbg_fd,"bios_com: COM2 - out%c 0x%x, 0x%x cs:eip = 0x%4.4x:%4.4x\n",
			(byte_p ? 'b' : 'w'), port, val, (u_short)state->cs, 
			(u_short)state->eip));
	}

	Debug0((dbg_fd,"com2: index %d vm_iid %x vm_int_map %x vm_ien %x isr %x\n",
		int_index, vm_iid, vm_int_map, vm_ien, vmodem_in_isr));

	switch (port) {
	    /*
	     * vm_txb, vm_rxb, vm_dlsb
	     */
	    case 0x2f8: {
		    if (vm_dlab == 0) {
			    if (inout == IN) {
				    Debug0((dbg_fd,"bios_com: vmodem read rx buf %c\n",
					    vm_rxb));
				    vm_rxb = vmodem_read();
				    vm_lsr = 0x60;
				    if ((vm_int_map == RECDTARDY_INT) &&
					(vmodem_in_isr == TRUE)) {
					    vm_int_map = 0;
					    set_vm_iid();
					    vmodem_in_isr = FALSE;
				    }
				    ret = vm_rxb;
			    }else{
				    Debug0((dbg_fd,"bios_com: vmodem write tx buf %c\n",
					    val));
				    vm_txb = (u_char)val;
				    vmodem_write(val);
				    vm_lsr = 0x60;
			    }
		    }else{
			    if (inout == IN) {
				    Debug0((dbg_fd,"bios_com: vmodem vm_dlsb read\n"));
			    }else{
				    Debug0((dbg_fd,"bios_com: vmodem vm_dlsb write\n"));
			    }
			    ret = 0x60;
		    }
		    break;
	    }
	    /*
	     * vm_ien, vm_dmsb
	     */
	    case 0x2f9: {
		    if (vm_dlab == 0) {
			    if (inout == IN) {
				    Debug0((dbg_fd,"bios_com: vmodem vm_ien read 0x%x\n",
					    vm_ien));
				    ret = vm_ien;
			    }else {
				    Debug0((dbg_fd,"bios_com: vmodem vm_ien written 0x%x\n",val));
				    vm_ien = val;
				    possible_intr();
			    }
		    }else{
			    if (inout == IN) {
				    Debug0((dbg_fd,"bios_com: vmodem vm_dmsb read\n"));
			    }else{
				    Debug0((dbg_fd,"bios_com: vmodem vm_dmsb write\n"));
			    }
			    ret = 0;
		    }
		    break;
	    }
	    /*
	     * vm_iid
	     */
	    case 0x2fa: {
		    if (inout == IN) {
			    Debug0((dbg_fd,"bios_com: vmodem vm_iid read 0x%x\n",
				    vm_iid));
#ifdef BREAK_STRATEGY
			    if (vmodem_state == GAVE_CHAR) {
				    vm_iid = IFL_REC_LN_STS;
				    vm_lsr |= LSR_BREAK_INTR;
			    }
#endif

			    ret = vm_iid;

			    if (vmodem_state == GAVE_CTS) {
				    vm_iid = IFL_TX_EMPTY;
				    possible_intr();
			    }
		    }else{
			    Debug0((dbg_fd,"bios_com: vmodem vm_iid write 0x%x ?XXX?\n",
				    val));
		    }
		    break;
	    }
	    /*
	     * vm_lcr
	     */
	    case 0x2fb: {
		    if (inout == IN) {
			    Debug0((dbg_fd,"bios_com: vmodem vm_lcr read 0x%x\n",
				    vm_lcr));
			    ret = vm_lcr;
		    }else{
			    vm_lcr = (u_char)val;
			    Debug0((dbg_fd,"bios_com: vmodem vm_lcr write 0x%x\n",
				    vm_lcr));
			    if ((vm_dlab ^ vm_lcr) & 0x80) {
				    Debug0((dbg_fd,"bios_com: vmodem vm_dlab turned %s\n",
					    ((vm_lcr & 0x80)? "on" : "off")));
			    }
			    vm_dlab = (vm_lcr & 0x80);
			    vm_lsr |= 0x60;
		    }
		    break;
	    }
	    /*
	     * vm_mcr
	     */
	    case 0x2fc: {
		    if (inout == IN) {
			    Debug0((dbg_fd,"bios_com: vmodem vm_mcr read 0x%x\n",
				    vm_mcr));
			    ret = vm_mcr;
		    }else{
			    int changed;
			    boolean_t intr = FALSE;

			    changed = vm_mcr ^ (u_char)val;
			    vm_mcr = (u_char)val;
			    Debug0((dbg_fd,"bios_com: vmodem vm_mcr written 0x%x\n",
				    vm_mcr));
			    /* XXX Does this cause two interrupts ? */
			    if ((changed & MCR_DTR) && (vm_mcr & MCR_DTR)) {
				    vm_msr |= MSR_DSR;
				    vm_msr |= MSR_DDSR;
				    intr = TRUE;
			    }
			    if ((changed & MCR_RTS) && (vm_mcr & MCR_RTS)) {
				    vm_msr |= MSR_CTS;
				    vm_msr |= MSR_DCTS;
				    intr = TRUE;
			    }
			    if (intr)
				    possible_intr();
			    
		    }
		    break;
	    }
	    /*
	     * vm_lsr
	     */
	    case 0x2fd: {
		    if (inout == IN) {
			    Debug0((dbg_fd,"bios_com: vmodem vm_lsr read 0x%x\n",
				    vm_lsr));
			    ret = vm_lsr;
			    vm_lsr &= ~(LSR_OVERRUN_ER|LSR_PARITY_ER|LSR_FRAMING_ER|
					LSR_BREAK_INTR);
			    if (vm_iid == IFL_REC_LN_STS)
				    vm_iid = IFL_NO_PENDING;
		    }else{
			    Debug0((dbg_fd,"bios_com: vmodem vm_lsr written 0x%x ?XXX?\n",
				    (u_char)val));
		    }
		    break;
	    }
	    /*
	     * vm_msr
	     */
	    case 0x2fe: {
		    if (inout == IN) {
			    Debug0((dbg_fd,"bios_com: vmodem vm_msr read 0x%x\n",
				    vm_msr));
			    ret = vm_msr;

			    if ((vm_iid == IFL_MDM_STS) &&
				(vmodem_state == GAVE_DCD)) {
				    vm_msr = MSR_DCD|MSR_DSR|MSR_DDSR;
				    vmodem_state = GAVE_DTR;
			    }else if ((vm_iid == IFL_MDM_STS) &&
				      (vmodem_state == GAVE_DTR)) {
				    vm_msr = MSR_DCD|MSR_DSR|MSR_CTS|MSR_DCTS;
				    vmodem_state = GAVE_CTS;
				    /* vm_iid = IFL_NO_PENDING; */
			    }else if (vmodem_state == GAVE_CTS) {
				    vm_iid = IFL_NO_PENDING; 
			    }else{

				    /* clear bits 0-3 */
				    vm_msr &= 0xf0;

				    if (vm_iid ==  IFL_MDM_STS) {
					    vm_iid = IFL_NO_PENDING;
					    vmodem_in_isr = FALSE;
				    }
			    }
		    }else{
			    Debug0((dbg_fd,"bios_com: vmodem vm_msr written 0x%x ?XXX?\n",
				    (u_char)val));
		    }
		    break;
	    }
	    default: {
		    Debug0((dbg_fd,"bios_com: vmodem unknown port 0x%x\n",port));
		    break;
	    }
	}

	unlock_com();

	return(ret);
}

void com2_loop()
{
	int counter = 0;
	u_long * cvec_loc;
	u_short * sp;
	state_t state;
	int state_count;
	int com_milli_pause = 5000;
	
	millisecond_wait(5000);

	Debug0((dbg_fd,"bios_com: vmodem init vm_iid = 0x%x vm_int_map = 0x%x\n",
		vm_iid, vm_int_map));

	while(TRUE){
		millisecond_wait(com_milli_pause);
#ifdef NOTDEF			
		lock_com();
		if (vm_ien && (vm_iid == IFL_NO_PENDING)) {
			Debug0((dbg_fd,"VMODEM wakeup and interrupting.\n"));
			vm_msr = 0xbb;
			vm_int_map = int_array[int_index++];
			if (vm_int_map == -1) {
				Debug0((dbg_fd,"Finished interrupting.\n"));
				exit_dos();
			}
			set_vm_iid();
			vmodem_in_isr = TRUE;
			cause_vmodem_intr(FALSE);
		}else{
			Debug0((dbg_fd,"VMODEM wakeup can't interrupt\n"));
		}
		unlock_com();
#endif NOTDEF
	}
}
