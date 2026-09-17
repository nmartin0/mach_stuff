/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * ×Mach for MPW 3.1
 *
 * File: machparam.h
 */

#ifndef _MACHPARAM_H_
#define _MACHPARAM_H_

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Machine dependent constants for mac2.
 */
#define	NBPG	8192		/* bytes/page */
#define	PGOFSET	(NBPG-1)	/* byte offset into page */
#define	PGSHIFT	13		/* LOG2(NBPG) */

#define	CLSIZE		1
#define	CLSIZELOG2	0

/* clicks to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to clicks */
#define	btoc(x)	((((unsigned)(x)+(NBPG-1))>>PGSHIFT))

#endif /* _MACHPARAM_H_ */