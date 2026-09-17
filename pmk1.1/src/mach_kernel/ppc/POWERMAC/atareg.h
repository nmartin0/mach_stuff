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
 * 
 */
/*
 * pmk1.1
 */

#define BOOTRECORDSIGNATURE			(0x55aa & 0x00ff)
#define FIXEDBITS				0x0a0
#define WHOLE_DISK(unit)			(((unit)<<4)+PART_DISK)

// these defines work for OHare and not necessarily any other controller
#define PORT_DATA(addr)				(0x0+(addr))
#define PORT_ERROR(addr)			(0x10+(addr))
#define PORT_PRECOMP(addr)			(0x10+(addr))
#define PORT_NSECTOR(addr)			(0x20+(addr))
#define PORT_SECTOR(addr)			(0x30+(addr))
#define PORT_CYLINDERLOWBYTE(addr)		(0x40+(addr))
#define PORT_CYLINDERHIBYTE(addr)		(0x50+(addr))
#define PORT_DRIVE_HEADREGISTER(addr)		(0x60+(addr))
#define PORT_STATUS(addr)			(0x70+(addr))
#define PORT_COMMAND(addr)			(0x70+(addr))

#define STAT_BUSY				0x080
#define STAT_READY 				0x040
#define STAT_WRITEFAULT				0x020
#define STAT_SEEKDONE				0x010
#define STAT_DATAREQUEST			0x008
#define STAT_ECC				0x004
#define STAT_INDEX 				0x002
#define STAT_ERROR				0x001 

#define CMD_RESTORE 				0x010
#define CMD_SEEK 				0x070
#define CMD_READ				0x020
#define CMD_WRITE				0x030
#define CMD_FORMAT				0x050
#define CMD_READVERIFY				0x040
#define CMD_DIAGNOSE				0x090
#define CMD_SETPARAMETERS			0x091
#define	CMD_READ_DMA				0x0C8
#define	CMD_WRITE_DMA				0x0CA
#define CMD_IDENTIFY				0x0EC

#define PATIENCE	3000000		/* how long to wait for controller */
#define PARTITION(z)	(minor(z) & 0x0f)
#define UNIT(z)		((minor(z) >> 4) & 0x03)

#define PDLOCATION	29
#define BAD_BLK		0x80			/* needed for V_VERIFY */

#define NDRIVES 1				/* drives per controller */
#define SECSIZE 512

/*
 *  PORT_NSECT register is 1 byte, limiting total # of sectors that can be read.
 *  It appears that 0 means read 256 sectors.
 */

#define SECLIMIT 256

/*  hh holds the state of the one and only (stupid board) current 
    block I/O request
*/
    
struct hh {
	u_char	curdrive;		/* drive the controller is using */
	u_char	controller_busy;	/* controller can't take cmd now */
	u_char	retry_count;		/* # of times cmd has been tried */
	u_char	restore_request;	/* restore command */
	u_char	identify_request;	/* identify command */
	int	rw_addr;		/* ram addr to read/write sector */
	int	physblock;		/* block # relative to partition 0 */
	u_int 	single_mode;		/* 1 = transfer one block each time */
	u_int	cylinder;		/* cylinder # rel. to part. 0 */
	u_int	head;			/* as it looks */
	u_int	sector;			/* as it looks */
	u_int	blockcount;		/* blocks done so far */
	u_int	blocktotal;		/* total blocks this request */
	u_int	start_of_unix[NDRIVES];	/* unix vs dos partitions */
};

/* the boot record partition table is documented in IBM AT Tech. Ref p. 9-6 */
struct 	boot_record	{
	u_char	boot_ind;	/* if it == 0x80, this partition is active */
	u_char	head;		/* driver does not look at this field */
	u_char	sector;		/* driver does not look at this field */
	u_char	cylinder;	/* driver does not look at this field */

	u_char	sys_ind;	/* driver does not look at this field */
	u_char	end_head;	/* driver does not look at this field */
	u_char	end_sector;	/* driver does not look at this field */
	u_char	end_cylinder;	/* driver does not look at this field */

	u_int	rel_sect;	/* where unix starts if boot_ind == 0x80 */
	u_int	num_sects;	/* driver does not look at this field */
};

/*
 * As described in AT attachment Interface for Disk Drives 
 * DRAFT X3T10 791D Revision 4c
 */

struct id_block {
  /* 000 */ 	unsigned short	config;		
  /* 002 */	unsigned short	cyls;		
  /* 004 */	unsigned short	reserved_0;	
  /* 006 */	unsigned short	heads;		
  /* 008 */	unsigned short	phys_bpt;	/* unformated bytes
     						   per track */
  /* 010 */	unsigned short	phys_bps;	/* unformated bytes
						   per sector */
  /* 012 */	unsigned short	spt;		/* sectors per track */
  /* 014 */	unsigned short	vuq_0[3];	/* vendor unique */
  /* 020 */	unsigned char	serial_nb[20];
  /* 040 */	unsigned short	buf_type;
  /* 042 */	unsigned short	buf_size;	/* 512 bytes unit */
  /* 044 */	unsigned short	ecc_bytes_lgth;
  /* 046 */	unsigned char	rev[8];
  /* 054 */	unsigned char	model[40];
  /* 094 */	unsigned char	max_sec;	/* max nb of sectors per r/w */
  /* 095 */	unsigned char	vuq_1;
  /* 096 */	unsigned short	double_io;	/* can perform double i/o */
  /* 098 */	unsigned char 	vuq_2;
  /* 099 */	unsigned char	capabilities;
  /* 100 */	unsigned short	reserved_1;
  /* 102 */	unsigned char	vuq_3;
  /* 103 */	unsigned char	PIO_cycle;
  /* 104 */	unsigned char	vuq_4;
  /* 105 */	unsigned char	DMA_sycle;
  /* 106 */	unsigned short	val_cur_values;	/* bit 0 tells if next
     						   4 cur_ fields are valid */ 
  /* 108 */	unsigned short	cur_cyls;
  /* 110 */	unsigned short	cur_heads;
  /* 112 */	unsigned short	cur_secs;
  /* 114 */	unsigned short	cur_capacity[2];	/* sector unit */
  /* 118 */	unsigned char	cur_max_sec;	
  /* 119 */	unsigned char	reserved_3;		
  /* 120 */	unsigned short	access_sects[2];
  /* 124 */	unsigned char	sw_dma;		/* is single word DMA transfer
					   	   supported ? */
  /* 125 */	unsigned char	sw_dma_on;	
  /* 126 */	unsigned char	mw_dma;		/* is mult. word DMA transfer
					   supported ? */
  /* 127 */	unsigned char	mw_dma_on;	
  /* 128 */	unsigned short	reserved_4[64];	
  /* 256 */	unsigned short	vuq_5[32];
  /* 320 */	unsigned short	reserved_5[96];
  /* 512 */
};

typedef struct id_block	hd_id_t;

typedef unsigned char   UInt8;
typedef unsigned short  UInt16;
typedef unsigned int    UInt32;





