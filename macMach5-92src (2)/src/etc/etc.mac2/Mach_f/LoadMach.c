/* the LoadMach() function does the actual loading of the Mach kernel */

#include <Types.h>
#include <Memory.h>
#include <Resources.h>
#include <Dialogs.h>
#include <Files.h>
#include <Retrace.h>
#include <OSUtils.h>

#include "types.h"
#include "param.h"
#include "fs.h"
#include "inode.h"
#include "conf.h"
#include "INIT_dialog.h"
#include "ufs.h"
#include "hfs.h"
#include "AlertMsg.h"
#include "exec.h"
#include "SerialPorts.h"
#include "LoadMach.h"
#include "string.h"
#include "device.h"
#include "TimeOffset.h"
#include "decompress.h"

#define RB_SINGLE 0x02

#define CHUNK (32*1024)

/* check load source, return non-zero if it is available, zero otherwise */
int CheckLoad(short device, char *file)
{
	UFSparam ufsparam;
	HFSparam hfsparam;

	if (device == Device_SystemVolume) {
		if (HFSopen(&hfsparam, file)) return 0;
	}
	else {
		if (UFSopen(&ufsparam, file, item_to_dev(device))) return 0;
	}
	return 1;
}

/* load, reallocate and jump to kernel, return if error */
void LoadMach(d)
DialogVars *d;
{
	Ptr p, block, image, LoadFile;
	long bar_num, bar_denom, n, kernel_size, new_size, result;
	struct exec hdr;
	short LoadDev, RootDev;
	unsigned long BootHowTo, kernel_base, *a;
	VolumeParam volume_params;
	struct relocation_info *reloc;
	UFSparam ufsparam;
	HFSparam hfsparam;
	ConfHandle conf;
	SysEnvRec sysenv;
	struct decompress_state s;
	long htab[HSIZE];
	unsigned short tab_prefix[HSIZE];

	block = 0;

	conf = (ConfHandle)GetResource('conf', resID_conf);
	(void)SysEnvirons(1, &sysenv);

	/* set default or alternate parameters */
	if ((*conf)->Flags & CONF_ALTERNATE) {
		LoadDev = (*conf)->LoadDev2;
		RootDev = (*conf)->RootDev2;
		LoadFile = (*conf)->LoadFile2;
	}
	else {
		LoadDev = (*conf)->LoadDev1;
		RootDev = (*conf)->RootDev1;
		LoadFile = (*conf)->LoadFile1;
	}

	/* set BootHowTo flags */
	BootHowTo = 0;
	if ((*conf)->Flags & CONF_SINGLE) BootHowTo |= RB_SINGLE;

	/* open mach_kernel file */
	if (LoadDev == Device_SystemVolume) {
		if (HFSopen(&hfsparam, LoadFile)) {
			AlertMsg(MSG_BAD_HFS_OPEN, LoadFile, 0, 0);
			goto error;
		}
		setup_decompress(&s, htab, tab_prefix, (char *)&hfsparam, (decompress_reader_t)HFSread);
	}
	else {
		if (result = UFSopen(&ufsparam, LoadFile, item_to_dev(LoadDev))) {
			switch (result) {
				case E_BAD_FS:
					AlertMsg(MSG_BAD_FS, 0, 0, 0);
					break;
				case E_BAD_LABEL_MAGIC:
					AlertMsg(MSG_BAD_LABEL_MAGIC, 0, 0, 0);
					break;
				case E_BAD_DRIVE_TYPE:
					AlertMsg(MSG_BAD_DRIVE_TYPE, 0, 0, 0);
					break;
				case E_BAD_FS_TYPE:
					AlertMsg(MSG_BAD_FS_TYPE, 0, 0, 0);
					break;
				case E_NOT_DIR:
					AlertMsg(MSG_NOT_DIR, 0, 0, 0);
					break;
				case E_EMPTY_DIR:
					AlertMsg(MSG_EMPTY_DIR, 0, 0, 0);
					break;
				case E_NAME_NOT_FOUND:
					AlertMsg(MSG_NAME_NOT_FOUND, 0, 0, 0);
					break;
				case E_NULL_PATH:
					AlertMsg(MSG_NULL_PATH, 0, 0, 0);
					break;
				case E_SCSIGET_ERR:
					AlertMsg(MSG_SCSIGET_ERR, 0, 0, 0);
					break;
				case E_SCSISEL_ERR:
					AlertMsg(MSG_SCSISEL_ERR, 0, 0, 0);
					break;
				case E_SCSICMD_ERR:
					AlertMsg(MSG_SCSICMD_ERR, 0, 0, 0);
					break;
				case E_SCSIREAD_ERR:
					AlertMsg(MSG_SCSIREAD_ERR, 0, 0, 0);
					break;
				case E_SCSIMSG_ERR:
					AlertMsg(MSG_SCSIMSG_ERR, 0, 0, 0);
					break;
				case E_BAD_DEVICE_MAJ:
					AlertMsg(MSG_BAD_DEVICE_MAJ, 0, 0, 0);
					break;
				case E_NO_PART_INFO:
					AlertMsg(MSG_NO_PART_INFO, 0, 0, 0);
					break;
				case E_BAD_DPME_MAGIC:
					AlertMsg(MSG_BAD_DPME_MAGIC, 0, 0, 0);
					break;
				case E_NO_MACH_PART:
					AlertMsg(MSG_NO_MACH_PART, 0, 0, 0);
					break;
			}
			goto error;
		}
		setup_decompress(&s, htab, tab_prefix, (char *)&ufsparam, (decompress_reader_t)UFSread);
	}

	/* read a.out header, verify magic number and machine type */
	if (decompress_read(&s, (char *)&hdr, sizeof(hdr)) != sizeof(hdr)) {
		AlertMsg(MSG_HDR_RD_ERR, 0, 0, 0);
		goto error;
	}
	if (N_BADMAG(hdr) || (hdr.a_machtype != M_68020)) {
		AlertMsg(MSG_BAD_HDR_MAGIC, 0, 0, 0);
		goto error;
	}

	/* calculate the size of the kernel image to be loaded */
	kernel_size = hdr.a_text + hdr.a_data;

	/* prepare progress bar indicator */
	bar_denom = kernel_size;
	bar_num = 0;
	ProgressBar(d, bar_num, bar_denom);

	/* the following allocates a non-relocatable block at the kernel base */
	MaxApplZone();
	kernel_base = (unsigned long)ApplicZone() + (*conf)->KernelSpace;
	if (!(block = StripAddress(NewPtr(kernel_size + NBPG)))) {
		AlertMsg(MSG_OUT_OF_MEMORY, 0, 0, 0);
		goto error;
	}
	/* if block address is beyond the desired kernel base, just use it */
	if (kernel_base < (unsigned long)block) kernel_base = (unsigned long)block;
	/* otherwise, extend block to allow image to be loaded at kernel base */
	else {
		new_size = GetPtrSize(block) + (kernel_base - (unsigned long)block);
		SetPtrSize(block, new_size);
		if (GetPtrSize(block) != new_size) {
			AlertMsg(MSG_NO_KERNEL_SPACE, 0, 0, 0);
			goto error;
		}
	}

	/* the TEXT and DATA will be loaded at the kernel base, page aligned */
	image = (Ptr)(kernel_base = (kernel_base + PGOFSET) & ~PGOFSET);

	/* load the kernel TEXT and DATA */
	for (p = image, n = kernel_size; n >= CHUNK; p += CHUNK, n -= CHUNK) {
		if (decompress_read(&s, p, CHUNK) != CHUNK) {
			AlertMsg(MSG_FILE_TRUNC, 0, 0, 0);
			goto error;
		}
		ProgressBar(d, bar_num += CHUNK, bar_denom);	
	}
	if (n > 0) {
		if (decompress_read(&s, p, n) != n) {
			AlertMsg(MSG_FILE_TRUNC, 0, 0, 0);
			goto error;
		}
		bar_num += n;
	}
	ProgressBar(d, bar_num, bar_denom);	

	/* relocate TEXT references */
	if (hdr.a_trsize) {
		if (!(p = NewPtr(hdr.a_trsize))) {
			AlertMsg(MSG_OUT_OF_MEMORY, 0, 0, 0);
			goto error;
		}
		reloc = (struct relocation_info *)p;
		if ((n = decompress_read(&s, (char *)reloc, hdr.a_trsize)) != hdr.a_trsize) {
			AlertMsg(MSG_FILE_TRUNC, LoadFile, 0, 0);
			DisposPtr(p);
			goto error;
		}
		for (n = hdr.a_trsize; n > 0; reloc++, n -= sizeof(*reloc)) {
			if (reloc->r_pcrel) continue;
			a = (unsigned long *)(reloc->r_address + image);
			*a += kernel_base;
		}
		DisposPtr(p);
	}

	/* relocate DATA references */
	if (hdr.a_drsize) {
		if (!(p = NewPtr(hdr.a_drsize))) {
			AlertMsg(MSG_OUT_OF_MEMORY, 0, 0, 0);
			goto error;
		}
		reloc = (struct relocation_info *)p;
		if ((n = decompress_read(&s, (char *)reloc, hdr.a_drsize)) != hdr.a_drsize) {
			AlertMsg(MSG_FILE_TRUNC, 0, 0, 0);
			DisposPtr(p);
			goto error;
		}
		for (n = hdr.a_drsize; n > 0; reloc++, n -= sizeof(*reloc)) {
			if (reloc->r_pcrel) continue;
			a = (unsigned long *)(reloc->r_address + hdr.a_text + image);
			*a += kernel_base;
		}
		DisposPtr(p);
	}

	/* load serial port drivers */
	if (SerialPorts()) goto error;

	/* take the system volume off-line */
	/* don't do this if booting an old Mach 2.5 kernel */
	volume_params.ioNamePtr = 0;
	volume_params.ioVRefNum = sysenv.sysVRefNum;
	PBOffLine((ParmBlkPtr)&volume_params);

	/* enter debugger if debug flag is set */
	if ((*conf)->Flags & CONF_DEBUG) DebugStr(getstr(STR_DEBUGGER));

	/* start up the mach kernel */
	/*a 32-bit clean kernel in 24-bit mode will return */
	/* the kernel will restore BufPtr to release the image space */
	(*((void (*)())image))(item_to_dev(RootDev), BootHowTo, item_to_TimeOffset((*conf)->TimeOffset));
	AlertMsg(MSG_32_BIT_KERNEL, 0, 0, 0);

error:
	if (block) DisposPtr(block);
	
}