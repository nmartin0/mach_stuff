/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: grfioctl.h,v $
 * Revision 1.1.6.2  1997/04/07  16:14:16  bruel
 * 	removed HPOSFCOMPAT.
 * 	[97/04/07            bruel]
 *
 * Revision 1.1.6.1  1997/04/07  09:27:16  bruel
 * 	cleaned up personalities dependancies.
 * 	[97/04/07            bruel]
 * 
 * Revision 1.1.4.1  1995/11/02  14:42:41  bruel
 * 	hpdev/ files moved to hp_pa/HP700
 * 	[95/09/04            bernadat]
 * 	[95/09/27            bruel]
 * 
 * Revision 1.1.18.1  1995/09/05  11:26:01  bruel
 * 	#ifdef hp_pa big cleanup.
 * 	[95/09/05            bruel]
 * 
 * Revision 1.1.14.1  1995/03/15  17:52:41  bruel
 * 	hppa merge
 * 	[1995/03/15  09:46:00  bruel]
 * 
 * Revision 1.1.2.1  1994/02/11  13:42:48  bruel
 * 	Created from Utah.
 * 	[93/11/29            bruel]
 * 
 * $EndLog$
 */
/* 
 * Copyright (c) 1987-1994, The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 *	Utah $Hdr: grfioctl.h 1.22 94/12/14$
 */

struct	grfinfo {
	int	gd_id;			/* HPUX identifier */
	caddr_t	gd_regaddr;		/* control registers physaddr */
	int	gd_regsize;		/* control registers size */
	caddr_t	gd_fbaddr;		/* frame buffer physaddr */
	int	gd_fbsize;		/* frame buffer size */
	short	gd_colors;		/* number of colors */
	short	gd_planes;		/* number of planes */
/* new stuff */
	int	gd_fbwidth;		/* frame buffer width */
	int	gd_fbheight;		/* frame buffer height */
	int	gd_dwidth;		/* displayed part width */
	int	gd_dheight;		/* displayed part height */
	int	gd_pad[6];		/* for future expansion */
};

/* types */
#define GRFGATOR	8
#define GRFBOBCAT	9
#define	GRFCATSEYE	9
#define GRFRBOX		10
#define GRFFIREEYE	11
#define GRFHYPERION	12
#define GRFDAVINCI	14

struct	grf_fbinfo {
	int	id;
	int	mapsize;
	int	dwidth, dlength;
	int	width, length;
	int	xlen;
	int	bpp, bppu;
	int	npl, nplbytes;
	char	name[32];
	int	attr;
	caddr_t	fbbase, regbase;
	caddr_t	regions[6];
};

#ifndef _IOH
#define _IOH(x,y)	_IO(x,y)

#define	GCID		_IOR('G', 0, int)
#define	GCON		_IOH('G', 1)
#define	GCOFF		_IOH('G', 2)
#define	GCAON		_IOH('G', 3)
#define	GCAOFF		_IOH('G', 4)
#define	GCMAP		_IOWR('G', 5, int)
#define	GCUNMAP		_IOWR('G', 6, int)
#define	GCMAP_HPUX	_IO('G', 5)
#define	GCUNMAP_HPUX	_IO('G', 6)
#define	GCLOCK		_IOH('G', 7)
#define	GCUNLOCK	_IOH('G', 8)
#define	GCLOCK_MINIMUM	_IOH('G', 9)
#define	GCUNLOCK_MINIMUM _IOH('G', 10)
#define	GCSTATIC_CMAP	_IOH('G', 11)
#define	GCVARIABLE_CMAP _IOH('G', 12)
#define GCDESCRIBE	_IOR('G', 21, struct grf_fbinfo)
#define GCFASTLOCK	_IOH('G', 26)

#endif

