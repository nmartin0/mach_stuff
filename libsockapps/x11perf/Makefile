# compile x11perf

CC = gcc
CFLAGS = -g

HEADERS = x11perf.h bitmaps.h

SRCS = x11perf.c bitmaps.c do_tests.c \
	do_simple.c do_rects.c do_valgc.c \
	do_lines.c do_segs.c \
	do_dots.c do_windows.c do_movewin.c do_text.c \
	do_copyarea.c do_arcs.c \
	do_triangles.c do_complex.c do_traps.c

OBJS = x11perf.o bitmaps.o do_tests.o \
	do_simple.o do_rects.o do_valgc.o \
	do_lines.o do_segs.o \
	do_dots.o do_windows.o do_movewin.o do_text.o \
	do_copyarea.o do_arcs.o \
	do_triangles.o do_complex.o do_traps.o

x11perf : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $> -lX11 -lm -lsockets -lthreads -lmach

clean:
	rm -f *.o *~ x11perf core netstats*
