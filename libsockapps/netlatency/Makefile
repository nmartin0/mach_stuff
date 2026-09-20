#
# Network Latency Benchmarks
# Adapted for user-level sockets library.
#

CC = gcc
CFLAGS = -g 


all: client udp_server tcp_server

client: client.o
	$(CC) $(CFLAGS) -o client client.o  -lsockets -lthreads -lmach

udp_server: udp_server.o
	$(CC) $(CFLAGS) -o udp_server udp_server.o  -lsockets -lthreads -lmach

tcp_server: tcp_server.o
	$(CC) $(CFLAGS) -o tcp_server tcp_server.o  -lsockets -lthreads -lmach

clean:
	rm -f *~ *.o netstats.* client udp_server tcp_server
