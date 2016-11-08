/*  listener.c - a datagram socket 'server'
 *  displays message received then dies!
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <errno.h>
 #include <string.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>

 #define MYPORT 123    /* the port users connect to */
 #define MAXBUFLEN 100

int main(void) {

  int sockfd;
  struct sockaddr_in my_addr;     /* info for my addr i.e. server */
  struct sockaddr_in their_addr;  /* client's address info */
  int addr_len, numbytes;
  char buf[MAXBUFLEN];

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("Listener socket");
    exit(1);
  }

  memset(&my_addr, 0, sizeof(my_addr)); /* zero my_addr struct */
  my_addr.sin_family = AF_INET;         /* host byte order ... */
  my_addr.sin_port = htons(MYPORT);     /* ... short, network byte order*/
  my_addr.sin_addr.s_addr = INADDR_ANY; /* any of server IP addrs */

  if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
    perror("Listener bind");
    exit(1);
  }

  addr_len = sizeof(struct sockaddr);
  if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
              (struct sockaddr_in *)&their_addr, &addr_len)) == -1) {
      perror("Listener recvfrom");
      exit(1);
  }

  printf("Got packet from %s\n", inet_ntoa(their_addr.sin_addr));
  printf("Packet is %d bytes long\n", numbytes);
  buf[numbytes] = '\0';   /* end of string */
  printf("Packet contains \"%s\"\n", buf);

  close(sockfd);
  return 0;
}
