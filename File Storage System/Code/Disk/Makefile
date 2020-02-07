CFLAGS = -Wall -g
sf-1920: shell.o fs.o disk.o
	gcc -g shell.o fs.o disk.o -o sf-1920 -lm -D DEBUG

shell.o: shell.c
	gcc $(CFLAGS) -c shell.c

fs.o: fs.c fs.h
	gcc $(CFLAGS) -c fs.c 

disk.o: disk.c disk.h
	gcc $(CFLAGS) -c  disk.c 

clean:
	rm sf-1920 disk.o fs.o shell.o
