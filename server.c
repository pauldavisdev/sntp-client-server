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
 #include "sntp.h"

 #define MYPORT 63333    /* the port users connect to */

int main(void) {

  int sockfd;
  struct sockaddr_in my_addr;     /* info for my addr i.e. server */
  struct sockaddr_in their_addr;  /* client's address info */
  int numbytes;

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

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);

  ntp_packet packet;
  struct timeval tv;
  memset(&packet, 0, sizeof(ntp_packet));

while(1)
{
  if((numbytes = recvfrom(sockfd, &packet, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("listener recv from");
    exit(1);
}

  network_to_host(&packet);

  packet.recvTimestamp = getCurrentTimestamp(&tv);
  packet.orgTimestamp = packet.transmitTimestamp;

  set_server_flags(&packet);

  packet.stratum = 1;
  packet.orgTimestamp = packet.transmitTimestamp;
  packet.transmitTimestamp = getCurrentTimestamp(&tv);
  /* host to network byte order */
  host_to_network(&packet);

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
