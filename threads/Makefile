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
# 11-Dec-92  Randall Dean (rwd) at Carnegie-Mellon University
#	Compile with -Wall.
#
# $Log:	Makefile,v $
# Revision 2.20  92/03/05  22:46:34  rpd
# 	Changed to use double-colon rules for top-level targets.
# 	[92/02/28            rpd]
# 
# Revision 2.19  92/01/23  15:22:53  rpd
# 	Revised for new Makefile organization.
# 	[92/01/16            rpd]
# 

include ${MAKETOP}Makefile-common

# find machine-dependent files in machine subdirectory

CFLAGS = $(CFLAGS) -Wall -g

VPATH = .:${cpu}

all :: libthreads.a

install :: ${INSTALLDIR}/lib/libthreads.a

release :: ${TRELEASEDIR}/lib/libthreads.a

clean ::
	${RM} ${INSTALLDIR}/lib/libthreads.a

# installation rules

${INSTALLDIR}/lib/libthreads.a : libthreads.a
	${RM} $@
	${CP} $? $@
	${CHMOD_LIB} $@

# release rules

${TRELEASEDIR}/lib/libthreads.a : ${FRELEASEDIR}/lib/libthreads.a
	${RM} $@
	${CP} $? $@

# build rules

OBJS = cthreads.o malloc.o call.o\
       mig_support.o stack.o sync.o \
       thread.o lock.o csw.o structs.o

libthreads.a : ${OBJS}
	${RM} $@
	${AR} cq $@ ${OBJS}
	${RANLIB} $@

-include Makedep
