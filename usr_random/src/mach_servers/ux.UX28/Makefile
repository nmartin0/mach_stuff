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

# Makefile for MACH3.0+BSD4.3 /mach_servers/ux

SERVERS=${INSTALL_TREE}/mach_servers
INCLUDE=${INSTALL_TREE}/usr/include
MAN8=	${INSTALL_TREE}/usr/man/man8

# /usr/src/Makeconf should override these
MKINC=	${INSTALL_TREE}/usr/include
MKLIB=	${INSTALL_TREE}/usr/lib

CONFIG.mac2=MACMACH

UX_CONFIG=${CONFIG.${TARGET}}

INCDIRS= bsd net netimp netinet netns sys ${TARGET}

INCS.MAC2= mac2/endian.h mac2/pcb.h mac2/queue.h mac2/reg.h mac2/vmparam.h \
	mac2/machparam.h mac2/DEFAULT.h mac2/SPECIAL.h mac2/SYS.h

INCS=	${INCS.${TARGET_MACHINE}} \
	bsd/ufs_subr.c bsd/ufs_tables.c \
	net/af.h net/dli_var.h net/if.h net/if_arp.h net/netisr.h \
	net/raw_cb.h net/route.h \
	netimp/if_imp.h netimp/if_imphost.h \
	netinet/agent_var.h netinet/bbn.h netinet/esp.h netinet/esp_cache.h \
	netinet/esp_ip.h netinet/esp_key.h netinet/esp_probe.h \
	netinet/esp_var.h netinet/group.h netinet/icmp_var.h \
	netinet/if_ether.h netinet/if_lan.h netinet/igmp.h \
	netinet/igmp_user.h netinet/igmp_var.h netinet/in.h netinet/in_pcb.h \
	netinet/in_systm.h netinet/in_var.h netinet/ip.h netinet/ip_icmp.h \
	netinet/ip_var.h netinet/pcbheadsw.h netinet/tcp.h \
	netinet/tcp_debug.h netinet/tcp_fsm.h netinet/tcp_seq.h \
	netinet/tcp_timer.h netinet/tcp_var.h netinet/tcpip.h netinet/udp.h \
	netinet/udp_var.h netinet/vmtp.h netinet/vmtp_buf.h \
	netinet/vmtp_cksum.h netinet/vmtp_ether.h netinet/vmtp_group.h \
	netinet/vmtp_ip.h netinet/vmtp_kdc.h netinet/vmtp_manager.h \
	netinet/vmtp_send.h netinet/vmtp_so.h netinet/vmtp_sys.h \
	netinet/vmtp_var.h \
	netns/idp.h netns/idp_var.h netns/ns.h netns/ns_error.h \
	netns/ns_if.h netns/ns_pcb.h netns/sp.h netns/spidp.h \
	netns/spp_debug.h netns/spp_var.h \
	sys/acct.h sys/assert.h sys/bkmac.h sys/buf.h sys/clist.h \
	sys/cmupty.h sys/coff.h sys/conf.h sys/dir.h sys/disklabel.h \
	sys/dk.h sys/dkbad.h sys/domain.h sys/enet.h sys/errno.h \
	sys/error.h sys/exec.h sys/fe.h sys/features.h sys/file.h \
	sys/file_info.h sys/filsys.h sys/fpsconf.h sys/fpsreg.h sys/fs.h \
	sys/gprof.h sys/ild.h sys/inode.h sys/ioctl.h sys/jioctl.h \
	sys/kernel.h sys/macro_help.h sys/map.h sys/mbuf.h sys/mfs.h \
	sys/mman.h sys/mount.h sys/msgbuf.h sys/mtio.h sys/namei.h \
	sys/parallel.h sys/param.h sys/proc.h sys/protosw.h sys/ptrace.h \
	sys/queue.h sys/quota.h sys/reboot.h sys/resource.h sys/rfs.h \
	sys/shared_lock.h sys/signal.h sys/signal_macros.h sys/socket.h \
	sys/socketvar.h sys/stat.h sys/synch.h sys/syscall.h \
	sys/syscall_emulation.h sys/syslog.h sys/sysnames.h sys/systm.h \
	sys/table.h sys/tablet.h sys/time.h sys/timeb.h sys/times.h \
	sys/trace.h sys/tty.h sys/ttychars.h sys/ttydev.h sys/ttyloc.h \
	sys/types.h sys/uio.h sys/un.h sys/unpcb.h sys/user.h sys/ushared.h \
	sys/ux_exception.h sys/vadvise.h sys/varargs.h sys/vcmd.h \
	sys/version.h sys/viceioctl.h sys/vicermt.h sys/vlimit.h sys/vsio.h \
	sys/vtimes.h sys/wait.h sys/xpr.h sys/zalloc.h

UX=	server/${UX_CONFIG}/vmunix

EMULATOR= emulator/emulator

compile:
	rm -f server/machine; ln -s ${target_machine} server/machine
	rm -f emulator/machine; ln -s ${target_machine} emulator/machine
	@MKINC="$${MKINC-${MKINC}}"; export MKINC; \
	MKLIB="$${MKLIB-${MKLIB}}"; export MKLIB; \
	LPATH="$${LPATH-/lib:/usr/lib}"; export LPATH; \
	cd server; make CONFIG=${UX_CONFIG}
	@MKINC="$${MKINC-${MKINC}}"; export MKINC; \
	MKLIB="$${MKLIB-${MKLIB}}"; export MKLIB; \
	LPATH="$${LPATH-/lib:/usr/lib}"; export LPATH; \
	cd emulator; make MKLIB="$$MKLIB" MKINC="$$MKINC"

install:
	install -c -o root -g bin -m 544 ${UX} ${SERVERS}/startup
	install -c -o root -g bin -m 544 ${EMULATOR} ${SERVERS}/emulator
	rm -f ${INSTALL_TREE}/vmunix; \
	  ln -s mach_servers/startup ${INSTALL_TREE}/vmunix

includes:
	-@for i in ${INCDIRS}; do \
	  [ -d ${INCLUDE}/$$i ] || { \
	    echo "mkdir ${INCLUDE}/$$i"; \
	    mkdir ${INCLUDE}/$$i ;\
	  }; \
	  chown bin.bin ${INCLUDE}/$$i; \
	  chmod 555 ${INCLUDE}/$$i; \
	done
	@for i in ${INCS}; do \
	  install -p -v -c -m 444 -o bin -g bin server/$$i ${INCLUDE}/$$i; \
	done
	@rm -f ${INCLUDE}/sys/message.h
	ln -s ../mach/message.h ${INCLUDE}/sys/message.h
	@rm -f ${INCLUDE}/sys/kern_return.h
	ln -s ../mach/kern_return.h ${INCLUDE}/sys/kern_return.h
	@rm -f ${INCLUDE}/errno.h
	ln -s sys/errno.h ${INCLUDE}/errno.h
	@rm -f ${INCLUDE}/signal.h
	ln -s sys/signal.h ${INCLUDE}/signal.h
	@rm -f ${INCLUDE}/syslog.h
	ln -s sys/syslog.h ${INCLUDE}/syslog.h
	@rm -f ${INCLUDE}/sys/dkstat.h
	ln -s dk.h ${INCLUDE}/sys/dkstat.h
	@rm -f ${INCLUDE}/machine
	ln -s ${target_machine} ${INCLUDE}/machine

manpages:
	install -c -m 444 -o bin -g bin crash.man ${MAN8}/crash.8
	install -c -m 444 -o bin -g bin reboot.man ${MAN8}/reboot.8

clean:
	rm -rf server/${UX_CONFIG} server/machine
	cd emulator; \
	  rm -rf *.o emulator uxkern Makedep emulator_base a.out \
	  mach_interface.h mach_user.c bsd_types_gen machine

