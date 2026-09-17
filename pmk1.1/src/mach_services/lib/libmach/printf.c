/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
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
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * pmk1.1
 */

#include <mach.h>
#include <device/device.h>
#include <stdarg.h>

static mach_port_t console_port;

void
printf_init(mach_port_t device_server_port)
{
	(void) device_open(device_server_port,
			   0,
			   "console",
			   &console_port);
}

#define	PRINTF_BUFMAX	128

struct printf_state {
	char buf[PRINTF_BUFMAX + 1]; /* extra for '\r\n' */
	unsigned int index;
};

static void
flush(struct printf_state *state)
{
	int amt;

	(void) device_write_inband(console_port, 0, 0,
				   state->buf, state->index, &amt);
	state->index = 0;
}

static void
putchar(char *arg, int c)
{
	struct printf_state *state = (struct printf_state *) arg;

	if (c == '\n') {
	    state->buf[state->index] = '\r';
	    state->index++;
	}
	state->buf[state->index] = c;
	state->index++;

	if (state->index >= PRINTF_BUFMAX)
	    flush(state);
}

/*
 * Printing (to console)
 */
vprintf(char *fmt, va_list args)
{
	struct printf_state state;

	state.index = 0;
	_doprnt(fmt, args, 0, (void (*)()) putchar, (char *) &state);

	if (state.index != 0)
	    flush(&state);
}

/*VARARGS1*/
printf(char *fmt, ...)
{
	va_list	args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

safe_gets(char *str, int maxlen)
{
	register char *lp;
	register int c;

	char	inbuf[IO_INBAND_MAX];
	unsigned int count;
	register char *ip;
	char *strmax = str + maxlen - 1; /* allow space for trailing 0 */

	lp = str;
	for (;;) {
	    count = IO_INBAND_MAX;
	    (void) device_read_inband(console_port, 0, 0,
				      sizeof(inbuf), inbuf, &count);
	    for (ip = inbuf; ip < &inbuf[count]; ip++) {
		c = *ip;
		switch (c) {
		    case '\n':
		    case '\r':
			printf("\n");
			*lp++ = 0;
			return;

		    case '\b':
		    case '#':
		    case '\177':
			if (lp > str) {
			    printf("\b \b");
			    lp--;
			}
			continue;
		    case '@':
		    case 'u'&037:
			lp = str;
			printf("\n\r");
			continue;
		    default:
			if (c >= ' ' && c < '\177') {
			    if (lp < strmax) {
				*lp++ = c;
				printf("%c", c);
			    }
			    else {
				printf("%c", '\007'); /* beep */
			    }
			}
		}
	    }
	}
}
