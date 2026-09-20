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
# Revision 2.3  92/04/29  16:31:04  grm
# 	Made make install create the directories.
# 	[92/04/29  16:11:14  grm]
# 
# Revision 2.2  91/12/05  16:39:17  grm
# 	Initial version of Mdos directory structure's makefile.
# 	[91/12/05            grm]
# 
#

all: mkdirs
	cd monitor;$(MAKE)
	cd tools;$(MAKE)
	cd setup;$(MAKE)

install: mkdirs
	cd monitor;$(MAKE) install
	cd tools;$(MAKE) install
	cd setup;$(MAKE) install

clean:
	cd monitor;$(MAKE) clean
	cd tools;$(MAKE) clean
	cd setup;$(MAKE) clean

mkdirs:
	-mkdir monitor/OBJ
	-mkdir tools/OBJ
	-mkdir setup/OBJ
