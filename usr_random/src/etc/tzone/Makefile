# Mach Operating System
# Copyright (c) 1992 Carnegie Mellon University
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

# Makefile for MACH3.0+BSD4.3 /etc/tzone

ETC=	${INSTALL_TREE}/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

# If you want something other than Eastern United States time used on your
# system, change the line below (after finding the zone you want in the
# time zone files, or adding it to a time zone file).
# Alternately, if you discover you've got the wrong time zone, you can just
#	zic -l rightzone
LOCALTIME=	US/Eastern

# If you want code inspired by certain emerging standards, add
#	-DSTD_INSPIRED
# to the end of the "CFLAGS=" line.
CFLAGS=	-O -MD

TZCOBJS=zic.o scheck.o ialloc.o

TZDOBJS=zdump.o ialloc.o

DOCS=	Theory README Makefile newctime.3 tzfile.5 zic.8 zdump.8

# If you want to handle solar-time-based time zones, remove the
# "#define NOSOLAR" from the include file usr/include/tzfile.h.
# (and add solar87 to the DATA= line below).
#DATA=	asia australasia europe etcetera northamerica pacificnew systemv
DATA=	etcetera northamerica pacificnew systemv

compile: zdump zic

zdump:	${TZDOBJS}
	cc ${TZDOBJS} -o $@

zic:	${TZCOBJS}
	cc ${TZCOBJS} -o $@

install:
	./zic ${DATA}
	./zic -d ${DESTDIR}/etc/zoneinfo -l ${LOCALTIME} ${DATA}

includes:

manpages:
	-install -c -m 444 -o bin -g bin tzone.man ${MAN8}/tzone.8

clean:
	rm -f zdump zic *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
