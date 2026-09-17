/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED

/*
 * OSF/Mach3 Linux Server options
 */
#define CONFIG_OSFMACH3 1
#define CONFIG_OSFMACH3_DEBUG 1
#define CONFIG_OSFMACH3_VM_TIMER_BACKOFF (2)
#define CONFIG_OSFMACH3_VM_BUFFER_CACHE_WITHDRAW_SIZE (20)

/*
 * Code maturity level options
 */
#undef  CONFIG_EXPERIMENTAL
#define CONFIG_GRF_CONSOLE 1

/*
 * Loadable module support
 */
#undef  CONFIG_MODULES

/*
 * General setup
 */
#define CONFIG_NET 1
#define CONFIG_SYSVIPC 1
#undef  CONFIG_BINFMT_AOUT
#define CONFIG_BINFMT_ELF 1
#undef  CONFIG_BINFMT_JAVA
#define CONFIG_KERNEL_ELF 1

/*
 * Additional Block Devices
 */
#undef  CONFIG_BLK_DEV_LOOP
#undef  CONFIG_BLK_DEV_MD
#undef  CONFIG_BLK_DEV_RAM

/*
 * Networking options
 */
#undef  CONFIG_FIREWALL
#define CONFIG_NET_ALIAS 1
#define CONFIG_INET 1
#undef  CONFIG_IP_FORWARD
#undef  CONFIG_IP_MULTICAST
#undef  CONFIG_SYN_COOKIES
#undef  CONFIG_IP_ACCT
#undef  CONFIG_IP_ROUTER
#undef  CONFIG_NET_IPIP
#define CONFIG_IP_ALIAS 1

/*
 * (it is safe to leave these untouched)
 */
#undef  CONFIG_INET_PCTCP
#undef  CONFIG_INET_RARP
#undef  CONFIG_NO_PATH_MTU_DISCOVERY
#define CONFIG_IP_NOSR 1
#define CONFIG_SKB_LARGE 1

/*
 *  
 */
#undef  CONFIG_IPX
#undef  CONFIG_ATALK
#undef  CONFIG_AX25
#undef  CONFIG_NETLINK

/*
 * SCSI support
 */

/*
 * Network device support
 */
#define CONFIG_NETDEVICES 1
#define CONFIG_DUMMY 1
#undef  CONFIG_PLIP
#define CONFIG_PPP 1

/*
 * CCP compressors for PPP are only built as modules.
 */
#undef  CONFIG_SLIP
#define CONFIG_NET_ETHERNET 1

/*
 * Filesystems
 */
#undef  CONFIG_QUOTA
#define CONFIG_MINIX_FS 1
#undef  CONFIG_EXT_FS
#define CONFIG_EXT2_FS 1
#define CONFIG_BEXT2_FS 1
#undef  CONFIG_XIA_FS
#define CONFIG_FAT_FS 1
#define CONFIG_MSDOS_FS 1
#define CONFIG_VFAT_FS 1
#undef  CONFIG_UMSDOS_FS
#define CONFIG_PROC_FS 1
#define CONFIG_NFS_FS 1
#undef  CONFIG_ROOT_NFS
#undef  CONFIG_SMB_FS
#undef  CONFIG_ISO9660_FS
#undef  CONFIG_HPFS_FS
#undef  CONFIG_SYSV_FS
#undef  CONFIG_UFS_FS
#undef  CONFIG_HFS_FS

/*
 * Character devices
 */
#define CONFIG_SERIAL 1
#undef  CONFIG_PRINTER
#define CONFIG_MOUSE 1
#define CONFIG_UMISC 1
#define CONFIG_CHR_DEV_ST 1

/*
 * Kernel hacking
 */
