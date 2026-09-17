# 
# @OSF_FREE_COPYRIGHT@
# 
#
#
# Mach Operating System
# Copyright (c) 1992, 1991 Carnegie Mellon University
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
# Revision 1.1.7.2  1996/03/15  15:01:18  barbou
# 	Merged with nmk20b7_shared submission.
# 	[96/03/04            barbou]
#
# 	Build on Linux:
# 	Reorganized the rules for ASM and C files to allow ASM
# 	files to be compiled with an explicit -D__LANGUAGE_ASSEMBLY.
# 	[96/03/01            barbou]
#
# Revision 1.1.3.2  1996/03/04  16:34:36  bernadat
# 	Added dependency on libboot.a
# 	[96/02/28            bernadat]
# 
# Revision 1.1.5.1  1995/12/28  17:01:33  barbou
# 	Self-Contained Mach Distribution:
# 	added "file_system" directory to INCFLAGS.
# 	[95/12/28            barbou]
# 
# Revision 1.1.3.1  1995/08/21  19:29:09  devrcs
# 	Copied from src/stand/AT386
# 	Adapted for primary boot
# 	[95/06/27            bernadat]
# 
# Revision 3.0.21.5  1995/05/14  19:46:40  dwm
# 	ri-osc CR1304 - merge (nmk19_latest - nmk19b1) diffs into mainline.
# 	       CR1140 -- (fix from osc1.3.1) remove -fomit-frame-pointer
# 	       in order to produce smaller code [jeffc]
# 	[1995/05/14  19:33:54  dwm]
# 
# Revision 3.0.21.4  1995/04/07  18:40:42  barbou
# 	Added mach_boot subdir.
# 	[95/02/21            bernadat]
# 	[95/03/16            barbou]
# 
# Revision 3.0.21.3  1995/01/06  21:31:34  devrcs
# 	mk6 CR668 - 1.3b26 merge
# 
# 	BEGIN OSC1_3 HISTORY
# 	Revision 3.0.4.10  1993/12/08  17:05:27  jeffc
# 	CR10280 -- change to not reinit com port on every putc. Requires
# 	building REMOTE version of boot.o
# 	[1993/12/07  16:10:09  jeffc]
# 
# Revision 3.0.4.9  1993/11/22  18:54:36  gm
# 	CR10191: Disable use of extra warnings during setup pass.
# 	[1993/11/22  16:47:10  gm]
# 
# Revision 3.0.4.8  1993/10/28  17:20:32  gm
# 	CR10034: Added rboot for remote boot support.
# 	[1993/10/27  20:02:43  gm]
# 
# Revision 3.0.4.7  1993/10/20  19:27:50  gm
# 	CR9704: Remove VPATH to kernel source directory.  Changed build
# 	targets to create bootprog before stripping it with rmelfhdr.
# 	Changed remote/local console distinction to be compile-time
# 	instead of run-time.
# 	[1993/10/13  17:38:42  gm]
# 
# 	END OSC1_3 HISTORY
# 	Revision 3.0.21.1  1994/09/10  22:05:44  bolinger
# 	Merge up to NMK17.3
# 	[1994/09/08  20:11:46  bolinger]
# 
# Revision 3.0.16.1  1994/08/31  17:35:57  paire
# 	Add mach_kboot as a SUBDIR.
# 	[94/08/31            paire]
# 
# Revision 3.0.4.6  1993/09/24  14:31:20  jeffc
# 	CR9830 - further fixes to rmelfhdr. Do not build except
# 	on setup pass
# 	[1993/09/24  13:26:14  jeffc]
# 
# Revision 3.0.4.5  1993/09/23  19:51:35  jeffc
# 	CR9830 - correctly install boot code
# 	[1993/09/23  19:37:04  jeffc]
# 
# Revision 3.0.4.4  1993/09/22  20:52:37  jeffc
# 	CR9830 - build rmelfhdr during setup pass
# 	[1993/09/22  18:57:06  jeffc]
# 
# Revision 3.0.4.3  1993/09/20  17:37:53  jph
# 	CR9385: Updated to build with ELF compiler.
# 	[1993/09/18  16:44:57  gm]
# 
# Revision 3.0.4.2  1993/06/07  19:03:29  gm
# 	Moved here from mach3 /boot.
# 	[1993/06/07  18:58:23  gm]
# 
# Revision 1.3  1993/04/19  15:03:49  devrcs
# 	Used ld -s instead of strip which used to be the Mach2.5
# 	strip command (The OSF/1 one would fail).
# 	[93/04/07            bernadat]
# 
# 	*** empty log message ***
# 	[1993/03/24  16:29:06  david]
# 
# 	Use ${STRIP} instead of strip
# 	[93/03/23            bernadat]
# 
# 	Boot didnt work: fixed .c.o rule to convert
# 	data sections to text ones.
# 
# 	Use strip to minimize boot for floppies (8K)
# 	[93/03/18            bernadat]
# 
# 	Rewritten for ODE 2.1.1.
# 	[1993/03/16  11:23:49  gm]
# 
# 	Corrected to build.
# 	[1993/02/11  14:36:40  bruel]
# 
# 	Fixed path for a.out strip command.
# 	[93/01/27            bernadat]
# 
# 	Added rules to build remote_boot.
# 	Removed rules to build slave_boot_code.c in mp/
# 	[92/12/18            bernadat]
# 
# Revision 1.2  1992/12/16  10:21:25  devrcs
# 	Fix up phillipe's long broken dependency information !
# 	[1992/12/11  21:55:09  david]
# 
# 	Updated for ODE build tools
# 	[1992/12/11  18:40:13  david]
# 
# Revision 1.3  1992/10/21  10:28:01  devrcs
# 	updated to NORMA_MK12
# 	[1992/08/25  12:05:22  david]
# 
# Revision 1.2  1992/06/15  14:39:18  devrcs
# 	New for NORMA_MK10 release
# 	[1992/05/07  16:39:39  david]
# 
# 	Changes to generate in the obj directory, and use kernel includes
# 	Copied from Main line
# 	[92/03/19            bernadat]
# 
# Revision 2.2  92/04/04  11:33:46  rpd
# 	Fabricated for 3.0
# 	[92/03/30            mg32]
# 
# $EndLog$

USE_STATIC_LIBRARIES 	=
OBJECT_FORMAT 		= ELF

PROGRAMS 		= bootprog rbootprog
bootprog_LDFLAGS 	= -nostdlib -Ttext 0
rbootprog_LDFLAGS 	= -nostdlib -Ttext 0
OTHERS 			= boot rboot
LIBS			= -lboot
INCFLAGS		= -I../include -I../../../../file_systems

ILIST 			= boot rboot
IDIR 			= /stand/

${PROGRAMS}: 		../lib/libboot.a

SETUP_PROGRAMS 		= rmelfhdr

# ugly workaround for common makefile problem

.if ((${.TARGETS} == "setup_all") || (${.TARGETS} == "setup_rmelfhdr"))
PROGRAMS		=$ {SETUP_PROGRAMS}
.else
DEPENDENCIES 		=
CFLAGS 			= ${${OBJECT_FORMAT}_EXTRA_WARNINGS} -m386 -DPRIMARY=1
rboot.S_CFLAGS 		= ${CFLAGS} -DREMOTE=1
.endif

rmelfhdr_OFILES 	= rmelfhdr.o

# start.o should be first, table.o should be second

OBJS 			= start.o table.o sys.o boot.o
ROBJS 			= start.o table.o sys.o rboot.o
bootprog_OFILES 	= ${OBJS}
rbootprog_OFILES 	= ${ROBJS}
SOBJS 			= start.o
COBJS 			= table.o boot.o rboot.o sys.o

MDINCFLAGS 		= -I.

bootprog: ${bootprog_OFILES}
	${_LD_} ${_LDFLAGS_} -o ${.TARGET}.X ${bootprog_OFILES} ${_LIBS_}
	${MV} ${.TARGET}.X ${.TARGET}

rbootprog: ${rbootprog_OFILES}
	${_LD_} ${_LDFLAGS_} -o ${.TARGET}.X ${rbootprog_OFILES} ${_LIBS_}
	${MV} ${.TARGET}.X ${.TARGET}

.include <${RULES_MK}>

# ykpark : (repalce rmelfhdr to objcopy below : I was working on Redhat 5.2/x86) : 
# ${TARGET_EXEC_PREFIX}rmelfhdr ${.TARGET}prog ${.TARGET}
${OTHERS}: $${.TARGET}prog
	objcopy -O binary ${.TARGET}prog ${.TARGET}
	@ls -l $@prog $@

${SOBJS}: $${.TARGET:.o=.s}
	${RM} ${_RMFLAGS_} ${.TARGET:.o=.S}
	${CP} ${${.TARGET:.o=.s}:P} ${.TARGET:.o=.S}
	${_CC_} -D__LANGUAGE_ASSEMBLY ${_CCFLAGS_} -c ${.TARGET:.o=.S}

# ykpark (insert folling to below : to locate Gdtr in table.c at 0x100(BOOTSEG) address ) : 
# sed 's/\.rodata/\.text/' ${.TARGET:.o=.S1} > ${.TARGET:.o=.S}

${COBJS}: $${.TARGET:.o=.c}
	${RM} ${_RMFLAGS_} ${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -S ${${.TARGET:.o=.c}:P}
	sed '/^	\.align	2$$/d' ${.TARGET:.o=.s} > ${.TARGET:.o=.S1}
	sed 's/\.rodata/\.text/' ${.TARGET:.o=.S1} > ${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -c ${.TARGET:.o=.S}

rboot.c: boot.c
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${boot.c:P} ${.TARGET}

.if exists(depend.mk)
.include "depend.mk"
.endif
