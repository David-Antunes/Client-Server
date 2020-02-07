#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#include "../Comms/tcp_comm.h"

#define BUFSIZE 4096
#define T_NUMBER 10

#define ON 1
#define OFF 0

char ServerRunning = ON;
pthread_t *t_ids;
char *t_bitmap;
unsigned int n_threads;

int searchAvailableThread() {
		for(int i = 0; i < n_threads; i++)
			if(t_bitmap[i] == OFF)
			return i;
		return -1;
}


void * ClientConsole(void *id) {

	int* args = (int*)id;

	char message[BUFSIZE];
	while(1){
		myReadSocket(args[1], message, BUFSIZE);
		if(!strcmp(message, "exit\n"))
		break;
		printf("%s", message);
		myWriteSocket(args[1], message, BUFSIZE);
	}
	t_bitmap[args[0]] = OFF;
	myCloseSocket(args[1]);
	return 0;

}


void * connectionHandler(void *id) {

	while(1){

			int port = *(int *) id;
			int socket = myAcceptServerSocket(port);
			printf("%d\n", socket);
			if(socket == OFF)
				sched_yield();
			else {
				int args[2];
				args[0] = searchAvailableThread();
				args[1] = socket;
				pthread_create(&t_ids[args[0]], NULL, ClientConsole, &args);
				t_bitmap[args[0]] = ON;
			}
		}
	return 0;
}

int main( int argc, char *argv[]){

	int port = myServerSocket(100000);

	//Creating space in memory for threads to answer requests
	t_ids = (pthread_t *) malloc(T_NUMBER*sizeof(pthread_t));

	//Bitmap to know which threads are currently running
	t_bitmap = (char *) malloc(T_NUMBER*sizeof(char));

	n_threads = T_NUMBER;

	//Initializing bitmap of threads
	for(int i = 0; i < T_NUMBER; i++)
	t_bitmap[i] = OFF;


	//Running thread for server console

	if(pthread_create(&t_ids[0], NULL, connectionHandler, &port))
		exit(1);

	//Updating bitmap for the server console thread
	t_bitmap[0] = ON;

			char message[BUFSIZE];
	while(1) {
				fgets(message, BUFSIZE, stdin);
				if(!strcmp(message, "exit\n"))
				break;
				printf("%s", message);
			}
			return 0;
}
