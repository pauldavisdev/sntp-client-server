/*  server.c - SNTP 'server'
 *  Receives packet from client and replies
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
 #include "sntp.h"

 #define MYPORT 63333    /* the port clients connect to */

 void set_server_reply(ntp_packet *p);

int main(void) {

  int sockfd;
  struct sockaddr_in my_addr;     /* info for my addr i.e. server */
  struct sockaddr_in their_addr;  /* client's address info */
  int numbytes;

  ntp_packet packet;
  memset(&packet, 0, sizeof(ntp_packet));

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
  perror("server socket");
  exit(1);
  }

  memset(&my_addr, 0, sizeof(my_addr)); /* zero my_addr struct */
  my_addr.sin_family = AF_INET;         /* host byte order ... */
  my_addr.sin_port = htons(MYPORT);     /* ... short, network byte order*/
  my_addr.sin_addr.s_addr = INADDR_ANY; /* any of server IP addrs */

  if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
    perror("server bind");
    exit(1);
  }

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);

  while(1)
  {
    if((numbytes = recvfrom(sockfd, &packet, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("server recv from");
    exit(1);
    }

    set_server_reply(&packet);

    /* send packet */
    if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
        perror("client sendto");
        exit(1);
    }
  }

  close(sockfd);
  return 0;
}

void set_server_reply(ntp_packet *p)
{
  network_to_host(p);

  p->recvTimestamp = getCurrentTimestamp();
  p->orgTimestamp = p->transmitTimestamp;

  /* set up flag bitfield of struct, so li is 0, vn is 4, mode is 4 */
  // li
  p->flags = 0;
  p->flags <<= 3;
  // vn
  p->flags |= 4;
  p->flags <<= 3;
  // mode
  p->flags |= 4;
  // set stratum to 2
  p->stratum = 2;

  p->transmitTimestamp = getCurrentTimestamp();

  print_ntp_packet(p);

  host_to_network(p);
}
