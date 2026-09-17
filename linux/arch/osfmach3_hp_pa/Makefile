# Copyright (c) Open Software Foundation, Inc.   
# 
# 
# pmk1.1


#
# osfmach3_hp_pa/Makefile
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies. Remember to do have actions
# for "archclean" and "archdep" for cleaning up and making dependencies for
# this architecture
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1994 by Linus Torvalds
#
HOSTCC		= cc

CROSS_COMPILE	=
CFLAGS		+= -nostdinc \
		-fno-builtin \
		-mportable-runtime \
		-Dhp_pa
LINKFLAGS       := -Ttext 0x1FE00000 -static -nostdlib -u __start_mach -e __start_mach
MIG		= mig
MIGCOM		= migcom
LIBS		:= $(LIBS) -L$(TOPDIR)/export-osfmach3/$(ARCH)/lib -lcthreads -lmach_maxonstack -lsa_mach 
CFLAGS		:= $(CFLAGS) 
ifdef	CONFIG_OSFMACH3_DEBUG
CFLAGS		:= $(CFLAGS) 
endif

# HEAD := arch/$(ARCH)/kernel/head.o

ARCH_SUBDIRS = arch/$(ARCH)/blk_dev arch/$(ARCH)/chr_dev arch/$(ARCH)/kernel arch/$(ARCH)/mm arch/$(ARCH)/server arch/$(ARCH)/lib
SUBDIRS := $(SUBDIRS) $(ARCH_SUBDIRS)
ARCHIVES := arch/$(ARCH)/blk_dev/blk_dev.o arch/$(ARCH)/chr_dev/chr_dev.o arch/$(ARCH)/kernel/kernel.o arch/$(ARCH)/mm/mm.o arch/$(ARCH)/server/server.o arch/$(ARCH)/lib/lib.o $(ARCHIVES)
#NO_RD_ARCHIVES := arch/$(ARCH)/kernel/no_ramdisk.o $(ARCHIVES)
#ARCHIVES := arch/$(ARCH)/kernel/ramdisk.o arch/$(ARCH)/kernel/root_sd.o $(ARCHIVES)

arch/$(ARCH)/blk_dev: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/$(ARCH)/blk_dev

arch/$(ARCH)/chr_dev: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/$(ARCH)/chr_dev

arch/$(ARCH)/kernel: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/$(ARCH)/kernel

arch/$(ARCH)/mm: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/$(ARCH)/mm

arch/$(ARCH)/server: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/$(ARCH)/server

arch/$(ARCH)/lib: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/$(ARCH)/lib

MAKEBOOT = $(MAKE) -C arch/$(ARCH)/boot

vmlinux.no_ramdisk: $(CONFIGURATION) init/main.o init/version.o linuxsubdirs arch/hp_pa/kernel/root_sd.o
	$(LD) $(LINKFLAGS) $(HEAD) init/main.o init/version.o \
		arch/hp_pa/kernel/root_sd.o \
		$(NO_RD_ARCHIVES) \
		$(FILESYSTEMS) \
		$(DRIVERS) \
		$(LIBS) -o vmlinux.no_ramdisk

vmlinux-hd.no_ramdisk: $(CONFIGURATION) init/main.o init/version.o linuxsubdirs arch/hp_pa/kernel/root_hd.o
	$(LD) $(LINKFLAGS) $(HEAD) init/main.o init/version.o \
		arch/hp_pa/kernel/root_hd.o \
		$(NO_RD_ARCHIVES) \
		$(FILESYSTEMS) \
		$(DRIVERS) \
		$(LIBS) -o vmlinux-hd.no_ramdisk

archclean:
	rm -f vmlinux.in_kernel
	rm -f osfmach3/server/serv_callback_user.c
	rm -f osfmach3/server/serv_callback_user.h
	rm -f osfmach3/server/serv_callback_server.c
	rm -f osfmach3/server/serv_callback_server.h
	rm -f osfmach3/server/memory_object_server.c
	rm -f osfmach3/server/memory_object_server.h
#	@$(MAKEBOOT) clean

archdep:
#	@$(MAKEBOOT) dep
