/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	isofs_util.c,v $
 * Revision 2.2  93/08/07  16:56:12  mrt
 * 	Took it from BSDSS, with heavy mods.
 * 	Separate out iso_date() from where it was embedded.
 * 	[93/07/03            af]
 * 
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *
 *	@(#)isofs_util.c
 */

#ifndef isonum_711
int
isonum_711 ( char *p )
{
	return (*p & 0xff);
}
#endif

int
isonum_712 ( char *p )
{
	int val;

	val = *p;
	if (val & 0x80)
		val |= ((int)(-1) << 8);
	return (val);
}

int
isonum_721 ( char *p )
{
	return ((p[0] & 0xff) | ((p[1] & 0xff) << 8));
}

int
isonum_722 ( char *p )
{
	return (((p[0] & 0xff) << 8) | (p[1] & 0xff));
}

#ifndef isonum_723
int
isonum_723 ( char *p )
{
#if 0
	if (p[0] != p[3] || p[1] != p[2]) {
		printf( "invalid format 7.2.3 number\n");
		return 0;
	}
#endif
	return (isonum_721 (p));
}
#endif

int
isonum_731 ( char *p )
{
	return ((p[0] & 0xff)
		| ((p[1] & 0xff) << 8)
		| ((p[2] & 0xff) << 16)
		| ((p[3] & 0xff) << 24));
}

int
isonum_732 ( char *p )
{
	return (((p[0] & 0xff) << 24)
		| ((p[1] & 0xff) << 16)
		| ((p[2] & 0xff) << 8)
		| (p[3] & 0xff));
}

#ifndef isonum_733
int
isonum_733 ( char *p )
{
	int i;

#if 0
	for (i = 0; i < 4; i++) {
		if (p[i] != p[7-i]) {
			printf( "bad format 7.3.3 number\n");
			return 0;
		}
	}
#endif
	return (isonum_731 (p));
}
#endif

int
iso_date( char *p, int isa_high_sierra)
{
	int year, month, day, hour, minute, second, tz;
	int crtime, days, i;

	year = p[0] - 70;
	month = p[1];
	day = p[2];
	hour = p[3];
	minute = p[4];
	second = p[5];
	tz = (isa_high_sierra) ? 0 : p[6];
	
	if (year < 0) {
		crtime = 0;
	} else {
		int monlen[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
		days = year * 365;
		if (year > 2)
			days += (year+2) / 4;
		for (i = 1; i < month; i++)
			days += monlen[i-1];
		if (((year+2) % 4) == 0 && month > 2)
			days++;
		days += day - 1;
		crtime = ((((days * 24) + hour) * 60 + minute) * 60)
			+ second;

		/* sign extend */
		if (tz & 0x80)
			tz |= (-1 << 8);

		/* timezone offset is unreliable on some disks */
		if (-48 <= tz && tz <= 52)
			crtime += tz * 15 * 60;
	}
	return crtime;
}

/*
 * translate and compare a filename
 */
int
isofncmp(
	char	*fn,
	int	fnlen,
	char	*isofn,
	int	isolen)
{
	int fnidx;

	fnidx = 0;
	for (fnidx = 0; fnidx < isolen; fnidx++, fn++) {
		char c = *isofn++;

		if (fnidx > fnlen)
			return (0);

		if (c >= 'A' && c <= 'Z') {
			if (c + ('a' - 'A') !=  *fn)
				return(0);
			else
				continue;
		}
		if (c == ';')
			return ((fnidx == fnlen));
		if (c != *fn)
			return (0);
	}
	return (1);
}

/*
 * translate a filename
 */
void
isofntrans(
	char	*infn,
	int	infnlen,
	char	*outfn,
	short	*outfnlen)
{
	int fnidx;

	fnidx = 0;
	for (fnidx = 0; fnidx < infnlen; fnidx++) {
		char c = *infn++;

		if (c >= 'A' && c <= 'Z')
			*outfn++ = c + ('a' - 'A');
		else if (c == ';') {
			*outfnlen = fnidx;
			return;
		} else
			*outfn++ = c;
	}
	*outfnlen = infnlen;
}
