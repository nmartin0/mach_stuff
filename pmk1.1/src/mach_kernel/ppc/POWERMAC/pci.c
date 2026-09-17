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
 */
/*
 * Copyright 1996 1995 by Apple Computer, Inc.   
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * pmk1.1
 */

#include <platforms.h>
#include <ppc/proc_reg.h> /* For isync */
#include <mach_debug.h>
#include <kern/assert.h>
#include <kern/cpu_number.h>
#include <kern/spl.h>
#include <mach/std_types.h>
#include <types.h>
#include <chips/busses.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/powermac_pci.h>
#include <ppc/io_map_entries.h>
#include <ppc/POWERMAC/device_tree.h>
#include <ppc/POWERMAC/pci.h>


#define	PCI_BANDIT		(11)
#define	APPLE_BANDIT_ID		0x106b

#define	PCI_REG_VENDOR_ID	0x00
#define	PCI_REG_DEVICE_ID	0x02
#define	PCI_REG_COMMAND		0x04
#define	PCI_REG_STATUS		0x06
#define	PCI_REG_REVISION_ID	0x08
#define	PCI_REG_CLASS_CODE	0x9
#define	PCI_REG_CACHE_LINE	0xC
#define	PCI_REG_LATENCY_TIMER	0xD
#define	PCI_REG_HEADER_TYPE	0xE
#define	PCI_REG_BIST		0xF
#define	PCI_REG_BANDIT_CFG	0x40
#define	PCI_REG_ADDR_MASK	0x48
#define	PCI_REG_MODE_SELECT	0x50
#define	PCI_REG_ARBUS_HOLDOFF	0x58


#define	PCI_STAT_IOSPACE	0x001	/* I/O space enabled */
#define	PCI_STAT_MEMSPACE	0x002	/* Memory space enabled */
#define	PCI_STAT_BUS_MASTER	0x004	/* Bus Master */
#define	PCI_STAT_SPECIAL_CYCLES 0x008	/* Respond to Special Cycles */
#define PCI_STAT_CACHE_ENABLED	0x010	/* Bandit is I/O coherent */
#define	PCI_STAT_VGA_SNOOP	0x020	/* .. not needed by Bandit */
#define	PCI_STAT_PARITY_ERR	0x040	/* Parity Error Response (not implemented) */
#define	PCI_STAT_WAIT_CYCLE	0x080	/* Not implemented*/
#define	PCI_STAT_SERR_ENABLE	0x100	/* System Error Pin implemented */
#define	PCI_STAT_FAST_BACK	0x200	/* Fast back to back cycles (not implemented) */

#define	PCI_MS_BYTESWAP		0x001	/* Enable Big Endian mode. (R/W)*/
#define	PCI_MS_PASSATOMIC	0x002	/* PCI Bus to ARBus Lock are always allowed (R)*/
#define	PCI_MS_NUMBER_MASK	0x00C	/* PCI Bus Number (R) */
#define	PCI_MS_IS_SYNC		0x010	/* Is Synchronous (1) or Async (0) ? (R)*/
#define	PCI_MS_VGA_SPACE	0x020	/* Map VGA I/O space  (R/W) */
#define	PCI_MS_IO_COHERENT	0x040	/* I/O Coherent (R/W) */
#define	PCI_MS_INT_ENABLE	0x080	/* Allow TEA or PCI Abort INT to pass to Grand Central (R/W) */

struct pci_bridge {
	int			bus_start;
	int			bus_end;
	unsigned long		config_addr;
	unsigned long		config_data;
	unsigned long		status;
	unsigned long		vendor_id; 
	device_node_t		*device;
	struct pci_bridge	*next;
};

typedef struct pci_bridge pci_bridge_t;

static char *pci_bridge_names[] = {
	"bandit",
	"chaos",
	NULL
};

static int	pci_bridge_count;
static pci_bridge_t	**pci_bridges, *pci_bridge_list = NULL;

static void scan_bridge(char *, pci_bridge_t **, unsigned int *);

static __inline__ void
store_le32(volatile unsigned long a, unsigned long x)
{       
	__asm__ volatile ("stwbrx %0,0,%1" : : "r" (x), "r" (a) : "memory");
	eieio();
	return;
}

static __inline__ unsigned long
read_le32(volatile unsigned long a)
{
	unsigned long swap;     

	eieio();
	__asm__ volatile ("lwbrx %0,0,%1" :  "=r" (swap) : "r" (a));
	return  swap;
}               

void
powermac_scan_bridges(unsigned int *first_avail)
{
	pci_bridge_t	*bridge = NULL;
	int	i;

	for (i = 0; pci_bridge_names[i]; i++)
		scan_bridge(pci_bridge_names[i], &bridge, first_avail);

	if (pci_bridge_list == NULL)
		return;

	pci_bridge_count++;

	pci_bridges = (pci_bridge_t **) *first_avail;
	*first_avail += sizeof(pci_bridge_t **) * pci_bridge_count;

	for  (i = 0; i < pci_bridge_count; i++)
		pci_bridges[i] = 0;

	for (bridge = pci_bridge_list; bridge; bridge = bridge->next)
		for (i = bridge->bus_start; i <= bridge->bus_end; i++)
			pci_bridges[i] = bridge;
}

static void
scan_bridge(char *name, pci_bridge_t **prev, unsigned int *first_avail)
{
	device_node_t	*bridge_dev;
	pci_bridge_t	*br;
	unsigned long		*range, length;
	
	bridge_dev = find_devices(name);

	while (bridge_dev) {
		range  = (unsigned long *) get_property(bridge_dev, "bus-range", &length);
		if (range == NULL)
			continue;

		br = (pci_bridge_t *) *first_avail;
		*first_avail += sizeof(*br);
		
		if (*prev) 
			(*prev)->next = br;

		*prev = br;


		if (pci_bridge_list == NULL)
			pci_bridge_list = br;

		br->next = NULL;
		br->bus_start = range[0];
		br->bus_end = range[1];
		br->device = bridge_dev;
		br->config_addr = bridge_dev->addrs[0].address + 0x800000;
		br->config_data = bridge_dev->addrs[0].address + 0xc00000;

		br->vendor_id = 0xdeadbeef;

		if (br->bus_end > pci_bridge_count)
			pci_bridge_count = br->bus_end;

		bridge_dev = bridge_dev->next;

	}
}

static __inline__ void
pci_set_config_reg(unsigned long addr, unsigned long value)
{
	store_le32(addr, value);
	delay(2);
}


boolean_t
powermac_is_coherent(void)
{
	pci_bridge_t	*bridge;
	unsigned long	cmd, data;
	boolean_t	is_coherent = TRUE;

	if (powermac_info.class != POWERMAC_CLASS_PCI)  
		return FALSE;

	for (bridge = pci_bridge_list ; bridge; bridge = bridge->next) {
		/* Next.. try to see if this is a I/O coherent system */

		pci_set_config_reg(bridge->config_addr,
				(1 << PCI_BANDIT) | PCI_REG_VENDOR_ID);

		data = read_le32(bridge->config_data);

		bridge->vendor_id = data & 0xffff;

		if (bridge->vendor_id != APPLE_BANDIT_ID)  
			continue;
		
		pci_set_config_reg(bridge->config_addr, (1 << PCI_BANDIT) | PCI_REG_MODE_SELECT);
		bridge->status = read_le32(bridge->config_data);

		if ((bridge->status & PCI_MS_IO_COHERENT) == 0)  {
			bridge->status |= PCI_MS_IO_COHERENT;
			store_le32(bridge->config_data, bridge->status);
		}
	}

	if (find_devices("ohare"))
		is_coherent = FALSE;

	return is_coherent;
}

void
powermac_display_pci_busses(void)
{
	pci_bridge_t	*bus;
	int	 i;

	for (i = 0; i < pci_bridge_count; i++, bus++) {
		bus = pci_bridges[i];

		if (bus == NULL)
			continue;

		printf("PCI Bridge %s at 0x%x: covers PCI bus %d",
			bus->device->name,
			bus->device->addrs[0].address, bus->bus_start);

		if (bus->bus_start != bus->bus_end)
			printf("to %d", bus->bus_end);

		if (bus->vendor_id != APPLE_BANDIT_ID) 
			printf(" (NON APPLE PCI BRIDGE! id = %x)",
				bus->vendor_id);
		else
			printf(" Status = %x", bus->status);

		printf("\n");
	}
}
