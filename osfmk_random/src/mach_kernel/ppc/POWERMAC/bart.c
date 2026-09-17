/************************************************************************
 * bart.c                                                               *
 *    This file is the beginning of a handler for NuBus cards attached  *
 *    to a BART controller.  The eventual intent, if possible,  is to   *
 *    make them appear to be a special case of a PCI bridge, so that    *
 *    drivers for boards that exist as both NuBus and PCI will require  *
 *    minimal modification.  This, however, will not be done in the     *
 *    initial testing stages....                                        *
 ************************************************************************/

#include "bart.h"
#include "conf.h"
#if	NBART > 0

#include <cpus.h>
#include <chained_ios.h>
#include <device/param.h>

#include <string.h>
#include <types.h>
#include <device/buf.h>
#include <device/conf.h>
#include <device/errno.h>
#include <device/subrs.h>
#include <device/misc_protos.h>
#include <device/ds_routines.h>
#include <device/param.h>
#include <device/driver_lock.h>
#include <sys/ioctl.h>
#include <kern/spl.h>
#include <kern/misc_protos.h>
#include <chips/busses.h>
#include <ppc/proc_reg.h>
#include <ppc/POWERMAC/io.h>
#include "pcivar.h"
#include "nubusvar.h"
#include "nubus_control.h"
#include <vm/vm_kern.h>
#include <ppc/proc_reg.h>
#include <ppc/io_map_entries.h>
#include <ppc/POWERMAC/powermac.h>
#include <ppc/POWERMAC/interrupts.h>
#include <ppc/POWERMAC/device_tree.h>
#include <ppc/POWERMAC/powermac_m2.h>
#include <ppc/POWERMAC/powermac_performa.h>
#include <ppc/POWERMAC/powermac_gestalt.h>
#include <ppc/misc_protos.h>

/* Machine-independent BART definitions */

#define BART_DEBUG 1    // TODO DAG: Change when development done

#define BART_MAGIC	0x43184000
#define VALID_NUBUS_SLOT(slot) ((slot >= 0x9) && (slot <= 0xe))

#if MACH_DEBUG
#undef BART_DEBUG
#define BART_DEBUG 1
#endif


/* Function Prototypes */

void pdm_display_model_code(void);
static boolean_t        bart_probe(
                                int                     port,
                                struct bus_device	*bus);
static void             bart_attach(
                                struct bus_device       *bus);

static void		bart_set_slot_disable(unsigned short slot,
				boolean_t dis);
static boolean_t	bart_get_slot_disable(unsigned short slot);
static void		bart_set_speed(boolean_t slow);
static boolean_t	bart_get_speed(void);
static boolean_t	bart_set_burst_enable(unsigned short slot,
				boolean_t ena);
void			bart_set_burst_enable_mod(unsigned short slot,
				boolean_t ena);
static boolean_t	bart_get_burst_enable(unsigned short slot);
static void		bart_reset(void);
void			bart_register_nubus(void);

/* The next two go. */
int		bart_intr(void);
void		bart_probe_slot(unsigned short slot,
				nubus_device *dev);


unsigned char		*getenv(char *);


// __inline__ int valid_slot(unsigned short slot) {
// 	if ((slot < 0x9) || (slot > 0xe)) return false;
// 	else return true;
// 	}


caddr_t bart_std[NBART] = { 0 };
struct  bus_device      *bart_info[NBART*16];
int bart_revision = 0x000;
// struct  bus_ctlr        *bart_info[NBART];

struct  bus_driver      bart_driver = {
        (probe_t)bart_probe, 0, bart_attach, 0, bart_std, "bart", bart_info
	};

nubus_device nubus_slots[5]; // 0x9 - 0xe
decl_simple_lock_data(, bart_lock)
vm_offset_t bart_base_virt, bart_base_phys;

/* for POWERMAC_IO3 */
#define nubus_controller_base_virt bart_base_virt
#define nubus_controller_base_phys bart_base_phys


/*****************************************************************************
 * bart_probe
 *      Checks to see if BART chip exists in machine.
 *
 *****************************************************************************/

static boolean_t
bart_probe(int port, struct bus_device *bus)
{
   u_int32_t *idptr;
   unsigned char *envp;
   jmp_buf_t faultbuf;


   simple_lock_init(&bart_lock, ETAP_IO_CHIP);

   if (_setjmp(&faultbuf)) {
	/* If we get here, then the idptr read caused
	   an explosion */
	memset((void *)&faultbuf, 0, sizeof(faultbuf));
	return(0);
   }

   pmap_add_physical_memory(PDM_BART_BASE_PHYS,
				(PDM_BART_BASE_PHYS + PDM_BART_SIZE),
				FALSE,
				PTE_WIMG_IO);
   bart_base_virt=io_map(PDM_BART_BASE_PHYS, PDM_BART_SIZE);
   bart_base_phys=PDM_BART_BASE_PHYS;

   /* For now, it'll be PDMs only. */
   switch(powermac_info.class) {
	case POWERMAC_CLASS_PDM:
		idptr = (u_int32_t *)(POWERMAC_IO3(PDM_BART_BASE_PHYS +
			BART_ID_OFFSET));
		break;
	default:
		idptr = (u_int32_t *)(0);
	}

   if (!idptr) return 0;

   // pdm_display_model_code();

   envp = getenv("probe_nubus");
   if (!envp) return 0;
   if (!atoi(envp)) return 0;

   if ((*idptr & 0xFFFFF000) != BART_MAGIC) {
	printf("WARNING: No BART controller found!\n");
	printf("idptr = 0x%x, *idptr = 0x%x\n", idptr, *idptr);
	return 0;
   } else {
	printf("BART controller at 0x%x, chip revision 0x%x\n",
		(idptr-BART_ID_OFFSET), (*idptr & 0x00000FFF));
	printf("No additional support is completed yet.\n");
	bart_revision = (*idptr & 0x00000FFF);

	bart_register_nubus();
	return 0; // returning 1 causes a crash... odd.
   }
}


/*****************************************************************************
 * bart_attach
 *      Build list of NuBus cards connected to BART chip.
 *
 *****************************************************************************/

static void bart_attach(struct bus_device *bus)
{
int slot;

/* this stuff happens in nubus_attach now */


return;

#if 0

   /* Probe for devices in slots */

#if BART_DEBUG
   printf("BART attach called.\n");
#endif

   simple_lock(&bart_lock);
#if BART_DEBUG
   printf("exclusive lock obtained.\n");
#endif


   for (slot = 0x9 ; slot <= 0xe ; slot++) {
	nubus_slots[(slot - 9)].flags = 0;
	bart_probe_slot(slot, &nubus_slots[(slot - 9)]);
   }

#if BART_DEBUG
   printf("unlocking.\n");
#endif
   simple_unlock(&bart_lock);
#if BART_DEBUG
   printf("Returning.\n");
#endif
   return;

#endif // 0
}


/*****************************************************************************
 * bart_intr
 *     Put in for formality only.  This function is as good as gone.
 *
 *****************************************************************************/

int bart_intr(void)
{
   /* Sporadic interrupt from some card we don't support */

#if BART_DEBUG
   printf("BART: received sporadic interrupt.\n");
#endif
   return 0;
}


/*****************************************************************************
 * bart_set_slot_disable
 *      Sets or clears the disable bit for a given NuBus slot.
 *      dis = 0 : enable slot
 *      dis = 1 : disable slot
 *
 *****************************************************************************/

static void	bart_set_slot_disable(unsigned short slot, boolean_t dis)
{
u_int8_t *slotptr;

   if (!VALID_NUBUS_SLOT(slot)) {
#if BART_DEBUG
	printf("BART: Attempt to set disable for invalid slot (%d)\n", slot);
#endif
	return;
   }

   simple_lock(&bart_lock);

   slotptr = (u_int8_t *)POWERMAC_IO3(PDM_BART_BASE_PHYS + 0x10 + (8 * (0xe - slot)) + 1);
   *slotptr = dis << 7; eieio(); // &0x80
   if (dis)
	nubus_slots[(slot - 9)].flags |= NUBUS_SLOT_DISABLE;
   else
	nubus_slots[(slot - 9)].flags &= ~NUBUS_SLOT_DISABLE;

   simple_unlock(&bart_lock);
}


/*****************************************************************************
 * bart_get_slot_disable
 *      Returns TRUE (1) if slot disabled, else FALSE (0)
 *
 *****************************************************************************/
static boolean_t	bart_get_slot_disable(unsigned short slot)
{
u_int8_t *slotptr;
boolean_t val;

   if (!VALID_NUBUS_SLOT(slot)) {
#if BART_DEBUG
	printf("BART: Attempt to test disable for invalid slot (%d)\n", slot);
#endif
	return 0;
   }

   simple_lock(&bart_lock);
#if 0
   slotptr = (u_int8_t *)POWERMAC_IO3(PDM_BART_BASE_PHYS + 0x10 + (8 * (0xe - slot)) + 1);
   val = ((*slotptr & 0x80) != 0);
#else
   val = ((nubus_slots[(slot - 9)].flags & NUBUS_SLOT_DISABLE) != 0);
#endif
   simple_unlock(&bart_lock);
   return val;
}


/*****************************************************************************
 * bart_set_speed
 *      Sets or clears the "slow" bit for the BART controller.  This bit
 *      should be left set, with the possible exception of 8100/100 and
 *      8100/110 machines and possibly upgraded slower machines.  It is
 *      not possible to determine what effect clearing this bit would have
 *      without extensive testing.
 *
 *****************************************************************************/

static void		bart_set_speed(boolean_t slow)
{
   u_int8_t *slotptr;

   simple_lock(&bart_lock);
   slotptr = (u_int8_t *)POWERMAC_IO3(PDM_BART_BASE_PHYS + 0x01);
   *slotptr = slow << 7; eieio(); // toggling the high bit
   simple_unlock(&bart_lock);
}



/*****************************************************************************
 * bart_get_speed
 *      Returns TRUE (1) if slow bit set, else FALSE (0)
 *
 *****************************************************************************/

static boolean_t	bart_get_speed(void)
{
   u_int8_t *slotptr;
   boolean_t val;

   simple_lock(&bart_lock);
   slotptr = (u_int8_t *)POWERMAC_IO3(PDM_BART_BASE_PHYS + 0x01);

   val = ((*slotptr & 0x80) != 0);
   simple_unlock(&bart_lock);
   return val;
}


/*****************************************************************************
 * bart_set_burst_enable_mod
 *      used internally by bart_set_burst_enable
 *
 *****************************************************************************/

void		bart_set_burst_enable_mod(unsigned short slot, boolean_t ena)
{
u_int8_t *slotptr;

   if (!VALID_NUBUS_SLOT(slot)) {
#if BART_DEBUG
	printf("BART: Attempt to set burst for invalid slot (%d)\n", slot);
#endif
	return;
   }

   simple_lock(&bart_lock);
   slotptr = (u_int8_t *)POWERMAC_IO3(PDM_BART_BASE_PHYS + 0x10 +
                                      (8 * (0xe - slot)));
   *slotptr = ena << 7; eieio(); // &0x80
   if (ena)
	nubus_slots[(slot - 9)].flags |= NUBUS_BURST_ENABLE;
   else
	nubus_slots[(slot - 9)].flags &= ~NUBUS_BURST_ENABLE;
   simple_unlock(&bart_lock);
}


/*****************************************************************************
 * bart_set_burst_enable
 *      Sets or clears burst enable bit for a given slot.  Some older BART
 *      chips cannot handle burst being enabled for only one slot.  Thus,
 *      if any device cannot handle bursting, bursting is not enabled for
 *      any slot in these machines.  For machines that can handle it, 
 *      burseting is handled on a per-slot basis, in which case, this fails
 *      only if an individual card in the specified slot cannot handle burst
 *      mode transactions
 *
 *****************************************************************************/

static boolean_t	bart_set_burst_enable(unsigned short slot, boolean_t ena)
{
   int retval = -1;

   simple_lock(&bart_lock);
   if (bart_revision < 3) {
	int i;

	for (i=0x9 ; i<=0xe ; i++)
	   if ((!ena) || (nubus_slots[(i - 9)].flags & NUBUS_SUPPORTS_BURST))
	   	bart_set_burst_enable_mod(i, ena);
	   else {
		/* Sorry, on revision < 3, if any slot does not support
		   bursting, it cannot safely be enabled */
		bart_set_burst_enable(i, 0);
		retval = 0;
		break;
	   }
   } else if (nubus_slots[(slot - 9)].flags & NUBUS_SUPPORTS_BURST) {
	   bart_set_burst_enable_mod(slot, ena);
	   simple_unlock(&bart_lock);
	   retval = 1;
   } else {
	simple_unlock(&bart_lock);
	retval = 0;
   }
   simple_unlock(&bart_lock);
   if (retval == -1) // success for bart_revision < 3
	retval = 1;

   return retval;
}


/*****************************************************************************
 * bart_get_burst_enable
 *      Returns TRUE(1) if bursting enabled, else FALSE (0)
 *
 *****************************************************************************/

static boolean_t	bart_get_burst_enable(unsigned short slot)
{
u_int8_t *slotptr;
boolean_t val;

   if (!VALID_NUBUS_SLOT(slot)) {
#if BART_DEBUG
	printf("BART: Attempt to test burst for invalid slot (%d)\n", slot);
#endif
	return 0;
   }

   simple_lock(&bart_lock);
#if 0
   slotptr = (u_int8_t *)POWERMAC_IO3(PDM_BART_BASE_PHYS + 0x10 + (8 * (0xe - slot)));
   val = ((*slotptr & 0x80) != 0);
#else
   val = ((nubus_slots[(slot - 9)].flags & NUBUS_BURST_ENABLE) != 0);
#endif
   simple_unlock(&bart_lock);
   return val;
}


/*****************************************************************************
 * bart_reset
 *      Resets NuBus devices to their default power-on/reboot state.
 *
 *****************************************************************************/

static void		bart_reset(void)
{
u_int8_t *slotptr;

   simple_lock(&bart_lock);
   slotptr = (u_int8_t *)POWERMAC_IO3(PDM_BART_BASE_PHYS);
   *slotptr = 0x80; 
   while (*slotptr) eieio();
   simple_unlock(&bart_lock);
}


/*****************************************************************************
 * bart_probe_slot
 *      Probes for NuBus devices in slot
 *
 *      THIS IS JUST A STUB!!!!!!!
 *
 *****************************************************************************/

void bart_probe_slot(unsigned short slot, nubus_device *dev)
{
vm_offset_t slotbase;

   /* Debug messages, check for valid args, set slotbase */

   if ((slot < 0x9) || (slot > 0xe)) {
#if BART_DEBUG
   	printf("WARNING: bart_probe_slot called for invalid slot\n");
	printf("slot = %d, dev = 0x%x\n", slot, dev);
#endif
	return;
   } else {
#if BART_DEBUG
	printf("bart_probe_slot called for slot %d, dev = 0x%x\n", slot, dev);
#endif
   }

   // slot base is 0xF9000000..FE000000, by slot number.
   slotbase = (vm_offset_t)(0xF0000000 + ((vm_offset_t)(slot) << 24));



/*** @@@ TODO DAG @@@ ***/

/* Is there a device at that slot?  If not, return. */

/* Set NUBUS_SUPPORTS_BURST flag for burst support if supported */

/* Set NUBUS_HAS_SUPERSLOT flag for cards that use addresses in the
   SuperSlot range */

/* Now copy info for slot into dev->whatever */

   // dev->flags |= NUBUS_SLOT_FILLED; // no point....
   return;
}

#define PDM_ID_ADDRESS 0x5FFFFFFC
void pdm_display_model_code(void) {
   u_int32_t *idptr;
   
	printf("Adding physical memory for ID Register\n");
	pmap_add_physical_memory(PDM_ID_ADDRESS,
				(PDM_ID_ADDRESS + sizeof(u_int32_t)),
			        FALSE, PTE_WIMG_IO);

	printf("Mapping physical memory for ID Register\n");
	idptr = (u_int32_t *)io_map(PDM_ID_ADDRESS, sizeof(u_int32_t));
	// idptr = (u_int32_t *)(POWERMAC_IO(PDM_ID_ADDRESS));

	printf("Reading PDM ID Value: ");
	printf("0x%x\n", *idptr);
	return;
}

void bart_register_nubus(void)
{
    /* build up a nubus_controller_t structure */

    nubus_controller_t controller;

    /* Shouldn't run out of memory, but.... */
    if (!(controller=(nubus_controller_t)io_map(0, sizeof(*controller)))) {
	printf("bart_register_contoller(): out of memory!\n");
	return;
    }

    controller->name = "bart";
    controller->bottom_slot = 0x9;
    controller->top_slot = 0xF;

    controller->set_slot_disable =
	bart_set_slot_disable;

    controller->get_slot_disable = 
	bart_get_slot_disable;

    controller->set_nubus_speed =
	bart_set_speed;

    controller->get_nubus_speed =
	bart_get_speed;

    controller->set_burst_enable =
	bart_set_burst_enable;

    controller->get_burst_enable =
	bart_get_burst_enable;

    controller->reset = bart_reset;


    nubus_register_controller(controller);

}

#endif /* NBART > 0 */

