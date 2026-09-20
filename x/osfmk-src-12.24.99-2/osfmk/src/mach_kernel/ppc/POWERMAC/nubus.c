/* FIX ME!  Please! */
#define UNCONF 0

/*	$NetBSD: nubus.c,v 1.47 1999/08/23 22:29:39 thorpej Exp $	*/
/*
 * Copyright (c) 1995, 1996 Allen Briggs.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Allen Briggs.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __netbsd__


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/buf.h>
#include <sys/conf.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>

#include <machine/autoconf.h>
#include <machine/bus.h>
#include <machine/vmparam.h>
#include <machine/param.h>
#include <machine/cpu.h>
#include <machine/pte.h>
#include <machine/viareg.h>

#include <vm/vm.h>

#include <mac68k/nubus/nubus.h>
#define NNUBUS 1

#else
/* MkLinux */

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
#include <ppc/POWERMAC/pcivar.h>
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
#include <ppc/POWERMAC/io.h>
#include <ppc/setjmp.h>
#include <ppc/POWERMAC/nubuspri.h>
#include <ppc/POWERMAC/nubusvar.h>
#include "cdefs.h"
#include <nubus.h>
#include "nubus_control.h"

#endif

// #undef NUBUS_MIN_SLOT
// #undef NUBUS_MAX_SLOT

#if NNUBUS > 0

#ifdef DEBUG
#define NDB_PROBE	0x1
#define NDB_FOLLOW	0x2
#define NDB_ARITH	0x4
static int	nubus_debug = 0 /* | NDB_PROBE */;
#endif

#undef DEBUG

static int	nubus_print __P((void *, const char *));
static int	nubus_probe __P((caddr_t addr, void *garbage));
static void	nubus_attach __P((struct bus_device *));
static int	nubus_video_resource __P((int));

static int	nubus_probe_slot (vm_offset_t, int, nubus_slot *);
static u_int32_t nubus_calc_CRC (nubus_slot *);

static u_long	nubus_adjust_ptr __P((u_int8_t, u_long, long));
static u_int8_t	nubus_read_1 __P((u_int8_t, u_long));
#ifdef notyet
static u_int16_t nubus_read_2 __P((u_int8_t, u_long));
#endif
static u_int32_t nubus_read_4 __P((u_int8_t, u_long));

boolean_t	find_nubus_controller_type(char *name);

#if 0
struct cfattach nubus_ca = {
	sizeof(struct nubus_softc), nubus_match, nubus_attach
};
#else
struct	bus_device *dinfo[NNUBUS];
struct  bus_driver      nubus_driver =
	{ (probe_t)nubus_probe, 0, nubus_attach, 0,
	   NULL, "nubus", dinfo, 0, 0, 0
	};
#endif

char *nubus_controller_names[] = {
	"bart",
	NULL
};

nubus_controller_t nubus_controller_list[NNUBUS];
int number_of_nubus_controllers=0;
vm_offset_t nubus_slots_base_virt;

nubus_device nubus_slots[16];

static int nubus_probe(caddr_t addr, void *garbage)
{
    /* who cares about the args -- we don't use 'em anyway */
    int i, j;

    for (i=0; i<number_of_nubus_controllers; i++) {
	if (!find_nubus_controller_type(nubus_controller_list[i]->name)) {
	    printf("Nubus: ignoring unknown controller type %s\n",
		nubus_controller_list[i]->name);
	    /* Crap!  io_map can't be undone.... */
	    /* kfree(nubus_controller_list[i]); */
	    for (j=i; j<(number_of_nubus_controllers-1); j++)
	    	nubus_controller_list[i] = nubus_controller_list[(i+1)];
	    number_of_nubus_controllers--;
	}
    }
    return (number_of_nubus_controllers > 0);
}

static void
nubus_attach(struct bus_device *self)
{
    nubus_slot fmtblock;
    nubus_dir dir;
    nubus_dirent dirent;
    nubus_type slottype;
    u_long entry;
    int ctrlr, i, rsrcid;
    u_int8_t lanes;

    printf("\n");

    for (i=0;i<16;i++) nubus_slots[i].flags = 0;

    for (ctrlr=0; ctrlr<number_of_nubus_controllers; ctrlr++) {
	// int NUBUS_MIN_SLOT, NUBUS_MAX_SLOT;

	if (!ctrlr) {
        	nubus_slots_base_virt = POWERMAC_IO2(PDM_NUBUS_BASE_PHYS);
		// (caddr_t)io_map(0xF9000000, 0x06000000);
        	if (!nubus_slots_base_virt) {
			printf("nubus:  Cannot map in slot space!\n");
			number_of_nubus_controllers = 0;
			return;
		}
	}

	printf("Nubus: attaching  to controller type %s\n",
	    nubus_controller_list[ctrlr]->name);
	// NUBUS_MIN_SLOT=nubus_controller_list[ctrlr]->bottom_slot;
	// NUBUS_MAX_SLOT=nubus_controller_list[ctrlr]->top_slot;

	for (i = NUBUS_MIN_SLOT; i <= NUBUS_MAX_SLOT; i++) {
		vm_offset_t location;

		location = (vm_offset_t)NUBUS_IO(NUBUS_SLOT2PA(i));

		if (nubus_probe_slot(location, i, &fmtblock) <= 0) {
notfound:
			/* io_map() can't be undone on PPC. */
			/* bus_space_unmap(NBMEMSIZE); */
			continue;
		}
		printf("nubus_probe_slot returned\n");
  
		rsrcid = 0x80;
		lanes = fmtblock.bytelanes;

		nubus_get_main_dir(&fmtblock, &dir);

		/*
		 * Get the resource for the first function on the card.
		 * This is assumed to be at resource ID 0x80.  If we can
		 * not find this entry (as we can not on some video cards),
		 * check to see if we can get a different ID from the list
		 * of video resources given to us by the booter.  If that
		 * doesn't work either, take the first resource following
		 * the board resource.
		 */
		if (nubus_find_rsrc(
		    &fmtblock, &dir, rsrcid, &dirent) <= 0) {
			if ((rsrcid = nubus_video_resource(i)) == -1) {
				/*
				 * Since nubus_find_rsrc failed, the directory
				 * is back at its base.
				 */
				entry = dir.curr_ent;

				/*
				 * All nubus cards should have a board
				 * resource, but be sure that's what it
				 * is before we skip it.
				 */
				rsrcid = nubus_read_1(
				    lanes, entry);
				if (rsrcid == 0x1)
					entry = nubus_adjust_ptr(lanes,
					    dir.curr_ent, 4);

				rsrcid = nubus_read_1(lanes, entry);
#ifdef DEBUG
				if (nubus_debug & NDB_FOLLOW)
					printf("\tUsing rsrc 0x%x.\n", rsrcid);
#endif
				if (rsrcid == 0xff)	/* end of chain */
					goto notfound;
			}
			/*
			 * Try to find the resource passed by the booter
			 * or the one we just tracked down.
			 */
			if (nubus_find_rsrc(
			    &fmtblock, &dir, rsrcid, &dirent) <= 0)
				goto notfound;
		}

		nubus_get_dir_from_rsrc(&fmtblock, &dirent, &dir);

		if (nubus_find_rsrc(
		    &fmtblock, &dir, NUBUS_RSRC_TYPE, &dirent) <= 0)
			goto notfound;

		if (nubus_get_ind_data(&fmtblock, &dirent,
		    (caddr_t)&slottype, sizeof(nubus_type)) <= 0)
			goto notfound;

		/*
		 * If this is a display card, try to pull out the correct
		 * display mode as passed by the booter.
		 */
		if (slottype.category == NUBUS_CATEGORY_DISPLAY) {
			int r;

			if ((r = nubus_video_resource(i)) != -1) {

				nubus_get_main_dir(&fmtblock, &dir);

				if (nubus_find_rsrc(
				    &fmtblock, &dir, r, &dirent) <= 0)
					goto notfound;

				nubus_get_dir_from_rsrc(&fmtblock,
				    &dirent, &dir);

				if (nubus_find_rsrc(&fmtblock, &dir,
				    NUBUS_RSRC_TYPE, &dirent) <= 0)
					goto notfound;

				if (nubus_get_ind_data(
				    &fmtblock, &dirent, (caddr_t)&slottype,
				    sizeof(nubus_type)) <= 0)
					goto notfound;

				rsrcid = r;
			}
		}

		nubus_slots[i].slot = i;
		nubus_slots[i].rsrcid = rsrcid;
		nubus_slots[i].category = slottype.category;
		nubus_slots[i].type = slottype.type;
		nubus_slots[i].drsw = slottype.drsw;
		nubus_slots[i].drhw = slottype.drhw;
		nubus_slots[i].fmt = &fmtblock;
		nubus_slots[i].flags |= NUBUS_SLOT_FILLED;
		nubus_slots[i].base_virt = (caddr_t)location;

		/* io_map() can't be undone on PPC. */
		/* That's why we have base_virt */
		/* bus_space_unmap(NBMEMSIZE); */

#if 0
		config_found(self, &na_args, nubus_print);
#endif
	}
#if 0
	enable_nubus_intr();
#endif
    }
}

static int
nubus_print(aux, pnp)
	void *aux;
	const char *pnp;
{
	struct nubus_device *na = (struct nubus_device *)aux;

	if (pnp) {
		printf("%s slot %x", pnp, na->slot);
		if (NUBUS_IO(
		    NUBUS_SLOT2PA(na->slot))) {
			printf(": %s", nubus_get_card_name(na->fmt));
			printf(" (Vendor: %s,", nubus_get_vendor(
			    na->fmt, NUBUS_RSRC_VEND_ID));
			printf(" Part: %s)", nubus_get_vendor(
			    na->fmt, NUBUS_RSRC_VEND_PART));
			/* io_map() can't be undone on PPC. */
			/* bus_space_unmap(NBMEMSIZE); */
		}
#ifdef DIAGNOSTIC
		else
			printf(":");
		printf(" Type: %04x %04x %04x %04x",
		    na->category, na->type, na->drsw, na->drhw);
#endif
	} else {
		printf(" slot %x", na->slot);
	}
	return (UNCONF);
}

char *getenv(char *);

#define MAX_VECS 6

static int
nubus_video_resource(slot)
	int slot;
{
#if 0
	/* We don't have this in our booter... maybe we could do
	   something similar with what our booter does pass in... */

	extern u_int16_t mac68k_vrsrc_vec[];
#else
	u_int16_t vrsrc_vec[MAX_VECS];
	int num_vecs;
	char *envp, *ptr;
#endif
	int i;

#if 0
	for (i = 0 ; i < 6 ; i++)
		if ((mac68k_vrsrc_vec[i] & 0xff) == slot)
			return ((mac68k_vrsrc_vec[i] >> 8) & 0xff);
	}
#else
	num_vecs=0;
	if (envp=getenv("video_rsrc")) {
	    for (ptr=envp ; (*ptr) && (num_vecs<MAX_VECS) ; ptr++) {
		if (*ptr == ',') {
		    *ptr=0;
		    vrsrc_vec[num_vecs++] = atoi((unsigned char *)envp);
		    envp=(++ptr);
		}
	    }
	    if ((ptr!=envp) && (!(*ptr)) && (*envp) && (num_vecs<MAX_VECS))
		vrsrc_vec[num_vecs++] = atoi((unsigned char *)envp);

	    for (i = 0 ; i < num_vecs ; i++)
		if ((vrsrc_vec[i] & 0xff) == slot)
			return ((vrsrc_vec[i] >> 8) & 0xff);
	}
#endif

	return (-1);
}

/*
 * Probe a given nubus slot.  If a card is there and we can get the
 * format block from it's clutching decl. ROMs, fill the format block
 * and return non-zero.  If we can't find a card there with a valid
 * decl. ROM, return 0.
 *
 * First, we check to see if we can access the memory at the tail
 * end of the slot.  If so, then we check for a bytelanes byte.  We
 * could probably just return a failure status if we bus error on
 * the first try, but there really is little reason not to go ahead
 * and check the other three locations in case there's a wierd card
 * out there.
 *
 * Checking for a card involves locating the "bytelanes" byte which
 * tells us how to interpret the declaration ROM's data.  The format
 * block is at the top of the card's standard memory space and the
 * bytelanes byte is at the end of that block.
 *
 * After some inspection of the bytelanes byte, it appears that it
 * takes the form 0xXY where Y is a bitmask of the bytelanes in use
 * and X is a bitmask of the lanes to ignore.  Hence, (X ^ Y) == 0
 * and (less obviously), Y will have the upper N bits clear if it is
 * found N bytes from the last possible location.  Both that and
 * the exclusive-or check are made.
 *
 * If a valid
 */

static u_int8_t	nbits[] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
static int
nubus_probe_slot(vm_offset_t base, int slot, nubus_slot *fmt)
{
	u_long ofs, hdr;
	int i, j, found, hdr_size;
	u_int8_t lanes;

// #ifdef DEBUG
	if (nubus_debug & NDB_PROBE)
		printf("probing slot %x\n", slot);
// #endif

	/*
	 * The idea behind this glorious work of art is to probe for only
	 * valid bytelanes values at appropriate locations (see DC&D p. 159
	 * for a list).  Note the pattern:  the first 8 values are at offset
	 * 0xffffff in the slot's space; the next 4 values at 0xfffffe; the
	 * next 2 values at 0xfffffd; and the last one at 0xfffffc.
	 *
	 * The nested loops implement an efficient search of this space,
	 * probing first for a valid address, then checking for each of the
	 * valid bytelanes values at that address.
	 */
	ofs = NBMEMSIZE;
	lanes = 0xf;

	for (j = 8, found = 0; j > 0 && !found; j >>= 1) {
		ofs--;
		for (i = j; i > 0; i--, lanes--) {
			printf("Trying bus space probe of %x\n", (base+ofs));
			if (!mac68k_bus_space_probe(base+ofs, 1)) {
				lanes -= i;
				break;
			}
			printf("Trying inb\n");
			if (inb(base+ofs) ==
			    (((~lanes & 0xf) << 4) | lanes)) {
				found = 1;
				break;
			}
			printf("done.\n");
		}
	}

	if (!found) {
#ifdef DEBUG
		if (nubus_debug & NDB_PROBE)
			printf("bytelanes not found for slot %x\n", slot);
#endif
		return 0;
	}

	fmt->bytelanes = lanes;
	fmt->step = nbits[(lanes & 0x0f)];
	fmt->slot = slot;	/* XXX redundant; get rid of this someday */

#ifdef DEBUG
	if (nubus_debug & NDB_PROBE)
		printf("bytelanes of 0x%x found for slot 0x%x.\n",
		    fmt->bytelanes, slot);
#endif

	/*
	 * Go ahead and attempt to load format header.
	 * First, we need to find the first byte beyond memory that
	 * would be valid.  This is necessary for NUBUS_ROM_offset()
	 * to work.
	 */
	hdr = NBMEMSIZE;
	hdr_size = 20;

	i = 0x10 | (lanes & 0x0f);
	while ((i & 1) == 0) {
		hdr++;
		i >>= 1;
	}
	fmt->top = hdr;
	hdr = nubus_adjust_ptr(lanes, hdr, -hdr_size);
#ifdef DEBUG
	if (nubus_debug & NDB_PROBE)
		printf("fmt->top is 0x%lx, that minus 0x%x puts us at 0x%lx.\n",
		    fmt->top, hdr_size, hdr);
	if (nubus_debug & NDB_ARITH)
		for (i = 1 ; i < 8 ; i++)
			printf("0x%lx - 0x%x = 0x%lx, + 0x%x = 0x%lx.\n",
			    hdr, i, nubus_adjust_ptr(lanes, hdr, -i),
			    i, nubus_adjust_ptr(lanes, hdr,  i));
#endif

	fmt->directory_offset =
	    0xff000000 | nubus_read_4(base+lanes, hdr);
	hdr = nubus_adjust_ptr(lanes, hdr, 4);
	fmt->length = nubus_read_4(base+lanes, hdr);
	hdr = nubus_adjust_ptr(lanes, hdr, 4);
	fmt->crc = nubus_read_4(base+lanes, hdr);
	hdr = nubus_adjust_ptr(lanes, hdr, 4);
	fmt->revision_level = nubus_read_1(base+lanes, hdr);
	hdr = nubus_adjust_ptr(lanes, hdr, 1);
	fmt->format = nubus_read_1(base+lanes, hdr);
	hdr = nubus_adjust_ptr(lanes, hdr, 1);
	fmt->test_pattern = nubus_read_4(base+lanes, hdr);

#ifdef DEBUG
	if (nubus_debug & NDB_PROBE) {
		printf("Directory offset 0x%x\t", fmt->directory_offset);
		printf("Length 0x%x\t", fmt->length);
		printf("CRC 0x%x\n", fmt->crc);
		printf("Revision level 0x%x\t", fmt->revision_level);
		printf("Format 0x%x\t", fmt->format);
		printf("Test Pattern 0x%x\n", fmt->test_pattern);
	}
#endif

	if ((fmt->directory_offset & 0x00ff0000) == 0) {
		printf("Invalid looking directory offset (0x%x)!\n",
		    fmt->directory_offset);
		return 0;
	}
	if (fmt->test_pattern != NUBUS_ROM_TEST_PATTERN) {
		printf("Nubus--test pattern invalid:\n");
		printf("       slot 0x%x, bytelanes 0x%x?\n", fmt->slot, lanes);
		printf("       read test 0x%x, compare with 0x%x.\n",
		    fmt->test_pattern, NUBUS_ROM_TEST_PATTERN);
		return 0;
	}

	/* Perform CRC */
	if (fmt->crc != nubus_calc_CRC(fmt)) {
		printf("Nubus--crc check failed, slot 0x%x.\n", fmt->slot);
		return 0;
	}

	return 1;
}

static u_int32_t
nubus_calc_CRC(fmt)
	nubus_slot	*fmt;
{
#if 0
	u_long base, ptr, crc_loc;
	u_int32_t sum;
	u_int8_t lanes = fmt->bytelanes;

	base = fmt->top;
	crc_loc = NUBUS_ROM_offset(fmt, base, -12);
	ptr = NUBUS_ROM_offset(fmt, base, -fmt->length);

	sum = 0;
	while (ptr < base)
		roll #1, sum
		if (ptr == crc_loc) {
			roll #3, sum
			ptr = nubus_adjust_ptr(lanes, ptr, 3);
		} else {
			sum += nubus_read_1(base+lanes, ptr);
		}
		ptr = nubus_adjust_ptr(lanes, ptr, 1);
	}

	return sum;
#endif
	return fmt->crc;
}

/*
 * Compute byte offset on card, taking into account bytelanes.
 * Base must be on a valid bytelane for this function to work.
 * Return the new address.
 *
 * XXX -- There has GOT to be a better way to do this.
 */
static u_long
nubus_adjust_ptr(u_int8_t lanes, u_long base, long amt)
{
	u_int8_t b, t;

	if (!amt)
		return base;

	if (amt < 0) {
		amt = -amt;
		b = lanes;
		t = (b << 4);
		b <<= (3 - (base & 0x3));
		while (amt) {
			b <<= 1;
			if (b == t)
				b = lanes;
			if (b & 0x08)
				amt--;
			base--;
		}
		return base;
	}

	t = (lanes & 0xf) | 0x10;
	b = t >> (base & 0x3);
	while (amt) {
		b >>= 1;
		if (b == 1)
			b = t;
		if (b & 1)
			amt--;
		base++;
	}

	return base;
}

static u_int8_t
nubus_read_1(u_int8_t lanes, u_long ofs)
{
	return inb(ofs);
}

#ifdef notyet
/* Nothing uses this, yet */
static u_int16_t
nubus_read_2(u_int8_t lanes, u_long ofs)
{
	u_int16_t s;

	s = (nubus_read_1(lanes, ofs) << 8);
	ofs = nubus_adjust_ptr(lanes, ofs, 1);
	s |= nubus_read_1(lanes, ofs);
	return s;
}
#endif

static u_int32_t
nubus_read_4(u_int8_t lanes, u_long ofs)
{
	u_int32_t l;
	int i;

	l = 0;
	for (i = 0; i < 4; i++) {
		l = (l << 8) | nubus_read_1(lanes, ofs);
		ofs = nubus_adjust_ptr(lanes, ofs, 1);
	}
	return l;
}

void
nubus_get_main_dir(fmt, dir_return)
	nubus_slot *fmt;
	nubus_dir *dir_return;
{
#ifdef DEBUG
	if (nubus_debug & NDB_FOLLOW)
		printf("nubus_get_main_dir(%p, %p)\n",
		    fmt, dir_return);
#endif
	dir_return->dirbase = nubus_adjust_ptr(fmt->bytelanes, fmt->top,
	    fmt->directory_offset - 20);
	dir_return->curr_ent = dir_return->dirbase;
}

void
nubus_get_dir_from_rsrc(fmt, dirent, dir_return)
	nubus_slot *fmt;
	nubus_dirent *dirent;
	nubus_dir *dir_return;
{
	u_long loc;

#ifdef DEBUG
	if (nubus_debug & NDB_FOLLOW)
		printf("nubus_get_dir_from_rsrc(%p, %p, %p).\n",
		    fmt, dirent, dir_return);
#endif
	if ((loc = dirent->offset) & 0x800000) {
		loc |= 0xff000000;
	}
	dir_return->dirbase =
	    nubus_adjust_ptr(fmt->bytelanes, dirent->myloc, loc);
	dir_return->curr_ent = dir_return->dirbase;
}

int
nubus_find_rsrc(nubus_slot *fmt, nubus_dir *dir, u_int8_t rsrcid,
	nubus_dirent *dirent_return)
{
	u_long entry;
	u_int8_t byte, lanes = fmt->bytelanes;

#ifdef DEBUG
	if (nubus_debug & NDB_FOLLOW)
		printf("nubus_find_rsrc(%p, %p, 0x%x, %p)\n",
		    fmt, dir, rsrcid, dirent_return);
#endif
	if (fmt->test_pattern != NUBUS_ROM_TEST_PATTERN)
		return -1;

	entry = dir->curr_ent;
	do {
		byte = nubus_read_1(lanes, entry);
#ifdef DEBUG
		if (nubus_debug & NDB_FOLLOW)
			printf("\tFound rsrc 0x%x.\n", byte);
#endif
		if (byte == rsrcid) {
			dirent_return->myloc = entry;
			dirent_return->rsrc_id = rsrcid;
			entry = nubus_read_4(lanes, entry);
			dirent_return->offset = (entry & 0x00ffffff);
			return 1;
		}
		if (byte == 0xff) {
			entry = dir->dirbase;
		} else {
			entry = nubus_adjust_ptr(lanes, entry, 4);
		}
	} while (entry != (u_long)dir->curr_ent);
	return 0;
}

int
nubus_get_ind_data(fmt, dirent, data_return, nbytes)
	nubus_slot *fmt;
	nubus_dirent *dirent;
	caddr_t data_return;
	int nbytes;
{
	u_long loc;
	u_int8_t lanes = fmt->bytelanes;

#ifdef DEBUG
	if (nubus_debug & NDB_FOLLOW)
		printf("nubus_get_ind_data(%p, %p, %p, %d).\n",
		    fmt, dirent, data_return, nbytes);
#endif
	if ((loc = dirent->offset) & 0x800000) {
		loc |= 0xff000000;
	}
	loc = nubus_adjust_ptr(lanes, dirent->myloc, loc);

	while (nbytes--) {
		*data_return++ = nubus_read_1(lanes, loc);
		loc = nubus_adjust_ptr(lanes, loc, 1);
	}
	return 1;
}

int
nubus_get_c_string(fmt, dirent, data_return, max_bytes)
	nubus_slot *fmt;
	nubus_dirent *dirent;
	caddr_t data_return;
	int max_bytes;
{
	u_long loc;
	u_int8_t lanes = fmt->bytelanes;

#ifdef DEBUG
	if (nubus_debug & NDB_FOLLOW)
		printf("nubus_get_c_string(%p, %p, %p, %d).\n",
		    fmt, dirent, data_return, max_bytes);
#endif
	if ((loc = dirent->offset) & 0x800000)
		loc |= 0xff000000;

	loc = nubus_adjust_ptr(lanes, dirent->myloc, loc);

	*data_return = '\0';
	while (max_bytes--) {
		if ((*data_return++ =
		    nubus_read_1(lanes, loc)) == 0)
			return 1;
		loc = nubus_adjust_ptr(lanes, loc, 1);
	}
	return 0;
}

static char	*huh = "???";

char *
nubus_get_vendor(fmt, rsrc)
	nubus_slot *fmt;
	int rsrc;
{
	static char str_ret[64];
	nubus_dir dir;
	nubus_dirent ent;

#ifdef DEBUG
	if (nubus_debug & NDB_FOLLOW)
		printf("nubus_get_vendor(%p, 0x%x).\n", fmt, rsrc);
#endif
	nubus_get_main_dir(fmt, &dir);
	if (nubus_find_rsrc(fmt, &dir, 1, &ent) <= 0)
		return huh;
	nubus_get_dir_from_rsrc(fmt, &ent, &dir);

	if (nubus_find_rsrc(fmt, &dir, NUBUS_RSRC_VENDORINFO, &ent)
	    <= 0)
		return huh;
	nubus_get_dir_from_rsrc(fmt, &ent, &dir);

	if (nubus_find_rsrc(fmt, &dir, rsrc, &ent) <= 0)
		return huh;

	nubus_get_c_string(fmt, &ent, str_ret, 64);

	return str_ret;
}

char *
nubus_get_card_name(fmt)
	nubus_slot *fmt;
{
	static char name_ret[64];
	nubus_dir dir;
	nubus_dirent ent;

#ifdef DEBUG
	if (nubus_debug & NDB_FOLLOW)
		printf("nubus_get_card_name(%p).\n", fmt);
#endif
	nubus_get_main_dir(fmt, &dir);

	if (nubus_find_rsrc(fmt, &dir, 1, &ent) <= 0)
		return huh;

	nubus_get_dir_from_rsrc(fmt, &ent, &dir);

	if (nubus_find_rsrc(fmt, &dir, NUBUS_RSRC_NAME, &ent) <= 0)
		return huh;

	nubus_get_c_string(fmt, &ent, name_ret, 64);

	return name_ret;
}

#if DEBUG
void
nubus_scan_slot(slotno)
	int slotno;
{
	int i=0, state=0;
	char twirl[] = "-\\|/";

	if (!(NUBUS_IO(NUBUS_SLOT2PA(slotno), NBMEMSIZE))) {
		printf("nubus_scan_slot: failed to map slot %d\n", slotno);
		return;
	}

	printf("Scanning slot %c for accessible regions:\n",
		slotno == 9 ? '9' : slotno - 10 + 'A');
	for (i=0 ; i<NBMEMSIZE; i++) {
		if (mac68k_bus_space_probe(i, 1)) {
			if (state == 0) {
				printf("\t0x%x-", i);
				state = 1;
			}
		} else {
			if (state) {
				printf("0x%x\n", i);
				state = 0;
			}
		}
		if (i%100 == 0) {
			printf("%c\b", twirl[(i/100)%4]);
		}
	}
	if (state) {
		printf("0x%x\n", i);
	}
	return;
}
#endif


void nubus_register_controller(nubus_controller_t controller)
{
    if (number_of_nubus_controllers >= NNUBUS) {
	/* No free slots.... */
	printf("nubus_register_controller(): No free slots!\n");
	return;
    }

    if (!controller) {
	/* bogus register -- shouldn't happen */
	printf("nubus_register_controller(): controller == NULL!\n");
	return;
    }

    nubus_controller_list[number_of_nubus_controllers++] = controller;
}

boolean_t	find_nubus_controller_type(char *name)
{
return 1;
}

#endif /* NNUBUS > 0 */
