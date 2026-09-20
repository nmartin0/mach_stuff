CC = gcc 
CFLAGS = -g

ttcp: ttcp.o 
	$(CC) $(CFLAGS) -o $@ $> -lsockets -lthreads -lmach 

ttcp.o : ttcp.c

clean:
	rm -f ttcp *.o netstats* *~

