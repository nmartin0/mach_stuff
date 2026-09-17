/* device functions */

#include <Menus.h>

#include "types.h"
#include "disklabel.h"
#include "device.h"
#include "resID.h"

/* convert device menu item to Unix device number */
dev_t item_to_dev(item)
short item;
{
	switch (item) {
		case Device_sd0a: return makedev(MAJOR_SCSI, 0);
		case Device_sd1a: return makedev(MAJOR_SCSI, 32);
		case Device_sd2a: return makedev(MAJOR_SCSI, 64);
		case Device_sd3a: return makedev(MAJOR_SCSI, 96);
		case Device_sd4a: return makedev(MAJOR_SCSI, 128);
		case Device_sd5a: return makedev(MAJOR_SCSI, 160);
		case Device_sd6a: return makedev(MAJOR_SCSI, 192);
		case Device_ramdisk0a: return makedev(/*MAJOR_RAMDISK*/8, 0);
		case Device_SystemVolume: return -1;
	}
}

/* return name of device (pascal string) */
void DeviceName(item, name)
short item;
char *name;
{
	MenuHandle m;
	m = GetMenu(resID_MENU_Device);
	GetItem(m, item, name);
}

/* return name of device (C string) */
void devicename(item, name)
short item;
char *name;
{
	int len;
	char *n;

	DeviceName(item, name);
	/* convert from pascal string */
	for (len = *name, n = name; len > 0; len--, n++) *n = *(n + 1);
	*n = 0;
}
