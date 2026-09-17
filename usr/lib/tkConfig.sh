# tkConfig.sh --
# 
# This shell script (for sh) is generated automatically by Tk's
# configure script.  It will create shell variables for most of
# the configuration options discovered by the configure script.
# This script is intended to be included by the configure scripts
# for Tk extensions so that they don't have to figure this all
# out for themselves.  This file does not duplicate information
# already provided by tclConfig.sh, so you may need to use that
# file in addition to this one.
#
# The information in this file is specific to a single platform.
#
# SCCS: @(#) tkConfig.sh.in 1.5 96/04/15 09:49:11

# Tk's version number.
TK_VERSION='4.2'
TK_MAJOR_VERSION='4'
TK_MINOR_VERSION='2'

# -D flags for use with the C compiler.
TK_DEFS=' -DHAVE_UNISTD_H=1 -DHAVE_LIMITS_H=1 -DSTDC_HEADERS=1 '

# The name of the Tk library (may be either a .a file or a shared library):
TK_LIB_FILE=libtk4.2.a

# Additional libraries to use when linking Tk.
TK_LIBS='-L/usr/X11R6.3/lib -lX11   -lm'

# Top-level directory in which Tcl's platform-independent files are
# installed.
TK_PREFIX='/usr'

# Top-level directory in which Tcl's platform-specific files (e.g.
# executables) are installed.
TK_EXEC_PREFIX='/usr'

# -I switch(es) to use to make all of the X11 include files accessible:
TK_XINCLUDES='-I/usr/X11R6.3/include'

# Linker switch(es) to use to link with the X11 library archive.
TK_XLIBSW='-L/usr/X11R6.3/lib -lX11'

# String to pass to linker to pick up the Tk library from its
# build directory.
TK_BUILD_LIB_SPEC='-L/usr/src/build/tk4.2/unix -ltk4.2'

# String to pass to linker to pick up the Tk library from its
# installed directory.
TK_LIB_SPEC='-L/usr/lib -ltk4.2'
