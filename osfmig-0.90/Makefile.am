## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = gnu

SUBDIRS = mach i386 powerpc

EXTRA_DIST = LICENSE

libexec_PROGRAMS = migcom
bin_SCRIPTS = mig

migcom_SOURCES = alloc.h error.c error.h global.c global.h header.c      \
		 lexxer.h lexxer.l migcom.c parser.h parser.y            \
		 routine.c routine.h server.c statement.c statement.h    \
		 strdefs.h string.c type.c type.h user.c utils.c utils.h \
		 vprint.c write.h
migcom_LDADD = @ALLOCA@ @LEXLIB@

INCLUDES += -I$(srcdir)/i386
