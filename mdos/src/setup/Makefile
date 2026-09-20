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
# Revision 2.4  92/04/17  16:47:02  grm
# 	Removed useless setup code compilation and scipts.  Added code to
# 	move mdos.install.doc into doc directory.
# 	[92/04/16            grm]
# 
# Revision 2.3  92/02/02  23:02:53  rvb
# 	Changed mdset1.csh stage1.csh, mdset2.csh to stage4.csh.
# 	Removed setup.floppy.doc, and added mdos.install.ps and
# 	dospaper.ps to the document installation.
# 	[92/02/02            grm]
# 	Changes for the first alpha release of Mdos.
# 	[92/01/31            grm]
# 
# Revision 2.2  91/12/05  16:43:35  grm
# 	Initial version
# 	[91/12/04            grm]
# 
#
# Mdos setup Makefile.
#

ETC = ${MAKECONF:h}/../../etc
DOC = ${MAKECONF:h}/../../doc
LIB = ${MAKECONF:h}/../../lib
CFLAGS=-O -DMACH3
LIBS = -lmach -lcmucs

all:

install: scripts doc

scripts:
	uudecode ${MAKECONF:h}/dos.floppy.u
	uncompress dos.floppy
	cp -p dos.floppy ${LIB}/dos.floppy
	rm dos.floppy
	chmod 644 ${LIB}/dos.floppy

doc:
	cp -p ${MAKECONF:h}/mdos.install.ps ${DOC}/mdos.install.ps
	chmod 644 ${DOC}/mdos.install.ps
	cp -p ${MAKECONF:h}/mdos.install.doc ${DOC}/mdos.install.doc
	chmod 644 ${DOC}/mdos.install.doc
	cp -p ${MAKECONF:h}/dospaper.ps ${DOC}/dospaper.ps
	chmod 644 ${DOC}/dospaper.ps

clean:
	rm -rf ${TARGS} *.o *.BAK

