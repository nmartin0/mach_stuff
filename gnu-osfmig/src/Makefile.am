## Process this file with automake to produce Makefile.in.

# gnu-osfmig 
# by Jose E. Marchesi <jemarch@gnu.org>

# Time-stamp: "2003-08-25 19:47:51 jemarch"

libexec_PROGRAMS = migcom
bin_SCRIPTS = mig

migcom_SOURCES = alloc.h error.c error.h global.c global.h header.c      \
		 lexxer.h lexxer.c mig.c parser.h parser.c               \
		 routine.c routine.h server.c statement.c statement.h    \
		 strdefs.h string.c type.c type.h user.c utils.c utils.h \
		 write.h
migcom_LDADD = -lfl

INCLUDES += -I$(srcdir)/@CPUTYPE@ -I@MACH_INCLUDE@ -I@MACH_INCLUDE@/@CPUTYPE@
