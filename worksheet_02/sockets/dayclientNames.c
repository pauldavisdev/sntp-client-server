/*-------1---------2---------3---------4---------5---------6---------7--------*/
/* Page 6 of Stevens, dayclient.c
* A simple daytime client, i.e. connects to port 13.
* Requires IP address to be given on command line,
* e.g. dayclient 127.0.0.1
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket(), connect() */
#include <string.h>     /* bzero() - should now use memset() */
#include <netinet/in.h> /* struct sockaddr_in, htons() */
#include <arpa/inet.h>  /* inet_aton() - should now use inet_pton()! */
#include <unistd.h>     /* read() */
#include <arpa/inet.h>
#include <netdb.h>      /* gethostbyname() */

#define MAXLINE 4096

int main( int argc, char * argv[]) {
  int sockfd;                   /* the socket file descriptor */
  int nbytes;                   /* number of bytes actually read from the socket */
  char recvline[ MAXLINE + 1];  /* to receive the day and time string */
  struct sockaddr_in servaddr;  /* socket internet address structure */
  struct hostent *servinfo;     /* ptr to server /etc/hosts/file entry */

  if( argc != 2) {
    fprintf( stderr, "usage: %s <IPaddress>\n", argv[ 0]);
    return 1;
  }

  servinfo = gethostbyname(argv[1]);  /* get ptr to the entry /etc/hosts */
  if(!servinfo) { /* if no /etc/hosts entry */
    perror("gethostbyname error");
    return 1;
  }

  /* Create an Internet (AF_INET) stream (SOCK_STREAM) socket
  * with default protocol, i.e. a TCP socket.
  * Try changing the default protocol from 0 to 1 to force error! */
  if( (sockfd = socket( AF_INET, SOCK_STREAM, 0)) < 0) {
    perror( "socket error");
    return 1;
  }
  /* zero the entire socket address structure to ensure we set up
  * only what we want! Then specify internet address family to AF_INET.
  * Set the port to that of the daytime server (i.e. 13) converting it from
  * host integer format to network format.
  * Finally convert the ascii quad format of the IP addr to network format.
  */
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  /* should now use PF_INET! */
  servaddr.sin_port = htons( 13); /* daytime server 'well known' port */

  /* copy the hsot server address to the inet address structure */
  memcpy((char *)&servaddr.sin_addr, servinfo->h_addr, servinfo->h_length);

  if( connect( sockfd, (struct sockaddr *) &servaddr, sizeof( servaddr)) < 0) {
    perror( "connect error");
    return 1;
  }
  /* Read data from the network. No bytes read implies end or error! */
  while( (nbytes = read( sockfd, recvline, MAXLINE)) > 0) {
    recvline[ nbytes] = '\0'; /* end-of-string char */
    if( fputs( recvline, stdout) == EOF) {
      perror( "fputs error");
      return 1;
    }
  } /* while */
  if( nbytes < 0) {
    perror( "read error");
    return 1;
  }
  exit( 0);
} /* main */
