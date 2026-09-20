#
# Mach Operating System
# Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
#  Software Distribution Coordinator  or  Software.Distribution@@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they make and grant Carnegie Mellon
# the rights to redistribute these changes.
#
#
# HISTORY:
# $Log:	Makefile,v $
# Revision 2.11  92/07/01  14:24:23  grm
# 	Added bios_cond to mdos.mon
# 	[92/06/30  13:45:53  grm]
# 
# Revision 2.10  92/05/22  16:01:55  grm
# 	Added bios_supervga.c
# 
# Revision 2.9  92/04/29  16:31:06  grm
# 	Changed for use with Bob's loader.
# 	[92/04/29            grm]
# 
# Revision 2.7.1.1  92/03/27  15:45:54  grm
# 	Now link mdos with libmach.a instead of libmach3_vus.a, and added
# 	the bios_supervga file.
# 	[92/03/27            grm]
# 
# Revision 2.7  92/02/14  17:44:24  grm
# 	Added CFLAGS for the OSF_SVR conditionals.
# 	[92/02/14            grm]
# 	Added bios_profile.{c,h} to makefile.  Added -DPROFILING
# 	to CFLAGS
# 	[92/02/12            grm]
# 
# Revision 2.6  92/02/03  14:24:27  rvb
# 	Clean Up
# 
# Revision 2.5  92/02/02  23:02:08  rvb
# 	Changes made for the first alpha release of Mdos.
# 	[92/01/31            grm]
# 
# Revision 2.4  91/12/06  15:27:44  grm
# 	Added bios_dosres to mdos.  Changed the libraries for linkage.
# 	[91/12/06            grm]
# 
# Revision 2.3  91/12/05  16:39:26  grm
# 	Makefile adjusted for use with the release directory structure.
# 	Changed the copyright notice.
# 	[91/12/04            grm]
# 
# Revision 2.1.1.8  91/07/16  17:52:27  grm
# 	Load mdos at 300000 instead of 200000.
# 
# Revision 2.1.1.7  91/06/28  18:58:24  grm
# 	Added bios_xms.c and bios_graphics.s
# 
# Revision 2.1.1.6  91/05/02  13:36:43  grm
# 	Put in mdsetup and mdpatch.
# 
# Revision 2.1.1.4  91/03/26  19:27:50  grm
# 	Ch ch ch changes....
# 
# Revision 2.1.1.1  90/04/07  00:42:06  grm
# 	Created this because doug had lock on Makefile
# 	and afs doesn't recognize dorr any more.
# 
# Revision 2.1.1.2  90/03/13  15:26:39  orr
# 	Added .h dependancies
# 
# Revision 2.1.1.1  90/03/12  02:22:30  orr
# 	first working version.
# 
#

BIN = ${MAKECONF:h}/../../bin
MAN = ${MAKECONF:h}/../../man
LIB = ${MAKECONF:h}/../../lib
# BSD 4.3 UX Server CFLAGS:
CFLAGS = -O -DPROFILING
# OSF Single Server CFLAGS:
#CFLAGS = -O -DPROFILING -DOSF_SVR

OBJS = v86.o bios_disk.o bios_kbd.o bios_misc.o bios_vga.o \
bios_video.o bios_mouse.o bios_emm.o bios_com.o dos_fs.o dos_disk.o \
dos_general.o pager.o excServer.o lock.o bios_graphics.o \
bios_xms.o bios_dosres.o bios_profile.o bios_supervga.o \
bios_cond.o

DEP = base.h bios.h
LDLIBS = -lthreads
#MDLIBS = -lmach3_vus -lc -lcs
MDLIBS = -lmach -lc -lcmucs
LDA=1000000

TARGS = mdos mdos.mon

all: $(TARGS)

mdos: mdos.c
	$(CC) -o $@ -DLoad_Addr=0x${LDA} mdos.c -lc -lmach -lcmucs

mdos.mon: $(OBJS)
	$(CC) $(LDFLAGS) -T${LDA} -o $@ $(OBJS) $(LDLIBS) $(MDLIBS)

v86.o: v86.c bios_profile.h $(DEP)
bios_disk.o: bios_disk.c bios_disk.h $(DEP)
bios_kbd.o: bios_kbd.c bios_kbd.h $(DEP)
bios_mouse.o: bios_mouse.c $(DEP)
bios_com.o: bios_com.c $(DEP)
bios_emm.o: bios_emm.c $(DEP)
dos_disk.o: dos_disk.c dos.h $(DEP)
dos_fs.o: dos_fs.c dos.h $(DEP)
dos_general.o: dos_general.c $(DEP)
pager.o: pager.c $(DEP)
excServer.o: excServer.c $(DEP)
bios_misc.o: bios_misc.c bios_misc.h $(DEP)
bios_video.o: bios_video.c bios_video.h bios_video_common.h bios_misc.h $(DEP)
bios_vga.o: bios_vga.c bios_video_common.h $(DEP)
bios_supervga.o: bios_supervga.c bios_video_common.h $(DEP)
bios_xms.o: bios_xms.c $(DEP)
bios_dosres.o: bios_dosres.c $(DEP)
bios_profile.o: bios_profile.c bios_profile.h $(DEP)
bios_cond.o: bios_cond.c bios_cond.h $(DEP)
lock.o: lock.s
bios_graphics.o: bios_graphics.s


install: all man
	cp -p mdos ${BIN}
	cp -p mdos.mon ${LIB}

man:
	-cp -p ${MAKECONF:h}/mdos.1 ${MAN}/mdos.1

clean:
	rm -rf ${TARGS} *.o *.BAK 

