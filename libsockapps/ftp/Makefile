#	@(#)Makefile	5.11 (Berkeley) 5/11/90

PROG=	ftp
SRCS=	cmds.c cmdtab.c ftp.c glob.c main.c ruserpass.c domacro.c

CFLAGS = -g
CC = gcc 
LIBS =  -lsockets -lthreads -lmach

ftp: ${SRCS:.c=.o}
	$(CC) $(CFLAGS) -o ftp ${SRCS:.c=.o} $(LIBS)

clean:
	rm -f ${SRCS:.c=.o} *~ ftp netstats*



#.EXIT:
#	md -d -u Makedep *.d

-include Makedep
