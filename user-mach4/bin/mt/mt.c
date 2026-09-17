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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: mt.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:52  sclawson
 * New files.
 *
 * Revision 2.3  93/08/06  23:50:31  mrt
 * 	Close tape on signals.  "status" was not working.
 * 	[93/07/15            af]
 * 
 * Revision 2.2  93/05/31  15:51:59  mrt
 * 	First checkin
 *	Created from BNR source by Alessandro Forin
 * 
 */
/*
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mt.c	5.6 (Berkeley) 6/6/91";
#endif /* not lint */

/*
 * mt --
 *   magnetic tape manipulation program
 */
#include <mach.h>
#include <device/device_types.h>
#include <device/tape_status.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>

#ifndef	DEFTAPE
#define	DEFTAPE "tz0"
#endif

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)
#define	Density	"density"
#define Speed	"speed"
#define	Flags	"flags"

struct commands {
	char *c_name;
	int c_code;
	int c_ronly;
} com[] = {
	{ "weof",	MTWEOF,	0 },
	{ "eof",	MTWEOF,	0 },
	{ "fsf",	MTFSF,	1 },
	{ "bsf",	MTBSF,	1 },
	{ "fsr",	MTFSR,	1 },
	{ "bsr",	MTBSR,	1 },
	{ "rewind",	MTREW,	1 },
	{ "offline",	MTOFFL,	1 },
	{ "rewoffl",	MTOFFL,	1 },
	{ "status",	MTNOP,	1 },
	{ Density,	-1,	1 },
	{ Speed,	-1,	1 },
	{ Flags,	-1,	1 },
	{ 0 }
};

struct tape_params mt_com;
struct mtget mt_regs;
struct tape_status mt_status;
char *tape;
device_t	mtport;

main(argc, argv)
	char **argv;
{
	char line[80], *getenv();
	register char *cp;
	register struct commands *comp;
	io_return_t     rc;
	natural_t	count;

	if (argc > 2 && (equal(argv[1], "-t") || equal(argv[1], "-f"))) {
		argc -= 2;
		tape = argv[2];
		argv += 2;
	} else
		if ((tape = getenv("TAPE")) == NULL)
			tape = DEFTAPE;
	if (argc < 2) {
		fprintf(stderr, "usage: mt [ -f device ] command [ count ]\n");
		exit(1);
	}

	cp = argv[1];
	for (comp = com; comp->c_name != NULL; comp++)
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
			break;
	if (comp->c_name == NULL) {
		fprintf(stderr, "mt: don't grok \"%s\"\n", cp);
		exit(1);
	}

	openit(tape, comp->c_ronly, &mtport);
	setsigs();

	if (comp->c_code == -1) {
		count = TAPE_STATUS_COUNT;
		rc = device_get_status(mtport, TAPE_STATUS, (int*)&mt_status,
					&count);
		if (rc != D_SUCCESS) {
			mach_error("device_get_status:", rc);
			goto done;
		}

		if (strncmp(comp->c_name, Density, sizeof(Density)) == 0)
			mt_status.density = (argc > 2 ? atoi(argv[2]) : 0);
		else if (strncmp(comp->c_name, Speed, sizeof(Speed)) == 0)
			mt_status.speed = (argc > 2 ? atoi(argv[2]) : 0);
		else if (strncmp(comp->c_name, Flags, sizeof(Flags)) == 0)
			mt_status.flags = (argc > 2 ? atoh(argv[2]) : 0);

		rc = device_set_status(mtport, TAPE_STATUS, (int*)&mt_status,
					TAPE_STATUS_COUNT);
		if (rc != D_SUCCESS) {
			mach_error("device_set_status:", rc);
			goto done;
		}
	}
	else if (comp->c_code == MTNOP) {
		count = sizeof(mt_regs) / sizeof(int);
		rc = device_get_status(mtport, MTIOCGET, (int*)&mt_regs,
					&count);
		if (rc != D_SUCCESS) {
			mach_error("mt", rc);
			goto done;
		}
		status(&mt_regs);
	} else {
		mt_com.mt_operation = comp->c_code;
		mt_com.mt_repeat_count = (argc > 2 ? atoi(argv[2]) : 1);
		if (mt_com.mt_repeat_count < 0) {
			fprintf(stderr, "mt: negative repeat count\n");
			goto done;
		}
		rc = device_set_status(mtport, MTIOCTOP, (int*)&mt_com,
					sizeof(mt_com) / sizeof(int));
		if (rc != D_SUCCESS) {
			fprintf(stderr, "%s %s %d ", tape, comp->c_name,
				mt_com.mt_repeat_count);
			mach_error("failed:", rc);
			goto done;
		}
	}
done:
	(void) device_close(mtport);
	exit(rc);
}

openit(
	char	*devicename,
	int	mode,
	device_t *device_port_p)
{
	mach_port_t     device_server_port;
	io_return_t     rc;

	device_server_port = task_by_pid(-2);
	if (device_server_port == MACH_PORT_NULL) {
		fprintf(stderr, "Could not get device_server_port\n");
		exit(1);
	}
	rc = device_open(device_server_port, mode, devicename, device_port_p);
	if (rc != D_SUCCESS) {
		mach_error(devicename, rc);
		exit(1);
	}
}

catchsig(
	int	sig,
	int	code,
	struct sigcontext *scp)
{
	(void) device_close(mtport);
	if (sig != SIGHUP)
		fprintf(stderr, "mt: got signal %d\n", sig);
	exit(-1);
}

setsigs()
{
	struct sigvec s;
	int i;

	s.sv_handler = catchsig;
	s.sv_mask = 0;
	s.sv_flags = 0;
	for (i = 1; i < 32; i++)
		(void) sigvec(i, &s, 0);
}

#ifdef vax
#include <vaxmba/mtreg.h>
#include <vaxmba/htreg.h>

#include <vaxuba/utreg.h>
#include <vaxuba/tmreg.h>
#undef b_repcnt		/* argh */
#include <vaxuba/tsreg.h>
#endif

#ifdef sun
#include <sundev/tmreg.h>
#include <sundev/arreg.h>
#endif

#ifdef tahoe
#include <tahoe/vba/cyreg.h>
#endif

struct tape_desc {
	short	t_type;		/* type of magtape device */
	char	*t_name;	/* printing name */
	char	*t_dsbits;	/* "drive status" register */
	char	*t_erbits;	/* "error" register */
} tapes[] = {
#ifdef vax
	{ MT_ISTS,	"ts11",		0,		TSXS0_BITS },
	{ MT_ISHT,	"tm03",		HTDS_BITS,	HTER_BITS },
	{ MT_ISTM,	"tm11",		0,		TMER_BITS },
	{ MT_ISMT,	"tu78",		MTDS_BITS,	0 },
	{ MT_ISUT,	"tu45",		UTDS_BITS,	UTER_BITS },
#endif
#ifdef sun
	{ MT_ISCPC,	"TapeMaster",	TMS_BITS,	0 },
	{ MT_ISAR,	"Archive",	ARCH_CTRL_BITS,	ARCH_BITS },
#endif
#ifdef tahoe
	{ MT_ISCY,	"cipher",	CYS_BITS,	CYCW_BITS },
#endif
	{ 0 }
};

/*
 * Interpret the status buffer returned
 */
status(bp)
	register struct mtget *bp;
{
	register struct tape_desc *mt;

	for (mt = tapes; mt->t_type; mt++)
		if (mt->t_type == bp->mt_type)
			break;
	if (mt->t_type == 0) {
		printf("unknown tape drive type (%d)\n", bp->mt_type);
		return;
	}
	printf("%s tape drive, residual=%d\n", mt->t_name, bp->mt_resid);
	printreg("ds", bp->mt_dsreg, mt->t_dsbits);
	printreg("\ner", bp->mt_erreg, mt->t_erbits);
	putchar('\n');
}

/*
 * Print a register a la the %b format of the kernel's printf
 */
printreg(s, v, bits)
	char *s;
	register char *bits;
	register unsigned short v;
{
	register int i, any = 0;
	register char c;

	if (bits && *bits == 8)
		printf("%s=%o", s, v);
	else
		printf("%s=%x", s, v);
	bits++;
	if (v && bits) {
		putchar('<');
		while (i = *bits++) {
			if (v & (1 << (i-1))) {
				if (any)
					putchar(',');
				any = 1;
				for (; (c = *bits) > 32; bits++)
					putchar(c);
			} else
				for (; *bits > 32; bits++)
					;
		}
		putchar('>');
	}
}
