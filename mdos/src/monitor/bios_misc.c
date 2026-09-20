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
 * $Log:	bios_misc.c,v $
 * Revision 2.12  92/07/01  14:24:48  grm
 * 	Added speaker_off() call in exit_dos routine.  Added some strings
 * 	to exit_message array.
 * 	[92/06/03            grm]
 * 
 * Revision 2.11  92/05/22  15:59:17  grm
 * 	Fixed timing problem introduced by kernel bug fix in MK73
 * 	(ipc_sched.c).  Removed extraneous code.
 * 	[92/05/21            grm]
 * 
 * Revision 2.10  92/04/29  16:31:14  grm
 * 	Changed for use with new override vectors code.
 * 	[92/04/29            grm]
 * 
 * Revision 2.9  92/04/14  13:20:10  grm
 * 	Added some new error messages.  The Mach3 version.
 * 	[92/03/27            grm]
 * 
 * Revision 2.8  92/03/02  15:47:19  grm
 * 	Moved the interrupt table indices into bios_misc.h for everyone
 * 	to use.  Found the Word bug that caused the keyboard to lag,
 * 	changes in queue_interrupt().
 * 	[92/02/27            grm]
 * 	Added a check in the queue_interrupt routine that lets you queue
 * 	more than 20 keyboard interrupts.  Added code in check_interrupt
 * 	that checks to see if there is anything left in the keyboard
 * 	queue when the interrupt table's count is zero.  Also changed
 * 	some constants for use with MK69.
 * 	[92/02/20            grm]
 * 
 * Revision 2.7  92/02/14  17:44:38  grm
 * 	Added two exit messages, and removed the dev_kbd_close from
 * 	exit_dos().
 * 	[92/02/12            grm]
 * 	Added some new exit_dos messages.  Calls dev_kbd_close from the
 * 	exit_dos() routine to clean up the keyboard state (specifically
 * 	for the OSF Single Server).
 * 	[92/02/11            grm]
 * 
 * Revision 2.6  92/02/03  14:24:43  rvb
 * 	Clean Up
 * 
 * Revision 2.5  92/02/02  23:02:26  rvb
 * 	Removed the thread_status.h include.  Rely on bios.h inclusion.
 * 	[92/02/01            grm]
 * 	Replaced the exit(0)'s with exit_dos()'s.  Added the exit messages for
 * 	new types.
 * 	[92/01/27            grm]
 * 
 * Revision 2.4  91/12/06  15:28:27  grm
 * 	Replaced the absolute mon_space code with the relative dosres
 * 	code.
 * 	[91/12/06            grm]
 * 
 * Revision 2.3  91/12/05  16:40:43  grm
 * 	Added some printer support.
 * 	Changed exit_dos printouts.
 * 	[91/08/09  19:42:32  grm]
 * 
 * 	Put in keyisr for use as the redirect in Ivec 0x9.
 * 	Changed so that keyboard interrupts only occur through
 * 	a force_interrupt (no kernel keyboard interrupts).
 * 	[91/07/16  17:42:24  grm]
 * 
 * 	Moved some constants to bios_misc.h.
 * 	Added Some EGA mouse support.
 * 	[91/06/28  18:12:01  grm]
 * 
 * 	Modified to use dbg's v86 kernel support.
 * 	[91/06/14  11:51:51  grm]
 * 
 * 	New Copyright
 * 	[91/05/28  14:57:00  grm]
 * 
 * 	Messed with the exit_dos routine.
 * 	[91/05/02  13:31:26  grm]
 * 
 * 	Rick's new self adjusting timer changes.
 * 	[91/04/30  13:31:38  grm]
 * 
 * 	Exit dos now uses restore_video_ram
 * 	[91/03/26  19:04:11  grm]
 * 
 * 	Mach3 version.
 * 	[91/02/01  13:26:35  grm]
 * 
 * 	Added timer thread loop.
 * 	[90/10/18  17:57:22  grm]
 * 
 * 	Get tick count and get date return correct values.
 * 	[90/08/01  16:35:14  grm]
 * 
 * 	IO sub function support added. Dos exit routine
 * 	modifications.
 * 	[90/05/25  16:10:50  grm]
 * 
 * 	Changed fprintfs to use dbg_fd instead of stderr.
 * 	[90/04/17  22:33:56  grm]
 * 
 * 	Moved all of the video constants and functions
 * 	to bios_video.c.
 * 	[90/04/07  00:32:37  grm]
 * 
 * 	Added constants.  Changed vga support.  Added
 * 	set_display_mode.
 * 	[90/04/05  21:17:56  grm]
 * 
 * 	Started editing as grm.  Moved to v86 branch
 * 	[90/03/28  18:36:02  grm]
 * 
 * Revision 2.1.1.4  90/03/22  21:48:26  dorr
 * 	Added code to use vga.  Changed get_display_mode to return
 * 	something useful.  Added basic printer and serial routines.
 * 
 * Revision 2.1.1.3  90/03/13  15:33:07  orr
 * 	Version that gives an A:> prompt and does a DIR.
 * 	Replaced DEBUG with DebugX. No major changes. 
 * 
 * Revision 2.1.1.2  90/03/12  02:13:02  orr
 * 	first working version.  use dbg_fd, for debug output.
 * 	hack up tick count to change.
 * 
 * Revision 2.1.1.1  90/03/11  22:18:44  orr
 * 
 * Revision 1.1  90/03/09  11:57:18  orr
 * Initial revision
 * 
 */
#include "base.h"
#include "bios.h"

#include <sys/time.h>
#include <sys/file.h>

#include <mach/message.h>

#include "bios_misc.h"

#define CLOCK_GET_TICK_COUNT	0x00
#define CLOCK_SET_TICK_COUNT	0x01
#define CLOCK_GET_TIME		0x02
#define CLOCK_SET_TIME		0x03
#define CLOCK_GET_DATE		0x04
#define CLOCK_SET_DATE		0x05
#define CLOCK_SET_ALARM		0x06
#define CLOCK_RESET_ALARM	0x07
#define CLOCK_GET_DAY_COUNT	0x0a
#define CLOCK_SET_DAY_COUNT	0x0b

#define SERIAL_INIT_COM_PORT	0x00
#define SERIAL_STATUS		0x03

#define IO_WAIT_ON_EXTERN_EVENT	0x41
#define IO_KEY_INTERCEPT	0x4f
#define IO_GET_SYS_ENV		0xc0

#define PRINTER_WRITE_CHAR	0x00
#define PRINTER_INIT		0x01
#define PRINTER_STATUS		0x02

#define SERIAL_TIME_OUT		0x80
#define PRINTER_TIME_OUT	0x01

#define SECSPERHOUR		(60 * 60)
#define SECSPERDAY		(60 * 60 * 24)

#define Tick_Addr		(u_long *)0x46c

#define LPT1	0
#define LPT1_val	(0x80 | 0x40 | 0x10)
#define LPTX_val	1

extern vm_address_t mach_planes;

struct queue_entry {
	int	entry;
	struct queue_entry *next;
};

struct queue_type {
	struct queue_entry * head;
	struct queue_entry * tail;
};

u_char itob(number)
	u_char number;
{
	u_char first;
	u_char second;
	
	first = number / 10;
	second = number % 10;

	return ((first<<4)|(second));
}

char * exit_message[] = {
#ifdef NOHUMOR
	"Shutting Mdos down.",				/* index = 0 */
#else
	"Exiting Gracefully.\n\rShutting Mdos down.",	/* index = 0 */
#endif NOHUMOR
	"Can't open vga.info.",				/* index = 1 */
	"Need setup version 2.0 to start.  Redo Mdos setup.",
	"Can't open font.info.vga.",
	"Error reading font.info.vga file.",		/* index = 4 */
	"Fix the Fake Mouse Driver.",
	"Error w/boot sector in read_abs_hdisk.",	/* index = 6 */
	"Error reading hard drive, validate drive.",
	"Couldn't open /dev/kbd.",			/* index = 8 */
	"Error reading /dev/kbd.",
	"Couldn't open /dev/iopl in keyboard_loop.",	/* index = 10 */
	"memory object server confused.",
	"Couldn't open /dev/iopl in v86.c.",		/* index = 12 */
	"Exiting from enable_i386_ports get_state.",
	"Exiting from enable_i386_ports set_state.",	/* index = 14 */
	"Mmap of bios rom failure.",
	"Bios bootstrap Int 0x19.",			/* index = 16 */
	"pager(new_memory_object): vm_map failed.",
	"pager(memory_object_remap): vm_map failed.",	/* index = 18 */
	"pager(memory_object_init): objtab lookup failed.",
	"Keyboard pause ioctl failed.",			/* index = 20 */
	"Keyboard close ioctl failed.",
	"Keyboard initial ioctl failed.",		/* index = 22 */
	"Couldn't open /dev/console."
};

int exit_index = 0;

void exit_dos() 
{
	extern int total_exceptions_processed;

	if (vga_state) 
		bios_vga_fn(VGA_DISABLED);

	if (vga_state) {
		/*	restore_vga_fonts(); */
		restore_video_ram(mach_planes);
	}

	reset_tty();

	speaker_off();

	printf("\n%s\n", exit_message[exit_index]);
#ifdef NOTDEF
	printf("Switch count = %d\n",cs_switch_count);	
	printf("Exceptions = %d\n",total_exceptions_processed);
#endif /* NOTDEF */

	bios_kbd_exit();
	bios_video_exit();
	exit(0);
}

long timeval_to_milli (tv)
	struct timeval * tv;
{
	return((tv->tv_sec * 1000) + (tv->tv_usec/1000));
}

u_long milliseconds_since_boot = 0;

long get_current_tick_count()
{
	struct timeval tp;
	struct timezone tzp;
	static long first_sec = 0;
	static long last_sec = 0;	
	long secs;
	long ret_val;
	static long initial_sec;
	static long initial_usec;
	
	gettimeofday(&tp, &tzp);

	if (last_sec == 0) {	
		secs = tp.tv_sec - (tzp.tz_minuteswest*60);
		if (localtime((time_t *)&secs)->tm_isdst)
			secs += SECSPERHOUR;

		first_sec = secs;
		last_sec = tp.tv_sec;

		initial_sec = tp.tv_sec;
		initial_usec = tp.tv_usec;
	} else {
		secs = (tp.tv_sec-last_sec) + first_sec;
		last_sec = tp.tv_sec;
		first_sec = secs;
	}

	milliseconds_since_boot = ((tp.tv_sec - initial_sec)*1000) +
				((tp.tv_usec - initial_usec)/1000);

	secs %= SECSPERDAY;
	ret_val = secs * 18.2 + (tp.tv_usec/54945);

	return(ret_val);
}

#define ivec1_value (Abs2Segoff(IRET_LOCATION))
#define ivec2_value (Abs2Segoff(IRET_LOCATION))


boolean_t 	timer_shortcut = FALSE;

#define TIMER1_INT	0x08
#define	KEYBOARD_INT	0x09
#define COM2_INT	0x0b
#define COM1_INT	0x0c
#define	TIMER2_INT	0x1c
#define MOUSE_INT	0xfd
#define BREAK_INT	0x1b

#define	MAX_INTS 7

struct v86_interrupt_table interrupt_table[MAX_INTS] = { 
#ifdef TRIAL1
	{0, FALSE, KEYBOARD_INT},
#else
	{0, TRUE,  KEYBOARD_INT},
#endif TRIAL1
	{0, FALSE, TIMER1_INT},
	{0, FALSE, TIMER2_INT},
	{0, FALSE, COM1_INT},
	{0, FALSE, COM2_INT},
	{0, FALSE, MOUSE_INT},
	{0, FALSE, BREAK_INT}
};

/*
 *  Interrupt vector addresses in the Interrupt Vector Table
 */
static	u_long * mouse_vector = (u_long *)0xcc;
static	u_long * mach_mouse_vector = (u_long *)0x3f4;
static	u_long * clock_vector1 = (u_long *)0x20;
static	u_long * clock_vector2 = (u_long *)0x70;
static	u_long * com_vector1 = (u_long *)0x30;
static	u_long * keyboard_vector = (u_long *)0x24;
static	u_long * emm_vector = (u_long *)0x19c;
static	u_long * keyboard_bios_vector = (u_long *)0x58;

int vectors_overridden = FALSE;

void
reset_interrupt_vectors()
{
	if (!vectors_overridden)
		init_dosres();

	*clock_vector1 = (u_long) Abs2Segoff(IRET_LOCATION);
	*clock_vector2 = (u_long) Abs2Segoff(IRET_LOCATION);
	*com_vector1 = (u_long) Abs2Segoff(IRET_LOCATION);
	*keyboard_vector = (u_long) Abs2Segoff(KEYISR+2);
	*keyboard_bios_vector = (u_long) Abs2Segoff(INT16_ISR);
	*mouse_vector = (u_long) Abs2Segoff(FAKE_MOUSE_DRIVER+0x28);
	*mach_mouse_vector = (u_long) Abs2Segoff(MOUSE_HELPER_1);
	*emm_vector = (u_long) Abs2Segoff(EMM_LOCATION);
}

#define vector_index_value(index) (interrupt_table[index].vec)
#define MAX_EFL_COUNT	100
int efl_counter = 0;
extern struct queue_type key_q;
extern u_char kbd_output_buffer;
long lastint = 0;
long lastkey = 0;

boolean_t keyboard_interrupt(main_thread)
	boolean_t main_thread;
{
	u_short * ptr = (u_short *)KEYISR;
	int entry;
	long keytime;
#ifdef NOTDEF
	state_t	state;
	int 	state_count;
	long	curtime;
	long	diffints;
	long	diffkeys;
	struct timeval	ttv;
	struct timezone	tzp;

	first_entry(&entry, &keytime);
	diffkeys = keytime - lastkey;

	gettimeofday(&ttv, &tzp);
	curtime = timeval_to_milli(&ttv);
	diffints = curtime - lastint;

	Kdebug0((dbg_fd,"curtime 0x%x lastint 0x%x diff 0x%x\n",
		 curtime, lastint, diffints));
	Kdebug0((dbg_fd,"keytime 0x%x lastkey 0x%x diff 0x%x\n",
		 keytime, lastkey, diffkeys));
	
	if (diffints < diffkeys) {
		boolean_t done = FALSE;

		Kdebug1((dbg_fd,"Keyint too close together keyint. 0x%x\n",
			 diffkeys - diffints));

		thread_resume(v86_thread);
		unlock_cs();

		millisecond_wait(diffkeys - diffints);

		while (TRUE) {
			lock_cs();
			MACH_CALL((thread_suspend(v86_thread)),"keyint")
			state_count = i386_THREAD_STATE_COUNT;
			MACH_CALL((thread_get_state(v86_thread, i386_THREAD_STATE,
						    &state, &state_count)),
				  "keyint");

			if ((!(state.efl&EFL_IF)) ||
			     ((!main_thread) && (state.efl & EFL_RF))) {
				thread_resume(v86_thread);
				unlock_cs();
				millisecond_wait(200);
				Kdebug0((dbg_fd,"keyint can't happen. 0x%x 0x%x\n",
					 main_thread, state.efl));
			}else{
				break;
			}
		}
	}
	
	gettimeofday(&ttv, &tzp);
	curtime = timeval_to_milli(&ttv);
	lastint = curtime;
	lastkey = keytime;
#endif NOTDEF	

	delete_entry(&key_q, &entry, &keytime);
	
	*ptr = (u_short)(entry & 0xffff);
	kbd_output_buffer = ((entry >> 16) & 0xffff);
	
	Kdebug0((dbg_fd,"Keyboard interrupt kob = 0x%x sc = 0x%x\n",
		 kbd_output_buffer, *ptr));

	return(TRUE);
}

void force_interrupt(vector_index, main_thread)
int		vector_index;
boolean_t	main_thread;
{
	state_t	state;
	int 	state_count;
	int	vector = vector_index_value(vector_index);

	Debug2((dbg_fd, "Force interrupt: %d (%x)\n",vector_index, main_thread));

	MACH_CALL((thread_suspend(v86_thread)), "t_sus  for interrupt");
	state_count = i386_THREAD_STATE_COUNT;
	MACH_CALL(( thread_get_state (v86_thread, i386_THREAD_STATE,
				      &state, &state_count )),
	  "thread_get_state");
try_again:
#ifndef OLD_CODE
	if (((!main_thread) && (state.efl & EFL_RF))||(!(state.efl&EFL_IF))) {
#else
	if (((!main_thread) && (state.efl & EFL_RF)) || 
	    ((vector_index == MOUSE_INT_VEC) &&
             (!(state.efl&EFL_IF)))) {
#endif OLD_CODE
		Debug2((dbg_fd, "interrupt unable to trigger: efl = %x\n",
			state.efl));
		if (!(state.efl&EFL_IF)) {
			efl_counter++;
			Kdebug0((dbg_fd,"efl_counter = 0x%x\n",efl_counter));
			if (efl_counter > MAX_EFL_COUNT) {
				state.efl |= EFL_IF;
				goto try_again;
			}
		}
		thread_resume(v86_thread);
	}else{
		if (vector_index == KEYBOARD_INT_VEC) {
			if (!keyboard_interrupt(main_thread))
				return;
		}
			
		Debug2((dbg_fd,
		    "interrupt cs 0x%x eip 0x%x efl 0x%x(%x)\n",
		    state.cs,state.eip, state.efl,vector));
		Debug2((dbg_fd,
		    "alarm = %x\n",
		    *(u_long *)0x20));
		efl_counter = 0;
		interrupt_table[vector_index].count--;
		simulate_interrupt(&state, vector, 0);
		MACH_CALL(( thread_set_state ( v86_thread,
					      i386_THREAD_STATE,
					      &state, state_count )),
			  "thread_set_state force_interrupt");

		MACH_CALL((thread_resume(v86_thread)), 
			  "t_res  for interrupt");
	}
}

boolean_t queue_interrupt(vector_index, cnt, main_thread)
int 		vector_index;
int 		cnt;
boolean_t	main_thread;
{
	static first_time = TRUE;

	Debug2((dbg_fd, "queue_interrupt = %x\n", vector_index));

	lock_cs();

	if (first_time) {
		struct i386_v86_assist_state state;
		int 	state_count = i386_V86_ASSIST_STATE_COUNT;
		state.int_table = (unsigned int)&interrupt_table;
		state.int_count = MAX_INTS;
		Debug2((dbg_fd, "Setup assist: %x\n",state.int_table));
		MACH_CALL((thread_suspend(v86_thread)), "setup assist");
		MACH_CALL(( thread_set_state (v86_thread,
					      i386_V86_ASSIST_STATE,
					      &state, state_count )),
			  "thread_set_state queue_interrupt");
		MACH_CALL((thread_resume(v86_thread)), "setup assist");
		first_time = FALSE;
	}

	if ((interrupt_table[vector_index].count < 20) ||
	    (vector_index == KEYBOARD_INT_VEC) ||
	    (vector_index == MOUSE_INT_VEC))  {
		interrupt_table[vector_index].count += cnt;
	}else{
		Debug0((dbg_fd,"INTCOUNT EMERGENCY!!! Dropping interrupt! %d\n",
			vector_index));
	}
	/* force_interrupt(vector_index, main_thread); */
	unlock_cs();
	return (TRUE);
}

void check_interrupt(main_thread)
boolean_t	main_thread;
{
	int vector, cnt;
	int i;
	lock_cs();
	for (i = 0; i < MAX_INTS ; i++) {
		if ((interrupt_table[i].count > 0) &&
		    ((!interrupt_table[i].mask) || (i == KEYBOARD_INT_VEC))) {
			Debug2((dbg_fd, "force_interrupt = %x\n", i));
			force_interrupt(i, main_thread);
			break;
		}else if ((i == KEYBOARD_INT_VEC) && do_key_int()) {
			Debug0((dbg_fd,"KEYBOARD ALERT, count = 0x%x, and queue not empty!\n",interrupt_table[i].count));
			interrupt_table[i].count = 1;
			force_interrupt(i,main_thread);
			break;
		}
	}
	unlock_cs();
}

void
clear_interrupt(vector_index)
int 		vector_index;
{
	Debug2((dbg_fd, "clear_interrupt = %x\n", vector_index));
	lock_cs();
	interrupt_table[vector_index].count = 0;
	unlock_cs();
}

int
timer_interrupt(main_thread) 
boolean_t main_thread;
{
	u_long * ivec1_loc;
	u_long * ivec2_loc;
	int interrupt_vector_index = 0;
	extern int timer_milli_pause;
	extern int timer_interrupt_rate;
			
	lock_cs();

	ivec1_loc = (u_long *)0x70;
	ivec2_loc = (u_long *)0x20;

	if (*ivec2_loc != ivec2_value) {
		Debug2((dbg_fd, "Clock 0x8\n"));
		interrupt_vector_index = TIMER1_INT_VEC;
	} else if (*ivec1_loc != ivec1_value) {
		Debug2((dbg_fd, "Clock 0x1c\n"));
		interrupt_vector_index = TIMER2_INT_VEC;
	}

	if (interrupt_vector_index != 0) {
		int num_ticks;
		num_ticks = timer_milli_pause/timer_interrupt_rate;
		unlock_cs();
		queue_interrupt(interrupt_vector_index,num_ticks,main_thread);
		if (!timer_shortcut || 
		    interrupt_table[interrupt_vector_index].count >= 20)
			check_interrupt(main_thread);
	} else {
		unlock_cs();
	}
	return;
}


int timer_interrupt_rate = 55;
int min_timer_milli_pause = 16;
int timer_milli_pause = 55;

void
override_interrupt_vectors()
{
	millisecond_wait(500);
	reset_interrupt_vectors();
	vectors_overridden = TRUE;
}

boolean_t
keyboard_bios_redirected()
{
	return (FALSE);
	if (vectors_overridden)
		return (*keyboard_bios_vector != (u_long) Abs2Segoff(INT16_ISR));
	else return (FALSE);
}

millisecond_wait(ms)
int ms;
{
	mach_msg_header_t msg;
	static mach_port_t port = MACH_PORT_NULL;
	if (port == MACH_PORT_NULL) {
		MACH_CALL((mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port)),
			   	"port_allocate in timer_loop");
	}

	mach_msg(&msg, MACH_RCV_TIMEOUT | MACH_RCV_MSG, 
		 0, sizeof(msg), port,
		 ms, MACH_PORT_NULL);

}

/*
 *  The loop for the timer_thread.  It updates the BIOS
 * scratchpad RAM's tick count field every milli_pause
 * milliseconds.
 */
void timer_loop()
{
	u_long tick_count;
	u_long * addr;
	u_long * ivec1_loc;
	u_long * ivec2_loc;
	u_short * sp;
	state_t state;
	int state_count;
	int last_time;
	int turn = 0;
	int pause_val;
	u_long time_error = 0, time_correction;

#ifdef	MOVED_TO_EXEC
	override_interrupt_vectors();
#else
	while(!vectors_overridden)
		millisecond_wait(500);
#endif	MOVED_TO_EXEC

	addr = Tick_Addr;

	*addr = get_current_tick_count();
	last_time = milliseconds_since_boot;
	while(TRUE) {
		if (timer_milli_pause < 10) timer_milli_pause = 10;

		Debug0((dbg_fd,"timer_loop: about to millipause of %d\n",
			timer_milli_pause+time_correction));

		pause_val = timer_milli_pause + time_correction - 10;
		if (pause_val < 0) pause_val = 1;
		millisecond_wait(pause_val);

		*addr = get_current_tick_count();
	 	Vdebug2((dbg_fd, "Timer wakeup at %d\n", 
				 milliseconds_since_boot));

		time_error += (timer_milli_pause-((timer_milli_pause/10)*10));
		time_correction = (time_error/10)*10;
		time_error -= time_correction;
	 	Vdebug2((dbg_fd, "Timer correction is %d\n", 
				  time_correction));

		check_interrupt(FALSE);
		timer_interrupt(FALSE);
	}
}

boolean_t bios_clock_fn(state)
	state_t *state;
{
	switch (HIGH(state->eax)) {
	    case CLOCK_GET_TICK_COUNT: {
		    long ret_val;

		    ret_val = get_current_tick_count();

		    SETWORD(&(state->edx), (ret_val & 0xffff));
		    SETWORD(&(state->ecx), ((ret_val & 0xffff0000)>>16));
		    SETLOW(&(state->eax), 0x00);
		    break;
	    }
	    case CLOCK_SET_TICK_COUNT: {
		    /*
		     * Not allowed to set the time.  Don't do anything.
		     */
		    break;
	    }
	    case CLOCK_GET_DATE: {
		    struct timeval tp;
		    struct timezone tzp;
		    struct tm *tm;
		    long secs;
		    u_char year;
		    u_char month;
		    u_char day;

		    gettimeofday(&tp, &tzp);

		    secs = tp.tv_sec - (tzp.tz_minuteswest*60);

		    tm = localtime((time_t *)&secs);

		    SETHIGH(&(state->ecx), 0x19);
		    SETLOW(&(state->ecx), itob(tm->tm_year));
		    SETHIGH(&(state->edx), itob(tm->tm_mon+1));
		    SETLOW(&(state->edx), itob(tm->tm_mday));
		    Debug2((dbg_fd, "Time returned: %x, %x\n",
				     state->ecx,state->edx));
		    break;
	    }
	    case CLOCK_SET_DATE: {
		    /*
		     * Not allowed to set the date.  Don't do anything.
		     */
		    break;
	    }
	    case CLOCK_GET_TIME:
	    {
		    struct timeval tp;
		    struct timezone tzp;
		    struct tm *tm;
		    long secs;
		    u_char year;
		    u_char month;
		    u_char day;
		
		    gettimeofday(&tp, &tzp);

		    secs = tp.tv_sec - (tzp.tz_minuteswest*60);

		    tm = localtime((time_t *)&secs);
		    SETHIGH(&(state->ecx), itob(tm->tm_hour));
		    SETLOW(&(state->ecx), itob(tm->tm_min));
		    SETHIGH(&(state->edx), itob(tm->tm_sec));
		    SETLOW(&(state->edx), itob(tm->tm_isdst));

		    break;
	    }
	    default:
		Debug2((dbg_fd,"mon: CLOCK op (0x%x)\n",HIGH(state->eax)));
		return (FALSE);
	}
	return (TRUE);
}

boolean_t bios_serial_fn(state)
	state_t *state;
{
	switch (HIGH(state->eax)) {
	    case SERIAL_INIT_COM_PORT: {
		    Debug2((dbg_fd, "\nmon: Serial Init Com Port\n"));
		    SETHIGH(&(state->eax), SERIAL_TIME_OUT);
		    break;
	    }
	    case SERIAL_STATUS: {
		    Debug2((dbg_fd, "\nmon: Serial Status\n"));
		    SETHIGH(&(state->eax), SERIAL_TIME_OUT);
		    break;
	    }
	    default: {
		    Debug2((dbg_fd,"\nmon: Serial op (0x%x)\n",HIGH(state->eax)));
		    return (FALSE);
		    break;
	    }
	}
	return (TRUE);
}

boolean_t bios_io_sub_fn(state)
	state_t *state;
{
	switch (HIGH(state->eax)) {
	    case IO_WAIT_ON_EXTERN_EVENT: {
		    Debug2((dbg_fd,"bios_misc: io_wait_on_extern_event.\n"));
		    return(UNCHANGED);
	    }
	    case IO_KEY_INTERCEPT: {
		    Debug2((dbg_fd, "\nmon: io_sub Keyboard Intercept\n"));
		    /* set carry flag */
		    return(FALSE);
		    break;
	    }
	    case IO_GET_SYS_ENV: {
		    Debug1((dbg_fd, "\rmon_io: get sys env.\n"));
		    SETWORD(&(state->es), Segment(SYS_CONFIG));
		    SETWORD(&(state->ebx), Offset(SYS_CONFIG));
		    return(UNCHANGED);
	    }
	    default: {
		    Debug0((dbg_fd,"\nmon: io_sub op (0x%x)\n", 
							HIGH(state->eax)));
		    return (bios_ems_fn(state));
	    }
	}
	return (TRUE);
}

extern FILE * printer_file;

boolean_t bios_printer_fn(state)
	state_t *state;
{
	switch (HIGH(state->eax)) {
	    case PRINTER_WRITE_CHAR: {
		    u_char ch = LOW(state->eax);
		    u_short ptr_num = WORD(state->edx);

		    Debug0((dbg_fd, "bios_misc: Printer_Write_Char. ch = '%c' 0x%x 0x%x\n",
			    ch, ch, ptr_num));
		    switch (ptr_num) {
			case LPT1:
			    fprintf(printer_file, "%c", ch);
			    SETHIGH(&(state->eax), LPT1_val);
			    break;
			default:
			    SETHIGH(&(state->eax), LPTX_val);
		    }
		    return(UNCHANGED);
		    break;
	    }
	    case PRINTER_INIT: {
		    u_short ptr_num = WORD(state->edx);

		    Debug0((dbg_fd, "bios_misc: Printer Init ptr_num = #%d\n", ptr_num));
		    switch (ptr_num) {
			case LPT1:
			    SETHIGH(&(state->eax), LPT1_val);
			    break;
			default:
			    SETHIGH(&(state->eax), LPTX_val);
		    }
		    return(UNCHANGED);
		    break;
	    }
	    case PRINTER_STATUS: {
		    u_short ptr_num = WORD(state->edx);
		    
		    Debug2((dbg_fd, "bios_misc: Printer Status ptr #%d\n", ptr_num));
		    switch (ptr_num) {
			/* Only support LPT1 now */
			case LPT1:
			    SETHIGH(&(state->eax), LPT1_val);
			    break;
			default:
			    SETHIGH(&(state->eax), LPTX_val);
		    }
		    break;
	    }
	    default: {
		    Debug2((dbg_fd,"bios_misc: Printer op (0x%x)\n",HIGH(state->eax)));
		    return (FALSE);
		    break;
	    }
	}
	return (TRUE);
}

#define PRINTER_VAL	(0x10)

u_char do_printer_in (port, byte_p, state)
	u_int port;
	boolean_t byte_p;
	state_t * state;
{
	u_char val = NULL;

	Debug0((dbg_fd,"bios_misc: printer io cs:eip = 0x%x:%x\n",
		state->cs, state->eip));

	switch (port) {
	    case 0x379:
		val = PRINTER_VAL;
		break;
	    default:
		break;
	}
	return (val);
}
