/* 
 * Mach Operating System
 * Copyright (c) 1990, 1991 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * ×Mach for MPW 3.1
 *
 * File: types.h
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef _TYPES_H_
#define _TYPES_H_

/*
 * Basic system types and major/minor device constructing/busting macros.
 */

/* major part of a device */
#define	major(x)	((long)(((unsigned)(x)>>8)&0377))

/* minor part of a device */
#define	minor(x)	((long)((x)&0377))

/* make a device number */
#define	makedev(x,y)	((dev_t)(((x)<<8) | (y)))

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned long	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		/* sys III compat */

typedef	struct	_quad { u_long val[2]; } quad;
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	u_long	ino_t;
typedef	long	swblk_t;

typedef	long	size_t;
typedef	long	time_t;
typedef	short	dev_t;
typedef	u_long	off_t;
typedef	u_short	uid_t;
typedef	u_short	gid_t;

#define	NBBY	8		/* number of bits in a byte */

#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#endif /* _TYPES_H_ */