/**************************************
 * PowerMac feature control registers *
 * from info gleaned from LinuxPPC    *
 **************************************/

enum system_feature {
	feature_null,
	feature_serial_reset,
	feature_serial_enable,
	feature_serial_io_a,
	feature_serial_io_b,
	feature_swim3_enable,
	feature_mesh_enable,
	feature_ide_enable,
	feature_via_enable,
	feature_cd_power,
	feature_mediabay_reset,
	feature_mediabay_enable,
	feature_mediabay_pci_enable,
	feature_mediabay_ide_enable,
	feature_mediabay_floppy_enable,
	feature_bmac_reset,
	feature_bmac_io_enable,
	feature_modem_poweron,
	feature_modem_reset,
	feature_last
};

int feature_set(struct bus_device *dev, enum system_feature f);
int feature_clear(struct bus_device *dev, enum system_feature f);
int feature_test(struct bus_device *dev, enum system_feature f);
void feature_init();

