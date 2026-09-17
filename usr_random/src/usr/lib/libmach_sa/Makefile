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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libmach_sa.a

USRLIB=	${INSTALL_TREE}/usr/lib

DEFS=	-DMACH_IPC_COMPAT=0 -DSTANDALONE -DTypeCheck=0

ASFLAGS= -MD ${DEFS}
CFLAGS=	-O -MD ${DEFS}

MIGFLAGS= ${DEFS}

VPATH=	${target_machine}:.

GENS=	mach_user.c \
	mach_port_user.c \
	mach_host_user.c \
	device_user.c \
	device_request_user.c \
	device_reply_server.c \
	exc_user.c exc_server.c \
	mach_debug_user.c \
	mach_interface.h \
	mach_port.h \
	mach_host.h \
	device.h \
	device_request.h \
	device_reply.h \
	exc.h \
	mach_debug.h


OBJS=	crt0.o exit.o mach_user.o mach_traps.o mig_support.o \
	mach_msg_destroy.o  mach_msg_server.o \
	bcopy.o strcmp.o strlen.o strncpy.o msg.o exc_server.o \
	mach_host_user.o mach_port_user.o mach_syscalls.o \
	device_user.o device_request_user.o device_reply_server.o \
	_setjmp.o bzero.o strcat.o strcpy.o mach_init.o mig_strncpy.o \
	mach_debug_user.o ms_mach_port_deallocate.o __main.o memcpy.o

compile: libmach_sa.a

libmach_sa.a: ${GENS} ${OBJS}
	ar cu $@ ${OBJS}

mach_user.c mach_interface.h : mach.defs
	mig ${MIGFLAGS} \
		-user mach_temp \
		-header mach_interface.h \
		-server /dev/null \
		$?
	sed 	-e 's/vm_allocate/mig_vm_allocate/' \
		-e 's/vm_map/mig_vm_map/' \
		-e 's/vm_deallocate/mig_vm_deallocate/' \
		-e 's/task_create/mig_task_create/' \
		-e 's/task_terminate/mig_task_terminate/' \
		-e 's/task_suspend/mig_task_suspend/' \
		-e 's/task_set_special_port/mig_task_set_special_port/' \
		< mach_temp > mach_user.c
	rm -f mach_temp

mach_port_user.c mach_port.h: mach_port.defs
	mig ${MIGFLAGS} \
		-user mach_port_temp \
		-server /dev/null \
		$?
	sed	-e 's/mach_port_allocate/mig_mach_port_allocate/' \
		-e 's/mach_port_deallocate/mig_mach_port_deallocate/' \
		-e 's/mach_port_insert_right/mig_mach_port_insert_right/' \
		< mach_port_temp > mach_port_user.c
	rm -f mach_port_temp

mach_host_user.c mach_host.h: mach_host.defs
	mig ${MIGFLAGS} -user mach_host_user.c -server /dev/null $?

device_user.c device.h: device.defs
	mig ${MIGFLAGS} -user device_user.c -server /dev/null $?

device_request_user.c device_request.h: device_request.defs
	mig ${MIGFLAGS} -user device_request_user.c -server /dev/null $?

device_reply_server.c device_reply.h: device_reply.defs
	mig ${MIGFLAGS} -user /dev/null -server device_reply_server.c $?

exc_user.c exc_server.c exc.h: exc.defs
	mig ${MIGFLAGS} -user exc_user.c -server exc_server.c $?

mach_debug_user.c mach_debug.h: mach_debug.defs
	mig ${MIGFLAGS} -user mach_debug_user.c -server /dev/null $?

install:
	install -c -m 444 -o bin -g bin libmach_sa.a ${USRLIB}/libmach_sa.a
	ranlib ${USRLIB}/libmach_sa.a

includes:

manpages:

clean:
	rm -f ${GENS} ${OBJS} libmach_sa.a Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep

