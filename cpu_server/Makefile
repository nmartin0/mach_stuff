#
# Mach Operating System
# Copyright (c) 1991,1990 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
# any improvements or extensions that they make and grant Carnegie the
# rights to redistribute these changes.
#
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.3  91/02/08  16:05:46  mrt
# 	Added new Mach copyright and author notices
# 
# Revision 2.2  90/01/24  23:33:23  mrt
# 	Adapted from envmgr Makefile.
# 	[89/08/04            dlb]
# 
#
# 

#
# "make interface CPDIR=<where you're at>" to make the User (and Server)
# side of an RPC interface for the cpu_server.  This is called by lib/libmach
# to get the interface into the mach library.
#

INSTALL_DIR=$(DSTBASE)/etc
TARGET_DIRS=${INSTALL_DIR}
CPDIR= ""
COPY_H = cpu_server.h


SRCS = assign.c main.c request.c task_special_user.c
DEFS = cpu_request.defs
PRGM = cpu_server
LIBS = $(LIBTHREADS) $(LIBMACH)

include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
