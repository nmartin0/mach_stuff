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
 * $Log:	dosfs_utils.c,v $
 * Revision 2.2  93/09/15  13:30:08  mrt
 * 	Fixed a number of spots in the name translation code.
 * 	[93/08/26  16:31:58  af]
 * 
 * 	First version that can write to the filesystem.
 * 	[93/07/30  00:09:57  af]
 * 
 * 	Created.
 * 	[93/07/12            af]
 * 
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>

#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

/*
 * Multiple dots and variable length extensions are possible.
 */
public int
dosfsfncmp(
	unsigned char	*nm,
	int		nmlen,
	struct dosfs_directory_record *ep,
	int		xlate)
{
	char		buf[13];
	short		len;

	if (dosfsfntrans(ep, buf, &len, xlate))
		return 0;

	if (nmlen < len) return 0;
	if (nm[0] != buf[0]) return 0;
	return (bcmp(nm, buf, nmlen) == 0);
}

/*
 * Translate from a DOS directory entry into a unix style name and length.
 */
public int
dosfsfntrans(
	struct dosfs_directory_record *ep,
	unsigned char	*buf,
	short		*nmlen,
	int		xlate)
{
	register unsigned char	*dn;
	register int	len, c, e;

	dn = ep->name;
	len = 0;
	if (*dn == DOS_NAME_ESCAPE) {
		buf[len++] = DOS_NAME_DELETED;
		dn++;
	}

	/*
	 * Convert upper to lower if case translation is
	 * enabled.  Also, drop blanks.
	 */
	if (xlate & DOSFSMNT_CASE_TR) {

		for ( ; len < 8; len++) {
			c = *dn++;
			if (c >= 'A' && c <= 'Z')
				c = c + ('a' - 'A');
			else if (c == ' ')
				break;
			buf[len] = c;
		}
		
		dn = ep->ext;
		c = *dn++;

		/* Handle implied dot */
		if ((xlate & DOSFSMNT_MDOT_TR) && (c != ' '))
			buf[len++] = '.';


		/* null extension ? */
		if (c != ' ')
			for (e = len + 3; len < e; len++) {
				if (c >= 'A' && c <= 'Z')
					c = c + ('a' - 'A');
				else if (c == ' ')
					break;
				buf[len] = c;
				c = *dn++;
			}

		buf[len] = 0;

	} else {
		/*
		 * Don't case translate. If there are any blanks
		 * (DOS) keep them.
		 */
		for (; len < 8; len++)
			buf[len] = *dn++;
		if (xlate & DOSFSMNT_MDOT_TR) {
			buf[len++] = '.';
			e = 12;
		} else
			e = 11;
		dn = ep->ext;
		for (; len < e; len++)
			buf[len] = *dn++;

		/* cleanup if silly */
silly:		while (buf[len-1] == ' ') len--;
		if ((len == 9) && (xlate & DOSFSMNT_MDOT_TR)) {
			len--;
			goto silly;
		}
		buf[len] = 0;
	}

	*nmlen = len;
	return 0;
}

/*
 * Copy a filename into a dos directory entry.  Do translations on filename.
 * Routine assumes that the name passed in is a valid name for the type of
 * translation required (see dosfs_dirbadname).
 */
public int
dosfsfntodos(
	struct dosfs_directory_record *ep,
	unsigned char	*namep,
	int		namlen,
	int		xlate)
{
	register unsigned char	*dn, *en;
	register int	c, len, e;

	dn = ep->name;
	len = 0;
	if (*namep == DOS_NAME_DELETED) {
		*dn++ = DOS_NAME_ESCAPE;
		len++;
	}

	/*
	 * Translate lower to upper if case translation
	 * is enabled.  Fill in blanks as DOS-appropriate.
	 */
	e = (xlate & DOSFSMNT_MDOT_TR) ? 12 : 11;

	if (xlate & DOSFSMNT_CASE_TR) {
		register int el;

		/*
		 * We know name and ext are contiguous
		 */
		el = 9999;
		for (; len < e; len++) {
			if ((len == 8) && (xlate & DOSFSMNT_MDOT_TR))
				if (len++ == namlen)  /* skip the dot */
					break;
			c = namep[len];
			if (c >= 'a' && c <= 'z')
				c = c - ('a' - 'A');
			else if (c == 0)
				break;
			else if (c == '.')
				el = len;
			*dn++ = c;
		}

		/*
		 * Pad if necessary
		 */
		for (; len < 11; len++)
			*dn++ = ' ';

		/*
		 * Watch out for when the
		 * extension must be shifted up.
		 */
		if ((xlate & DOSFSMNT_MDOT_TR) &&
		    (namlen <= 8) &&
		    ((namlen - el) > 1) &&
		    ((namlen - el) < 5)) {
			dn = ep->name;
			ep->ext[0] = dn[el+1];
			ep->ext[1] = dn[el+2];
			ep->ext[2] = dn[el+3];
			while (el < 8)
				dn[el++] = ' ';
		}

	} else {
		/*
		 * We know name and ext are contiguous
		 */
		for (; len < e; len++) {
			if ((len == 8) && (xlate & DOSFSMNT_MDOT_TR))
				if (len++ == namlen)  /* skip the dot */
					break;
			c = namep[len];
			if (c == 0)
				break;
			*dn++ =	c;
		}
		/*
		 * Pad if necessary
		 */
		for (; len < 11; len++)
			*dn++ = ' ';

	}

	return 0;
}

/*
 * Parse from dos to unix date rep
 */
static int days_per_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

/* xxxx Unix and DOS endup 4 hours apart: */
/* DOS:  mnt1.raw 1048576 07-26-93 9:09p */
/* Unix: mnt1.raw 1048576 Jul 26   5:09p */
/* this is in Pgh, GMT-5h USA dst --> ?? */

public int
dosfs_date(
	unsigned char	*date,
	unsigned char	*time)
{
	unsigned int year, month, day, hour, minute, second;
	int crtime, days, i;

	year = (date[1] >> 1) + 10;	/* was '80 based */
	month = ((date[0] >> 5) & 0x7) | ((date[1] & 1) << 3);
	day = date[0] & 0x1f;
	hour = time[1] >> 3;
	minute = ((time[0] >> 5) & 0x7) | ((time[1] & 0x7) << 3);
	second = (time[0] & 0x1f) << 1;

	days = year * 365;
	days += (year+2) / 4;
	for (i = 1; i < month; i++)
		days += days_per_month[i-1];
	if (((year+2) % 4) == 0 && month > 2)
		days++;
	days += day - 1;
	crtime = ((((days * 24) + hour) * 60 + minute) * 60)
			+ second;
	return crtime;
}

/*
 * The other way around
 */
public void
dosfs_dosdate(
	unsigned int	unix_seconds,
	unsigned char	*dtime,
	unsigned char	*ddate)
{
	register unsigned years, months, days, hours, minutes, seconds;

#define	SECMIN	((unsigned)60)			/* seconds per minute */
#define	SECHOUR	((unsigned)(60*SECMIN))		/* seconds per hour */
#define	SECDAY	((unsigned)(24*SECHOUR))	/* seconds per day */
#define	SECYR	((unsigned)(365*SECDAY))	/* sec per reg year */

#define	YRREF		1970
#define	LEAPYEAR(x)	(((x) % 4) == 0)

	years = YRREF;
	while (1) {
		seconds = SECYR;
		if (LEAPYEAR(years))
			seconds += SECDAY;
		if (unix_seconds < seconds)
			break;
		unix_seconds -= seconds;
		years++;
	}

	months = 0;
	while (1) {
		seconds = days_per_month[months++] * SECDAY;
		if (months == 2 /* February */ && LEAPYEAR(years))
			seconds += SECDAY;
		if (unix_seconds < seconds)
			break;
		unix_seconds -= seconds;
	}

	days = unix_seconds / SECDAY;
	unix_seconds -= SECDAY * days++;

	hours = unix_seconds / SECHOUR;
	unix_seconds -= SECHOUR * hours;

	minutes = unix_seconds / SECMIN;
	unix_seconds -= SECMIN * minutes;

	seconds = unix_seconds;

	/*
	 * On to dos now
	 */

	dtime[0] = ((seconds >> 1) & 0x1f) |
		   ((minutes & 0x7) << 5);
	dtime[1] = ((minutes >> 3) & 0x7) |
		   ((hours << 3));
	ddate[0] = (days & 0x1f) |
		   (months << 5);
	ddate[1] = ((months >> 3) & 1) |
		   ((years - 1980) << 1);

}
