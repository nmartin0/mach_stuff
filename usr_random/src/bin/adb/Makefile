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

# Makefile for MACH3.0+BSD4.3 /usr/src/bin/adb

SUBDIR=	scripts adb.${target_machine} adb.${target_machine}/scripts

compile:
	for i in ${SUBDIR}; do ( cd $$i; make compile ); done

install:
	for i in ${SUBDIR}; do ( cd $$i; make install ); done

includes:
	for i in ${SUBDIR}; do ( cd $$i; make includes ); done

manpages:
	for i in ${SUBDIR}; do ( cd $$i; make manpages ); done

clean:
	for i in ${SUBDIR}; do ( cd $$i; make clean ); done
