
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

int myServerSocket( int port );

int myAcceptServerSocket( int s);

int myConnectSocket(char *hostname, int port);

int myWriteSocket( int s, char *buf, int nw);

int myReadSocket( int s, char *buf, int max);

void myCloseSocket( int s );




