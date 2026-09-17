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

# Makefile for MACH3.0+BSD4.3 /usr/src.

# To build a system, setup the parameters in "Makeconf" then do "make BUILD".

# In general, all Makefile's in this source tree support each
# of the following five targets:
#   make compile  -- compile-only (the usual default target)
#   make includes -- install any include files (libraries only)
#   make install  -- install programs
#   make manpages -- install any manual pages
#   make clean    -- remove any created objects
#
# The /usr/src/Makefile has four additional targets:
#   make          -- display configuration parameters
#   make build    -- build an entire target system from scratch
#   make checksum -- compute checksums for all INSTALL_TREE files
#
#   make shell  -- enter a shell within the host environment
#
#   make source_tree  -- insure that the SOURCE_TREE exists
#   make install_tree -- create the INSTALL_TREE if it does not exist
#   make object_tree  -- create the OBJECT_TREE if it does not exist
#   make build_tree   -- create the BUILD_TREE if it does not exist
#
# Note that /usr/src/Makeconf contains all configuration parameters.
#
# This system is capable of being cross-compiled on a host system
# that is different from the target system.  This is accomplished
# by setting up a compilation environment where all of the tools used
# in building the target system are built with the host compiler.
# The PATH, CPATH and LPATH are restricted to this area during the
# building of all target system components.
#
# It is also possible for the these sources to be read-only.  When
# a target system is built, object files are created in an object
# tree which usually consists of symbolic links to individual source
# files.  This allows for source "shadowing", in that the files
# referenced by the symbolic links can exist anywhere...  The "fixit"
# utility is used to build entire directory structures where the
# file components are symbolic links.  Of course, the source tree
# itself can be used to hold the objects too.

SOURCE_TREE_READY=${SOURCE_TREE}/.source_tree_ready
# SOURCE_TREE_READY is a flag which, if it exists, indicates that
# the SOURCE_TREE is ready to use.

INSTALL_TREE_READY=${INSTALL_TREE}/.install_tree_ready
# INSTALL_TREE_READY is a flag which, if it exists, indicates that
# the INSTALL_TREE is ready to use.

OBJECT_TREE_READY= ${OBJECT_TREE}/.object_tree_ready
# OBJECT_TREE_READY is a flag which, if it exists, indicates that
# the OBJECT_TREE is ready to use.

BUILD_TREE_READY= ${BUILD_TREE}/.build_tree_ready
# BUILD_TREE_READY is a flag file which, if it exists, indicates that
# the BUILD_TREE holds a complete compilation environment.

SHELL= ${SHELL?${SHELL}:/bin/csh}
# The SHELL is your favorite command shell.

# System directories in the INSTALL_TREE.
SYSTEM_DIRECTORIES= \
	mach_servers dev bin lib etc tmp etc/newconfig usr usr/adm usr/etc \
	usr/include usr/include/arpa usr/include/protocols usr/include/X11 \
	usr/bin usr/bin/X11 usr/dict usr/ucb usr/lib usr/lib/X11 usr/games \
	usr/games/lib usr/games/hide usr/man usr/man/man1 usr/man/man2 \
	usr/man/man3 usr/man/man4 usr/man/man5 usr/man/man6 usr/man/man7 \
	usr/man/man8 usr/man/doc usr/man/info usr/spool usr/tmp usr/users

# These items are built with the host environment.
BUILD1_ITEMS= bin/ar bin/awk bin/cat bin/chmod bin/chown bin/cp bin/csh \
	bin/date bin/echo bin/ed bin/false bin/gas bin/grep bin/hostname \
	bin/ld bin/ln bin/ls bin/make bin/md bin/mkdir bin/mv bin/nm \
	bin/pwd bin/rm bin/sed bin/sh bin/size bin/test bin/true \
	etc/config usr/bin/gcc usr/bin/install usr/bin/join usr/bin/lex \
	usr/bin/lorder usr/bin/mig usr/bin/ranlib usr/bin/sort \
	usr/bin/touch usr/bin/tsort usr/bin/uniq usr/bin/wh usr/bin/xstrip \
	usr/bin/yacc usr/ucb/ex usr/ucb/head usr/ucb/what usr/ucb/xstr \
	lib/pcccpp

# These items are built with the compile environment.
BUILD2_ITEMS= lib/crt0 lib/libc usr/lib/lib2648 usr/lib/libcmucs \
	usr/lib/libcurses usr/lib/libdbm usr/lib/libg usr/lib/libln \
	usr/lib/libm usr/lib/libmach usr/lib/libmach_sa usr/lib/libmachid \
	usr/lib/libmachterm usr/lib/libmp usr/lib/libnetname usr/lib/libplot \
	usr/lib/libreadline usr/lib/libservice usr/lib/libsolicit \
	usr/lib/libtermcap usr/lib/libthreads usr/lib/liby

# These items are unique to mac2.
ITEMS.mac2= etc/mac2part usr/bin/macintosh

# These items make up /bin.
BIN=	bin/adb bin/ar bin/awk bin/cat bin/chmod bin/chown bin/cmp bin/cp \
	bin/cpio bin/csh bin/dd bin/df bin/diff bin/du bin/echo bin/ed \
	bin/expr bin/false bin/gas bin/grep bin/hostid bin/hostname bin/kill \
	bin/ld bin/ln bin/login bin/ls bin/mach3 bin/machine bin/machterm \
	bin/make bin/md bin/mkdir bin/mkill bin/mnice bin/ms bin/mt bin/mv \
	bin/nice bin/nm bin/od bin/pagesize bin/passwd bin/pr bin/ps \
	bin/pwd bin/rcp bin/rm bin/rmdir bin/sed bin/sh bin/size bin/strip \
	bin/stty bin/su bin/sync bin/tar bin/tee bin/test bin/time bin/tp \
	bin/true bin/wall bin/who bin/write

# These items make up /lib.
LIB=	lib/crt0 lib/libc lib/pcccpp

# These items make up /etc.
ETC=	etc/XNSrouted etc/ac etc/accton etc/badsect etc/bootp etc/bootpd \
	etc/chroot etc/clri etc/comsat etc/config etc/cron etc/dcheck \
	etc/disklabel etc/diskpart etc/dmesg etc/dump etc/dumpfs etc/edquota \
	etc/fastboot etc/fasthalt etc/fingerd etc/fsck etc/fsdb etc/fstat \
	etc/ftpd etc/gettable etc/getty etc/halt etc/htable etc/icheck \
	etc/ifconfig etc/inetd etc/init etc/kgmon etc/machid etc/makedev \
	etc/mkhosts etc/mklost+found etc/mknod etc/mkpasswd etc/mkproto \
	etc/mount etc/mpolicy etc/named etc/ncheck etc/newfs etc/newsys \
	etc/ping etc/pstat etc/quot etc/quotacheck etc/quotaon etc/rc \
	etc/reboot etc/renice etc/repquota etc/restore etc/rexecd \
	etc/rlogind etc/rmt etc/route etc/routed etc/rshd etc/rwhod etc/sa \
	etc/savecore etc/shutdown etc/slattach etc/snames etc/sony \
	etc/stacks etc/syslogd etc/talkd etc/telnetd etc/termcap etc/tftpd \
	etc/timed etc/tunefs etc/tzone etc/update etc/version etc/vipw

# These items make up /usr/bin.
USRBIN=	usr/bin/ar11 usr/bin/archie usr/bin/at usr/bin/basename usr/bin/bc \
	usr/bin/bison usr/bin/cal usr/bin/calendar usr/bin/cb \
	usr/bin/checkeq usr/bin/col usr/bin/comm usr/bin/crypt usr/bin/dc \
	usr/bin/deroff usr/bin/diction usr/bin/diff3 usr/bin/efl \
	usr/bin/emacs usr/bin/eqn usr/bin/file usr/bin/find usr/bin/fixit \
	usr/bin/gcc usr/bin/gdb usr/bin/graph usr/bin/hash_info \
	usr/bin/install usr/bin/ipc_perf usr/bin/ipc_test usr/bin/join \
	usr/bin/jpeg usr/bin/kermit usr/bin/learn usr/bin/less usr/bin/lex \
	usr/bin/lint usr/bin/look usr/bin/lorder usr/bin/m4 usr/bin/massign \
	usr/bin/mcreate usr/bin/mesg usr/bin/mig usr/bin/mkdep usr/bin/nohup \
	usr/bin/patch usr/bin/pinfo usr/bin/plot usr/bin/prof usr/bin/ptx \
	usr/bin/ranlib usr/bin/rcs usr/bin/refer usr/bin/rev usr/bin/roff \
	usr/bin/sc usr/bin/shar usr/bin/sleep usr/bin/sort usr/bin/spell \
	usr/bin/spline usr/bin/split usr/bin/struct usr/bin/sum usr/bin/sup \
	usr/bin/tabs usr/bin/tac usr/bin/tbl usr/bin/tc usr/bin/thstate \
	usr/bin/tip usr/bin/tk usr/bin/top usr/bin/touch usr/bin/tr \
	usr/bin/tsort usr/bin/tty usr/bin/uemacs usr/bin/uniq usr/bin/units \
	usr/bin/uucp usr/bin/vminfo usr/bin/vmread usr/bin/vmsearch \
	usr/bin/wh usr/bin/xlisp usr/bin/xptest usr/bin/xsend usr/bin/xstrip \
	usr/bin/yacc usr/bin/zprint

# These items make up /usr/lib.
USRLIB=	usr/lib/getNAME usr/lib/learn usr/lib/lib2648 usr/lib/libcmucs \
	usr/lib/libcurses usr/lib/libdbm usr/lib/libg usr/lib/libln \
	usr/lib/libm usr/lib/libmach usr/lib/libmach_sa usr/lib/libmachid \
	usr/lib/libmachterm usr/lib/libmp usr/lib/libnetname usr/lib/libplot \
	usr/lib/libreadline usr/lib/libservice usr/lib/libsolicit \
	usr/lib/libtermcap usr/lib/libthreads usr/lib/liby usr/lib/lpr \
	usr/lib/makekey usr/lib/man usr/lib/me usr/lib/sendmail

# These items make up /usr/ucb.
USRUCB=	usr/ucb/Mail usr/ucb/apply usr/ucb/biff usr/ucb/checknr \
	usr/ucb/clear usr/ucb/colcrt usr/ucb/colrm usr/ucb/compress \
	usr/ucb/ctags usr/ucb/dsh usr/ucb/error usr/ucb/ex usr/ucb/expand \
	usr/ucb/finger usr/ucb/fold usr/ucb/from usr/ucb/ftp usr/ucb/gcore \
	usr/ucb/gprof usr/ucb/groups usr/ucb/head usr/ucb/indent \
	usr/ucb/last usr/ucb/lastcomm usr/ucb/leave usr/ucb/lock \
	usr/ucb/logger usr/ucb/man usr/ucb/mkstr usr/ucb/more usr/ucb/msgs \
	usr/ucb/netstat usr/ucb/printenv usr/ucb/quota usr/ucb/rdist \
	usr/ucb/rlogin usr/ucb/rsh usr/ucb/ruptime usr/ucb/rwho \
	usr/ucb/sccs usr/ucb/script usr/ucb/sendbug usr/ucb/soelim \
	usr/ucb/strings usr/ucb/symorder usr/ucb/sysline usr/ucb/systat \
	usr/ucb/tail usr/ucb/talk usr/ucb/tcopy usr/ucb/telnet usr/ucb/tftp \
	usr/ucb/tn3270 usr/ucb/tset usr/ucb/ul usr/ucb/unifdef usr/ucb/users \
	usr/ucb/vacation usr/ucb/vgrind usr/ucb/vlp usr/ucb/vmstat usr/ucb/w \
	usr/ucb/wc usr/ucb/what usr/ucb/whereis usr/ucb/which usr/ucb/whoami \
	usr/ucb/whois usr/ucb/window usr/ucb/xstr usr/ucb/yes

# These items make up /usr/games.
USRGAM= usr/games/advent usr/games/arithmetic usr/games/atc \
	usr/games/backgammon usr/games/banner usr/games/battlestar \
	usr/games/bcd usr/games/bench usr/games/boggle usr/games/btlgammon \
	usr/games/canfield usr/games/cribbage usr/games/dm usr/games/factor \
	usr/games/fish usr/games/fortune usr/games/hangman usr/games/hunt \
	usr/games/jivval usr/games/mille usr/games/monop usr/games/number \
	usr/games/ppt usr/games/primes usr/games/quiz usr/games/rain \
	usr/games/robots usr/games/rogue usr/games/sail usr/games/snake \
	usr/games/trek usr/games/wargames usr/games/worm usr/games/worms \
	usr/games/wump

# These are special Mach system items.
MACH=	mach_kernel mach_servers/ux mach_servers/mach_init

# These items make up the system.
SYSTEM_ITEMS= \
	${MACH} ${BIN} ${ETC} ${LIB} ${USRBIN} ${USRLIB} ${USRUCB} \
	${USRGAM} ${ITEMS.${TARGET}}

# Setup the host environment variables.
HOST_PATH=${HOST_PATH?${HOST_PATH}:/bin:/usr/bin:/usr/ucb:/etc}
HOST_CPATH=${HOST_CPATH?${HOST_CPATH}:/usr/include}
HOST_LPATH=${HOST_LPATH?${HOST_LPATH}:"/lib:/usr/lib"}

# The default target displays configuration parameters.
config:
	-@PATH=${HOST_PATH}; export PATH; \
	  echo "These are the current configuration parameters."; \
	  echo "HOST           = ${HOST}"; \
	  echo "TARGET         = ${TARGET}"; \
	  echo "SOURCE_TREE    = ${SOURCE_TREE}"; \
	  echo "INSTALL_TREE   = ${INSTALL_TREE}"; \
	  echo "OBJECT_TREE    = ${OBJECT_TREE}"; \
	  echo "BUILD_TREE     = ${BUILD_TREE}"; \
	  echo "BUILD_PATH     = ${BUILD_PATH}"; \
	  echo "BUILD_CPATH    = ${BUILD_CPATH}"; \
	  echo "BUILD_LPATH    = ${BUILD_LPATH}"; \
	  echo "Use \"make build\" to build a system."; \
	  [ -d ${SOURCE_TREE} ] || \
	    echo "WARNING: ${SOURCE_TREE} does not exist."; \
	  [ -d ${INSTALL_TREE} ] || \
	    echo "WARNING: ${INSTALL_TREE} does not exist."; \
	  [ -d ${OBJECT_TREE} ] || \
	    echo "WARNING: ${OBJECT_TREE} does not exist."; \
	  [ -d ${BUILD_TREE} ] || \
	    echo "WARNING: ${BUILD_TREE} does not exist."

# Build a complete system from scratch.
build:	clean build_tree clean compile install includes manpages checksum

# Do a "make build" inside of a log file.
BUILD:	${OBJECT_TREE_READY}
	-@PATH=${HOST_PATH}; export PATH; \
	  rm ${BUILD_TREE_READY} ;\
	  cd ${OBJECT_TREE} ;\
	  exec script BUILD.LOG "make build"

# Compile everything in the OBJECT_TREE.
compile:
	@echo "=== Compiling ${TARGET} system..."
	-@cd ${OBJECT_TREE}; make -k compile_X11
	-@for i in ${SYSTEM_ITEMS}; do ( \
	  [ -d $$i ] && { \
	    echo "cd ${OBJECT_TREE}/$$i; make -k INSTALL_TREE=/ compile" ;\
	    cd ${OBJECT_TREE}/$$i; make -k INSTALL_TREE=/ compile ;\
	  } ;\
	); done
	-@cd ${OBJECT_TREE}; make -k compile_motif

# Install everything into the INSTALL_TREE.
install:
	@echo "=== Installing ${TARGET} system..."
	-@for i in ${SYSTEM_ITEMS}; do ( \
	  [ -d $$i ] && { \
	    echo "cd ${OBJECT_TREE}/$$i; make install" ;\
	    cd ${OBJECT_TREE}/$$i; make install ;\
	  } ;\
	); done
	-@[ -d ${OBJECT_TREE}/X11 ] && { \
	  rm -rf ${INSTALL_TREE}/usr/include/X11 ;\
	  mkdir ${INSTALL_TREE}/usr/include/X11 ;\
	  chown bin.bin ${INSTALL_TREE}/usr/include/X11 ;\
	  chmod 555 ${INSTALL_TREE}/usr/include/X11 ;\
	  rm -f ${INSTALL_TREE}/usr/lib/libX11.a ;\
	  rm -f ${INSTALL_TREE}/usr/lib/liboldX.a ;\
	  rm -f ${INSTALL_TREE}/usr/lib/libXt.a ;\
	  rm -f ${INSTALL_TREE}/usr/lib/libXext.a ;\
	  cd ${OBJECT_TREE}/X11; make DESTDIR=$$DESTDIR install ;\
	  cd ${OBJECT_TREE}/motif; make DESTDIR=$$DESTDIR install ;\
	  rm -f ${INSTALL_TREE}/usr/bin/X11/X ;\
	  echo "ln -s X${TARGET} ${INSTALL_TREE}/usr/bin/X11/X" ;\
	  ln -s X${TARGET} ${INSTALL_TREE}/usr/bin/X11/X ;\
	}
	-@for i in ${SYSTEM_DIRECTORIES}; do \
	  echo "chown bin.bin ${INSTALL_TREE}/$$i" ;\
	  chown bin.bin ${INSTALL_TREE}/$$i ;\
	  echo "chmod 555 ${INSTALL_TREE}/$$i" ;\
	  chmod 555 ${INSTALL_TREE}/$$i ;\
	done; \
	chmod 777 ${INSTALL_TREE}/tmp
	chmod 777 ${INSTALL_TREE}/usr/tmp
	chown games.bin ${INSTALL_TREE}/usr/games/hide
	chmod 550 ${INSTALL_TREE}/usr/games/hide
	chown games.bin ${INSTALL_TREE}/usr/games/lib
	chmod 750 ${INSTALL_TREE}/usr/games/lib

# Install include files into the INSTALL_TREE.
includes:
	@echo "=== Installing ${TARGET} include files..."
	@for i in ${SYSTEM_ITEMS}; do ( \
	  [ -d $$i ] && { \
	    echo "cd ${OBJECT_TREE}/$$i; make includes" ;\
	    cd ${OBJECT_TREE}/$$i; make includes ;\
	  } ;\
	); done

# Install manual pages into the INSTALL_TREE.
manpages:
	@echo "=== Installing ${TARGET} manual pages..."
	@for i in ${SYSTEM_ITEMS}; do ( \
          [ -d $$i ] && { \
	    echo "cd ${OBJECT_TREE}/$$i; make -k manpages" ;\
	    cd ${OBJECT_TREE}/$$i; make -k manpages ;\
	  } ;\
	); done
	-@cd ${OBJECT_TREE}/X11; make DESTDIR=$$DESTDIR install.man
	-@cd ${OBJECT_TREE}/motif; make DESTDIR=$$DESTDIR install.man

# Clean out the OBJECT_TREE.
clean:
	-@BUILD_PATH=${HOST_PATH}; export BUILD_PATH ;\
	  PATH=$$BUILD_PATH; export PATH ;\
	  echo "=== Cleaning ${OBJECT_TREE}..." ;\
	  for i in ${SYSTEM_ITEMS}; do ( \
	    [ -d $$i ] && { \
	      echo "cd ${OBJECT_TREE}/$$i; make -k clean" ;\
	      cd ${OBJECT_TREE}/$$i; make -k clean ;\
	    } ;\
	  ); done ;\
	  echo "cd ${OBJECT_TREE}/X11; make -k clean" ;\
	  cd ${OBJECT_TREE}/X11; make -k clean ;\
	  echo "cd ${OBJECT_TREE}/motif; make -k clean" ;\
	  cd ${OBJECT_TREE}/motif; make -k clean

# Create the ".fixit" file that is a list of checksums for all files.
checksum:
	@echo "=== Creating ${INSTALL_TREE}/.fixit"
	fixit -update ${INSTALL_TREE} \
	  <${OBJECT_TREE}/etc/newsys/${TARGET}.fixit \
	  >${INSTALL_TREE}/etc/newconfig/.fixit

# Enter shell within the host environment.
shell:
	-@BUILD_PATH=${HOST_PATH}; export BUILD_PATH ;\
	  BUILD_CPATH=${HOST_CPATH}; export BUILD_CPATH ;\
	  BUILD_LPATH=${HOST_LPATH}; export BUILD_LPATH ;\
	  PATH=$$BUILD_PATH; export PATH ;\
	  CPATH=$$BUILD_CPATH; export CPATH ;\
	  LPATH=$$BUILD_LPATH; export LPATH ;\
	  echo "=== Entering host environment." ;\
	  cd ${OBJECT_TREE}; ${SHELL} ;\
	  echo "=== Leaving host environment."

# Manually create the BUILD_TREE.
build_tree: ${BUILD_TREE_READY}

# Create the compilation environment using the host environment.  Note
# that these programs are intended to run on the host machine.  They will
# be used to build the programs that will run on the target machine.  These
# are the *only* tools that will be used to build the target system.  The
# "make" for each tool is done with all parameters needed for the
# cross-compilation from HOST to TARGET running in the BUILD_TREE.
${BUILD_TREE_READY}: ${OBJECT_TREE_READY} ${INSTALL_TREE_READY}
	-@PATH=${HOST_PATH}; export PATH ;\
	  [ -d ${BUILD_TREE} ] || echo "Missing BUILD_TREE = ${BUILD_TREE}"
	@PATH=${HOST_PATH}; export PATH; [ -d ${BUILD_TREE} ]
	-@BUILD_PATH=${HOST_PATH}; export BUILD_PATH ;\
	  BUILD_CPATH=${HOST_CPATH}; export BUILD_CPATH ;\
	  BUILD_LPATH=${HOST_LPATH}; export BUILD_LPATH ;\
	  PATH=$$BUILD_PATH; export PATH ;\
	  CPATH=$$BUILD_CPATH; export CPATH ;\
	  LPATH=$$BUILD_LPATH; export LPATH ;\
	  echo "=== Creating compilation environment: ${BUILD_TREE}"; \
	  for i in ${SYSTEM_DIRECTORIES}; do \
	    [ -d ${BUILD_TREE}/$$i ] || { \
	      echo "mkdir ${BUILD_TREE}/$$i" ;\
	      mkdir ${BUILD_TREE}/$$i ;\
	    }; \
	  done; \
	  for i in ${BUILD1_ITEMS}; do ( \
	    echo "cd ${OBJECT_TREE}/$$i" ;\
	    cd ${OBJECT_TREE}/$$i ;\
	    echo "make INSTALL_TREE=${BUILD_TREE} compile" ;\
	    make INSTALL_TREE=${BUILD_TREE} compile ;\
	    echo "make INSTALL_TREE=${BUILD_TREE} install" ;\
	    make INSTALL_TREE=${BUILD_TREE} install ;\
	  ); done
	@for i in ${SYSTEM_ITEMS}; do ( \
	  [ -d $$i ] && { \
	    echo "cd ${OBJECT_TREE}/$$i" ;\
	    cd ${OBJECT_TREE}/$$i ;\
	    echo "make INSTALL_TREE=${BUILD_TREE} includes" ;\
	    make INSTALL_TREE=${BUILD_TREE} includes ;\
	  } ;\
	); done
	@for i in ${BUILD2_ITEMS}; do ( \
	  echo "cd ${OBJECT_TREE}/$$i" ;\
	  cd ${OBJECT_TREE}/$$i ;\
	  echo "make INSTALL_TREE=${BUILD_TREE} compile" ;\
	  make INSTALL_TREE=${BUILD_TREE} compile ;\
	  echo "make INSTALL_TREE=${BUILD_TREE} install" ;\
	  make INSTALL_TREE=${BUILD_TREE} install ;\
	); done
	-@[ -d ${OBJECT_TREE}/X11 ] && { \
	  rm -rf ${BUILD_TREE}/usr/include/X11 ;\
	  ln -s ${OBJECT_TREE}/X11/X11 ${BUILD_TREE}/usr/include/X11 ;\
	  rm -rf ${BUILD_TREE}/usr/lib/libX11.a ;\
	  ln -s ${OBJECT_TREE}/X11/lib/X/libX11.a ${BUILD_TREE}/usr/lib/libX11.a ;\
	  rm -f ${BUILD_TREE}/usr/lib/liboldX.a ;\
	  ln -s ${OBJECT_TREE}/X11/lib/oldX/liboldX.a ${BUILD_TREE}/usr/lib/liboldX.a ;\
	  rm -f ${BUILD_TREE}/usr/lib/libXt.a ;\
	  ln -s ${OBJECT_TREE}/X11/lib/Xt/libXt.a ${BUILD_TREE}/usr/lib/libXt.a ;\
	  rm -f ${BUILD_TREE}/usr/lib/libXext.a ;\
	  ln -s ${OBJECT_TREE}/X11/extensions/lib/libXext.a ${BUILD_TREE}/usr/lib/libXext.a ;\
	}
	date >${BUILD_TREE_READY}

# Manually create the OBJECT_TREE.
object_tree: ${OBJECT_TREE_READY}

# Create the OBJECT_TREE tree if it does not exist.
# This is trivial if the SOURCE_TREE is also the OBJECT_TREE.
${OBJECT_TREE_READY}: ${SOURCE_TREE_READY}
	-@PATH=${HOST_PATH}; export PATH ;\
	  [ -d ${OBJECT_TREE} ] || echo "Missing OBJECT_TREE = ${OBJECT_TREE}"
	@PATH=${HOST_PATH}; export PATH; [ -d ${OBJECT_TREE} ]
	-@PATH=${HOST_PATH}; export PATH ;\
	  LPATH=${HOST_LPATH}; export LPATH ;\
	  CPATH=${HOST_CPATH}; export CPATH ;\
	  cd ${OBJECT_TREE} ;\
	  OBJECT_TREE=`pwd` ;\
	  cd ${SOURCE_TREE} ;\
	  SOURCE_TREE=`pwd` ;\
	  if [ "$$OBJECT_TREE" = "$$SOURCE_TREE" ]; then \
	    echo "=== Source tree is Object tree" ;\
	  else \
	    echo "=== Creating object tree."; \
	    cc -O $$SOURCE_TREE/usr/bin/fixit/fixit.c \
	      -o $$OBJECT_TREE/fixit; \
	    $$OBJECT_TREE/fixit \
	      -ln $$SOURCE_TREE $$OBJECT_TREE >/dev/null; \
	    rm $$OBJECT_TREE/fixit; \
	  fi ;\
	  date >${OBJECT_TREE_READY}

# Manually create the INSTALL_TREE.
install_tree: ${INSTALL_TREE_READY}

# Create the INSTALL_TREE if it does not exist.
${INSTALL_TREE_READY}:
	-@PATH=${HOST_PATH}; export PATH ;\
	  [ -d ${INSTALL_TREE} ] || echo "Missing INSTALL_TREE = ${INSTALL_TREE}"
	@PATH=${HOST_PATH}; export PATH; [ -d ${INSTALL_TREE} ]
	-@PATH=${HOST_PATH}; export PATH ;\
	  echo "=== Creating install tree."; \
	  for i in ${SYSTEM_DIRECTORIES}; do \
	    [ -d ${INSTALL_TREE}/$$i ] || { \
	      echo "mkdir ${INSTALL_TREE}/$$i"; \
	      mkdir ${INSTALL_TREE}/$$i; \
	    }; \
	  done; \
	  [ -d ${INSTALL_TREE}/users ] || { \
	    echo ln -s usr/users ${INSTALL_TREE}/users; \
	    ln -s usr/users ${INSTALL_TREE}/users; \
	  } ;\
	  date >${INSTALL_TREE_READY}

# Manually insure that the SOURCE_TREE exists.
source_tree: ${SOURCE_TREE_READY}

# Create the SOURCE_TREE_READY flag file.
${SOURCE_TREE_READY}:
	-@PATH=${HOST_PATH}; export PATH ;\
	  [ -d ${SOURCE_TREE} ] || echo "Missing SOURCE_TREE = ${SOURCE_TREE}"
	@PATH=${HOST_PATH}; export PATH; [ -d ${SOURCE_TREE} ]
	@PATH=${HOST_PATH}; export PATH; date >${SOURCE_TREE_READY}

# special rule to compile X11
compile_X11:
	@echo "=== Compiling X11"
	cd ${OBJECT_TREE}/X11/config; make imake DESTDIR=/
	cd ${OBJECT_TREE}/X11; make Makefile DESTDIR=/
	cd ${OBJECT_TREE}/X11; make Makefiles DESTDIR=/
	cd ${OBJECT_TREE}/X11; make includes DESTDIR=/
	cd ${OBJECT_TREE}/X11; make depend DESTDIR=/
	cd ${OBJECT_TREE}/X11; make -k DESTDIR=/

# special rule to compile motif
compile_motif:
	@echo "=== Compiling motif"
	cd ${OBJECT_TREE}/motif/config; make imake DESTDIR=/
	cd ${OBJECT_TREE}/motif; make Makefile DESTDIR=/
	cd ${OBJECT_TREE}/motif; make Makefiles DESTDIR=/
	cd ${OBJECT_TREE}/motif; make includes DESTDIR=/
	cd ${OBJECT_TREE}/motif; make depend DESTDIR=/
	cd ${OBJECT_TREE}/motif; make -k DESTDIR=/

frogs:
	-@for i in ${SYSTEM_DIRECTORIES}; do \
	  echo "chown bin.bin ${INSTALL_TREE}/$$i" ;\
	  chown bin.bin ${INSTALL_TREE}/$$i ;\
	  echo "chmod 555 ${INSTALL_TREE}/$$i" ;\
	  chmod 555 ${INSTALL_TREE}/$$i ;\
	done; \
	chmod 777 ${INSTALL_TREE}/tmp
	chmod 777 ${INSTALL_TREE}/usr/tmp
	chown games.bin ${INSTALL_TREE}/usr/games/hide
	chmod 550 ${INSTALL_TREE}/usr/games/hide
	chown games.bin ${INSTALL_TREE}/usr/games/lib
	chmod 750 ${INSTALL_TREE}/usr/games/lib

