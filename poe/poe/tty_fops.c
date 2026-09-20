/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	tty_fops.c,v $
 * Revision 2.6  92/02/02  13:02:57  rpd
 * 	Removed old IPC vestiges.
 * 	Fixed cthread_fork argument types.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.5  91/12/19  20:29:43  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  91/06/18  13:07:07  jjc
 * 	Picked up line discipline support from Adam Richter at Berkeley:
 * 	Added support for the flow control and line editting control
 * 	characters (^D, ^O, ^S, ^Q, ^R, ^V, backspace, ^U, ^W), and the
 * 	ioctls that stty uses to customize them.
 * 	[91/05/30            jjc]
 * 
 * Revision 2.3  90/09/27  13:55:46  rwd
 * 	Fix casting problem.
 * 	[90/09/10            rwd]
 * 
 * Revision 2.2  90/09/08  00:21:12  rwd
 * 	Replace all tv_sleeps with reasonable condtion_* logic.
 * 	[90/07/26            rwd]
 * 
 * 	Lets use device_read_inband and device_write.
 * 	[90/07/20            rwd]
 * 
 * 	Fix things sun was unhappier about than mips (was that possible?)
 * 	For now, wire cthreads.
 * 	[90/07/16            rwd]
 * 
 */
/*
 *	File:	./tty_fops.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <config.h>
#include <mach.h>
#include <mach/message.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ux_user.h>
#include <fnode.h>
#include <sys/ioctl.h>
#include <bsd_ioctl.h>
#include <subr_fops.h>
#include <device/device_types.h>

#define	TTY_RAWBSIZE	256
#define	TTY_OUTBSIZE	4096

extern int tty_open();
extern int tty_close();
extern int tty_read();
extern int tty_write();
extern int tty_getstat();
extern int tty_setstat();
extern int tty_select();
extern int tty_ioctl();
extern int nil_getpager();
extern int nil_lookup();
extern int nil_create();
extern int nil_link();
extern int nil_unlink();

static mach_port_t tty_port = MACH_PORT_NULL;
extern mach_port_t device_server_port;

extern struct cbuf *cbuf_alloc();
extern void cbuf_expunge();

extern void cbuf_get_last_char(/* struct cbuf *cb */);
extern int tty_driver();
extern int tty_output_thread();

struct fops tty_fops = {
	tty_open,
	tty_close,
	tty_read,
	tty_write,
	tty_getstat,
	tty_setstat,
	tty_select,
	tty_ioctl,
	nil_getpager,
	nil_lookup,
	nil_create,
	nil_link,
	nil_unlink,
};
	
struct fnfs tty_fs = {
	0,		/* fs_mountpoint */
	0,		/* fs_root */
	&tty_fops,	/* fs_fops */
	FALSE,		/* fs_mayseek */
	FALSE,		/* fs_maymap */
};

#define LOG2_BITS_PER_BYTE	3
#define BITS_PER_BYTE		(1<<LOG2_BITS_PER_BYTE)
#define BITS_PER_BYTE_MASK	(BITS_PER_BYTE - 1)

struct ttstate {
	struct sgttyb		tt_sgttyb;
	struct winsize		tt_winsize;
	struct tchars		tt_tchars;
	struct ltchars		tt_ltchars;
	int			tt_pgrp;
	int			tt_lmodes;
	int			tt_ldisc;
	struct mutex *		tt_lock;
	struct condition *	tt_ricondition;	/* can read  input  */
	struct condition *	tt_wicondition;	/* can write input  */
	struct condition *	tt_rocondition;	/* can read  output */
	struct condition *	tt_wocondition;	/* can write output */
	int			tt_opens;
	char *			tt_name;
	device_t		tt_port;
	boolean_t		freeze_output;
	boolean_t		discard_output;
	boolean_t		next_char_literal;

	unsigned char		tt_literal_flags[TTY_RAWBSIZE / BITS_PER_BYTE];
	char			tt_inbuf[TTY_RAWBSIZE];
	int			last_readable, last_editable, last_free;

	/* There are three regions of the circular buffer tt_inbuf[]:
	   the readable region, the editable region, and the free region,
	   in that order.

	   The readable region contains the characters which are available
	   right now for a read() system call.  In "cooked" mode, these are
	   all of the characters up to and including the last carriage
	   return that the user types.  In raw and cbreak modes, these are
	   all of the characters that the user has typed.

	   The editable region is the line that the user is currently
	   typing in in "cooked" mode.  These characters will not be
	   returned to read() system call until the user hit the <return>
	   key.  When the user hits <return>, the contents of editable
	   region is appended to the readable region, and the editable
	   region itself becomes empty.  In raw and cbreak modes, the
	   editable region is always empty.

	   The free region contains bytes available to hold user input.
	   There must always be at least one byte in the free region in
	   order to distinguish the buffer empty case from the buffer
	   full case.  It simplifies the code to waste that one byte.

	   It also happens to simplify the code to keep track of the
	   boundaries betwen the regions with indicies to the last
	   characters of each region, rather the indicies to the
	   first character, which might seem more intuitive.

	   tt_literal_flags[] is an array of bits, one for each byte in
	   tt_inbuf[], indicating whether or not the contents
	   corresponding character in tt_inbuf[] was quoted with ^V in
	   cooked mode.  Characters quoted with ^V are passed without
	   modification to the read() system call, and any special meaning
	   that they might normally have is ignored.  tt_literal_flags[]
	   is used to decide whether to generate an end-of-file condition
	   for ^D and whether to suspend the program for ^Y.  In raw
	   mode, tt_literal_flags[] are ignored.
	   */
	   
	struct mutex *		tt_output_lock;	/* Used for ^S and ^Q */
	struct cbuf *		tt_outcb;
} tty_default;

#define SET_LITERAL_FLAG(tt,offset)	{ 			\
   (tt)->tt_literal_flags[(offset) >> BITS_PER_BYTE] |=		\
      1<<(offset & BITS_PER_BYTE_MASK);				\
}
#define CLEAR_LITERAL_FLAG(tt,offset)	{ 			\
   (tt)->tt_literal_flags[(offset) >> BITS_PER_BYTE] &=		\
      ~(1<<(offset & BITS_PER_BYTE_MASK));			\
}
#define TEST_LITERAL_FLAG(tt,offset)				\
   ((tt)->tt_literal_flags[(offset) >> BITS_PER_BYTE] &		\
    (1<<(offset & BITS_PER_BYTE_MASK)))				\

#define INCR_INPUT_INDEX(index)	{if(++(index) == TTY_RAWBSIZE) (index) = 0; }
#define DECR_INPUT_INDEX(index) {if((index)-- == 0) (index) = TTY_RAWBSIZE-1;}
#define CIRCULAR_BUFFER_DISTANCE(end,start,buflen)	\
   (((end)+(buflen)-(start))%(buflen))

#define INPUT_BUFFER_DISTANCE(end, start)	\
	CIRCULAR_BUFFER_DISTANCE((end),(start),TTY_RAWBSIZE)

/* There must be one empty position in the buffer to prevent the buffer
   empty case from being confused with the buffer full case. */
#define NUM_FREE_BYTES(tt)	\
   (((tt)->last_free == (tt)->last_editable) ? (TTY_RAWBSIZE - 1) :	\
    (INPUT_BUFFER_DISTANCE((tt)->last_free, (tt)->last_editable)-1))

#define assert(cond) { if( ! (cond )) panic("assertion failed"); }
boolean_t tty_no_read = FALSE;
struct mutex tty_no_read_mutex = MUTEX_INITIALIZER;
struct condition tty_no_read_condition = CONDITION_INITIALIZER;

tty_special(fnp, fmt, dev, name)
	struct fnode **fnp;
	int fmt;
	dev_t dev;
	char *name;
{
	struct snode *sn;
	struct ttstate *tt;
	int stl;
	int error;

	error = spec_make_special(fnp, fmt, dev, &tty_fs);
	if (error) {
		return error;
	}
	tt = (struct ttstate *)malloc(sizeof(struct ttstate));
	tty_init(tt);
	((struct snode *) *fnp)->sn_private = (char *)tt; 
	if (name)
	    stl = strlen(name);
	else
	    stl = 0;
	tt->tt_name = (char *)malloc(stl+1);
	if (name) {
	    bcopy(name, tt->tt_name, stl+1);
	    if (tt->tt_name[stl - 1] == '#')
		tt->tt_name[stl - 1] = 'a' + minor(dev);
	}
	tt->tt_name[stl] = '\0';
	
	return 0;
}

tty_init(tt)
	struct ttstate *tt;
{

	bzero(tt, sizeof(*tt));
	tt->tt_tchars.t_intrc	= 'C' - '@';
	tt->tt_tchars.t_quitc	= '\\' - '@';
	tt->tt_tchars.t_startc	= 'Q' - '@';
	tt->tt_tchars.t_stopc	= 'S' - '@';
	tt->tt_tchars.t_eofc	= 'D' - '@';
	tt->tt_tchars.t_brkc	= '\n';

	tt->tt_ltchars.t_suspc	= 'Z' - '@';
	tt->tt_ltchars.t_dsuspc	= 'Y' - '@';
	tt->tt_ltchars.t_rprntc	= 'R' - '@';
	tt->tt_ltchars.t_flushc	= 'O' - '@';
	tt->tt_ltchars.t_werasc	= 'W' - '@';
	tt->tt_ltchars.t_lnextc	= 'V' - '@';

	tt->tt_lock		= mutex_alloc();
	tt->tt_output_lock	= mutex_alloc();
	tt->tt_ricondition	= condition_alloc();
	tt->tt_rocondition	= condition_alloc();
	tt->tt_wicondition	= condition_alloc();
	tt->tt_wocondition	= condition_alloc();
	tt->last_readable = tt->last_editable = tt->last_free = TTY_RAWBSIZE-1;
	tt->tt_outcb		= cbuf_alloc(TTY_OUTBSIZE);
	tt->tt_sgttyb.sg_erase	= 'H' - '@';
	tt->tt_sgttyb.sg_kill	= 'U' - '@';
	tt->tt_sgttyb.sg_flags	= ECHO | CRMOD;
	tt->tt_opens		= 0;
	tt->tt_pgrp		= 1;	/* XXXXXX */
	tt->tt_port = MACH_PORT_NULL;
	mutex_init(tt->tt_lock);
	mutex_init(tt->tt_output_lock);
	condition_init(tt->tt_wicondition);
	condition_init(tt->tt_wocondition);
	condition_init(tt->tt_ricondition);
	condition_init(tt->tt_rocondition);
	cthread_detach(cthread_fork((any_t (*)()) tty_driver,
				    (any_t) tt));
	cthread_detach(cthread_fork((any_t (*)()) tty_output_thread,
				    (any_t) tt));
	return 0;
}

tty_open(sn)
	struct snode *sn;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;
	int error;

	if (tt->tt_port != MACH_PORT_NULL)
	    return EINVAL;

#if	STANDALONE
#if 0
	if (!strcmp("console",tt->tt_name)) {
	    tt->tt_port = (device_t)mach_console_port();
	    if (tt->tt_port == MACH_PORT_NULL) {
		printf("tty_open mach_console_port = %d\n", tt->tt_port);
		panic("tty_open:console");
	    }
	} else {
#endif
	    error = device_open(device_server_port,
			    0, tt->tt_name, &tt->tt_port);
	    if (error) {
		printf("tty_open (%s) error = %d\n", tt->tt_name, error);
		panic("tty_open");
	    }
#if 0
	}
#endif
#else	STANDALONE
	tt->tt_port = open("/dev/tty", 2, 0);
	{
	    struct sgttyb sg;
	    ioctl(tt->tt_port, TIOCGETP, &sg);
	    sg.sg_flags |= RAW;
	    sg.sg_flags &= ~ECHO;
	    ioctl(tt->tt_port, TIOCSETP, &sg);
	}
#endif	STANDALONE
	mutex_lock(tt->tt_lock);
	condition_broadcast(tt->tt_wicondition);
	mutex_unlock(tt->tt_lock);
	return 0;
}

char tty_getc(tt)
	struct ttstate *tt;
{
	int rv, ad;
	char c;

	mutex_lock(tt->tt_lock);
	while(tt->tt_port == MACH_PORT_NULL) {
		condition_wait(tt->tt_wicondition, tt->tt_lock);
	}
	mutex_unlock(tt->tt_lock);

#if	STANDALONE
	mutex_lock(&tty_no_read_mutex);
	while (tty_no_read)
		condition_wait(&tty_no_read_condition, &tty_no_read_mutex);
	mutex_unlock(&tty_no_read_mutex);

	ad = 1;
	rv = device_read_inband(tt->tt_port, 0, 0, 1, &c, &ad);
	if (rv) {
		printf("tty_getc (%s) error = %d\n", tt->tt_name, rv);
		panic("tty_getc");
	}
#else	STANDALONE
	read(tt->tt_port, &c, 1);
#endif	STANDALONE
	return c;
}

tty_output_thread(tt)
	struct ttstate *tt;
{
	char buf[TTY_OUTBSIZE];
	int count;
	int rv, actual, sofar;

	mutex_lock(tt->tt_lock);
	for (;;) {
		count = cbuf_read(tt->tt_outcb, buf, sizeof(buf));
		if (count) {
		    if (count == TTY_OUTBSIZE)
			condition_broadcast(tt->tt_wocondition);
		    if( tt->discard_output ) continue;
		    mutex_unlock(tt->tt_lock);
		    mutex_lock(tt->tt_output_lock);
		    if( !tt->discard_output ) {
#if	STANDALONE
		       sofar = 0;
		       while(sofar<count) {
			  rv = device_write(tt->tt_port, 0, 0, &buf[sofar],
					    count-sofar, &actual);
			  if (rv != 0) {
			     printf("tty_output_thread (%s) rv= %d\n",
				    tt->tt_name, rv);
			     panic("tty_output_thread");
			  }
			  sofar+=actual;
		       }
#else	STANDALONE
		       write(tt->tt_port, buf, count);
#endif	STANDALONE
		    }	/* if !tt->discard_output  */

		    mutex_unlock(tt->tt_output_lock);
		    mutex_lock(tt->tt_lock);
		} else {
		   condition_wait(tt->tt_rocondition, tt->tt_lock);
		}
	}
}

void
tty_output_string(tt, str, len)
	struct ttstate *tt;
	char *str;
        int len;
{
   int count;
   mutex_lock(tt->tt_lock);

   for(;;) {
      if( count = cbuf_write( tt->tt_outcb, str, len )) {
	 condition_broadcast(tt->tt_rocondition);
      }
      str += count;
      if( len -= count ) {
	 condition_wait(tt->tt_wocondition, tt->tt_lock);
      }
      else
	 break;
   } 

   mutex_unlock(tt->tt_lock);
}

tty_output(tt, c)
	struct ttstate *tt;
	char c;
{
   tty_output_string(tt, &c, 1 );
}

tty_echo(tt, c)
	struct ttstate *tt;
	char c;
{
	c &= 0x7f;		/* XXX? Why not echo 8-bit characters? */

	if (!(tt->tt_sgttyb.sg_flags & RAW)) {
	    if (c == '\r' || c == '\n') {
	       tty_output( tt, '\r' );
	       tty_output( tt, '\n' );
	    } else if (c < ' ') {
	       tty_output( tt, '^' );
	       tty_output( tt, c + '@' );
	    } else if (c == 127) {
	       tty_output( tt, '^' );
	       tty_output( tt, '?' );
	    } else {
	       tty_output( tt, c );
	    }
	} else {
	   tty_output( tt, c );
	}
}

static void
expunge_terminal_buffers( tt )
     struct ttstate *tt;
{
   mutex_lock( tt->tt_lock );
   tt->last_free = tt->last_readable = tt->last_editable = TTY_RAWBSIZE - 1;
   cbuf_expunge( tt->tt_outcb );
   mutex_unlock( tt->tt_lock );
}

static void
unblock_terminal( tt )
     struct ttstate *tt;
{
   tt->discard_output = FALSE;
   if( tt->freeze_output ) {
      tt->freeze_output = FALSE;
      mutex_unlock( tt->tt_output_lock );
   }
}
   

tty_driver(tt)
	struct ttstate *tt;
{
	char c, c_trans;
	int loc;
	int count;

	cthread_set_kernel_limit(cthread_kernel_limit() + 1);
	cthread_wire();
	for (;;) {
	    c = tty_getc(tt);
	    c_trans = c == '\r' ? '\n' : c; /* Translate carriage return to
					       line feed for comparisons. */

	    /* If the buffer is full, beep to indicate that input is
	       being ignored.  We force one extra space in the buffer
	       to be reserved for the carriage return in cooked mode.
	       */
	    mutex_lock( tt->tt_lock );
	    if( NUM_FREE_BYTES( tt ) <= 1 &&
	       ( NUM_FREE_BYTES( tt ) == 0 || c_trans != '\n' ||
		(tt->tt_sgttyb.sg_flags & (RAW | CBREAK)))) {
	       mutex_unlock(tt->tt_lock);
	       tty_output( tt, '\a' );
	       continue;
	    }
	    mutex_unlock( tt->tt_lock );

	    if (!(tt->tt_sgttyb.sg_flags & RAW) && ! (tt->next_char_literal)) {
		if (c_trans == tt->tt_tchars.t_intrc) {
			expunge_terminal_buffers( tt );
			unblock_terminal( tt );
			bsd_master(0);
			bsd_killpg(tt->tt_pgrp, SIGINT);
			bsd_release();
			continue;
		}
		else if (c_trans == tt->tt_ltchars.t_suspc) {
			expunge_terminal_buffers( tt );
			unblock_terminal( tt );
			bsd_master(0);
			bsd_killpg(tt->tt_pgrp, SIGTSTP);
			bsd_release();
			continue;
		}
		else if (c_trans == tt->tt_tchars.t_stopc) {
		   if( !tt->freeze_output && !tt->discard_output ) {
				/* Stop output */
		      mutex_lock( tt->tt_output_lock );
		      tt->freeze_output = TRUE;
		   }
		   continue;
		}
		else if (c_trans == tt->tt_tchars.t_startc) {
				/* Resume output */

		   tt->discard_output = FALSE; /* Also undo ^O, while
						  we're at it. */

		   if( tt->freeze_output ) {
		      tt->freeze_output = FALSE;
		      mutex_unlock( tt->tt_output_lock );
		   }
		   continue;
		}
		else if (c_trans == tt->tt_ltchars.t_lnextc) {
		   tt->next_char_literal = TRUE;
		   continue;
		}
		else if (c_trans == tt->tt_ltchars.t_flushc) {
		   /* Toggle discarding output */
		   if ( tt->discard_output = !tt->discard_output ) {

		      /* Expunge any output already waiting in the buffer. */
		      cbuf_expunge( tt->tt_outcb );
		      if( tt->freeze_output ) {
			 /* Don't freeze output if we're just going to
			    ignore it anyhow. */
			 tt->freeze_output = FALSE;
			 mutex_unlock( tt->tt_output_lock );
		      }
		   }
		   continue;
		}
		else if (!(tt->tt_sgttyb.sg_flags & CBREAK)) {
		   /* Comand line editting */
		   if( c_trans == tt->tt_sgttyb.sg_erase ) { /* Backspace */
		      if( tt->last_editable != tt->last_readable ) {
			 DECR_INPUT_INDEX( tt->last_editable );
			 tty_output_string( tt, "\b \b", 3 );
		      }
		      continue;
		   }
		   else if( c_trans == tt->tt_sgttyb.sg_kill ) {
		      while( tt->last_editable != tt->last_readable ) {
			 DECR_INPUT_INDEX( tt->last_editable );
			 tty_output_string( tt, "\b \b", 3);
		      }
		      continue;
		   }
		   else if( c_trans == tt->tt_ltchars.t_werasc ) {
				/* Word erase */
		      /* First, remove trailing spaces. */
		      while( tt->last_editable != tt->last_readable &&
			    (tt->tt_inbuf[ tt->last_editable ] == ' ' ) ||
			    (tt->tt_inbuf[ tt->last_editable ] == '\t' )) {
			 DECR_INPUT_INDEX( tt->last_editable );
			 tty_output_string( tt, "\b \b", 3);
		      }

		      /* Then, remove the last word. */
		      while( tt->last_editable != tt->last_readable &&
			    tt->tt_inbuf[ tt->last_editable ] != ' ' &&
			    tt->tt_inbuf[ tt->last_editable ] != '\t' ) {
			 DECR_INPUT_INDEX( tt->last_editable );
			 tty_output_string( tt, "\b \b", 3);
		      }
		      continue;
		   }		/* End of word erase */
		   else if ( c_trans == tt->tt_ltchars.t_rprntc ) {
		      int i;

		      tty_output_string( tt, "^R\r\n" , 4 );
		      i = tt->last_readable;
		      while( i != tt->last_editable ) {
			 INCR_INPUT_INDEX( i );
			 tty_output( tt, tt->tt_inbuf[ i ] );
		      }
		      continue;
		   }
		}		/* End of command line editting */
	     }			/* End of checking for special characters */

	    /* We are now dealing with a normal character and there is
	       space in the buffer to process it. */

	    if (c == '^'-'@') {
		panic("double-hat shutdown");
	    }
	    if (tt->tt_sgttyb.sg_flags & ECHO) {
		tty_echo(tt, c);
		/* We know that there is still space in the buffer even
		   though tty_echo may have blocked because tty_driver
		   is the only thread that adds characters to the buffer. */
	    }

	    INCR_INPUT_INDEX( tt->last_editable );
	    tt->tt_inbuf[ tt->last_editable ] = c;
	    mutex_lock( tt->tt_lock );
	    if( tt->next_char_literal ) {
	       SET_LITERAL_FLAG( tt, tt->last_editable );
	    }
	    else {
	       CLEAR_LITERAL_FLAG( tt, tt->last_editable );
	    }
	    /* On newline, make the line readable. */
	    if((tt->tt_sgttyb.sg_flags & (CBREAK|RAW)) ||
	       (!tt->next_char_literal && 
		(c_trans == tt->tt_tchars.t_eofc || c_trans == '\n' ||
		 c_trans == tt->tt_tchars.t_brkc ))) {

	       tt->last_readable = tt->last_editable;
	       condition_broadcast( tt->tt_ricondition );
	    }
	    mutex_unlock( tt->tt_lock );
	    tt->next_char_literal = FALSE;
	 }
}

tty_read(sn, offset, buffer, size, resid)
	struct snode *sn;
	vm_offset_t offset;
	vm_offset_t buffer;
	vm_size_t size;
	vm_size_t *resid;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;
	int error, count;
	struct ux_task *ut, *ut_self();
	char *buf_p = (char*) buffer;
	boolean_t suspended;

	ut = ut_self();

	/*
	 * Check for proper process group
	 */
	if (ut->ut_pgrp != tt->tt_pgrp) {
		dprintf("{tty_read:ttin}");
		bsd_killpg(ut->ut_pgrp, SIGTTIN);
		return EINTR;	/* XXX ??? */
	}

	/*
	 * Sanity
	 */
	if (size == 0) {
		*resid = 0;
		return 0;
	}

	/* Wait until there is at least one chacter.
	   XXX What about non-blocking IO? */
	
	mutex_lock(tt->tt_lock);
	while (tt->last_free == tt->last_readable) {
		bsd_release();
		condition_wait(tt->tt_ricondition, tt->tt_lock);
		mutex_unlock(tt->tt_lock);
		bsd_master(ut);
		mutex_lock(tt->tt_lock);
	}

	count = 0;
	suspended = FALSE;
	/* Copy the characters, translating carriage returns to line feeds. */
	do {
	   char c;
	   INCR_INPUT_INDEX( tt->last_free );
	   c = tt->tt_inbuf[tt->last_free];
	   if( !TEST_LITERAL_FLAG( tt, tt->last_free ) &&
	      !(tt->tt_sgttyb.sg_flags & RAW)) {
	      if( c == tt->tt_ltchars.t_dsuspc ) {
		 struct ux_task *ut_self();
		 bsd_killpg(tt->tt_pgrp, SIGTSTP);
		 suspended = TRUE;
		 break;
	      }
	      else if( c == tt->tt_tchars.t_eofc ) {
		 /* End-of-file character.  If there are any other
		    characters preceding the EOF character, return them
		    and process the end of file mark on the next read. */
		 if( count ) {
		    DECR_INPUT_INDEX( tt->last_free );
		 }
		 break;
	      }
	      else if( c == '\n' || c == '\r' ) {
		 *(buf_p++) = '\n';
		 count++;
		 break;
	      }
	      /* If it's not a special character, or it is a literal
		 character, or we are in raw mode, then just add it to the
		 input buffer (fall through). */
	   }
	   *(buf_p++) = c;
	   count++;
	} while( count != size && tt->last_free != tt->last_readable );
	condition_broadcast(tt->tt_wicondition);
	mutex_unlock(tt->tt_lock);
	*resid = size - count;

	/* If we're being suspended with no data to return, then return
	   EINTR rather than "success, 0 bytes read", which would imply
	   end of file. */
	return (suspended && count == 0 ) ? EINTR : 0;
}

tty_write(sn, offset, buffer, size, resid)
	struct snode *sn;
	vm_offset_t offset;
	char *buffer;
	vm_size_t size;
	vm_size_t *resid;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;
	int i;

	for (i = 0; i < size; i++) {
		if ((tt->tt_sgttyb.sg_flags & CRMOD) && buffer[i] == '\n') {
		   tty_output(tt, '\r');
		}
		tty_output(tt, buffer[i]);
	}
	*resid = 0;
	return 0;
}

tty_close(sn)
	struct snode *sn;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;
}

tty_getstat(sn, stp, mask)
	struct snode *sn;
	struct stat *stp;
	unsigned long mask;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;
	struct stat st;

	bzero(&st, sizeof(st));
	st.st_mode = 0777;
	st.st_nlink = 1;
	st.st_blksize = 8192;
	st_copy(&st, stp, mask);
	return 0;
}

tty_setstat(sn, stp, mask)
	struct snode *sn;
	struct stat *stp;
	unsigned long mask;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;

	if (mask == FATTR_SIZE && stp->st_size == 0) {
		/* XXX truncate -- from open (ok), or syscall truncate (bad) */
		return 0; /* XXX */
	}
	return EPERM;
}

tty_ioctl(sn, rw, type, command, param, psize)
	struct snode *sn;
	int rw;
	int type;
	int command;
	char *param;
	int psize;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;

	if (type != 't') {
		return EINVAL;
	}
	switch (command) {
		case icmd(TIOCGETD):
		case icmd(TIOCSETD):
			PCOPY(&tt->tt_ldisc);
			return 0;

		case icmd(TIOCSETP):
		case icmd(TIOCSETN):
			PCOPY(&tt->tt_sgttyb);

			/* If we've just switched to raw or cbreak mode,
			   then make all of the data previously buffed by
			   the line discipline available for reading.
			   */
			if (tt->tt_sgttyb.sg_flags & (CBREAK|RAW)) {
			   mutex_lock(tt->tt_lock);
			   if ((tt->last_readable = tt->last_editable)
			       != tt->last_free ) {
			      condition_broadcast( tt->tt_ricondition );
			   }
			   mutex_unlock(tt->tt_lock);
			}
			return 0;

		case icmd(TIOCGETP):
			PCOPY(&tt->tt_sgttyb);
			return 0;

		case icmd(TIOCSETC):
		case icmd(TIOCGETC):
			PCOPY(&tt->tt_tchars);
			return 0;

		case icmd(TIOCSLTC):
		case icmd(TIOCGLTC):
			PCOPY(&tt->tt_ltchars);
			return 0;

		case icmd(TIOCGWINSZ):
		case icmd(TIOCSWINSZ):
			PCOPY(&tt->tt_winsize);
			return 0;

		case icmd(TIOCGPGRP):
		case icmd(TIOCSPGRP):
			PCOPY(&tt->tt_pgrp);
			return 0;

		case icmd(TIOCLGET):
		case icmd(TIOCLSET):
			PCOPY(&tt->tt_lmodes);
			return 0;

		default:
			return EINVAL;
	}
}

tty_select(sn)
	struct snode *sn;
{
	struct ttstate *tt = (struct ttstate *) sn->sn_private;
	printf("tty_select called.\n");
}

tty_suspend()
{
	mutex_lock(&tty_no_read_mutex);
	tty_no_read = TRUE;
	mutex_unlock(&tty_no_read_mutex);
}

tty_resume()
{
	mutex_lock(&tty_no_read_mutex);
	tty_no_read = FALSE;
	condition_broadcast(&tty_no_read_condition);
	mutex_unlock(&tty_no_read_mutex);
}

Bsd_ttyc(ut, rval, flag)
struct ux_task *ut;
int rval[2];
boolean_t flag;
{
	if (flag)
		tty_resume();
	else
		tty_suspend();
	return 0;
}
