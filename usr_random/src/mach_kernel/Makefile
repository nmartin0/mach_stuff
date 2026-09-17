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

# Makefile for MACH3.0+BSD4.3 /mach_kernel

ROOT=	${INSTALL_TREE}
INCLUDE=${INSTALL_TREE}/usr/include

#CONFIG.mac2=MACMACH-mode24
CONFIG.mac2=MACMACH

MK_CONFIG=${CONFIG.${TARGET}}

KERNEL=	kernel/${MK_CONFIG}/mach_kernel

INCDIRS.mac2= mac2 mac2dev mac2if mac2os mach/mac2

INCDIRS= mach mach_debug device servers ${INCDIRS.${TARGET}}

INCS.mac2= mach/mac2/boolean.h mach/mac2/exception.h mach/mac2/frame.h \
	mach/mac2/kern_return.h mach/mac2/macserver.defs mach/mac2/reg.h \
	mach/mac2/syscall_sw.h mach/mac2/thread_status.h \
	mach/mac2/vm_param.h mach/mac2/vm_types.h mac2/act.h mac2/asm.h \
	mac2/ast.h mac2/call_frame.h mac2/clock.h mac2/cpu.h mac2/exec.h \
	mac2/mach_param.h mac2/machine_routines.h mac2/mc68851.h \
	mac2/pmap.h mac2/psl.h mac2/slots.h mac2/thread.h mac2/time_stamp.h \
	mac2/trap.h mac2/vm_tuning.h mac2/xpr.h mac2dev/adb.h mac2dev/video.h \
	mac2dev/macdisk.h mac2dev/sony.h mac2if/if_en.h mac2os/DeskBus.h \
	mac2os/Disks.h mac2os/Errors.h mac2os/Ethernet.h mac2os/Files.h \
	mac2os/Globals.h mac2os/Timer.h mac2os/Types.h mac2os/Video.h

INCS=	${INCS.${TARGET}} \
	device/cirbuf.h device/conf.h device/dev_forward.defs \
	device/dev_hdr.h device/dev_master.h device/device.defs \
	device/device_port.h device/device_reply.defs \
	device/device_request.defs device/device_types.defs \
	device/device_types.h device/device_types_kernel.h \
	device/disk_status.h device/dk.h device/ds_routines.h \
	device/errno.h device/if_ether.h device/if_hdr.h device/io_req.h \
	device/map.h device/net_io.h device/net_status.h device/param.h \
	device/tape_status.h device/tty.h device/tty_status.h \
	mach/boolean.h mach/default_pager_object.defs mach/error.h \
	mach/exc.defs mach/exception.h mach/host_info.h mach/kern_return.h \
	mach/mach.defs mach/mach_host.defs mach/mach_param.h \
	mach/mach_port.defs mach/mach_traps.h mach/mach_types.defs \
	mach/mach_types.h mach/machine.h mach/memory_object.defs \
	mach/memory_object.h mach/memory_object_default.defs mach/message.h \
	mach/mig_errors.h mach/msg_type.h mach/notify.defs mach/notify.h \
	mach/policy.h mach/port.h mach/processor_info.h mach/std_types.defs \
	mach/std_types.h mach/syscall_sw.h mach/task_info.h \
	mach/task_special_ports.h mach/thread_info.h \
	mach/thread_special_ports.h mach/thread_status.h \
	mach/thread_switch.h mach/time_value.h mach/vm_attributes.h \
	mach/vm_inherit.h mach/vm_param.h mach/vm_prot.h mach/vm_statistics.h \
	mach_debug/hash_info.h mach_debug/ipc_info.h \
	mach_debug/mach_debug.defs mach_debug/mach_debug_types.defs \
	mach_debug/mach_debug_types.h mach_debug/page_info.h \
	mach_debug/vm_info.h mach_debug/zone_info.h

compile:
	rm -f kernel/machine; ln -s ${TARGET} kernel/machine
	rm -f kernel/mach/machine; ln -s ${TARGET} kernel/mach/machine
	@LPATH="$${LPATH-/lib:/usr/lib}"; export LPATH; \
	cd kernel; make CONFIG=${MK_CONFIG}

install:
	install -c -o root -g bin -m 444 ${KERNEL} ${ROOT}/mach_kernel

includes:
	-@for i in ${INCDIRS}; do \
	  [ -d ${INCLUDE}/$$i ] || { \
	    echo "mkdir ${INCLUDE}/$$i"; \
	    mkdir ${INCLUDE}/$$i ;\
	  } ;\
	  chown bin.bin ${INCLUDE}/$$i; \
	  chmod 555 ${INCLUDE}/$$i; \
	done
	@for i in ${INCS}; do \
	  install -p -v -c -m 444 -o bin -g bin kernel/$$i ${INCLUDE}/$$i; \
	done
	@rm -f ${INCLUDE}/machine
	ln -s ${TARGET} ${INCLUDE}/machine
	@rm -f ${INCLUDE}/mach/machine
	ln -s ${TARGET} ${INCLUDE}/mach/machine

manpages:
	for i in kernel/man/*.man; do \
	  install -c -o bin -g bin -m 444 $$i \
	    ${DESTDIR}/usr/man/man2/`basename $$i .man`.2; \
	done

clean:
	rm -rf kernel/${MK_CONFIG} kernel/machine

#	cd kernel/src/config; make clean
#	cd kernel/src/mig; \
#	  rm -f mig *.o Makedep parser.c parser.h lexxer.c migcom.x
#	cd kernel/bin; rm -f config mig
#	cd kernel/lib; rm -f migcom
