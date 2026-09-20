/* $Id: aic7xxx.h,v 1.1 1999/07/05 22:02:09 vallon Exp $ */

#ifndef _ppc_POWERMAC_pci_aic7xxx
#define _ppc_POWERMAC_pci_aic7xxx

typedef enum {
	AHC_NONE	= 0x0000,
	AHC_CHIPID_MASK	= 0x00FF,
	AHC_AIC7770	= 0x0001,
	AHC_AIC7850	= 0x0002,
	AHC_AIC7860	= 0x0003,
	AHC_AIC7870	= 0x0004,
	AHC_AIC7880	= 0x0005,
	AHC_AIC7890	= 0x0006,
	AHC_AIC7895	= 0x0007,
	AHC_AIC7896	= 0x0008,
	AHC_VL		= 0x0100,	/* Bus type VL */
	AHC_EISA	= 0x0200,	/* Bus type EISA */
	AHC_PCI		= 0x0400	/* Bus type PCI */
} ahc_chip;

typedef enum {
	AHC_FENONE	= 0x0000,
	AHC_ULTRA	= 0x0001,	/* Supports 20MHz Transfers */
	AHC_ULTRA2	= 0x0002,	/* Supports 40MHz Transfers */
	AHC_WIDE  	= 0x0004,	/* Wide Channel */
	AHC_TWIN	= 0x0008,	/* Twin Channel */
	AHC_MORE_SRAM	= 0x0010,	/* 80 bytes instead of 64 */
	AHC_CMD_CHAN	= 0x0020,	/* Has a Command DMA Channel */
	AHC_QUEUE_REGS	= 0x0040,	/* Has Queue management registers */
	AHC_SG_PRELOAD	= 0x0080,	/* Can perform auto-SG preload */
	AHC_SPIOCAP	= 0x0100,	/* Has a Serial Port I/O Cap Register */
	AHC_MULTI_TID	= 0x0200,	/* Has bitmask of TIDs for select-in */
	AHC_HS_MAILBOX	= 0x0400,	/* Has HS_MAILBOX register */
	AHC_AIC7770_FE	= AHC_FENONE,	
	AHC_AIC7850_FE	= AHC_FENONE|AHC_SPIOCAP,
	AHC_AIC7860_FE	= AHC_ULTRA|AHC_SPIOCAP,
	AHC_AIC7870_FE	= AHC_FENONE,
	AHC_AIC7880_FE	= AHC_ULTRA,
	AHC_AIC7890_FE	= AHC_MORE_SRAM|AHC_CMD_CHAN|AHC_ULTRA2|AHC_QUEUE_REGS
			  |AHC_SG_PRELOAD|AHC_MULTI_TID|AHC_HS_MAILBOX,
	AHC_AIC7895_FE	= AHC_MORE_SRAM|AHC_CMD_CHAN|AHC_ULTRA,
	AHC_AIC7896_FE	= AHC_MORE_SRAM|AHC_CMD_CHAN|AHC_ULTRA2|AHC_QUEUE_REGS
			  |AHC_SG_PRELOAD|AHC_MULTI_TID|AHC_HS_MAILBOX
} ahc_feature;

typedef enum {
	AHC_FNONE		= 0x000,
	AHC_PAGESCBS		= 0x001,/* Enable SCB paging */
	AHC_CHANNEL_B_PRIMARY	= 0x002,/*
					 * On twin channel adapters, probe
					 * channel B first since it is the
					 * primary bus.
					 */
	AHC_USEDEFAULTS		= 0x004,/*
					 * For cards without an seeprom
					 * or a BIOS to initialize the chip's
					 * SRAM, we use the default target
					 * settings.
					 */
	AHC_SHARED_SRAM		= 0x010,
	AHC_LARGE_SEEPROM	= 0x020,/* Uses C56_66 not C46 */
	AHC_RESET_BUS_A		= 0x040,
	AHC_RESET_BUS_B		= 0x080,
	AHC_EXTENDED_TRANS_A	= 0x100,
	AHC_EXTENDED_TRANS_B	= 0x200,
	AHC_TERM_ENB_A		= 0x400,
	AHC_TERM_ENB_B		= 0x800,
	AHC_INITIATORMODE	= 0x1000,/*
					  * Allow initiator operations on
					  * this controller.
					  */
	AHC_TARGETMODE		= 0x2000,/*
					  * Allow target operations on this
					  * controller.
					  */
	AHC_NEWEEPROM_FMT	= 0x4000,
	AHC_RESOURCE_SHORTAGE	= 0x8000,
	AHC_TQINFIFO_BLOCKED	= 0x10000 /* Blocked waiting for ATIOs */
} ahc_flag;

/*
 * The driver keeps up to MAX_SCB scb structures per card in memory.  The SCB
 * consists of a "hardware SCB" mirroring the fields availible on the card
 * and additional information the kernel stores for each transaction.
 */
struct hardware_scb {
/*0*/   u_int8_t  control;
/*1*/	u_int8_t  tcl;		/* 4/1/3 bits */
/*2*/	u_int8_t  status;
/*3*/	u_int8_t  SG_count;
/*4*/	u_int32_t SG_pointer;
/*8*/	u_int8_t  residual_SG_count;
/*9*/	u_int8_t  residual_data_count[3];
/*12*/	u_int32_t data;
/*16*/	u_int32_t datalen;		/* Really only three bytes, but its
					 * faster to treat it as a long on
					 * a quad boundary.
					 */
/*20*/	u_int32_t cmdpointer;
/*24*/	u_int8_t  cmdlen;
/*25*/	u_int8_t  tag;			/* Index into our kernel SCB array.
					 * Also used as the tag for tagged I/O
					 */
/*26*/	u_int8_t  next;			/* Used for threading SCBs in the
					 * "Waiting for Selection" and
					 * "Disconnected SCB" lists down
					 * in the sequencer.
					 */
/*27*/	u_int8_t  scsirate;		/* Value for SCSIRATE register */
/*28*/	u_int8_t  scsioffset;		/* Value for SCSIOFFSET register */
/*29*/	u_int8_t  spare[3];		/*
					 * Spare space available on
					 * all controller types.
					 */
/*32*/	u_int8_t  cmdstore[16];		/*
					 * CDB storage for controllers
					 * supporting 64 byte SCBs.
					 */
/*48*/	u_int32_t cmdstore_busaddr;	/*
					 * Address of command store for
					 * 32byte SCB adapters
					 */
/*48*/	u_int8_t  spare_64[12];	/*
					 * Pad to 64 bytes.
					 */
};

struct scb {
	struct	hardware_scb	*hscb;
#if !defined(MACH_KERNEL)
	SLIST_ENTRY(scb)	 links;	 /* for chaining */
	union ccb		*ccb;	 /* the ccb for this cmd */
	scb_flag		 flags;
	bus_dmamap_t		 dmamap;
	struct	ahc_dma_seg 	*sg_list;
	bus_addr_t		 sg_list_phys;
	u_int			 sg_count;/* How full ahc_dma_seg is */
#endif
};

/*
 * Connection desciptor for select-in requests in target mode.
 * The first byte is the connecting target, followed by identify
 * message and optional tag information, terminated by 0xFF.  The
 * remainder is the command to execute.  The cmd_valid byte is on
 * an 8 byte boundary to simplify setting it on aic7880 hardware
 * which only has limited direct access to the DMA FIFO.
 */
struct target_cmd {
	u_int8_t initiator_channel;
	u_int8_t targ_id;	/* Target ID we were selected at */
	u_int8_t identify;	/* Identify message */
	u_int8_t bytes[21];
	u_int8_t cmd_valid;
	u_int8_t pad[7];
};

struct scb_data {
#if !defined(MACH_KERNEL)
	struct	hardware_scb	*hscbs;	    /* Array of hardware SCBs */
	struct	scb *scbarray;		    /* Array of kernel SCBs */
	SLIST_HEAD(, scb) free_scbs;	/*
					 * Pool of SCBs ready to be assigned
					 * commands to execute.
					 */
	struct	scsi_sense_data *sense; /* Per SCB sense data */

	/*
	 * "Bus" addresses of our data structures.
	 */
	bus_dma_tag_t	 hscb_dmat;	/* dmat for our hardware SCB array */
	bus_dmamap_t	 hscb_dmamap;
	bus_addr_t	 hscb_busaddr;
	bus_dma_tag_t	 sense_dmat;
	bus_dmamap_t	 sense_dmamap;
	bus_addr_t	 sense_busaddr;
	bus_dma_tag_t	 sg_dmat;	/* dmat for our sg segments */
	SLIST_HEAD(, sg_map_node) sg_maps;
	u_int8_t	numscbs;
	u_int8_t	maxhscbs;	/* Number of SCBs on the card */
	u_int8_t	init_level;	/*
					 * How far we've initialized
					 * this structure.
					 */
#endif
};

#endif

