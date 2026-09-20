#
# Mach Operating System
# Copyright (c) 1991,1990,1989,1988,1987,1986 Carnegie Mellon University
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
# Revision 2.5  94/03/25  18:21:05  mrt
# 	Updated for odemake
# 	[93/11/15            mrt]
# 
# Revision 2.4  92/02/02  13:00:03  rpd
# 	Revised for the new Makefile organization.
# 	[92/01/30            rpd]
# 

DEPENDENCIES	=

# find machine-dependent files in machine subdirectory
# ${EXPORTBASE}/poe has the machine link

VPATH 		= ${target_cpu}:${EXPORTBASE}/poe
INCDIRS		:= -I${EXPORTBASE}/poe ${INCDIRS}

# We want the POE version to be part of the emulator name.
# Makefile-version defines VERSION

.if exists( ${MAKETOP}Makefile-version)
.include "${MAKETOP}Makefile-version"
.endif

EMULATOR 	= poe_emulator.${VERSION}

# We want to include files from the poe sources.
INCFLAGS	= -I../poe

LIBS 		= ${LIBMACH_SA}

MIG_HDRS	=
PROGRAMS	= emulator_base
OTHERS		= ${EMULATOR}
IDIR		= /special/
ILIST		= ${EMULATOR}

OFILES		= bsd_user_side.o emul_init.o emul_stack_alloc.o emul_generic.o \
		  allocator.o syscall_table.o emul_machdep.o emul_vector.o \
		  emul_cache.o bsd_1_user.o htg_vm_map.o



emulator_base_CCTYPE	= host
xxx_emulator_base.o_CCTYPE	= host
emulator_base_OFILES	= xxx_emulator_base.o
emulator_base_LIBS	= 

.include <${RULES_MK}>

emulator poe_emulator : ${EMULATOR}

${EMULATOR} : ${OFILES} emulator_base
	@${RM} ${_RMFLAGS_} ${.TARGET} ${.TARGET}.out
	${_LD_} -z -o ${.TARGET}.out -T `./emulator_base` ${_LDFLAGS_} \
		${CRT0} ${OFILES} ${LIBS} ${LDLIBS}  && \
	${MV} ${.TARGET}.out ${.TARGET}

# explicit dependencies on generated files

bsd_user_side.o : bsd_1.h

# mig rules to generate intermediate source files in the object directory
# Make execution of mig dynamically conditional, since one mig creates
# both user and header.

# htg_vm_map comes from mach/mach.defs, but we generate a special vesrion
# of it here.

MAKE_MACH = [ -d mach ] || mkdir mach
MACH_SRCS = mach/mach_interface.h mach/htg_vm_map.c

${MACH_SRCS} : mach_src_stamp
mach_src_stamp: mach/mach.defs
	-${MAKE_MACH}
	${MIG} ${_MIGFLAGS_} -DEMULATOR \
		-header mach/mach_interface.h \
		-i mach/ \
		-user mach/htg_vm_map.c \
		-server /dev/null \
		${mach/mach.defs:P} && \
		touch mach_src_stamp

BSD_SRCS = bsd_1.h bsd_1_user.c

${BSD_SRCS} : bsd_src_stamp
bsd_src_stamp: bsd_1.defs
	${MIG} ${_MIGFLAGS_} \
		-header bsd_1.h \
		-user bsd_1_user.c \
		-server /dev/null \
		${bsd_1.defs:P} && \
		touch bsd_src_stamp

# build rules for mig objects.
# When talking to a server we trust (kernel or Poe),
# we disable type-checking.

bsd_1_user.o_CFLAGS	= -DTypeCheck=0
htg_vm_map.o_CFLAGS	= -DTypeCheck=0

# explicit rules for dependencies on generated files to make sure
# mig gets run first

bsd_1_user.o: bsd_1_user.c
htg_vm_map.o : mach/htg_vm_map.c


# rules to generate emulator_base
# even though we preprocess emulator_base with the target cpp,
# to get appropriate predefines, we need to define ${TARGET_MACHINE},
# because it isn't always predefined.

# The -P switch is supposed to eliminate the #line and #define directives
# from the cpp output.

xxx_emulator_base.c: emulator_base.c
	${ansi_CPP} -P ${_CCFLAGS_} -D${TARGET_MACHINE} ${emulator_base.c:P} \
		 > xxx_emulator_base.c
	cat /dev/null >> emulator_base.d
	sed 's/emulator_base\.o/xxx_emulator_base.c/' \
			emulator_base.d > xxx_emulator_base.c.d
	${RM} emulator_base.d

