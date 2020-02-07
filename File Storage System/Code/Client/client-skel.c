#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#include "../Comms/tcp_comm.h"

#define BUFSIZE 			4096
#define USERLENGTH 			128
#define PASSLENGTH 			32
#define KEYLENGTH			8
#define IP 					100000
#define IN 					0
#define OUT 				1
#define NOTCONNECT 			-1

pthread_t * t_reader;
char userId[USERLENGTH];
char password[PASSLENGTH];
char key[KEYLENGTH];
int server;

void searchFor

int AuthUser() {

	int error = 0;

	while(!error) {
		printf("User ID: ");
		read(IN, userId, USERLENGTH);

		if(!strncmp(userId, "\n", 1)) {
			error = -1;
			break;
		}

		printf("\nPassword: ");
		read(IN, password, PASSLENGTH);

		myWriteSocket(server, userId, USERLENGTH);
		myWriteSocket(server, password, PASSLENGTH);

		myReadSocket(server, key, KEYLENGTH);

		if(!strncmp(key, "00000000", KEYLENGTH))
			printf("\nAuthentication failed. User ID or password are incorrect.\n");
		else
			break;
	}
	return error;
}

int main( int argc, char *argv[]){

	int error;

	server = myConnectSocket("localhost", IP);
	if(server == NOTCONNECT) {
		while(server == NOTCONNECT) {
			char cmd[2];
			printf("Want to try again? [y/n]\n");
			//Two bytes for the letter and the \n
			read(IN, cmd, 2);
			if(!strncmp(cmd, "y", 1))
			server = myConnectSocket("localhost", IP);
			else
			exit(0);
		}
	}
		error = AuthUser();
		if(error == -1)
			exit(0);

/*
	char message[BUFSIZE];
	while(1) {
		fgets(message, BUFSIZE, stdin);
		if(!strcmp(message, "exit\n"))
		break;
		else if(!strcmp(message, "\n")) {
			myWriteSocket(server, message, BUFSIZE);
		}
		myCloseSocket(server);
		return 0;
	}
	*/
}
