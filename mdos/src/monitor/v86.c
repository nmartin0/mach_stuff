/*
 * Copyright (c) 1992, 1991 Carnegie Mellon University
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
 * The V86 Mode Monitor:
 *
 * Create a v86 thread to share task's space with.
 * set up exception port to go to this thread.
 *
 * HISTORY:
 * $Log:	v86.c,v $
 * Revision 2.12  92/07/01  14:25:06  grm
 * 	Ifdef idle code out for release.
 * 	[92/07/01            grm]
 * 	Started adding idle code.
 * 	[92/06/30  13:44:41  grm]
 * 
 * Revision 2.11  92/05/22  15:59:42  grm
 * 	Fixed single drive installation bug, introduced by changing
 * 	config val in MDOS56.
 * 	[92/05/21            grm]
 * 
 * Revision 2.10  92/04/29  16:31:30  grm
 * 	Minor Changes.  Changed Int 0x11, get sys config to look at BIOS
 * 	SPRAM.  Added some coprocessor checking code.
 * 	[92/04/29  15:59:19  grm]
 * 
 * Revision 2.9  92/04/14  13:20:43  grm
 * 	Version to be used with the updated i386 pmap module which has
 * 	the fix for mapping page zero.  This is the version replaces the
 * 	previous experimental version that directly manipulated the
 * 	device interface.  It will not work with kernels without the
 * 	iopl_all feature.
 * 	[92/04/06            grm]
 * 
 * Revision 2.8  92/03/02  15:47:40  grm
 * 	Changed for use with MK69.  The EXC_MSG_SIZE_MAX hack was added.
 * 	[92/02/20            grm]
 * 
 * Revision 2.7  92/02/14  17:44:56  grm
 * 	Conditionalized the mem.0000, mem.0400, and bootstrap loading
 * 	code to use the DOSPREFIX environment variable when OSF_SVR is
 * 	defined.
 * 	[92/02/14            grm]
 * 	Added profiling code, coditionalized with PROFILING.  Moved the
 * 	console initializing code to bios_kbd.c
 * 	[92/02/12            grm]
 * 	Added vm_regions function.  Merged in the OSF Single Server
 * 	changes, specifically the mmap of the BIOS area.
 * 	[92/02/11            grm]
 * 
 * Revision 2.6  92/02/03  14:25:24  rvb
 * 	Clean Up
 * 
 * Revision 2.5  92/02/02  23:02:48  rvb
 * 	Fixed the DOSROOT problem with '/'
 * 	[92/01/29            grm]
 * 	Changed rhd0h to dosdisk.
 * 	[92/01/28            grm]
 * 	Replaced the exit(0)'s with exit_dos()'s.
 * 	[92/01/27            grm]
 * 
 * Revision 2.4  91/12/06  15:29:51  grm
 * 	Added MACH_FMD as 0xfb to tell the user when a mouse program has
 * 	cheated.  This is never called, so I never have put in
 * 	redirecting measures.  Also changed some debugging code in minor
 * 	ways.
 * 	[91/12/06            grm]
 * 
 * Revision 2.3  91/12/05  16:43:15  grm
 * 	Removed some obsolete code.  Added more debugging code.  Ifdefed
 * 	out the gdb dissassembler calls and the debugging code that
 * 	relied on them.  Changed the initialization code so that the
 * 	LPATH is searched for the initialization files.  Added code to
 * 	initialize the scratch pad ram keybuf area.
 * 	[91/12/04            grm]
 * 	Added support for DOSROOT.
 * 	[91/08/09  19:58:49  grm]
 * 
 * 	Removed vgets.
 * 	[91/07/16  17:54:53  grm]
 * 
 * 	Changes for xms.  Boot file looks at boot device instead of
 * 	ms.dos by default.
 * 	[91/06/28  19:03:55  grm]
 * 
 * 	Modified for dbg's v86 kernel support.
 * 	Check to make sure run from console if not
 * 	startup mode.  Added com2's thread.
 * 	[91/06/14  12:01:54  grm]
 * 
 * 	New Copyright.
 * 	[91/05/28  15:24:03  grm]
 * 
 * 	Removed tracing support.  Put in support 
 * 	for the startup switch -s.
 * 	[91/05/02  13:57:59  grm]
 * 
 * 	Minor changes.
 * 	[91/04/30  14:00:30  grm]
 * 
 * 	Boots off of /dev/rhd0h.
 * 	[91/03/26  19:07:20  grm]
 * 
 * 	Mach3 changes.
 * 	[91/02/01  13:33:27  grm]
 * 
 * 	Added support for repz outs instructions.
 * 	Changed debug from DebugX to XdebugX.
 * 	[90/11/09  21:07:49  grm]
 * 
 * 	Added hack to bypass inb(0x3da).  Restructured
 * 	do_in().  
 * 	[90/10/11  20:29:09  grm]
 * 
 * 	Utilize vgets support.  Started arithmetic code.
 * 	[90/10/04  21:06:56  grm]
 * 
 * 	Breakpoint exceptions now work from v86 thread.
 * 	[90/08/28  15:50:40  grm]
 * 
 * 	Modifier scan codes work.  Sped up 
 * 	Epsilon loop checks. Fixed the stack 
 * 	allocation problem that caused the 
 * 	segmentation faults.
 * 	[90/05/25  16:12:48  grm]
 * 
 * 	Added support for emulating keyboard
 * 	interrupts.  Changed do_in and do_out
 * 	around.
 * 	[90/05/08  17:22:31  grm]
 * 
 * 	Corrections.  Keyboard_loop thread and support
 * 	added.  Fixed ins.
 * 	[90/04/30  15:40:47  grm]
 * 
 * 	Put in io_port bitmap support and checking.
 * 	Changed fprintfs to use dbg_fd instead of stderr.
 * 	[90/04/17  22:42:14  grm]
 * 
 * 	Changed monitor names to video names.
 * 	Put in the -b flag.
 * 	[90/04/07  00:34:39  grm]
 * 
 * 	Added constants.  Added trace_state & vga_state support.
 * 	Changed the vga support for use with bios_vga_fn.
 * 	Enhanced tracing.  Reboot now exits instead of
 * 	pausing then continuing.  Added IN and OUT function-
 * 	ality.  Step over REALLY BAD INSTRUCTIONS.  Debugging
 * 	changes.  Added -v flag to enable vga setup.
 * 	[90/04/05  21:24:04  grm]
 * 
 * 	Version that maps in the physical memory
 * 	from 0xa0000 to 0x100000.
 * 	[90/03/30  17:12:29  grm]
 * 
 * 	Changed it back for use as a program. Now that
 * 	the load address can be specified by the -T flag
 * 	to ld.
 * 	[90/03/29  17:18:53  grm]
 * 
 * 	Started editing as grm.  Moved to v86 branch.
 * 	[90/03/28  18:38:18  grm]
 * 
 * Revision 2.1.1.5  90/03/22  21:52:29  dorr
 * 	Added vga support but ifdef out later. Made monitor easier to
 * 	break out of.  Changed some debug stuff to use Fprintfs.
 * 
 * Revision 2.1.1.4  90/03/19  17:40:43  orr
 * 	Added diagnostics.  Added support for the -dX flag.
 *	Initialized registers.
 * 
 * Revision 2.1.1.3  90/03/14  17:00:21  orr
 * 	first pass at generalizing device stuff.
 * 
 * Revision 2.1.1.2  90/03/13  15:26:12  orr
 * 	Version that gives an A:> prompt and does a DIR.
 * 	Added run time flags, replaced DEBUG with DebugX.
 * 	Settable dosdevice added to replace stdout.
 * 
 * Revision 2.1.1.1  90/03/12  02:16:09  orr
 * 	first working version.
 * 
 * Revision 1.1  90/03/09  15:38:14  grm
 * Initial revision
 * 
 * Revision 1.1  90/03/09  15:10:25  grm
 * Initial revision
 * 
 * Initial version grm (Gerald Malan) 2/?/90
 *
 */

#include "base.h"
#include "bios.h"
#include "bios_profile.h"

#include <stdio.h>
#include <machine/asm.h>
#include <machine/psl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <mach/message.h>
#include <mach/exception.h>

#include <mach/machine/vm_param.h>

#define EXC_TYPES	7
char *exception_names[6] = {
	"EXC_BAD_ACCESS",
	"EXC_BAD_INSTRUCTION",
	"EXC_ARITHMETIC",
	"EXC_EMULATION",
	"EXC_SOFTWARE",
	"EXC_BREAKPOINT" };

/*
 * XXX This is a BIG hack.  This should be changed.
 */
#define	EXC_MSG_SIZE_MAX	1024


#define	DO_NOPS		TRUE
#define	NOP_CLI_STI	TRUE

#define INT_01		0x01

#define BIOS_VIDEO	0x10
#define BIOS_CONFIG	0x11
#define BIOS_MEMORY	0x12
#define BIOS_DISK	0x13
#define BIOS_SERIAL	0x14
#define BIOS_IO_SUB	0x15
#define BIOS_KBD	0x16
#define BIOS_PRN	0x17
#define BIOS_BASIC	0x18
#define BIOS_REBOOT	0x19
#define BIOS_CLOCK	0x1a
#define DOS_GENERAL	0x21
#define DOS_TTY		0x29
#define BIOS_MOUSE	0x33
#define BIOS_EMM	0x67
#define	DOS_SAFE_TO_USE	0x28
#define DOS_NETWORK_INF	0x2a
#define DOS_EXTRA	0x2f
#define	MACH_EXIT	0xfa
#define MACH_FMD	0xfb
#define	MACH_XMS	0xfc
#define	MACH_MOUSE	0xfd
#define	MACH_FS_DEV	0xfe
#define BIOS_KBD_REDIRECT 0xff

#define DOS_FS_REDIRECT	0x11

#define I386_NOP	0x90
#define I386_INT	0xcd
#define I386_HLT	0xf4
#define I386_CLI	0xfa
#define I386_STI	0xfb
#define I386_PUSHF	0x9c
#define I386_POPF	0x9d
#define I386_LOCK	0xf0
#define I386_IRET	0xcf
#define I386_IN1	0xe4
#define I386_IN2	0xe5
#define I386_IN3	0xec
#define I386_IN4	0xed
#define I386_OUT1	0xe6
#define I386_OUT2	0xe7
#define I386_OUT3	0xee
#define I386_OUT4	0xef
#define I386_INS1	0x6c
#define I386_INS2	0x6d
#define I386_OUTS1	0x6e
#define I386_OUTS2	0x6f
#define I386_REPZ	0xf3
#define I386_OP_SIZE	0x66

#define DOS_BOOT_BLOCK_SIZE	0x200
#define BOOT_LOAD_ADDRESS	0x7c00

#define K64			0x40
#define DOS_MEM_MAX		(K64 * 10)
#define	USER_ADDR_MAX		0x0a0000
#define	BIOS_ADDR_MAX		0x100000
#define BIOS_ROM_EXPANSION	0x0c0000
#define EMM_START		0x0d0000
#define EMM_END			0x0e0000
#define BIOS_SYSTEM_ROM_EXP	0x0e0000
#define BIOS_SYSTEM_ROM_STD	0x0f0000
#define MON_TEXT_START		0x300000


#define EOS		'\0'
#define	SLASH		'/'
#define BACKSLASH	'\\'

/* page 535 of AMSDOS */
#define EQUIPMENT_CODE		0x4423
u_short equipment_code = EQUIPMENT_CODE;
u_short * equip_ptr = (u_short *)0x410;

int number_of_diskettes = 0;

idt_t idt = (idt_t)0;

#define  TOP_OF_STACK_AREA	(vm_address_t) 0xbfff0000
#define  BOTTOM_OF_STACK_AREA	(vm_address_t) 0xbff80000

vm_address_t lowest_stack_pointer;
thread_t v86_thread;
struct mutex cs_lock_data = {0,0};
mutex_t cs_lock = &cs_lock_data;
boolean_t cs_switch_needed = FALSE;
int cs_switch_turns = 0;
int cs_switch_count = 0;
u_char io_ports[MAX_IO_PORTS];
boolean_t floppy_boot = FALSE;

#ifdef	IDLE_WORK_IN_PROGRESS
struct mutex idle_lock_data = {0,0};
mutex_t idle_lock = &idle_lock_data;
mach_port_t idle_port;
boolean_t idling = FALSE;
#endif	/* IDLE_WORK_IN_PROGRESS */

/* bsd variable for osf vs bsd mmap allocation */
boolean_t bsd = FALSE;

int iopl_fd;
int dd_fd;
FILE * dbg_fd;
int us_debug_level;
int video_debug_level;
int key_debug_level;
int disk_debug_level;

int debug_toggle = 0;

onoff_t vga_state;
onoff_t debug_state;
boolean_t startup_flag = FALSE;

struct device devices[MAX_DEVICES];

int mon_space = 0xc0000;

int total_exceptions_processed = 0;

void keyboard_loop();
void timer_loop();
void mouse_loop();
void com2_loop();
/* void exit_loop(); */

char * ttyname();

char * getenv();
char * dos_root = NULL;
int dos_root_len;

#ifdef	OSF_SVR
char * dosprefix = NULL;
int prefix_len;
char osfdir[256];	/* Used for making the paths */
#else	/* BSD 4.3 UX Server */
char * lpath;
#endif	OSF_SVR


extern int exit_index;

#ifdef PROFILING
extern struct profile_type profile_info[PROF_INTS][PROF_AX];
extern profiling;
extern u_long total_exceptions;
extern u_long cli_exceptions;
extern u_long sti_exceptions;
extern u_long iret_exceptions;
extern u_long pushf_exceptions;
extern u_long popf_exceptions;
extern u_long in_exceptions;
extern u_long out_exceptions;
extern u_long realint_exceptions;

struct timeval profile_start;
struct timeval profile_end;

struct timezone tztmp;

int profile_int;
int profile_ax;
#endif PROFILING

extern char * names21[DOS21_NAMES];
extern char * bios_13_names[BIOS13_NAMES];

exit_loop()
{

}

/*
 *
 * Enable IOPL level so that we can map in physical memory.
 *
 */
void enable_iopl()
{
	if ((iopl_fd = open("/dev/iopl", O_RDWR)) < 0) {
		fprintf(dbg_fd,"Error in iopl_fd open. fd = %x\n", iopl_fd);
		exit_index = 12;
		exit_dos();
	}
}

enable_i386_ports(thread, ports)
thread_t	thread;
int *		ports;
{
	struct i386_isa_port_map_state	pm;
	int				i;
	u_char 				*p;
	kern_return_t			kret;

	i = i386_ISA_PORT_MAP_STATE_COUNT;
	kret = thread_get_state(thread,
				i386_ISA_PORT_MAP_STATE,
				&pm,
				&i);
	if (kret != KERN_SUCCESS) {
		printf("thread_get_state() ret = %d\n", kret);
		exit_index = 13;
		exit_dos();
	} 

	p = pm.pm;
#define bitclr(p, port)	p[(port)>>3] &= ~(1 << ((port) & 7))
	while ((i = *ports++)) {
		bitclr(p, i);
	}

	kret = thread_set_state(thread,
				i386_ISA_PORT_MAP_STATE,
				&pm,
				i386_ISA_PORT_MAP_STATE_COUNT);

	if (kret != KERN_SUCCESS) {
		printf("thread_set_state() ret = %d\n", kret);
		exit_index = 14;
		exit_dos();
	}
}

void pause(str)
char * str;
{
	char buf[1024];
	
	if (vga_state == VGA_ENABLED)
		bios_vga_fn(VGA_DISABLED);
	reset_tty();

	fflush (dbg_fd);
	printf("\n%s", str);
	while(1);

	if (vga_state == VGA_DISABLED)
		bios_vga_fn(VGA_ENABLED);
	set_tty();
}

void dump_state ( thread_state )
	state_t *thread_state;
{
	
	if (vga_state == VGA_ENABLED)
		bios_vga_fn(VGA_DISABLED);
	reset_tty();
	
	fprintf(dbg_fd, "\rmon:  -------------\n");
	fprintf(dbg_fd,"eax: 0x%8.8x ebx: 0x%8.8x ecx: 0x%8.8x edx: 0x%8.8x\n",
		thread_state->eax,
		thread_state->ebx,
		thread_state->ecx,
		thread_state->edx);

	fprintf(dbg_fd,"ebp: 0x%8.8x esp: 0x%8.8x eip: 0x%8.8x usp: 0x%8.8x\n",
		thread_state->ebp,
		thread_state->esp,
		thread_state->eip,
		thread_state->uesp);

	fprintf(dbg_fd,"gs:  0x%8.8x fs:  0x%8.8x es:  0x%8.8x ds:  0x%8.8x\n",
		thread_state->gs,
		thread_state->fs,
		thread_state->es,
		thread_state->ds);

	fprintf(dbg_fd,"cs:  0x%8.8x ss:  0x%8.8x edi: 0x%8.8x esi: 0x%8.8x\n",
		thread_state->cs,
		thread_state->ss,
		thread_state->edi,
		thread_state->esi);


	fprintf(dbg_fd, "efl: 0x%8.8x\n", thread_state->efl);
	fprintf(dbg_fd, "mon:  -------------\n");

	fprintf(dbg_fd, "mdos going down.... arrrggghhhhh, I can't hold it...\n");
	fprintf(dbg_fd, "...I'm melting.... look out!!! CCCCCRRRRRRAAAAASSHHHHH\n");
	/* you can still pause the program to get out of it on a 
	   non networked machine */
	while (TRUE) {
		sleep(500);
	}
	exit_dos();

	if (vga_state == VGA_DISABLED)
		bios_vga_fn(VGA_ENABLED);
	set_tty();
}

simulate_interrupt(state, interrupt, incr_eip)
state_t *state;
int	interrupt;
int	incr_eip;
{
    	u_short * sp;
	u_short cs = idt[interrupt].selector;
	u_short eip = idt[interrupt].offset;
	
	if (Addr_8086(cs,eip) < 0x400) {
	    	Debug2((dbg_fd,"Simulate interrupt bad: %x:%x\n", cs, eip));
		return;		
	}

    	Debug2((dbg_fd,"Simulate interrupt: 0x%x\n", interrupt));
    	state->eip += incr_eip;
    	sp = (u_short *)Addr(state, ss, uesp);
    	*--sp = (state->efl & EFL_SAFE);
    	*--sp = state->cs;
    	*--sp = state->eip;
    	state->uesp -= 6;
    	state->cs = idt[interrupt].selector;
    	state->eip = idt[interrupt].offset;
	state->efl &= ~(EFL_IF | EFL_TF);
    	Debug2((dbg_fd,"mon: CALLING 0x%x\n",Addr(state,cs,eip)));
}

/*
 *  From IBM tech ref pg 1-11:
 *   8088 Hardware Interrupt Listing
 *    number    usage
 *      0	Timer
 *	1	Keyboard
 *	2	Reserved
 *	3	ASync Secondary
 *	4	ASync Primary
 *	5	Fixed Disk
 *	6	Diskette
 *	7	Printer
 * 
 *	Port 0x20 - PIC Mask Reg (0 - enabled, 1 - disabled)
 *	Port 0x21 - PIC EOI Reg
 */
#define TIMER_EN	0x01
#define KBD_EN		0x02
#define ASY2_EN		0x08
#define ASY1_EN		0x10
#define FIX_EN		0x20
#define FLOP_EN		0x40
#define PRINT_EN	0x80

int pic_mask = 0;

int do_21(in_out, val, byte_word, state)
	boolean_t in_out;
	int val;
	boolean_t byte_word;
	state_t *state;
{
	int ret;
	static port_21_value = 0;
	if (in_out) {  /* true => in, false => out */
		ret = pic_mask;
	} else {
		pic_mask = val;
		ret = 0;
	}
	return (ret);
}

void do_breakpoint (eip, state)
	u_char *eip;
	state_t *state;
{
	char buf[1024];
	int numbytes;


	/* 
	 * Check and see if it was called from the v86 thread.
	 */
	if (!(state->efl & EFL_TF)) {
	/* (*(eip-(u_char)1) == 0xcc) */
		simulate_interrupt(state, 3, 1);
		return;
	}

	if (vga_state)
		bios_vga_fn(VGA_DISABLED);
	reset_tty();

	fprintf(dbg_fd, "\rbrk: efl = 0x%x\n",state->efl);

	/*
	 * PIC was used to do an int 1.
	 */
	if (debug_state) {
		simulate_interrupt(state, 1, 1);
		return;
	}

	Debug0((dbg_fd,"v86:  Shouldn't be here.  Problem with trace flag.\n"));

	if (vga_state)
		bios_vga_fn(VGA_ENABLED);
	set_tty();
		
}

void do_arithmetic (eip, state, code, subcode)
	u_char *eip;
	state_t *state;
	int code;
	int subcode;
{
	u_short * sp;
	u_char exno;

	/* 
	 * Process arithmetic exceptions.
	 */
	if (code == EXC_I386_DIV)
		exno = 0x00;
	else if (code == EXC_I386_INTO)
		exno = 0x04;
	else if (code == EXC_I386_BOUND)
		exno = 0x05;
	else if (code == EXC_I386_EXTERR)
		exno = 0x10;
	else {
		if (vga_state == VGA_ENABLED)
			bios_vga_fn(VGA_DISABLED);
		reset_tty();
		fprintf(dbg_fd,"mon.do_arithmetic(): kill or pause me NOW.");
		fprintf(dbg_fd, "\nmon: %s code = 0x%x subcode = 0x%x\n",
			exception_names[2], code, subcode);
		fprintf(dbg_fd,"mon: instruction = 0x%8.8x %x %x\n",*eip,
			*(eip+1), *(eip+2));
		dump_state (state);
		if (vga_state == VGA_DISABLED)
			bios_vga_fn(VGA_ENABLED);
		set_tty();
		return;
	}

	/*
	 * Call the interrupt service routine in the v86 process space.
	 * Not sure about calling semantics.  Should eip point before,at,
	 * or after the offending instruction? -XXX-
	 */

	Debug2((dbg_fd,"mon.do_arithmetic(): INT 0x%x\n", exno));
	Debug2((dbg_fd,"cs, eip = %x, 0x%x\n", state->cs, state->eip));
	simulate_interrupt(state, exno, 0);
	return;
}

void enable_inout_ports()
{
	PORT_ENABLE(0x201);
}

void do_int(eip, state)
	u_char *eip;
	state_t *state;
{
	boolean_t ret;
	int interrupt_number;
	state->eip++;
	eip++;
	interrupt_number = *eip;

#ifdef PROFILING
	if (profiling && (interrupt_number < PROF_INTS)) {
		profile_int = interrupt_number;

		switch (profile_int) {
		    case 0x2f:
			profile_ax = ((HIGH(state->eax) == 0x11) ?
				      LOW(state->eax) : 0);
			break;
		    case 0x33:
			profile_ax = ((WORD(state->eax) < PROF_AX) ?
				      WORD(state->eax) : 0xff);
			break;
		    default:
			profile_ax = HIGH(state->eax);
			break;
		}

		profile_info[profile_int][profile_ax].invoked++;
	}
#endif PROFILING

	if ((interrupt_number == DOS_GENERAL) && (HIGH(state->eax) < DOS21_NAMES )) {
		Debug1((dbg_fd,"Interrupt--21, ax = %x, bx = %x, cx = %x, dx = %x, ds = %x, eip = 0x%x:%x '%s'\n",
			WORD(state->eax), WORD(state->ebx), WORD(state->ecx),
			WORD(state->edx), 
			WORD(state->ds), state->cs, (state->eip -1),
			names21[HIGH(state->eax)]));
#ifdef NOTDEF
		if (us_debug_level > Debug_Level_1) {
			u_short * ptr = (u_short *)Addr(state, ss, uesp);
			int i;
			fprintf (dbg_fd,"Stack 16 words (hex): ");
			for (i=0; i < 16; i++) {
				fprintf(dbg_fd,"%x ", *ptr++);
			}
			fprintf(dbg_fd, "\n");
		}
#endif NOTDEF
	}else if ((interrupt_number == 0x13) && (HIGH(state->eax) < 0x1b)) {
		Debug1((dbg_fd,"Interrupt++13, ax = %x, bx = %x, cx = %x, dx = %x, eip = 0x%x:%x '%s'\n",
			WORD(state->eax), WORD(state->ebx), WORD(state->ecx),
			WORD(state->edx), state->cs, (state->eip -1),
			bios_13_names[HIGH(state->eax)]));
	}else{
		Debug1((dbg_fd,"Interrupt--%x, ax = %x, bx = %x, cx = %x, dx = %x, ds = %x es = %x eip = 0x%x:%x\n",
			interrupt_number, WORD(state->eax), WORD(state->ebx),
			WORD(state->ecx), WORD(state->edx), WORD(state->ds),
			WORD(state->es), state->cs, (state->eip -1)));
	}
	switch (*eip) {
#ifdef	DO_NOPS
	    case DOS_SAFE_TO_USE:
	    case DOS_NETWORK_INF:
		if (eip <= (u_char *) USER_ADDR_MAX) {
			eip--;
			*eip = I386_NOP;
			eip++;
			*eip = I386_NOP;
		}
		ret = TRUE;
		break; 
#endif	DO_NOPS
	    case INT_01:
		fprintf(dbg_fd,"mon: int 01 occurred.\n");
		(void)do_breakpoint(eip, state);
		break;
	    case MACH_EXIT:
		break;
	    case MACH_FS_DEV:
		Debug2((dbg_fd,"mon: mach_fs_dev interrupt\n"));
		ret = dos_fs_dev(state);
		break;
	    case MACH_FMD:
		/* Tell me something! */
		exit_index = 5;
		exit_dos();
		break;
	    case MACH_MOUSE:
		ret = do_mouse_interrupt(state);
		break;
	    case MACH_XMS:
		ret = bios_xms_fn(state);
		break;
	    case BIOS_VIDEO:
		ret = bios_video_fn(state);
		if (ret == REDIRECT) {
			goto redirect;
		}
		break;
	    case DOS_EXTRA:
		switch (HIGH(state->eax)) {
			case DOS_FS_REDIRECT:
				ret = dos_fs_redirect(state);
				if (ret == REDIRECT) {
					goto redirect;
				}
				break;
			case 0x16:
#ifdef	IDLE_WORK_IN_PROGRESS
				/* Idle loop */
				if (LOW(state->eax) == 0x80) {
					lock_idle();
					idling = TRUE;
					unlock_idle();
					condition_wait(idle_port);
				}
#endif	/* IDLE_WORK_IN_PROGRESS */

				if (LOW(state->eax) != 0x89) 
					goto redirect;
				if (eip <= (u_char *) USER_ADDR_MAX) {
					eip--;
					*eip = I386_NOP;
					eip++;
					*eip = I386_NOP;
				}
				ret = TRUE;
				break; 
			case 0x43:
				ret = bios_xms_driver(state);
				break;
			default:
				goto redirect;
		}
		break;
	    case BIOS_CONFIG:
		Debug2((dbg_fd,"mon: get config\n"));
#ifdef	WRONG
		SETWORD(&(state->eax), equipment_code);
#else
		SETWORD(&(state->eax), *equip_ptr);
#endif	WRONG
		ret = TRUE;
		break;
	    case BIOS_MEMORY:
		Debug2((dbg_fd,"mon: get conv mem size\n"));
		SETWORD(&(state->eax), DOS_MEM_MAX);
		ret = TRUE;
		break;
	    case BIOS_DISK:
		ret = bios_disk_fn(state);
		break;
	    case BIOS_SERIAL:
		ret = bios_serial_fn(state);
		break;
	    case BIOS_IO_SUB:
		ret = bios_io_sub_fn(state);
		break;
	    case BIOS_KBD:
		if (keyboard_bios_redirected()) goto redirect;
		ret = bios_kbd_fn(state);
		break;
	    case BIOS_KBD_REDIRECT:
		ret = bios_kbd_fn(state);
		if (ret == UNCHANGED) {
			u_short * sp;
			sp = (u_short *)Addr(state, ss, uesp);
			sp++; sp++;
			if (state->efl & EFL_ZF) {
				*sp |= EFL_ZF;
			} else {
				*sp &= ~EFL_ZF;
			}
		}
		break;
	    case BIOS_PRN:
		ret = bios_printer_fn(state);
		break;
	    case BIOS_BASIC:
		Fprintf((dbg_fd,"\nmon: INT 0x18 (basic loader) never IRETs.\n"));
		pause("mon: Suggest you kill me now.  If you don't I will freak.");
		break;
	    case BIOS_REBOOT:
		Fprintf((dbg_fd,"\nmon: INT 0x19 (bootstrap loader) never IRETs.\n"));
		/* don't do anything clever, just get out with hide intact*/
		exit_index = 16;
		exit_dos();
		break;
	    case BIOS_CLOCK:
		ret = bios_clock_fn(state); 
		break;
	    case BIOS_EMM:
		ret = bios_emm_fn(state);
		break;
	    case DOS_GENERAL:
		ret = dos_general_fn(state);
		if (ret == REDIRECT) {
			goto redirect;
		}
		break;
	    case BIOS_MOUSE:
		ret = bios_mouse_fn(state);
		break;
	    case DOS_TTY: {
		    u_char ch = LOW(state->eax);

		    if (vga_state) {
			    ret = do_tty_output(&ch, 0x07, 1);
			    if (ret == REDIRECT) {
				    goto redirect;
			    }
		    }else{
			    fprintf(stderr, "%c", ch);
		    }
		    break;		    
	    }
	    default: {
redirect:
		    Debug1((dbg_fd,"\rmon: redirected int 0x%x\n",(u_char)*eip));
		    simulate_interrupt(state, *eip, 1);
		    return;

	    }
	}
	if (ret == LEAVE_EIP_ALONE)
		return;
	if (ret != UNCHANGED)
		state->efl = (ret ? (state->efl & ~EFL_CF) : (state->efl | EFL_CF));
	state->eip++;
}	

void do_pushf_32 (eip, state)
	u_char *eip;
	state_t *state;
{
	u_short * sp;
	Debug2((dbg_fd, "mon: pushf 0x%x\n",state->efl));
	/* more than one byte to bypass */
	sp = (u_short *)Addr(state, ss, uesp);
	*--sp = state->efl;
	state->uesp -= sizeof(int);
	state->eip++;
}


void do_popf_32 (eip, state)
	u_char *eip;
	state_t *state;
{
	u_short * sp;

	/*
	 * Caution.  the use of EFL_TSAFE enables the v86 thread to
	 * turn off the tracing of the monitor thread.  This was added
	 * so that single stepping interrupts would still work like
	 * they should on the 8086.
	 */

	sp = (u_short *)Addr(state, ss, uesp);
	state->efl &= ~EFL_TSAFE;
	state->efl |= (*sp++ & EFL_TSAFE);
	state->uesp += sizeof(int);
	Debug2((dbg_fd, "mon: popf 0x%x 0x%x\n",*(sp-1),state->efl));

	state->eip++;
}

void do_pushf (eip, state)
	u_char *eip;
	state_t *state;
{
	u_short * sp;
	Debug2((dbg_fd, "mon: pushf 0x%x\n",state->efl));
	/* more than one byte to bypass */
	sp = (u_short *)Addr(state, ss, uesp);
	*--sp = state->efl;
	state->uesp -= sizeof(short);

	state->eip++;
}


void do_popf (eip, state)
	u_char *eip;
	state_t *state;
{
	u_short * sp;

	/*
	 * Caution.  the use of EFL_TSAFE enables the v86 thread to
	 * turn off the tracing of the monitor thread.  This was added
	 * so that single stepping interrupts would still work like
	 * they should on the 8086.
	 */

	sp = (u_short *)Addr(state, ss, uesp);
	state->efl &= ~EFL_TSAFE;
	state->efl |= (*sp++ & EFL_TSAFE);
	state->uesp += sizeof(short);
	Debug2((dbg_fd, "mon: popf 0x%x 0x%x\n",*(sp-1),state->efl));

	state->eip++;
}

void do_iret (eip, state)
	u_char *eip;
	state_t *state;
{
	extern keyboard_eip;
	extern timer_interrupt_eip;
#ifdef NOT_USED
	extern com_interrupt_eip;
#endif NOT_USED
	u_short * sp;
	sp = (u_short *)Addr(state, ss, uesp);
	state->eip = *sp++;
	state->cs = *sp++;
	state->efl &= ~EFL_SAFE;
	state->efl |= (*sp++ & EFL_SAFE);
	state->uesp += 6;

	Debug2((dbg_fd, "iret cs 0x%x eip 0x%x efl 0x%x\n",state->cs,state->eip,
		state->efl));
	Debug1((dbg_fd, "iret ax 0x%x bx 0x%x cx 0x%x dx 0x%x es 0x%x si 0x%x di 0x%x\n",
		state->eax, state->ebx, state->ecx, state->edx, state->es, state->esi,
		state->edi));
}

void do_in(eip, state)
	u_char *eip;
	state_t *state;
{
	char buf[1024];
	boolean_t byte_p = FALSE;
	u_char * addr;
	u_char first_byte;
	u_char second_byte;
	u_int port_addr;
	u_int val;
	u_int stepover;

	addr = (u_char *)Addr(state, cs, eip);
	first_byte = *(addr);
	second_byte = *(addr+1);

	switch (first_byte) {
	    case I386_IN1: {
		    port_addr = second_byte;
		    byte_p = TRUE;
		    stepover = 2;
		    break;
	    }
	    case I386_IN2: {
		    port_addr = second_byte;
		    stepover = 2;
		    break;
	    }
	    case I386_IN3: {
		    port_addr = WORD(state->edx);
		    byte_p = TRUE;
		    stepover = 1;
		    break;
	    }
	    case I386_IN4: {
		    port_addr = WORD(state->edx);
		    stepover = 1;
		    break;
	    }
	    case I386_INS1: {
		    port_addr = WORD(state->edx);
		    byte_p = TRUE;
		    stepover = 1;
		    break;
	    }
	    case I386_INS2: {
		    port_addr = WORD(state->edx);
		    stepover = 1;
		    break;
	    }
	    default: {
		    fprintf(dbg_fd,"\rmon: Undefined in: %x %x\n",
			    first_byte, second_byte);
		    stepover = -1;
		    break;
	    }
	}

	/*
	 * Do special things for certain ports.
	 */
	switch (port_addr) {
	    case 0x43: {
		    Debug0((dbg_fd, "\nin 43 %x\n",port_addr));
		    val = (char) do_43(TRUE, NULL, byte_p, state);
		    break;
	    }
	    case 0x20: {
		Debug0((dbg_fd, "\nin 20\n"));
#ifdef	DO_NOPS
		switch (first_byte) {
			case I386_IN1: 
			{
 			    u_char *addr = (u_char *)Addr(state, cs, eip);
			    *addr = 0x30;
			    state->eip++;
			    stepover--; addr++;
			    *addr = 0xc0;
			    state->eip++;
			    stepover--;
			    break;
			}
			case I386_IN2: 
			{
 			    u_char *addr = (u_char *)Addr(state, cs, eip);
			    *addr = 0x31;
			    state->eip++;
			    stepover--; addr++;
			    *addr = 0xc0;
			    state->eip++;
			    stepover--;
			    break;
			}
			default:
			    val = 0;
			    break;
		}		
#else	DO_NOPS
		val = 0;
		break;
#endif	DO_NOPS
	    }
	    case 0x21: {
		    Debug0((dbg_fd, "\nin 21\n"));
#ifdef	DO_NOPS
		switch (first_byte) {
			case I386_IN1: 
			{
 			    u_char *addr = (u_char *)Addr(state, cs, eip);
			    *addr = 0x30;
			    state->eip++;
			    stepover--; addr++;
			    *addr = 0xc0;
			    state->eip++;
			    stepover--;
			    break;
			}
			case I386_IN2: 
			{
 			    u_char *addr = (u_char *)Addr(state, cs, eip);
			    *addr = 0x31;
			    state->eip++;
			    stepover--; addr++;
			    *addr = 0xc0;
			    state->eip++;
			    stepover--;
			    break;
			}
			default:
		    	    val = (char) do_21(TRUE, NULL, byte_p, state);
			    break;
		}		
#else	DO_NOPS
		    val = (char) do_21(TRUE, NULL, byte_p, state);
		    break;
#endif	DO_NOPS
	    }
	    case 0x60: {
		    Debug0((dbg_fd, "\nin 60 %x\n",port_addr));
		    val = (char) do_60(TRUE, NULL, byte_p, state);
		    break;
	    }
	    case 0x61: {
		    Debug0((dbg_fd, "\nin 61 %x\n",port_addr));
		    val = (char) do_61(TRUE, NULL, byte_p, state);
		    break;
	    }
	    case 0x64: {
		    Debug0((dbg_fd, "\nin 64 %x\n",port_addr));
		    val = (char) do_64(TRUE, NULL, byte_p, state);
		    break;
	    }
	    case 0xf0:
	    case 0xf1:
	    case 0xf8:
	    case 0xf9:
	    case 0xfa:
	    case 0xfb:
	    case 0xfc:
	    case 0xfd:
	    case 0xfe:
	    case 0xff: {
		    /* Coprocessor dealies */
		    Debug0((dbg_fd,"Coprocessor in(0x%x) \n",port_addr));
		    if (byte_p) {
			    val = inb(port_addr);
		    }else{
			    val = inw(port_addr);
		    }
		    break;
		
	    }
	    case 0x378:
	    case 0x379:
	    case 0x37a: {
		    val = (char) do_printer_in(port_addr, byte_p, state);
		    break;
	    }
	    case 0x3f8:
	    case 0x3f9:
	    case 0x3fa:
	    case 0x3fb:
	    case 0x3fc:
	    case 0x3fd:
	    case 0x3fe: {
		    val = do_com1_port(TRUE, port_addr, NULL, byte_p, state);
		    break;
	    }
	    case 0x2f8:
	    case 0x2f9:
	    case 0x2fa:
	    case 0x2fb:
	    case 0x2fc:
	    case 0x2fd:
	    case 0x2fe: {
		    val = do_com2_port(TRUE, port_addr, NULL, byte_p, state);
		    break;
	    }
	    case 0x201: {
		    val = 0;
		    break;
	    }
	    case 0x331: { /* setup for castles.bat */
		    val = 0;
		    break;
	    }
	    default: {
		    if (stepover == -1) {
			    stepover = 1;
			    val = 0;
		    }else if (byte_p) {
			    val = inb(port_addr);
		    } else {
			    val = inw(port_addr);
		    }
		    break;
	    }
	}

	if (first_byte == I386_INS1) {
		u_char * ptr;
		
		ptr = (u_char *)Addr(state, es, edi);
		*ptr = (u_char)val;
		if (state->efl & EFL_DF) 
			state->edi -= 1;
		else 
			state->edi += 1;
	}else if (first_byte == I386_INS2) {
		u_short * ptr;
		
		ptr = (u_short *)Addr(state, es, edi);
		*ptr = (u_short)val;
		if (state->efl & EFL_DF) 
			state->edi -= 2;
		else 
			state->edi += 2;
	}else{
		if (byte_p) {
			SETLOW(&(state->eax), val);
			Debug2((dbg_fd,"\rmon: inb 0x%x = 0x%x\n",port_addr,val));
		} else {
			SETWORD(&(state->eax), val);
			Debug2((dbg_fd,"\rmon: inw 0x%x = 0x%x\n",port_addr,val));
		}
	}

	state->eip += stepover;
}

void do_out(eip, state)
	u_char *eip;
	state_t *state;
{
	char buf[1024];
	boolean_t byte_p = FALSE;
	u_char * addr;
	u_char first_byte;
	u_char second_byte;
	u_int port;
	u_int val;
	u_int stepover;

	addr = (u_char *)Addr(state, cs, eip);
	first_byte = *(addr);
	second_byte = *(addr+1);
	
	switch (first_byte) {
	    case I386_OUT1: {
		    port = second_byte;
		    val = LOW(state->eax);
		    byte_p = TRUE;
		    stepover = 2;
		    break;
	    }
	    case I386_OUT2: {
		    port = second_byte;
		    val = WORD(state->eax);
		    byte_p = FALSE;
		    stepover = 2;
		    break;
	    }
	    case I386_OUT3: {
		    port = WORD(state->edx);
		    val = LOW(state->eax);
		    byte_p = TRUE;
		    stepover = 1;
		    break;
	    }
	    case I386_OUT4: {
		    port = WORD(state->edx);
		    val = WORD(state->eax);
		    byte_p = FALSE;
		    stepover = 1;
		    break;
	    }
	    case I386_OUTS1: {
		    u_char * ptr;

		    port = WORD(state->edx);
		    ptr = (u_char *)Addr(state, ds, esi);
		    val = *ptr;
		    byte_p = TRUE;
		    stepover = 1;
		    if (state->efl & EFL_DF) {
			    state->esi -= 1;
		    }else{
			    state->esi += 1;
		    }
		    break;
	    }
	    case I386_OUTS2: {
		    u_short * ptr;

		    port = WORD(state->edx);
		    ptr = (u_short *)Addr(state, ds, esi);
		    val = *ptr;
		    byte_p = FALSE;
		    stepover = 1;
		    if (state->efl & EFL_DF) {
			    state->esi -= 2;
		    }else{
			    state->esi += 2;
		    }
		    break;
	    }
	    default: {
		    fprintf(dbg_fd,"\rmon: Undefined out: %x %x\n",
			    first_byte, second_byte);
		    stepover = -1;
		    break;
	    }
	}

	if (port == 0x60) {
		Debug0((dbg_fd, "\nout 60 %x\n",port));
		(void) do_60(FALSE, val, byte_p, state);
	} else if (port == 0x40) {
		Debug0((dbg_fd, "\nout 40 %x\n",port));
		(void) do_40(FALSE, val, byte_p, state);
	} else if (port == 0x42) {
		Debug0((dbg_fd, "\nout 42 %x\n",port));
		(void) do_42(FALSE, val, byte_p, state);
	} else if (port == 0x43) {
		Debug0((dbg_fd, "\nout 43 %x\n",port));
		(void) do_43(FALSE, val, byte_p, state);
        } else if (port == 0x61) {
		Debug0((dbg_fd, "\nout 61 %x\n",port));
		(void) do_61(FALSE, val, byte_p, state);
	} else if (port == 0x21) {
		Debug0((dbg_fd, "\nout 21 %x\n",val));
#ifdef 	DO_NOPS
		while (stepover > 0) {
			u_char *addr = (u_char *)Addr(state, cs, eip);
			*addr = I386_NOP;
			state->eip++;
			stepover--;
		}
#else	DO_NOPS
		(void) do_21(FALSE, val, byte_p, state);
#endif	DO_NOPS
	} else if ((port > 0x3f7) && (port < 0x3ff)) {
		do_com1_port(FALSE, port, val, byte_p, state);
	} else if ((port > 0x2f7) && (port < 0x2ff)) {
		do_com2_port(FALSE, port, val, byte_p, state);
	} else if (port == 0x20) {
		Debug0((dbg_fd, "\nout 20 %x\n",val));
#ifdef 	DO_NOPS
		while (stepover > 0) {
			u_char *addr = (u_char *)Addr(state, cs, eip);
			*addr = I386_NOP;
			state->eip++;
			stepover--;
		}
#endif	DO_NOPS
	} else if (stepover == -1) {
		stepover = 1;
	} else if (byte_p) {
		if (!PORT_OK(port)) {
			Debug0((dbg_fd,"\rmon: Bad port 0x%x 0x%x\n",port,val));
		} else {
			outb (port, val);
			Debug2((dbg_fd,"\rmon: Outb 0x%x,%#x\n",port,val));
		}
	} else {
		if (!(PORT_OK(port) && PORT_OK(port+1))) {
			Debug0((dbg_fd,"\rmon: Bad ports 0x%x\n",port));
		} else {
			outw (port, val);
			Debug2((dbg_fd,"\rmon: Outw 0x%x,%#x\n",port,val));
		}
	}

	state->eip += stepover;
}

void do_repz (eip,state)
	u_char * eip;
	state_t * state;
{
	u_short io_port;
	u_char * addr;

	addr = (u_char *)Addr(state,cs,eip);

	/* if is an outs bytes */
	switch (*(addr + 1)) {
	    case 0x6e: {
		    u_short count;
		    u_char * ptr;

		    ptr = (u_char *)Addr(state,ds,esi);
		    io_port = WORD(state->edx);
		    count = WORD(state->ecx);

		    if (!PORT_OK(io_port)) {
			    Debug0((dbg_fd,"\rmon: Bad port in repz 0x%x\n",io_port));
		    }else{
			    for (; count; count--) {
				    outb ( io_port, *(ptr++) );
			    }
		    }

		    state->ecx &= 0xffff0000;
		    state->eip += 0x02;
		    break;
	    }
	    default: {
		    char outbuf[1024];
		    int stepover;
		    if (vga_state == VGA_ENABLED)
			    bios_vga_fn(VGA_DISABLED);
		    reset_tty();
		    fprintf(dbg_fd,"\nmon: REAL BAD REPZ instruction=0x%x %x %x\n",
			    *eip, *(eip+1), *(eip+2));
#ifdef USE_GDB_STUFF
		    stepover = i386dis (FALSE, eip, eip, outbuf);
		    fprintf(dbg_fd, "ins: 0x%8.8x     %s\n", 
			    Addr(state,cs,eip), outbuf);
#endif /* USE_GDB_STUFF */
		    dump_state (state);
		    state->eip += stepover;
		    pause("mon: pausing before exiting do_bad_instruction.");
		    if (vga_state == VGA_DISABLED)
			    bios_vga_fn(VGA_ENABLED);
		    set_tty();
		    break;
	    }
	}
}

/*
 *  A bad instruction GP fault occurred.
 *  Determine if it should be emulated.
 */
void do_bad_instruction (eip, state)
	u_char *eip;
	state_t *state;
{
	boolean_t	operand_size_override = FALSE;
restart:
	switch (*eip) {
	    case I386_INT:
#ifdef	PROFILING
		if (profiling) {
			realint_exceptions++;
		}
#endif	PROFILING
		do_int(eip, state);
		break;
	    case I386_CLI:
		Debug2((dbg_fd, "mon: CLI\n"));
		Debug2((dbg_fd, "cs 0x%x eip 0x%x efl 0x%x\n",
			       state->cs, state->eip, state->efl));
#ifdef	PROFILING
		if (profiling) {
			cli_exceptions++;
		}
#endif	PROFILING
#ifdef	NOP_CLI_STI
		/* make cli a nop */
		if (eip <= (u_char *) USER_ADDR_MAX)
			*eip = I386_NOP;
#endif	NOP_CLI_STI
		/* step over it */
		state->eip++;
		break;
	    case I386_HLT:
		Debug2((dbg_fd, "mon: HLT\n"));
		/* step over it */
		state->eip++;
		break;
	    case I386_STI:
		Debug2((dbg_fd, "mon: STI\n"));
		Debug2((dbg_fd, "cs 0x%x eip 0x%x efl 0x%x\n",
			       state->cs, state->eip, state->efl));
#ifdef	PROFILING
		if (profiling) {
			sti_exceptions++;
		}
#endif	PROFILING
#ifdef	NOP_CLI_STI
		/* make sti a nop */
		if (eip <= (u_char *) USER_ADDR_MAX)
			*eip = I386_NOP;
#endif	NOP_CLI_STI
		/* step over it */
		state->eip++;
		break;
	    case I386_OP_SIZE:
		operand_size_override = TRUE;
		state->eip++;
		eip++;
		goto restart;
	    case I386_PUSHF:
		Debug2((dbg_fd, "pushf cs 0x%x eip 0x%x efl 0x%x\n",
				state->cs, state->eip, state->efl));
#ifdef	PROFILING
		if (profiling) {
			pushf_exceptions++;
		}
#endif	PROFILING
		if (operand_size_override)
			do_pushf_32(eip, state);
		else 	do_pushf(eip, state);
		break;
	    case I386_POPF:
		Debug2((dbg_fd, "popf cs 0x%x eip 0x%x efl 0x%x\n",
				state->cs, state->eip, state->efl));
#ifdef	PROFILING
		if (profiling) {
			popf_exceptions++;
		}
#endif	PROFILING
		if (operand_size_override) 
		     do_popf_32(eip, state);
		else do_popf(eip, state);
		break;
	    case I386_LOCK:
		Fprintf((dbg_fd, "\nmon: LOCK\n"));
		state->eip++;
		break;
	    case I386_IRET:
#ifdef	PROFILING
		if (profiling) {
			iret_exceptions++;
		}
#endif	PROFILING
		do_iret (eip, state);
		break;
	    case I386_IN1:
	    case I386_IN2:
	    case I386_IN3:
	    case I386_IN4:
	    case I386_INS1:
	    case I386_INS2:
#ifdef	PROFILING
		if (profiling) {
			in_exceptions++;
		}
#endif	PROFILING
		do_in(eip, state);
		break;
	    case I386_OUT1:
	    case I386_OUT2:
	    case I386_OUT3:
	    case I386_OUT4:
	    case I386_OUTS1:
	    case I386_OUTS2:
#ifdef	PROFILING
		if (profiling) {
			out_exceptions++;
		}
#endif	PROFILING
		do_out(eip, state);
		break;
	    case I386_REPZ:
		do_repz(eip, state);
		break;
	    default: {
		    char outbuf[1024];
		    int stepover;
		    if (vga_state == VGA_ENABLED)
			    bios_vga_fn(VGA_DISABLED);
		    reset_tty();
		    fprintf(dbg_fd,"\nmon: REAL BAD instruction=0x%x %x %x\n",
			    *eip, *(eip+1), *(eip+2));
#ifdef USE_GDB_STUFF
		    stepover = i386dis (FALSE, eip, eip, outbuf);
		    fprintf(dbg_fd, "ins: 0x%8.8x     %s\n", 
			    Addr(state,cs,eip), outbuf);
#endif /* USE_GDB_STUFF */
		    dump_state (state);
		    state->eip += stepover;
		    pause("mon: pausing before exiting do_bad_instruction.");
		    if (vga_state == VGA_DISABLED)
			    bios_vga_fn(VGA_ENABLED);
		    set_tty();
		    break;
	    }
	}
}

void do_bad_access(eip, state, addr)
	u_char *eip;
	state_t *state;
	vm_offset_t addr;
{
	char outbuf[1024];
	int numbytes;
	
#ifdef USE_GDB_STUFF
	numbytes = i386dis (FALSE, eip, eip, outbuf);
#endif /* USE_GDB_STUFF */
	if (vga_state == VGA_ENABLED)
		bios_vga_fn(VGA_DISABLED);
	reset_tty();
#ifdef USE_GDB_STUFF
	fprintf(dbg_fd, "\nacc(0x%8.8x): 0x%8.8x     %s  %2.2x%2.2x%2.2x%2.2x (%d)", 
		addr, Addr(state,cs,eip), outbuf, 
		*(eip), *(eip+1), *(eip+2), *(eip+3),
		numbytes);
#endif /* USE_GDB_STUFF */

	exit_dos();        

	pause("mon: Hit any key to continue, ^Z to pause or ^C to kill.");

	if (vga_state == VGA_DISABLED)
		bios_vga_fn(VGA_ENABLED);
	set_tty();

	state->efl = (state->efl | EFL_CF);
	state->eip += numbytes;
}

void bios_init()
{
	int i;
	for (i=0;i < MAX_IO_PORTS;i++)
		io_ports[i] = 0;
	bios_disk_init();
	bios_kbd_init();
	bios_emm_init();
}

kern_return_t catch_exception_raise (port, thread, task,
				     exception, code, subcode)
	mach_port_t port;
	thread_t thread;
	task_t task;
	int exception, code, subcode;
{
	int instruction;
	int state_count;
	u_char * eip;
	state_t state;

#ifdef	PROFILING
	if (profiling) {
		profile_int = -1;

		total_exceptions++;

		gettimeofday(&profile_start, &tztmp);

	}
#endif	PROFILING

	state_count = i386_THREAD_STATE_COUNT;
	MACH_CALL(( thread_get_state (thread, i386_THREAD_STATE,
				      &state, &state_count )),
		  "thread_get_state");

	eip = (u_char *)Addr(&state, cs, eip);

	switch (exception) {
	    case EXC_BAD_INSTRUCTION:
		do_bad_instruction (eip, &state);
		MACH_CALL(( thread_set_state ( thread, i386_THREAD_STATE,
					      &state, state_count )),
			  "thread_set_state do_bad_instruction");
		break;
	    case EXC_BREAKPOINT:
		do_breakpoint (eip, &state);
		MACH_CALL(( thread_set_state ( thread, i386_THREAD_STATE,
					      &state, state_count )),
			  "thread_set_state do_breakpoint");
		break;
	    case EXC_BAD_ACCESS:
		do_bad_access(eip, &state, subcode);
		MACH_CALL(( thread_set_state ( thread, i386_THREAD_STATE,
					      &state, state_count )),

			  "thread_set_state do_bad_access");
		break;
	    case EXC_ARITHMETIC: 
		do_arithmetic (eip, &state, code, subcode);
		MACH_CALL(( thread_set_state ( thread, i386_THREAD_STATE,
					      &state, state_count )),

			  "thread_set_state do_bad_access");
		break;
	    default:
		if (vga_state == VGA_ENABLED)
			bios_vga_fn(VGA_DISABLED);
		reset_tty();
		fprintf(dbg_fd,"mon: pausing before exiting catch_exception_raise");
		if (exception < EXC_TYPES)
			fprintf(dbg_fd, "\nmon: %s code = 0x%x subcode = 0x%x\n",
				exception_names[exception - 1], code, subcode);
		else
			fprintf(dbg_fd,
				"\nmon: exception = 0x%x code = 0x%x subcode = 0x%x\n",
				exception, code, subcode);
		fprintf(dbg_fd,"mon: instruction = 0x%8.8x %x %x\n",*eip, *(eip+1), *(eip+2));
		dump_state (&state);
		if (vga_state == VGA_DISABLED)
			bios_vga_fn(VGA_ENABLED);
		set_tty();
		break;
	}		

#ifdef	PROFILING
	if (profiling && (profile_int != -1)) {
		u_long udiff, sdiff, adiff;

		gettimeofday(&profile_end, &tztmp);

		if (profile_end.tv_usec >= profile_start.tv_usec) {
			udiff = profile_end.tv_usec - profile_start.tv_usec;
		}else{
			profile_end.tv_usec += 1000000;
			udiff = profile_end.tv_usec - profile_start.tv_usec;
			profile_end.tv_sec--;
		}

		sdiff = profile_end.tv_sec - profile_start.tv_sec;

		adiff = (sdiff * 1000) + udiff/1000;
		
		if (profile_info[profile_int][profile_ax].max_time < adiff) {
			profile_info[profile_int][profile_ax].max_time = adiff;
		}

		profile_info[profile_int][profile_ax].ave_time += adiff;
	}
#endif	PROFILING

	check_interrupt(TRUE);
	return(KERN_SUCCESS);
}

void process_exceptions ( exc_port )
	mach_port_t exc_port;
{
	kern_return_t ret;
	
	struct msg {
		mach_msg_header_t    header;
		int             data[EXC_MSG_SIZE_MAX - sizeof (mach_msg_header_t)];
	} in_msg, out_msg, * inptr, * outptr, *tmp;

	/* 
	 * Here's where we do the waiting part
	 */
	inptr  = &in_msg;
 	outptr = &out_msg;

	(void) mach_msg(inptr, MACH_RCV_MSG, 0, sizeof(in_msg), exc_port,
		 0, MACH_PORT_NULL);

	while (TRUE) {
		(void) exc_server (inptr, outptr);
		(void) mach_msg(outptr, MACH_SEND_MSG | MACH_RCV_MSG, 
			 outptr->header.msgh_size, sizeof(in_msg),
			 exc_port, 0, MACH_PORT_NULL);
		total_exceptions_processed++;
		tmp = inptr;
		inptr = outptr;
		outptr = tmp;
	}
}

void set_up_v86thread (v86thread, except_port, entry_point)
	thread_t *v86thread;
	mach_port_t *except_port;
	vm_address_t entry_point;
{
	mach_port_t	tmp1, tmp2;
	int state_count;
	int enable_ports[] = {0x380, 0x381, 0x382, 0x383, 0x384,
			      0x385, 0x386, 0x387, 0x388, 0x389,
			      0x38a, 0x38b, 0x38c, 0x38d, 0x38d, 0x38e, 0x38f,
			      0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4,
			      0x3c5, 0x3c6, 0x3c7, 0x3c8, 0x3c9,
			      0x3ca, 0x3cb, 0x3cc, 0x3cd, 0x3ce, 0x3cf,
			      0x3d0, 0x3d1, 0x3d2, 0x3d3, 0x3d4,
			      0x3d5, 0x3d6, 0x3d7, 0x3d8, 0x3d9,
			      0x3da, 0x3db, 0x3dc, 0x3dd, 0x3de, 0x3df,
			      0x42, 0x61, 0x201, 0x2fd, 0x2f8,
			      0x220, 0x221, 0x222, 0x223, 0x224, 0x225, 0x226,
		      	      0x227, 0x228, 0x229, 0x22a, 0x22b, 0x22c, 0x22d,
			      0x22e, 0x22f, 0};

	state_t thread_state;

	MACH_CALL((thread_create(mach_task_self(), v86thread)),
		  "thread_create of v86thread");

	MACH_CALL((mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, except_port)),
		  "port_allocate of except_port");

	mach_port_extract_right(mach_task_self(), *except_port, 
				MACH_MSG_TYPE_MAKE_SEND, &tmp1, &tmp2);

	MACH_CALL((thread_set_special_port ( *v86thread,
					    THREAD_EXCEPTION_PORT,
					    *except_port)),
		  "thread_set_special_port");
	
	state_count = i386_THREAD_STATE_COUNT;
	MACH_CALL((thread_get_state(*v86thread, i386_THREAD_STATE,
				    &thread_state, &state_count )),
		  "thread_get_state");

	thread_state.eip = entry_point;
	thread_state.efl = (thread_state.efl | EFL_VM);

	thread_state.cs = 0x00;
	thread_state.ds = 0x00;
	thread_state.ss = 0x00;
	thread_state.gs = 0x00;
	thread_state.es = 0x00;
	thread_state.ebp = 0x00;
	thread_state.eax = 0x00;
	thread_state.ebx = 0x00;
	thread_state.ecx = 0x00;
	thread_state.edx = 0x00;
	thread_state.edi = 0x00;
	thread_state.esi = 0x00;
	thread_state.uesp = 0x00;
	
	MACH_CALL((thread_set_state(*v86thread, i386_THREAD_STATE,
				    &thread_state, state_count )),
		  "thread_set_state");

	enable_i386_ports(*v86thread, enable_ports);

}

void create_a_thread(the_thread, entry_point)
	thread_t * the_thread;
	vm_address_t entry_point;
{
	int state_count;
	state_t thread_state;
	
	MACH_CALL((thread_create(mach_task_self(), the_thread)),
		  "thread_create in create a thread");
	
	state_count = i386_THREAD_STATE_COUNT;
	MACH_CALL((thread_get_state(*the_thread, i386_THREAD_STATE,
				    &thread_state, &state_count )),
		  "thread_get_state");
	
	thread_state.eip = entry_point;


	if ((thread_state.uesp > lowest_stack_pointer) ||
	    (thread_state.uesp < BOTTOM_OF_STACK_AREA)) {
		thread_state.uesp = lowest_stack_pointer - 0x4000;
	} else {
		thread_state.uesp = thread_state.uesp - 0x4000;
	}

	lowest_stack_pointer = thread_state.uesp;

	MACH_CALL((thread_set_state(*the_thread, i386_THREAD_STATE,
				    &thread_state, state_count )),
		  "thread_set_state create_a_thread");

}

void vm_regions()
{
	vm_address_t address;
	vm_size_t size;
	vm_prot_t protection;
	vm_prot_t max_protection;
	vm_inherit_t inheritance;
	boolean_t shared;
	mach_port_t  object_name;
	vm_offset_t offset;
	kern_return_t r;

	return;

	fprintf(stderr,"+");
	sleep(2);
    
	fprintf(dbg_fd,
	    "Start      Size       P    Mp   Ih   Sh  Object     Offset\n");

	for (address = 0;
	     ((r = vm_region(mach_task_self(), &address, &size,
			     &protection, &max_protection,
			     &inheritance, &shared,
			     &object_name, &offset)) == KERN_SUCCESS);) {
		fprintf(dbg_fd,
		        "0x%8.8x 0x%8.8x 0x%2.2x 0x%2.2x 0x%2.2x %2.2d",
		       	address, size, protection, max_protection,
		       	inheritance, shared);
			
		fprintf(dbg_fd, "  0x%8.8x 0x%8.8x\n", object_name, offset);
		if (size == 0)
			break;
		address += size;
	}

	fprintf(stderr,"-");
	sleep(2);

}

void set_up_task_vm ( )
{
	char *map_addr;
	char *addr;
	char *first_page;
	vm_offset_t address;
	vm_size_t mem_size, size_returned;
	vm_prot_t protection, max_protection;
	vm_inherit_t inheritance;
	boolean_t shared;
	mach_port_t object_name;
	mach_port_t obj;
	vm_offset_t offset, mask;
	kern_return_t ret;
	int fd, rc;

	if (us_debug_level > 0) {
		fprintf(dbg_fd,"Start of set_up_task_vm\n");
		vm_regions(dbg_fd);
	}

	addr = 0;
	size_returned = MON_TEXT_START;
	MACH_CALL((vm_deallocate(mach_task_self(), addr, size_returned)),
		  "vm_deallocate of low memory.\n");
	if (us_debug_level > 0) {
		fprintf(dbg_fd,"vm_deallocate of entire 3 megs\n");
		vm_regions();
	}

	/* enable the iopl level so that we can do the mmaps */
	enable_iopl();

	addr = 0;
	MACH_CALL((vm_allocate(mach_task_self(), &addr, BIOS_ADDR_MAX, FALSE)),
		  "vm_allocate of task address space.");
	if (us_debug_level > 0) {
		fprintf(dbg_fd,"vm_allocate of task address space\n");
		vm_regions();
	}

#define BIOS_SIZE	BIOS_ADDR_MAX - BIOS_SYSTEM_ROM_EXP
#define NON_USER_SIZE	BIOS_ADDR_MAX - USER_ADDR_MAX

	/* map in the bios rom */
	map_addr = (char *) USER_ADDR_MAX;
	size_returned = NON_USER_SIZE;

	MACH_CALL((vm_deallocate(mach_task_self(), map_addr, size_returned)),
		  "vm_deallocate of mmap area's memory");
	map_addr = (char *) USER_ADDR_MAX;
	
	Debug2((dbg_fd,"map_addr = 0x%x\n",map_addr));
	Debug2((dbg_fd,"bios_size = 0x%x\n",NON_USER_SIZE));

	/* Try OSF mmap first, then if fails try UX BSD Server mmap */
       	if ((rc = mmap(map_addr, NON_USER_SIZE, 3, 0x21, iopl_fd, 0)) < 0) {
		Debug0((dbg_fd,"mmap of bios rom, osf style failed.\n"));
		
		/* try with vm_allocated */
		
		map_addr = (char *) USER_ADDR_MAX;
		size_returned = NON_USER_SIZE;
		MACH_CALL((vm_allocate(mach_task_self(), &map_addr,
				       size_returned, FALSE)),
			  "vm_allocate of mmap area's memory");

		if ((rc = mmap(0, 0x100000, 3, 0x1, iopl_fd, 0x0)) < 0) {
			perror("mmap of bios rom, bsd style failed");
			exit(-1);
		}

		bsd = TRUE;
	}

	Debug0((dbg_fd,"mmap address = 0x%x\n",rc));

	addr = (char *) 0xd0000;
	MACH_CALL((vm_deallocate(mach_task_self(), (vm_address_t)0xd0000,
			      (vm_size_t)0x10000)), "vm_deallocate");
	if (us_debug_level > 0) {
		fprintf(dbg_fd,"vm_deallocate at 0xd0000\n");
		vm_regions(dbg_fd);
	}
	MACH_CALL((vm_allocate(mach_task_self(), &addr, (vm_size_t)0x10000, 
		FALSE)), "vm_allocate of ems address space.");
	if (us_debug_level > 0) {
		fprintf(dbg_fd,"vm_allocate of ems address space.\n");
		vm_regions();
	}

	/*
	 * Map the wrap around area and allocate upper memory for our use.
	 */

	A20_init();

#ifdef	USE_FILES
	
	if (!startup_flag && !bsd) {
		char buffer[256];
		/*
		 * Read in the interrupt vectors.
		 */
#ifdef	OSF_SVR
		strcpy((char *)osfdir+prefix_len, "mem.0000");
		if ((fd = open(osfdir, O_RDONLY)) < 0) {
			fprintf(dbg_fd,"\rmon: Couldn't open int vector file.\n\r");
			exit(1);
		}else{
			Debug0((dbg_fd,"mon: mem.0000 found in %s\n",osfdir));
		}

#else	/* BSD 4.3 UX Server */

		if ((fd = openp(lpath, "mem.0000",buffer,O_RDONLY,0)) < 0) {
			fprintf(dbg_fd,"\rmon: Couldn't open int vector file.\n\r");
			exit(1);
		}else{
			Debug0((dbg_fd,"mon: mem.0000 found in %s\n",buffer));
		}
#endif	OSF_SVR

		addr = 0x0;
		if (read (fd, addr, 0x0400) != 0x400) {
			fprintf(dbg_fd,	"\rmon: Couldn't read in the int vectors.\n\r");
			exit(1);
		}
		if (close (fd) < 0) {
			fprintf(dbg_fd,"\rmon: int vector close error, exiting.\n\r");
			exit(1);
		}
		
		/*
		 * Read in initial bios ram scratchpad.
		 */
#ifdef	OSF_SVR
		strcpy((char *)osfdir+prefix_len, "mem.0400");
		if ((fd = open(osfdir, O_RDONLY)) < 0) {
			fprintf(dbg_fd,"\rmon: Couldn't open bios mem file.\n\r");
			exit(1);
		}else{
			Debug0((dbg_fd,"mon: mem.0400 found in %s\n",osfdir));
		}

#else	/* BSD 4.3 UX Server */		

		if ((fd = openp(lpath,"mem.0400",buffer,O_RDONLY,0)) < 0) {
			fprintf(dbg_fd,"\rmon: Couldn't open bios mem file.\n\r");
			exit(1);
		}else{
			Debug0((dbg_fd,"mon: mem.0400 found in %s\n",buffer));
		}
#endif	OSF_SVR

		addr = (char *)0x0400;
		if (read (fd, addr, 0x0100) != 0x0100) {
			fprintf(dbg_fd,	"\rmon: Couldn't read in bios mem file.\n\r");
			exit(1);
		}
		
		if (close (fd) < 0) {
			fprintf(dbg_fd,"\rmon: int vector close error, exiting.\n\r");
			exit(1);
		}
	}else
#endif	USE_FILES
		{
			u_char * ptr;
			u_short * sptr;
			int i;
			
			/* fix for Dos 3.3 boot floppy motor on */
			ptr = (u_char *)0x43f;
			*ptr = 0x11;
			
			/* put keybuf pointer into spram */
			sptr = (u_short *)0x480;
			*sptr++ = 0x1e;
			*sptr = 0x3e;
			sptr = (u_short *)0x41a;
			*sptr++ = 0x1e;
			*sptr = 0x1e;

			/* test for diskette fix */
			sptr = (u_short *)0x410;
			*sptr = (*sptr) & 0xff3f;
			for(i=0; i< 2; i++) {
				if (devices[i].d_state != DEVICE_NOT_PRESENT)
					number_of_diskettes++;
			}
			*sptr = (*sptr) | (((--number_of_diskettes)&0x3)<<6);
		}
}

void load_boot_file ( filename , load_address )
	char *filename;
	vm_address_t load_address;
{
	int fd;
	int i;
	char buffer[256];

	if (!floppy_boot) {
		if ((fd = open("/dev/dosdisk", O_RDONLY)) < 0){
			fprintf(dbg_fd,"\rcouldn't open '/dev/dosdisk'\n");
			fprintf(dbg_fd,"\rtrying '%s' file'\n",filename);
			
#ifdef	OSF_SVR
			strcpy((char *)osfdir+prefix_len, filename);
			if ((fd = open(osfdir, O_RDONLY)) < 0) {
				fprintf(dbg_fd,"\rcouldn't open '%s'\n", filename);
				exit(1);
			}else{
				Debug0((dbg_fd,"found %s in %s\n",filename,osfdir));
			}

#else	/* BSD 4.3 UX Server */

			if ((fd = openp (lpath,filename,buffer, O_RDONLY, 0)) < 0) {
				fprintf(dbg_fd,"\rcouldn't open '%s'\n", filename);
				exit(1);
			}else{
				Debug0((dbg_fd,"found %s in %s\n",filename,buffer));
			}
#endif	OSF_SVR
		}
	}else{
#ifdef	OSF_SVR
		strcpy((char *)osfdir+prefix_len, filename);
		if ((fd = open(osfdir, O_RDONLY, 0)) < 0) {
			fprintf(dbg_fd,"\rcouldn't open '%s'\n", filename);
			fprintf(dbg_fd,"\rtrying /dev/dosdisk\n");
			
			if ((fd = open("/dev/dosdisk", O_RDONLY,0)) < 0){
				fprintf(dbg_fd,"\rcouldn't open '/dev/dosdisk'\n");
				exit(1);
			}
		}else{
			Debug0((dbg_fd,"found %s in %s\n",filename, osfdir));
		}

#else	/* BSD 4.3 UX Server */

		if ((fd = openp (lpath,filename,buffer, O_RDONLY, 0)) < 0) {
			fprintf(dbg_fd,"\rcouldn't open '%s'\n", filename);
			fprintf(dbg_fd,"\rtrying /dev/dosdisk\n");
			
			if ((fd = open("/dev/dosdisk", O_RDONLY,0)) < 0){
				fprintf(dbg_fd,"\rcouldn't open '/dev/dosdisk'\n");
				exit(1);
			}
		}else{
			Debug0((dbg_fd,"found %s in %s\n",filename, buffer));
		}
#endif	OSF_SVR
	}
	
	Debug2((dbg_fd, "\rmon: loading boot block at 0x%x\n",load_address));
	if (read(fd, load_address, DOS_BOOT_BLOCK_SIZE) != 
	    DOS_BOOT_BLOCK_SIZE) {
		fprintf(dbg_fd,"\rmon: disk boot load error, exiting.");
		exit(1);
	}
	
	if (close (fd) < 0) {
		fprintf(dbg_fd,"\rmon: disk boot close error, exiting.");
		exit(1);
	}
	
	Debug2((dbg_fd,"\rmon: done loading boot block at 0x%x.\n\n\n",
		load_address));
}

main(argc, argv)
	int argc;
	char **argv;
{
	char * out_file;
	char * my_tty;

	thread_t keyboard_thread;
	thread_t timer_thread;
	thread_t mouse_thread;
	thread_t com2_thread;
	thread_t exit_thread;
	mach_port_t except_port;
	vm_address_t load_address;
	vm_address_t entry_point;

	char * file_name;
	char * Ifile_name = NULL;

	/* Make sure that the user is on the console. */
	my_tty = ttyname(0);

	if (!my_tty || strcmp(my_tty, "/dev/console") != 0) {
		printf("You can only start this program from the console.\n");
		printf("Exiting mdos.\n");
		exit(-1);
	}

#ifdef	OSF_SVR

	dosprefix = getenv("DOSPREFIX");

	if (dosprefix == NULL) {
		dosprefix = "./";
		strcpy(osfdir, dosprefix);
		prefix_len = strlen(dosprefix);
	}else{
		strcpy(osfdir, dosprefix);
		prefix_len = strlen(dosprefix);
		osfdir[prefix_len] = '/';
		prefix_len++;
		osfdir[prefix_len] = EOS;
	}

	printf("osfdir = '%s'\n",osfdir);

#else	/* BSD 4.3 UX Server */

	lpath = getenv("LPATH");

#endif	OSF_SVR

	dos_root = getenv("DOSROOT");
	if (dos_root == NULL) {
		printf("DOSROOT not set, using HOME.\n");
		dos_root = getenv("HOME");
	}
	printf("Using '%s' as the ufs root directory.\n", dos_root);
	dos_root_len = strlen(dos_root);
	if (dos_root[dos_root_len - 1] != SLASH) {
		dos_root[dos_root_len++] = SLASH;
		dos_root[dos_root_len] = EOS;
	}
	     
	debug_state = OFF;
	dbg_fd = stderr;

	argc--, argv++;

	/* init this before setting up device defaults and -d settings */
	bios_init();

	/* default values: */
	vga_state = VGA_DISABLED;
	debug_state = OFF;
	out_file = "/dev/console";
	load_address = BOOT_LOAD_ADDRESS;
	entry_point = load_address;
	us_debug_level = 0;
	video_debug_level = 0;
	key_debug_level = 0;
	disk_debug_level = 0;
	floppy_boot = FALSE;

	devices[0].d_path = "ms.dos";
	devices[0].d_state = DEVICE_NOT_INITIALIZED;
	devices[1].d_path = "/dev/rfloppy";
	devices[1].d_state = DEVICE_NOT_PRESENT;
	devices[0x80].d_path = "/dev/dosdisk";
	devices[0x80].d_state = DEVICE_NOT_INITIALIZED;

	file_name = devices[0].d_path;

    argloop:
	if (argc < 1) 
		goto jmploop;

	if (*argv[0] == '-') {
		char c;
		char *string;
		c = *(*argv + sizeof(char));
		string = argv[0]+2;
		switch (c) {
		    case 's':
			startup_flag = TRUE;
			vga_state = VGA_OFF;
			fprintf(dbg_fd,"\rmon: Startup environment enabled.\n");
			fprintf(dbg_fd,"\rmon: & VGA usage disabled.\n");
			argc--,argv++;
			break;
		    case 'f':
			floppy_boot = TRUE;
			argc--,argv++;
			break;
		    case 'v':
			vga_state = VGA_OFF;
			fprintf(dbg_fd,"\rmon: VGA disabled\n");
			argc--,argv++;
			break;
		    case 'X':
			debug_state = ON;
			if ((dbg_fd = fopen ("mon.out", "w+")) 
			    == NULL) {
				fprintf(stderr,"\rCouldn't open dbg_fd\n");
				exit(1);
			}
			fprintf(dbg_fd,"\rmon: debugging into /tmp/mon.out\n");
			argc--,argv++;
			break;
		    case 'I':
			Ifile_name = string;
			Fprintf((dbg_fd,"mon: load file = '%s'\n",file_name));
			argc--,argv++;
			break;
		    case 'd': {
			    char dev_name;
			    int device;

			    sscanf(string, "%c", &dev_name);
			    argc--,argv++;
			    switch(dev_name) {
				case 'a':
				case 'A':
				    file_name = *argv;
				    Debug2((dbg_fd,"boot file info set to '%s'\n",
					    file_name));
				    device = 0;
				    break;
				case 'b':
				case 'B':
				    device = 1;
				    equipment_code = (equipment_code & ~0xc0) | 0x40;
				    break;
				case 'c':
				case 'C':
				    device = 0x80;
				    break;
				case 'd':
				case 'D':
				    device = 0x81;
				    break;
				defaut:
				    device = -1;
				    break;
			    }
			    if (device != -1) {
				    devices[device].d_state = DEVICE_NOT_INITIALIZED;
				    devices[device].d_path = *argv;
				    Fprintf((dbg_fd,"\rmon: device(0x%x) = %s\n",device, devices[device].d_path));
			    }
			    argc--,argv++;
			    break;
		    }
		    case 'O':
			out_file = string;
			fprintf(dbg_fd,"mon: output file = '%s'\n",out_file);
			argc--,argv++;
			break;
		    case 'U':
			sscanf(string, "%x", &us_debug_level);
			fprintf(dbg_fd,"mon: debug level = 0x%x\n",us_debug_level);
			argc--,argv++;
			break;
		    case 'V':
			sscanf(string, "%x", &video_debug_level);
			fprintf(dbg_fd,"\rmon: video debug level = 0x%x\n",
				video_debug_level);
			argc--,argv++;
			break;
		    case 'K':
			sscanf(string, "%x", &key_debug_level);
			fprintf(dbg_fd,"\rmon: keyboard debug level = 0x%x\n",
				key_debug_level);
			argc--,argv++;
			break;
		    case 'D':
			sscanf(string, "%x", &disk_debug_level);
			fprintf(dbg_fd,"\rmon: disk debug level = 0x%x\n",
				disk_debug_level);
			argc--,argv++;
			break;
		}
	} else {
		argc--, argv++;
	}
	goto argloop;
    jmploop:

	lowest_stack_pointer = TOP_OF_STACK_AREA;

	set_up_task_vm();
	
	if (vga_state)
		bios_vga_init();

	load_boot_file((Ifile_name != NULL ? Ifile_name : file_name), load_address);

	if (us_debug_level > Debug_Level_2) {
		reset_tty();
		printf("Done loading 5.0 boot code.\n");
		while(1) {
			sleep(10);
		}
	}

	set_up_v86thread(&v86_thread, &except_port, entry_point);

	create_a_thread(&keyboard_thread, keyboard_loop);
	create_a_thread(&timer_thread, timer_loop);
	create_a_thread(&mouse_thread, mouse_loop);
#ifdef	USE_COM_2
	create_a_thread(&com2_thread, com2_loop);
#endif	USE_COM_2
	create_a_thread(&exit_thread, exit_loop);

	/* Number of fixed drives in Bios scratch pad RAM */
	/* probably don't need this! XXX */

	if (devices[0x81].d_state == DEVICE_NOT_INITIALIZED) {
		u_char * ptr;
		ptr = (u_char *)0x475;
		*ptr = 2;
	}

	enable_inout_ports();

#ifdef	IDLE_WORK_IN_PROGRESS
	condition_init(&idle_port);
#endif	/* IDLE_WORK_IN_PROGRESS */

	thread_resume(timer_thread);
	thread_resume(mouse_thread);
	thread_resume(keyboard_thread);
#ifdef USE_COM_2
	thread_resume(com2_thread);
#endif USE_COM_2
	/* thread_priority(mouse_thread, 31, FALSE); */
/*	thread_resume(exit_thread); */

	bios_video_init();

	thread_resume(v86_thread);

	process_exceptions(except_port);
}
