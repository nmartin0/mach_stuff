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
 * $Log:	rock_ridge.h,v $
 * Revision 2.3  93/08/10  18:25:15  mrt
 * 	Got one more change from Sandro PD[0] -> PD[1].
 * 	[93/08/10            mrt]
 * 
 * Revision 2.2  93/08/07  16:56:31  mrt
 * 	Revised, based on my reading of the Rock Ridge specifications,
 * 	Version 1, Rev 1.09, Aug 14, 1991.
 * 	[93/07/04  21:50:37  af]
 * 
 * 	Taken from Linux source and heavily pounded upon.
 * 	[93/06/29            af]
 * 
 */
/*
 *  linux/fs/isofs/rock.h
 *
 *  (C) 1992  Eric Youngdale
 *
 *  Rock Ridge Extensions to iso9660
 *
 * This file is part of Linux.
 *
 * It is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * It is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAS; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/* These structs are used by the system-use-sharing protocol, in which the
   Rock Ridge extensions are imbedded.  It is quite possible that other
   extensions are present on the disk, and this is fine as long as they
   all use SUSP */

struct SU_SP {
  unsigned char magic[2];
  unsigned char skip;
};

struct SU_CE {
  char extent[8];
  char offset[8];
  char size[8];
};

struct SU_PD {
  char pad[1];
};

struct SU_ER {
  unsigned char len_id;
  unsigned char len_des;
  unsigned char len_src;
  unsigned char ext_ver;
  char data[0];
};

struct RR_RR {
  char flags[1];	/* someday it will grow.. */
#define RR_RR_PX	0x1
#define RR_RR_PN	0x2
#define RR_RR_SL	0x4
#define RR_RR_NM	0x8
#define RR_RR_CL	0x10
#define RR_RR_PL	0x20
#define RR_RR_RE	0x40
#define RR_RR_TF	0x80
};

struct RR_PX {
  char mode[8];	/* same as in sys/inode.h, except pipe */
#define RR_PX_MMASK	(0167777)
  char n_links[8];
  char uid[8];
  char gid[8];
};

struct RR_PN {
  char dev_high[8];
  char dev_low[8];
};


struct SL_component {
  unsigned char flags;
#define RR_SL_CONT	0x1
#define RR_SL_DOT	0x2
#define RR_SL_DOTDOT	0x4
#define RR_SL_ROOT	0x8
#define RR_SL_VOLROOT	0x10
#define RR_SL_HOST	0x20
  unsigned char len;
  char text[0];
};

struct RR_SL {
  unsigned char flags;
  struct SL_component link;
};

struct RR_NM {
  unsigned char flags;
#define RR_NM_CONT	0x1
#define RR_NM_DOT	0x2
#define RR_NM_DOTDOT	0x4
#define RR_NM_res	0x18
#define RR_NM_HOST	0x20
  char name[0];
};

struct RR_CL {
  char location[8];
};

struct RR_PL {
  char location[8];
};

struct stamp {
  char time[7];
};

struct longstamp {
  char time[17];
};

struct RR_TF {
  char flags;
#define RR_TF_CREATE	0x1
#define RR_TF_MODIFY	0x2
#define RR_TF_ACCESS	0x4
#define RR_TF_ATTRIBUTES 0x8
#define RR_TF_BACKUP	0x10
#define RR_TF_EXPIRATION 0x20
#define RR_TF_EFFECTIVE	0x40
#define RR_TF_LONG_FORM	0x80
  struct stamp times[0];  /* Variable number of these beasts */
};


struct rock_ridge {
  char signature[2];
  unsigned char len;
  unsigned char version;
  union {
    struct SU_SP SP;
    struct SU_CE CE;
    struct SU_PD PD;
    struct SU_ER ER;
    struct RR_RR RR;
    struct RR_PX PX;
    struct RR_PN PN;
    struct RR_SL SL;
    struct RR_NM NM;
    struct RR_CL CL;
    struct RR_PL PL;
    struct RR_TF TF;
  } x;
};

