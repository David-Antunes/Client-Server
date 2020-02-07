GCCFLAGS = -Wall -g

client: client-skel.c ../Comms/tcp_comm.c
	gcc $(CFLAGS) -o client ../Comms/tcp_comm.o client-skel.c -l pthread
	
clean:
	rm  client
