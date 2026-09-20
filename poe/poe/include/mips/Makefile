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
# Revision 2.2  94/03/25  18:23:00  mrt
# 	[93/11/22  18:04:49  mrt]
# 
#	Removed root_device.h, added spec.h
# 	Created for odemake.
# 	[93/11/16            mrt]
# 

# This Makefile copies the header files from the poe source
# mips directory to the export directory where they can be found
# in the directory "machine". This eliminates the need for 
# machine links in the sources.

VPATH			= ../../mips

INCLUDES		= vmparam.h conf.h mips_ptrace.h spec.h

EXPDIR			= /poe/mips/

.include <${RULES_MK}>
