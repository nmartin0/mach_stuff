dnl Process this file with autoconf to produce a configure script.

# gnu-osfmig 
# by Jose E. Marchesi <jemarch@gnu.org>

# Time-stamp: "2003-08-25 21:14:32 jemarch"

AC_INIT(src/mig.c)
AC_PREREQ(2.13)

AC_PROG_MAKE_SET

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(gnu-osfmig, 0.01)

dnl AC_CANONICAL_SYSTEM has been called
case "$host_cpu" in
i[[3456]]86)
  CPUTYPE=i386
  ;;
powerpc)
  CPUTYPE=ppc
  ;;
alpha)
  CPUTYPE=alpha
  ;;
hppa*)
  CPUTYPE=hp_pa
  ;;
*)
  AC_MSG_ERROR([unsupported CPU type])
  ;;
esac


AC_SUBST(CPUTYPE)


dnl Check --with-gnuosfmk
AC_ARG_WITH(gnuosfmk-headers,
[  --with-gnuosfmk-headers=LOCATION        Headers files of your gnu-osfmk.
                                          Default is /usr/local/include/mach],
    , )

if test -z "$with_gnuosfmk_headers"; then
 MACH_INCLUDE=/usr/local/include/mach
else
 MACH_INCLUDE=$with_gnuosfmk_headers
fi

AC_SUBST(MACH_INCLUDE)


dnl Checks for programs.
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL

dnl Checks for header files.
AC_STDC_HEADERS

AC_OUTPUT([Makefile src/Makefile src/mig])
