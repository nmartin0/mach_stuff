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

# Makefile for MACH3.0+BSD4.3 /lib/libc.a

LIB=	${INSTALL_TREE}/lib
USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN2=	${INSTALL_TREE}/usr/man/man2
MAN3=	${INSTALL_TREE}/usr/man/man3

ASFLAGS= -MD -DNAMED
CFLAGS=	-O -MD -DNAMED

INCS=	a.out.h assert.h ctype.h disktab.h fcntl.h fstab.h grp.h lastlog.h \
	memory.h mntent.h mp.h mtab.h ndbm.h netdb.h nlist.h pwd.h resolv.h \
	setjmp.h sgtty.h stab.h stdio.h stdarg.h stdlib.h string.h strings.h \
	struct.h sysexits.h time.h ttyent.h tzfile.h utmp.h varargs.h vfont.h

BSD41=	ftime.o getpw.o gtty.o nice.o pause.o rand.o reset.o stty.o tell.o \
	times.o utime.o vlimit.o vtimes.o

BSD43=	ecvt.o gcvt.o sibuf.o sobuf.o strout.o

SYS5=	getopt.o memccpy.o memchr.o memcmp.o memcpy.o memset.o strchr.o \
	strcspn.o strpbrk.o strrchr.o strspn.o strtok.o tmpnam.o strcatn.o \
	strcmpn.o strcpyn.o

GEN=	_setjmp.o abort.o abs.o alarm.o alloca.o atof.o atoi.o atol.o bcmp.o \
	bcopy.o bzero.o calloc.o closedir.o crypt.o ctime.o ctype_.o \
	disklabel.o errlst.o execvp.o fabs.o fakcu.o ffs.o frexp.o fstab.o \
	getenv.o getgrent.o getgrgid.o getgrnam.o getlogin.o getpass.o \
	getpwent.o getpwnamuid.o getttyent.o getttynam.o getusershell.o \
	getwd.o index.o initgroups.o insque.o isatty.o ldexp.o malloc.o \
	mktemp.o modf.o ndbm.o nlist.o opendir.o perror.o popen.o psignal.o \
	putenv.o qsort.o random.o readdir.o regerror.o regex.o regexp.o \
	regsub.o remque.o rewinddir.o rindex.o scandir.o seekdir.o setegid.o \
	setenv.o seteuid.o setgid.o setjmp.o setjmperr.o setrgid.o setruid.o \
	setuid.o siginterrupt.o siglist.o signal.o sleep.o strcasecmp.o \
	strcat.o strcmp.o strcpy.o strdup.o strlen.o strncat.o strncmp.o \
	strncpy.o strtod.o swab.o syslog.o system.o telldir.o time.o \
	timezone.o ttyname.o ttyslot.o ualarm.o usleep.o valloc.o

INET=	inet_addr.o inet_network.o inet_netof.o inet_ntoa.o inet_lnaof.o \
	inet_makeaddr.o

NET=	gethostent.o gethostnamaddr.o getnetbyaddr.o getnetent.o \
	getnetbyname.o getproto.o getprotoent.o getprotoname.o getservent.o \
	getservbyport.o getservbyname.o herror.o htonl.o htons.o ntohl.o \
	ntohs.o rcmd.o rexec.o ruserpass.o res_comp.o res_debug.o res_init.o \
	res_mkquery.o res_query.o res_send.o sethostent.o

NS=	ns_addr.o ns_ntoa.o

STDIO=	clrerr.o doprnt.o doscan.o exit.o fdopen.o fgetc.o fgets.o filbuf.o \
	findiop.o flsbuf.o fopen.o fprintf.o fputc.o fputs.o fread.o freopen.o \
	fseek.o ftell.o fwrite.o getchar.o gets.o getw.o printf.o putchar.o \
	puts.o putw.o rew.o scanf.o setbuf.o setbuffer.o sprintf.o ungetc.o \
	vfprintf.o vprintf.o vsprintf.o

# Standard syscall's.
SYS=	_exit.o accept.o access.o acct.o adjtime.o bind.o brk.o cerror.o \
	chdir.o chmod.o chown.o chroot.o close.o connect.o creat.o \
	dup.o dup2.o execl.o execle.o execv.o execve.o exect.o fchmod.o \
	fchown.o fcntl.o flock.o fork.o fstat.o fsync.o ftruncate.o \
	getdtablesize.o getegid.o geteuid.o getgid.o getgroups.o \
	gethostid.o gethostname.o getitimer.o getpagesize.o \
	getpeername.o getpgrp.o getpid.o getppid.o getpriority.o \
	getrlimit.o getrusage.o getsockname.o getsockopt.o \
	gettimeofday.o getuid.o ioctl.o kill.o killpg.o link.o \
	listen.o lseek.o lstat.o mkdir.o mknod.o mount.o open.o \
	pipe.o profil.o ptrace.o quota.o read.o readlink.o readv.o \
	reboot.o recv.o recvfrom.o recvmsg.o rename.o rmdir.o sbrk.o \
	select.o send.o sendmsg.o sendto.o setgroups.o sethostid.o \
	sethostname.o setitimer.o setpgrp.o setpriority.o setquota.o \
	setregid.o setreuid.o setrlimit.o setsockopt.o settimeofday.o \
	shutdown.o sigblock.o sigpause.o sigreturn.o sigsetmask.o \
	sigstack.o sigvec.o socket.o socketpair.o stat.o swapon.o \
	symlink.o sync.o syscall.o truncate.o umask.o umount.o \
	unlink.o utimes.o vfork.o vhangup.o wait.o wait3.o \
	write.o writev.o

# These are the other syscall's in the CMU Unix Server.
CMUSYS=	afs_call.o async_daemon.o exportfs.o fstatfs.o getaid.o \
	getdirentries.o getdomainname.o getdopt.o getmodes.o icreate.o \
	idec.o iinc.o iopen.o iread.o iwrite.o madvise.o mincore.o mmap.o \
	mprotect.o mremap.o munmap.o nfs_getfh.o nfs_svc.o pioctl.o \
	quotactl.o rpause.o setaid.o setdomainname.o setdopt.o setmodes.o \
	setpag.o sstk.o statfs.o table.o unmount.o vfsmount.o xutimes.o

MAN2S=	accept access acct adjtime bind brk chdir chmod chown chroot close \
	connect creat dup execve fcntl flock fork fsync getdirentries \
	getdomainname getdtablesize getgid getgroups gethostid gethostname \
	getitimer getpagesize getpeername getpgrp getpid getpriority \
	getrlimit getrusage getsockname getsockopt gettimeofday getuid ioctl \
	kill killpg link listen lseek mkdir mknod mount open pipe profil \
	ptrace quota read readlink reboot recv rename rmdir rpause select \
	send setgroups setpgrp setquota setregid setreuid shutdown sigblock \
	sigpause sigreturn sigsetmask sigstack sigvec socket socketpair stat \
	statfs swapon symlink sync syscall table truncate umask unlink \
	unmount utimes vfork vfsmount vhangup wait write

MAN3S=	abort abs abspath accept access acct adjtime alarm assert atof \
	atoh atot bind bindresvport boolarg bstring byteorder chdir chmod \
	chown chrarg ci close compat-sys5 concat connect convert_ts_to_tv \
	creat crypt ctime ctype del directory doublearg dup ecvt editor \
	end env_conn env_list env_port env_string execl execve exit expand \
	fclose fcntl fdate ferror fgetpass fgets filecopy flock fmap fold \
	fopen fork fread frexp fseek fsync getacent getbool getc getchr \
	getdirentries getdiskbyname getdomainname getdouble getdtablesize \
	getenv getfsent getgid getgrent getgroups gethostbyname gethostid \
	gethostname getint getitimer getlogin getmntent getname getnetent \
	getoct getopt getpagesize getpass getpeername getpgrp getpid \
	getpriority getprotoent getpw getpwent getrlimit getrpcent \
	getrpcport getrusage gets getservent getsockname getsockopt getstr \
	gettimeofday getttyent getuid getusershell getwd gtime hardcrypt \
	inet initgroups insque intarg ioctl ipcx kill killpg link listen \
	lseek mach_error mach_exception mach_init makepath malloc mkdir \
	mknod mkstemp mktemp monitor mount movefile mp ndbm netname nice \
	nlist ns nxtarg octarg oklogin okpassword open openp parsedate \
	path pathof pause perror pipe plot popen printf profil prstab \
	psignal ptrace putc puts qsort quit quota rand random rcmd regex \
	regexp resolver rexec run salloc scandir scanf service_checkin \
	setbuf setjmp setuid siginterrupt signal sindex skipto sleep \
	slot_name srchscore stablk stabsearch stdio stlmatch strarg string \
	stty swab syslog system termcap threads time times ttyname ualarm \
	ungetc usleep utime valloc varargs vlimit vtimes wantread wantwrite \
	warnlogin xdr

OBJS=	${BSD41} ${BSD43} ${SYS5} ${GEN} ${INET} ${NET} ${NS} ${STDIO} \
	${SYS} ${CMUSYS}

VPATH=	${TARGET}:src:regexp

compile: profiled libc.a libc_p.a

profiled:
	mkdir profiled

libc.a:	${OBJS}
	ls *.o | sort -t/ +1 >t1
	ar cr libc.a `lorder \`cat t1\` | tsort`
	ar ma findiop.o libc.a exit.o
	ar ma exit.o libc.a fakcu.o
	rm -f t1

libc_p.a: ${OBJS}
	ls profiled/*.o | sort -t/ +1 > t1
	ar cr libc_p.a `lorder \`cat t1\` | tsort`
	ar ma findiop.o libc_p.a exit.o
	ar ma exit.o libc_p.a fakcu.o
	rm -f t1

errlst.o: errlst.c
	cc -S errlst.c
	ed - <${TARGET}/:errfix errlst.s
	as -o errlst.o errlst.s
	cp errlst.o profiled/errlst.o
	rm -f errlst.s

.c.o:
	@echo "cc ${CFLAGS} -c $*.c"
	@cc -p ${CFLAGS} -c $*.c
	@ld -X -o profiled/$*.o -r $*.o
	@cc ${CFLAGS} -c $*.c
	@ld -x -r $*.o
	@mv a.out $*.o

.S.o:
	@echo "cc ${ASFLAGS} -c $*.S"
	@cc -DPROF ${ASFLAGS} -c $*.S
	@ld -X -o profiled/$*.o -r $*.o
	@cc ${ASFLAGS} -c $*.S
	@ld -x -r $*.o
	@mv a.out $*.o

install:
	install -c -m 444 -o bin -g bin libc.a ${LIB}/libc.a
	ranlib ${LIB}/libc.a
	install -c -m 444 -o bin -g bin libc_p.a ${USRLIB}/libc_p.a
	ranlib ${USRLIB}/libc_p.a

includes:
	@cd include; install -p -v -c -m 444 -o bin -g bin ${INCS} ${INCLUDE}
	@cd regexp; install -p -v -c -m 444 -o bin -g bin regexp.h ${INCLUDE}

manpages:
	@for i in ${MAN2S}; do \
	  install -c -v -o bin -g bin -m 444 man/$$i.man ${MAN2}/$$i.2 ;\
	done
	@for i in ${MAN3S}; do \
	  install -c -v -o bin -g bin -m 444 man/$$i.man ${MAN3}/$$i.3 ;\
	done
	rm -f ${MAN3}/atoi.3; ln -s atof.3 ${MAN3}/atoi.3
	rm -f ${MAN3}/atol.3; ln -s atof.3 ${MAN3}/atol.3
	rm -f ${MAN3}/bcmp.3; ln -s bstring.3 ${MAN3}/bcmp.3
	rm -f ${MAN3}/bcopy.3; ln -s bstring.3 ${MAN3}/bcopy.3
	rm -f ${MAN3}/bzero.3; ln -s bstring.3 ${MAN3}/bzero.3
	rm -f ${MAN3}/ffs.3; ln -s bstring.3 ${MAN3}/ffs.3
	rm -f ${MAN3}/htonl.3; ln -s byteorder.3 ${MAN3}/htonl.3
	rm -f ${MAN3}/htons.3; ln -s byteorder.3 ${MAN3}/htons.3
	rm -f ${MAN3}/ntohl.3; ln -s byteorder.3 ${MAN3}/ntohl.3
	rm -f ${MAN3}/ntohs.3; ln -s byteorder.3 ${MAN3}/ntohs.3
	rm -f ${MAN3}/memccpy.3; ln -s compat-sys5.3 ${MAN3}/memccpy.3
	rm -f ${MAN3}/memchr.3; ln -s compat-sys5.3 ${MAN3}/memchr.3
	rm -f ${MAN3}/memcmp.3; ln -s compat-sys5.3 ${MAN3}/memcmp.3
	rm -f ${MAN3}/memcpy.3; ln -s compat-sys5.3 ${MAN3}/memcpy.3
	rm -f ${MAN3}/memset.3; ln -s compat-sys5.3 ${MAN3}/memset.3
	rm -f ${MAN3}/strchr.3; ln -s compat-sys5.3 ${MAN3}/strchr.3
	rm -f ${MAN3}/strcspn.3; ln -s compat-sys5.3 ${MAN3}/strcspn.3
	rm -f ${MAN3}/strrchr.3; ln -s compat-sys5.3 ${MAN3}/strrchr.3
	rm -f ${MAN3}/strspn.3; ln -s compat-sys5.3 ${MAN3}/strspn.3
	rm -f ${MAN3}/strtok.3; ln -s compat-sys5.3 ${MAN3}/strtok.3
	rm -f ${MAN3}/tempnam.3; ln -s compat-sys5.3 ${MAN3}/tempnam.3
	rm -f ${MAN3}/tmpfile.3; ln -s compat-sys5.3 ${MAN3}/tmpfile.3
	rm -f ${MAN3}/tmpnam.3; ln -s compat-sys5.3 ${MAN3}/tmpnam.3
	rm -f ${MAN3}/encrypt.3; ln -s crypt.3 ${MAN3}/encrypt.3
	rm -f ${MAN3}/setkey.3; ln -s crypt.3 ${MAN3}/setkey.3
	rm -f ${MAN3}/asctime.3; ln -s ctime.3 ${MAN3}/asctime.3
	rm -f ${MAN3}/gmtime.3; ln -s ctime.3 ${MAN3}/gmtime.3
	rm -f ${MAN3}/localtime.3; ln -s ctime.3 ${MAN3}/localtime.3
	rm -f ${MAN3}/timezone.3; ln -s ctime.3 ${MAN3}/timezone.3
	rm -f ${MAN3}/tzset.3; ln -s ctime.3 ${MAN3}/tzset.3
	rm -f ${MAN3}/isalnum.3; ln -s ctype.3 ${MAN3}/isalnum.3
	rm -f ${MAN3}/isalpha.3; ln -s ctype.3 ${MAN3}/isalpha.3
	rm -f ${MAN3}/isascii.3; ln -s ctype.3 ${MAN3}/isascii.3
	rm -f ${MAN3}/iscntrl.3; ln -s ctype.3 ${MAN3}/iscntrl.3
	rm -f ${MAN3}/isdigit.3; ln -s ctype.3 ${MAN3}/isdigit.3
	rm -f ${MAN3}/islower.3; ln -s ctype.3 ${MAN3}/islower.3
	rm -f ${MAN3}/isprint.3; ln -s ctype.3 ${MAN3}/isprint.3
	rm -f ${MAN3}/ispunct.3; ln -s ctype.3 ${MAN3}/ispunct.3
	rm -f ${MAN3}/isspace.3; ln -s ctype.3 ${MAN3}/isspace.3
	rm -f ${MAN3}/isupper.3; ln -s ctype.3 ${MAN3}/isupper.3
	rm -f ${MAN3}/isxdigit.3; ln -s ctype.3 ${MAN3}/isxdigit.3
	rm -f ${MAN3}/toascii.3; ln -s ctype.3 ${MAN3}/toascii.3
	rm -f ${MAN3}/tolower.3; ln -s ctype.3 ${MAN3}/tolower.3
	rm -f ${MAN3}/toupper.3; ln -s ctype.3 ${MAN3}/toupper.3
	rm -f ${MAN3}/closedir.3; ln -s directory.3 ${MAN3}/closedir.3
	rm -f ${MAN3}/dirfd.3; ln -s directory.3 ${MAN3}/dirfd.3
	rm -f ${MAN3}/opendir.3; ln -s directory.3 ${MAN3}/opendir.3
	rm -f ${MAN3}/readdir.3; ln -s directory.3 ${MAN3}/readdir.3
	rm -f ${MAN3}/rewinddir.3; ln -s directory.3 ${MAN3}/rewinddir.3
	rm -f ${MAN3}/seekdir.3; ln -s directory.3 ${MAN3}/seekdir.3
	rm -f ${MAN3}/telldir.3; ln -s directory.3 ${MAN3}/telldir.3
	rm -f ${MAN3}/fcvt.3; ln -s ecvt.3 ${MAN3}/fcvt.3
	rm -f ${MAN3}/gcvt.3; ln -s ecvt.3 ${MAN3}/gcvt.3
	rm -f ${MAN3}/edata.3; ln -s end.3 ${MAN3}/edata.3
	rm -f ${MAN3}/etext.3; ln -s end.3 ${MAN3}/etext.3
	rm -f ${MAN3}/exec.3; ln -s execl.3 ${MAN3}/exec.3
	rm -f ${MAN3}/execle.3; ln -s execl.3 ${MAN3}/execle.3
	rm -f ${MAN3}/execlp.3; ln -s execl.3 ${MAN3}/execlp.3
	rm -f ${MAN3}/exect.3; ln -s execl.3 ${MAN3}/exect.3
	rm -f ${MAN3}/execv.3; ln -s execl.3 ${MAN3}/execv.3
	rm -f ${MAN3}/execvp.3; ln -s execl.3 ${MAN3}/execvp.3
	rm -f ${MAN3}/fflush.3; ln -s fclose.3 ${MAN3}/fflush.3
	rm -f ${MAN3}/clearerr.3; ln -s ferror.3 ${MAN3}/clearerr.3
	rm -f ${MAN3}/feof.3; ln -s ferror.3 ${MAN3}/feof.3
	rm -f ${MAN3}/fileno.3; ln -s ferror.3 ${MAN3}/fileno.3
	rm -f ${MAN3}/fdopen.3; ln -s fopen.3 ${MAN3}/fdopen.3
	rm -f ${MAN3}/freopen.3; ln -s fopen.3 ${MAN3}/freopen.3
	rm -f ${MAN3}/fwrite.3; ln -s fread.3 ${MAN3}/fwrite.3
	rm -f ${MAN3}/ldexp.3; ln -s frexp.3 ${MAN3}/ldexp.3
	rm -f ${MAN3}/modf.3; ln -s frexp.3 ${MAN3}/modf.3
	rm -f ${MAN3}/ftell.3; ln -s fseek.3 ${MAN3}/ftell.3
	rm -f ${MAN3}/rewind.3; ln -s fseek.3 ${MAN3}/rewind.3
	rm -f ${MAN3}/fgetc.3; ln -s getc.3 ${MAN3}/fgetc.3
	rm -f ${MAN3}/getchar.3; ln -s getc.3 ${MAN3}/getchar.3
	rm -f ${MAN3}/getw.3; ln -s getc.3 ${MAN3}/getw.3
	rm -f ${MAN3}/setenv.3; ln -s getenv.3 ${MAN3}/setenv.3
	rm -f ${MAN3}/unsetenv.3; ln -s getenv.3 ${MAN3}/unsetenv.3
	rm -f ${MAN3}/endfsent.3; ln -s getfsent.3 ${MAN3}/endfsent.3
	rm -f ${MAN3}/getfsfile.3; ln -s getfsent.3 ${MAN3}/getfsfile.3
	rm -f ${MAN3}/getfsspec.3; ln -s getfsent.3 ${MAN3}/getfsspec.3
	rm -f ${MAN3}/getfstype.3; ln -s getfsent.3 ${MAN3}/getfstype.3
	rm -f ${MAN3}/setfsent.3; ln -s getfsent.3 ${MAN3}/setfsent.3
	rm -f ${MAN3}/endgrent.3; ln -s getgrent.3 ${MAN3}/endgrent.3
	rm -f ${MAN3}/getgrgid.3; ln -s getgrent.3 ${MAN3}/getgrgid.3
	rm -f ${MAN3}/getgrnam.3; ln -s getgrent.3 ${MAN3}/getgrnam.3
	rm -f ${MAN3}/setgrent.3; ln -s getgrent.3 ${MAN3}/setgrent.3
	rm -f ${MAN3}/setgrfile.3; ln -s getgrent.3 ${MAN3}/setgrfile.3
	rm -f ${MAN3}/endhostent.3; ln -s gethostbyname.3 ${MAN3}/endhostent.3
	rm -f ${MAN3}/gethostbyaddr.3; ln -s gethostbyname.3 ${MAN3}/gethostbyaddr.3
	rm -f ${MAN3}/gethostent.3; ln -s gethostbyname.3 ${MAN3}/gethostent.3
	rm -f ${MAN3}/sethostent.3; ln -s gethostbyname.3 ${MAN3}/sethostent.3
	rm -f ${MAN3}/sethostfile.3; ln -s gethostbyname.3 ${MAN3}/sethostfile.3
	rm -f ${MAN3}/endnetent.3; ln -s getnetent.3 ${MAN3}/endnetent.3
	rm -f ${MAN3}/getnetbyaddr.3; ln -s getnetent.3 ${MAN3}/getnetbyaddr.3
	rm -f ${MAN3}/getnetbyname.3; ln -s getnetent.3 ${MAN3}/getnetbyname.3
	rm -f ${MAN3}/setnetent.3; ln -s getnetent.3 ${MAN3}/setnetent.3
	rm -f ${MAN3}/endprotoent.3; ln -s getprotoent.3 ${MAN3}/endprotoent.3
	rm -f ${MAN3}/getprotobyname.3; ln -s getprotoent.3 ${MAN3}/getprotobyname.3
	rm -f ${MAN3}/getprotobynumber.3; ln -s getprotoent.3 ${MAN3}/getprotobynumber.3
	rm -f ${MAN3}/setprotoent.3; ln -s getprotoent.3 ${MAN3}/setprotoent.3
	rm -f ${MAN3}/endpwent.3; ln -s getpwent.3 ${MAN3}/endpwent.3
	rm -f ${MAN3}/getpwnam.3; ln -s getpwent.3 ${MAN3}/getpwnam.3
	rm -f ${MAN3}/getpwuid.3; ln -s getpwent.3 ${MAN3}/getpwuid.3
	rm -f ${MAN3}/setpwent.3; ln -s getpwent.3 ${MAN3}/setpwent.3
	rm -f ${MAN3}/setpwfile.3; ln -s getpwent.3 ${MAN3}/setpwfile.3
	rm -f ${MAN3}/endservent.3; ln -s getservent.3 ${MAN3}/endservent.3
	rm -f ${MAN3}/getservbyname.3; ln -s getservent.3 ${MAN3}/getservbyname.3
	rm -f ${MAN3}/getservbyport.3; ln -s getservent.3 ${MAN3}/getservbyport.3
	rm -f ${MAN3}/setservent.3; ln -s getservent.3 ${MAN3}/setservent.3
	rm -f ${MAN3}/endttyent.3; ln -s getttyent.3 ${MAN3}/endttyent.3
	rm -f ${MAN3}/getttynam.3; ln -s getttyent.3 ${MAN3}/getttynam.3
	rm -f ${MAN3}/setttyent.3; ln -s getttyent.3 ${MAN3}/setttyent.3
	rm -f ${MAN3}/endusershell.3; ln -s getusershell.3 ${MAN3}/endusershell.3
	rm -f ${MAN3}/setusershell.3; ln -s getusershell.3 ${MAN3}/setusershell.3
	rm -f ${MAN3}/addr.3; ln -s inet.3 ${MAN3}/addr.3
	rm -f ${MAN3}/inet_addr.3; ln -s inet.3 ${MAN3}/inet_addr.3
	rm -f ${MAN3}/inet_lnaof.3; ln -s inet.3 ${MAN3}/inet_lnaof.3
	rm -f ${MAN3}/inet_makeaddr.3; ln -s inet.3 ${MAN3}/inet_makeaddr.3
	rm -f ${MAN3}/inet_netof.3; ln -s inet.3 ${MAN3}/inet_netof.3
	rm -f ${MAN3}/inet_network.3; ln -s inet.3 ${MAN3}/inet_network.3
	rm -f ${MAN3}/inet_ntoa.3; ln -s inet.3 ${MAN3}/inet_ntoa.3
	rm -f ${MAN3}/network.3; ln -s inet.3 ${MAN3}/network.3
	rm -f ${MAN3}/ntoa.3; ln -s inet.3 ${MAN3}/ntoa.3
	rm -f ${MAN3}/remque.3; ln -s insque.3 ${MAN3}/remque.3
	rm -f ${MAN3}/getpwwho.3; ln -s intarg.3 ${MAN3}/getpwwho.3
	rm -f ${MAN3}/alloca.3; ln -s malloc.3 ${MAN3}/alloca.3
	rm -f ${MAN3}/calloc.3; ln -s malloc.3 ${MAN3}/calloc.3
	rm -f ${MAN3}/free.3; ln -s malloc.3 ${MAN3}/free.3
	rm -f ${MAN3}/realloc.3; ln -s malloc.3 ${MAN3}/realloc.3
	rm -f ${MAN3}/moncontrol.3; ln -s monitor.3 ${MAN3}/moncontrol.3
	rm -f ${MAN3}/monstartup.3; ln -s monitor.3 ${MAN3}/monstartup.3
	rm -f ${MAN3}/dbm_clearerr.3; ln -s ndbm.3 ${MAN3}/dbm_clearerr.3
	rm -f ${MAN3}/dbm_close.3; ln -s ndbm.3 ${MAN3}/dbm_close.3
	rm -f ${MAN3}/dbm_delete.3; ln -s ndbm.3 ${MAN3}/dbm_delete.3
	rm -f ${MAN3}/dbm_error.3; ln -s ndbm.3 ${MAN3}/dbm_error.3
	rm -f ${MAN3}/dbm_fetch.3; ln -s ndbm.3 ${MAN3}/dbm_fetch.3
	rm -f ${MAN3}/dbm_firstkey.3; ln -s ndbm.3 ${MAN3}/dbm_firstkey.3
	rm -f ${MAN3}/dbm_nextkey.3; ln -s ndbm.3 ${MAN3}/dbm_nextkey.3
	rm -f ${MAN3}/dbm_open.3; ln -s ndbm.3 ${MAN3}/dbm_open.3
	rm -f ${MAN3}/dbm_store.3; ln -s ndbm.3 ${MAN3}/dbm_store.3
	rm -f ${MAN3}/ns_addr.3; ln -s ns.3 ${MAN3}/ns_addr.3
	rm -f ${MAN3}/ns_ntoa.3; ln -s ns.3 ${MAN3}/ns_ntoa.3
	rm -f ${MAN3}/errlist.3; ln -s perror.3 ${MAN3}/errlist.3
	rm -f ${MAN3}/sys.3; ln -s perror.3 ${MAN3}/sys.3
	rm -f ${MAN3}/sys_errlist.3; ln -s perror.3 ${MAN3}/sys_errlist.3
	rm -f ${MAN3}/sys_nerr.3; ln -s perror.3 ${MAN3}/sys_nerr.3
	rm -f ${MAN3}/arc.3; ln -s plot.3 ${MAN3}/arc.3
	rm -f ${MAN3}/circle.3; ln -s plot.3 ${MAN3}/circle.3
	rm -f ${MAN3}/cont.3; ln -s plot.3 ${MAN3}/cont.3
	rm -f ${MAN3}/erase.3; ln -s plot.3 ${MAN3}/erase.3
	rm -f ${MAN3}/label.3; ln -s plot.3 ${MAN3}/label.3
	rm -f ${MAN3}/line.3; ln -s plot.3 ${MAN3}/line.3
	rm -f ${MAN3}/linemod.3; ln -s plot.3 ${MAN3}/linemod.3
	rm -f ${MAN3}/move.3; ln -s plot.3 ${MAN3}/move.3
	rm -f ${MAN3}/openpl.3; ln -s plot.3 ${MAN3}/openpl.3
	rm -f ${MAN3}/point.3; ln -s plot.3 ${MAN3}/point.3
	rm -f ${MAN3}/space.3; ln -s plot.3 ${MAN3}/space.3
	rm -f ${MAN3}/pclose.3; ln -s popen.3 ${MAN3}/pclose.3
	rm -f ${MAN3}/fprintf.3; ln -s printf.3 ${MAN3}/fprintf.3
	rm -f ${MAN3}/sprintf.3; ln -s printf.3 ${MAN3}/sprintf.3
	rm -f ${MAN3}/sys_siglist.3; ln -s psignal.3 ${MAN3}/sys_siglist.3
	rm -f ${MAN3}/fputc.3; ln -s putc.3 ${MAN3}/fputc.3
	rm -f ${MAN3}/putchar.3; ln -s putc.3 ${MAN3}/putchar.3
	rm -f ${MAN3}/putw.3; ln -s putc.3 ${MAN3}/putw.3
	rm -f ${MAN3}/fputs.3; ln -s puts.3 ${MAN3}/fputs.3
	rm -f ${MAN3}/srand.3; ln -s rand.3 ${MAN3}/srand.3
	rm -f ${MAN3}/initstate.3; ln -s random.3 ${MAN3}/initstate.3
	rm -f ${MAN3}/setstate.3; ln -s random.3 ${MAN3}/setstate.3
	rm -f ${MAN3}/srandom.3; ln -s random.3 ${MAN3}/srandom.3
	rm -f ${MAN3}/rresvport.3; ln -s rcmd.3 ${MAN3}/rresvport.3
	rm -f ${MAN3}/ruserok.3; ln -s rcmd.3 ${MAN3}/ruserok.3
	rm -f ${MAN3}/re_comp.3; ln -s regex.3 ${MAN3}/re_comp.3
	rm -f ${MAN3}/re_exec.3; ln -s regex.3 ${MAN3}/re_exec.3
	rm -f ${MAN3}/regcomp.3; ln -s regexp.3 ${MAN3}/regcomp.3
	rm -f ${MAN3}/regerror.3; ln -s regexp.3 ${MAN3}/regerror.3
	rm -f ${MAN3}/regexec.3; ln -s regexp.3 ${MAN3}/regexec.3
	rm -f ${MAN3}/regsub.3; ln -s regexp.3 ${MAN3}/regsub.3
	rm -f ${MAN3}/alphasort.3; ln -s scandir.3 ${MAN3}/alphasort.3
	rm -f ${MAN3}/fscanf.3; ln -s scanf.3 ${MAN3}/fscanf.3
	rm -f ${MAN3}/sscanf.3; ln -s scanf.3 ${MAN3}/sscanf.3
	rm -f ${MAN3}/setbuffer.3; ln -s setbuf.3 ${MAN3}/setbuffer.3
	rm -f ${MAN3}/setlinebuf.3; ln -s setbuf.3 ${MAN3}/setlinebuf.3
	rm -f ${MAN3}/_longjmp.3; ln -s setjmp.3 ${MAN3}/_longjmp.3
	rm -f ${MAN3}/_setjmp.3; ln -s setjmp.3 ${MAN3}/_setjmp.3
	rm -f ${MAN3}/longjmp.3; ln -s setjmp.3 ${MAN3}/longjmp.3
	rm -f ${MAN3}/setegid.3; ln -s setuid.3 ${MAN3}/setegid.3
	rm -f ${MAN3}/seteuid.3; ln -s setuid.3 ${MAN3}/seteuid.3
	rm -f ${MAN3}/setgid.3; ln -s setuid.3 ${MAN3}/setgid.3
	rm -f ${MAN3}/setrgid.3; ln -s setuid.3 ${MAN3}/setrgid.3
	rm -f ${MAN3}/setruid.3; ln -s setuid.3 ${MAN3}/setruid.3
	rm -f ${MAN3}/index.3; ln -s string.3 ${MAN3}/index.3
	rm -f ${MAN3}/rindex.3; ln -s string.3 ${MAN3}/rindex.3
	rm -f ${MAN3}/searchp.3; ln -s string.3 ${MAN3}/searchp.3
	rm -f ${MAN3}/strcasecmp.3; ln -s string.3 ${MAN3}/strcasecmp.3
	rm -f ${MAN3}/strcat.3; ln -s string.3 ${MAN3}/strcat.3
	rm -f ${MAN3}/strcmp.3; ln -s string.3 ${MAN3}/strcmp.3
	rm -f ${MAN3}/strcpy.3; ln -s string.3 ${MAN3}/strcpy.3
	rm -f ${MAN3}/strlen.3; ln -s string.3 ${MAN3}/strlen.3
	rm -f ${MAN3}/strncasecmp.3; ln -s string.3 ${MAN3}/strncasecmp.3
	rm -f ${MAN3}/strncat.3; ln -s string.3 ${MAN3}/strncat.3
	rm -f ${MAN3}/strncmp.3; ln -s string.3 ${MAN3}/strncmp.3
	rm -f ${MAN3}/strncpy.3; ln -s string.3 ${MAN3}/strncpy.3
	rm -f ${MAN3}/gtty.3; ln -s stty.3 ${MAN3}/gtty.3
	rm -f ${MAN3}/closelog.3; ln -s syslog.3 ${MAN3}/closelog.3
	rm -f ${MAN3}/openlog.3; ln -s syslog.3 ${MAN3}/openlog.3
	rm -f ${MAN3}/setlogmask.3; ln -s syslog.3 ${MAN3}/setlogmask.3
	rm -f ${MAN3}/tgetent.3; ln -s termcap.3 ${MAN3}/tgetent.3
	rm -f ${MAN3}/tgetflag.3; ln -s termcap.3 ${MAN3}/tgetflag.3
	rm -f ${MAN3}/tgetnum.3; ln -s termcap.3 ${MAN3}/tgetnum.3
	rm -f ${MAN3}/tgetstr.3; ln -s termcap.3 ${MAN3}/tgetstr.3
	rm -f ${MAN3}/tgoto.3; ln -s termcap.3 ${MAN3}/tgoto.3
	rm -f ${MAN3}/tputs.3; ln -s termcap.3 ${MAN3}/tputs.3
	rm -f ${MAN3}/ftime.3; ln -s time.3 ${MAN3}/ftime.3
	rm -f ${MAN3}/isatty.3; ln -s ttyname.3 ${MAN3}/isatty.3
	rm -f ${MAN3}/ttyslot.3; ln -s ttyname.3 ${MAN3}/ttyslot.3

clean:
	rm -rf profiled
	rm -f libc.a libc_p.a *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
