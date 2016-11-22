/*  client.c - a datagram 'client'
 *
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

  // NTP server address
  char serverIP[] = "0.uk.pool.ntp.org";

  // Initialise NTP packet struct
  ntp_packet packet;

  // zero packet struct
  memset(&packet, 0, sizeof(packet));

  // set the first 8 bits of struct, so li is 0, vn is 4, mode is 3
  *((char *)&packet) = 0b00100011;

  /*if(argc != 2) {
    fprintf(stderr, "usage: client serverIP\n");
    exit(1);
  }*/

  /* resolve server host name or IP address */
  if((he = gethostbyname(serverIP)) == NULL) {
    perror("client gethostbyname");
    exit(1);
  }

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("client socket");
    exit(1);
  }

  printf("Packet left client at: \n");
  gettimeofday(&tv, NULL);
  print_unix_time(&tv);
  convert_unix_to_ntp(&tv, &ntp_temp);
  printf("NTP time: %ld.%ld \n", (long int)ntp_temp.second, (long int)ntp_temp.fraction);

  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(PORT);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  packet.transmitTimestamp = getCurrentTimestamp();

  host_to_network(&packet);


  if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);
  numbytes = 0;

  ntp_packet recvBuf;
  memset(&recvBuf, 0, sizeof(recvBuf));

  if((numbytes = recvfrom(sockfd, &recvBuf, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("listener recv from");
    exit(1);
}

host_to_network(&recvBuf);

printf("\nPacket left server at: \n");
convert_ntp_to_unix(&recvBuf.transmitTimestamp, &tv);
print_unix_time(&tv);

convert_unix_to_ntp(&tv, &ntp_temp);
printf("NTP time: %ld.%ld \n", (long int)ntp_temp.second, (long int)ntp_temp.fraction);

printf("Stratum: %d \n", recvBuf.stratum );

close(sockfd);

  return 0;
}
