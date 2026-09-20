CC = gcc
CFLAGS = -g 

all: client udp_server tcp_server

client: client.o
	$(CC) $(CFLAGS) -o client client.o

udp_server: udp_server.o
	$(CC) $(CFLAGS) -o udp_server udp_server.o

tcp_server: tcp_server.o
	$(CC) $(CFLAGS) -o tcp_server tcp_server.o

clean:
	rm -f *~ *.o client udp_server tcp_server
