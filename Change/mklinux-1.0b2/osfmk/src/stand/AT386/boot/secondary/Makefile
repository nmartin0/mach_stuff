# 
# @OSF_COPYRIGHT@
# 
#
# Mach Operating System
# Copyright (c) 1994 Carnegie Mellon University
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
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they make and grant Carnegie Mellon
# the rights to redistribute these changes.
#
#
# HISTORY
# $Log: Makefile,v $
# Revision 1.1.9.2  1996/03/15  15:01:24  barbou
# 	Build on Linux:
# 	Reorganized rules to build ASM files with -D__LANGUAGE_ASSEMBLY defined.
# 	Use LD to link-edit the programs (for the -Ttext option).
# 	[96/03/06            barbou]
#
# 	Merged changes from nmk20b7_shared submission.
# 	[96/03/04            barbou]
#
# Revision 1.1.3.3  1996/03/04  16:34:40  bernadat
# 	Added dependency on libboot.a
# 	[96/02/28            bernadat]
# 
# Revision 1.1.9.1  1996/02/27  16:22:16  barbou
# 	Merged MkLinux changes.
# 	[1996/02/27  16:13:01  barbou]
# 
# Revision 1.1.7.1  1995/12/28  17:01:47  barbou
# 	Self-Contained Mach Distribution:
# 	added "file_system" directory to INCFLAGS.
# 	[95/12/28            barbou]
# 
# Revision 1.1.3.2  1995/11/15  06:29:07  bernadat
# 	Set GEDATE to be silent.
# 	[95/09/28            bernadat]
# 	[95/11/13            bernadat]
# 
# Revision 1.1.3.1  1995/08/21  19:29:36  devrcs
# 	Only generate one secondary boot, with remote console which
# 	the primary enables/disables.
# 	[95/07/07            yp]
# 
# 	Added network files and flags.
# 	[95/05/10            yp]
# 
# 	Used CC instead of LD. Cleanups
# 	Added libsa_fs
# 	[95/03/20            bernadat]
# 
# 	Initial revision
# 	Started with mach_kboot/Makefile
# 	[95/02/20            bernadat]
# 
# $EndLog$
#

VPATH 			= ..:net:protocols

INCFLAGS		= -I../include -I../ -I../../../../file_systems

USE_STATIC_LIBRARIES 	=
OBJECT_FORMAT 		= ELF

PROGRAMS 		= versiondate.h mach_boot.x mach_boot.lilo.x
OTHERS			= mach_boot mach_boot.lilo

ILIST 			= mach_boot mach_boot.lilo
IDIR 			= /stand/

LDFLAGS			= -nostdlib -Ttext 0x0 
LIBS			= -lboot -lsa_fs -lsa_mach

${PROGRAMS}: 		../lib/libboot.a

# ykpark: delete, just booting from floppy: CFLAGS +=  -DDEVICE_NS8390 -DDEVICE_3C503 -m386
CFLAGS +=  -m386 -DDEBUG
# The following flags are used for debugging precise pieces of software:
#	-DNETDEBUG
#	-DPROTODEBUG
#	-DTFTPDEBUG
#	-DBOOTPDEBUG
#	-DUDPIPDEBUG
#	-DETHERDEBUG
#	-DNS8390DEBUG
#	-DSMCDEBUG
#	-DARPDEBUG
#	-DDEBUG
#	-DEXCTRAP	/* future release, with control over IDT */

print_env:	
	@echo PATH=  ${PATH}
	@echo VPATH= ${VPATH}

boot.S_CFLAGS 		= ${CFLAGS} -DREMOTE

# start.o should be first
# ykpark: delete, just booting from floppy: NOBJS = net_debug.o 3c503.o ns8390.o smc.o endian.o dlink.o \
# ykpark: delete, just booting from floppy: 	i386at.o more_asm.o \
# ykpark: delete, just booting from floppy: 	arp.o bootp.o tftp.o udpip.o icmp.o \
# ykpark: delete, just booting from floppy: 	net.o
NOBJS = net_debug.o endian.o dlink.o \
	i386at.o more_asm.o \
	arp.o bootp.o tftp.o udpip.o icmp.o \
	net.o
OBJS  = start.o table.o boot.o mach_boot.o malloc.o \
	${NOBJS}
LOBJS = lilo.o table.o boot.o mach_boot.o malloc.o \
	${NOBJS}

mach_boot.x_OFILES 	= ${OBJS}
mach_boot.lilo.x_OFILES	= ${LOBJS}
SOBJS = start.o lilo.o more_asm.o
# ykpark: delete, just booting from floppy: COBJS = boot.o mach_boot.o malloc.o \
# ykpark: delete, just booting from floppy: 	net_debug.o 3c503.o ns8390.o smc.o endian.o dlink.o \
# ykpark: delete, just booting from floppy: 	i386at.o \
# ykpark: delete, just booting from floppy: 	arp.o bootp.o tftp.o udpip.o icmp.o \
# ykpark: delete, just booting from floppy: 	net.o 
COBJS = boot.o mach_boot.o malloc.o \
	net_debug.o endian.o dlink.o \
	i386at.o \
	arp.o bootp.o tftp.o udpip.o icmp.o \
	net.o 
COBJS_ALIGN8 = table.o

NO_STRICT_ANSI =

mach_boot.x: ${${.TARGET}_OFILES}
	${_LD_} ${_LDFLAGS_} -o ${.TARGET}.X ${${.TARGET}_OFILES} ${_LIBS_}
	${MV} ${.TARGET}.X ${.TARGET}

mach_boot.lilo.x: ${${.TARGET}_OFILES}
	${_LD_} ${_LDFLAGS_} -o ${.TARGET}.X ${${.TARGET}_OFILES} ${_LIBS_}
	${MV} ${.TARGET}.X ${.TARGET}

.include <${RULES_MK}>

# ykpark: change rmelfhdr to objcopy, I was working on redhat 5.2/x86 :
#  ${TARGET_EXEC_PREFIX}rmelfhdr ${.TARGET}.x ${.TARGET}
${OTHERS}: $${.TARGET}.x
	objcopy -O binary ${.TARGET}.x ${.TARGET}
	@ls -l $@.x $@

${SOBJS}: $${.TARGET:.o=.s}
	${RM} ${_RMFLAGS_} ${.TARGET:.o=.S}
	${CP} ${${.TARGET:.o=.s}:P} ${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -traditional -D__LANGUAGE_ASSEMBLY -E \
		${.TARGET:.o=.S} > ${.TARGET:.o=.i}
	sed '/^\#/d' ${.TARGET:.o=.i} > X${.TARGET:.o=.s}
	${_CC_} -c X${.TARGET:.o=.s} -o ${.TARGET}

${COBJS}: $${.TARGET:.o=.c}
	${RM} ${_RMFLAGS_} ${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -S ${${.TARGET:.o=.c}:P} 
	sed -e "s/\\.data/.text/"  ${.TARGET:.o=.s} > ${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -traditional -E ${.TARGET:.o=.S} > ${.TARGET:.o=.i}
	sed '/^\#/d' ${.TARGET:.o=.i} > X${.TARGET:.o=.s}
	${_CC_} -c X${.TARGET:.o=.s} -o ${.TARGET}

${COBJS_ALIGN8}: $${.TARGET:.o=.c}
	${RM} ${_RMFLAGS_} ${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -S ${${.TARGET:.o=.c}:P} 
	sed -e "s/\\.data/.text/" -e "s/\\.align.*2/.align 8/" \
	  ${.TARGET:.o=.s} > ${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -traditional -E ${.TARGET:.o=.S} > ${.TARGET:.o=.i}
	sed '/^\#/d' ${.TARGET:.o=.i} > X${.TARGET:.o=.s}
	${_CC_} -c X${.TARGET:.o=.s} -o ${.TARGET}

versiondate.h: GETDATE

GETDATE:
	@echo "#define VERSIONDATE	"\"`date +"%D %T"`\" > versiondate.h

.if exists(depend.mk)
.include "depend.mk"
.endif



