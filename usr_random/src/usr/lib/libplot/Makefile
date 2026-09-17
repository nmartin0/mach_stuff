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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libplot.a, etc...

USRLIB=	${INSTALL_TREE}/usr/lib

CFLAGS=	-O -MD

STD=	libf77plot libplot lib300 lib300s lib4013 lib4014 lib450 libvt0 \
	libplotaed libplotbg libplotdumb libplotgigi libplot2648 libplot7221 \
	libplotimagen libplotgrn

SUBDIRS=tf77 plot t4013 t4014 t300 t300s t450 vt0 aed bitgraph dumb gigi \
	hp2648 hp7221 imagen grn

compile: ${STD}

libf77plot:
	cd tf77; make

libplot:
	cd plot; make

lib4013:
	cd t4013; make

lib4014:
	cd t4014; make

lib300:
	cd t300; make

lib300s:
	cd t300s; make

lib450:
	cd t450; make

libvt0:
	cd vt0; make

libplotaed:
	cd aed; make

libplotbg:
	cd bitgraph; make

libplotdumb:
	cd dumb; make

libplotgigi:
	cd gigi; make

libplot2648:
	cd hp2648; make

libplot7221:
	cd hp7221; make

libplotimagen:
	cd imagen; make

libplotgrn:
	cd grn; make

install:
	for i in ${STD}; do \
	  install -c -o bin -g bin -m 644 $$i ${USRLIB}/$$i.a; \
	  ranlib ${USRLIB}/$$i.a; \
	done

includes:

manpages:

clean:
	rm -f ${STD}
	for i in ${SUBDIRS}; do (cd $$i; make clean); done
