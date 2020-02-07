GCCFLAGS = -Wall -g

server: server-skel.c ../Comms/tcp_comm.c
	gcc $(CFLAGS) -o server ../Comms/tcp_comm.o server-skel.c -l pthread

clean:
	rm server
