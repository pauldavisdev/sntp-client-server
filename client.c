/*  client.c - a datagram 'client'
 *  need to supply host name/IP and one word message
 *  e.g. client localhost hello
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "sntp.h"

#define PORT 123         /* server port the client connects to */

int main(int argc, char const *argv[]) {
  int sockfd, numbytes;
  struct hostent *he;            /* server data struct */
  struct sockaddr_in their_addr; /* server addr info */

  ntp_packet packet;
  // zero packet struct
  memset(&packet, 0, sizeof(packet));
  // set the first 8 bits of struct, so li is 0, vn is 4, mode is 3
  *((char *)&packet) = 0b00100011;

  if(argc != 2) {
    fprintf(stderr, "usage: client serverIP\n");
    exit(1);
  }

  /* resolve server host name or IP address */
  if((he = gethostbyname(argv[1])) == NULL) {
    perror("server gethostbyname");
    exit(1);
  }

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("client socket");
    exit(1);
  }



  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(PORT);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  if((numbytes = sendto(sockfd, (char*)&packet), sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  printf("Sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));

  close(sockfd);

  return 0;
}
