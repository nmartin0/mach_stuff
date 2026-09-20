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
 *	V86 BIOS keyboard routine emulation
 *
 * HISTORY: 
 * $Log:	bios_kbd.c,v $
 * Revision 2.12  92/07/01  14:24:42  grm
 * 	Ifdef out idle work.
 * 	[92/07/01            grm]
 * 	Started adding idle locking code.
 * 	[92/06/30  13:42:18  grm]
 * 
 * 	Replaced kev_type with int (jvt@kampi.hut.fi).  This fixes the
 * 	keyboard hanging problem.  Turn off pc speaker when pausing or
 * 	exiting mdos using speaker_off().
 * 	[92/06/03            grm]
 * 
 * Revision 2.11  92/05/22  15:59:11  grm
 * 	Minor bug fix.
 * 	[92/05/21            grm]
 * 
 * Revision 2.10  92/04/29  16:31:08  grm
 * 	Modified override vectors code.
 * 	[92/04/29            grm]
 * 
 * Revision 2.9  92/04/14  13:20:04  grm
 * 
 * 	 Changed device interface back to using /dev/iopl for release.
 * 	[92/04/06            grm]
 * 	Mach3 Version with device map instead of opening /dev/iopl.  This
 * 	will be changed for the next release, but I wanted a version in
 * 	RCS.
 * 	[92/03/27            grm]
 * 
 * Revision 2.8  92/03/02  15:47:12  grm
 * 	Fixed Ravi's exit_dos() recursion bug.
 * 	[92/02/27            grm]
 * 	Moved the interrupt vector table indices into bios_misc.h.
 * 	[92/02/27            grm]
 * 	Added do_key_int.  Used by the check_interrupt routine in
 * 	bios_misc.c to check for error with interrupt table count field.
 * 	[92/02/20            grm]
 * 
 * Revision 2.7  92/02/14  17:44:33  grm
 * 	Moved some routines so that compiler wouldn't complain.
 * 	[92/02/12  16:12:45  grm]
 * 
 * 	Moved the console initializing code from v86.c to here.  Added
 * 	profiling code, which is conditionalized with PROFILING.  Fixed
 * 	the pause problem introduced with the OSF changes.
 * 	[92/02/12            grm]
 * 	Merged in the OSF Single Server code.  Added dev_kbd_close
 * 	function which resets the keyboard mode.  This was needed for the
 * 	OSF Single Server.
 * 	[92/02/11            grm]
 * 
 * Revision 2.6  92/02/03  14:24:38  rvb
 * 	Clean Up
 * 
 * Revision 2.5  92/02/02  23:02:18  rvb
 * 	Removed the exit(0)'s and replaced them with exit_dos().
 * 	[92/01/27            grm]
 * 
 * Revision 2.4  91/12/06  15:28:12  grm
 * 	Replaced the absolute mon_space code with the relative dosres
 * 	code.
 * 	[91/12/06            grm]
 * 
 * Revision 2.3  91/12/05  16:40:23  grm
 * 	Added code to reset keyboard state after a pause, when keyboard
 * 	is in scan code mode.  Removed the suspend_monitor routine (it
 * 	was never called).  Streamlined the read_dev_kbd procedure since
 * 	the keyboard thread always knows when mdos has been paused.
 * 	[91/12/04            grm]
 * 	Minor change.  Changed constant to a definition.
 * 	[91/08/09  19:40:53  grm]
 * 
 * 	Rewrote most of the code.  Works with a resident ISR in 
 * 	Mach area of dos memory (0xc9000).  This lets programs
 * 	use the keyboard correctly.  Keybuf moved to use the 
 * 	scratch pad ram's keybuf area.  Uses spram's pointers too.
 * 	[91/07/16  17:37:37  grm]
 * 
 * 	Suspend monitor made into procedure.
 * 	[91/06/28  18:08:49  grm]
 * 
 * 	Modified to use dbg's new v86 kernel support.
 * 	Generates a keyboard break interrupt on cntl-c's.
 * 	Need to change this, it's wrong.
 * 	[91/06/14  11:49:59  grm]
 * 
 * 	New Copyright.  Change line status support added.
 * 	[91/05/28  14:46:31  grm]
 * 
 * 	Rick's New self adjusting timer changes.
 * 	Started to put in advanced pausing feature.
 * 	[91/04/30  13:29:38  grm]
 * 
 * 	Put in the pausing feature.  Cntl-Alt-Z
 * 	[91/03/26  19:02:49  grm]
 * 
 * 	Major changes.  Mach3 version.
 * 	[91/02/01  13:25:27  grm]
 * 
 * 	Scan code changes #1.
 * 	[90/12/04  14:50:13  grm]
 * 
 * 	Added enhanced keyboad operations.  Changed 
 * 	debug to Kdebugs.
 * 	[90/11/09  20:59:41  grm]
 * 
 * 	Moved some constants and structures to 
 * 	bios_kbd.h.  Added support for extended
 * 	character set (function keys, alt-letter
 * 	combinations, etc.).
 * 	[90/10/11  20:26:03  grm]
 * 
 * 	Added vgets and modified key_special.
 * 	Keyboard_thread now opens /dev/iopl.
 * 	[90/10/04  21:00:51  grm]
 * 
 * 	Modifier scan codes work.  IO port 0x64
 * 	support added.
 * 	[90/05/25  16:09:44  grm]
 * 
 * 	Added support for eumulating hardware
 * 	keyboard interrupts.
 * 	[90/05/08  17:20:17  grm]
 * 
 * 	Added the keyboard_loop thread routine to scan the
 * 	keyboard at all times and deal with the keystrokes
 * 	in real time (ie, ^G, '^'), and the option of 
 * 	expanding this to utilize interrupts.
 * 	[90/04/30  15:35:15  grm]
 * 
 * 	Changed fprintfs to use dbg_fd instead of stderr.
 * 	[90/04/17  22:31:58  grm]
 * 
 * 	Started editing as grm.  Moved to v86 branch
 * 	[90/03/28  18:35:43  grm]
 * 
 * Revision 2.1.1.4  90/03/22  21:46:46  dorr
 * 	Added routines to take the tty into and out of raw mode.
 * 	Created an exit_dos routine.  Changed some debug stuff
 * 	to use Fprintfs.
 * 
 * Revision 2.1.1.3  90/03/13  15:32:16  orr
 * 	Version that does an A:> prompt and a DIR.
 * 	bios_kbd_init saves the initial tty state
 * 	and turns off echo and turns on raw mode.
 * 	Control-g resets the tty and exits the program.
 * 	Added peeked and peekc which are used so
 * 	the get_keyboard_status thinks it didn't really
 * 	remove char from the input queue.
 * 
 * Revision 2.1.1.2  90/03/12  02:12:15  orr
 * 	first working version.  route all debug output through
 * 	dbg_fd.  use dosdevice for i/o
 * 
 * Revision 2.1.1.1  90/03/12  01:14:20  orr
 *
 * Revision 1.1  90/03/09  12:36:51  orr
 * Initial revision
 * 
 * 
 */

#include "base.h"
#include "bios.h"

#include <sys/file.h>
#include <sys/ioctl.h>
#include <signal.h>

#include <sys/time.h>

#include "bios_kbd.h"
#include "bios_video_common.h"
#include "bios_misc.h"

#include "dos.h"

#define Keyboard_Control_Byte_1	((u_char *) 0x417)
#define Keyboard_Control_Byte_2	((u_char *) 0x418)

/* pc sourcebook pg 407 */
#define Keyboard_inhibited	0x10
#define Command_byte_last	0x08
#define Input_buffer_full	0x02
#define Output_buffer_full	0x01

/* read only 0x64 */
u_char kbd_status_register = 0x1c;
/* last 0x64 write */
u_char kbd_command_byte;
/* last 0x60 write */
u_char kbd_data_byte;
/* read only 0x60 */
u_char kbd_output_buffer = 0;

#define Buffer_base	0x400
#define Start_of_buffer	0x1e
#define End_of_buffer	0x3e

u_short * buffer_head = (u_short *)0x41a;
u_short * buffer_tail = (u_short *)0x41c;
u_short * keybuf = (u_short *)0x41e;

#define BUF_HEAD	(Buffer_base + *buffer_head)
#define BUF_TAIL	(Buffer_base + *buffer_tail)
#define BUF_START	(Buffer_base + Start_of_buffer)
#define BUF_END		(Buffer_base + End_of_buffer)

u_long * keyint_vec_loc = (u_long *)0x24;
u_long keyint_vec_val = 0;

u_char * spram_kcbyte_1 = Keyboard_Control_Byte_1;
u_char * spram_kcbyte_2 = Keyboard_Control_Byte_2;

u_char internal_kcbyte_1;
u_char internal_kcbyte_2;

int key_mode = KS_NORM;

int dd_fd = -1;	/* /dev/console file descriptor */

struct sgttyb old_tty_state;

struct queue_type key_q;

struct sgttyb old_tty_state;

boolean_t mon_intercept = FALSE;
boolean_t reset_kbd_dev = FALSE;

int total_entries = 0;

#define UP_ALT	0xb8
#define UP_CTRL	0x9d

extern int vectors_overridden;

extern int errno;
extern int debug_toggle;
extern onoff_t debug_state;
boolean_t kbd_on = FALSE;
extern int exit_index;

extern vm_address_t mach_planes;
extern vm_address_t dos_planes;

extern boolean_t reset_mouse_dev;
extern int mouseFd;

extern u_short com_psp;

#ifdef PROFILING
extern boolean_t profiling;
#endif PROFILING

#ifdef	IDLE_WORK_IN_PROGRESS
extern mach_port_t idle_port;
extern boolean_t idling;
extern mutex_t idle_lock;
#endif	/* IDLE_WORK_IN_PROGRESS */

/*
 * Scan code keyboard code vars
 */
#define MAXKEVENTS	32
kd_event kevents[MAXKEVENTS];
int kbdFd;

/*
 * Start of Routines
 */

void append_entry(name, ch, time)
	struct queue_type *name;
	int ch;
	long time;
{
	struct queue_entry *new_entry;

	Kdebug0((dbg_fd,"append_entry ch 0x%x, time 0x%x.\n", ch , time));
	new_entry = (struct queue_entry *)malloc(sizeof(struct queue_entry));
	new_entry->entry = ch;
	new_entry->time = time;
	new_entry->next = NULL;

	if (name->tail != NULL) {
		name->tail->next = new_entry;
		name->tail = new_entry;
	} else {
		name->head = new_entry;
		name->tail = new_entry;
	}

	total_entries++;
}

void delete_entry(name, ch, time)
	struct queue_type *name;
	int * ch;
	long * time;
{
	struct queue_entry *tmp_entry;

	if (name->head != NULL) {
		*ch = name->head->entry;
		*time = name->head->time;
		tmp_entry = name->head->next;
		free(name->head);
		name->head = tmp_entry;
		if (name->head == NULL)
			name->tail = NULL;
		total_entries--;
	} else {
		ch = NULL;
	}

	Kdebug0((dbg_fd,"delete entry ch 0x%x, time 0x%x.\n", *ch , *time));
	return;
}

void first_entry(ch, time)
	int * ch;
	long * time;
{
	if (key_q.head != NULL) {
		*ch = key_q.head->entry;
		*time = key_q.head->time;
	}else{
		*ch = NULL;
	}
}

boolean_t empty_entry(name)
	struct queue_type *name;
{
	return(name->head == NULL);
}

void enqueue(ch)
	u_short ch;
{
	int next_pos = *buffer_tail + 2;

	if (next_pos >= End_of_buffer)
		next_pos = Start_of_buffer;
	if (next_pos == *buffer_head){
		/* beep() */
	} else {
		keybuf[(*buffer_tail - Start_of_buffer)>>1] = ch;
		*buffer_tail = next_pos;
	}
	Debug0((dbg_fd,"enqueue: ch=0x%x, tail = %x\n",ch,next_pos));

}


u_short dequeue()
{
	u_short ch;

	for(;*buffer_head == *buffer_tail;) {
#ifdef WRONG
		if ((kbd_output_buffer) || (!(empty_entry(&key_q)))) {
			int scan_code;
			if (kbd_output_buffer) {
				scan_code = kbd_output_buffer;
				kbd_output_buffer = 0;
				
			} else scan_code = delete_entry(&key_q);
			Kdebug1((dbg_fd,"bios_kbd: dequeue scan_code = 0x%x\n",
						 scan_code));
			(void) decode_sc_and_add_ch(scan_code);
		}
#endif WRONG
		unlock_cs();
		lock_cs();
	}
	ch = keybuf[(*buffer_head - Start_of_buffer)>>1];
	*buffer_head += 2;
	if (*buffer_head >= End_of_buffer)
		*buffer_head = Start_of_buffer;

	Debug0((dbg_fd,"dequeue: ch=0x%x, head = %x\n",ch, buffer_head));
	return(ch);
}

u_short decode_sc();

u_short peek_queue()
{
	u_short ch;

	if (*buffer_head != *buffer_tail) {
		ch = keybuf[(*buffer_head - Start_of_buffer)>>1];
	} else {
		ch = NULL;
	}

	Kdebug1((dbg_fd,"bios_kbd: peek thinks ch = 0x%x\n", ch));
	return(ch);
}

void init_tty ()
{
	struct sgttyb tty_state;
	int ret;

	if ((dd_fd = open("/dev/console", O_RDWR)) < 0) {
		fprintf(dbg_fd, "couldn't open console file\n");
		exit_index = 23;
		exit_dos();
	}

	ret = ioctl(dd_fd, TIOCGETP, &tty_state);
	if (ret < 0) {
		fprintf(dbg_fd,"init_tty: ioctl failed. 0x%x",errno);
		exit_dos();
	}
	Kdebug1((dbg_fd,"mon: sg_flags = %x\n",tty_state.sg_flags));
	old_tty_state = tty_state;
}

void set_tty ()
{
	struct sgttyb tty_state;
	int ret;

	ret = ioctl(dd_fd, TIOCGETP, &tty_state);
#if	0
	if (ret < 0) {
		fprintf(dbg_fd,"set_tty: first ioctl failed. 0x%x",errno);
		exit_dos();
	}
#endif
	tty_state.sg_flags |= RAW;
	tty_state.sg_flags &= ~ECHO;
	ret = ioctl(dd_fd, TIOCSETP, &tty_state);
#if	0
	if (ret < 0) {
		fprintf(dbg_fd,"set_tty: second ioctl failed. 0x%x",errno);
		exit_dos();
	}
#endif
}

void reset_tty ()
{
	ioctl(dd_fd, TIOCSETP, &old_tty_state);
}

void dev_kbd_init()
{
	int ret;
	int mode;

	Kdebug0((dbg_fd,"mon_kbd: dev_kbd_init()\n"));

	init_tty();
	set_tty();

	if ((kbdFd = open("/dev/kbd",O_RDONLY)) <= 0) {
		fprintf(dbg_fd,"error opening /dev/kbd\n");
		exit_index = 8;
		exit_dos();
	}
	mode = KB_EVENT;

	ret = ioctl (kbdFd,KDSKBDMODE,&mode);
	if (ret < 0) {
		fprintf(dbg_fd,"error with kbd ioctl. 0x%x\n",errno);
		exit_index = 22;
		exit_dos();
	}

	kbd_on = TRUE;
}

void dev_kbd_close()
{
	int ret;
	int mode;

	Kdebug0((dbg_fd,"mon_kbd: dev_kbd_close()\n"));

	mode = KB_ASCII;

	ret = ioctl (kbdFd,KDSKBDMODE, &mode);
	if (ret < 0) {
		fprintf(dbg_fd,"error with kbd ioctl. 0x%x\n",errno);
		/* Ravi's bug -- don't call exit_dos() */
		exit(-1);
	}

	close(kbdFd);
	close(dd_fd);
}

void bios_kbd_init()
{

#ifdef NO_INIT
	*buffer_head = Start_of_buffer;
	*buffer_tail = End_of_buffer;
#endif NO_INIT

	key_q.head = NULL;
	key_q.tail = NULL;
}

void bios_kbd_exit()
{
	struct queue_entry *curr;
	struct queue_entry *next;
	int val;
	for (curr = key_q.head; curr != NULL; curr = next) {
		next = curr->next;
		free(curr);
	}

	/*
	 * Turn off speaker!
	 */
	val = inb(0x61);
	outb(0x61, val & ~(0x3));

	dev_kbd_close();
}

boolean_t bios_kbd_fn(state)
	state_t *state;
{
	lock_cs();
	switch (HIGH(state->eax)) {
	    case KBD_READ: {
		    u_short ch;
		    
		    Kdebug1((dbg_fd,"mon:kbd_read char.\n"));
		    
		    ch = dequeue();
		    
		    Kdebug0((dbg_fd,"ch = %c\n",ch));
		    
		    SETHIGH(&(state->eax),HIGH(ch));
		    SETLOW(&(state->eax),LOW(ch));
		    
		    unlock_cs();
		    return(UNCHANGED);
	    }
	    case KBD_GET_KEYBOARD_STATUS: {
		    int n;
		    u_short ch;
		    
		    ch = peek_queue();
		    Kdebug1((dbg_fd,"bios_kbd: peek = 0x%x\n", ch));
		    
		    if (ch == NULL) {
			    SETWORD(&(state->eax), 0);
			    state->efl |= EFL_ZF;
		    } else {
			    SETHIGH(&(state->eax),HIGH(ch));
			    SETLOW(&(state->eax),LOW(ch));
			    state->efl &= ~EFL_ZF;
		    }
		    
		    Kdebug1((dbg_fd,"mon: kbd_get_status: %x\n",
			     state->eax));
		    
		    unlock_cs();
		    return(UNCHANGED);
	    }
	    case KBD_GET_KEYBOARD_FLAGS: {
		    Kdebug1((dbg_fd,"mon: kbd_get_flags\n"));
		    SETLOW(&(state->eax), *spram_kcbyte_1);
		    unlock_cs();
		    return(UNCHANGED);
	    }
	    case KBD_SET_REPEAT_RATE: {
		    Kdebug1((dbg_fd,"mon: kbd_set_repeat\n"));
		    break;
	    }
	    case KBD_SET_KEYCLICK: {
		    Kdebug1((dbg_fd,"mon: kbd_set_keyclick\n"));
		    break;
	    }
	    case KBD_PUSH_C_AND_S: {
		    Kdebug1((dbg_fd,"mon: kbd_push_c&s\n"));
		    break;
	    }
	    case KBD_READ_EN_CHAR: {
		    u_short ch;
		    
		    ch = dequeue();
		    
		    Kdebug1((dbg_fd,"\rmon:kbd_en_read char = %c\n",ch));
		    
		    SETHIGH(&(state->eax),HIGH(ch));
		    SETLOW(&(state->eax),LOW(ch));
		    
		    unlock_cs();
		    return(UNCHANGED);
	    }
	    case KBD_GET_EN_STATUS: {
		    int n;
		    u_short ch;
		    
		    Kdebug1((dbg_fd,"mon: kbd_get_en_status\n"));
		    
		    ch = peek_queue();
		    
		    if (ch == NULL) {
			    SETWORD(&(state->eax), 0);
			    state->efl |= EFL_ZF;
#ifdef	IDLE_WORK_IN_PROGRESS
#if	0
			    lock_idle();
			    idling = TRUE;
			    unlock_idle();
			    condition_wait(idle_port);
#endif
#endif	/* IDLE_WORK_IN_PROGRESS */
		    } else {
			    SETHIGH(&(state->eax),HIGH(ch));
			    SETLOW(&(state->eax),LOW(ch));
			    state->efl &= ~EFL_ZF;
		    }
		    Debug0((dbg_fd,"en_status: %x, %x\n",
			    HIGH(ch), LOW(ch)));
		    unlock_cs();
		    return(UNCHANGED);
	    }
	    case KBD_GET_EN_FLAGS: {
		    Kdebug1((dbg_fd,"\rmon:kbd_get_enh_key flags\n"));
		    SETLOW(&(state->eax), *spram_kcbyte_1);
		    SETHIGH(&(state->eax), *spram_kcbyte_2);
		    Debug0((dbg_fd,"enh_flags: %x, %x\n",
			    *spram_kcbyte_1,*spram_kcbyte_2));
		    unlock_cs();
		    return(UNCHANGED);
	    }
	    default:
		Kdebug0((dbg_fd,"mon: kbd_fn, unknown ah=%x\n",
			 HIGH(state->eax)));
		unlock_cs();
		return (FALSE);
	}
	unlock_cs();
	return (TRUE);
}

extern int timer_interrupt_rate;
extern int min_timer_milli_pause;
extern int timer_milli_pause;
extern timer_interrupt();

int timer0_read_latch_value = 0;
int timer0_read_latch_value_orig = 0;
int timer0_latched_value = 0;
int timer0_new_value = 0;
boolean_t latched_counter_msb = FALSE;

int do_40(in_out, val, byte_word, state)
	boolean_t in_out;
	int val;
	boolean_t byte_word;
	state_t *state;
{
	int ret;

	if (in_out) {  /* true => in, false => out */
		extern u_long milliseconds_since_boot;

		if (timer0_latched_value == 0) {
			timer0_latched_value = timer_interrupt_rate*850;
		}
		switch (timer0_read_latch_value) {
			case 0x30:
				ret = timer0_latched_value&0xf;
				timer0_read_latch_value = 0x20;
				break;
			case 0x10:
				ret = timer0_latched_value&0xf;
				timer0_read_latch_value = 0x0;
				break;
			case 0x20:
				ret = (timer0_latched_value&0xf0)>>8;
				timer0_read_latch_value = 0x0;
				timer0_latched_value = 0;
				break;
			case 0x0:
				if (latched_counter_msb) {
					ret = 
					(milliseconds_since_boot >> 8) & 0xff;
				} else {
					ret = milliseconds_since_boot & 0xff;
				}
				latched_counter_msb = !latched_counter_msb;
				break;

		}
		Kdebug0((dbg_fd, "\rkey: do_40, in = 0x%x\n",ret));
	} else {
		switch (timer0_read_latch_value) {
			case 0x0:
			case 0x30:
				timer0_new_value = val;
				timer0_read_latch_value = 0x20;
				break;
			case 0x10:
				timer0_new_value = val;
				timer0_read_latch_value = 0x0;
				break;
			case 0x20:
				timer0_new_value |= (val<<8);
				timer0_read_latch_value = 0x0;
				break;
		}
		Kdebug0((dbg_fd, "\rkey: do_40, out = 0x%x\n",val));
		if (timer0_read_latch_value == 0) {
			timer0_read_latch_value=timer0_read_latch_value_orig;
			if (timer0_new_value == 0) timer0_new_value = 64*1024;
			timer_interrupt_rate = ((timer0_new_value)/1193);
			if (timer_interrupt_rate < min_timer_milli_pause) {
				int i;
				if (timer_interrupt_rate == 0) 
					timer_interrupt_rate = 1;
				for (i = 1; i<=min_timer_milli_pause;i++) {
				    if (i*timer_interrupt_rate >= 
						min_timer_milli_pause){
				    	timer_milli_pause = i*
						timer_interrupt_rate;
					break;
				    }
				}
			} else {
				timer_milli_pause = timer_interrupt_rate;
			}
			Kdebug0((dbg_fd, "timer_interrupt_rate = %d\n",
					  timer_interrupt_rate));
			Kdebug0((dbg_fd, "timer_milli_pause = %d\n",
					  timer_milli_pause));
		}
	}
	return(ret);
}

int do_42(in_out, val, byte_word, state)
	boolean_t in_out;
	int val;
	boolean_t byte_word;
	state_t *state;
{
	int ret;
	if (in_out) {  /* true => in, false => out */
		ret = inb(0x42);
		Kdebug0((dbg_fd, "\rkey: do_42, in = 0x%x\n",ret));
	} else {
		outb(0x42, val);
		Kdebug0((dbg_fd, "\rkey: do_42, out = 0x%x\n",val));
	}
	return(ret);
}

int do_43(in_out, val, byte_word, state)
	boolean_t in_out;
	int val;
	boolean_t byte_word;
	state_t *state;
{
	int ret;
	
	if (in_out) {  /* true => in, false => out */
		Kdebug0((dbg_fd, "\rkey: do_43, in = 0x%x\n",ret));
	} else {
		Kdebug0((dbg_fd, "\rkey: do_43, out = 0x%x\n",val));
		switch(val>>6) {
			case 0:
				timer0_read_latch_value = (val&0x30);
				timer0_read_latch_value_orig = (val&0x30);
				if (timer0_read_latch_value == 0) {
					latched_counter_msb = FALSE;
				}
				break;
			case 2:
				outb(0x43, val);
				Kdebug0((dbg_fd, "\rReally do_43\n"));
				break;
		}
	}
	return(ret);
}

/*
 * Port 0x60:
 *
 * info from 1) TechRef 1-12
 *	     2) PCSource 7.10-13
 *
 * Bits 7-0: Scan code.
 */
int do_60(in_out, val, byte_word, state)
	boolean_t in_out;
	int val;
	boolean_t byte_word;
	state_t *state;
{
	int ret;
	u_short * ptr;
	int entry;

	lock_cs();
	if (in_out) {  /* true => in, false => out */
#ifdef TRIAL1
		entry = delete_entry(&key_q);
		kbd_output_buffer = ((entry >> 16) & 0xffff);
		ptr = (u_short *)KEYISR;
		*ptr = (u_short)(entry & 0xffff);
#else
#endif TRIAL1		
		
		ret = kbd_output_buffer;

		Kdebug0((dbg_fd, "\rkey: do_60, in = 0x%x\n",ret));
	} else {
		Kdebug0((dbg_fd,"\rDoing an out on port 0x60. 0x%x\n",val));
		kbd_data_byte = val;
		kbd_status_register &= ~Command_byte_last;
		kbd_status_register |= 0x1;
	}
	unlock_cs();
	return(ret);
}

/*
 * Port 0x61:
 *
 * info from 1) i386at/kd.h 
 *           2) TechRef 1-12
 *	     3) PCSource 7.12
 *
 * Bits 0 - Timer 2 Gate Speaker '0'-ON  '1'-OFF  K_ENABLETMR2
 *	1 - Speaker Data   K_SPKRDATA (i386at/kd.h)
 *      2 - ?
 *	3 - ?
 *	4 - ?
 *	5 - ?
 *	6 - ?
 *	7 - Enable Keyboard  '0'-Inhibited '1'-Enabled
 */
int do_61(in_out, val, byte_word, state)
	boolean_t in_out;
	int val;
	boolean_t byte_word;
	state_t *state;
{
	int ret;

	lock_cs();
	if (in_out) {
		ret = inb(0x61);
		Kdebug0((dbg_fd, "\rkey: do_61 in oldval = %x\n",ret));
		ret &= 0x3;
		ret |= 0x80;
	} else {
		int x;
		x = inb(0x61);
		Kdebug0((dbg_fd, "\rkey: do_61 out = 0x%x, oldval = %x\n",val,x));
		outb(0x61, x | (val&0x3));
		if ((val & 0x80)) {
		} else {
		}
		ret = NULL;
	}
	unlock_cs();
	return(ret);
}

/*
 * Port 0x64:
 *
 * info from 1) PCSource pg 407
 *
 * Bits 7-0: Keyboard command byte. In & Out.
 */
int do_64(in_out, val, byte_word, state)
	boolean_t in_out;
	int val;
	boolean_t byte_word;
	state_t *state;
{
	int ret;

	lock_cs();
	if (in_out) {
		/* pc sourcebook pg 407 */
		ret = kbd_status_register;
		Kdebug0((dbg_fd, "\rkey: do_64, in = 0x%x\n",ret));
	} else {
		Kdebug0((dbg_fd, "\rkey: do_64, out = 0x%x\n",val));
		kbd_status_register |= Command_byte_last;
		kbd_command_byte = val;
		switch (kbd_command_byte) {
		    /* read write controller */
		    case 0x20:
		    case 0x60:
		    /* self test */
		    case 0xaa:
		    case 0xab:
			break;
		    /* disable keyboard */
		    case 0xad: {
			    kbd_status_register &= ~Keyboard_inhibited;
			    break;
		    }
		    /* enable keyboard */
		    case 0xae: {
			    kbd_status_register |= Keyboard_inhibited;
			    break;
		    }
		    default: {
			    printf("kbd: unknown out 0x64 number 0x%x\n",val);
			    break;
		    }
		}
		ret = NULL;
	}
	unlock_cs();
	return(ret);
}

/*
 * Turn pc speaker off.
 */
void speaker_off()
{
	u_char status;

	status = (inb(0x61) & ~(0x2 | 0x1));
	outb(0x61, status);
}


/* 
 * Scan code keyboard routines.
 */

int read_dev_kbd()
{
	int nbytes;
	int ncodes;


	int nfound;
	fd_set readfds;
	struct timeval timeout;

	FD_ZERO(&readfds);
	FD_SET(kbdFd, &readfds);
	
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	nfound = select(kbdFd+1, &readfds, 0, 0, 0);

	Kdebug0((dbg_fd,"Start of read kdb.\n"));
	nbytes = read(kbdFd, kevents, sizeof(kevents));
	Kdebug0((dbg_fd,"End of read kdb.\n"));

	if (nbytes < 0) {
		fprintf(dbg_fd,"\rError reading /dev/kbd.\n");
		exit_index = 9;
		exit_dos();
	}

	ncodes = nbytes/sizeof(kd_event);

	return(ncodes);
}

boolean_t delete_down;
void key_int();

boolean_t
check_special_codes(sc)
int	sc;
{
	state_t state;
	int state_count;
	int flavor;
	thread_basic_info_t thread_info;
	u_int cnt;

	if (((internal_kcbyte_1)&Cntrl_key_down) &&
		    ((internal_kcbyte_1)&Alt_key_down)) {
		if (delete_down) {
			/* ^G  exit the monitor */
			exit_dos();
		} else 	switch(sc) {
		    case K_zSC: {
			    int ret;
			    int mode;

			    /* ^-alt-Z  pause monitor */

			    lock_cs();
			    MACH_CALL((thread_suspend(v86_thread)),
				      "key_special: thread_suspend");

			    /* save then reset the vga state */
			    if (vga_state) {
				    save_video_ram(dos_planes);
				    bios_vga_fn(VGA_DISABLED);
				    restore_video_ram(mach_planes);
			    }

			    /* reset the keyboard */

			    Kdebug0((dbg_fd,"PAUSING dos monitor.\n"));

			    reset_tty();

			    speaker_off();

			    mode = KB_ASCII;

			    ret = ioctl (kbdFd,KDSKBDMODE, &mode);
			    if (ret < 0) {
				    fprintf(dbg_fd,"error ioctl. 0x%x\n",errno);
				    exit_index = 20;
				    exit_dos();
			    }
			    close(kbdFd);
			    close(dd_fd);

#ifdef TWILIGHT_ZONE
			    fprintf(stderr,"debug_state = %d\n", debug_state);

			    if (debug_state == ON) {
				    fprintf(stderr,"closing dbg_fd\n");
				    fclose(dbg_fd);
			    }
#endif TWILIGHT_ZONE

			    /* Pause the monitor */
			    kill(0,SIGSTOP);

#ifdef TWILIGHT_ZONE
			    if (debug_state == ON) {
				    fprintf("opening mon.out again\n");
				    if((dbg_fd = fopen ("mon.out", "a+")) ==
				       NULL) {
					    fprintf(stderr,"\rCouldn't reopen mon.out\n");
				    }
			    }
#endif TWILIGHT_ZONE

			    /* Monitor is resumed */
			    Kdebug0((dbg_fd,"RESUMING dos monitor.\n"));

			    /* tell the mouse to reinit */
			    reset_mouse_dev = TRUE;

			    /* set the keyboard for dos again */
			    dev_kbd_init();

			    /* reset keyboard status */
			    internal_kcbyte_1 &= 0xf0;
			    internal_kcbyte_2 = 0x0;
			    *spram_kcbyte_1 &= 0xf0;
			    *spram_kcbyte_2 = 0x0;
			    key_mode = KS_NORM;

			    /* restore video area */
			    if (vga_state) {
				    save_video_ram(mach_planes);
				    bios_vga_fn(VGA_ENABLED);
				    restore_video_ram(dos_planes);
			    }
			    
			    MACH_CALL((thread_resume (v86_thread)),
				      "key_special: thread_resume");

			    /* if keyboard is in interrupt driven mode
			     * add up codes for alt and cntrl to list. */
			    if ((keyint_vec_val != *keyint_vec_loc)) {
				    int sc;
				    Kdebug0((dbg_fd,"bios_kbd: Adding alt and ctrl up codes.\n"));
				    sc = create_sc_entry(UP_ALT);
				    append_entry(&key_q, sc, 0);
				    sc = create_sc_entry(UP_CTRL);
				    append_entry(&key_q, sc, 0);
			    }

			    unlock_cs();

			    key_int(FALSE);
			    
			    return(TRUE);
		    }
		    case K_EQLSC: 
			timer_milli_pause = 250;
			min_timer_milli_pause = 250;
			timer_interrupt_rate = 250;
			return(TRUE);
		    case K_MINUSSC: 
			timer_milli_pause = 55;
			min_timer_milli_pause = 55;
			timer_interrupt_rate = 55;
			return(TRUE);
		    case K_ZEROSC: 
			timer_milli_pause = 20;
			min_timer_milli_pause = 20;
			timer_interrupt_rate = 20;
			return(TRUE);
		    case K_NINESC: 
			timer_milli_pause = 20;
			min_timer_milli_pause = 10;
			timer_interrupt_rate = 10;
			return(TRUE);
		    case K_EIGHTSC: 
			timer_milli_pause = 48;
			min_timer_milli_pause = 10;
			timer_interrupt_rate = 12;
			return(TRUE);
		    case K_SEVENSC: 
			timer_milli_pause = 56;
			min_timer_milli_pause = 14;
			timer_interrupt_rate = 14;
			return(TRUE);
		    case K_SIXSC: 
			timer_milli_pause = 20;
			min_timer_milli_pause = 2;
			timer_interrupt_rate = 2;
			return(TRUE);
		    case K_FIVESC: 
			timer_milli_pause = 10;
			min_timer_milli_pause = 1;
			timer_interrupt_rate = 1;
			return(TRUE);
		    case K_ESCSC: 
			if (dbg_fd == stderr) {
				debug_state = ON;
				if ((dbg_fd = fopen ("mon.out", "w+")) == NULL)
					fprintf(stderr,"\rCouldn't open mon.out\n");
			}
			
			if (!debug_toggle) {
				fprintf(dbg_fd,"Starting DEBUGGING output.\n");
				us_debug_level = 3;
				video_debug_level = 3;
				key_debug_level = 3;
				disk_debug_level = 3;
				debug_toggle = 1;
			}else{
				us_debug_level = 0;
				video_debug_level = 0;
				key_debug_level = 0;
				disk_debug_level = 0;
				debug_toggle = 0;

				fprintf(dbg_fd,"Ending DEBUGGING output.\n");
			}
				
			return(TRUE);
#ifdef PROFILING
		    case K_pSC:
			if (profiling) {
				end_prof();
			}else{
				begin_prof();
			}
			return(TRUE);
#endif PROFILING
		    case K_tSC: {
			extern boolean_t timer_shortcut;
			timer_shortcut = TRUE;
			return(TRUE);
	    		}
		    case K_sSC: {
			extern boolean_t timer_shortcut;
			timer_shortcut = FALSE;
			return(TRUE);
	    		}
		    case K_cSC: {
			    extern u_char change_line_status;
			    change_line_status = 0x06;
			    return(TRUE);
		    }
		    case K_aSC: {
			int i;
			u_long * ptr = (u_long *) 0x0;
			for (i = 0; i < 0x1d4; i+=16) {
		fprintf(dbg_fd,"%x: %x %x %x %x\n",
		i, ptr[i/4], ptr[(i+4)/4], ptr[(i+8)/4], ptr[(i+12)/4]);
			}
			return(TRUE);
	    		}
		    default:
			return(FALSE);
		}
	}
	return (FALSE);
}

boolean_t
set_internal_kbstatus(scan_code)
	int	scan_code;
{
	boolean_t up;
	boolean_t ret = FALSE;

	up = scan_code & 0x80;
	scan_code &= 0x7f;

	switch(scan_code) {
	    case K_Cntrl: {
		    if (!up) {
			    internal_kcbyte_1 |= Cntrl_key_down;
			    ret = check_special_codes(scan_code);
		    }else{
			    internal_kcbyte_1 &= ~Cntrl_key_down;
		    }
		    break;
	    }
	    case K_cSC: {
		    if (internal_kcbyte_1 & Cntrl_key_down) {
			    if (!up) {
#ifdef NOTDEF
				    u_char * ptr;

				    ptr = (u_char *)0x471;
				    *ptr |= 0x80;
#endif NOTDEF
				    Kdebug0((dbg_fd,"bios_kbd: queuing break interrupt.\n"));
				    queue_interrupt(BREAK_INT_VEC, 1, FALSE);
				    check_interrupt(FALSE);
			    }
			    return (FALSE);
		    }else{
			    return (FALSE);
		    }
		    break;
	    }
	    case K_Alt: {
		    if (!up) {
			    internal_kcbyte_1 |= Alt_key_down;
			    ret = check_special_codes(scan_code);
		    }else{
			    internal_kcbyte_1 &= ~Alt_key_down;
		    }  
		    break;
	    }
	    case K_R_Shift: {
		    if (!up) {
			    internal_kcbyte_1 |= R_Shift_key_down;
		    }else{
			    internal_kcbyte_1 &= ~R_Shift_key_down;
		    }
		    break;
	    }
	    case K_L_Shift: {
		    if (!up) {
			    internal_kcbyte_1 |= L_Shift_key_down;
		    }else{
			    internal_kcbyte_1 &= ~L_Shift_key_down;
		    }
		    break;
	    }
	    case K_Del: {
		    if (!up) {
			    delete_down = TRUE;
			    ret = check_special_codes(scan_code);
		    }else{
			    delete_down = FALSE;
		    }
		    /* don't break, treat as regular character */
	    }
	    default: {
		    if (!up) 
			    ret = check_special_codes(scan_code);
	    }
	}
	return (ret);
}

void determine_kb_state()
{
	if (internal_kcbyte_1 & Cntrl_key_down) {
		key_mode = KS_CNTRL;
	}else if ((internal_kcbyte_1 & L_Shift_key_down) ||
		  (internal_kcbyte_1 & R_Shift_key_down)) {
		key_mode = KS_SHIFT;
	}else if (internal_kcbyte_1 & Alt_key_down) {
		key_mode = KS_ALT;
	}else{
		key_mode = KS_NORM;
	}
}

u_int conv_sc_mode(scancode)
	u_char scancode;
{
	u_int ret;

	switch(key_mode) {
	    case KS_NORM:
		ret = pc_keymap[scancode].base;
		break;
	    case KS_SHIFT:
		ret = pc_keymap[scancode].shift;
		break;
	    case KS_CNTRL:
		ret = pc_keymap[scancode].cntrl;
		break;
	    case KS_ALT:
		ret = pc_keymap[scancode].alt;
		break;
	    default:
		Kdebug0((dbg_fd,"\rbios_kbd: unknown key_mode.\n"));
		ret = pc_keymap[scancode].base;
		break;
	}
	return(ret);
}	

u_short decode_sc(scancode)
unsigned char scancode;
{
	unsigned char	c;
	int		char_idx;
	boolean_t	up = FALSE;		/* key-up event */
	int 		kmode;
	u_int		conv_val;
	u_short		queue_val;

	if (scancode & K_UP) {
		up = TRUE;
		scancode &= ~K_UP;
	}
	if (scancode < Numkeys) {
		/* Lookup in map, then process. */

		conv_val = conv_sc_mode(scancode);

		if (conv_val & 0xffff) {
			if (!up) {
			if (conv_val & 0x7f) {
				queue_val = (scancode <<8) | (conv_val & 0x7f);
			}else{
				queue_val = conv_val;
			}
			return(queue_val);
			}
		}else{
			determine_kb_state();
		}
	}
	return(NULL);
}

decode_sc_and_add_ch(scancode)
unsigned char scancode;
{
	unsigned char	c;
	int		char_idx;
	boolean_t	up = FALSE;		/* key-up event */
	int 		kmode;
	u_int		conv_val;
	u_short		queue_val;

	if (scancode & K_UP) {
		up = TRUE;
		scancode &= ~K_UP;
	}
	if (scancode < Numkeys) {
		/* Lookup in map, then process. */

		conv_val = conv_sc_mode(scancode);

		if (conv_val & 0xffff) {
			if (!up) {
				if (conv_val & 0x7f) {
					queue_val = (scancode <<8) | (conv_val & 0x7f);
				}else{
					queue_val = conv_val;
				}
				enqueue(queue_val);
			}
		}else{
			determine_kb_state();
		}
	}
}

int create_sc_entry(scan_code)
	int scan_code;
{
	int entry = 0;
	u_int conv_val;
	u_short scval;
	u_char sc;
	boolean_t up = FALSE;

	sc = scan_code;

	if (sc & K_UP) {
		up = TRUE;
		sc &= ~K_UP;
	}

	if (sc < Numkeys) {
		conv_val = conv_sc_mode(sc);
		if (conv_val & 0xffff) {
			if (!up) {
				if (conv_val & 0x7f) {
					/* 0xScCh */
					entry = (scan_code << 8) | (conv_val & 0x7f);
				}else{
					entry = conv_val; /* 0xXX00 */
				}
			}else{
				entry = 0x00ff;	/* do nothing */
			}
		}else{
			/* this could be more elegantly placed */
			determine_kb_state();
			/*                                     */

			switch (sc) {
			    case K_R_Shift:
				entry = 0;
				break;
			    case K_L_Shift:
				entry = 1;
				break;
			    case K_Cntrl:
				entry = 2;
				break;
			    case K_Alt:
				entry = 3;
				break;
			    case K_Scroll_lock:
				entry = 0xc;
				break;
			    case K_Num_lock:
				entry = 0xd;
				break;
			    case K_Caps_lock:
				entry = 0xe;
				break;
			    case K_Insert:
				entry = 0xf;
				break;
			    default:
				entry = 0x00ff;
				break;
			}
			if (!up)
				entry |= 0x80;
		}
	}

	entry |= (u_int)(scan_code << 16);

	return(entry);
}

boolean_t do_key_int()
{
	if (empty_entry(&key_q)) {
		return(FALSE);
	}else
		return(TRUE);
}

void
key_int(main_thread)
boolean_t main_thread;
{
	if (empty_entry(&key_q))
		return;

	queue_interrupt(KEYBOARD_INT_VEC, 1, main_thread);
	check_interrupt(main_thread);
}

void keyboard_loop()
{
	int klioplfd;
	int num_sc;
	int cur_sc;
	int first_time = 0;
	mach_port_t	iopl_device_port;
	kern_return_t	rc;

	if ((klioplfd = open("/dev/iopl", O_RDWR)) < 0) {
		fprintf(dbg_fd,"\rmon_kbd: Error in iopl fd open. fd = %d\n",klioplfd);
		exit_index = 10;
		exit_dos();
	}

 	fprintf(dbg_fd,"\rKeyboard loop active.\n");

	dev_kbd_init();

	/* initialize keyboard flags */
	*spram_kcbyte_1 = 0;
	*spram_kcbyte_2 = 0;

	internal_kcbyte_1 = 0;
	internal_kcbyte_2 = 0;

#ifdef	MOVED_TO_EXEC
	override_interrupt_vectors();
#else
	while(!vectors_overridden)
		millisecond_wait(500);

	keyint_vec_val = (u_long) Abs2Segoff(KEYISR+2);
#endif	MOVED_TO_EXEC

	while(TRUE) {
		Kdebug1((dbg_fd, "About to read scan code\n"));
		num_sc = read_dev_kbd();
		Kdebug1((dbg_fd, "Num codes = %x\n",num_sc));

#ifdef	MOVED_TO_EXEC
		if (!first_time) { 
			override_interrupt_vectors();
			keyint_vec_val = *keyint_vec_loc;
			first_time = 1;
		}
#endif	MOVED_TO_EXEC

		for(cur_sc = 0; cur_sc < num_sc; cur_sc++) {
			int sc = kevents[cur_sc].value.sc;

			if (kevents[cur_sc].type != KEYBD_EVENT) {
				Kdebug1((dbg_fd, "scan type = %x\n",
					kevents[cur_sc].type));
				continue;
			}

			Kdebug1((dbg_fd, "scan code = %x\n",sc));

			/* check for cntl-alt-del, etc. */
			if (set_internal_kbstatus(sc)) continue;

#ifdef	IDLE_WORK_IN_PROGRESS
			/* Throw into gear if idling */
			lock_idle();
			if (idling) {
				idling = FALSE;
				condition_signal(idle_port);
			}
			unlock_idle();
#endif	/* IDLE_WORK_IN_PROGRESS */

			lock_cs();
			/* check keyvec redirection */

			if ((keyint_vec_val == *keyint_vec_loc)) {
				Kdebug1((dbg_fd, "decode and add = %x\n",sc));
				decode_sc_and_add_ch(sc);
				*spram_kcbyte_1 = internal_kcbyte_1;
				*spram_kcbyte_2 = internal_kcbyte_2;
			}else{
				Kdebug1((dbg_fd, "append_entry = %x\n",sc));
				sc = create_sc_entry(sc);
				append_entry(&key_q, sc,
					     timeval_to_milli(&(kevents[cur_sc].time))
					     );
				Kdebug1((dbg_fd, "append_entry done = %x\n",sc));
			}
			unlock_cs();

			key_int(FALSE);
		}
		
	}
}

