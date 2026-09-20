/* All possible device drivers. */

/* #include <linux_config_scsi.h> */
#include <linux_config_inet.h>
/* #include <linux_config_blk_dev_fd.h> */
/* #include <linux_config_blk_dev_ide.h> */
/* #include <linux_config_scsi_advansys.h> */
/* #include <linux_config_scsi_buslogic.h> */
/* #include <linux_config_scsi_omit_flashpoint.h> */
/* #include <linux_config_scsi_u14_34f.h> */
/* #include <linux_config_scsi_ultrastor.h> */
/* #include <linux_config_scsi_aha152x.h> */
/* #include <linux_config_scsi_aha1542.h> */
/* #include <linux_config_scsi_aha1740.h> */
/* #include <linux_config_scsi_aic7xxx.h> */
/* #include <linux_config_scsi_future_domain.h> */
/* #include <linux_config_scsi_in2000.h> */
/* #include <linux_config_scsi_generic_ncr5380.h> */
/* #include <linux_config_scsi_ncr53c406a.h> */
/* #include <linux_config_scsi_pass16.h> */
/* #include <linux_config_scsi_seagate.h> */
/* #include <linux_config_scsi_t128.h> */
/* #include <linux_config_scsi_ncr53c7xx.h> */
/* #include <linux_config_scsi_eata_dma.h> */
/* #include <linux_config_scsi_eata_pio.h> */
/* #include <linux_config_scsi_7000fasst.h> */
/* #include <linux_config_scsi_eata.h> */
/* #include <linux_config_scsi_am53c974.h> */
/* #include <linux_config_scsi_dtc3280.h> */
/* #include <linux_config_scsi_ncr53c8xx.h> */
/* #include <linux_config_scsi_dc390w.h> */
/* #include <linux_config_scsi_dc390t.h> */
/* #include <linux_config_scsi_ppa.h> */
/* #include <linux_config_scsi_qlogic_fas.h> */
/* #include <linux_config_scsi_qlogic_isp.h> */
/* #include <linux_config_scsi_gdth.h> */
/* #include <linux_config_ne2000.h> */
/* #include <linux_config_el2.h> */
/* #include <linux_config_el3.h> */
/* #include <linux_config_wd80x3.h> */
/* #include <linux_config_el1.h> */
/* #include <linux_config_ultra.h> */
/* #include <linux_config_ultra32.h> */
/* #include <linux_config_hplan_plus.h> */
/* #include <linux_config_hplan.h> */
/* #include <linux_config_vortex.h> */
/* #include <linux_config_seeq8005.h> */
/* #include <linux_config_hp100.h> */
/* #include <linux_config_ac3200.h> */
/* #include <linux_config_e2100.h> */
/* #include <linux_config_at1700.h> */
/* #include <linux_config_eth16i.h> */
/* #include <linux_config_znet.h> */
/* #include <linux_config_eexpress.h> */
/* #include <linux_config_eexpress_pro.h> */
/* #include <linux_config_eexpress_pro100b.h> */
/* #include <linux_config_depca.h> */
/* #include <linux_config_ewrk3.h> */
/* #include <linux_config_de4x5.h> */
/* #include <linux_config_apricot.h> */
/* #include <linux_config_wavelan.h> */
/* #include <linux_config_el16.h> */
/* #include <linux_config_elplus.h> */
/* #include <linux_config_de600.h> */
/* #include <linux_config_de620.h> */
/* #include <linux_config_sk_g16.h> */
/* #include <linux_config_ni52.h> */
/* #include <linux_config_ni65.h> */
/* #include <linux_config_atp.h> */
/* #include <linux_config_lance.h> */
/* #include <linux_config_dec_elcp.h> */
/* #include <linux_config_fmv18x.h> */
/* #include <linux_config_3c515.h> */
/* #include <linux_config_pcnet32.h> */
/* #include <linux_config_ne2k_pci.h> */
/* #include <linux_config_yellowfin.h> */
/* #include <linux_config_rtl8139.h> */
/* #include <linux_config_epic.h> */
/* #include <linux_config_tlan.h> */
/* #include <linux_config_via_rhine.h> */

#if LINUX_CONFIG_SCSI
#define CONFIG_SCSI
#else
#undef CONFIG_SCSI
#endif

#if LINUX_CONFIG_INET
#define CONFIG_INET
#else
#undef CONFIG_INET
#endif

#if LINUX_CONFIG_BLK_DEV_FD
#define CONFIG_BLK_DEV_FD
#else
#undef CONFIG_BLK_DEV_FD
#endif

#if LINUX_CONFIG_BLK_DEV_IDE
#define CONFIG_BLK_DEV_IDE
#else
#undef CONFIG_BLK_DEV_IDE
#endif

#if LINUX_CONFIG_SCSI_ADVANSYS
#define CONFIG_SCSI_ADVANSYS
#else
#undef CONFIG_SCSI_ADVANSYS
#endif

#if LINUX_CONFIG_SCSI_BUSLOGIC
#define CONFIG_SCSI_BUSLOGIC
#else
#undef CONFIG_SCSI_BUSLOGIC
#endif

#if LINUX_CONFIG_SCSI_OMIT_FLASHPOINT
#define CONFIG_SCSI_OMIT_FLASHPOINT
#else
#undef CONFIG_SCSI_OMIT_FLASHPOINT
#endif

#if LINUX_CONFIG_SCSI_U14_34F
#define CONFIG_SCSI_U14_34F
#else
#undef CONFIG_SCSI_U14_34F
#endif

#if LINUX_CONFIG_SCSI_ULTRASTOR
#define CONFIG_SCSI_ULTRASTOR
#else
#undef CONFIG_SCSI_ULTRASTOR
#endif

#if LINUX_CONFIG_SCSI_AHA152X
#define CONFIG_SCSI_AHA152X
#else
#undef CONFIG_SCSI_AHA152X
#endif

#if LINUX_CONFIG_SCSI_AHA1542
#define CONFIG_SCSI_AHA1542
#else
#undef CONFIG_SCSI_AHA1542
#endif

#if LINUX_CONFIG_SCSI_AHA1740
#define CONFIG_SCSI_AHA1740
#else
#undef CONFIG_SCSI_AHA1740
#endif

#if LINUX_CONFIG_SCSI_AIC7XXX
#define CONFIG_SCSI_AIC7XXX
#else
#undef CONFIG_SCSI_AIC7XXX
#endif

#if LINUX_CONFIG_SCSI_FUTURE_DOMAIN
#define CONFIG_SCSI_FUTURE_DOMAIN
#else
#undef CONFIG_SCSI_FUTURE_DOMAIN
#endif

#if LINUX_CONFIG_SCSI_IN2000
#define CONFIG_SCSI_IN2000
#else
#undef CONFIG_SCSI_IN2000
#endif

#if LINUX_CONFIG_SCSI_GENERIC_NCR5380
#define CONFIG_SCSI_GENERIC_NCR5380
#else
#undef CONFIG_SCSI_GENERIC_NCR5380
#endif

#if LINUX_CONFIG_SCSI_NCR53C406A
#define CONFIG_SCSI_NCR53C406A
#else
#undef CONFIG_SCSI_NCR53C406A
#endif

#if LINUX_CONFIG_SCSI_PASS16
#define CONFIG_SCSI_PASS16
#else
#undef CONFIG_SCSI_PASS16
#endif

#if LINUX_CONFIG_SCSI_SEAGATE
#define CONFIG_SCSI_SEAGATE
#else
#undef CONFIG_SCSI_SEAGATE
#endif

#if LINUX_CONFIG_SCSI_T128
#define CONFIG_SCSI_T128
#else
#undef CONFIG_SCSI_T128
#endif

#if LINUX_CONFIG_SCSI_NCR53C7xx
#define CONFIG_SCSI_NCR53C7xx
#else
#undef CONFIG_SCSI_NCR53C7xx
#endif

#if LINUX_CONFIG_SCSI_EATA_DMA
#define CONFIG_SCSI_EATA_DMA
#else
#undef CONFIG_SCSI_EATA_DMA
#endif

#if LINUX_CONFIG_SCSI_EATA_PIO
#define CONFIG_SCSI_EATA_PIO
#else
#undef CONFIG_SCSI_EATA_PIO
#endif

#if LINUX_CONFIG_SCSI_7000FASST
#define CONFIG_SCSI_7000FASST
#else
#undef CONFIG_SCSI_7000FASST
#endif

#if LINUX_CONFIG_SCSI_EATA
#define CONFIG_SCSI_EATA
#else
#undef CONFIG_SCSI_EATA
#endif

#if LINUX_CONFIG_SCSI_AM53C974
#define CONFIG_SCSI_AM53C974
#else
#undef CONFIG_SCSI_AM53C974
#endif

#if LINUX_CONFIG_SCSI_DTC3280
#define CONFIG_SCSI_DTC3280
#else
#undef CONFIG_SCSI_DTC3280
#endif

#if LINUX_CONFIG_SCSI_NCR53C8XX
#define CONFIG_SCSI_NCR53C8XX
#else
#undef CONFIG_SCSI_NCR53C8XX
#endif

#if LINUX_CONFIG_SCSI_DC390W
#define CONFIG_SCSI_DC390W
#else
#undef CONFIG_SCSI_DC390W
#endif

#if LINUX_CONFIG_SCSI_DC390T
#define CONFIG_SCSI_DC390T
#else
#undef CONFIG_SCSI_DC390T
#endif

#if LINUX_CONFIG_SCSI_PPA
#define CONFIG_SCSI_PPA
#else
#undef CONFIG_SCSI_PPA
#endif

#if LINUX_CONFIG_SCSI_QLOGIC_FAS
#define CONFIG_SCSI_QLOGIC_FAS
#else
#undef CONFIG_SCSI_QLOGIC_FAS
#endif

#if LINUX_CONFIG_SCSI_QLOGIC_ISP
#define CONFIG_SCSI_QLOGIC_ISP
#else
#undef CONFIG_SCSI_QLOGIC_ISP
#endif

#if LINUX_CONFIG_SCSI_GDTH
#define CONFIG_SCSI_GDTH
#else
#undef CONFIG_SCSI_GDTH
#endif

#if LINUX_CONFIG_NE2000
#define CONFIG_NE2000
#else
#undef CONFIG_NE2000
#endif

#if LINUX_CONFIG_EL2
#define CONFIG_EL2
#else
#undef CONFIG_EL2
#endif

#if LINUX_CONFIG_EL3
#define CONFIG_EL3
#else
#undef CONFIG_EL3
#endif

#if LINUX_CONFIG_WD80x3
#define CONFIG_WD80x3
#else
#undef CONFIG_WD80x3
#endif

#if LINUX_CONFIG_EL1
#define CONFIG_EL1
#else
#undef CONFIG_EL1
#endif

#if LINUX_CONFIG_ULTRA
#define CONFIG_ULTRA
#else
#undef CONFIG_ULTRA
#endif

#if LINUX_CONFIG_ULTRA32
#define CONFIG_ULTRA32
#else
#undef CONFIG_ULTRA32
#endif

#if LINUX_CONFIG_HPLAN_PLUS
#define CONFIG_HPLAN_PLUS
#else
#undef CONFIG_HPLAN_PLUS
#endif

#if LINUX_CONFIG_HPLAN
#define CONFIG_HPLAN
#else
#undef CONFIG_HPLAN
#endif

#if LINUX_CONFIG_VORTEX
#define CONFIG_VORTEX
#else
#undef CONFIG_VORTEX
#endif

#if LINUX_CONFIG_SEEQ8005
#define CONFIG_SEEQ8005
#else
#undef CONFIG_SEEQ8005
#endif

#if LINUX_CONFIG_HP100
#define CONFIG_HP100
#else
#undef CONFIG_HP100
#endif

#if LINUX_CONFIG_AC3200
#define CONFIG_AC3200
#else
#undef CONFIG_AC3200
#endif

#if LINUX_CONFIG_E2100
#define CONFIG_E2100
#else
#undef CONFIG_E2100
#endif

#if LINUX_CONFIG_AT1700
#define CONFIG_AT1700
#else
#undef CONFIG_AT1700
#endif

#if LINUX_CONFIG_ETH16I
#define CONFIG_ETH16I
#else
#undef CONFIG_ETH16I
#endif

#if LINUX_CONFIG_ZNET
#define CONFIG_ZNET
#else
#undef CONFIG_ZNET
#endif

#if LINUX_CONFIG_EEXPRESS
#define CONFIG_EEXPRESS
#else
#undef CONFIG_EEXPRESS
#endif

#if LINUX_CONFIG_EEXPRESS_PRO
#define CONFIG_EEXPRESS_PRO
#else
#undef CONFIG_EEXPRESS_PRO
#endif

#if LINUX_CONFIG_EEXPRESS_PRO100B
#define CONFIG_EEXPRESS_PRO100B
#else
#undef CONFIG_EEXPRESS_PRO100B
#endif

#if LINUX_CONFIG_DEPCA
#define CONFIG_DEPCA
#else
#undef CONFIG_DEPCA
#endif

#if LINUX_CONFIG_EWRK3
#define CONFIG_EWRK3
#else
#undef CONFIG_EWRK3
#endif

#if LINUX_CONFIG_DE4X5
#define CONFIG_DE4X5
#else
#undef CONFIG_DE4X5
#endif

#if LINUX_CONFIG_APRICOT
#define CONFIG_APRICOT
#else
#undef CONFIG_APRICOT
#endif

#if LINUX_CONFIG_WAVELAN
#define CONFIG_WAVELAN
#else
#undef CONFIG_WAVELAN
#endif

#if LINUX_CONFIG_EL16
#define CONFIG_EL16
#else
#undef CONFIG_EL16
#endif

#if LINUX_CONFIG_ELPLUS
#define CONFIG_ELPLUS
#else
#undef CONFIG_ELPLUS
#endif

#if LINUX_CONFIG_DE600
#define CONFIG_DE600
#else
#undef CONFIG_DE600
#endif

#if LINUX_CONFIG_DE620
#define CONFIG_DE620
#else
#undef CONFIG_DE620
#endif

#if LINUX_CONFIG_SK_G16
#define CONFIG_SK_G16
#else
#undef CONFIG_SK_G16
#endif

#if LINUX_CONFIG_NI52
#define CONFIG_NI52
#else
#undef CONFIG_NI52
#endif

#if LINUX_CONFIG_NI65
#define CONFIG_NI65
#else
#undef CONFIG_NI65
#endif

#if LINUX_CONFIG_ATP
#define CONFIG_ATP
#else
#undef CONFIG_ATP
#endif

#if LINUX_CONFIG_LANCE
#define CONFIG_LANCE
#else
#undef CONFIG_LANCE
#endif

#if LINUX_CONFIG_DEC_ELCP
#define CONFIG_DEC_ELCP
#else
#undef CONFIG_DEC_ELCP
#endif

#if LINUX_CONFIG_FMV18X
#define CONFIG_FMV18X
#else
#undef CONFIG_FMV18X
#endif

#if LINUX_CONFIG_3C515
#define CONFIG_3C515
#else
#undef CONFIG_3C515
#endif

#if LINUX_CONFIG_PCNET32
#define CONFIG_PCNET32
#else
#undef CONFIG_PCNET32
#endif

#if LINUX_CONFIG_NE2K_PCI
#define CONFIG_NE2K_PCI
#else
#undef CONFIG_NE2K_PCI
#endif

#if LINUX_CONFIG_YELLOWFIN
#define CONFIG_YELLOWFIN
#else
#undef CONFIG_YELLOWFIN
#endif

#if LINUX_CONFIG_RTL8139
#define CONFIG_RTL8139
#else
#undef CONFIG_RTL8139
#endif

#if LINUX_CONFIG_EPIC
#define CONFIG_EPIC
#else
#undef CONFIG_EPIC
#endif

#if LINUX_CONFIG_TLAN
#define CONFIG_TLAN
#else
#undef CONFIG_TLAN
#endif

#if LINUX_CONFIG_VIA_RHINE
#define CONFIG_VIA_RHINE
#else
#undef CONFIG_VIA_RHINE
#endif
