#
# Mach Operating System
# Copyright (c) 1993 Carnegie Mellon University
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
# $Log:	Makefile,v $
# Revision 2.2  94/03/25  18:22:53  mrt
# 	Created.
# 	[93/11/16            mrt]
# 

#  This makefile creates a symlink named machine to the
#  directory ${KERN_MACHINE_DIR} in the exports/usr/include
#  area. ${KERN_MACHINE_DIR} is usually the same as ${target_machine}
#  but is given a variable of its own, for machines where it might be
#  different.
#


EXPINC_SUBDIRS	= ${KERN_MACHINE_DIR}
EXPORT_USING_TAR=

OTHERS		= machine

INCLUDES	= ${OTHERS}

EXPDIR		= /poe/

.include <${RULES_MK}>

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	${LN} -s ${KERN_MACHINE_DIR} machine
