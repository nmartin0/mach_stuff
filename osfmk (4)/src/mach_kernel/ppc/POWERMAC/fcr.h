/*
 * fcr.h - feature control register definitions
 *
 * Based upon information contained in a driver from LinuxPPC
 * originally written by Paul Mackerras
 */

/* offset from ohare base for feature control register */
#define FEATURE_OFFSET		0x38

/*
 * Bits in feature control register.
 * These were mostly derived by experiment on a powerbook 3400
 * and may differ for other machines.
 */
#define SCC_RESET		1
#define EXPBAY_RESET		2	/* a guess */
#define EXPBAY_PCI_ENABLE	4	/* a guess */
#define EXPBAY_IDE_ENABLE	8
#define EXPBAY_FLOPPY_ENABLE	0x10
#define IDE_ENABLE		0x20
#define EXPBAY_ENABLE		0x80
#define SCC_ENABLE		0x200
#define MESH_ENABLE		0x400
#define FLOPPY_ENABLE		0x800
#define SCCA_IO			0x2000
#define SCCB_IO			0x4000
#define VIA_ENABLE		0x10000
#define IDE_CD_POWER		0x800000


/*
 * Last feature in array
 */
#define LAST_FEATURE		18

/*
 * Bits to set in the feature control register on PowerBooks.
 */
#define POWERBOOK_DEFAULT	(IDE_ENABLE | SCC_ENABLE | \
				 MESH_ENABLE | SCCA_IO | SCCB_IO)

/*
 * StarMax requires a particular setting in its FCR to enable
 * IDE CD-ROM drives.  This is probably not an issue in MkLinux,
 * and is mainly left for historical reasons.
 *
 * Value contributed to LinuxPPC by Harry Eaton.
 */
#define STARMAX_FEATURES	0xbeff7a

