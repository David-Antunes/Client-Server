#include <stdio.h>
#include <strings.h>
#include <errno.h>

#include "tcp_comm.h"

#define QLEN 5

int myServerSocket( int port ){
	int s, retcode;

	struct sockaddr_in sin;
	bzero( (char *)&sin, sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons((ushort)port);

	s = socket( AF_INET, SOCK_STREAM, 0);

	if( s < 0){
		printf("unable to create serverSocket\n");
		return -1;
	}

	retcode = bind( s, (struct sockaddr *)&sin, sizeof(sin));

	if( retcode < 0){
		printf("unable to bind serverSocket to port %d\n", port);
		return -1;
	}

	retcode = listen( s, QLEN);

	if( retcode < 0){
		printf("unable to make listen on serverSocket\n");
		return -1;
	}

	return s;
}

int myAcceptServerSocket( int s){
	int csock; //int clen;
	//struct sockaddr_in csin;

	csock = accept( s, NULL, NULL);
	if( csock < 0){
		printf("unable to complete accept\n");
		perror("Accept");
		return -1;
	}
	return csock;
}

int myConnectSocket(char *hostname, int port){
	int s, retcode;
	struct sockaddr_in sin;
	struct hostent *serv;

	bzero( (char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((u_short)port);

	serv = gethostbyname(hostname);
	if( serv ==NULL){
		printf("unable to obtain an IP address for %s\n", hostname);
		return -1;
	}
	bcopy( serv->h_addr, (char *)&sin.sin_addr, serv->h_length);

	s = socket(PF_INET, SOCK_STREAM, 0);
	if( s < 0) {
		printf("unable to create client socket\n");
		return -1;
	}
	retcode = connect( s, (struct sockaddr *)&sin, sizeof(sin));
	if(retcode <0) {
		printf("unable to connect to host %s, port %d\n",
		hostname, port);
		return -1;
	}else return s;
}

int myWriteSocket( int s, char *buf, int nw){
	return write(s, buf, nw);
}

int myReadSocket( int s, char *buf, int max){
	int n;
	n = read( s, buf, max);
	return n;
}

void myCloseSocket( int s ){
	close(s);
}
