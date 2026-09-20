/* $Id: aic7xxx_mach.c,v 1.3 1999/07/05 22:02:09 vallon Exp $ */

#include <ppc/POWERMAC/pci/aic7xxx_mach.h>
#include <kern/misc_protos.h>
#include <ppc/POWERMAC/pcivar.h>
#include <ppc/POWERMAC/pci/types.h>
#include <ppc/POWERMAC/pcireg.h>
#include <ppc/POWERMAC/pci/aic7xxx.h>

#define __inline

static void aic7xxx_attach(pcici_t tag, int unit);

static u_long ahc_unit; /* Number of attached devices */

#define AHC_PCI_IOADDR	PCIR_MAPS		/* I/O Address */
#define AHC_PCI_MEMADDR	(PCIR_MAPS + 4)	/* Mem I/O Address */

static __inline u_int64_t
ahc_compose_id(u_int device, u_int vendor, u_int subdevice, u_int subvendor)
{
	u_int64_t id;

	id = subvendor
	   | (subdevice << 16)
	   | ((u_int64_t)vendor << 32)
	   | ((u_int64_t)device << 48);

	return (id);
}

#define ID_ALL_MASK		0xFFFFFFFFFFFFFFFFull
#define ID_DEV_VENDOR_MASK	0xFFFFFFFF00000000ull
#define ID_AIC7850		0x5078900400000000ull
#define ID_AHA_2910_15_20_30C	0x5078900478509004ull
#define ID_AIC7855		0x5578900400000000ull
#define ID_AIC7860		0x6078900400000000ull
#define ID_AIC7860C		0x6078900478609004ull
#define ID_AHA_2940AU_0		0x6178900400000000ull
#define ID_AHA_2940AU_1		0x6178900478619004ull
#define ID_AHA_2930C_VAR	0x6038900438689004ull

#define ID_AIC7870		0x7078900400000000ull
#define ID_AHA_2940		0x7178900400000000ull
#define ID_AHA_3940		0x7278900400000000ull
#define ID_AHA_398X		0x7378900400000000ull
#define ID_AHA_2944		0x7478900400000000ull
#define ID_AHA_3944		0x7578900400000000ull

#define ID_AIC7880		0x8078900400000000ull
#define ID_AIC7880_B		0x8078900478809004ull
#define ID_AHA_2940AU_CN	0x2178900478219004ull
#define ID_AHA_2940U		0x8178900400000000ull
#define ID_AHA_3940U		0x8278900400000000ull
#define ID_AHA_2944U		0x8478900400000000ull
#define ID_AHA_3944U		0x8578900400000000ull
#define ID_AHA_398XU		0x8378900400000000ull
#define ID_AHA_4944U		0x8678900400000000ull
#define ID_AHA_2940UB		0x8178900478819004ull
#define ID_AHA_2930U		0x8878900478889004ull
#define ID_AHA_2940U_PRO	0x8778900478879004ull
#define ID_AHA_2940U_CN		0x0078900478009004ull

#define ID_AIC7895		0x7895900478959004ull
#define ID_AIC7895_RAID_PORT	0x7893900478939004ull
#define ID_AHA_2940U_DUAL	0x7895900478919004ull
#define ID_AHA_3940AU		0x7895900478929004ull
#define ID_AHA_3944AU		0x7895900478949004ull

#define ID_AIC7890		0x001F9005000F9005ull
#define ID_AHA_2930U2		0x0011900501819005ull
#define ID_AHA_2940U2B		0x00109005A1009005ull
#define ID_AHA_2940U2_OEM	0x0010900521809005ull
#define ID_AHA_2940U2		0x00109005A1809005ull
#define ID_AHA_2950U2B		0x00109005E1009005ull

#define ID_AIC7896		0x005F9005FFFF9005ull
#define ID_AHA_3950U2B_0	0x00509005FFFF9005ull
#define ID_AHA_3950U2B_1	0x00509005F5009005ull
#define ID_AHA_3950U2D_0	0x00519005FFFF9005ull
#define ID_AHA_3950U2D_1	0x00519005B5009005ull

#define ID_AIC7810		0x1078900400000000ull
#define ID_AIC7815		0x1578900400000000ull

typedef int (ahc_device_setup_t)(pcici_t, char *, ahc_chip *,
				 ahc_feature *, ahc_flag *);

static ahc_device_setup_t ahc_aic7850_setup;
static ahc_device_setup_t ahc_aic7860_setup;
static ahc_device_setup_t ahc_aic7870_setup;
static ahc_device_setup_t ahc_aha394X_setup;
static ahc_device_setup_t ahc_aha398X_setup;
static ahc_device_setup_t ahc_aic7880_setup;
static ahc_device_setup_t ahc_aha394XU_setup;
static ahc_device_setup_t ahc_aha398XU_setup;
static ahc_device_setup_t ahc_aic7890_setup;
static ahc_device_setup_t ahc_aic7895_setup;
static ahc_device_setup_t ahc_aic7896_setup;
static ahc_device_setup_t ahc_raid_setup;
static ahc_device_setup_t ahc_aha394XX_setup;
static ahc_device_setup_t ahc_aha398XX_setup;

struct ahc_pci_identity {
	u_int64_t		 full_id;
	u_int64_t		 id_mask;
	const char		*name;
	ahc_device_setup_t	*setup;
};

struct ahc_pci_identity ahc_pci_ident_table [] =
{
	/* aic7850 based controllers */
	{
		ID_AHA_2910_15_20_30C,
		ID_ALL_MASK,
		"Adaptec 2910/15/20/30C SCSI adapter",
		ahc_aic7850_setup
	},
	/* aic7860 based controllers */
	{
		ID_AHA_2940AU_0 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940A Ultra SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AHA_2930C_VAR,
		ID_ALL_MASK,
		"Adaptec 2930C SCSI adapter (VAR)",
		ahc_aic7860_setup
	},
	/* aic7870 based controllers */
	{
		ID_AHA_2940,
		ID_ALL_MASK,
		"Adaptec 2940 SCSI adapter",
		ahc_aic7870_setup
	},
	{
		ID_AHA_3940,
		ID_ALL_MASK,
		"Adaptec 3940 SCSI adapter",
		ahc_aha394X_setup
	},
	{
		ID_AHA_398X,
		ID_ALL_MASK,
		"Adaptec 398X SCSI RAID adapter",
		ahc_aha398X_setup
	},
	{
		ID_AHA_2944,
		ID_ALL_MASK,
		"Adaptec 2944 SCSI adapter",
		ahc_aic7870_setup
	},
	{
		ID_AHA_3944,
		ID_ALL_MASK,
		"Adaptec 3944 SCSI adapter",
		ahc_aha394X_setup
	},
	/* aic7880 based controllers */
	{
		ID_AHA_2940AU_CN & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940A/CN Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_2940U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_3940U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 3940 Ultra SCSI adapter",
		ahc_aha394XU_setup
	},
	{
		ID_AHA_2944U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2944 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_3944U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 3944 Ultra SCSI adapter",
		ahc_aha394XU_setup
	},
	{
		ID_AHA_398XU & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 398X Ultra SCSI RAID adapter",
		ahc_aha398XU_setup
	},
	{
		/* XXX Don't know the slot numbers so can't identify channels */
		ID_AHA_4944U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 4944 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_2930U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2930 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_2940U_PRO & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940 Pro Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_2940U_CN & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940/CN Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	/* aic7890 based controllers */
	{
		ID_AHA_2930U2,
		ID_ALL_MASK,
		"Adaptec 2930 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2940U2B,
		ID_ALL_MASK,
		"Adaptec 2940B Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2940U2_OEM,
		ID_ALL_MASK,
		"Adaptec 2940 Ultra2 SCSI adapter (OEM)",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2940U2,
		ID_ALL_MASK,
		"Adaptec 2940 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2950U2B,
		ID_ALL_MASK,
		"Adaptec 2950 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	/* aic7895 based controllers */	
	{
		ID_AHA_2940U_DUAL,
		ID_ALL_MASK,
		"Adaptec 2940/DUAL Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AHA_3940AU,
		ID_ALL_MASK,
		"Adaptec 3940A Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AHA_3944AU,
		ID_ALL_MASK,
		"Adaptec 3944A Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	/* aic7896/97 based controllers */	
	{
		ID_AHA_3950U2B_0,
		ID_ALL_MASK,
		"Adaptec 3950B Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AHA_3950U2B_1,
		ID_ALL_MASK,
		"Adaptec 3950B Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AHA_3950U2D_0,
		ID_ALL_MASK,
		"Adaptec 3950D Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AHA_3950U2D_1,
		ID_ALL_MASK,
		"Adaptec 3950D Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	/* Generic chip probes for devices we don't know 'exactly' */
	{
		ID_AIC7850 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7850 SCSI adapter",
		ahc_aic7850_setup
	},
	{
		ID_AIC7855 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7855 SCSI adapter",
		ahc_aic7850_setup
	},
	{
		ID_AIC7860 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7860 SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AIC7870 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7870 SCSI adapter",
		ahc_aic7870_setup
	},
	{
		ID_AIC7880 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7880 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AIC7890 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7890/91 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AIC7895 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7895 Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AIC7895_RAID_PORT & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7895 Ultra SCSI adapter (RAID PORT)",
		ahc_aic7895_setup
	},
	{
		ID_AIC7896 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7896/97 Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AIC7810 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7810 RAID memory controller",
		ahc_raid_setup
	},
	{
		ID_AIC7815 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7815 RAID memory controller",
		ahc_raid_setup
	}
};

static const int ahc_num_pci_devs =
	sizeof(ahc_pci_ident_table) / sizeof(*ahc_pci_ident_table);
		
#define AHC_394X_SLOT_CHANNEL_A	4
#define AHC_394X_SLOT_CHANNEL_B	5

#define AHC_398X_SLOT_CHANNEL_A	4
#define AHC_398X_SLOT_CHANNEL_B	8
#define AHC_398X_SLOT_CHANNEL_C	12

#define	DEVCONFIG		0x40
#define		SCBSIZE32	0x00010000ul	/* aic789X only */
#define		MPORTMODE	0x00000400ul	/* aic7870 only */
#define		RAMPSM		0x00000200ul	/* aic7870 only */
#define		VOLSENSE	0x00000100ul
#define		SCBRAMSEL	0x00000080ul
#define		MRDCEN		0x00000040ul
#define		EXTSCBTIME	0x00000020ul	/* aic7870 only */
#define		EXTSCBPEN	0x00000010ul	/* aic7870 only */
#define		BERREN		0x00000008ul
#define		DACEN		0x00000004ul
#define		STPWLEVEL	0x00000002ul
#define		DIFACTNEGEN	0x00000001ul	/* aic7870 only */

static const char* ahc_pci_probe(pcici_t tag, pcidi_t type);
static void ahc_pci_attach(pcici_t config_id, int unit);

static struct	pci_device ahc_pci_driver = {
	"ahc",
	ahc_pci_probe,
	ahc_pci_attach,
	&ahc_unit,
	NULL
}; 

static struct ahc_pci_identity *
ahc_find_pci_device(pcici_t tag)
{
	u_int64_t  full_id;
	struct	   ahc_pci_identity *entry;
	u_int	   deviceid;
	u_int	   vendorid;
	u_int	   subdeviceid;
	u_int	   subvendorid;
	u_int	   i;

	deviceid = pci_cfgread(tag, PCIR_DEVICE, /*bytes*/2);
	vendorid = pci_cfgread(tag, PCIR_VENDOR, /*bytes*/2);
	subdeviceid = pci_cfgread(tag, PCIR_SUBDEV_0, /*bytes*/2);
	subvendorid = pci_cfgread(tag, PCIR_SUBVEND_0, /*bytes*/2);
	full_id = ahc_compose_id(deviceid,
				 vendorid,
				 subdeviceid,
				 subvendorid);

	for (i = 0; i < ahc_num_pci_devs; i++) {
		entry = &ahc_pci_ident_table[i];
		if (entry->full_id == (full_id & entry->id_mask))
			return (entry);
	}
	return (NULL);
}

static const char*
ahc_pci_probe(pcici_t tag, pcidi_t type)
{
	struct	   ahc_pci_identity *entry;

	entry = ahc_find_pci_device(tag);
	if (entry != NULL)
		return (entry->name);
	return (NULL);
}

static void
ahc_pci_attach(pcici_t config_id, int unit)
{
	pci_port_t	   io_port;
#if !defined(MACH_KERNEL)
	bus_dma_tag_t	   parent_dmat;
#endif
	struct		   ahc_softc *ahc;
	struct		   ahc_pci_identity *entry;
	vm_offset_t	   vaddr;
#ifdef AHC_ALLOW_MEMIO
	vm_offset_t	   paddr;
#endif
	u_int		   command;
	struct scb_data   *shared_scb_data;
	ahc_chip	   ahc_t = AHC_NONE;
	ahc_feature	   ahc_fe = AHC_FENONE;
	ahc_flag	   ahc_f = AHC_FNONE;
	u_int		   our_id = 0;
	u_int		   sxfrctl1;
	u_int		   scsiseq;
	int		   error;
	int		   opri;
	char		   channel;

	shared_scb_data = NULL;
	command = pci_cfgread(config_id, PCIR_COMMAND, /*bytes*/1);
	entry = ahc_find_pci_device(config_id);
	if (entry == NULL)
		return;

	error = entry->setup(config_id, &channel, &ahc_t, &ahc_fe, &ahc_f);
	if (error != 0)
		return;

	vaddr = 0 /*NULL*/;
#ifdef AHC_ALLOW_MEMIO
	if ((command & PCI_COMMAND_MEM_ENABLE) == 0
	 || (pci_map_mem(config_id, AHC_PCI_MEMADDR, &vaddr, &paddr)) == 0)
#endif
		if ((command & PCI_COMMAND_IO_ENABLE) == 0
		 || (pci_map_port(config_id, AHC_PCI_IOADDR, &io_port)) == 0)
			return;

	/* Ensure busmastering is enabled */
	command |= PCIM_CMD_BUSMASTEREN;
	pci_cfgwrite(config_id, PCIR_COMMAND, command, /*bytes*/1);

#if !defined(MACH_KERNEL)
	/* Allocate a dmatag for our SCB DMA maps */
	/* XXX Should be a child of the PCI bus dma tag */
	error = bus_dma_tag_create(/*parent*/NULL, /*alignment*/0,
				   /*boundary*/0,
				   /*lowaddr*/BUS_SPACE_MAXADDR_32BIT,
				   /*highaddr*/BUS_SPACE_MAXADDR,
				   /*filter*/NULL, /*filterarg*/NULL,
				   /*maxsize*/MAXBSIZE, /*nsegments*/AHC_NSEG,
				   /*maxsegsz*/AHC_MAXTRANSFER_SIZE,
				   /*flags*/BUS_DMA_ALLOCNOW, &parent_dmat);

	if (error != 0) {
		printf("ahc_pci_attach: Could not allocate DMA tag "
		       "- error %d\n", error);
		return;
	}

	/* On all PCI adapters, we allow SCB paging */
	ahc_f |= AHC_PAGESCBS;
	if ((ahc = ahc_alloc(unit, io_port, vaddr, parent_dmat,
			     ahc_t|AHC_PCI, ahc_fe, ahc_f,
			     shared_scb_data)) == NULL)
		return;

	ahc->channel = channel;

	/* Store our PCI bus information for use in our PCI error handler */
	ahc->pci_config_id = config_id;
	
	/* Remeber how the card was setup in case there is no SEEPROM */
	ahc_outb(ahc, HCNTRL, ahc->pause);
	if ((ahc->features & AHC_ULTRA2) != 0)
		our_id = ahc_inb(ahc, SCSIID_ULTRA2) & OID;
	else
		our_id = ahc_inb(ahc, SCSIID) & OID;
	sxfrctl1 = ahc_inb(ahc, SXFRCTL1) & STPWEN;
	scsiseq = ahc_inb(ahc, SCSISEQ);

	if (ahc_reset(ahc) != 0) {
		/* Failed */
		ahc_free(ahc);
		return;
	}

	/*
	 * Take a look to see if we have external SRAM.
	 * We currently do not attempt to use SRAM that is
	 * shared among multiple controllers.
	 */
	if ((ahc->features & AHC_ULTRA2) != 0) {
		u_int dscommand0;

		dscommand0 = ahc_inb(ahc, DSCOMMAND0);
		if ((dscommand0 & RAMPS) != 0) {
			u_int32_t devconfig;

			devconfig = pci_cfgread(config_id, DEVCONFIG,
						/*bytes*/4);
			if ((devconfig & MPORTMODE) != 0) {
				/* Single user mode */

				/*
				 * XXX Assume 9bit SRAM and enable
				 * parity checking
				 */
				devconfig |= EXTSCBPEN;
				pci_cfgwrite(config_id, DEVCONFIG,
					     devconfig, /*bytes*/4);

				/*
				 * Set the bank select apropriately.
				 */
				if (ahc->channel == 'B')
					ahc_outb(ahc, SCBBADDR, 1);
				else
					ahc_outb(ahc, SCBBADDR, 0);
				
				/* Select external SCB SRAM */
				dscommand0 &= ~INTSCBRAMSEL;
				ahc_outb(ahc, DSCOMMAND0, dscommand0);

				if (ahc_probe_scbs(ahc) == 0) {
					/* External ram isn't really there */
					dscommand0 |= INTSCBRAMSEL;
					ahc_outb(ahc, DSCOMMAND0, dscommand0);
				} else if (bootverbose)
					printf("%s: External SRAM bank%d\n",
					       ahc_name(ahc),
					       ahc->channel == 'B' ? 1 : 0);
			}

		}
	} else if ((ahc->chip & AHC_CHIPID_MASK) >= AHC_AIC7870) {
		u_int32_t devconfig;

		devconfig = pci_cfgread(config_id, DEVCONFIG, /*bytes*/4);
		if ((devconfig & RAMPSM) != 0
		 && (devconfig & MPORTMODE) != 0) {

			/* XXX Assume 9bit SRAM and enable parity checking */
			devconfig |= EXTSCBPEN;

			/* XXX Assume fast SRAM */
			devconfig &= ~EXTSCBTIME;

			/*
			 * Set the bank select apropriately.
			 */
			if ((ahc->chip & AHC_CHIPID_MASK) == AHC_AIC7895) {
				if (ahc->channel == 'B')
					ahc_outb(ahc, SCBBADDR, 1);
				else
					ahc_outb(ahc, SCBBADDR, 0);
			}

			/* Select external SRAM */
			devconfig &= ~SCBRAMSEL;
			pci_cfgwrite(config_id, DEVCONFIG,
				     devconfig, /*bytes*/4);

			if (ahc_probe_scbs(ahc) == 0) {
				/* External ram isn't really there */
				devconfig |= SCBRAMSEL;
				pci_cfgwrite(config_id, DEVCONFIG,
					     devconfig, /*bytes*/4);
			} else if (bootverbose)
				printf("%s: External SRAM bank%d\n",
				       ahc_name(ahc),
				       ahc->channel == 'B' ? 1 : 0);
		}
	}

	if (!(pci_map_int(config_id, ahc_intr, (void *)ahc, &cam_imask))) {
		ahc_free(ahc);
		return;
	}

	/*
	 * Protect ourself from spurrious interrupts during
	 * intialization.
	 */
	opri = splcam();

	/*
	 * Do aic7880/aic7870/aic7860/aic7850 specific initialization
	 */
	{
		u_int8_t sblkctl;
		char	 *id_string;

		switch(ahc_t) {
		case AHC_AIC7896:
		{
			u_int dscommand0;

			/*
			 * DPARCKEN doesn't work correctly on
			 * some MBs so don't use it.
			 */
			id_string = "aic7896/97 ";
			dscommand0 = ahc_inb(ahc, DSCOMMAND0);
			dscommand0 &= ~(USCBSIZE32|DPARCKEN);
			dscommand0 |= CACHETHEN|MPARCKEN;
			ahc_outb(ahc, DSCOMMAND0, dscommand0);
			break;
		}
		case AHC_AIC7890:
		{
			u_int dscommand0;

			/*
			 * DPARCKEN doesn't work correctly on
			 * some MBs so don't use it.
			 */
			id_string = "aic7890/91 ";
			dscommand0 = ahc_inb(ahc, DSCOMMAND0);
			dscommand0 &= ~(USCBSIZE32|DPARCKEN);
			dscommand0 |= CACHETHEN|MPARCKEN;
			ahc_outb(ahc, DSCOMMAND0, dscommand0);
			break;
		}
		case AHC_AIC7895:
			id_string = "aic7895 ";
			break;
		case AHC_AIC7880:
			id_string = "aic7880 ";
			break;
		case AHC_AIC7870:
			id_string = "aic7870 ";
			break;
		case AHC_AIC7860:
			id_string = "aic7860 ";
			break;
		case AHC_AIC7850:
			id_string = "aic7850 ";
			break;
		default:
			printf("ahc: Unknown controller type.  Ignoring.\n");
			ahc_free(ahc);
			splx(opri);
			return;
		}

		/* See if we have an SEEPROM and perform auto-term */
		check_extport(ahc, &sxfrctl1);

		/*
		 * Take the LED out of diagnostic mode
		 */
		sblkctl = ahc_inb(ahc, SBLKCTL);
		ahc_outb(ahc, SBLKCTL, (sblkctl & ~(DIAGLEDEN|DIAGLEDON)));

		/*
		 * I don't know where this is set in the SEEPROM or by the
		 * BIOS, so we default to 100% on Ultra or slower controllers
		 * and 75% on ULTRA2 controllers.
		 */
		if ((ahc->features & AHC_ULTRA2) != 0) {
			ahc_outb(ahc, DFF_THRSH, RD_DFTHRSH_75|WR_DFTHRSH_75);
		} else {
			ahc_outb(ahc, DSPCISTATUS, DFTHRSH_100);
		}

		if (ahc->flags & AHC_USEDEFAULTS) {
			/*
			 * PCI Adapter default setup
			 * Should only be used if the adapter does not have
			 * an SEEPROM.
			 */
			/* See if someone else set us up already */
			if (scsiseq != 0) {
				printf("%s: Using left over BIOS settings\n",
					ahc_name(ahc));
				ahc->flags &= ~AHC_USEDEFAULTS;
			} else {
				/*
				 * Assume only one connector and always turn
				 * on termination.
				 */
 				our_id = 0x07;
				sxfrctl1 = STPWEN;
			}
			ahc_outb(ahc, SCSICONF,
				 (our_id & 0x07)|ENSPCHK|RESET_SCSI);

			ahc->our_id = our_id;
		}

		printf("%s: %s", ahc_name(ahc), id_string);
	}

	/*
	 * Record our termination setting for the
	 * generic initialization routine.
	 */
	if ((sxfrctl1 & STPWEN) != 0)
		ahc->flags |= AHC_TERM_ENB_A;

	if (ahc_init(ahc)) {
		ahc_free(ahc);
		splx(opri);
		return;
	}

	/* XXX Crude hack - fix sometime */
	if (ahc->flags & AHC_SHARED_SRAM) {
		/* Only set this once we've successfully probed */
		if (shared_scb_data == NULL)
			first_398X = ahc;
	}

	splx(opri);

	ahc_attach(ahc);
#endif
}

static int
ahc_aic7850_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	*channel = 'A';
	*chip = AHC_AIC7850;
	*features = AHC_AIC7850_FE;
	return (0);
}

static int
ahc_aic7860_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	*channel = 'A';
	*chip = AHC_AIC7860;
	*features = AHC_AIC7860_FE;
	return (0);
}

static int
ahc_aic7870_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	*channel = 'A';
	*chip = AHC_AIC7870;
	*features = AHC_AIC7870_FE;
	return (0);
}

static int
ahc_aha394X_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	int error;

	error = ahc_aic7870_setup(dev, channel, chip, features, flags);
	if (error == 0)
		error = ahc_aha394XX_setup(dev, channel, chip, features, flags);
	return (error);
}

static int
ahc_aha398X_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	int error;

	error = ahc_aic7870_setup(dev, channel, chip, features, flags);
	if (error == 0)
		error = ahc_aha398XX_setup(dev, channel, chip, features, flags);
	return (error);
}

static int
ahc_aic7880_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	*channel = 'A';
	*chip = AHC_AIC7880;
	*features = AHC_AIC7880_FE;
	return (0);
}

static int
ahc_aha394XU_setup(pcici_t dev, char *channel, ahc_chip *chip,
		   ahc_feature *features, ahc_flag *flags)
{
	int error;

	error = ahc_aic7880_setup(dev, channel, chip, features, flags);
	if (error == 0)
		error = ahc_aha394XX_setup(dev, channel, chip, features, flags);
	return (error);
}

static int
ahc_aha398XU_setup(pcici_t dev, char *channel, ahc_chip *chip,
		   ahc_feature *features, ahc_flag *flags)
{
	int error;

	error = ahc_aic7880_setup(dev, channel, chip, features, flags);
	if (error == 0)
		error = ahc_aha398XX_setup(dev, channel, chip, features, flags);
	return (error);
}

static int
ahc_aic7890_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	*channel = 'A';
	*chip = AHC_AIC7890;
	*features = AHC_AIC7890_FE;
	*flags |= AHC_NEWEEPROM_FMT;
	return (0);
}

static int
ahc_aic7895_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	u_int32_t devconfig;

	*channel = dev->func == 1 ? 'B' : 'A';
	*chip = AHC_AIC7895;
	*features = AHC_AIC7895_FE;
 	*flags |= AHC_NEWEEPROM_FMT;
	devconfig = pci_cfgread(dev, DEVCONFIG, /*bytes*/4);
	devconfig &= ~SCBSIZE32;
	pci_cfgwrite(dev, DEVCONFIG, devconfig, /*bytes*/4);
	return (0);
}

static int
ahc_aic7896_setup(pcici_t dev, char *channel, ahc_chip *chip,
		  ahc_feature *features, ahc_flag *flags)
{
	*channel = dev->func == 1 ? 'B' : 'A';
	*chip = AHC_AIC7896;
	*features = AHC_AIC7896_FE;
	*flags |= AHC_NEWEEPROM_FMT;
	return (0);
}

static int
ahc_raid_setup(pcici_t dev, char *channel, ahc_chip *chip,
	       ahc_feature *features, ahc_flag *flags)
{
	printf("RAID functionality unsupported\n");
	return -1 /* ENXIO */;
}

static int
ahc_aha394XX_setup(pcici_t dev, char *channel, ahc_chip *chip,
		   ahc_feature *features, ahc_flag *flags)
{
	switch (dev->slot) {
	case AHC_394X_SLOT_CHANNEL_A:
		*channel = 'A';
		break;
	case AHC_394X_SLOT_CHANNEL_B:
		*channel = 'B';
		break;
	default:
		printf("adapter at unexpected slot %d\n"
		       "unable to map to a channel\n",
		       dev->slot);
	}
	return (0);
}

static int
ahc_aha398XX_setup(pcici_t dev, char *channel, ahc_chip *chip,
		   ahc_feature *features, ahc_flag *flags)
{
	switch (dev->slot) {
	case AHC_398X_SLOT_CHANNEL_A:
		*channel = 'A';
		break;
	case AHC_398X_SLOT_CHANNEL_B:
		*channel = 'B';
		break;
	case AHC_398X_SLOT_CHANNEL_C:
		*channel = 'C';
		break;
	default:
		printf("adapter at unexpected slot %d\n"
		       "unable to map to a channel\n",
		       dev->slot);
	}
	*flags |= AHC_LARGE_SEEPROM;
	return (0);
}



static void aic7xxx_attach(pcici_t tag, int unit) {
    printf("aic7xxx_attach: Found one, unit %d\n");
}

int aic7xxx_init(void) {
    int res;

    if (res = pci_register_lkm(&ahc_pci_driver, 0)) {
	printf("aic7xxx_init: pci_register_lkm failed (%d)", res);
	return -1;
    }
    
    return 0;
}

