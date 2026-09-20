/*
 * Portions Copyright (c) 1999 The MkLinux Project.
 *              All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice and disclaimer
 * appear in all copies of the software, derivative works or modified
 * versions, and any portions thereof, and that these notices also
 * appear in supporting documentation, and provided that all advertising
 * materials mentioning features or use of this software displays the
 * following acknowledgement:
 * 
 *	This product includes software developed by The MkLinux Project
 *	and its contributors.
 * 
 * Neither the name of The MkLinux Project nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND THE MKLINUX PROJECT HEREBY DISCLAIMS ALL
 * SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT."
 *  
 * IN NO EVENT SHALL THE MKLINUX PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN ACTION OF CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF OR IN CONNECTION WITH THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fcr.h>
#if	NFCR > 0

#include <platforms.h>
#include <ppc/proc_reg.h> /* For isync */
#include <mach_debug.h>
#include <kern/spl.h>
#include <mach/std_types.h>
#include <types.h>
#include <chips/busses.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_gestalt.h>
#include <ppc/POWERMAC/device_tree.h>

/* Used only for debugging asserts */
#include <ppc/mem.h>
#include <ppc/pmap.h>
#include <ppc/POWERMAC/interrupts.h>

#include <ppc/POWERMAC/io.h>
#include <ppc/POWERMAC/fcr.h>

/*
 * Internal forward declarations.
 */
void feature_init(void);
int feature_test(struct device_node* device, int f);
int feature_set(struct device_node* device, int f);
int feature_clear(struct device_node* device, int f);

static long feature_bits_ohare[] = {
	0,			/* FEATURE_null */
	SCC_RESET,		/* FEATURE_Serial_reset */
	SCC_ENABLE,		/* FEATURE_Serial_enable */
	SCCA_IO,		/* FEATURE_Serial_IO_A */
	SCCB_IO,		/* FEATURE_Serial_IO_B */
	FLOPPY_ENABLE,		/* FEATURE_SWIM3_enable */
	MESH_ENABLE,		/* FEATURE_MESH_enable */
	IDE_ENABLE,		/* FEATURE_IDE_enable */
	VIA_ENABLE,		/* FEATURE_VIA_enable */
	IDE_CD_POWER,		/* FEATURE_CD_power */
	EXPBAY_RESET,		/* FEATURE_Mediabay_reset */
	EXPBAY_ENABLE,		/* FEATURE_Mediabay_enable */
	EXPBAY_PCI_ENABLE,	/* FEATURE_Mediabay_PCI_enable */
	EXPBAY_IDE_ENABLE,	/* FEATURE_Mediabay_IDE_enable */
	EXPBAY_FLOPPY_ENABLE,	/* FEATURE_Mediabay_floppy_enable */
	0,			/* FEATURE_Ethernet_reset */
	0,			/* FEATURE_Ethernet_IO_enable */
	0,			/* FEATURE_Modem_PowerOn -> guess...*/
	0			/* FEATURE_Modem_Reset -> guess...*/
};

/* assume these are the same as the ohare until proven otherwise */
static long feature_bits_heathrow[] = {
	0,			/* FEATURE_null */
	SCC_RESET,		/* FEATURE_Serial_reset */
	SCC_ENABLE,		/* FEATURE_Serial_enable */
	SCCA_IO,		/* FEATURE_Serial_IO_A */
	SCCB_IO,		/* FEATURE_Serial_IO_B */
	FLOPPY_ENABLE,		/* FEATURE_SWIM3_enable */
	MESH_ENABLE,		/* FEATURE_MESH_enable */
	IDE_ENABLE,		/* FEATURE_IDE_enable */
	VIA_ENABLE,		/* FEATURE_VIA_enable */
	IDE_CD_POWER,		/* FEATURE_CD_power */
	EXPBAY_RESET,		/* FEATURE_Mediabay_reset */
	EXPBAY_ENABLE,		/* FEATURE_Mediabay_enable */
	EXPBAY_PCI_ENABLE,	/* FEATURE_Mediabay_PCI_enable */
	EXPBAY_IDE_ENABLE,	/* FEATURE_Mediabay_IDE_enable */
	EXPBAY_FLOPPY_ENABLE,	/* FEATURE_Mediabay_floppy_enable */
	0x80000000,		/* FEATURE_BMac_reset */
	0x60000000,		/* FEATURE_BMac_IO_enable */
	0x02000000,		/* FEATURE_Modem_PowerOn -> guess...*/
	0x07000000		/* FEATURE_Modem_Reset -> guess...*/
};

long *controller_bits[5] = {
	0,					/* PDM */
	0,					/* OHARE */
	(long *)(&feature_bits_ohare),		/* OHARE */
	(long *)(&feature_bits_heathrow),	/* HEATHROW */
	(long *)(&feature_bits_heathrow),	/* GATWICK */
	};

/* static variables */
int controller_type = -1;
struct device_node *primary_controller_dev;

#define FCR_PDM 0
#define FCR_OHARE 1
#define FCR_OHARE_PB 2
#define FCR_HEATHROW 3
#define FCR_GATWICK 4   // really a heathrow/gatwick pair
#define FCR_UNKNOWN -1

void
feature_init(void)
{
	struct device_node *dev;

	switch(powermac_info.class) {
	   case POWERMAC_CLASS_PDM:
	   case POWERMAC_CLASS_PERFORMA:
	   case POWERMAC_CLASS_POWERBOOK:
		controller_type = FCR_PDM;
		primary_controller_dev = NULL;
		break;
	   case POWERMAC_CLASS_PCI:
	   default:
		if (dev = find_devices("mac-io")) {
			if (dev->next) controller_type = FCR_GATWICK;
			else controller_type = FCR_HEATHROW;
			primary_controller_dev = dev;
		} else {
			if (dev = find_devices("ohare")) {
				if (find_devices("via-pmu"))
					controller_type = FCR_OHARE_PB;
				else controller_type = FCR_OHARE;
				primary_controller_dev = dev;
			} else {
				controller_type = FCR_UNKNOWN;
				primary_controller_dev = NULL;
			}
		}
	}
}

static int
feature_lookup_controller(struct device_node *device)
{
	// Saner than LinuxPPC's approach -- if the device doesn't
	// have the main i/o controller in its path and there's a
	// second one available, that must be in its path by default,
	// else either something's wrong in hardware or the person
	// coding some device driver is an idiot.  If there's no
	// secondary controller, then either there's something wrong
	// in OpenFirmware or the person coding some device driver is
	// an idiot for not checking OF.  Either way, no point in
	// worrying about it.  ;-)
	
	if (device == NULL)
		return -1;

	while(device)
	{
		if (device == primary_controller_dev) return 0;
		device = device->parent;
	}
	if (primary_controller_dev->next) return 1;

	return -1;
}

int
feature_set(struct device_node* device, int f)
{
	int		controller;
// 	unsigned long	flags;

	if (f > LAST_FEATURE)
		return -1;

	if (controller_type == FCR_PDM) {
		return -1;
	}

	if ((controller = feature_lookup_controller(device)) == -1) return -1;

#if 0
	save_flags(flags);
	cli();
#endif
	if (controller) outl_le( io_base_virt2,
		 inl_le(io_base_virt2) |
		 ((long *)(*controller_bits[controller_type]))[f]);
	else outl_le( powermac_info.io_base_virt,
		 inl_le(powermac_info.io_base_virt) |
		 ((long *)(*controller_bits[controller_type]))[f]);
#if 0
	restore_flags(flags);
#endif
	delay(10);

	return 0;
}

int
feature_clear(struct device_node* device, int f)
{
	int		controller;
	unsigned long	flags;

	if (f > LAST_FEATURE)
		return -1;

	if (controller_type == FCR_PDM) {
		return -1;
	}

	if ((controller = feature_lookup_controller(device)) == -1) return -1;
	
#if 0
	save_flags(flags);
	cli();
#endif
	if (controller) outl_le( io_base_virt2,
		 inl_le(io_base_virt2) &
		 ~(((long *)(*controller_bits[controller_type]))[f]));
	else outl_le( powermac_info.io_base_virt,
		 inl_le(powermac_info.io_base_virt) &
		 ~(((long *)(*controller_bits[controller_type]))[f]));
#if 0
	restore_flags(flags);
	udelay(10);
#endif
	
	return 0;
}

int
feature_test(struct device_node* device, int f)
{
	int		controller;

	if (f > LAST_FEATURE)
		return -1;

	if (controller_type == FCR_PDM) {
		return -1;
	}

	if ((controller = feature_lookup_controller(device)) == -1) return -1;

	if (controller) return (inl_le(io_base_virt2) &
		((long *)(*controller_bits[controller_type]))[f]) != 0;

	return (inl_le(powermac_info.io_base_virt) &
		((long *)(*controller_bits[controller_type]))[f]) != 0;
}

#endif // NFCR
