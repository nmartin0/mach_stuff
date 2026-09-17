/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	stamp.h,v $
 * Revision 2.2  92/05/22  15:47:33  jfriedl
 * 	Spin longer, and less intensely.
 * 	[92/05/20  23:01:58  af]
 * 
 * 	Moved and adapted to 3.0 sources from 2.5 tree.
 * 	[92/05/10            af]
 * 
 * Revision 2.3  91/10/29  14:09:24  mja
 * 	Upgraded to Ultrix 4.2 version.
 * 	[91/09/25            moore]
 * 
 * Revision 2.2  90/08/08  17:25:47  mrt
 * 	Picked up from Ultrix.
 * 	[90/04/18            af]
 * 
 */

/*
 *	@(#)stamp.h	2.2	(ULTRIX)	3/3/90
 */
/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University	of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * 16-Jan-90	Sam Hsu
 *	Remove symbol conflicts with the ucode/server.  Move STIC definitions
 *	to gx.h
 *
 * 00-Xyz-89	Sam Hsu
 *	$Header: stamp.h,v 2.2 92/05/22 15:47:33 jfriedl Exp $
 *	Created.
 */
#ifndef _STAMP_H_
#define _STAMP_H_

typedef struct _stamp_cmd {
    unsigned opcode : 4;		/* <3:0> */
#define	STAMP_CMD_POINTS	0
#define	STAMP_CMD_LINES		1
#define	STAMP_CMD_TRIANGLES	2
#define	STAMP_CMD_COPYSPANS	5
#define	STAMP_CMD_READSPANS	6
#define	STAMP_CMD_WRITESPANS	7
#define	STAMP_CMD_VIDEO		8
    unsigned rgb_format : 2;		/* <5:4> */
#define STAMP_RGB_NONE		0
#define STAMP_RGB_CONST		1
#define STAMP_RGB_FLAT		2
#define STAMP_RGB_SMOOTH	3
    unsigned z_format : 2;		/* <7:6> */
#define STAMP_Z_NONE		0
#define STAMP_Z_CONST		1
#define STAMP_Z_FLAT		2
#define STAMP_Z_SMOOTH		3
    unsigned xymask_format : 2;		/* <9:8> */
#define	STAMP_XY_NONE		0
#define	STAMP_XY_PERPACKET	1
#define	STAMP_XY_PERPRIMITIVE	2
    unsigned linewidth_format : 2;	/* <11:10> */
#define	STAMP_LW_NONE		0
#define	STAMP_LW_PERPACKET	1
#define	STAMP_LW_PERPRIMITIVE	2
#define STAMP_LW_MASK		0x3fff	/* 14 bits */
    unsigned : 7;			/* <18:12> */
    unsigned cliprect : 1;		/* <19> */
    unsigned : 1;			/* <20> */
    unsigned mesh : 1;			/* <21> */
    unsigned : 1;			/* <22> */
    unsigned aa_line : 1;		/* <23> */
    unsigned : 7;			/* <30:24> */
    unsigned hs_equals : 1;		/* <31> */
} stampCmd;
#define STAMP_cmd	stampCmd

		/* COMMAND <3:0> */

#define CMD_MASK		(0xf)
#define CMD_POINTS		(0)
#define CMD_LINES		(1)
#define CMD_TRIANGLES		(2)
#define CMD_COPYSPANS		(5)
#define CMD_READSPANS		(6)
#define CMD_WRITESPANS		(7)
#define CMD_VIDEO		(8)

		/* COMMAND <5:4> */

#define RGB_MASK        	(0x30)
#define RGB_NONE  	        (0<<4)
#define RGB_CONST	 	(1<<4)
#define RGB_FLAT		(2<<4)
#define RGB_SMOOTH        	(3<<4)

		/* COMMAND <7:6> */

#define Z_MASK	        	(0xc0)
#define Z_NONE  	        (0<<6)
#define Z_CONST		 	(1<<6)
#define Z_FLAT			(2<<6)
#define Z_SMOOTH        	(3<<6)

		/* COMMAND <9:8> */

#define XY_MASK			(0x300)
#define XY_NONE			(0<<8)
#define XY_PERPKT		(1<<8)
#define XY_PERPRIM		(2<<8)

		/* COMMAND <11:10> */

#define LW_MASK			(0xc00)
#define LW_NONE	   		(0<<10)
#define LW_PERPKT	  	(1<<10)
#define LW_PERPRIM	 	(2<<10)

		/* COMMAND <19> */

#define CLIPRECT_MASK        	(0x80000)
#define CLIPRECT        	(1<<19)

		/* COMMAND <21> */

#define MESH_MASK		(0x200000)
#define MESH         		(1<<21)

		/* COMMAND <23> */

#define AA_LINE_MASK		(0x800000)
#define AA_LINE               	(1<<23)

		/* COMMAND <31> */

#define HS_EQUALS_MASK		(0x80000000)
#define HS_EQUALS              	(1<<31)


typedef struct _stamp_zcount {
    unsigned mask : 24;
    unsigned count : 8;
} stampZCount;
#define STAMP_zcount	stampZCount

#define STAMP_MAX_CMDS	((1<<8)-1)

typedef struct _stamp_update {
    unsigned enable : 1;		/* <0> */
    unsigned read_buff : 2;		/* <2:1> */
    unsigned write_buff : 2;		/* <4:3> */
    unsigned plane : 1;			/* <5> */
#define STAMP_PLANE_8X3		0
#define STAMP_PLANE_24		1
    unsigned save_sign : 1;		/* <6> */
    unsigned save_alpha : 1;		/* <7> */
    unsigned make_we : 3;		/* <10:8> */
#define STAMP_WE_SIGN		0x4
#define STAMP_WE_XYMASK		0x2
#define STAMP_WE_CLIPRECT	0x1
#define STAMP_WE_NONE		0x0
    unsigned supersample : 1;		/* <11> */
    unsigned umet : 7;			/* <18:12> */
#define STAMP_UMET_CLEAR	0x60
#define STAMP_UMET_AND		0x14
#define STAMP_UMET_ANDREV	0x15
#define STAMP_UMET_COPY		0x20
#define STAMP_UMET_ANDINV	0x16
#define STAMP_UMET_NOOP		0x40
#define STAMP_UMET_XOR		0x11
#define STAMP_UMET_OR		0x0f
#define STAMP_UMET_NOR		0x17
#define STAMP_UMET_EQUIV	0x10
#define STAMP_UMET_INV		0x4e
#define STAMP_UMET_ORREV	0x0e
#define STAMP_UMET_COPYINV	0x2d
#define STAMP_UMET_ORINV	0x0d
#define STAMP_UMET_NAND		0x0c
#define STAMP_UMET_SET		0x6c
#define STAMP_UMET_SUM		0x00
#define STAMP_UMET_DIFF		0x02
#define STAMP_UMET_REVDIFF	0x01
    unsigned span : 1;			/* <19> */
    unsigned aligned : 1;		/* <20> */
    unsigned minmax : 1;		/* <21> */
    unsigned mult : 1;			/* <22> */
    unsigned multacc : 1;		/* <23> */
    unsigned : 3;			/* <26:24> */
    unsigned halfbuff : 1;		/* <27> */
    unsigned dblbuff : 3;		/* <30:28> */
#define STAMP_DB_NONE		0x0
#define STAMP_DB_01		0x1
#define STAMP_DB_12		0x2
#define STAMP_DB_02		0x4
    unsigned init : 1;			/* <31> */
} stampUpdate;
#define STAMP_update	stampUpdate

		/* UPDATE{RGB,Z} <0> */

#define UPD_ENABLE_MASK		(0x1)
#define UPD_ENABLE             	(1)

		/* UPDATE <2:1> */

#define READ_BUFF_MASK    	(0x6)
#define READ_BUFF_0		(0<<1)
#define READ_BUFF_1		(1<<1)
#define READ_BUFF_2		(2<<1)
#define READ_BUFF_3		(3<<1)

		/* UPDATE <4:3> */

#define WRITE_BUFF_MASK    	(0x18)
#define WRITE_BUFF_0		(0<<3)
#define WRITE_BUFF_1		(1<<3)
#define WRITE_BUFF_2		(2<<3)
#define WRITE_BUFF_3		(3<<3)

		/* UPDATE <5> */

#define PLANE_MASK		(0x20)
#define PLANE_8x3		(0<<5)
#define PLANE_24		(1<<5)

		/* UPDATE <6> */

#define SAVE_SIGN_MASK		(0x40)
#define SAVE_SIGN		(1<<6)

		/* UPDATE <7> */

#define SAVE_ALPHA_MASK		(0x80)
#define SAVE_ALPHA		(1<<7)

		/* UPDATE <10:8> (Indicates how to construct write-enables */

#define WE_MASK			(0x700)
#define WE_CLIPRECT		(1<<8)
#define WE_XYMASK		(2<<8)
#define WE_SIGN			(4<<8)

		/* UPDATE <11> */

#define SUPERSAMPLE_MASK	(0x800)
#define SUPERSAMPLE		(1<<11)

		/* UPDATE <18:12> (Update method)

        /* logical functions */
#define UMET_MASK		(0x7f000)
#define UMET_CLEAR		(0x60<<12)
#define UMET_AND		(0x14<<12)
#define UMET_ANDREVERSE		(0x15<<12)
#define UMET_COPY		(0x20<<12)
#define UMET_ANDINVERTED	(0x16<<12)
#define UMET_NOOP		(0x40<<12)
#define UMET_XOR		(0x11<<12)
#define UMET_OR			(0x0F<<12)
#define UMET_NOR		(0x17<<12)
#define UMET_EQUIV		(0x10<<12)
#define UMET_INVERT		(0x4E<<12)
#define UMET_ORREVERSE		(0x0E<<12)
#define UMET_COPYINVERTED	(0x2D<<12)
#define UMET_ORINVERTED		(0x0D<<12)
#define UMET_NAND		(0x0C<<12)
#define UMET_SET		(0x6C<<12)
        /* arithmetic functions */
#define UMET_SUM		(0x00<<12)
#define UMET_DIFF		(0x02<<12)
#define UMET_REVDIFF		(0x01<<12)


		/* UPDATE <19> */

#define SPAN_MASK		(0x80000)
#define SPAN			(1<<19)

		/* UPDATE <20> */

#define COPYSPAN_ALIGNED_MASK	(0x100000)
#define COPYSPAN_ALIGNED	(1<<20)

		/* UPDATE <21> */

#define MINMAX_MASK		(0x200000)
#define MINMAX			(1<<21)

		/* UPDATE <22> */

#define MULT_MASK		(0x400000)
#define MULT			(1<<22)

		/* UPDATE <23> */

#define MULTACC_MASK		(0x800000)
#define MULTACC			(1<<23)

		/* UPDATE <27> */

#define HALF_BUFF_MASK		(0x8000000)
#define HALF_BUFF		(1<27)

		/* UPDATE <30:28> */

#define DB_MASK			(0x70000000)
		      /* for {4,5}x2 only */
#define DB_BUFF02_13		(1<<28)
		      /* for {4,5}x1 only */
#define DB_BUFF0_1		(1<<28)
#define DB_BUFF1_2		(2<<28)
#define DB_BUFF0_2		(4<<28)

		/* UPDATE <31> */

#define INITIALIZE_MASK		(0x80000000)
#define INITIALIZE		(1<<31)


/*
 * per-packet context for xymask 16x16 (text - 8x15)
 * nb: also need per-primitive xymask_addr context.
 */

#ifdef KERNEL
#define STAMP_WIDTH	(gx->stamp_width)
#define STAMP_HEIGHT	(gx->stamp_height)
#endif

typedef short stampXYMask[16];
#define STAMP_xymask	stampXYMask

#define XMASKADDR(startX, A)	(((A)-((startX)%STAMP_WIDTH))&0xF)
#define YMASKADDR(startY, B)	(((B)-((startY)%STAMP_HEIGHT))&0xF)

#define XYMASKADDR(X,Y, A,B) \
        ( XMASKADDR(X,A) << 16 \
         |YMASKADDR(Y,B) )

#define CONSXYADDR(X,Y)		XYMASKADDR(X,Y, 0,0)

/* optional per-packet context:
       line width
       xymask
       cliprect min & max
       rgb constant
       z constant
 * optional per-primitive context:
       xymask
       xymask addr
       prim data (vertices, spans info, video)
       line width
       halfspace equals conditions
       rgb flat, or rgb{1,2,3} smooth
       z flat, or z{1,2,3} smooth
 
  The 3 operations needed for the console are:
 	clear screen (wide line, umet=copy)
 	write text (wide line, umet=copy, xymask)
 	scroll (copy spans, umet=copy)
 */

#define STAMP_GOOD	(0)
#define STAMP_BUSY	(1)
#define STAMP_RETRIES	(20000)		/* = 400 msec */
#define STAMP_DELAY	(20)		/* = 20 usec */

#if defined(STIC) && !defined(KERNEL)
/*
 * When the VDAC is moved out from under the STIC, this will all be purely
 * useless, since STIC video packets will have very limited functionality.
 */
#define STIC_V_C0		(1<<30)
#define STIC_V_C1		(1<<31)
#define STIC_V_CMASK		(3<<30)

#define STIC_V_PIX_COLOR_LO	0x000000
#define STIC_V_PIX_COLOR_HI	(STIC_V_C0|0x000000)

#define STIC_V_OVRLY_COLOR0_LO	0x000000
#define STIC_V_OVRLY_COLOR0_HI	(STIC_V_C0|0x010101)

#define STIC_V_CUR_COLOR1_LO	0x818181
#define STIC_V_CUR_COLOR1_HI	(STIC_V_C0|0x010101)

#define STIC_V_CUR_COLOR2_LO	0x828282
#define STIC_V_CUR_COLOR2_HI	(STIC_V_C0|0x010101)

#define STIC_V_CUR_COLOR3_LO	0x838383
#define STIC_V_CUR_COLOR3_HI	(STIC_V_C0|0x010101)

#define STIC_V_ID_REG_LO	0x000000
#define STIC_V_ID_REG_HI	(STIC_V_C0|0x020202)

#define STIC_V_CMD_0_LO		0x010101
#define STIC_V_CMD_0_HI		(STIC_V_C0|0x020202)

#define STIC_V_CMD_1_LO		0x020202
#define STIC_V_CMD_1_HI		(STIC_V_C0|0x020202)

#define STIC_V_CMD_2_LO		0x030303
#define STIC_V_CMD_2_HI		(STIC_V_C0|0x020202)

#define STIC_V_PIX_RMASK_LO	0x040404
#define STIC_V_PIX_RMASK_HI	(STIC_V_C0|0x020202)

#define STIC_V_PIX_BMASK_LO	0x050505
#define STIC_V_PIX_BMASK_HI	(STIC_V_C0|0x020202)

#define STIC_V_INTERL_REG_LO	0x0a0a0a
#define STIC_V_INTERL_REG_HI	(STIC_V_C0|0x020202)

#define STIC_V_TEST_REG_LO	0x0b0b0b
#define STIC_V_TEST_REG_HI	(STIC_V_C0|0x020202)

#define STIC_V_SIG_RED_LO	0x0c0c0c
#define STIC_V_SIG_RED_HI	(STIC_V_C0|0x020202)

#define STIC_V_SIG_GREEN_LO	0x0d0d0d
#define STIC_V_SIG_GREEN_HI	(STIC_V_C0|0x020202)

#define STIC_V_SIG_BLUE_LO	0x0e0e0e
#define STIC_V_SIG_BLUE_HI	(STIC_V_C0|0x020202)

#define STIC_V_CUR_CMD_LO	0x000000
#define STIC_V_CUR_CMD_HI	(STIC_V_C0|0x030303)

#define STIC_V_CUR_XLO_LO	0x010101
#define STIC_V_CUR_XLO_HI	(STIC_V_C0|0x030303)

#define STIC_V_CUR_XHI_LO	0x020202
#define STIC_V_CUR_XHI_HI	(STIC_V_C0|0x030303)

#define STIC_V_CUR_YLO_LO	0x030303
#define STIC_V_CUR_YLO_HI	(STIC_V_C0|0x030303)

#define STIC_V_CUR_YHI_LO	0x040404
#define STIC_V_CUR_YHI_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_XLO_LO	0x050505
#define STIC_V_WIN_XLO_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_XHI_LO	0x060606
#define STIC_V_WIN_XHI_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_YLO_LO	0x070707
#define STIC_V_WIN_YLO_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_YHI_LO	0x080808
#define STIC_V_WIN_YHI_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_WLO_LO	0x090909
#define STIC_V_WIN_WLO_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_WHI_LO	0x0a0a0a
#define STIC_V_WIN_WHI_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_HLO_LO	0x0b0b0b
#define STIC_V_WIN_HLO_HI	(STIC_V_C0|0x030303)

#define STIC_V_WIN_HHI_LO	0x0c0c0c
#define STIC_V_WIN_HHI_HI	(STIC_V_C0|0x030303)

#define STIC_V_CUR_RAM_LO	0x000000
#define STIC_V_CUR_RAM_HI	(STIC_V_C0|0x040404)
#endif

/*
 * STamp Interface Chip
 */
typedef struct _stic_regs {
    volatile int __pad0;			/* 0x..180000 */
    volatile int __pad1;			/* 0x..180004 */
    volatile int	hsync;			/* 0x..180008 */
    volatile int	hsync2;			/* 0x..18000c */
    volatile int	hblank;			/* 0x..180010 */
    volatile int	vsync;			/* 0x..180014 */
    volatile int	vblank;			/* 0x..180018 */
    volatile int	vtest;			/* 0x..18001c */
    volatile int	ipdvint;		/* 0x..180020 */
    volatile int	__pad2;			/* 0x..180024 */
    volatile int	sticsr;			/* 0x..180028 */
    volatile int	busdat;			/* 0x..18002c */
    volatile int	busadr;			/* 0x..180030 */
    volatile int	__pad3;			/* 0x..180034 */
    volatile int	buscsr;			/* 0x..180038 */
    volatile int	modcl;			/* 0x..18003c */
} sticRegs;

typedef struct _stic_csr {
    unsigned tstfnc : 2;
    unsigned checkpar : 1;
    unsigned startvt : 1;
    unsigned start : 1;
    unsigned reset : 1;
    unsigned autoread : 1;
    unsigned startst : 1;
    unsigned : 24;
} stic_csr;

/* masks for STIC CSR register */
#define STIC_CSR_TSTFNC		0x00000003
#define STIC_CSR_TSTFNC_NORMAL 	0
#define STIC_CSR_TSTFNC_PARITY 	1
#define STIC_CSR_TSTFNC_CNTPIX 	2
#define STIC_CSR_TSTFNC_TSTDAC 	3
#define STIC_CSR_CHECKPAR	0x00000004
#define STIC_CSR_STARTVT	0x00000008
#define STIC_CSR_START		0x00000010
#define STIC_CSR_RESET		0x00000020
#define STIC_CSR_AUTOREAD	0x00000040
#define STIC_CSR_STARTST	0x00000080

typedef struct _stic_intr {
    unsigned	eint_en : 1;	     /* 0 */
    unsigned	eint : 1;	     /* 1 */
    unsigned	eint_mask : 1;	     /* 2 */
    unsigned	: 5;
    unsigned	vint_en : 1;	     /* 8 */
    unsigned	vint : 1;	     /* 9 */
    unsigned	vint_mask : 1;	     /* 10 */
    unsigned	: 5;
    unsigned	pint : 1;	     /* 16 */
    unsigned	pint_en : 1;	     /* 17 */
    unsigned	pint_mask : 1;	     /* 18 */
} sticIntr;
#define STIC_intr	sticIntr

/* masks for %int register */
#define STIC_INT_E_EN 0x00000001
#define STIC_INT_E    0x00000002
#define STIC_INT_E_WE 0x00000004
#define STIC_INT_V_EN 0x00000100
#define STIC_INT_V    0x00000200
#define STIC_INT_V_WE 0x00000400
#define STIC_INT_P_EN 0x00010000
#define STIC_INT_P    0x00020000
#define STIC_INT_P_WE 0x00040000

#define STIC_INT_WE   (STIC_INT_E_WE|STIC_INT_V_WE|STIC_INT_P_WE)
#define STIC_INT_CLR  (STIC_INT_E_EN|STIC_INT_E_WE|STIC_INT_V_WE|STIC_INT_P_WE)

typedef struct _stic_modtype {
    unsigned	: 8;
    unsigned	vdac : 1;
    unsigned	yconfig : 2;
    unsigned	xconfig : 1;
    unsigned	option : 3;
    unsigned	: 9;
    unsigned	revision : 8;
} sticCf;

/* masks for %modcl bits */
#define STIC_CF_VDAC_T		0x00000100 /* <8> */
#define STIC_CF_CONFIG_Y	0x00000600 /* <10:9> */
#define STIC_CF_CONFIG_X	0x00000800 /* <11> */
#define STIC_CF_CONFIG_OPTION	0x00007000 /* <14:12> */
#define STIC_CF_PLANES		0x00004000 /* <14> */
#define STIC_CF_ZPLANES		0x00001000 /* <12> */
#define STIC_CF_REV		0xff000000 /* <31:24> */

/* STIC option types (derived from the STIC modtype option field) */
#define STIC_OPT_2DA		0x0		/* 2D Accelerator */
#define STIC_OPT_2DA_SH		0x0		/* 2D Accelerator */
#define STIC_OPT_3DA_SH		STIC_CF_CONFIG_OPTION

#endif _STAMP_H_
