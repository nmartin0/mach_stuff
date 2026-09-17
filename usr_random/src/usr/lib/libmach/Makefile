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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libmach.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3
MANDOC=	${INSTALL_TREE}/usr/man/doc

ASFLAGS= -MD
CFLAGS=	-O -MD

VPATH=	${target_machine}:.

DOCS=	mach_bib.doc machsys.doc machuse.doc manual.doc procmanual.doc
INCS=	mach.h mach_error.h mach_init.h mach_privileged_ports.h
MACH_DEBUG_INCS= mach_debug.h
MACH_INCS= memory_object_default.h memory_object_user.h default_pager_object.h \
	exc.h notify_user.h mach_host.h mach_port.h mach_interface.h
DEVICE_INCS= device.h device_reply.h device_request.h
SERVERS_INCS= errorlib.h

GENS=	mach_user.c \
	mach_port_user.c \
	mach_host_user.c \
	device_user.c \
	device_request_user.c \
	device_reply_server.c \
	notify_user.c notify_server.c \
	seqnos_notify_server.c \
	exc_user.c exc_server.c \
	default_pager_object_user.c \
	memory_object_server.c \
	seqnos_memory_object_server.c \
	memory_object_default_server.c \
	seqnos_memory_object_default_server.c \
	mach_debug_user.c \
	mach_interface.h device.h notify_user.h memory_object_user.h \
	mach_host.h memory_object_default.h \
	mach_debug.h exc.h default_pager_object.h \
	device_reply.h device_request.h mach_port.h

OBJS_MK = mach_traps.o \
	mig_support.o mig_strncpy.o \
	mach_msg.o mach_msg_send.o mach_msg_receive.o \
	mach_msg_destroy.o mach_msg_server.o \
	device_user.o device_request_user.o device_reply_server.o \
	mach_user.o mach_port_user.o mach_host_user.o \
	mach_debug_user.o \
	default_pager_object_user.o \
	memory_object_server.o seqnos_memory_object_server.o \
	memory_object_default_server.o seqnos_memory_object_default_server.o \
	exc_user.o exc_server.o \
	notify_user.o notify_server.o seqnos_notify_server.o

OBJS_MU = mach_init.o mach_syscalls.o\
	slot_name.o error_codes.o mach_error.o \
	mach_privileged_ports.o

OBJS_LIBC = sbrk.o fork.o Ovfork.o 

OBJS = $(OBJS_MK) $(OBJS_MU) $(OBJS_LIBC)

compile: libmach.a

libmach.a: ${GENS} ${OBJS}
	ar cu $@ ${OBJS}

mach_user.c mach_interface.h: mach.defs
	mig \
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
	mig \
		-user mach_port_temp \
		-server /dev/null \
		$?
	sed	-e 's/mach_port_allocate/mig_mach_port_allocate/' \
		-e 's/mach_port_deallocate/mig_mach_port_deallocate/' \
		-e 's/mach_port_insert_right/mig_mach_port_insert_right/' \
		< mach_port_temp > mach_port_user.c
	rm -f mach_port_temp

mach_host_user.c mach_host.h: mach_host.defs
	mig -user mach_host_user.c -header mach_host.h -server /dev/null $?

device_user.c device.h: device.defs
	mig -user device_user.c -header device.h -server /dev/null $?

device_request_user.c device_request.h: device_request.defs
	mig -user device_request_user.c -server /dev/null $?

device_reply_server.c device_reply.h: device_reply.defs
	mig -user /dev/null -server device_reply_server.c $?

notify_user.c notify_server.c notify_user.h: notify.defs
	mig \
	  -header notify_user.h \
	  -user notify_user.c \
	  -server notify_server.c \
	  $?

seqnos_notify_server.c: notify.defs
	mig -DSEQNOS -header /dev/null -user /dev/null -server $@ $?

exc_user.c exc_server.c exc.h: exc.defs
	mig -user exc_user.c -server exc_server.c $?

default_pager_object_user.c default_pager_object.h: default_pager_object.defs
	mig -user default_pager_object_user.c -server /dev/null $?

memory_object_server.c memory_object_user.h: memory_object.defs
	mig \
	  -header memory_object_user.h \
	  -user /dev/null \
	  -server memory_object_server.c \
	  $?

seqnos_memory_object_server.c: memory_object.defs
	mig -DSEQNOS -header /dev/null -user /dev/null -server $@ $?

memory_object_default_server.c memory_object_default.h: memory_object_default.defs
	mig -user /dev/null -server memory_object_default_server.c $?

seqnos_memory_object_default_server.c: memory_object_default.defs
	mig -DSEQNOS -header /dev/null -user /dev/null -server $@ $?

mach_debug_user.c mach_debug.h: mach_debug.defs
	mig -user mach_debug_user.c -server /dev/null $?

install:
	install -c -m 444 -o bin -g bin libmach.a ${USRLIB}/libmach.a
	ranlib ${USRLIB}/libmach.a

includes: ${INCS} ${MACH_INCS} ${MACH_DEBUG_INCS} ${DEVICE_INCS}
	@install -p -v -c -m 444 -o bin -g bin ${INCS} ${INCLUDE}
	@install -p -v -c -m 444 -o bin -g bin ${MACH_INCS} ${INCLUDE}/mach
	@install -p -v -c -m 444 -o bin -g bin ${MACH_DEBUG_INCS} ${INCLUDE}/mach_debug
	@install -p -v -c -m 444 -o bin -g bin ${DEVICE_INCS} ${INCLUDE}/device
	@install -p -v -c -m 444 -o bin -g bin ${SERVERS_INCS} ${INCLUDE}/servers
	rm -f ${INCLUDE}/mach/mach.h; ln -s ../mach.h ${INCLUDE}/mach/mach.h

manpages:
	install -c -m 444 -o bin -g bin mach_error.man ${MAN3}/mach_error.3
	install -c -m 444 -o bin -g bin mach_init.man ${MAN3}/mach_init.3
	install -c -m 444 -o bin -g bin slot_name.man ${MAN3}/slot_name.3
	-[ -d ${MANDOC}/mach ] || mkdir ${MANDOC}/mach
	chown bin.bin ${MANDOC}/mach
	chmod 555 ${MANDOC}/mach
	install -c -o bin -g bin -m 444 ${DOCS} ${MANDOC}/mach
	-[ -d ${MANDOC}/mach/examples ] || mkdir ${MANDOC}/mach/examples
	chown bin.bin ${MANDOC}/mach/examples
	chmod 555 ${MANDOC}/mach/examples
	install -c -o bin -g bin -m 444 examples/* ${MANDOC}/mach/examples

clean:
	rm -f ${GENS} ${OBJS} libmach.a Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
