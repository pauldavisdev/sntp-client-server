/*-------1---------2---------3---------4---------5---------6---------7--------*/
/* Page 13 of Stevens, dayserver.c
* A simple daytime server, replacing the normal one on port 13.
* Use telnet 127.0.0.1 portNo to connect.
* Try telnet localhost or to a neighbour's host and port number.
* On a slow machine try netstat again to see left over connection!
* When experimentation complete kill -9 pid to remove the server.
* The port may still be in use if restart too quickly - bind() error!
*/
#include <stdio.h>
#include <sys/socket.h> /* socket(), bind(), listen(), accept() */
#include <string.h>     /* bzero() - should now use memset() */
#include <netinet/in.h> /* struct sockaddr_in, htons(), htonl() */
#include<unistd.h>      /* write(), close() */
#include <arpa/inet.h>

#include <time.h>

#define MAXLINE 4096
#define MAXQ 10

int main( int argc, char * argv[]) {
  int listenfd;
  int connfd;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;
  int lenstruct;
  char buff[ MAXLINE];
  time_t ticks;

  /* Create socket for listening on and set up internet address strucure */
  listenfd = socket( AF_INET, SOCK_STREAM, 0);
  if( listenfd < 0) {
    perror( "socket error");
    return 1;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons( 60000); /* daytime 'well known' port in use! */
  /* allow connection on any of my IP addresses - if I have more than one! */
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY);

  lenstruct = sizeof( servaddr);
  /* bind the inet address structure to the listening socket ready for use */
  if( (bind( listenfd, (struct sockaddr *) &servaddr, lenstruct)) < 0) {
    perror( "bind error");
    return 1;
  }
  /* actually put the socket in listen mode and set the q length */
  if( listen( listenfd, MAXQ) < 0) {
    perror( "listen error");
    return 1;
  }
  for( ; ; ) { /* loop forever */
    /* wait (block) for a client request on the listen socket
    * when a request appears connect it to the new socket connfd
    * ready for the actual communication with the client.
    * A successful connection will also get client IP details.
    */
    connfd = accept( listenfd, (struct sockaddr *) &cliaddr, (socklen_t*)&lenstruct);
    if( connfd < 0) {
      perror( "accept error");
      return 1;
    }
    /* get the date and time, and write it to the connected socket */
    ticks = time( NULL);
    snprintf( buff, sizeof( buff), "%.24s\r\n", ctime( &ticks));
    if( write( connfd, buff, strlen( buff)) < 0) {
      fprintf( stderr, "write error\n");
      return 1;
    }
    /* close the connected socket */
    close( connfd);
  } /* for ever */

  /* should never get here, will need to kill the process!
  * So the listen socket will remain open for a while -
  * until garbage collection clears up! */
close( listenfd);
} /* main */
