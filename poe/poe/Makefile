#
# Mach Operating System
# Copyright (c) 1991,1990 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
# 
# Carnegie Mellon requests users of this software to return to
# 
#  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they made and grant Carnegie Mellon
# the rights to redistribute these changes.
#  
# HISTORY
# $Log:	Makefile,v $
# Revision 2.7  94/04/13  14:48:31  mrt
# 	Fixed to build vers.c every time poe is built.
# 	Updated for odemake.
# 	[94/04/13            mrt]
# 
# Revision 2.6  92/02/02  13:01:27  rpd
# 	Revised for the new Makefile organization.
# 	[92/01/30            rpd]
# 


DEPENDENCIES	=
EXPINC_SUBDIRS	= include

# find machine-dependent files in machine subdirectory
# ${EXPORTBASE}/poe has the machine link

VPATH = ${target_cpu}:${EXPORTBASE}/poe

# The EXPORTBASE/server directory contains the include files
# in machine

INCDIRS		:= -I${EXPORTBASE}/poe ${INCDIRS}

# We want the POE version to be part of the poe name.
# Makefile-version defines VERSION for us.


.if exists( ${MAKETOP}Makefile-version)
.include "${MAKETOP}Makefile-version"
.endif

POE = poe.${VERSION}


PROGRAMS	= boot poe_init 
OTHERS		= ${POE}
IDIR		= /etc/
${POE}_IDIR	= /special/
ILIST		= ${PROGRAMS} ${OTHERS}

# if STANDALONE is = 1, Poe is compiled to run as the only
# server running on MK. If STANDALONE  is 0 Poe is compiled
# to be started by the unix server so that POE can be debugged
# config.h defines it to be 1 if it is not already defined by CFLAGS
#CFLAGS		= -DSTANDALONE=0 -g

CFLAGS		= -g

MIG_SRVS	= bsd_1Server.c ufs_memory_objectServer.c udev_memory_objectServer.c
ufs_memory_objectServer.c_MIGSDEF	= memory_object.defs
ufs_memory_objectServer.c_MIGFLAGS	= "-DSUBSYSTEM=ufs_memory_object -DSERVERPREFIX=ufs_"

udev_memory_objectServer.c_MIGSDEF	= memory_object.defs
udev_memory_objectServer.c_MIGFLAGS	= "-DSUBSYSTEM=udev_memory_object -DSERVERPREFIX=udev_"

poe_init_OFILES	= minit.o
poe_init_LIBS	=

boot_LIBS	= ${LIBMACH}

# Poe source files include machine-dependent files
# with <machine/root_device.h>, etc.
# We setup a link in the object directory to make this work.
# So that shadowing (sort-of) works, we search MAKEPSD
# for the first appropriate directory.

${POE}_OFILES = bsd_1Server.o udev_memory_objectServer.o \
	ufs_memory_objectServer.o bdev_fops.o bsd_copy.o bsd_descrip.o \
	bsd_dops.o bsd_exception.o bsd_exec.o bsd_exit.o bsd_fops.o \
	bsd_fops2.o bsd_fork.o bsd_host.o bsd_io.o bsd_ioctl.o \
	bsd_itimer.o bsd_lookup.o bsd_machdep.o bsd_misc.o bsd_mman.o \
	bsd_mount.o bsd_prot.o machine_ptrace.o bsd_select.o \
	bsd_signal.o bsd_server_side.o bsd_special.o init_sysent.o \
	main.o mem_fops.o server_loop.o syscall.o tty_fops.o \
	ufs_devpager.o ufs_dir.o ufs_dops.o ufs_fops.o ufs_inode.o \
	ufs_mount.o ufs_pager.o pipe_fops.o subr_cbuf.o subr_copy.o \
	subr_fops.o subr_timeout.o subr_uerror.o map_file.o \
	printf.o libc.o 

${POE}_LIBS 	= ${LIBTHREADS} ${LIBMACH_SA}
${POE}_NOSTRIP	=

.include <${RULES_MK}>


# rules to make vers.c

VERSION_FILES = conf/version.major \
		conf/version.variant \
		conf/version.edit \
		conf/version.patch \
		conf/newvers.sh \
		conf/copyright

VERSION_FILES_SOURCE = ${conf/version.major:P} \
		${conf/version.variant:P} \
		${conf/version.edit:P} \
		${conf/version.patch:P}


poe:  ${POE}

${POE}:	${${POE}_OFILES} ${VERSION_FILES}
	@echo " "
	@echo "creating vers.o"
	@${RM} -f vers.c vers.o
	@/bin/sh ${conf/newvers.sh:P} ${conf/copyright:P} `cat ${VERSION_FILES_SOURCE}`
	${_CC_} -c ${_CCFLAGS_} vers.c
	${_CC_} ${_LDFLAGS_} -o ${.TARGET}.X ${${POE}_OFILES} vers.o ${_LIBS_} 
	${SIZE} ${.TARGET}.X
	cp -p ${.TARGET}.X ${.TARGET}.nostrip
	${XSTRIP} ${.TARGET}.X && ${MV} ${.TARGET}.X ${.TARGET}
